/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"
#include "struct.h"
#include "check.h"
#include "convert.h"
#include "execMMC.h"
#include "execMMCforCD.h"
#include "execIoctl.h"
#include "get.h"
#include "init.h"
#include "output.h"
#include "outputMMCLog.h"
#include "outputMMCLogforCD.h"
#include "set.h"
#include "_external/3do.h"

BOOL ProcessReadCD(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	PC2_ERROR_DATA pC2ErrorData,
	PC2_ERROR_DATA_PER_SECTOR pC2ErrorDataPerSector,
	UINT uiC2ErrorLBACnt,
	INT nLBA,
	LPBYTE lpBuf,
	LPBYTE lpBufTmp,
	LPBYTE lpCmd,
	PSUB_Q_DATA pPrevSubQ,
	LPBYTE lpSubcode,
	LPBYTE lpNextSubcode,
	FILE* fpImg,
	BOOL bCheckReading
	)
{
	BOOL bRet = RETURNED_C2_ERROR_NO_1ST;
	if (*pExecType != gd) {
		if (pDiscData->SCSI.nFirstLBAof2ndSession != -1) {
			if (pExtArg->bReverse) {
				if (pDiscData->SCSI.nFirstLBAof2ndSession == nLBA + 1) {
					OutputInfoLogA(
						"Skip from Leadout of Session 1 [%d, %#x] to Leadin of Session 2 [%d, %#x]\n",
						pDiscData->SCSI.nFirstLBAofLeadout, pDiscData->SCSI.nFirstLBAofLeadout, 
						pDiscData->SCSI.nFirstLBAof2ndSession - 1, pDiscData->SCSI.nFirstLBAof2ndSession - 1);
					pPrevSubQ->nAbsoluteTime = nLBA - 11400 - 150;
					return RETURNED_SKIP_LBA;
				}
			}
			else {
				if (pDiscData->MAIN_CHANNEL.nFixFirstLBAofLeadout == nLBA) {
					OutputInfoLogA(
						"Skip from Leadout of Session 1 [%d, %#x] to Leadin of Session 2 [%d, %#x]\n",
						pDiscData->SCSI.nFirstLBAofLeadout, pDiscData->SCSI.nFirstLBAofLeadout, 
						pDiscData->SCSI.nFirstLBAof2ndSession - 1, pDiscData->SCSI.nFirstLBAof2ndSession - 1);
					if (pDiscData->MAIN_CHANNEL.nCombinedOffset > 0) {
						pPrevSubQ->nAbsoluteTime = 
							nLBA + 11400 + 150 - pDiscData->MAIN_CHANNEL.nAdjustSectorNum - 1;
					}
					else if (pDiscData->MAIN_CHANNEL.nCombinedOffset < 0) {
						pPrevSubQ->nAbsoluteTime = 
							nLBA + 11400 + 150 + pDiscData->MAIN_CHANNEL.nAdjustSectorNum;
					}
					return RETURNED_SKIP_LBA;
				}
			}
		}
	}
	try {
		lpCmd[2] = HIBYTE(HIWORD(nLBA));
		lpCmd[3] = LOBYTE(HIWORD(nLBA));
		lpCmd[4] = HIBYTE(LOWORD(nLBA));
		lpCmd[5] = LOBYTE(LOWORD(nLBA));
		BYTE byScsiStatus = 0;
		if (!ScsiPassThroughDirect(pDevData, lpCmd, CDB12GENERIC_LENGTH, lpBufTmp,
			pDevData->TRANSFER_DATA.dwBufLen + pDevData->TRANSFER_DATA.dwAdditionalBufLen,
			&byScsiStatus, _T(__FUNCTION__), __LINE__)) {
			throw RETURNED_FALSE;
		}

		if (byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			if (*pExecType != gd && !bCheckReading) {
				UINT uiSize = 0;
				ZeroMemory(lpBufTmp, pDevData->TRANSFER_DATA.dwBufLen);
				if (nLBA == pDiscData->MAIN_CHANNEL.nFixStartLBA) {
					uiSize = 
						CD_RAW_SECTOR_SIZE - pDiscData->MAIN_CHANNEL.uiMainDataSlideSize;
					fwrite(lpBufTmp + pDiscData->MAIN_CHANNEL.uiMainDataSlideSize,
						sizeof(BYTE), uiSize, fpImg);
				}
				else if (nLBA == pDiscData->MAIN_CHANNEL.nFixEndLBA) {
					uiSize = pDiscData->MAIN_CHANNEL.uiMainDataSlideSize;
					fwrite(lpBufTmp, sizeof(BYTE), uiSize, fpImg);
				}
				else {
					uiSize = CD_RAW_SECTOR_SIZE;
					fwrite(lpBufTmp, sizeof(BYTE), uiSize, fpImg);
				}
				OutputErrorStringA(
					"LBA[%06d, %#07x] Read error. Zero padding [%ubyte]\n",
					nLBA, nLBA, uiSize);
				if (pPrevSubQ) {
					if (pPrevSubQ->byIndex == 0) {
						pPrevSubQ->nRelativeTime--;
					}
					else {
						pPrevSubQ->nRelativeTime++;
					}
					pPrevSubQ->nAbsoluteTime++;
				}
				throw RETURNED_CONTINUE;
			}
			else {
				throw RETURNED_FALSE;
			}
		}
		memcpy(lpBuf, lpBufTmp, pDevData->TRANSFER_DATA.dwBufLen);
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}

	if (bRet == RETURNED_C2_ERROR_NO_1ST && !bCheckReading) {
		if (pC2ErrorDataPerSector && pDevData->bC2ErrorData && pExtArg->bC2) {
			bRet = CheckC2Error(pC2ErrorData,
				pC2ErrorDataPerSector, pDevData, lpBuf, uiC2ErrorLBACnt);
		}
		AlignRowSubcode(lpBuf + pDevData->TRANSFER_DATA.dwBufSubOffset, lpSubcode);
		if (pDevData->TRANSFER_DATA.uiTransferLen > 1) {
			if (pC2ErrorDataPerSector && pDevData->bC2ErrorData && pExtArg->bC2) {
				AlignRowSubcode(lpBuf + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE
					+ pDevData->TRANSFER_DATA.dwBufSubOffset, lpNextSubcode);
			}
			else {
				AlignRowSubcode(lpBuf + CD_RAW_SECTOR_WITH_SUBCODE_SIZE
					+ pDevData->TRANSFER_DATA.dwBufSubOffset, lpNextSubcode);
			}
		}
	}
	return bRet;
}

BOOL ReadCDForCheckingIndex0InTrack1(
	PEXT_ARG pExtArg,
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData
	)
{
	BOOL bRet = TRUE;
	if (pDevData->byPlexType == PLEX_DRIVE_TYPE::PX760 ||
		pDevData->byPlexType == PLEX_DRIVE_TYPE::PX755 ||
		pDevData->byPlexType == PLEX_DRIVE_TYPE::PX716 ||
		pDevData->byPlexType == PLEX_DRIVE_TYPE::PX712 ||
		pDevData->byPlexType == PLEX_DRIVE_TYPE::PX708 ||
		pDevData->byPlexType == PLEX_DRIVE_TYPE::Premium2 ||
		pDevData->byPlexType == PLEX_DRIVE_TYPE::PXW5232 ||
		pDevData->byPlexType == PLEX_DRIVE_TYPE::PXW5224) {
		pDiscData->SUB_CHANNEL.bIndex0InTrack1 = TRUE;
	}
	else {
		OutputString(
			_T("This drive doesn't support to rip from 00:00:00 to 00:01:74 AMSF. /p option is ignored\n"));
		pExtArg->bPre = FALSE;
	}
	return bRet;
}

BOOL ReadCDForCheckingCDG(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData
	)
{
	BOOL bRet = TRUE;
	LPBYTE pBuf = NULL;
	try {
		pBuf = (LPBYTE)calloc(
			CD_RAW_SECTOR_WITH_SUBCODE_SIZE + pDevData->AlignmentMask, sizeof(BYTE));
		if (!pBuf) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevData, pBuf);
		CDB::_READ_CD cdb = { 0 };
		SetReadCDCommand(pDevData, NULL, &cdb, TRUE, READ_CD_FLAG::All, 1, TRUE);

		for (BYTE i = 0; i < pDiscData->SCSI.toc.LastTrack; i++) {
			if ((pDiscData->SCSI.toc.TrackData[i].Control & AUDIO_DATA_TRACK) == 0) {
				INT nTmpLBA =
					pDiscData->SCSI.lpFirstLBAListOnToc[i] + 100;
				cdb.StartingLBA[0] = HIBYTE(HIWORD(nTmpLBA));
				cdb.StartingLBA[1] = LOBYTE(HIWORD(nTmpLBA));
				cdb.StartingLBA[2] = HIBYTE(LOWORD(nTmpLBA));
				cdb.StartingLBA[3] = LOBYTE(LOWORD(nTmpLBA));

				BYTE byScsiStatus = 0;
				if (!ScsiPassThroughDirect(pDevData, &cdb, CDB12GENERIC_LENGTH, lpBuf,
					CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
					|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
					throw FALSE;
				}
				else {
					OutputDiscLogA("======== Check CD+G for track[%02d] ========\n", i + 1);
					BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
					AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
					BYTE lpSubcodeOrg[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
					memcpy(lpSubcodeOrg, lpBuf + CD_RAW_SECTOR_SIZE, CD_RAW_READ_SUBCODE_SIZE);
					OutputMmcCDSub96Align(lpSubcode, nTmpLBA);

					SUB_R_TO_W_DATA scRW[4] = { 0 };
					BYTE tmpCode[24] = { 0 };
					INT nRtoW = 0;
					BOOL bCDG = FALSE;
					BOOL bCDEG = FALSE;
					for (INT k = 0; k < 4; k++) {
						for (INT j = 0; j < 24; j++) {
							tmpCode[j] = (BYTE)(*(lpSubcodeOrg + (k * 24 + j)) & 0x3F);
						}
						memcpy(&scRW[k], tmpCode, sizeof(scRW[k]));
						switch (scRW[k].command) {
						case 0: // MODE 0, ITEM 0
							break;
						case 8: // MODE 1, ITEM 0
							break;
						case 9: // MODE 1, ITEM 1
							bCDG = TRUE;
							break;
						case 10: // MODE 1, ITEM 2
							bCDEG = TRUE;
							break;
						case 20: // MODE 2, ITEM 4
							break;
						case 24: // MODE 3, ITEM 0
							break;
						case 56: // MODE 7, ITEM 0
							break;
						default:
							break;
						}
					}
					for (INT j = 24; j < CD_RAW_READ_SUBCODE_SIZE; j++) {
						nRtoW += lpSubcode[j];
					}
					// 0xff * 72 = 0x47b8
					if (nRtoW == 0x47b8) {
						// Why R-W bit is full? Basically, a R-W bit should be off except CD+G or CD-MIDI
						//  Alanis Morissette - Jagged Little Pill (UK)
						//  WipEout 2097: The Soundtrack
						//  and more..
						// Sub Channel LBA 75
						// 	  +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B
						// 	P 00 00 00 00 00 00 00 00 00 00 00 00
						// 	Q 01 01 01 00 01 00 00 00 03 00 2c b9
						// 	R ff ff ff ff ff ff ff ff ff ff ff ff
						// 	S ff ff ff ff ff ff ff ff ff ff ff ff
						// 	T ff ff ff ff ff ff ff ff ff ff ff ff
						// 	U ff ff ff ff ff ff ff ff ff ff ff ff
						// 	V ff ff ff ff ff ff ff ff ff ff ff ff
						// 	W ff ff ff ff ff ff ff ff ff ff ff ff
						pDiscData->SUB_CHANNEL.lpRtoWList[i] = SUB_RTOW_TYPE::Fill;
						OutputDiscLogA("Track[%02u]: RtoW all 0xff\n", i + 1);
					}
					else if (bCDG && nRtoW > 0 && nRtoW != 0x200) {
						pDiscData->SUB_CHANNEL.lpRtoWList[i] = SUB_RTOW_TYPE::CDG;
						OutputDiscLogA("Track[%02u]: CD+G\n", i + 1);
					}
					else if (bCDEG && nRtoW > 0 && nRtoW != 0x200) {
						pDiscData->SUB_CHANNEL.lpRtoWList[i] = SUB_RTOW_TYPE::CDG;
						OutputDiscLogA("Track[%02u]: CD+EG\n", i + 1);
					}
					else {
						pDiscData->SUB_CHANNEL.lpRtoWList[i] = SUB_RTOW_TYPE::Zero;
						OutputDiscLogA("Track[%02u]: Nothing\n", i + 1);
					}
				}
			}
			else {
				pDiscData->SUB_CHANNEL.lpRtoWList[i] = SUB_RTOW_TYPE::Zero;
			}
		}
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(pBuf);
	return bRet;
}

BOOL ReadCDForCheckingByteOrder(
	PDEVICE_DATA pDevData,
	PEXT_ARG pExtArg,
	PDRIVE_DATA_ORDER pOrder
	)
{
	UINT newBufLen = 
		CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE + pDevData->TRANSFER_DATA.dwAdditionalBufLen;
	LPBYTE pBuf = (LPBYTE)calloc(newBufLen + pDevData->AlignmentMask, sizeof(BYTE));
	if (!pBuf) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevData, pBuf);

	BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
	if (pDevData->byPlexType) {
		CDB::_PLXTR_READ_CDDA cdb = { 0 };
		SetReadD8Command(pDevData, pExtArg, &cdb, 1, TRUE, FALSE);
		memcpy(lpCmd, &cdb, sizeof(lpCmd));
	}
	else {
		// non plextor && support scrambled ripping
		CDB::_READ_CD cdb = { 0 };
		SetReadCDCommand(pDevData, pExtArg, &cdb, TRUE, READ_CD_FLAG::CDDA, 1, FALSE);
		memcpy(lpCmd, &cdb, sizeof(lpCmd));
	}

	BOOL bRet = TRUE;
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pDevData, lpCmd, CDB12GENERIC_LENGTH, lpBuf, 
		newBufLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		OutputErrorString(_T("This drive doesn't support C2 error report\n"));
		bRet = FALSE;
	}
	else {
#ifdef _DEBUG
		OutputMmcCDMain2352(lpBuf, 0);
#endif
		OutputDriveLogA("========= Check main+c2+sub =========\n");
		OutputMmcCDC2Error296(drive, lpBuf + CD_RAW_SECTOR_SIZE, 0);
		OutputMmcCDSub96Raw(drive, lpBuf + CD_RAW_SECTOR_WITH_C2_294_SIZE, 0);

		OutputDriveLogA("================== Check main+sub+c2 ==================\n");
		OutputMmcCDSub96Raw(drive, lpBuf + CD_RAW_SECTOR_SIZE, 0);
		OutputMmcCDC2Error296(drive, lpBuf + CD_RAW_SECTOR_WITH_SUBCODE_SIZE, 0);

		BYTE subcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		memcpy(subcode, lpBuf + CD_RAW_SECTOR_WITH_C2_294_SIZE, CD_RAW_READ_SUBCODE_SIZE);
		// check main + c2 + sub order
		BOOL bMainSubC2 = TRUE;
		for (INT i = 0; i < CD_RAW_READ_SUBCODE_SIZE; i++) {
			if (subcode[i]) {
				bMainSubC2 = FALSE;
				break;
			}
		}
		if (bMainSubC2) {
			*pOrder = DRIVE_DATA_ORDER::MainSubC2;
		}
	}
	FreeAndNull(pBuf);
	return bRet;
}

BOOL ReadCDForCheckingReadInOut(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	LPCTSTR pszPath,
	READ_CD_FLAG::SECTOR_TYPE flg
	)
{
	BOOL bRet = TRUE;
	if (pDiscData->MAIN_CHANNEL.nCombinedOffset < 0) {
		OutputString(_T("Checking reading lead-in\n"));
		bRet = ReadCDPartial(pExecType, pExtArg, pDevData,
			pDiscData, pszPath, -1, 0, flg, TRUE);
		if (bRet) {
			OutputDriveLogA("Reading lead-in: OK\n");
			OutputString(_T("Reading lead-in: OK\n"));
		}
		else {
			OutputDriveLogA("Reading lead-in: NG\n");
			OutputString(_T("Reading lead-in: NG\n"));
			bRet = FALSE;
		}
	}
	else if (0 < pDiscData->MAIN_CHANNEL.nCombinedOffset) {
		OutputString(_T("Checking reading lead-out\n"));
		bRet = ReadCDPartial(pExecType, pExtArg, pDevData, pDiscData, pszPath, 
			pDiscData->SCSI.nAllLength, pDiscData->SCSI.nAllLength, flg, TRUE);
		if (bRet) {
			OutputDriveLogA("Reading lead-out: OK\n");
			OutputString(_T("Reading lead-out: OK\n"));
		}
		else {
			OutputDriveLogA("Reading lead-out: NG\n");
			OutputString(_T("Reading lead-out: NG\n"));
			bRet = FALSE;
		}
	}
	return bRet;
}

BOOL ReadCDForFlushingDriveCache(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData
	)
{
	LPBYTE pBuf = (LPBYTE)calloc(
		pDevData->uiMaxTransferLength + pDevData->AlignmentMask, sizeof(BYTE));
	if (!pBuf) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevData, pBuf);
	UINT uiTransferLen = pDevData->uiMaxTransferLength / DISC_RAW_READ;

	BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
	// Drive cache flush??
	lpCmd[0] = SCSIOP_READ12;
	lpCmd[1] = CDB_FORCE_MEDIA_ACCESS;
	lpCmd[2] = HIBYTE(HIWORD(pDiscData->SCSI.nFirstLBAofDataTrack + 675));
	lpCmd[3] = LOBYTE(HIWORD(pDiscData->SCSI.nFirstLBAofDataTrack + 675));
	lpCmd[4] = HIBYTE(LOWORD(pDiscData->SCSI.nFirstLBAofDataTrack + 675));
	lpCmd[5] = LOBYTE(LOWORD(pDiscData->SCSI.nFirstLBAofDataTrack + 675));
	lpCmd[6] = HIBYTE(HIWORD(uiTransferLen));
	lpCmd[7] = LOBYTE(HIWORD(uiTransferLen));
	lpCmd[8] = HIBYTE(LOWORD(uiTransferLen));
	lpCmd[9] = LOBYTE(LOWORD(uiTransferLen));

	BOOL bRet = TRUE;
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pDevData, lpCmd, CDB12GENERIC_LENGTH, lpBuf, 
		pDevData->uiMaxTransferLength, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		bRet = FALSE;
	}
	FreeAndNull(pBuf);
	return bRet;
}

BOOL ReadCDForVolumeDescriptor(
	PDEVICE_DATA pDevData,
	PCDROM_TOC pToc,
	INT nFirstLBAofDataTrack
	)
{
	BOOL bRet = TRUE;
	LPBYTE pBuf = (LPBYTE)calloc(
		CD_RAW_SECTOR_SIZE + pDevData->AlignmentMask, sizeof(BYTE));
	if (!pBuf) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevData, pBuf);
	CDB::_READ_CD cdb = { 0 };
	SetReadCDCommand(pDevData, NULL, &cdb, FALSE, READ_CD_FLAG::All, 1, TRUE);

	BYTE byScsiStatus = 0;
	try {
		// almost data track disc
		if ((pToc->TrackData[0].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			if (!ScsiPassThroughDirect(pDevData, &cdb, CDB12GENERIC_LENGTH, lpBuf,
				CD_RAW_SECTOR_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			if (IsValid3doDataHeader(lpBuf)) {
				OutputFs3doStructure(lpBuf, 0);
				DWORD rootDir =
					(DWORD)MAKELONG(MAKEWORD(lpBuf[123], lpBuf[122]), MAKEWORD(lpBuf[121], lpBuf[120]));
				cdb.ErrorFlags = 0;
				cdb.IncludeEDC = 0;
				cdb.HeaderCode = 0;
				cdb.IncludeSyncData = 0;
				RecuseDir(pDevData, &cdb, "/", rootDir);
			}
			else {
				cdb.StartingLBA[3] = 1;
				if (!ScsiPassThroughDirect(pDevData, &cdb, CDB12GENERIC_LENGTH, lpBuf,
					CD_RAW_SECTOR_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
					|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
					throw FALSE;
				}
				if (IsValidMacDataHeader(lpBuf + 1040)) {
					OutputFsMasterDirectoryBlocks(lpBuf, 1040, 1);
				}
				else if (IsValidMacDataHeader(lpBuf + 528)) {
					OutputFsMasterDirectoryBlocks(lpBuf, 528, 1);
				}
				else {
					cdb.StartingLBA[3] = 16;
					if (!ScsiPassThroughDirect(pDevData, &cdb, CDB12GENERIC_LENGTH, lpBuf,
						CD_RAW_SECTOR_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
						|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
						throw FALSE;
					}
					if (lpBuf[15] == DATA_BLOCK_MODE1) {
						OutputFsVolumeDescriptor(lpBuf, SYNC_SIZE + HEADER_SIZE, 16);
						if (lpBuf[SYNC_SIZE + HEADER_SIZE] == 0xff) {
							throw TRUE;
						}
					}
					else if (lpBuf[15] == DATA_BLOCK_MODE2) {
						OutputFsVolumeDescriptor(lpBuf,
							SYNC_SIZE + HEADER_SIZE + SUBHEADER_SIZE, 16);
						if (lpBuf[SYNC_SIZE + HEADER_SIZE + SUBHEADER_SIZE] == 0xff) {
							throw TRUE;
						}
					}
				}
			}
		}
		// pce, pc-fx disc
		else if ((pToc->TrackData[1].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			INT nLBA = nFirstLBAofDataTrack;
			cdb.StartingLBA[0] = HIBYTE(HIWORD(nLBA));
			cdb.StartingLBA[1] = LOBYTE(HIWORD(nLBA));
			cdb.StartingLBA[2] = HIBYTE(LOWORD(nLBA));
			cdb.StartingLBA[3] = LOBYTE(LOWORD(nLBA));
			if (!ScsiPassThroughDirect(pDevData, &cdb, CDB12GENERIC_LENGTH, lpBuf,
				CD_RAW_SECTOR_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			OutputFsPceStuff(lpBuf, nLBA);

			nLBA = nFirstLBAofDataTrack + 1;
			cdb.StartingLBA[0] = HIBYTE(HIWORD(nLBA));
			cdb.StartingLBA[1] = LOBYTE(HIWORD(nLBA));
			cdb.StartingLBA[2] = HIBYTE(LOWORD(nLBA));
			cdb.StartingLBA[3] = LOBYTE(LOWORD(nLBA));
			if (!ScsiPassThroughDirect(pDevData, &cdb, CDB12GENERIC_LENGTH, lpBuf,
				CD_RAW_SECTOR_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			OutputFsPceBootSector(lpBuf, nLBA);
		}
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(pBuf);
	return bRet;
}

BOOL ReadCDForRereadingSector(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	PC2_ERROR_DATA pC2ErrorData,
	PC2_ERROR_DATA_PER_SECTOR pC2ErrorDataPerSector,
	UINT uiC2ErrorLBACnt,
	LPBYTE lpBuf,
	LPBYTE lpBufTmp,
	LPBYTE lpCmd,
	PSUB_Q_DATA pPrevSubQ,
	LPBYTE lpSubcode,
	LPBYTE lpNextSubcode,
	FILE* fpImg
	)
{
	BOOL bProcessRet = TRUE;
	if (uiC2ErrorLBACnt > 0) {
		OutputString(
			_T("\nChange reading speed: %ux\n"), pExtArg->dwRereadSpeedNum);
		SetCDSpeed(pDevData, pExtArg->dwRereadSpeedNum);
	}
	UINT uiCnt = 0;

	while (uiC2ErrorLBACnt > 0) {
		if (uiCnt == pExtArg->dwMaxRereadNum) {
			OutputString(_T("Reread reached max: %u\n"), uiCnt);
			bProcessRet = RETURNED_FALSE;
			break;
		}
		if (!pDiscData->SCSI.bAudioOnly) {
			ReadCDForFlushingDriveCache(pDevData, pDiscData);
		}
		UINT uiC2ErrorLBACntBackup = uiC2ErrorLBACnt;
		uiC2ErrorLBACnt = 0;
		SetC2ErrorBackup(pC2ErrorDataPerSector, 
			uiC2ErrorLBACntBackup, pDevData->TRANSFER_DATA.dwAllBufLen);

		UINT i = 0;
		for (INT nLBA = pC2ErrorDataPerSector[0].nErrorLBANumBackup; i < uiC2ErrorLBACntBackup; i++) {
			OutputString(
				_T("\rReread times %4u, ErrSectorNum %4u/%4u"), 
				uiCnt + 1, i + 1, uiC2ErrorLBACntBackup);
			nLBA = pC2ErrorDataPerSector[i].nErrorLBANumBackup;
			bProcessRet = ProcessReadCD(pExecType, pExtArg, pDevData, pDiscData, 
				pC2ErrorData, pC2ErrorDataPerSector, uiC2ErrorLBACnt, nLBA, lpBuf,
				lpBufTmp, lpCmd, pPrevSubQ, lpSubcode, lpNextSubcode, fpImg, FALSE);

			if (bProcessRet == RETURNED_C2_ERROR_EXIST) {
				SetAndOutputc2ErrorDataPerSector(pC2ErrorData, pC2ErrorDataPerSector,
					nLBA, pDevData->TRANSFER_DATA.dwAllBufLen, uiC2ErrorLBACnt);
				uiC2ErrorLBACnt++;
			}
			else if (bProcessRet == RETURNED_C2_ERROR_NO_1ST) {
				if (pC2ErrorDataPerSector[i].bErrorFlagBackup == RETURNED_C2_ERROR_EXIST ||
					pC2ErrorDataPerSector[i].bErrorFlagBackup == RETURNED_C2_ERROR_NO_BUT_BYTE_ERROR) {
					SetAndOutputC2NoErrorData(pC2ErrorDataPerSector, lpBuf, nLBA,
						pDevData->TRANSFER_DATA.dwAllBufLen, uiC2ErrorLBACnt);
					if (pC2ErrorDataPerSector[i].bErrorFlagBackup == RETURNED_C2_ERROR_NO_BUT_BYTE_ERROR) {
						if (CheckByteError(pC2ErrorDataPerSector, lpBuf, i)) {
							SetAndOutputC2NoErrorByteErrorData(pC2ErrorDataPerSector, lpBuf, nLBA,
								pDevData->TRANSFER_DATA.dwAllBufLen, uiC2ErrorLBACnt);
							uiC2ErrorLBACnt++;
						}
						else {
							LONG lPos = LONG(CD_RAW_SECTOR_SIZE * nLBA - pDiscData->MAIN_CHANNEL.nCombinedOffset);
							LONG lEndPos = lPos + CD_RAW_SECTOR_SIZE - 1;
							OutputErrorLogA(
								"LBA[%06d, %#07x], PrevBuf vs. PresentBuf matched: Fixed Main data from [%d, %#x] to [%d, %#x]\n",
								nLBA, nLBA, lPos, lPos, lEndPos, lEndPos);
							fseek(fpImg, lPos, SEEK_SET);
							WriteMainChannel(pExtArg, pDiscData, lpBuf, nLBA, fpImg);
						}
					}
					else {
						uiC2ErrorLBACnt++;
					}
				}
				else {
					if (CheckByteError(pC2ErrorDataPerSector, lpBuf, i)) {
						SetAndOutputC2NoErrorByteErrorData(pC2ErrorDataPerSector, lpBuf, nLBA,
							pDevData->TRANSFER_DATA.dwAllBufLen, uiC2ErrorLBACnt);
						uiC2ErrorLBACnt++;
					}
					else {
						LONG lPos = LONG(CD_RAW_SECTOR_SIZE * nLBA - pDiscData->MAIN_CHANNEL.nCombinedOffset);
						LONG lEndPos = lPos + CD_RAW_SECTOR_SIZE - 1;
						OutputErrorLogA(
							"LBA[%06d, %#07x], Reread data matched: Fixed Main data from [%d, %#x] to [%d, %#x]\n",
							nLBA, nLBA, lPos, lPos, lEndPos, lEndPos);
						fseek(fpImg, lPos, SEEK_SET);
						// Write track to scrambled again
						WriteMainChannel(pExtArg, pDiscData, lpBuf, nLBA, fpImg);
					}
				}
			}
			else if (bProcessRet == RETURNED_FALSE) {
				break;
			}
		}
		OutputString(_T("\n"));
		uiCnt++;
	}
	if (uiCnt == 0 && uiC2ErrorLBACnt == 0) {
		OutputString(_T("C2 error didn't exist\n"));
	}
	else if (uiCnt > 0 && uiC2ErrorLBACnt == 0) {
		OutputString(_T("C2 error was fixed at all\n"));
	}
	else if (uiC2ErrorLBACnt > 0) {
		OutputString(_T("There are unrecoverable errors\n"));
	}
	return bProcessRet;
}

BOOL ReadCDAll(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	LPCTSTR pszPath,
	FILE* fpCcd
	)
{
	if (pDiscData->SCSI.toc.FirstTrack < 1 || 99 < pDiscData->SCSI.toc.FirstTrack ||
		pDiscData->SCSI.toc.LastTrack < 1 || 99 < pDiscData->SCSI.toc.LastTrack) {
		return FALSE;
	}
	FILE* fpImg = NULL;
	_TCHAR pszOutReverseScmFile[_MAX_PATH] = { 0 };
	_TCHAR pszOutScmFile[_MAX_PATH] = { 0 };
	if (pExtArg->bReverse) {
		if (NULL == (fpImg = CreateOrOpenFileW(
			pszPath, _T("_reverse"), pszOutReverseScmFile, NULL, NULL, _T(".scm"), _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}
	else {
		if (NULL == (fpImg = CreateOrOpenFileW(
			pszPath, NULL, pszOutScmFile, NULL, NULL, _T(".scm"), _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}
	BOOL bRet = TRUE;
	FILE* fpC2 = NULL;
	FILE* fpCue = NULL;
	FILE* fpCueForImg = NULL;
	FILE* fpParse = NULL;
	FILE* fpSub = NULL;
	FILE* fpTbl = NULL;
	LPBYTE pBuf = NULL;
	LPBYTE pBufTmp = NULL;
	PC2_ERROR_DATA_PER_SECTOR pC2ErrorDataPerSector = NULL;
	try {
		// init start
		if (NULL == (fpTbl = OpenProgrammabledFile(_T("scramble.bin"), _T("rb")))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpCue = CreateOrOpenFileW(
			pszPath, NULL, NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpCueForImg = CreateOrOpenFileW(
			pszPath, _T("_img"), NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpParse = CreateOrOpenFileW(
			pszPath, _T("_sub"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpSub = CreateOrOpenFileW(
			pszPath, NULL, NULL, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}

		size_t dwTrackAllocSize = (size_t)pDiscData->SCSI.toc.LastTrack + 1;
		if (NULL == (pDiscData->SUB_CHANNEL.lpFirstLBAListOnSub = 
			(LPINT*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (pDiscData->SUB_CHANNEL.lpFirstLBAListOnSubSync = 
			(LPINT*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (pDiscData->SUB_CHANNEL.lpFirstLBAListOfDataTrackOnSub = 
			(LPINT)calloc(dwTrackAllocSize, sizeof(INT)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (pDiscData->SUB_CHANNEL.lpLastLBAListOfDataTrackOnSub = 
			(LPINT)calloc(dwTrackAllocSize, sizeof(INT)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (pDiscData->SUB_CHANNEL.lpISRCList = 
			(LPBOOL)calloc(dwTrackAllocSize, sizeof(BOOL)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (pDiscData->SUB_CHANNEL.lpCtlList = 
			(LPBYTE)calloc(dwTrackAllocSize, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (pDiscData->SUB_CHANNEL.lpModeList = 
			(LPBYTE)calloc(dwTrackAllocSize, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (pDiscData->SUB_CHANNEL.lpEndCtlList = 
			(LPBYTE)calloc(dwTrackAllocSize, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}

		size_t dwIndexAllocSize = (size_t)MAXIMUM_NUMBER_INDEXES * sizeof(UINT_PTR);
		for (INT h = 0; h < pDiscData->SCSI.toc.LastTrack + 1; h++) {
			if (NULL == (pDiscData->SUB_CHANNEL.lpFirstLBAListOnSub[h] = (LPINT)malloc(dwIndexAllocSize))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			FillMemory(pDiscData->SUB_CHANNEL.lpFirstLBAListOnSub[h], dwIndexAllocSize, -1);
			if (NULL == (pDiscData->SUB_CHANNEL.lpFirstLBAListOnSubSync[h] = (LPINT)malloc(dwIndexAllocSize))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			FillMemory(pDiscData->SUB_CHANNEL.lpFirstLBAListOnSubSync[h], dwIndexAllocSize, -1);
			pDiscData->SUB_CHANNEL.lpFirstLBAListOfDataTrackOnSub[h] = -1;
			pDiscData->SUB_CHANNEL.lpLastLBAListOfDataTrackOnSub[h] = -1;
		}

		SetCDTransferData(pDevData, DRIVE_DATA_ORDER::C2None);
		C2_ERROR_DATA pC2ErrorData = { 0 };
		if (pDevData->bC2ErrorData && pExtArg->bC2) {
			if (NULL == (fpC2 = CreateOrOpenFileW(
				pszPath, NULL, NULL, NULL, NULL, _T(".c2"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			SetCDTransferData(pDevData, DRIVE_DATA_ORDER::MainC2Sub);
			DRIVE_DATA_ORDER dataOrder = DRIVE_DATA_ORDER::MainC2Sub;

			if (!InitC2ErrorData(pExtArg, pDiscData, &pC2ErrorData,
				&pC2ErrorDataPerSector, pDevData->TRANSFER_DATA.dwAllBufLen)) {
				throw FALSE;
			}
			if (!ReadCDForCheckingByteOrder(pDevData, pExtArg, &dataOrder)) {
				Terminatec2ErrorDataPerSector(pDevData, pExtArg, &pC2ErrorDataPerSector);
				pDevData->bC2ErrorData = FALSE;
				SetCDTransferData(pDevData, DRIVE_DATA_ORDER::C2None);
			}
			if (dataOrder == DRIVE_DATA_ORDER::MainSubC2) {
				OutputDriveLogA(
					"Byte order of this drive is main + sub + c2\n");
				SetCDTransferData(pDevData, DRIVE_DATA_ORDER::MainSubC2);
			}
			else {
				OutputDriveLogA(
					"Byte order of this drive is main + c2 + sub\n");
			}
		}
#ifdef _DEBUG
		OutputString(
			_T("TransferLen %u, BufLen %ubyte, AdditionalBufLen %ubyte, AllBufLen %ubyte, BufC2Offset %ubyte, BufSubOffset %ubyte\n"),
			pDevData->TRANSFER_DATA.uiTransferLen, pDevData->TRANSFER_DATA.dwBufLen,
			pDevData->TRANSFER_DATA.dwAdditionalBufLen, pDevData->TRANSFER_DATA.dwAllBufLen, 
			pDevData->TRANSFER_DATA.dwBufC2Offset, pDevData->TRANSFER_DATA.dwBufSubOffset);
#endif
		// store main + (c2) + sub data all
		if (NULL == (pBuf = (LPBYTE)calloc(
			pDevData->TRANSFER_DATA.dwAllBufLen + pDevData->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevData, pBuf);

		if (NULL == (pBufTmp = (LPBYTE)calloc(pDevData->TRANSFER_DATA.dwBufLen + 
			pDevData->TRANSFER_DATA.dwAdditionalBufLen + pDevData->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBufTmp = (LPBYTE)ConvParagraphBoundary(pDevData, pBufTmp);

		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		_TCHAR szSubCode[5] = { 0 };
		if (pDevData->byPlexType && !pDiscData->SCSI.bAudioOnly) {
			CDB::_PLXTR_READ_CDDA cdb = { 0 };
			SetReadD8Command(pDevData, pExtArg, &cdb, 1, TRUE, FALSE);
			if (cdb.SubCode == READ_D8_FLAG::MainPack) {
				_tcsncpy(szSubCode, _T("Pack"), 4);
			}
			else if (cdb.SubCode == READ_D8_FLAG::MainC2Raw) {
				_tcsncpy(szSubCode, _T("Raw"), 3);
			}
			memcpy(lpCmd, &cdb, sizeof(lpCmd));
		}
		else {
			// non plextor && support scrambled ripping
			CDB::_READ_CD cdb = { 0 };
			SetReadCDCommand(pDevData, pExtArg, &cdb,
				TRUE, READ_CD_FLAG::CDDA, 1, FALSE);
			if (cdb.SubChannelSelection == READ_CD_FLAG::Raw) {
				_tcsncpy(szSubCode, _T("Raw"), 3);
			}
			memcpy(lpCmd, &cdb, sizeof(lpCmd));
		}
		OutputString(_T("Operation Code: %#x, Sub Code: %s\n"), lpCmd[0], szSubCode);

		BYTE byScsiStatus = 0;
		BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		SUB_Q_DATA prevSubQ = { 0 };

		lpCmd[2] = HIBYTE(HIWORD(-1));
		lpCmd[3] = LOBYTE(HIWORD(-1));
		lpCmd[4] = HIBYTE(LOWORD(-1));
		lpCmd[5] = LOBYTE(LOWORD(-1));
		if (!ScsiPassThroughDirect(pDevData, lpCmd, CDB12GENERIC_LENGTH, lpBuf,
			pDevData->TRANSFER_DATA.dwAllBufLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw FALSE;
		}
		AlignRowSubcode(lpBuf + pDevData->TRANSFER_DATA.dwBufSubOffset, lpSubcode);
		SetSubQDataFromReadCD(pDiscData, &prevSubQ, lpBuf, lpSubcode);
		if (prevSubQ.byAdr != ADR_ENCODES_CURRENT_POSITION) {
			// 1552 Tenka Tairan
			prevSubQ.byAdr = ADR_ENCODES_CURRENT_POSITION;
			prevSubQ.byTrackNum = 1;
		}
		if (prevSubQ.nAbsoluteTime != 149) {
			// 1552 Tenka Tairan
			prevSubQ.nAbsoluteTime = 149;
		}

		for (INT p = 0; p < pDiscData->SCSI.toc.LastTrack; p++) {
			lpCmd[2] = HIBYTE(HIWORD(pDiscData->SCSI.lpFirstLBAListOnToc[p]));
			lpCmd[3] = LOBYTE(HIWORD(pDiscData->SCSI.lpFirstLBAListOnToc[p]));
			lpCmd[4] = HIBYTE(LOWORD(pDiscData->SCSI.lpFirstLBAListOnToc[p]));
			lpCmd[5] = LOBYTE(LOWORD(pDiscData->SCSI.lpFirstLBAListOnToc[p]));
			if (!ScsiPassThroughDirect(pDevData, lpCmd, CDB12GENERIC_LENGTH, lpBuf, 
				pDevData->TRANSFER_DATA.dwAllBufLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			AlignRowSubcode(lpBuf + pDevData->TRANSFER_DATA.dwBufSubOffset, lpSubcode);
			pDiscData->SUB_CHANNEL.lpEndCtlList[p] = (BYTE)((lpSubcode[12] >> 4) & 0x0F);
		}

		SetCDOffsetData(pDiscData, 0, pDiscData->SCSI.nAllLength);
		BYTE byCurrentTrackNum = pDiscData->SCSI.toc.FirstTrack;
		INT nFirstLBAForSub = 0;
		INT nFirstLBA = pDiscData->MAIN_CHANNEL.nOffsetStart;
		INT nLastLBA = pDiscData->SCSI.nAllLength + pDiscData->MAIN_CHANNEL.nOffsetEnd;
		INT nLBA = nFirstLBA;	// This value switches by /r option.

		if (pExtArg->bReverse) {
			SetCDOffsetData(pDiscData, nLastLBA, pDiscData->SCSI.nFirstLBAofDataTrack);
			byCurrentTrackNum = pDiscData->SCSI.byLastDataTrack;
			nFirstLBA = pDiscData->SCSI.nFirstLBAofDataTrack;
			nLastLBA = pDiscData->SCSI.nLastLBAofDataTrack;
			nLBA = nLastLBA;
			if (pDiscData->MAIN_CHANNEL.nCombinedOffset > 0) {
				prevSubQ.nAbsoluteTime = 149 + nLastLBA;
			}
			else if (pDiscData->MAIN_CHANNEL.nCombinedOffset < 0) {
				prevSubQ.nAbsoluteTime = 150 + nLastLBA + pDiscData->MAIN_CHANNEL.nAdjustSectorNum - 1;
			}
			else {
				prevSubQ.nAbsoluteTime = 149 + nLastLBA;
			}

			prevSubQ.byCtl = pDiscData->SUB_CHANNEL.lpEndCtlList[pDiscData->SCSI.byLastDataTrack - 1];
			prevSubQ.byAdr = ADR_ENCODES_CURRENT_POSITION;
			prevSubQ.byTrackNum = pDiscData->SCSI.byLastDataTrack;
			prevSubQ.byIndex = pDiscData->MAIN_CHANNEL.nOffsetStart < 0 ? (BYTE)0 : (BYTE)1;
			prevSubQ.byMode = DATA_BLOCK_MODE0;
		}
		else if (pDiscData->SUB_CHANNEL.bIndex0InTrack1) {
			nFirstLBAForSub = -5000;
			nFirstLBA = -5000;
			nLBA = nFirstLBA;
			pDiscData->MAIN_CHANNEL.nOffsetStart = -5000;
		}
		// init end
#ifndef _DEBUG
		FlushLog();
#endif
		BYTE lpNextSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		SUB_Q_DATA prevPrevSubQ = { 0 };
		BOOL bCatalog = FALSE;
		UINT uiC2ErrorLBACnt = 0;
		BOOL bReadOK = pDiscData->SUB_CHANNEL.bIndex0InTrack1 ? FALSE : TRUE;

		while (nFirstLBA <= nLastLBA) {
			BOOL bProcessRet = ProcessReadCD(pExecType, pExtArg, pDevData,
				pDiscData, &pC2ErrorData, pC2ErrorDataPerSector, uiC2ErrorLBACnt, 
				nLBA, lpBuf, lpBufTmp, lpCmd, &prevSubQ, lpSubcode, lpNextSubcode, fpImg, FALSE);
#if defined _DEBUG
//			if (nLBA == 100) {
//				lpBuf[2352+100] = 0xff;
//				bProcessRet = RETURNED_C2_ERROR_EXIST;
//			}
#endif
			if (pC2ErrorDataPerSector && bProcessRet == RETURNED_C2_ERROR_EXIST) {
				INT nTmpLBA = nLBA + pC2ErrorData.cSlideSectorNum;
				OutputErrorStringA(
					" Detected C2 error. LBA[%06d, %#07x]\n", nTmpLBA, nTmpLBA);
				SetAndOutputc2ErrorDataPerSector(&pC2ErrorData, pC2ErrorDataPerSector,
					nTmpLBA, pDevData->TRANSFER_DATA.dwAllBufLen, uiC2ErrorLBACnt);
				uiC2ErrorLBACnt++;
				if (uiC2ErrorLBACnt == pExtArg->dwMaxC2ErrorNum) {
					OutputErrorStringA("C2 error Max: %u\n", uiC2ErrorLBACnt);
					throw FALSE;
				}
			}
			else if (bProcessRet == RETURNED_SKIP_LBA) {
				if (pExtArg->bReverse) {
					nLBA = pDiscData->SCSI.nFirstLBAof2ndSession - 11400;
				}
				else {
					nLBA = pDiscData->MAIN_CHANNEL.nFixFirstLBAof2ndSession - 1;
					nFirstLBA = nLBA;
				}
			}
			else if (bProcessRet == RETURNED_FALSE) {
				throw FALSE;
			}
			if (bProcessRet != RETURNED_CONTINUE &&
				bProcessRet != RETURNED_SKIP_LBA) {
				SUB_Q_DATA subQ = { 0 };
				SetSubQDataFromReadCD(pDiscData, &subQ, lpBuf, lpSubcode);
				if (pDiscData->SUB_CHANNEL.bIndex0InTrack1 && -5000 <= nLBA && nLBA <= -76) {
					if (subQ.byTrackNum == 1 && subQ.nAbsoluteTime == 0) {
						prevSubQ.nRelativeTime = subQ.nRelativeTime + 1;
						prevSubQ.nAbsoluteTime = -1;
						pDiscData->MAIN_CHANNEL.nFixStartLBA = nLBA;
						bReadOK = TRUE;
						if (pDiscData->MAIN_CHANNEL.nAdjustSectorNum < 0 ||
							1 < pDiscData->MAIN_CHANNEL.nAdjustSectorNum) {
							for (INT i = 0; i < abs(pDiscData->MAIN_CHANNEL.nAdjustSectorNum) * CD_RAW_SECTOR_SIZE; i++) {
								fputc(0, fpImg);
							}
						}
					}
					if (bReadOK) {
						if (subQ.byTrackNum == 1 && subQ.nAbsoluteTime == 74) {
							nFirstLBA = -76;
						}
					}
				}
				if (bReadOK) {
					SUB_Q_DATA nextSubQ = { 0 };
					SetSubQDataFromReadCD(pDiscData, &nextSubQ,
						lpBuf + pDevData->TRANSFER_DATA.dwBufLen, lpNextSubcode);
					if (nFirstLBAForSub <= nLBA && nLBA < pDiscData->SCSI.nAllLength) {
						if (nLBA < pDiscData->SCSI.nFirstLBAofLeadout ||
							pDiscData->SCSI.nFirstLBAof2ndSession <= nLBA) {
							BOOL bLibCrypt = CheckLibCryptSector(pExtArg->bLibCrypt, nLBA);
							if (!pExtArg->bReverse) {
								CheckAndFixSubChannel(pExtArg, pDiscData, lpSubcode,
									&nextSubQ, &subQ, &prevSubQ, &prevPrevSubQ,
									&byCurrentTrackNum, &bCatalog, nLBA, bLibCrypt);
								BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
								// fix raw subchannel
								AlignColumnSubcode(lpSubcode, lpSubcodeRaw);
#if 0
								OutputMmcCDSub96Align(lpSubcode, nLBA->fpError);
#endif
								// 3rd:Write subchannel
								WriteSubChannel(pDiscData, pDevData, lpBuf, lpSubcode,
									nLBA, byCurrentTrackNum, fpSub, fpParse);
							}
							PreserveTrackAttribution(pExtArg, pDiscData, nLBA,
								&byCurrentTrackNum, &subQ, &prevSubQ, &prevPrevSubQ);
							UpdateSubQData(&subQ, &prevSubQ, &prevPrevSubQ, bLibCrypt);
						}
					}
					CheckMainChannel(pDiscData, lpBuf, &nextSubQ, &subQ, 
						&prevSubQ, &prevPrevSubQ, byCurrentTrackNum, nLBA);
					// 4th:Write track to scrambled
					WriteMainChannel(pExtArg, pDiscData, lpBuf, nLBA, fpImg);
					if (pDevData->bC2ErrorData && pExtArg->bC2) {
						// TODO: exist an offset?
						fwrite(lpBuf + pDevData->TRANSFER_DATA.dwBufC2Offset,
							sizeof(BYTE), CD_RAW_READ_C2_294_SIZE, fpC2);
					}
				}
			}

			if (pExtArg->bReverse) {
				OutputString(_T("\rCreating img(LBA) %6d/%6d"),
					nLBA, pDiscData->SCSI.nFirstLBAofDataTrack);
				nLBA--;
			}
			else {
				OutputString(_T("\rCreating img(LBA) %6d/%6d"),
					nFirstLBA - 1, pDiscData->SCSI.nAllLength - 1);
				if (nFirstLBA == -76) {
					nLBA = nFirstLBA;
				}
				nLBA++;
			}
			nFirstLBA++;
		}
		OutputString(_T("\n"));
		if (pDevData->bC2ErrorData && pExtArg->bC2) {
			FcloseAndNull(fpC2);
		}
		FcloseAndNull(fpSub);
		FcloseAndNull(fpParse);

		if (pDiscData->SCSI.toc.FirstTrack == pDiscData->SCSI.toc.LastTrack) {
			pDiscData->SUB_CHANNEL.lpFirstLBAListOnSub[0][1] = pDiscData->SCSI.lpLastLBAListOnToc[0];
		}
		for (INT i = pDiscData->SCSI.toc.FirstTrack; i <= pDiscData->SCSI.toc.LastTrack; i++) {
			BOOL bErr = FALSE;
			LONG lLine = 0;
			if (pDiscData->SUB_CHANNEL.lpFirstLBAListOnSub[i - 1][1] == -1) {
				bErr = TRUE;
				lLine = __LINE__;
			}
			else if ((pDiscData->SCSI.toc.TrackData[i - 1].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				if (pDiscData->SUB_CHANNEL.lpFirstLBAListOfDataTrackOnSub[i - 1] == -1) {
					bErr = TRUE;
					lLine = __LINE__;
				}
				else if (pDiscData->SUB_CHANNEL.lpLastLBAListOfDataTrackOnSub[i - 1] == -1) {
					bErr = TRUE;
					lLine = __LINE__;
				}
			}
			if (bErr) {
				OutputErrorStringA(
					"[L:%d] Internal error. Failed to analyze the subchannel. Track[%02u]/[%02u]\n",
					lLine, i, pDiscData->SCSI.toc.LastTrack);
				throw FALSE;
			}
		}
		// c2 error: reread sector 
		if (pDevData->bC2ErrorData && pExtArg->bC2) {
			if (!ReadCDForRereadingSector(pExecType, pExtArg, pDevData, pDiscData,
				&pC2ErrorData, pC2ErrorDataPerSector, uiC2ErrorLBACnt, lpBuf,
				lpBufTmp, lpCmd, &prevSubQ, lpSubcode, lpNextSubcode, fpImg)) {
				throw FALSE;
			}
		}
		FcloseAndNull(fpImg);

		if (!pExtArg->bReverse) {
			OutputMmcTocWithPregap(pDiscData);
		}
		else {
			FILE* fpImg_r = NULL;
			if (NULL == (fpImg_r = CreateOrOpenFileW(
				pszPath, _T("_reverse"), NULL, NULL, NULL, _T(".scm"), _T("rb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == (fpImg = CreateOrOpenFileW(
				pszPath, NULL, pszOutScmFile, NULL, NULL, _T(".scm"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			OutputString(_T("Reversing _reverse.scm to .scm\n"));
			BYTE rBuf[CD_RAW_SECTOR_SIZE] = { 0 };
			DWORD dwRoop = GetFileSize(0, fpImg_r) - CD_RAW_SECTOR_SIZE;
			LONG lSeek = CD_RAW_SECTOR_SIZE - (LONG)pDiscData->MAIN_CHANNEL.uiMainDataSlideSize;
			fseek(fpImg_r, -lSeek, SEEK_END);
			fread(rBuf, sizeof(BYTE), (size_t)lSeek, fpImg_r);
			fwrite(rBuf, sizeof(BYTE), (size_t)lSeek, fpImg);
			fseek(fpImg_r, -lSeek, SEEK_CUR);
			for (DWORD i = 0; i < dwRoop; i += CD_RAW_SECTOR_SIZE) {
				fseek(fpImg_r, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
				fread(rBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpImg_r);
				fwrite(rBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpImg);
				fseek(fpImg_r, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
			}
			fseek(fpImg_r, -(LONG)pDiscData->MAIN_CHANNEL.uiMainDataSlideSize, SEEK_CUR);
			fread(rBuf, sizeof(BYTE), pDiscData->MAIN_CHANNEL.uiMainDataSlideSize, fpImg_r);
			fwrite(rBuf, sizeof(BYTE), pDiscData->MAIN_CHANNEL.uiMainDataSlideSize, fpImg);
			FcloseAndNull(fpImg);
			FcloseAndNull(fpImg_r);
		}

		_TCHAR pszNewPath[_MAX_PATH] = { 0 };
		_tcsncpy(pszNewPath, pszPath, _MAX_PATH);
		if (!PathRenameExtension(pszNewPath, _T(".img"))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		// 5th:audio only -> from .scm to .img. other descramble img.
		if (pDiscData->SCSI.bAudioOnly) {
			OutputString(_T("Moving .scm to .img\n"));
			if (!MoveFileEx(pszOutScmFile, pszNewPath, MOVEFILE_REPLACE_EXISTING)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
		}
		else {
			OutputString(_T("Copying .scm to .img\n"));
			if (!CopyFile(pszOutScmFile, pszNewPath, FALSE)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			_TCHAR pszImgPath[_MAX_PATH] = { 0 };
			if (NULL == (fpImg = CreateOrOpenFileW(
				pszPath, NULL, pszImgPath, NULL, NULL, _T(".img"), _T("rb+"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			DescrambleMainChannel(pExtArg, pDiscData, fpTbl, fpImg);
			FcloseAndNull(fpImg);
			FcloseAndNull(fpTbl);
			_TCHAR cmd[_MAX_PATH] = { 0 };
			if (GetEccEdcCheckCmd(cmd, _MAX_PATH, pszImgPath)) {
				_tsystem(cmd);
			}
		}

		if (pExtArg->bReverse) {
			_TCHAR pszNewPath2[_MAX_PATH] = { 0 };
			FILE* fpBin = CreateOrOpenFileW(pszPath, NULL, pszNewPath2, NULL, NULL, _T(".bin"),
				_T("wb"), pDiscData->SCSI.byFirstDataTrack, pDiscData->SCSI.byFirstDataTrack);
			if (!fpBin) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			OutputString(_T("Copying .img to %s\n"), pszNewPath2);
			if (!CopyFile(pszNewPath, pszNewPath2, FALSE)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			FcloseAndNull(fpBin);
		}
		else {
			_TCHAR pszImgName[_MAX_FNAME] = { 0 };
			if (NULL == (fpImg = CreateOrOpenFileW(
				pszPath, NULL, NULL, pszImgName, NULL, _T(".img"), _T("rb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (!CreateBinCueCcd(pDiscData, pszPath,
				pszImgName, bCatalog, fpImg, fpCue, fpCueForImg, fpCcd)) {
				throw FALSE;
			}
		}
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpImg);
	if (pDevData->bC2ErrorData && pExtArg->bC2) {
		FcloseAndNull(fpC2);
	}
	FcloseAndNull(fpCueForImg);
	FcloseAndNull(fpCue);
	FcloseAndNull(fpParse);
	FcloseAndNull(fpSub);
	FcloseAndNull(fpTbl);
	for (INT h = 0; h < pDiscData->SCSI.toc.LastTrack + 1; h++) {
		if (pDiscData->SUB_CHANNEL.lpFirstLBAListOnSub) {
			FreeAndNull(pDiscData->SUB_CHANNEL.lpFirstLBAListOnSub[h]);
		}
		if (pDiscData->SUB_CHANNEL.lpFirstLBAListOnSubSync) {
			FreeAndNull(pDiscData->SUB_CHANNEL.lpFirstLBAListOnSubSync[h]);
		}
	}
	FreeAndNull(pDiscData->SUB_CHANNEL.lpFirstLBAListOnSub);
	FreeAndNull(pDiscData->SUB_CHANNEL.lpFirstLBAListOnSubSync);
	FreeAndNull(pDiscData->SUB_CHANNEL.lpFirstLBAListOfDataTrackOnSub);
	FreeAndNull(pDiscData->SUB_CHANNEL.lpLastLBAListOfDataTrackOnSub);
	FreeAndNull(pDiscData->SUB_CHANNEL.lpCtlList);
	FreeAndNull(pDiscData->SUB_CHANNEL.lpEndCtlList);
	FreeAndNull(pDiscData->SUB_CHANNEL.lpISRCList);
	FreeAndNull(pDiscData->SUB_CHANNEL.lpModeList);
	FreeAndNull(pBuf);
	FreeAndNull(pBufTmp);
	Terminatec2ErrorDataPerSector(pDevData, pExtArg, &pC2ErrorDataPerSector);

	return bRet;
}

BOOL ReadCDForSearchingOffset(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData
	)
{
	BOOL bRet = FALSE;
	INT nDriveSampleOffset = 0;
	BOOL bGetDrive = GetDriveOffset(pDevData->szProductId, &nDriveSampleOffset);
	if (!bGetDrive) {
		CHAR buf[6] = { 0 };
		OutputString(
			_T("This drive doesn't define in driveOffset.txt\n")
			_T("Please input Drive offset(Samples): "));
		INT b = scanf("%6[^\n]%*[^\n]", buf);
		b = getchar();
		nDriveSampleOffset = atoi(buf) * 4;
	}

	INT nDriveOffset = nDriveSampleOffset * 4;
	if (pDiscData->SCSI.bAudioOnly) {
		pDiscData->MAIN_CHANNEL.nCombinedOffset = nDriveOffset;
		OutputMmcCDOffset(pExtArg, pDiscData, bGetDrive, nDriveSampleOffset, nDriveOffset);
		bRet = TRUE;
	}
	else {
		CONST DWORD dwBufLen = CD_RAW_SECTOR_SIZE * 2;
		LPBYTE pBuf = (LPBYTE)calloc(dwBufLen + pDevData->AlignmentMask, sizeof(BYTE));
		if (!pBuf) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevData, pBuf);

		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		if (pDevData->byPlexType) {
			CDB::_PLXTR_READ_CDDA cdb = { 0 };
			SetReadD8Command(pDevData, NULL, &cdb, 2, FALSE, TRUE);
			memcpy(lpCmd, &cdb, sizeof(lpCmd));
		}
		else {
			CDB::_READ_CD cdb = { 0 };
			SetReadCDCommand(pDevData, NULL, &cdb, FALSE, READ_CD_FLAG::CDDA, 2, TRUE);
			memcpy(lpCmd, &cdb, sizeof(lpCmd));
		}

		if (*pExecType == gd) {
			pDiscData->SCSI.nFirstLBAofDataTrack = 45000;
		}
		lpCmd[2] = HIBYTE(HIWORD(pDiscData->SCSI.nFirstLBAofDataTrack));
		lpCmd[3] = LOBYTE(HIWORD(pDiscData->SCSI.nFirstLBAofDataTrack));
		lpCmd[4] = HIBYTE(LOWORD(pDiscData->SCSI.nFirstLBAofDataTrack));
		lpCmd[5] = LOBYTE(LOWORD(pDiscData->SCSI.nFirstLBAofDataTrack));
#ifdef _DEBUG
		for (INT j = 0; j < CDB12GENERIC_LENGTH; j++) {
			OutputString(_T("Command[%d]: %#04x\n"), j, lpCmd[j]);
		}
#endif
		BYTE byScsiStatus = 0;
		try {
			if (!ScsiPassThroughDirect(pDevData, lpCmd, CDB12GENERIC_LENGTH, lpBuf, 
				dwBufLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				// PLEXTOR(PX-W8432, PX-W1210T, PX-W2410T)
				// if Track1 is DataTrack (mostly game)
				// ==>Sense data, Key:Asc:Ascq: 05:64:00(ILLEGAL_REQUEST. ILLEGAL MODE FOR THIS TRACK)
				// else if Track1 isn't DataTrack (pc engine etc)
				// ==>no error.
				if (*pExecType == gd) {
					OutputErrorString(
						_T("Could't read a data sector at scrambled mode\n")
						_T("Please start it again from 1st step or after waiting a little\n"));
				}
				else {
					OutputErrorString(
						_T("This drive can't read a data sector at scrambled mode\n")
						_T("Please start it again at plextor drive (PX-708, 712, 716, 755, 760 etc)\n"));
				}
				throw FALSE;
			}
			OutputDiscLogA("=================== Check Drive+CD offset ===================\n");
			OutputMmcCDMain2352(lpBuf, pDiscData->SCSI.nFirstLBAofDataTrack);

			BYTE aBuf2[dwBufLen] = { 0 };
			memcpy(aBuf2, lpBuf, dwBufLen);
			if (!GetWriteOffset(pDiscData, aBuf2)) {
				OutputErrorString(_T("Failed to get write-offset\n"));
				throw FALSE;
			}
			OutputMmcCDOffset(pExtArg, pDiscData, bGetDrive, nDriveSampleOffset, nDriveOffset);
			bRet = TRUE;
		}
		catch (BOOL bErr) {
			bRet = bErr;
		}
		FreeAndNull(pBuf);
	}

	if (bRet) {
		if (0 < pDiscData->MAIN_CHANNEL.nCombinedOffset) {
			pDiscData->MAIN_CHANNEL.nAdjustSectorNum =
				pDiscData->MAIN_CHANNEL.nCombinedOffset / CD_RAW_SECTOR_SIZE + 1;
		}
		else if (pDiscData->MAIN_CHANNEL.nCombinedOffset < 0) {
			pDiscData->MAIN_CHANNEL.nAdjustSectorNum =
				pDiscData->MAIN_CHANNEL.nCombinedOffset / CD_RAW_SECTOR_SIZE - 1;
		}
		OutputDiscLogA(
			"\tOverread sector: %d\n", pDiscData->MAIN_CHANNEL.nAdjustSectorNum);
	}
	return bRet;
}

BOOL ReadCDPartial(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	LPCTSTR pszPath,
	INT nStart,
	INT nEnd,
	READ_CD_FLAG::SECTOR_TYPE flg,
	BOOL bCheckReading
	)
{
	CONST INT size = 8;
	_TCHAR szPlusFnameSub[size] = { 0 };
	_TCHAR szPlusFnameTxt[size] = { 0 };
	_TCHAR szExt[size] = { 0 };

	if (*pExecType == gd) {
		_tcsncpy(szPlusFnameSub, _T("_dc"), size);
		_tcsncpy(szPlusFnameTxt, _T("_dc_sub"), size);
		_tcsncpy(szExt, _T(".scm2"), size);
	}
	else {
		_tcsncpy(szPlusFnameTxt, _T("_sub"), size);
		_tcsncpy(szExt, _T(".bin"), size);
	}
	FILE* fpBin = CreateOrOpenFileW(pszPath, NULL, NULL, NULL, NULL, szExt, _T("wb"), 0, 0);
	if (!fpBin) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	FILE* fpC2 = NULL;
	FILE* fpParse = NULL;
	FILE* fpSub = NULL;
	BOOL bRet = TRUE;
	LPBYTE pBuf = NULL;
	LPBYTE pBufTmp = NULL;
	C2_ERROR_DATA pC2ErrorData;
	PC2_ERROR_DATA_PER_SECTOR pC2ErrorDataPerSector = NULL;
	try {
		if (!pExtArg->bReverse) {
			if (NULL == (fpParse = CreateOrOpenFileW(
				pszPath, szPlusFnameTxt, NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == (fpSub = CreateOrOpenFileW(
				pszPath, szPlusFnameSub, NULL, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
		}

		SetCDTransferData(pDevData, DRIVE_DATA_ORDER::C2None);
		if (!bCheckReading && pDevData->bC2ErrorData && pExtArg->bC2) {
			if (NULL == (fpC2 = CreateOrOpenFileW(
				pszPath, NULL, NULL, NULL, NULL, _T(".c2"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			DRIVE_DATA_ORDER dataOrder = DRIVE_DATA_ORDER::MainC2Sub;
			SetCDTransferData(pDevData, DRIVE_DATA_ORDER::MainC2Sub);

			if (!InitC2ErrorData(pExtArg, pDiscData, &pC2ErrorData, 
				&pC2ErrorDataPerSector, pDevData->TRANSFER_DATA.dwAllBufLen)) {
				throw FALSE;
			}
			if (!ReadCDForCheckingByteOrder(pDevData, pExtArg, &dataOrder)) {
				Terminatec2ErrorDataPerSector(pDevData, pExtArg, &pC2ErrorDataPerSector);
				pDevData->bC2ErrorData = FALSE;
				SetCDTransferData(pDevData, DRIVE_DATA_ORDER::C2None);
			}
			if (dataOrder == DRIVE_DATA_ORDER::MainSubC2) {
				OutputDriveLogA(
					"Byte order of this drive is main + sub + c2\n");
				SetCDTransferData(pDevData, DRIVE_DATA_ORDER::MainSubC2);
			}
			else {
				OutputDriveLogA(
					"Byte order of this drive is main + c2 + sub\n");
			}
		}

		// store main+(c2)+sub data
		if (NULL == (pBuf = (LPBYTE)calloc(
			pDevData->TRANSFER_DATA.dwAllBufLen + pDevData->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevData, pBuf);

		if (NULL == (pBufTmp = (LPBYTE)calloc(
			pDevData->TRANSFER_DATA.dwBufLen + pDevData->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBufTmp = (LPBYTE)ConvParagraphBoundary(pDevData, pBufTmp);

		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		if (pDevData->byPlexType) {
			CDB::_PLXTR_READ_CDDA cdb = { 0 };
			SetReadD8Command(pDevData, pExtArg, &cdb, 1, TRUE, bCheckReading);
			memcpy(lpCmd, &cdb, sizeof(lpCmd));
		}
		else {
			// non plextor && support scrambled ripping
			CDB::_READ_CD cdb = { 0 };
			SetReadCDCommand(pDevData, pExtArg, &cdb, TRUE, flg, 1, bCheckReading);
			memcpy(lpCmd, &cdb, sizeof(lpCmd));
		}

		BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		BYTE lpNextSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		SetCDOffsetData(pDiscData, nStart, nEnd);
		if (*pExecType == gd) {
			// because address out of range
			pDiscData->MAIN_CHANNEL.nOffsetEnd = 0;
		}

#ifndef _DEBUG
		FlushLog();
#endif
		BYTE byCurrentTrackNum =
			*pExecType == gd ? (BYTE)3 : pDiscData->SCSI.toc.FirstTrack;
		UINT uiC2ErrorLBACnt = 0;

		for (INT nLBA = nStart + pDiscData->MAIN_CHANNEL.nOffsetStart; 
			nLBA <= nEnd + pDiscData->MAIN_CHANNEL.nOffsetEnd; nLBA++) {
			BOOL bProcessRet = ProcessReadCD(pExecType, pExtArg, pDevData, 
				pDiscData, &pC2ErrorData, pC2ErrorDataPerSector, uiC2ErrorLBACnt,
				nLBA, lpBuf, lpBufTmp, lpCmd, NULL, lpSubcode, lpNextSubcode, fpBin, bCheckReading);
			if (pC2ErrorDataPerSector && bProcessRet == RETURNED_C2_ERROR_EXIST) {
				INT nTmpLBA = nLBA + pC2ErrorData.cSlideSectorNum;
				OutputErrorStringA(
					" Detected C2 error. LBA[%06d, %#07x]\n", nTmpLBA, nTmpLBA);
				SetAndOutputc2ErrorDataPerSector(&pC2ErrorData, pC2ErrorDataPerSector,
					nTmpLBA, pDevData->TRANSFER_DATA.dwAllBufLen, uiC2ErrorLBACnt);
				uiC2ErrorLBACnt++;
				if (uiC2ErrorLBACnt == pExtArg->dwMaxC2ErrorNum) {
					OutputErrorStringA("C2 error Max: %u\n", uiC2ErrorLBACnt);
					throw FALSE;
				}
#ifdef _DEBUG
				OutputMmcCDMain2352(lpBuf, nLBA);
#endif
			}
			else if (bProcessRet == RETURNED_CONTINUE) {
				continue;
			}
			else if (bProcessRet == RETURNED_SKIP_LBA) {
				nLBA = pDiscData->SCSI.nFirstLBAof2ndSession - 1;
				continue;
			}
			else if (bProcessRet == RETURNED_FALSE) {
				throw FALSE;
			}
			if (nStart <= nLBA && nLBA < nEnd) {
				// TODO: Fix Subchannnel
#if 0
				SUB_Q_DATA subQ = { 0 };
				SetSubQDataFromReadCD(&subQ, lpBuf, lpSubcode);
				SUB_Q_DATA nextSubQ = { 0 };
				SetSubQDataFromReadCD(&nextSubQ, lpBuf, lpSubcode);
				CheckAndFixSubChannel(pExtArg, pDiscData, lpSubcode, &nextSubQ,
					&subQ, &prevSubQ, &prevPrevSubQ, &byCurrentTrackNum, &bCatalog,
					lpISRCList,	lpEndCtlList, lpFirstLBAListOnSub, 
					lpFirstLBAListOfDataTrackOnSub,	nLBA->fpError);
				BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
				// fix raw subchannel
				AlignColumnSubcode(lpSubcode, lpSubcodeRaw);
#else
				byCurrentTrackNum = BcdToDec(lpSubcode[13]);
				if (byCurrentTrackNum < 1 || 99 < byCurrentTrackNum) {
					byCurrentTrackNum = 1;
				}
#endif
#if 0
				OutputMmcCDSub96Align(lpSubcode, nLBA->fpError);
#endif
				if (!pExtArg->bReverse) {
					WriteSubChannel(pDiscData, pDevData, lpBuf, lpSubcode,
						nLBA, byCurrentTrackNum, fpSub, fpParse);
				}
			}
			WriteMainChannel(pExtArg, pDiscData, lpBuf, nLBA, fpBin);
			if (!bCheckReading && pDevData->bC2ErrorData && pExtArg->bC2) {
				// TODO: exist an offset?
				fwrite(lpBuf + pDevData->TRANSFER_DATA.dwBufC2Offset,
					sizeof(BYTE), CD_RAW_READ_C2_294_SIZE, fpC2);
			}
			OutputString(_T("\rCreating bin from %d to %d (LBA) %6d"), 
				nStart + pDiscData->MAIN_CHANNEL.nOffsetStart, 
				nEnd + pDiscData->MAIN_CHANNEL.nOffsetEnd, nLBA);
		}
		OutputString(_T("\n"));
		FcloseAndNull(fpC2);
		FcloseAndNull(fpParse);
		FcloseAndNull(fpSub);

		if (!bCheckReading) {
			if (pC2ErrorDataPerSector && pDevData->bC2ErrorData && pExtArg->bC2) {
				if (!ReadCDForRereadingSector(pExecType, pExtArg, 
					pDevData, pDiscData, &pC2ErrorData, pC2ErrorDataPerSector, uiC2ErrorLBACnt, lpBuf,
					lpBufTmp, lpCmd, NULL, lpSubcode, lpNextSubcode, fpBin)) {
					throw FALSE;
				}
			}
		}
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpBin);
	FcloseAndNull(fpC2);
	FcloseAndNull(fpParse);
	FcloseAndNull(fpSub);
	FreeAndNull(pBuf);
	FreeAndNull(pBufTmp);
	Terminatec2ErrorDataPerSector(pDevData, pExtArg, &pC2ErrorDataPerSector);

	if (bRet && *pExecType == gd) {
		if (!DescrambleMainChannelForGD(pszPath)) {
			return FALSE;
		}
		if (!SplitFileForGD(pszPath)) {
			return FALSE;
		}
	}
	return bRet;
}
