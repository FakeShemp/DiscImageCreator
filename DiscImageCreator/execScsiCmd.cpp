/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "check.h"
#include "convert.h"
#include "execIoctl.h"
#include "execScsiCmd.h"
#include "init.h"
#include "get.h"
#include "output.h"
#include "outputScsiCmdLog.h"
#include "outputScsiCmdLogforCD.h"
#include "set.h"

BOOL GetConfiguration(
	PDEVICE pDevice,
	PDISC pDisc
	)
{
	CDB::_GET_CONFIGURATION cdb = { 0 };
	cdb.OperationCode = SCSIOP_GET_CONFIGURATION;
	cdb.RequestType = SCSI_GET_CONFIGURATION_REQUEST_TYPE_CURRENT;
	cdb.StartingFeature[1] = FeatureProfileList;
	cdb.AllocationLength[0] = HIBYTE(sizeof(GET_CONFIGURATION_HEADER));
	cdb.AllocationLength[1] = LOBYTE(sizeof(GET_CONFIGURATION_HEADER));

	_declspec(align(4)) GET_CONFIGURATION_HEADER configHeader = { 0 };
	BYTE byScsiStatus = 0;
	OutputDriveLogA(
		"================================ Configuration ================================\n");
	if (!ScsiPassThroughDirect(pDevice, &cdb, CDB10GENERIC_LENGTH, &configHeader,
		sizeof(GET_CONFIGURATION_HEADER), &byScsiStatus, _T(__FUNCTION__), __LINE__) 
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		pDisc->SCSI.wCurrentMedia = ProfileCdrom;
		// not false. because undefined mmc1..
		OutputDriveLogA(
			"\tUndefined SCSIOP_GET_CONFIGURATION Command on this drive\n");
		return TRUE;
	}
	OutputDriveLogA("\tCurrentProfile: ");
	pDisc->SCSI.wCurrentMedia =
		MAKEWORD(configHeader.CurrentProfile[1], configHeader.CurrentProfile[0]);
	OutputFeatureProfileType(pDisc->SCSI.wCurrentMedia);
	OutputDriveLogA("\n");

	DWORD dwAllLen =
		MAKELONG(MAKEWORD(configHeader.DataLength[3], configHeader.DataLength[2]), 
			MAKEWORD(configHeader.DataLength[1], configHeader.DataLength[0])) -
			sizeof(configHeader.DataLength) + sizeof(GET_CONFIGURATION_HEADER);
	LPBYTE pPConf = NULL;
	LPBYTE lpConf = NULL;
	if (!GetAlignedAllocatedBuffer(pDevice, &pPConf,
		dwAllLen, &lpConf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
#if 0
	LPBYTE pPConf = (LPBYTE)calloc(dwAllLen + pDevice->AlignmentMask, sizeof(BYTE));
	if (!pPConf) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}

	LPBYTE lpConf = (LPBYTE)ConvParagraphBoundary(pDevice, pPConf);
#endif
	cdb.AllocationLength[0] = HIBYTE(dwAllLen);
	cdb.AllocationLength[1] = LOBYTE(dwAllLen);

	if (!ScsiPassThroughDirect(pDevice, &cdb, CDB10GENERIC_LENGTH, lpConf, 
		dwAllLen, &byScsiStatus, _T(__FUNCTION__), __LINE__) 
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		// not false. because undefined mmc1..
		OutputDriveLogA(
			"\tUndefined SCSIOP_GET_CONFIGURATION Command on this drive\n");
	}
	else {
		OutputFeatureNumber(pDevice,
			lpConf + sizeof(GET_CONFIGURATION_HEADER), dwAllLen - sizeof(GET_CONFIGURATION_HEADER));
	}
	FreeAndNull(pPConf);
	return TRUE;
}

BOOL Inquiry(
	PDEVICE pDevice
	)
{
	CDB::_CDB6INQUIRY3 cdb = { 0 };
	cdb.OperationCode = SCSIOP_INQUIRY;
	cdb.AllocationLength = sizeof(INQUIRYDATA);

	_declspec(align(4)) INQUIRYDATA inquiryData = { 0 };
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pDevice, &cdb, CDB6GENERIC_LENGTH, &inquiryData, 
		sizeof(INQUIRYDATA), &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	OutputInquiry(pDevice, &inquiryData);
	return TRUE;
}

BOOL ModeSense10(
	PDEVICE pDevice
	)
{
	CDB::_MODE_SENSE10 cdb = { 0 };
	cdb.OperationCode = SCSIOP_MODE_SENSE10;
#if (NTDDI_VERSION <= NTDDI_WIN7)
	cdb.LogicalUnitNumber = pDevice->address.Lun;
#endif
	cdb.PageCode = MODE_PAGE_CAPABILITIES;
	cdb.Pc = MODE_SENSE_CURRENT_VALUES;
	cdb.AllocationLength[0] = HIBYTE(sizeof(SENSE));
	cdb.AllocationLength[1] = LOBYTE(sizeof(SENSE));

	_declspec(align(4)) SENSE modesense = { 0 };
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pDevice, &cdb, CDB10GENERIC_LENGTH, &modesense,
		sizeof(SENSE),&byScsiStatus, _T(__FUNCTION__), __LINE__) ||
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		// not false. because undefined mmc1..
		OutputDriveLogA(
			"\tUndefined SCSIOP_MODE_SENSE10 Command on this drive\n");
		return TRUE;
	}
	OutputModeSense10(pDevice, &modesense);
	return TRUE;
}

BOOL ReadBufferCapacity(
	PDEVICE pDevice
	)
{
	CDB::_READ_BUFFER_CAPACITY cdb = { 0 };
	cdb.OperationCode = SCSIOP_READ_BUFFER_CAPACITY;
	cdb.AllocationLength[0] = HIBYTE(sizeof(READ_BUFFER_CAPACITY_DATA));
	cdb.AllocationLength[1] = LOBYTE(sizeof(READ_BUFFER_CAPACITY_DATA));

	_declspec(align(4)) READ_BUFFER_CAPACITY_DATA readBufCapaData = { 0 };
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH, &readBufCapaData, 
		sizeof(READ_BUFFER_CAPACITY_DATA), &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	OutputBufferCapacity(&readBufCapaData);
	return TRUE;
}

BOOL ReadDiscInformation(
	PDEVICE pDevice
	)
{
	CDB::_READ_DISK_INFORMATION cdb = { 0 };
	cdb.OperationCode = SCSIOP_READ_DISC_INFORMATION;
	cdb.Lun = pDevice->address.Lun;
	cdb.AllocationLength[0] = HIBYTE(sizeof(DISC_INFORMATION));
	cdb.AllocationLength[1] = LOBYTE(sizeof(DISC_INFORMATION));

	_declspec(align(4)) DISC_INFORMATION discInformation = { 0 };
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pDevice, &cdb, CDB10GENERIC_LENGTH, &discInformation,
		sizeof(DISC_INFORMATION), &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	OutputDiscInformation(&discInformation);
	return TRUE;
}

BOOL ReadTOC(
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	PDISC pDisc
	)
{
	CDB::_READ_TOC cdb = { 0 };
	cdb.OperationCode = SCSIOP_READ_TOC;
	cdb.LogicalUnitNumber = pDevice->address.Lun;
	cdb.Format2 = CDROM_READ_TOC_EX_FORMAT_TOC;
	cdb.StartingTrack = 1;
	cdb.AllocationLength[0] = HIBYTE(CDROM_TOC_SIZE);
	cdb.AllocationLength[1] = LOBYTE(CDROM_TOC_SIZE);

#ifdef _DEBUG
	OutputString(_T("pDisc->SCSI.toc address: %p\n"), &pDisc->SCSI.toc);
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pDevice, &cdb, CDB10GENERIC_LENGTH, &pDisc->SCSI.toc,
		CDROM_TOC_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__) || 
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		if (pDisc->SCSI.wCurrentMedia != ProfileDvdRom) {
			return FALSE;
		}
	}
	if (!InitLBAPerTrack(pExecType, &pDisc)) {
		return FALSE;
	}
	if (byScsiStatus == SCSISTAT_GOOD) {
		pDevice->bSuccessReadToc = TRUE;
		SetAndOutputToc(pExecType, pDisc);
	}
	return TRUE;
}

BOOL ReadTOCFull(
	PDEVICE pDevice,
	PDISC pDisc,
	FILE* fpCcd
	)
{
	CDB::_READ_TOC cdb = { 0 };
	cdb.OperationCode = SCSIOP_READ_TOC;
	cdb.LogicalUnitNumber = pDevice->address.Lun;
	cdb.Format2 = CDROM_READ_TOC_EX_FORMAT_FULL_TOC;
	cdb.StartingTrack = 1;
	cdb.AllocationLength[0] = HIBYTE(sizeof(CDROM_TOC_FULL_TOC_DATA));
	cdb.AllocationLength[1] = LOBYTE(sizeof(CDROM_TOC_FULL_TOC_DATA));

	_declspec(align(4)) CDROM_TOC_FULL_TOC_DATA fullToc = { 0 };
#ifdef _DEBUG
	OutputString(_T("fullToc address: %p\n"), &fullToc);
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pDevice, &cdb, CDB10GENERIC_LENGTH, &fullToc,
		sizeof(CDROM_TOC_FULL_TOC_DATA), &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	WORD wFullTocLen = MAKEWORD(fullToc.Length[1], fullToc.Length[0]);
	WORD wTocEntriesAll = wFullTocLen - sizeof(fullToc.Length);
	WORD wTocEntries = wTocEntriesAll / sizeof(CDROM_TOC_FULL_TOC_DATA_BLOCK);

	if (fpCcd) {
		WriteCcdFileForDisc(wTocEntries, fullToc.LastCompleteSession, fpCcd);
		if (pDevice->bCanCDText) {
			ReadTOCText(pDevice, pDisc, fpCcd);
		}
	}

	WORD wFullTocLenFix = wTocEntriesAll + sizeof(CDROM_TOC_FULL_TOC_DATA);
	// 4 byte padding
	if (wFullTocLenFix % 4) {
		wFullTocLenFix = (WORD)((wFullTocLenFix / 4 + 1) * 4);
	}
#ifdef _DEBUG
	OutputDiscLogA(
		"FullTocLen: %u, TocEntriesAll: %u, TocEntries: %u, FullTocLenFix: %u\n",
		wFullTocLen, wTocEntriesAll, wTocEntries, wFullTocLenFix);
#endif
	LPBYTE pPFullToc = NULL;
	LPBYTE pFullToc = NULL;
	if (!GetAlignedAllocatedBuffer(pDevice, &pPFullToc,
		wFullTocLenFix, &pFullToc, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
#if 0
	LPBYTE pPFullToc =
		(LPBYTE)calloc(wFullTocLenFix + pDevice->AlignmentMask, sizeof(BYTE));
	if (!pPFullToc) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	LPBYTE pFullToc = (LPBYTE)ConvParagraphBoundary(pDevice, pPFullToc);
#endif
#ifdef _DEBUG
	OutputString(_T("pPFullToc address: %p\n"), &pPFullToc);
	OutputString(_T("pFullToc address: %p\n"), &pFullToc);
#endif
	cdb.AllocationLength[0] = HIBYTE(wFullTocLenFix);
	cdb.AllocationLength[1] = LOBYTE(wFullTocLenFix);

	BOOL bRet = TRUE;
	LPBYTE aBuf2 = NULL;
	try {
		if (!ScsiPassThroughDirect(pDevice, &cdb, CDB10GENERIC_LENGTH, pFullToc, 
			wFullTocLenFix, &byScsiStatus, _T(__FUNCTION__), __LINE__) 
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw FALSE;
		}
		PCDROM_TOC_FULL_TOC_DATA_BLOCK pTocData = 
			((PCDROM_TOC_FULL_TOC_DATA)pFullToc)->Descriptors;
		BYTE bySession = 0;
		// session1, session2
		INT aLBA[] = { 0, pDisc->SCSI.nFirstLBAofDataTrack };
		LPBYTE aBuf2 = NULL;
		LPBYTE lpBuf = NULL;
		if (!GetAlignedAllocatedBuffer(pDevice, &aBuf2,
			CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &lpBuf, _T(__FUNCTION__), __LINE__)) {
			return FALSE;
		}
#if 0
		aBuf2 = (LPBYTE)calloc(
			CD_RAW_SECTOR_WITH_SUBCODE_SIZE + pDevice->AlignmentMask, sizeof(BYTE));
		if (!aBuf2) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevice, aBuf2);
#endif
		CDB::_READ_CD cdb = { 0 };
		SetReadCDCommand(NULL, pDevice, &cdb, READ_CD_FLAG::All, 1, READ_CD_FLAG::Raw, TRUE);

		for (WORD b = 0; b < wTocEntries; b++) {
			if (pTocData[b].Point < 100 && bySession < pTocData[b].SessionNumber) {
				BYTE byMode = DATA_BLOCK_MODE0;
				// check 2 MCN, ISRC frame
				INT nTmpMCNLBA = -1;
				INT nTmpISRCLBA = -1;
				CHAR szTmpCatalog[META_CATALOG_SIZE] = { 0 };
				for (INT nLBA = aLBA[bySession]; nLBA < aLBA[bySession] + 200; nLBA++) {
					cdb.StartingLBA[0] = HIBYTE(HIWORD(nLBA));
					cdb.StartingLBA[1] = LOBYTE(HIWORD(nLBA));
					cdb.StartingLBA[2] = HIBYTE(LOWORD(nLBA));
					cdb.StartingLBA[3] = LOBYTE(LOWORD(nLBA));
					if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
						CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
						|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
						throw FALSE;
					}
					BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
					AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
					if (nLBA == aLBA[bySession]) {
						BYTE byCtl = (BYTE)((lpSubcode[12] >> 4) & 0x0f);
						byMode = GetMode(lpBuf, byCtl);
					}
					if (pTocData[b].SessionNumber == 1) {
						BYTE byAdr = (BYTE)(lpSubcode[12] & 0x0f);
						if (byAdr == ADR_ENCODES_MEDIA_CATALOG) {
							BOOL bMCN = IsValidSubQMCN(lpSubcode);
							if (bMCN) {
								CHAR szCatalog[META_CATALOG_SIZE] = { 0 };
								if (nTmpMCNLBA == -1) {
									SetMCNToString(pDisc, lpSubcode, szCatalog, FALSE);
									strncpy(szTmpCatalog, szCatalog, META_CATALOG_SIZE);
									szTmpCatalog[META_CATALOG_SIZE - 1] = 0;
									nTmpMCNLBA = nLBA;
								}
								else if (!pDisc->SUB.bCatalog) {
									SetMCNToString(pDisc, lpSubcode, szCatalog, FALSE);
									if (!strncmp(szTmpCatalog, szCatalog, META_CATALOG_SIZE)) {
										strncpy(pDisc->SUB.szCatalog, szCatalog, META_CATALOG_SIZE);
										pDisc->SUB.bCatalog = bMCN;
										pDisc->SUB.nFirstLBAForMCN = nTmpMCNLBA;
										pDisc->SUB.nRangeLBAForMCN = nLBA - nTmpMCNLBA;
										WriteCcdFileForDiscCatalog(pDisc, fpCcd);
										OutputDiscLogA(
											"First MCN Sector: %d, Range of MCN: %d frame\n"
											, nTmpMCNLBA, pDisc->SUB.nRangeLBAForMCN);
									}
								}
							}
						}
						else if (byAdr == ADR_ENCODES_ISRC) {
							BOOL bISRC = IsValidSubQISRC(lpSubcode);
							if (bISRC) {
								if (nTmpISRCLBA == -1) {
									nTmpISRCLBA = nLBA;
								}
								else {
									pDisc->SUB.bISRC = bISRC;
									pDisc->SUB.nFirstLBAForISRC = nTmpISRCLBA;
									pDisc->SUB.nRangeLBAForISRC = nLBA - nTmpISRCLBA;
									OutputDiscLogA(
										"First ISRC Sector: %d, Range of ISRC: %d frame\n"
										, nTmpISRCLBA, pDisc->SUB.nRangeLBAForISRC);
								}
							}
						}
					}
					else {
						break;
					}
				}
				WriteCcdFileForSession(pTocData[b].SessionNumber, byMode, fpCcd);
				bySession++;
			}
		}
		SetAndOutputTocFull(pDisc, &fullToc, pTocData, wTocEntries, fpCcd);
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FreeAndNull(pPFullToc);
	FreeAndNull(aBuf2);
	return bRet;
}

BOOL ReadTOCText(
	PDEVICE pDevice,
	PDISC pDisc,
	FILE* fpCcd
	)
{
	CDB::_READ_TOC cdb = { 0 };
	cdb.OperationCode = SCSIOP_READ_TOC;
	cdb.LogicalUnitNumber = pDevice->address.Lun;
	cdb.Format2 = CDROM_READ_TOC_EX_FORMAT_CDTEXT;
	cdb.AllocationLength[0] = HIBYTE(sizeof(CDROM_TOC_CD_TEXT_DATA));
	cdb.AllocationLength[1] = LOBYTE(sizeof(CDROM_TOC_CD_TEXT_DATA));

	_declspec(align(4)) CDROM_TOC_CD_TEXT_DATA tocText = { 0 };
#ifdef _DEBUG
	OutputString(_T("tocText address: %p\n"), &tocText);
#endif
	OutputDiscLogA(
		"========================== CDTEXT on SCSIOP_READ_TOC ==========================\n");
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pDevice, &cdb, CDB10GENERIC_LENGTH, &tocText,
		sizeof(CDROM_TOC_CD_TEXT_DATA), &byScsiStatus, _T(__FUNCTION__), __LINE__) 
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		// not false. because undefined mmc1..
		OutputErrorString(
			_T("Nothing CDTEXT or Undefined CDROM_READ_TOC_EX_FORMAT_CDTEXT on this drive\n"));
		return TRUE;
	}
	WORD wTocTextLen = MAKEWORD(tocText.Length[1], tocText.Length[0]);
	WORD wTocTextEntriesAll = wTocTextLen - sizeof(tocText.Length);

	WriteCcdFileForDiscCDTextLength(wTocTextEntriesAll, fpCcd);
	if (!wTocTextEntriesAll) {
		OutputDiscLogA("\tNothing\n");
		// many CD is nothing text
		return TRUE;
	}

	WORD wTocTextEntries = wTocTextEntriesAll / sizeof(CDROM_TOC_CD_TEXT_DATA_BLOCK);
	WriteCcdFileForCDText(wTocTextEntries, fpCcd);

	WORD wTocTextLenFix = wTocTextEntriesAll + sizeof(CDROM_TOC_CD_TEXT_DATA);
	// 4 byte padding
	if (wTocTextLenFix % 4) {
		wTocTextLenFix = (WORD)((wTocTextLenFix / 4 + 1) * 4);
	}
#ifdef _DEBUG
	OutputDiscLogA(
		"TocTextLen: %u, TocTextEntriesAll: %u, TocTextEntries: %u, TocTextLenFix: %u\n",
		wTocTextLen, wTocTextEntriesAll, wTocTextEntries, wTocTextLenFix);
#endif
	LPBYTE pPTocText = NULL;
	LPBYTE pTocText = NULL;
	if (!GetAlignedAllocatedBuffer(pDevice, &pPTocText,
		wTocTextLenFix, &pTocText, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
#if 0
	LPBYTE pPTocText = 
		(LPBYTE)calloc(wTocTextLenFix + pDevice->AlignmentMask, sizeof(BYTE));
	if(!pPTocText) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	LPBYTE pTocText = (LPBYTE)ConvParagraphBoundary(pDevice, pPTocText);
#endif
#ifdef _DEBUG
	OutputString(_T("pPTocText address: %p\n"), &pPTocText);
	OutputString(_T("pTocText address: %p\n"), &pTocText);
#endif
	cdb.AllocationLength[0] = HIBYTE(wTocTextLenFix);
	cdb.AllocationLength[1] = LOBYTE(wTocTextLenFix);

	PCHAR pTmpText = NULL;
	BOOL bRet = TRUE;
	try {
		if (!ScsiPassThroughDirect(pDevice, &cdb, CDB10GENERIC_LENGTH, pTocText, 
			wTocTextLenFix, &byScsiStatus, _T(__FUNCTION__), __LINE__) 
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw FALSE;
		}
		PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc = 
			((PCDROM_TOC_CD_TEXT_DATA)pTocText)->Descriptors;
		WriteCcdFileForCDTextEntry(pDesc, wTocTextEntries, fpCcd);

		WORD wAllTextSize = wTocTextEntries * sizeof(pDesc->Text);
		if (NULL == (pTmpText = (PCHAR)calloc(wAllTextSize, sizeof(_TCHAR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		WORD wEntrySize = 0;
		BOOL bUnicode = FALSE;
		while (wEntrySize < wTocTextEntries) {
			if (pDesc[wEntrySize].Unicode == 1) {
				bUnicode = TRUE;
				break;
			}
			wEntrySize++;
		}
		SetAndOutputTocCDText(pDisc, pDesc, pTmpText, wEntrySize, wAllTextSize);
		if (bUnicode) {
			PWCHAR pTmpWText = NULL;
			if (NULL == (pTmpWText = (PWCHAR)calloc(wAllTextSize, sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			SetAndOutputTocCDWText(pDesc, 
				pTmpText, wEntrySize, wTocTextEntries, wAllTextSize);
			FreeAndNull(pTmpWText);
		}
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FreeAndNull(pPTocText);
	FreeAndNull(pTmpText);
	return bRet;
}

BOOL SetCDSpeed(
	PDEVICE pDevice,
	DWORD dwCDSpeedNum
	)
{
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

	if (dwCDSpeedNum > DRIVE_MAX_SPEED) {
		dwCDSpeedNum = 0;
	}

	CDB::_SET_CD_SPEED cdb = { 0 };
	cdb.OperationCode = SCSIOP_SET_CD_SPEED;
	cdb.ReadSpeed[0] = HIBYTE(wCDSpeedList[dwCDSpeedNum]);
	cdb.ReadSpeed[1] = LOBYTE(wCDSpeedList[dwCDSpeedNum]);

	_declspec(align(4)) CDROM_SET_SPEED setspeed;
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH, &setspeed, 
		sizeof(CDROM_SET_SPEED), &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	OutputDriveSpeed(&setspeed);
	return TRUE;
}

BOOL StartStopUnit(
	PDEVICE pDevice,
	BYTE Start,
	BYTE LoadEject
	)
{
	CDB::_START_STOP cdb = { 0 };
	cdb.OperationCode = SCSIOP_START_STOP_UNIT;
	cdb.LogicalUnitNumber = pDevice->address.Lun;
	cdb.Start = Start;
	cdb.LoadEject = LoadEject;

	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pDevice, &cdb, CDB6GENERIC_LENGTH,
		NULL, 0, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	return TRUE;
}

BOOL TestUnitReady(
	PDEVICE pDevice
	)
{
	CDB::_CDB6GENERIC cdb = { 0 };
	cdb.OperationCode = SCSIOP_TEST_UNIT_READY;
	cdb.LogicalUnitNumber = pDevice->address.Lun;

	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pDevice, &cdb, CDB6GENERIC_LENGTH,
		NULL, 0, &byScsiStatus, _T(__FUNCTION__), __LINE__) 
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	return TRUE;
}

// feature Plextor drive below
BOOL Reset(
	PDEVICE pDevice
	)
{
	CDB::_CDB6GENERIC cdb = { 0 };
	cdb.OperationCode = SCSIOP_PLEX_RESET;

	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pDevice, &cdb, CDB6GENERIC_LENGTH,
		NULL, 0, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	return TRUE;
}

BOOL ReadEeprom(
	PDEVICE pDevice
	)
{
	DWORD tLen = 128;
	BOOL bHigh = FALSE;
	INT nLife = 0;
	switch (pDevice->byPlexType) {
	case PLEX_DRIVE_TYPE::PXW5224:
		tLen = 160;
		break;
	case PLEX_DRIVE_TYPE::PXW5232:
	case PLEX_DRIVE_TYPE::Premium2:
	case PLEX_DRIVE_TYPE::PX708:
	case PLEX_DRIVE_TYPE::PX708A2:
		tLen = 256;
		break;
	case PLEX_DRIVE_TYPE::PX712:
		tLen = 512;
		nLife = 1;
		break;
	case PLEX_DRIVE_TYPE::PX716:
	case PLEX_DRIVE_TYPE::PX716AL:
	case PLEX_DRIVE_TYPE::PX755:
	case PLEX_DRIVE_TYPE::PX760:
		tLen = 256;
		nLife = 2;
		bHigh = TRUE;
		break;
	}
	DWORD BufLen = tLen;
	LPBYTE pPBuf = NULL;
	LPBYTE pBuf = NULL;
	if (!GetAlignedAllocatedBuffer(pDevice, &pPBuf,
		BufLen, &pBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
#if 0
	LPBYTE pPBuf =
		(LPBYTE)calloc(tLen + pDevice->AlignmentMask, sizeof(BYTE));
	if (!pPBuf) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	LPBYTE pBuf = (LPBYTE)ConvParagraphBoundary(pDevice, pPBuf);
#endif
	CDB::_CDB12 cdb = { 0 };
	cdb.OperationCode = SCSIOP_PLEX_READ_EEPROM;
	cdb.RelativeAddress = (BYTE)bHigh;

	BYTE byScsiStatus = 0;
	OutputDriveLogA(
		"=================================== EEPROM ==================================\n");
	for (BYTE idx = 0; idx < 4; idx++) {
		cdb.TransferLength[0] = HIBYTE(HIWORD(tLen));
		cdb.TransferLength[1] = LOBYTE(HIWORD(tLen));
		cdb.TransferLength[2] = HIBYTE(LOWORD(tLen));
		cdb.TransferLength[3] = LOBYTE(LOWORD(tLen));
		if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH,
			pBuf, BufLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			return FALSE;
		}
		OutputEeprom(pBuf, tLen, idx, nLife);
		if (bHigh) {
			tLen += 0x10000;
		}
		else {
			break;
		}
	}
	FreeAndNull(pPBuf);
	return TRUE;
}

BOOL SetSpeedRead(
	PDEVICE pDevice,
	BOOL bState
	)
{
	// PX-708, PXW4012 or older doesn't support SpeedRead
	if (pDevice->byPlexType == PLEX_DRIVE_TYPE::PX760 ||
		pDevice->byPlexType == PLEX_DRIVE_TYPE::PX755 ||
		pDevice->byPlexType == PLEX_DRIVE_TYPE::PX716 ||
		pDevice->byPlexType == PLEX_DRIVE_TYPE::PX716AL ||
		pDevice->byPlexType == PLEX_DRIVE_TYPE::PX712 ||
		pDevice->byPlexType == PLEX_DRIVE_TYPE::Premium2 ||
		pDevice->byPlexType == PLEX_DRIVE_TYPE::PXW5232 ||
		pDevice->byPlexType == PLEX_DRIVE_TYPE::PXW5224 ||
		pDevice->byPlexType == PLEX_DRIVE_TYPE::PXW4824
		) {
		CONST WORD size = 8;
		BYTE buf[size] = { 0 };

		CDB::_CDB12 cdb = { 0 };
		cdb.OperationCode = SCSIOP_PLEX_EXTEND;
		cdb.DisablePageOut = TRUE;
		cdb.LogicalBlock[0] = PLEX_FLAG_SPEED_READ;
		cdb.LogicalBlock[1] = (BYTE)bState;
		cdb.Reserved2 = 0x08;

		BYTE byScsiStatus = 0;
		if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH,
			buf, sizeof(buf), &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			return FALSE;
		}
#if 0
		for (INT i = 0; i < size; i++) {
			OutputString(_T("%02x "), buf[i]);
		}
		OutputString(_T("\n");
#endif
	}
	return TRUE;
}
