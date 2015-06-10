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

BOOL ModeSense(
	PDEVICE pDevice
	)
{
	if (pDevice->FEATURE.byModePage2a) {
		CDB::_MODE_SENSE cdb = { 0 };
		cdb.OperationCode = SCSIOP_MODE_SENSE;
#if (NTDDI_VERSION <= NTDDI_WIN7)
		cdb.LogicalUnitNumber = pDevice->address.Lun;
#endif
		cdb.PageCode = MODE_PAGE_CAPABILITIES;
		cdb.Pc = MODE_SENSE_CURRENT_VALUES;
		cdb.AllocationLength = sizeof(CDVD_CAPABILITIES_PAGE_WITH_HEADER);

		_declspec(align(4)) CDVD_CAPABILITIES_PAGE_WITH_HEADER modesense = { 0 };
		BYTE byScsiStatus = 0;
		if (!ScsiPassThroughDirect(pDevice, &cdb, CDB6GENERIC_LENGTH, &modesense,
			sizeof(CDVD_CAPABILITIES_PAGE_WITH_HEADER), &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
			byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			// not false.
		}
		else {
			if (modesense.cdvd.PageCode == MODE_PAGE_CAPABILITIES) {
				OutputModeParmeterHeader(&modesense.header);
				OutputCDVDCapabilitiesPage(pDevice, &modesense.cdvd);
			}
			else {
				if (pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PXW1210S) {
					pDevice->wDriveBufSize = 4096;
				}
				OutputDriveLogA(
					"SCSIOP_MODE_SENSE didn't fail. But it couldn't get PageCode on this drive\n"
					"\tSet drive buffer size: %u\n"
					, pDevice->wDriveBufSize);
			}
		}
	}
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
		pDevice->bySuccessReadToc = TRUE;
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
		if (pDevice->FEATURE.byCanCDText) {
			ReadTOCText(pDevice, pDisc, fpCcd);
		}
	}

	WORD wFullTocLenFix = wTocEntriesAll + sizeof(CDROM_TOC_FULL_TOC_DATA);
	// 4 byte padding
	if (wFullTocLenFix % 4) {
		wFullTocLenFix = (WORD)((wFullTocLenFix / 4 + 1) * 4);
	}
	LPBYTE pPFullToc = NULL;
	LPBYTE pFullToc = NULL;
	if (!GetAlignedAllocatedBuffer(pDevice, &pPFullToc,
		wFullTocLenFix, &pFullToc, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
#ifdef _DEBUG
	OutputDiscLogA(
		"FullTocLen: %u, TocEntriesAll: %u, TocEntries: %u, FullTocLenFix: %u\n",
		wFullTocLen, wTocEntriesAll, wTocEntries, wFullTocLenFix);
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
		LPBYTE lpBuf = NULL;
		if (!GetAlignedAllocatedBuffer(pDevice, &aBuf2,
			CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &lpBuf, _T(__FUNCTION__), __LINE__)) {
			throw FALSE;
		}
		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		INT nOfs = 0;
		if (pDevice->byPlxtrType) {
			CDB::_PLXTR_READ_CDDA cdb = { 0 };
			SetReadD8Command(pDevice, &cdb, 1, PLXTR_READ_CDDA_FLAG::MainPack);
			memcpy(lpCmd, &cdb, sizeof(lpCmd));
			nOfs = pDisc->MAIN.nCombinedOffset % CD_RAW_SECTOR_SIZE;
			if (pDisc->MAIN.nCombinedOffset < 0) {
				nOfs = CD_RAW_SECTOR_SIZE + nOfs;
			}
		}
		else {
			CDB::_READ_CD cdb = { 0 };
			SetReadCDCommand(NULL, pDevice, &cdb,
				READ_CD_FLAG::All, 1, READ_CD_FLAG::Raw, TRUE);
			memcpy(lpCmd, &cdb, sizeof(lpCmd));
		}

		for (WORD b = 0; b < wTocEntries; b++) {
			if (pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PX40TS) {
				// Somehow Ultraplex seems to get the fulltoc data as "hexadecimal"
				pTocData[b].Msf[0] = BcdToDec(pTocData[b].Msf[0]);
				pTocData[b].Msf[1] = BcdToDec(pTocData[b].Msf[1]);
				pTocData[b].Msf[2] = BcdToDec(pTocData[b].Msf[2]);
				pTocData[b].MsfExtra[0] = BcdToDec(pTocData[b].MsfExtra[0]);
				pTocData[b].MsfExtra[1] = BcdToDec(pTocData[b].MsfExtra[1]);
				pTocData[b].MsfExtra[2] = BcdToDec(pTocData[b].MsfExtra[2]);
				if (pTocData[b].Point < 0xa0) {
					pTocData[b].Point = BcdToDec(pTocData[b].Point);
				}
			}
			if (pTocData[b].Point < 100 && bySession < pTocData[b].SessionNumber) {
				BYTE byMode = DATA_BLOCK_MODE0;
				// check 2 MCN, ISRC frame
				INT nTmpMCNLBA = -1;
				INT nTmpISRCLBA = -1;
				BOOL bCheckMCN = FALSE;
				BOOL bCheckISRC = FALSE;
				CHAR szTmpCatalog[META_CATALOG_SIZE] = { 0 };
				for (INT nLBA = aLBA[bySession]; nLBA < aLBA[bySession] + 200; nLBA++) {
					lpCmd[2] = HIBYTE(HIWORD(nLBA));
					lpCmd[3] = LOBYTE(HIWORD(nLBA));
					lpCmd[4] = HIBYTE(LOWORD(nLBA));
					lpCmd[5] = LOBYTE(LOWORD(nLBA));
					if (!ScsiPassThroughDirect(pDevice, lpCmd, CDB12GENERIC_LENGTH, lpBuf,
						CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
						|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
						throw FALSE;
					}
					LPBYTE lpBuf2 = lpBuf;
					if (pDevice->byPlxtrType) {
						lpBuf2 = lpBuf + nOfs;
					}
					BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
					AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
#if 0
					OutputCDMain(lpBuf2, nLBA, CD_RAW_SECTOR_SIZE);
					OutputCDSub96Align(lpSubcode, nLBA);
#endif
					if (nLBA == aLBA[bySession]) {
						BYTE byCtl = (BYTE)((lpSubcode[12] >> 4) & 0x0f);
						byMode = GetMode(lpBuf2, byCtl);
					}
					if (pTocData[b].SessionNumber == 1) {
						BYTE byAdr = (BYTE)(lpSubcode[12] & 0x0f);
						if (byAdr == ADR_ENCODES_MEDIA_CATALOG) {
							BYTE byMCN = IsValidSubQMCN(lpSubcode);
							if (byMCN) {
								CHAR szCatalog[META_CATALOG_SIZE] = { 0 };
								if (nTmpMCNLBA == -1) {
									SetMCNToString(pDisc, lpSubcode, szCatalog, FALSE);
									strncpy(szTmpCatalog, szCatalog, META_CATALOG_SIZE);
									szTmpCatalog[META_CATALOG_SIZE - 1] = 0;
									nTmpMCNLBA = nLBA;
								}
								else if (!pDisc->SUB.byCatalog && !bCheckMCN) {
									SetMCNToString(pDisc, lpSubcode, szCatalog, FALSE);
									if (!strncmp(szTmpCatalog, szCatalog, META_CATALOG_SIZE)) {
										strncpy(pDisc->SUB.szCatalog, szCatalog, META_CATALOG_SIZE);
										pDisc->SUB.byCatalog = byMCN;
										pDisc->SUB.nFirstLBAForMCN = nTmpMCNLBA;
										if (pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PXS88T) {
											// Somehow PX-S88T is sliding subchannel +1;
											pDisc->SUB.nFirstLBAForMCN++;
										}
										pDisc->SUB.nRangeLBAForMCN = nLBA - nTmpMCNLBA;
										WriteCcdFileForDiscCatalog(pDisc, fpCcd);
										OutputDiscLogA(
											"First MCN sector: %d, MCN sector exists per: %d frame\n"
											, pDisc->SUB.nFirstLBAForMCN, pDisc->SUB.nRangeLBAForMCN);
										bCheckMCN = TRUE;
									}
								}
							}
						}
						else if (byAdr == ADR_ENCODES_ISRC) {
							BYTE byISRC = IsValidSubQISRC(lpSubcode);
							if (byISRC) {
								if (nTmpISRCLBA == -1) {
									nTmpISRCLBA = nLBA;
								}
								else if (!bCheckISRC) {
									pDisc->SUB.byISRC = byISRC;
									pDisc->SUB.nFirstLBAForISRC = nTmpISRCLBA;
									if (pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PXS88T) {
										// Somehow PX-S88T is sliding subchannel +1;
										pDisc->SUB.nFirstLBAForISRC++;
									}
									pDisc->SUB.nRangeLBAForISRC = nLBA - nTmpISRCLBA;
									OutputDiscLogA(
										"First ISRC sector: %d, ISRC sector exists per: %d frame\n"
										, pDisc->SUB.nFirstLBAForISRC, pDisc->SUB.nRangeLBAForISRC);
									bCheckISRC = TRUE;
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
		OutputDiscLogA(
			"Nothing CDTEXT or Undefined CDROM_READ_TOC_EX_FORMAT_CDTEXT on this drive\n");
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
	LPBYTE pPTocText = NULL;
	LPBYTE pTocText = NULL;
	if (!GetAlignedAllocatedBuffer(pDevice, &pPTocText,
		wTocTextLenFix, &pTocText, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
#ifdef _DEBUG
	OutputDiscLogA(
		"TocTextLen: %u, TocTextEntriesAll: %u, TocTextEntries: %u, TocTextLenFix: %u\n",
		wTocTextLen, wTocTextEntriesAll, wTocTextEntries, wTocTextLenFix);
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
	if (!ScsiPassThroughDirect(pDevice, &cdb, CDB10GENERIC_LENGTH, &configHeader,
		sizeof(GET_CONFIGURATION_HEADER), &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		pDisc->SCSI.wCurrentMedia = ProfileCdrom;
		// not false. because undefined mmc1..
		if (pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PX40TS) {
			pDevice->FEATURE.byCanCDText = TRUE;
			pDevice->FEATURE.byC2ErrorData = TRUE;
		}
		OutputDriveLogA(
			"Undefined SCSIOP_GET_CONFIGURATION Command on this drive\n");
	}
	else {
		pDisc->SCSI.wCurrentMedia =
			MAKEWORD(configHeader.CurrentProfile[1], configHeader.CurrentProfile[0]);
		if (pDisc->SCSI.wCurrentMedia == ProfileInvalid) {
			OutputDriveLogA(
				"SCSIOP_GET_CONFIGURATION didn't fail. But it couldn't get CurrentMedia on this drive\n"
				"\tSet CurrentMedia to CD-ROM\n");
			configHeader.CurrentProfile[1] = ProfileCdrom;
			pDisc->SCSI.wCurrentMedia = ProfileCdrom;
		}
		OutputGetConfigurationHeader(&configHeader);

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
		cdb.AllocationLength[0] = HIBYTE(dwAllLen);
		cdb.AllocationLength[1] = LOBYTE(dwAllLen);

		if (!ScsiPassThroughDirect(pDevice, &cdb, CDB10GENERIC_LENGTH, lpConf,
			dwAllLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			// not false. because undefined mmc1..
			OutputDriveLogA(
				"Undefined SCSIOP_GET_CONFIGURATION Command on this drive\n");
		}
		else {
			OutputGetConfigurationFeatureNumber(pDevice,
				lpConf + sizeof(GET_CONFIGURATION_HEADER), dwAllLen - sizeof(GET_CONFIGURATION_HEADER));
			if (pDevice->byPlxtrType == (BYTE)PLXTR_DRIVE_TYPE::PXW1210A ||
				pDevice->byPlxtrType == (BYTE)PLXTR_DRIVE_TYPE::PXW1210S) {
				// Somehow SetCDSpeed fails in PX-W1210...
				pDevice->FEATURE.bySetCDSpeed = FALSE;
			}
			if (pDevice->byPlxtrType == (BYTE)PLXTR_DRIVE_TYPE::PXS88T ||
				pDevice->byPlxtrType == (BYTE)PLXTR_DRIVE_TYPE::PXW1210A) {
				// If use PLXTR_READ_CDDA(0xd8) and subch(0x08), it fails.
				pDevice->FEATURE.byC2ErrorData = FALSE;
			}
		}
		FreeAndNull(pPConf);
	}
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
		// not false.
		OutputDiscLogA(
			"Undefined SCSIOP_READ_DISC_INFORMATION Command on this drive\n");
	}
	else {
		OutputDiscInformation(&discInformation);
	}
	return TRUE;
}

BOOL ModeSense10(
	PDEVICE pDevice
	)
{
	if (pDevice->FEATURE.byModePage2a) {
		CDB::_MODE_SENSE10 cdb = { 0 };
		cdb.OperationCode = SCSIOP_MODE_SENSE10;
#if (NTDDI_VERSION <= NTDDI_WIN7)
		cdb.LogicalUnitNumber = pDevice->address.Lun;
#endif
		cdb.PageCode = MODE_PAGE_CAPABILITIES;
		cdb.Pc = MODE_SENSE_CURRENT_VALUES;
		cdb.AllocationLength[0] = HIBYTE(sizeof(CDVD_CAPABILITIES_PAGE_WITH_HEADER10));
		cdb.AllocationLength[1] = LOBYTE(sizeof(CDVD_CAPABILITIES_PAGE_WITH_HEADER10));

		_declspec(align(4)) CDVD_CAPABILITIES_PAGE_WITH_HEADER10 modesense = { 0 };
		BYTE byScsiStatus = 0;
		if (!ScsiPassThroughDirect(pDevice, &cdb, CDB10GENERIC_LENGTH, &modesense,
			sizeof(CDVD_CAPABILITIES_PAGE_WITH_HEADER10), &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
			byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			// not false.
			OutputDriveLogA(
				"Undefined SCSIOP_MODE_SENSE10 Command on this drive\n");
			ModeSense(pDevice);
		}
		else {
			if (modesense.cdvd.PageCode == MODE_PAGE_CAPABILITIES) {
				OutputModeParmeterHeader10(&modesense.header);
				OutputCDVDCapabilitiesPage(pDevice, &modesense.cdvd);
			}
			else {
				OutputDriveLogA(
					"SCSIOP_MODE_SENSE10 didn't fail. But it couldn't get PageCode on this drive\n");
				ModeSense(pDevice);
			}
		}
	}
	else {
		if (pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PX40TS) {
			pDevice->wDriveBufSize = 512;
		}
		else if (pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PXS88T ||
			pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PX320A) {
			pDevice->wDriveBufSize = 2048;
			pDevice->FEATURE.bySetCDSpeed = TRUE;
			pDevice->wMaxReadSpeed = CD_RAW_SECTOR_SIZE * 75 * 52 / 1000;
		}
		else if (pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PXW8220T) {
			pDevice->wDriveBufSize = 4096;
		}
		else if (
			!strncmp(pDevice->szProductId, "CD-ROM  TS-H192C", DRIVE_PRODUCT_ID_SIZE) ||
			!strncmp(pDevice->szProductId, "CD-ROM TS-H192CN", DRIVE_PRODUCT_ID_SIZE)
			) {
			pDevice->FEATURE.bySetCDSpeed = TRUE;
			pDevice->wMaxReadSpeed = CD_RAW_SECTOR_SIZE * 75 * 52 / 1000;
		}
		OutputDriveLogA(
			"Doesn't support SCSIOP_MODE_SENSE10 Command on this drive\n"
			"\tSet CD Speed: %s\n"
			"\tSet drive buffer size: %u\n"
			, BOOLEAN_TO_STRING_TRUE_FALSE_A(pDevice->FEATURE.bySetCDSpeed)
			, pDevice->wDriveBufSize);
	}
	return TRUE;
}

BOOL ReadBufferCapacity(
	PDEVICE pDevice
	)
{
	if (pDevice->FEATURE.byReadBufCapa) {
		CDB::_READ_BUFFER_CAPACITY cdb = { 0 };
		cdb.OperationCode = SCSIOP_READ_BUFFER_CAPACITY;
		cdb.AllocationLength[0] = HIBYTE(sizeof(READ_BUFFER_CAPACITY_DATA));
		cdb.AllocationLength[1] = LOBYTE(sizeof(READ_BUFFER_CAPACITY_DATA));

		_declspec(align(4)) READ_BUFFER_CAPACITY_DATA readBufCapaData = { 0 };
		BYTE byScsiStatus = 0;
		if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH, &readBufCapaData,
			sizeof(READ_BUFFER_CAPACITY_DATA), &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			// not false.
			OutputDriveLogA(
				"Undefined SCSIOP_READ_BUFFER_CAPACITY Command on this drive\n");
		}
		else {
			OutputReadBufferCapacity(&readBufCapaData);
		}
	}
	return TRUE;
}

// https://msdn.microsoft.com/ja-jp/library/ff551396(v=vs.85).aspx
BOOL SetCDSpeed(
	PDEVICE pDevice,
	DWORD dwCDSpeedNum
	)
{
	if (pDevice->FEATURE.bySetCDSpeed) {
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
		setspeed.RequestType = CdromSetSpeed;
		if (dwCDSpeedNum == 0) {
			setspeed.ReadSpeed = pDevice->wMaxReadSpeed;
		}
		else {
			setspeed.ReadSpeed = wCDSpeedList[dwCDSpeedNum];
		}
		setspeed.WriteSpeed = 0;
		setspeed.RotationControl = CdromDefaultRotation;
		BYTE byScsiStatus = 0;
		if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH, &setspeed, 
			sizeof(CDROM_SET_SPEED), &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			// Somehow PX-W1210S fails...
			OutputDriveLogA(
				"Undefined SCSIOP_SET_CD_SPEED Command on this drive\n");
		}
		else {
			OutputSetSpeed(&setspeed);
		}
	}
	return TRUE;
}

// feature PLEXTOR drive below
BOOL SetSpeedRead(
	PDEVICE pDevice,
	BOOL bState
	)
{
	// PX-708, PXW4012 or older doesn't support SpeedRead
	if (pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PX760A ||
		pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PX755A ||
		pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PX716AL ||
		pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PX716A ||
		pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PX714A ||
		pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PX712A ||
		pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PREMIUM2 ||
		pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PREMIUM ||
		pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PXW5224A ||
		pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PXW4824A
		) {
		CONST WORD size = 8;
		BYTE buf[size] = { 0 };

		CDB::_CDB12 cdb = { 0 };
		cdb.OperationCode = SCSIOP_PLXTR_EXTEND;
		cdb.DisablePageOut = TRUE;
		cdb.LogicalBlock[0] = PLXTR_FLAG_SPEED_READ;
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

BOOL Reset(
	PDEVICE pDevice
	)
{
	CDB::_CDB6GENERIC cdb = { 0 };
	cdb.OperationCode = SCSIOP_PLXTR_RESET;

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
	switch (pDevice->byPlxtrType) {
	case PLXTR_DRIVE_TYPE::PXW5224A:
	case PLXTR_DRIVE_TYPE::PREMIUM:
	case PLXTR_DRIVE_TYPE::PREMIUM2:
		tLen = 160;
		break;
	case PLXTR_DRIVE_TYPE::PX320A:
	case PLXTR_DRIVE_TYPE::PX704A:
	case PLXTR_DRIVE_TYPE::PX708A:
	case PLXTR_DRIVE_TYPE::PX708A2:
		tLen = 256;
		break;
	case PLXTR_DRIVE_TYPE::PX712A:
		tLen = 512;
		break;
	case PLXTR_DRIVE_TYPE::PX714A:
	case PLXTR_DRIVE_TYPE::PX716A:
	case PLXTR_DRIVE_TYPE::PX716AL:
	case PLXTR_DRIVE_TYPE::PX755A:
	case PLXTR_DRIVE_TYPE::PX760A:
		tLen = 256;
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
	CDB::_CDB12 cdb = { 0 };
	cdb.OperationCode = SCSIOP_PLXTR_READ_EEPROM;
	cdb.RelativeAddress = (BYTE)bHigh;

	BOOL bRet = TRUE;
	BYTE byScsiStatus = 0;
	OutputDriveLogA(
		"=================================== EEPROM ==================================\n");
	try {
		for (BYTE idx = 0; idx < 4; idx++) {
			cdb.TransferLength[0] = HIBYTE(HIWORD(tLen));
			cdb.TransferLength[1] = LOBYTE(HIWORD(tLen));
			cdb.TransferLength[2] = HIBYTE(LOWORD(tLen));
			cdb.TransferLength[3] = LOBYTE(LOWORD(tLen));
			if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH,
				pBuf, BufLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			OutputEeprom(pBuf, tLen, idx, pDevice->byPlxtrType);
			if (bHigh) {
				tLen += 0x10000;
			}
			else {
				break;
			}
		}
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(pPBuf);
	return bRet;
}
