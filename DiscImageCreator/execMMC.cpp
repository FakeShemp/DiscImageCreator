/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"
#include "struct.h"
#include "check.h"
#include "convert.h"
#include "execIoctl.h"
#include "execMMC.h"
#include "init.h"
#include "get.h"
#include "output.h"
#include "outputMMCLog.h"
#include "set.h"

BOOL GetConfiguration(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData
	)
{
	BYTE byScsiStatus = 0;
	CDB cdb = { 0 };
	GET_CONFIGURATION_HEADER configHeader = { 0 };
	DWORD dwConfigHeaderSize = sizeof(GET_CONFIGURATION_HEADER);

	cdb.GET_CONFIGURATION.OperationCode = SCSIOP_GET_CONFIGURATION;
	cdb.GET_CONFIGURATION.RequestType = SCSI_GET_CONFIGURATION_REQUEST_TYPE_CURRENT;
	cdb.GET_CONFIGURATION.StartingFeature[1] = FeatureProfileList;
	cdb.GET_CONFIGURATION.AllocationLength[0] = HIBYTE(dwConfigHeaderSize);
	cdb.GET_CONFIGURATION.AllocationLength[1] = LOBYTE(dwConfigHeaderSize);

	OutputDriveLog(_T("Configuration\n"));
	BOOL bRet = ScsiPassThroughDirect(pDevData, &cdb.GET_CONFIGURATION, CDB10GENERIC_LENGTH,
		&configHeader, dwConfigHeaderSize, &byScsiStatus, _T(__FUNCTION__), __LINE__);
	if (!bRet || byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		pDiscData->wCurrentMedia = ProfileCdrom;
		// not false. because undefined mmc1..
		OutputDriveLog(
			_T("\tUndefined SCSIOP_GET_CONFIGURATION Command on this drive\n"));
		bRet = TRUE;
	}
	else {
		OutputDriveLog(_T("\tCurrenProfile: "));
		pDiscData->wCurrentMedia =
			MAKEWORD(configHeader.CurrentProfile[1], configHeader.CurrentProfile[0]);
		OutputMmcFeatureProfileType(pDiscData->wCurrentMedia);
		OutputDriveLog(_T("\n"));

		DWORD dwAllLen =
			MAKELONG(MAKEWORD(configHeader.DataLength[3], configHeader.DataLength[2]), 
				MAKEWORD(configHeader.DataLength[1], configHeader.DataLength[0])) +
				sizeof(configHeader.DataLength);
		LPBYTE pPConf = (LPBYTE)calloc(
			(size_t)dwAllLen + pDevData->AlignmentMask, sizeof(BYTE));
		if (!pPConf) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}

		LPBYTE lpConf = (LPBYTE)ConvParagraphBoundary(pDevData, pPConf);
		cdb.GET_CONFIGURATION.AllocationLength[0] = HIBYTE(dwAllLen);
		cdb.GET_CONFIGURATION.AllocationLength[1] = LOBYTE(dwAllLen);

		bRet = ScsiPassThroughDirect(pDevData, &cdb.GET_CONFIGURATION, CDB10GENERIC_LENGTH,
			lpConf, dwAllLen, &byScsiStatus, _T(__FUNCTION__), __LINE__);
		if (!bRet || byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			// not false. because undefined mmc1..
			OutputDriveLog(
				_T("\tUndefined SCSIOP_GET_CONFIGURATION Command on this drive\n"));
			bRet = TRUE;
		}
		else {
			OutputMmcFeatureNumber(pDevData, lpConf, dwAllLen, dwConfigHeaderSize);
		}
		FreeAndNull(pPConf);
	}
	return bRet;
}

BOOL Inquiry(
	PDEVICE_DATA pDevData
	)
{
	BYTE byScsiStatus = 0;
	CDB cdb = { 0 };
	INQUIRYDATA inquiryData = { 0 };

	cdb.CDB6INQUIRY3.OperationCode = SCSIOP_INQUIRY;
	cdb.CDB6INQUIRY3.AllocationLength = sizeof(INQUIRYDATA);

	if (!ScsiPassThroughDirect(pDevData, &cdb.CDB6INQUIRY3, CDB6GENERIC_LENGTH, &inquiryData, 
		sizeof(INQUIRYDATA), &byScsiStatus, _T(__FUNCTION__), __LINE__) || 
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	OutputMmcInquiryData(pDevData, &inquiryData);
	return TRUE;
}

BOOL ReadBufferCapacity(
	PDEVICE_DATA pDevData
	)
{
	BYTE byScsiStatus = 0;
	CDB cdb = { 0 };
	READ_BUFFER_CAPACITY_DATA readBufCapaData = { 0 };

	cdb.READ_BUFFER_CAPACITY.OperationCode = SCSIOP_READ_BUFFER_CAPACITY;
	cdb.READ_BUFFER_CAPACITY.AllocationLength[1] = sizeof(READ_BUFFER_CAPACITY_DATA);

	if (!ScsiPassThroughDirect(pDevData, &cdb.READ_BUFFER_CAPACITY, 
		CDB12GENERIC_LENGTH, &readBufCapaData, sizeof(READ_BUFFER_CAPACITY_DATA), 
		&byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	OutputMmcBufferCapacity(&readBufCapaData);
	return TRUE;
}

BOOL ReadDiscInformation(
	PDEVICE_DATA pDevData
	)
{
	BYTE byScsiStatus = 0;
	CDB cdb = { 0 };
	DISC_INFORMATION discInformation = { 0 };

	cdb.READ_DISC_INFORMATION.OperationCode = SCSIOP_READ_DISC_INFORMATION;
	cdb.READ_DISC_INFORMATION.AllocationLength[1] = sizeof(DISC_INFORMATION);

	if (!ScsiPassThroughDirect(pDevData, &cdb.READ_DISC_INFORMATION, 
		CDB10GENERIC_LENGTH, &discInformation, sizeof(DISC_INFORMATION), &byScsiStatus,
		_T(__FUNCTION__), __LINE__) || 
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	OutputMmcDiscInformation(&discInformation);
	return TRUE;
}

BOOL ReadTOC(
	PEXEC_TYPE pExecType,
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData
	)
{
	BYTE byScsiStatus = 0;
	CDB cdb = { 0 };

	cdb.READ_TOC.OperationCode = SCSIOP_READ_TOC;
	cdb.READ_TOC.StartingTrack = 1;
	cdb.READ_TOC.AllocationLength[0] = HIBYTE(CDROM_TOC_SIZE);
	cdb.READ_TOC.AllocationLength[1] = LOBYTE(CDROM_TOC_SIZE);

	if (!ScsiPassThroughDirect(pDevData, &cdb.READ_TOC, CDB10GENERIC_LENGTH, 
		&pDiscData->toc, CDROM_TOC_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__) || 
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		if (pDiscData->wCurrentMedia != ProfileDvdRom) {
			return FALSE;
		}
	}
	if (!InitTocData(pExecType, &pDiscData)) {
		return FALSE;
	}
	if (byScsiStatus == SCSISTAT_GOOD) {
		pDiscData->bSuccessReadToc = TRUE;
		SetAndOutputMmcToc(pDiscData);
	}
	return TRUE;
}

BOOL ReadTOCFull(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	FILE* fpCcd
	)
{
	BYTE byScsiStatus = 0;
	CDB cdb = { 0 };
	CDROM_TOC_FULL_TOC_DATA fullToc = { 0 };
	DWORD dwFullTocHeaderSize = sizeof(CDROM_TOC_FULL_TOC_DATA);

	cdb.READ_TOC.OperationCode = SCSIOP_READ_TOC;
	cdb.READ_TOC.StartingTrack = 1;
	cdb.READ_TOC.AllocationLength[0] = HIBYTE(dwFullTocHeaderSize);
	cdb.READ_TOC.AllocationLength[1] = LOBYTE(dwFullTocHeaderSize);
	cdb.READ_TOC.Format2 = CDROM_READ_TOC_EX_FORMAT_FULL_TOC;

	if (!ScsiPassThroughDirect(pDevData, &cdb.READ_TOC, CDB10GENERIC_LENGTH, &fullToc,
		dwFullTocHeaderSize, &byScsiStatus, _T(__FUNCTION__), __LINE__) 
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}

	DWORD dwFullTocSize =
		MAKEWORD(fullToc.Length[1], fullToc.Length[0]) - sizeof(fullToc.Length);
	DWORD dwTocEntries = dwFullTocSize / sizeof(CDROM_TOC_FULL_TOC_DATA_BLOCK);

	if (fpCcd) {
		WriteCcdFileForDisc(dwTocEntries, fullToc.LastCompleteSession, fpCcd);
		if (pDevData->bCanCDText) {
			ReadTOCText(pDevData, pDiscData, fpCcd);
		}
	}
	DWORD dwFullTocStructSize = dwFullTocSize + dwFullTocHeaderSize;
	if (pDevData->bPlextor && pDevData->PLEX_DRIVE_TYPE.bPlextorPX712A && dwFullTocStructSize > 1028) {
		OutputErrorString(
			_T("This drive can't get CDROM_TOC_FULL_TOC_DATA of this disc\n"));
		return TRUE;
	}
	DWORD dwFullTocFixStructSize = dwFullTocStructSize;
	// 4 byte padding
	if (dwFullTocFixStructSize % 4) {
		dwFullTocFixStructSize = (dwFullTocFixStructSize / 4 + 1) * 4;
	}
#ifdef _DEBUG
	OutputDiscLog(
		_T("FullTocSize: %d, FullTocStructSize: %d, FullTocFixStructSize: %d\n"),
		dwFullTocSize, dwFullTocStructSize, dwFullTocFixStructSize);
#endif
	LPBYTE pPFullToc = 
		(LPBYTE)calloc(dwFullTocFixStructSize + pDevData->AlignmentMask, sizeof(BYTE));
	if (!pPFullToc) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	LPBYTE pFullToc = (LPBYTE)ConvParagraphBoundary(pDevData, pPFullToc);

	cdb.READ_TOC.AllocationLength[0] = HIBYTE(dwFullTocFixStructSize);
	cdb.READ_TOC.AllocationLength[1] = LOBYTE(dwFullTocFixStructSize);

	BOOL bRet = TRUE;
	LPBYTE aBuf2 = NULL;
	try {
		if (!ScsiPassThroughDirect(pDevData, &cdb.READ_TOC, CDB10GENERIC_LENGTH, pFullToc, 
			dwFullTocFixStructSize, &byScsiStatus, _T(__FUNCTION__), __LINE__) 
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw FALSE;
		}
		CDROM_TOC_FULL_TOC_DATA_BLOCK* pTocData = 
			((CDROM_TOC_FULL_TOC_DATA*)pFullToc)->Descriptors;
		if (fpCcd) {
			size_t uiIdx = 0;
			// session1, session2
			INT aLBA[] = {0, pDiscData->nFirstDataLBA};
			for (size_t b = 0; b < dwTocEntries; b++) {
				if (pTocData[b].Point < 100 && uiIdx < pTocData[b].SessionNumber) {
					aBuf2 = (LPBYTE)calloc(
						CD_RAW_SECTOR_WITH_SUBCODE_SIZE + pDevData->AlignmentMask, sizeof(BYTE));
					if (!aBuf2) {
						throw FALSE;
					}
					LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevData, aBuf2);
					BYTE aCmd2[CDB12GENERIC_LENGTH] = { 0 };
					SetReadCDCommand(pDevData, NULL, aCmd2, FALSE, READ_CD_FLAG::All, 1);

					BYTE byMode = 0;
					BOOL bMCN = FALSE;
					for (INT nLBA = aLBA[uiIdx]; nLBA < aLBA[uiIdx] + 100; nLBA++) {
						aCmd2[2] = HIBYTE(HIWORD(nLBA));
						aCmd2[3] = LOBYTE(HIWORD(nLBA));
						aCmd2[4] = HIBYTE(LOWORD(nLBA));
						aCmd2[5] = LOBYTE(LOWORD(nLBA));

						if (!ScsiPassThroughDirect(pDevData, aCmd2, CDB12GENERIC_LENGTH, lpBuf,
							CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
							|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
							throw FALSE;
						}
						if (nLBA == aLBA[uiIdx]) {
							byMode = GetMode(lpBuf);
						}
						if (pTocData[b].SessionNumber == 1) {
							BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
							AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);

							BYTE byAdr = (BYTE)(lpSubcode[12] & 0x0F);
							if (byAdr == ADR_ENCODES_MEDIA_CATALOG) {
								bMCN = IsValidMCN(lpSubcode);
								_TCHAR szCatalog[META_CATALOG_SIZE] = { 0 };
								SetMCNToString(pDiscData, lpSubcode, szCatalog, bMCN);
								WriteCcdFileForDiscCatalog(pDiscData, fpCcd);
								break;
							}
						}
						else {
							break;
						}
					}
					FreeAndNull(aBuf2);
					WriteCcdFileForSession(pTocData[b].SessionNumber, byMode, fpCcd);
					uiIdx++;
				}
			}
		}
		SetAndOutputMmcTocFull(pDiscData, &fullToc, pTocData, dwTocEntries, fpCcd);
	}
	catch(BOOL ret) {
		bRet = ret;
	}
	fflush(fpCcd);
	FreeAndNull(pPFullToc);
	FreeAndNull(aBuf2);
	return bRet;
}

BOOL ReadTOCText(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	FILE* fpCcd
	)
{
	BYTE byScsiStatus = 0;
	CDB cdb = { 0 };
	CDROM_TOC_CD_TEXT_DATA tocText = { 0 };
	UINT uiCDTextDataSize = sizeof(CDROM_TOC_CD_TEXT_DATA);

	cdb.READ_TOC.OperationCode = SCSIOP_READ_TOC;
	cdb.READ_TOC.AllocationLength[0] = HIBYTE(uiCDTextDataSize);
	cdb.READ_TOC.AllocationLength[1] = LOBYTE(uiCDTextDataSize);
	cdb.READ_TOC.Format2 = CDROM_READ_TOC_EX_FORMAT_CDTEXT;

	BOOL bRet = ScsiPassThroughDirect(pDevData, &cdb.READ_TOC, CDB10GENERIC_LENGTH, &tocText,
		(DWORD)uiCDTextDataSize, &byScsiStatus, _T(__FUNCTION__), __LINE__);
	OutputDiscLog(_T("CDTEXT on SCSIOP_READ_TOC\n"));
	if (!bRet || byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		// not false. because undefined mmc1..
		OutputDiscLog(
			_T("\tUndefined CDROM_READ_TOC_EX_FORMAT_CDTEXT on this drive\n"));
		bRet = TRUE;
	}
	else {
		UINT uiTocTextsize = 
			MAKEWORD(tocText.Length[1], tocText.Length[0]) - sizeof(tocText.Length);
		WriteCcdFileForDiscCDTextLength(uiTocTextsize, fpCcd);

		UINT uiTocTextEntries = uiTocTextsize / sizeof(CDROM_TOC_CD_TEXT_DATA_BLOCK);
		if (!uiTocTextEntries) {
			OutputDiscLog(_T("\tNothing\n"));
			// many CD is nothing text
			return TRUE;
		}
		WriteCcdFileForCDText(uiTocTextEntries, fpCcd);

		LPBYTE pPTocText = NULL;
		UINT uiCDTextDataMaxSize = uiTocTextsize + uiCDTextDataSize;
		if (NULL == (pPTocText = (LPBYTE)calloc(
			uiCDTextDataMaxSize + pDevData->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		LPBYTE pTocText = (LPBYTE)ConvParagraphBoundary(pDevData, pPTocText);
		cdb.READ_TOC.AllocationLength[0] = HIBYTE(uiCDTextDataMaxSize);
		cdb.READ_TOC.AllocationLength[1] = LOBYTE(uiCDTextDataMaxSize);
		PCHAR pTmpText = NULL;
		try {
			if (!ScsiPassThroughDirect(pDevData, &cdb.READ_TOC, CDB10GENERIC_LENGTH, pTocText, 
				(DWORD)uiCDTextDataMaxSize, &byScsiStatus, _T(__FUNCTION__), __LINE__) 
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc = 
				((PCDROM_TOC_CD_TEXT_DATA)pTocText)->Descriptors;
			WriteCcdFileForCDTextEntry(pDesc, uiTocTextEntries, fpCcd);

			UINT uiAllTextSize = uiTocTextEntries * sizeof(pDesc->Text);
			if (NULL == (pTmpText = (PCHAR)calloc(uiAllTextSize, sizeof(_TCHAR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			UINT entrySize = 0;
			BOOL bUnicode = FALSE;
			while(entrySize < uiTocTextEntries) {
				if (pDesc[entrySize].Unicode == 1) {
					bUnicode = TRUE;
					break;
				}
				entrySize++;
			}
			SetAndOutputMmcTocCDText(pDiscData, pDesc, pTmpText, entrySize, uiAllTextSize);
			if (bUnicode) {
				PWCHAR pTmpWText = NULL;
				if (NULL == (pTmpWText = (PWCHAR)calloc(uiAllTextSize, sizeof(BYTE)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				SetAndOutputMmcTocCDWText(pDiscData, pDesc, pTmpText,
					entrySize, uiTocTextEntries, uiAllTextSize);
				FreeAndNull(pTmpWText);
			}
		}
		catch(BOOL ret) {
			bRet = ret;
		}
		FreeAndNull(pPTocText);
		FreeAndNull(pTmpText);
	}
	return bRet;
}

BOOL SetCDSpeed(
	PDEVICE_DATA pDevData,
	UINT uiCDSpeedNum
	)
{
	BYTE byScsiStatus = 0;
	CDB cdb = { 0 };
	CDROM_SET_SPEED setspeed;
	WORD wCDSpeedList[] = {
		0xFFFF,									// MAX
		CD_RAW_SECTOR_SIZE * 75 * 1 / 1000,		// 176.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 2 / 1000, 	// 352.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 3 / 1000, 	// 529.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 4 / 1000, 	// 705.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 5 / 1000, 	// 882.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 6 / 1000, 	// 1058.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 7 / 1000, 	// 1234.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 8 / 1000, 	// 1411.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 9 / 1000, 	// 1587.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 10 / 1000, 	// 1764.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 11 / 1000, 	// 1940.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 12 / 1000, 	// 2116.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 13 / 1000, 	// 2293.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 14 / 1000, 	// 2469.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 15 / 1000, 	// 2646.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 16 / 1000, 	// 2822.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 17 / 1000, 	// 2998.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 18 / 1000, 	// 3175.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 19 / 1000, 	// 3351.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 20 / 1000, 	// 3528.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 21 / 1000, 	// 3704.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 22 / 1000, 	// 3880.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 23 / 1000, 	// 4057.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 24 / 1000, 	// 4233.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 25 / 1000, 	// 4410.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 26 / 1000, 	// 4586.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 27 / 1000, 	// 4762.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 28 / 1000, 	// 4939.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 29 / 1000, 	// 5115.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 30 / 1000, 	// 5292.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 31 / 1000, 	// 5468.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 32 / 1000, 	// 5644.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 33 / 1000, 	// 5821.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 34 / 1000, 	// 5997.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 35 / 1000, 	// 6174.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 36 / 1000, 	// 6350.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 37 / 1000, 	// 6526.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 38 / 1000, 	// 6703.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 39 / 1000, 	// 6879.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 40 / 1000, 	// 7056.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 41 / 1000, 	// 7232.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 42 / 1000, 	// 7408.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 43 / 1000, 	// 7585.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 44 / 1000, 	// 7761.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 45 / 1000, 	// 7938.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 46 / 1000, 	// 8114.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 47 / 1000, 	// 8290.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 48 / 1000, 	// 8467.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 49 / 1000, 	// 8643.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 50 / 1000, 	// 8820.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 51 / 1000, 	// 8996.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 52 / 1000, 	// 9172.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 53 / 1000, 	// 9349.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 54 / 1000, 	// 9525.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 55 / 1000, 	// 9702.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 56 / 1000, 	// 9878.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 57 / 1000, 	// 10054.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 58 / 1000, 	// 10231.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 59 / 1000, 	// 10407.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 60 / 1000, 	// 10584.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 61 / 1000, 	// 10760.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 62 / 1000, 	// 10936.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 63 / 1000, 	// 11113.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 64 / 1000, 	// 11289.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 65 / 1000, 	// 11466.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 66 / 1000, 	// 11642.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 67 / 1000, 	// 11818.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 68 / 1000, 	// 11995.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 69 / 1000, 	// 12171.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 70 / 1000, 	// 12348.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 71 / 1000, 	// 12524.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 72 / 1000, 	// 12700.800 Kbytes/sec
	};

	if (uiCDSpeedNum > DRIVE_MAX_SPEED) {
		uiCDSpeedNum = 0;
	}
	cdb.SET_CD_SPEED.OperationCode = SCSIOP_SET_CD_SPEED;
	cdb.SET_CD_SPEED.ReadSpeed[0] = HIBYTE(wCDSpeedList[uiCDSpeedNum]);
	cdb.SET_CD_SPEED.ReadSpeed[1] = LOBYTE(wCDSpeedList[uiCDSpeedNum]);

	if (!ScsiPassThroughDirect(pDevData, &cdb.SET_CD_SPEED, CDB12GENERIC_LENGTH, 
		&setspeed, sizeof(CDROM_SET_SPEED), &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	OutputMmcDriveSpeed(&setspeed);
	return TRUE;
}

BOOL StartStopUnit(
	PDEVICE_DATA pDevData,
	BYTE Start,
	BYTE LoadEject
	)
{
	BYTE byScsiStatus = 0;
	CDB cdb = { 0 };

	cdb.START_STOP.OperationCode = SCSIOP_START_STOP_UNIT;
	cdb.START_STOP.Start = Start;
	cdb.START_STOP.LoadEject = LoadEject;

	if (!ScsiPassThroughDirect(pDevData, &cdb.START_STOP, CDB6GENERIC_LENGTH, 
		NULL, 0, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	return TRUE;
}

BOOL TestUnitReady(
	PDEVICE_DATA pDevData
	)
{
	BYTE byScsiStatus = 0;
	CDB cdb = { 0 };

	cdb.CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;

	if (!ScsiPassThroughDirect(pDevData, &cdb.CDB6GENERIC, CDB6GENERIC_LENGTH, 
		NULL, 0, &byScsiStatus, _T(__FUNCTION__), __LINE__) 
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	return TRUE;
}
