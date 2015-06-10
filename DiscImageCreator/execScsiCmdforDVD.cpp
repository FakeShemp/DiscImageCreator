/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "check.h"
#include "convert.h"
#include "execIoctl.h"
#include "execScsiCmdforDVD.h"
#include "output.h"
#include "outputScsiCmdLog.h"
#include "outputScsiCmdLogforDVD.h"

BOOL ReadDVD(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszPath
	)
{
	FILE* fp = CreateOrOpenFileW(
		pszPath, NULL, NULL, NULL, NULL, _T(".iso"), _T("wb"), 0, 0);
	if (!fp) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = TRUE;
	LPBYTE pBuf = NULL;
	try {
		if (NULL == (pBuf = (LPBYTE)calloc(
			pDevice->uiMaxTransferLength + pDevice->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevice, pBuf);

		UINT uiTransferLen = pDevice->uiMaxTransferLength / DISC_RAW_READ;
		CDB::_READ12 cdb = { 0 };
		cdb.OperationCode = SCSIOP_READ12;
		cdb.LogicalUnitNumber = pDevice->address.Lun;
		cdb.TransferLength[0] = HIBYTE(HIWORD(uiTransferLen));
		cdb.TransferLength[1] = LOBYTE(HIWORD(uiTransferLen));
		cdb.TransferLength[2] = HIBYTE(LOWORD(uiTransferLen));
		cdb.TransferLength[3] = LOBYTE(LOWORD(uiTransferLen));
		if (pExtArg->bFua) {
			cdb.ForceUnitAccess = TRUE;
		}
		else {
			cdb.Streaming = TRUE;
		}

		BYTE byScsiStatus = 0;
		if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
			pDevice->uiMaxTransferLength, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			OutputErrorString(_T("This drive doesn't support streaming\n"));
			cdb.Streaming = FALSE;
		}

		if (cdb.ForceUnitAccess == FALSE && cdb.Streaming == FALSE) {
			if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
				pDevice->uiMaxTransferLength, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
		}
		INT nVal1 = DISC_RAW_READ * 16;
		INT nVal2 = DISC_RAW_READ * 21;
		for (INT i = nVal1; i <= nVal2; i += DISC_RAW_READ) {
			OutputFsVolumeRecognitionSequence(pExtArg, pDisc, lpBuf + i);
		}

		cdb.LogicalBlock[3] = 32;
		if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf, 
			pDevice->uiMaxTransferLength, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw FALSE;
		}
		if (lpBuf[20] == 0 && lpBuf[21] == 0 && lpBuf[22] == 0 && lpBuf[23] == 0) {
			INT nVal3 = DISC_RAW_READ * 5;
			for (INT i = 0; i <= nVal3; i += DISC_RAW_READ) {
				OutputFsVolumeDescriptorSequence(lpBuf + i);
			}
		}

		cdb.LogicalBlock[2] = 1;
		cdb.LogicalBlock[3] = 0;
		if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf, 
			pDevice->uiMaxTransferLength, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw FALSE;
		}
		OutputFsVolumeDescriptorSequence(lpBuf);

		for (INT nLBA = 0; nLBA < pDisc->SCSI.nAllLength; nLBA += uiTransferLen) {
			if (pDisc->SCSI.nAllLength - nLBA < (INT)uiTransferLen) {
				uiTransferLen = (UINT)(pDisc->SCSI.nAllLength - nLBA);
				cdb.TransferLength[0] = HIBYTE(HIWORD(uiTransferLen));
				cdb.TransferLength[1] = LOBYTE(HIWORD(uiTransferLen));
				cdb.TransferLength[2] = HIBYTE(LOWORD(uiTransferLen));
				cdb.TransferLength[3] = LOBYTE(LOWORD(uiTransferLen));
			}
			cdb.LogicalBlock[0] = HIBYTE(HIWORD(nLBA));
			cdb.LogicalBlock[1] = LOBYTE(HIWORD(nLBA));
			cdb.LogicalBlock[2] = HIBYTE(LOWORD(nLBA));
			cdb.LogicalBlock[3] = LOBYTE(LOWORD(nLBA));
			if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf, 
				pDevice->uiMaxTransferLength, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			fwrite(lpBuf, sizeof(BYTE), (size_t)DISC_RAW_READ * uiTransferLen, fp);
			OutputString(_T("\rCreating iso(LBA) %8u/%8u"),
				nLBA + uiTransferLen - 1, pDisc->SCSI.nAllLength - 1);
		}
		OutputString(_T("\n"));
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FreeAndNull(pBuf);
	FcloseAndNull(fp);
	return bRet;
}

BOOL ReadDVDRaw(
	PDEVICE pDevice,
	PDISC pDisc,
	LPCSTR szVendorId,
	LPCTSTR pszPath
	)
{
	FILE* fp = CreateOrOpenFileW(
		pszPath, NULL, NULL, NULL, NULL, _T(".raw"), _T("wb"), 0, 0);
	if (!fp) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	UINT uiTransferLen = 31;
	LPBYTE pBuf = NULL;
	BOOL bRet = TRUE;
	try {
		if (NULL == (pBuf = (LPBYTE)calloc(DVD_RAW_READ *
			uiTransferLen + pDevice->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevice, pBuf);
		BYTE cdblen = CDB12GENERIC_LENGTH;
		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		if (szVendorId && !strncmp(szVendorId, "PLEXTER", 7)) {
			lpCmd[0] = SCSIOP_READ_DATA_BUFF;
			lpCmd[1] = 0x02;
			lpCmd[2] = 0x00;
			lpCmd[6] = LOBYTE(HIWORD(DVD_RAW_READ * uiTransferLen));
			lpCmd[7] = HIBYTE(LOWORD(DVD_RAW_READ * uiTransferLen));
			lpCmd[8] = LOBYTE(LOWORD(DVD_RAW_READ * uiTransferLen));
			cdblen = CDB10GENERIC_LENGTH;
		}
		else if (szVendorId && !strncmp(szVendorId, "HL-DT-ST", 8)) {
			lpCmd[0] = 0xE7; // vendor specific command (discovered by DaveX)
			lpCmd[1] = 0x48; // H
			lpCmd[2] = 0x49; // I
			lpCmd[3] = 0x54; // T
			lpCmd[4] = 0x01; // read MCU memory sub-command
			lpCmd[10] = HIBYTE(LOWORD(DVD_RAW_READ * uiTransferLen));
			lpCmd[11] = LOBYTE(LOWORD(DVD_RAW_READ * uiTransferLen));
		}

		for (INT nLBA = 0; nLBA < pDisc->SCSI.nAllLength; nLBA += uiTransferLen) {
			if (pDisc->SCSI.nAllLength - nLBA < (INT)uiTransferLen) {
				uiTransferLen = (UINT)(pDisc->SCSI.nAllLength - nLBA);
				if (szVendorId && !strncmp(szVendorId, "PLEXTER", 7)) {
					lpCmd[6] = LOBYTE(HIWORD(DVD_RAW_READ * uiTransferLen));
					lpCmd[7] = HIBYTE(LOWORD(DVD_RAW_READ * uiTransferLen));
					lpCmd[8] = LOBYTE(LOWORD(DVD_RAW_READ * uiTransferLen));
				}
				else if (szVendorId && !strncmp(szVendorId, "HL-DT-ST", 8)) {
					lpCmd[10] = HIBYTE(LOWORD(DVD_RAW_READ * uiTransferLen));
					lpCmd[11] = LOBYTE(LOWORD(DVD_RAW_READ * uiTransferLen));
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
			if (!ScsiPassThroughDirect(pDevice, lpCmd, cdblen, lpBuf, 
				(DWORD)DVD_RAW_READ * uiTransferLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}

			fwrite(lpBuf, sizeof(BYTE), (size_t)DVD_RAW_READ * uiTransferLen, fp);
			OutputString(_T("\rCreating raw(LBA) %8u/%8u"), 
				nLBA + uiTransferLen - 1, pDisc->SCSI.nAllLength - 1);
		}
		OutputString(_T("\n"));
	}
	catch (BOOL bErr) {
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

BOOL ReadDVDForCMI(
	PDEVICE pDevice,
	PDISC pDisc
	)
{
	CONST WORD wSize = 
		sizeof(DVD_DESCRIPTOR_HEADER) + sizeof(DVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR);
	_declspec(align(4)) BYTE pBuf[wSize] = { 0 };

	CDB::_READ_DVD_STRUCTURE cdb = { 0 };
	cdb.OperationCode = SCSIOP_READ_DVD_STRUCTURE;
	cdb.Format = DvdMaxDescriptor;
	cdb.AllocationLength[0] = HIBYTE(wSize);
	cdb.AllocationLength[1] = LOBYTE(wSize);
	OutputDiscLogA("\tCopyright Management Information\n");

	BOOL bRet = TRUE;
	BYTE byScsiStatus = 0;
	for (INT nLBA = 0; nLBA < pDisc->SCSI.nAllLength; nLBA++) {
		cdb.RMDBlockNumber[0] = HIBYTE(HIWORD(nLBA));
		cdb.RMDBlockNumber[1] = LOBYTE(HIWORD(nLBA));
		cdb.RMDBlockNumber[2] = HIBYTE(LOWORD(nLBA));
		cdb.RMDBlockNumber[3] = LOBYTE(LOWORD(nLBA));

		if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH, pBuf, 
			wSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			bRet = FALSE;
			break;
		}
		OutputDVDCopyrightManagementInformation(
			(PDVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR)(pBuf + sizeof(DVD_DESCRIPTOR_HEADER)), nLBA);
		OutputString(
			_T("\rOutputting CMI log(LBA) %8u/%8u"), nLBA, pDisc->SCSI.nAllLength - 1);
	}
	OutputString(_T("\n"));
	return bRet;
}

BOOL ReadDVDStructure(
	PDEVICE pDevice,
	PDISC pDisc
	)
{
	CONST WORD wMaxDVDStructureSize = 
		sizeof(DVD_DESCRIPTOR_HEADER) + sizeof(DVD_STRUCTURE_LIST_ENTRY) * 0xff;
	_declspec(align(4)) BYTE pBuf[wMaxDVDStructureSize] = { 0 };

	CDB::_READ_DVD_STRUCTURE cdb = { 0 };
	cdb.OperationCode = SCSIOP_READ_DVD_STRUCTURE;
	cdb.Format = 0xff;
	cdb.AllocationLength[0] = HIBYTE(wMaxDVDStructureSize);
	cdb.AllocationLength[1] = LOBYTE(wMaxDVDStructureSize);

	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH,
		pBuf, wMaxDVDStructureSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}

	PDVD_DESCRIPTOR_HEADER pDescHeader = ((PDVD_DESCRIPTOR_HEADER)pBuf);
	REVERSE_SHORT(&pDescHeader->Length);
	WORD wDataSize = pDescHeader->Length - sizeof(pDescHeader->Length);
	WORD wEntrySize = wDataSize / sizeof(DVD_STRUCTURE_LIST_ENTRY);

	BYTE byLayerNum = 0;
	OutputInfoLogA("================================ DVDStructure =================================\n");
	OutputDriveLogA(
		"================================ DVDStructure =================================\n"
		"\tDVDStructureList\n");
	for (WORD i = 0; i < wEntrySize; i++) {
		PDVD_STRUCTURE_LIST_ENTRY pEntry = 
			((PDVD_STRUCTURE_LIST_ENTRY)pDescHeader->Data + i);
		WORD wFormatLen = MAKEWORD(pEntry->FormatLength[1], pEntry->FormatLength[0]);
		OutputDriveLogA(
			"\t\tFormatCode: %02x, Sendable: %3s, Readable: %3s, FormatLength: %u\n", 
			pEntry->FormatCode,
			BOOLEAN_TO_STRING_YES_NO_A(pEntry->Sendable),
			BOOLEAN_TO_STRING_YES_NO_A(pEntry->Readable),
			wFormatLen);

		if (wFormatLen == 0 || pEntry->FormatCode == 0x05 || pEntry->FormatCode == 0xff) {
			continue;
		}
		else if ((0x08 <= pEntry->FormatCode && pEntry->FormatCode <= 0x0B) &&
			pDisc->SCSI.wCurrentMedia != ProfileDvdRam) {
			continue;
		}
		else if (((0x0C <= pEntry->FormatCode && 
			pEntry->FormatCode <= 0x11) || pEntry->FormatCode == 0x30) &&
			((pDisc->SCSI.wCurrentMedia != ProfileDvdRecordable) &&
			(pDisc->SCSI.wCurrentMedia != ProfileDvdRWSequential) &&
			(pDisc->SCSI.wCurrentMedia != ProfileDvdPlusRW) &&
			(pDisc->SCSI.wCurrentMedia != ProfileDvdPlusR))) {
			continue;
		}
		else if ((0x20 <= pEntry->FormatCode && pEntry->FormatCode <= 0x24) &&
			pDisc->SCSI.wCurrentMedia != ProfileDvdDashRDualLayer) {
			continue;
		}
		else if ((pEntry->FormatCode == 0xc0) &&
			pDisc->SCSI.wCurrentMedia != ProfileDvdRewritable) {
			continue;
		}

		LPBYTE lpFormat = (LPBYTE)calloc(wFormatLen, sizeof(BYTE));
		if (!lpFormat) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		cdb.LayerNumber = 0;
		cdb.Format = pEntry->FormatCode;
		cdb.AllocationLength[0] = HIBYTE(wFormatLen);
		cdb.AllocationLength[1] = LOBYTE(wFormatLen);

		if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH, 
			lpFormat, wFormatLen, &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
			byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			OutputErrorString(_T("Failure - Format %02x\n"), pEntry->FormatCode);
		}
		else {
			OutputDVDStructureFormat(pDisc, pEntry->FormatCode, 
				wFormatLen - sizeof(DVD_DESCRIPTOR_HEADER), 
				lpFormat + sizeof(DVD_DESCRIPTOR_HEADER), &byLayerNum, 0,
				pDevice->bSuccessReadToc);
			if (byLayerNum == 1 &&
				(pEntry->FormatCode == 0 || pEntry->FormatCode == 0x01 || 
				pEntry->FormatCode == 0x04 || pEntry->FormatCode == 0x10 || 
				pEntry->FormatCode == 0x12 || pEntry->FormatCode == 0x15)) {
				cdb.LayerNumber = byLayerNum;

				if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH, 
					lpFormat, wFormatLen, &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
					byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
					OutputErrorString(_T("Failure - Format %02x\n"), pEntry->FormatCode);
				}
				else {
					OutputDVDStructureFormat(pDisc, pEntry->FormatCode,
						wFormatLen - sizeof(DVD_DESCRIPTOR_HEADER), 
						lpFormat + sizeof(DVD_DESCRIPTOR_HEADER), &byLayerNum,
						1, pDevice->bSuccessReadToc);
				}
			}
		}
		FreeAndNull(lpFormat);
	}
	return TRUE;
}
