/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"
#include "struct.h"
#include "check.h"
#include "convert.h"
#include "execIoctl.h"
#include "execMMCforDVD.h"
#include "output.h"
#include "outputMMCLog.h"

BOOL ReadDVD(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	PEXT_ARG pExtArg,
	LPCTSTR pszPath
	)
{
	FILE* fp = CreateOrOpenFileW(
		pszPath, NULL, NULL, NULL, _T(".iso"), _T("wb"), 0, 0);
	if (!fp) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = TRUE;
	BYTE byTransferLen = (BYTE)(pDevData->uiMaxTransferLength / DISC_RAW_READ);
	LPBYTE pPBuf = NULL;
	LPBYTE pPBuf2 = NULL;
	try {
		pPBuf = (LPBYTE)calloc(pDevData->uiMaxTransferLength + pDevData->AlignmentMask, sizeof(BYTE));
		if (!pPBuf) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevData, pPBuf);

		BYTE byScsiStatus = 0;
		CDB cdb = { 0 };
		cdb.READ12.OperationCode = SCSIOP_READ12;
		cdb.READ12.TransferLength[3] = byTransferLen;
		if (pExtArg->bFua) {
			cdb.READ12.ForceUnitAccess = TRUE;
		}
		else {
			cdb.READ12.Streaming = TRUE;
		}
		if (!ScsiPassThroughDirect(pDevData, &cdb.READ12, CDB12GENERIC_LENGTH, 
			lpBuf, pDevData->uiMaxTransferLength, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			OutputErrorString(
				_T("[F:%s][L:%d] Not supported Streaming\n"),
				_T(__FUNCTION__), __LINE__);
			cdb.READ12.Streaming = FALSE;
		}

		if (!ScsiPassThroughDirect(pDevData, &cdb.READ12, CDB12GENERIC_LENGTH, 
			lpBuf, pDevData->uiMaxTransferLength, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw FALSE;
		}
		for (INT i = DISC_RAW_READ * 16; i <= DISC_RAW_READ * 21; i += DISC_RAW_READ) {
			OutputFsVolumeRecognitionSequence(lpBuf, i);
		}

		cdb.READ12.LogicalBlock[3] = 32;
		if (!ScsiPassThroughDirect(pDevData, &cdb.READ12, CDB12GENERIC_LENGTH, 
			lpBuf, pDevData->uiMaxTransferLength, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw FALSE;
		}
		if (lpBuf[20] == 0 && lpBuf[21] == 0 && lpBuf[22] == 0 && lpBuf[23] == 0) {
			for (INT i = 0; i <= DISC_RAW_READ * 5; i += DISC_RAW_READ) {
				OutputFsVolumeDescriptorSequence(lpBuf, i);
			}
		}

		cdb.READ12.LogicalBlock[2] = 1;
		cdb.READ12.LogicalBlock[3] = 0;
		if (!ScsiPassThroughDirect(pDevData, &cdb.READ12, CDB12GENERIC_LENGTH, 
			lpBuf, pDevData->uiMaxTransferLength, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw FALSE;
		}
		OutputFsVolumeDescriptorSequence(lpBuf, 0);

		CDB cdb2 = { 0 };
		LPBYTE pBuf2 = NULL;
		size_t uiSize = 0;
		if (pExtArg->bCmi) {
			pPBuf2 = (LPBYTE)calloc(sizeof(DVD_DESCRIPTOR_HEADER) +
				sizeof(DVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR) +
				pDevData->AlignmentMask, sizeof(BYTE));
			if (!pPBuf2) {
				throw FALSE;
			}
			pBuf2 = (LPBYTE)ConvParagraphBoundary(pDevData, pPBuf2);
			uiSize = sizeof(DVD_DESCRIPTOR_HEADER) + 
				sizeof(DVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR);
			cdb2.READ_DVD_STRUCTURE.OperationCode = SCSIOP_READ_DVD_STRUCTURE;
			cdb2.READ_DVD_STRUCTURE.Format = DvdMaxDescriptor;
			cdb2.READ_DVD_STRUCTURE.AllocationLength[0] = HIBYTE(LOWORD(uiSize));
			cdb2.READ_DVD_STRUCTURE.AllocationLength[1] = LOBYTE(LOWORD(uiSize));
			OutputDiscLog(_T("\tCopyright Management Information\n"));
		}
		for (INT nLBA = 0; nLBA < pDiscData->nAllLength; nLBA += byTransferLen) {
			if (pDiscData->nAllLength - nLBA < byTransferLen) {
				byTransferLen = (BYTE)(pDiscData->nAllLength - nLBA);
				cdb.READ12.TransferLength[3] = byTransferLen;
			}
			cdb.READ12.LogicalBlock[0] = HIBYTE(HIWORD(nLBA));
			cdb.READ12.LogicalBlock[1] = LOBYTE(HIWORD(nLBA));
			cdb.READ12.LogicalBlock[2] = HIBYTE(LOWORD(nLBA));
			cdb.READ12.LogicalBlock[3] = LOBYTE(LOWORD(nLBA));
			if (!ScsiPassThroughDirect(pDevData, &cdb.READ12, CDB12GENERIC_LENGTH, 
				lpBuf, pDevData->uiMaxTransferLength, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			fwrite(lpBuf, sizeof(BYTE), (size_t)DISC_RAW_READ * byTransferLen, fp);

			if (pExtArg->bCmi) {
				for (INT i = 0; i < byTransferLen; i++) {
					cdb2.READ_DVD_STRUCTURE.RMDBlockNumber[0] = HIBYTE(HIWORD(nLBA + i));
					cdb2.READ_DVD_STRUCTURE.RMDBlockNumber[1] = LOBYTE(HIWORD(nLBA + i));
					cdb2.READ_DVD_STRUCTURE.RMDBlockNumber[2] = HIBYTE(LOWORD(nLBA + i));
					cdb2.READ_DVD_STRUCTURE.RMDBlockNumber[3] = LOBYTE(LOWORD(nLBA + i));
					if (!ScsiPassThroughDirect(pDevData, &cdb2.READ_DVD_STRUCTURE, 
						CDB10GENERIC_LENGTH, pBuf2, (DWORD)uiSize, &byScsiStatus, 
						_T(__FUNCTION__), __LINE__)) {
						throw FALSE;
					}
					if (byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
						OutputErrorString(
							_T("[F:%s][L:%d] LBA %7d, Read Disc Structure Cmd error\n"),
							_T(__FUNCTION__), __LINE__, nLBA);
					}
					OutputMmcDVDCopyrightManagementInformation(
						(PDVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR)(pBuf2 + 4), nLBA, i);
				}
			}
			OutputString(_T("\rCreating iso(LBA) %8u/%8u"), 
				nLBA + byTransferLen - 1, pDiscData->nAllLength - 1);
		}
		OutputString(_T("\n"));
	}
	catch(BOOL ret) {
		bRet = ret;
	}
	FreeAndNull(pPBuf);
	if (pExtArg->bCmi) {
		FreeAndNull(pPBuf2);
	}
	FcloseAndNull(fp);
	return bRet;
}

BOOL ReadDVDRaw(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	LPCSTR szVendorId,
	LPCTSTR pszPath
	)
{
	FILE* fp = CreateOrOpenFileW(
		pszPath, NULL, NULL, NULL, _T(".raw"), _T("wb"), 0, 0);
	if (!fp) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BYTE byTransferLen = 31;
	LPBYTE pBuf = NULL;
	BOOL bRet = TRUE;
	try {
		if (NULL == (pBuf = (LPBYTE)calloc(DVD_RAW_READ *
			(size_t)byTransferLen + pDevData->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevData, pBuf);
		BYTE cdblen = CDB12GENERIC_LENGTH;
		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		if (szVendorId && !strncmp(szVendorId, "PLEXTER", 7)) {
			lpCmd[0] = SCSIOP_READ_DATA_BUFF;
			lpCmd[1] = 0x02;
			lpCmd[2] = 0x00;
			lpCmd[6] = LOBYTE(HIWORD(DVD_RAW_READ * (size_t)byTransferLen));
			lpCmd[7] = HIBYTE(LOWORD(DVD_RAW_READ * (size_t)byTransferLen));
			lpCmd[8] = LOBYTE(LOWORD(DVD_RAW_READ * (size_t)byTransferLen));
			cdblen = CDB10GENERIC_LENGTH;
		}
		else if (szVendorId && !strncmp(szVendorId, "HL-DT-ST", 8)) {
			lpCmd[0] = 0xE7; // vendor specific command (discovered by DaveX)
			lpCmd[1] = 0x48; // H
			lpCmd[2] = 0x49; // I
			lpCmd[3] = 0x54; // T
			lpCmd[4] = 0x01; // read MCU memory sub-command
			lpCmd[10] = HIBYTE(LOWORD(DVD_RAW_READ * (size_t)byTransferLen));
			lpCmd[11] = LOBYTE(LOWORD(DVD_RAW_READ * (size_t)byTransferLen));
		}

		for (INT nLBA = 0; nLBA < pDiscData->nAllLength; nLBA += byTransferLen) {
			if (pDiscData->nAllLength - nLBA < byTransferLen) {
				byTransferLen = (BYTE)(pDiscData->nAllLength - nLBA);
				if (szVendorId && !strncmp(szVendorId, "PLEXTER", 7)) {
					lpCmd[6] = LOBYTE(HIWORD(DVD_RAW_READ * (size_t)byTransferLen));
					lpCmd[7] = HIBYTE(LOWORD(DVD_RAW_READ * (size_t)byTransferLen));
					lpCmd[8] = LOBYTE(LOWORD(DVD_RAW_READ * (size_t)byTransferLen));
				}
				else if (szVendorId && !strncmp(szVendorId, "HL-DT-ST", 8)) {
					lpCmd[10] = HIBYTE(LOWORD(DVD_RAW_READ * (size_t)byTransferLen));
					lpCmd[11] = LOBYTE(LOWORD(DVD_RAW_READ * (size_t)byTransferLen));
				}
			}
			if (szVendorId && !strncmp(szVendorId, "PLEXTER", 7)) {
				lpCmd[3] = LOBYTE(HIWORD(nLBA));
				lpCmd[4] = HIBYTE(LOWORD(nLBA));
				lpCmd[5] = LOBYTE(LOWORD(nLBA));
			}
			else if (szVendorId && !strncmp(szVendorId, "HL-DT-ST", 8)) {
				lpCmd[6] = HIBYTE(HIWORD(nLBA));
				lpCmd[7] = LOBYTE(HIWORD(nLBA));
				lpCmd[8] = HIBYTE(LOWORD(nLBA));
				lpCmd[9] = LOBYTE(LOWORD(nLBA));
			}
			BYTE byScsiStatus = 0;
			if (!ScsiPassThroughDirect(pDevData, lpCmd, cdblen, lpBuf, 
				(DWORD)DVD_RAW_READ * byTransferLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}

			fwrite(lpBuf, sizeof(BYTE), (size_t)DVD_RAW_READ * byTransferLen, fp);
			OutputString(_T("\rCreating raw(LBA) %7d/%7d"), 
				nLBA + byTransferLen - 1, pDiscData->nAllLength - 1);
		}
		OutputString(_T("\n"));
	}
	catch(BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(pBuf);
	FcloseAndNull(fp);
#if 0
	// TODO: descrambled wii-rom
	unscrambler *u = unscrambler_new ();
	CHAR pszPathA[_MAX_PATH];
	ZeroMemory(pszPathA, sizeof(pszPathA));
	WideCharToMultiByte(CP_THREAD_ACP, 0, pszPath, -1, pszPathA, _MAX_PATH, NULL, NULL);

	CHAR pszInPath[_MAX_PATH];
	ZeroMemory(pszInPath, sizeof(pszInPath));
	FILE* fpIn = CreateOrOpenFileA(pszPathA, pszInPath, NULL, ".raw", "rb", 0, 0);
	if (!fpIn) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	CHAR pszOutPath[_MAX_PATH];
	ZeroMemory(pszOutPath, sizeof(pszOutPath));
	FILE* fpOut = CreateOrOpenFileA(pszPathA, pszOutPath, NULL, ".iso", "wb", 0, 0);
	if (!fpOut) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	UINT current_sector = 0;
	unscrambler_set_disctype(3);
	unscrambler_unscramble_file(u, pszInPath, pszOutPath, &current_sector);
	u = (unscrambler *)unscrambler_destroy (u);
#endif
	return bRet;
}

BOOL ReadDVDStructure(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData
	)
{
	CDB cdb = { 0 };
	size_t uiMaxDVDStructureSize = sizeof(DVD_STRUCTURE_LIST_ENTRY) * 0xFF;

	cdb.READ_DVD_STRUCTURE.OperationCode = SCSIOP_READ_DVD_STRUCTURE;
	cdb.READ_DVD_STRUCTURE.Format = 0xFF;
	cdb.READ_DVD_STRUCTURE.AllocationLength[0] = HIBYTE(uiMaxDVDStructureSize);
	cdb.READ_DVD_STRUCTURE.AllocationLength[1] = LOBYTE(uiMaxDVDStructureSize);

	BYTE pDiscStructure[sizeof(DVD_STRUCTURE_LIST_ENTRY)* 0xFF + 0x10];
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pDevData, &cdb.READ_DVD_STRUCTURE, CDB10GENERIC_LENGTH, 
		pDiscStructure, (DWORD)uiMaxDVDStructureSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}

	WORD wDataLen = MAKEWORD(pDiscStructure[1], pDiscStructure[0]);
	INT nAllStructureSize = wDataLen - 2;
	size_t uiFormatSize = (size_t)(nAllStructureSize / 4);
	if (uiFormatSize == 0) {
		OutputDiscLog(_T("DVD Structure failed. formatSize is 0\n"));
		return FALSE;
	}
	LPBYTE lpFormat = (LPBYTE)calloc(uiFormatSize, sizeof(BYTE));
	if (!lpFormat) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}

	BOOL bRet = TRUE;
	LPWORD lpStructureLength = NULL;
	LPBYTE lpStructure = NULL;
	try {
		if (NULL == (lpStructureLength =(LPWORD)calloc(uiFormatSize, sizeof(WORD)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		OutputDiscLog(_T("DVD Structure\n"));
		for (INT i = 0, j = 0; i < nAllStructureSize; i+=4, j++) {
			if (i == 0) {
				OutputDiscLog(_T("\tDisc Structure List\n"));
			}
			OutputDiscLog(
				_T("\t\tFormatCode: %02x, SDR: %3s, RDS: %3s, Structure Length: %d\n"), 
				pDiscStructure[4 + i],
				BOOLEAN_TO_STRING_YES_NO(pDiscStructure[5 + i] & 0x80),
				BOOLEAN_TO_STRING_YES_NO(pDiscStructure[5 + i] & 0x40),
				(pDiscStructure[6 + i] << 8) | pDiscStructure[7 + i]);
			lpFormat[j] = pDiscStructure[4 + i];
			lpStructureLength[j] =
				MAKEWORD(pDiscStructure[7 + i], pDiscStructure[6 + i]);
		}

		BYTE byLayerNum = 0;
		for (size_t i = 0; i < uiFormatSize; i++) {
			if (lpStructureLength[i] == 0 || lpFormat[i] == 0x05 || lpFormat[i] == 0xff) {
				continue;
			}
			else if ((0x08 <= lpFormat[i] && lpFormat[i] <= 0x0B) &&
				pDiscData->wCurrentMedia != ProfileDvdRam) {
				continue;
			}
			else if (((0x0C <= lpFormat[i] && lpFormat[i] <= 0x11) || lpFormat[i] == 0x30) &&
				((pDiscData->wCurrentMedia != ProfileDvdRecordable) &&
				(pDiscData->wCurrentMedia != ProfileDvdRWSequential) &&
				(pDiscData->wCurrentMedia != ProfileDvdPlusRW) &&
				(pDiscData->wCurrentMedia != ProfileDvdPlusR))) {
				continue;
			}
			else if ((0x20 <= lpFormat[i] && lpFormat[i] <= 0x24) &&
				pDiscData->wCurrentMedia != ProfileDvdDashRDualLayer) {
				continue;
			}
			else if ((lpFormat[i] == 0xC0) &&
				pDiscData->wCurrentMedia != ProfileDvdRewritable) {
				continue;
			}

			if (NULL == (lpStructure = (LPBYTE)calloc(lpStructureLength[i], sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			cdb.READ_DVD_STRUCTURE.LayerNumber = 0;
			cdb.READ_DVD_STRUCTURE.Format = lpFormat[i];
			cdb.READ_DVD_STRUCTURE.AllocationLength[0] = HIBYTE(lpStructureLength[i]);
			cdb.READ_DVD_STRUCTURE.AllocationLength[1] = LOBYTE(lpStructureLength[i]);

			if (!ScsiPassThroughDirect(pDevData, &cdb.READ_DVD_STRUCTURE, 
				CDB12GENERIC_LENGTH, lpStructure, lpStructureLength[i], 
				&byScsiStatus, _T(__FUNCTION__), __LINE__) ||
				byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				OutputErrorString(
					_T("[F:%s][L:%d] Failure - Format %02x\n"),
					_T(__FUNCTION__), __LINE__, lpFormat[i]);
			}
			else {
				OutputMmcDVDStructureFormat(pDiscData, lpStructure, 
					lpStructureLength, &byLayerNum, lpFormat, i, 0);
				if (byLayerNum == 1 &&
					(lpFormat[i] == 0 || lpFormat[i] == 0x01 || lpFormat[i] == 0x04 ||
					lpFormat[i] == 0x10 || lpFormat[i] == 0x12 || lpFormat[i] == 0x15)) {
					cdb.READ_DVD_STRUCTURE.LayerNumber = byLayerNum;

					if (!ScsiPassThroughDirect(pDevData, &cdb.READ_DVD_STRUCTURE, 
						CDB12GENERIC_LENGTH, lpStructure, lpStructureLength[i], 
						&byScsiStatus, _T(__FUNCTION__), __LINE__) ||
						byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
						OutputErrorString(
							_T("[F:%s][L:%d] Failure - Format %02x\n"),
							_T(__FUNCTION__), __LINE__, lpFormat[i]);
					}
					else {
						OutputMmcDVDStructureFormat(pDiscData, lpStructure, 
							lpStructureLength, &byLayerNum, lpFormat, i, 1);
					}
				}
			}
			FreeAndNull(lpStructure);
		}
	}
	catch(BOOL ret) {
		bRet = ret;
	}
	FreeAndNull(lpFormat);
	FreeAndNull(lpStructure);
	FreeAndNull(lpStructureLength);
	return bRet;
}
