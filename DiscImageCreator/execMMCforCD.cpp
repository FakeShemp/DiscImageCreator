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

BOOL ExecReadCD(
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	PDISC pDisc,
	INT nLBA,
	LPBYTE lpBuf,
	LPBYTE lpCmd,
	PSUB_Q pPrevSubQ,
	FILE* fpImg,
	BOOL bCheckReading
	)
{
	lpCmd[2] = HIBYTE(HIWORD(nLBA));
	lpCmd[3] = LOBYTE(HIWORD(nLBA));
	lpCmd[4] = HIBYTE(LOWORD(nLBA));
	lpCmd[5] = LOBYTE(LOWORD(nLBA));
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pDevice, lpCmd, CDB12GENERIC_LENGTH, lpBuf,
		pDevice->TRANSFER.dwBufLen + pDevice->TRANSFER.dwAdditionalBufLen,
		&byScsiStatus, _T(__FUNCTION__), __LINE__)) {
		return RETURNED_FALSE;
	}

	if (byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		if (*pExecType != gd && !bCheckReading) {
			UINT uiSize = 0;
			ZeroMemory(lpBuf, pDevice->TRANSFER.dwBufLen);
			if (nLBA == pDisc->MAIN.nFixStartLBA) {
				uiSize =
					CD_RAW_SECTOR_SIZE - pDisc->MAIN.uiMainDataSlideSize;
				fwrite(lpBuf + pDisc->MAIN.uiMainDataSlideSize,
					sizeof(BYTE), uiSize, fpImg);
			}
			else if (nLBA == pDisc->MAIN.nFixEndLBA) {
				uiSize = pDisc->MAIN.uiMainDataSlideSize;
				fwrite(lpBuf, sizeof(BYTE), uiSize, fpImg);
			}
			else {
				uiSize = CD_RAW_SECTOR_SIZE;
				fwrite(lpBuf, sizeof(BYTE), uiSize, fpImg);
			}
			OutputErrorString(
				_T("LBA[%06d, %#07x] Read error. Zero padding [%ubyte]\n"),
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
			return RETURNED_CONTINUE;
		}
		else {
			return RETURNED_FALSE;
		}
	}
	return RETURNED_C2_ERROR_NO_1ST;
}

BOOL ProcessReadCD(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PC2_ERROR pC2Error,
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	UINT uiC2ErrorLBACnt,
	INT nLBA,
	LPBYTE lpBuf,
	LPBYTE lpBufNext,
	LPBYTE lpCmd,
	PSUB_Q pPrevSubQ,
	LPBYTE lpSubcode,
	LPBYTE lpNextSubcode,
	FILE* fpImg,
	BOOL bCheckReading
	)
{
	BOOL bRet = RETURNED_C2_ERROR_NO_1ST;
	if (*pExecType != gd) {
		if (pDisc->SCSI.nFirstLBAof2ndSession != -1) {
			if (pExtArg->bReverse) {
				if (pDisc->SCSI.nFirstLBAof2ndSession == nLBA + 1) {
					OutputInfoLogA(
						"Skip from Leadout of Session 1 [%d, %#x] to Leadin of Session 2 [%d, %#x]\n",
						pDisc->SCSI.nFirstLBAofLeadout, pDisc->SCSI.nFirstLBAofLeadout, 
						pDisc->SCSI.nFirstLBAof2ndSession - 1, pDisc->SCSI.nFirstLBAof2ndSession - 1);
					pPrevSubQ->nAbsoluteTime = nLBA - 11400 - 150;
					return RETURNED_SKIP_LBA;
				}
			}
			else {
				if (pDisc->MAIN.nFixFirstLBAofLeadout == nLBA) {
					OutputInfoLogA(
						"Skip from Leadout of Session 1 [%d, %#x] to Leadin of Session 2 [%d, %#x]\n",
						pDisc->SCSI.nFirstLBAofLeadout, pDisc->SCSI.nFirstLBAofLeadout, 
						pDisc->SCSI.nFirstLBAof2ndSession - 1, pDisc->SCSI.nFirstLBAof2ndSession - 1);
					if (pDisc->MAIN.nCombinedOffset > 0) {
						pPrevSubQ->nAbsoluteTime = 
							nLBA + 11400 + 150 - pDisc->MAIN.nAdjustSectorNum - 1;
					}
					else if (pDisc->MAIN.nCombinedOffset < 0) {
						pPrevSubQ->nAbsoluteTime = 
							nLBA + 11400 + 150 + pDisc->MAIN.nAdjustSectorNum;
					}
					return RETURNED_SKIP_LBA;
				}
			}
		}
	}
	if (pExtArg->bFua/* || uiC2ErrorLBACnt > 0*/) {
		ReadCDForFlushingDriveCache(pDevice, nLBA);
	}
	bRet = ExecReadCD(pExecType, pDevice, pDisc, nLBA, lpBuf, lpCmd, pPrevSubQ, fpImg, bCheckReading);
	// Drive of buffer Size under 256 is very slow.
	if (pDevice->wDriveBufSize > 256) {
		bRet = ExecReadCD(pExecType, pDevice, pDisc, nLBA + 1, lpBufNext, lpCmd, pPrevSubQ, fpImg, bCheckReading);
	}
	if (bRet == RETURNED_C2_ERROR_NO_1ST && !bCheckReading) {
		if (pC2ErrorPerSector && pDevice->bC2ErrorData && pExtArg->bC2) {
			bRet = CheckC2Error(pC2Error,
				pC2ErrorPerSector, pDevice, lpBuf, uiC2ErrorLBACnt);
		}
		AlignRowSubcode(lpBuf + pDevice->TRANSFER.dwBufSubOffset, lpSubcode);
		if (pDevice->wDriveBufSize > 256) {
			AlignRowSubcode(lpBufNext + pDevice->TRANSFER.dwBufSubOffset, lpNextSubcode);
		}
	}
	return bRet;
}

BOOL ReadCDForCheckingIndex0InTrack1(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	)
{
	BOOL bRet = TRUE;
	if (pDevice->byPlexType == PLEX_DRIVE_TYPE::PX760 ||
		pDevice->byPlexType == PLEX_DRIVE_TYPE::PX755 ||
		pDevice->byPlexType == PLEX_DRIVE_TYPE::PX716 ||
		pDevice->byPlexType == PLEX_DRIVE_TYPE::PX716AL ||
		pDevice->byPlexType == PLEX_DRIVE_TYPE::PX712 ||
		pDevice->byPlexType == PLEX_DRIVE_TYPE::PX708 ||
		pDevice->byPlexType == PLEX_DRIVE_TYPE::PX708A2 ||
		pDevice->byPlexType == PLEX_DRIVE_TYPE::Premium2 ||
		pDevice->byPlexType == PLEX_DRIVE_TYPE::PXW5232 ||
		pDevice->byPlexType == PLEX_DRIVE_TYPE::PXW5224) {
		pDisc->SUB.bIndex0InTrack1 = TRUE;
	}
	else {
		OutputString(
			_T("This drive doesn't support to rip from 00:00:00 to 00:01:74 AMSF. /p option is ignored\n"));
		pExtArg->bPre = FALSE;
	}
	return bRet;
}

BOOL ReadCDForCheckingSubPtoW(
	PDEVICE pDevice,
	PEXEC_TYPE pExecType
	)
{
	BOOL bRet = TRUE;
	if (pDevice->byPlexType && *pExecType == cd) {
		LPBYTE pBuf = NULL;
		try {
			pBuf = (LPBYTE)calloc(
				CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE + pDevice->AlignmentMask, sizeof(BYTE));
			if (!pBuf) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevice, pBuf);
			CDB::_READ_CD cdb = { 0 };
			BYTE byScsiStatus = 0;
			SetReadCDCommand(pDevice, NULL, &cdb, READ_CD_FLAG::All, 1, READ_CD_FLAG::Raw, TRUE);
			if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
				CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			else {
				BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
				AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
				OutputDiscLogA("====== Check SubP to W, OperationCode: 0xbe, Subcode: 1(=Raw) ======\n");
				OutputMmcCDSub96Align(lpSubcode, 0);
			}
			SetReadCDCommand(pDevice, NULL, &cdb, READ_CD_FLAG::All, 1, READ_CD_FLAG::Pack, TRUE);
			if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
				CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			else {
				BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
				AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
				OutputDiscLogA("====== Check SubP to W, OperationCode: 0xbe, Subcode: 4(=Pack) ======\n");
				OutputMmcCDSub96Align(lpSubcode, 0);
			}
			CDB::_PLXTR_READ_CDDA cdb2 = { 0 };
			SetReadD8Command(pDevice, NULL, &cdb2, 1, TRUE, TRUE);
			cdb2.SubCode = READ_D8_FLAG::MainPack;
			if (!ScsiPassThroughDirect(pDevice, &cdb2, CDB12GENERIC_LENGTH, lpBuf,
				CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			else {
				BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
				AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
				OutputDiscLogA("====== Check SubP to W, OperationCode: 0xd8, Subcode: 0x02(=Pack) ======\n");
				OutputMmcCDSub96Align(lpSubcode, 0);
			}
			cdb2.SubCode = READ_D8_FLAG::Raw;
			if (!ScsiPassThroughDirect(pDevice, &cdb2, CDB12GENERIC_LENGTH, lpBuf,
				CD_RAW_READ_SUBCODE_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			else {
				BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
				AlignRowSubcode(lpBuf, lpSubcode);
				OutputDiscLogA("====== Check SubP to W, OperationCode: 0xd8, Subcode: 0x03(=Pack) ======\n");
				OutputMmcCDSub96Align(lpSubcode, 0);
			}
			cdb2.SubCode = READ_D8_FLAG::MainC2Raw;
			if (!ScsiPassThroughDirect(pDevice, &cdb2, CDB12GENERIC_LENGTH, lpBuf,
				CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			else {
				BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
				AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE + CD_RAW_READ_C2_294_SIZE, lpSubcode);
				OutputDiscLogA("====== Check SubP to W, OperationCode: 0xd8, Subcode: 0x08(=Raw) ======\n");
				OutputMmcCDSub96Align(lpSubcode, 0);
			}
		}
		catch (BOOL bErr) {
			bRet = bErr;
		}
		FreeAndNull(pBuf);
	}
	return bRet;
}

BOOL ReadCDForCheckingCDG(
	PDEVICE pDevice,
	PDISC pDisc
	)
{
	BOOL bRet = TRUE;
	LPBYTE pBuf = NULL;
	try {
		pBuf = (LPBYTE)calloc(
			CD_RAW_SECTOR_WITH_SUBCODE_SIZE + pDevice->AlignmentMask, sizeof(BYTE));
		if (!pBuf) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevice, pBuf);
		CDB::_READ_CD cdb = { 0 };
		SetReadCDCommand(pDevice, NULL, &cdb, READ_CD_FLAG::All, 1, READ_CD_FLAG::Raw, TRUE);

		for (BYTE i = 0; i < pDisc->SCSI.toc.LastTrack; i++) {
			if ((pDisc->SCSI.toc.TrackData[i].Control & AUDIO_DATA_TRACK) == 0) {
				INT nTmpLBA =
					pDisc->SCSI.lpFirstLBAListOnToc[i] + 100;
				cdb.StartingLBA[0] = HIBYTE(HIWORD(nTmpLBA));
				cdb.StartingLBA[1] = LOBYTE(HIWORD(nTmpLBA));
				cdb.StartingLBA[2] = HIBYTE(LOWORD(nTmpLBA));
				cdb.StartingLBA[3] = LOBYTE(LOWORD(nTmpLBA));

				BYTE byScsiStatus = 0;
				if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
					CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
					|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
					throw FALSE;
				}
				else {
					BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
					AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
					BYTE lpSubcodeOrg[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
					memcpy(lpSubcodeOrg, lpBuf + CD_RAW_SECTOR_SIZE, CD_RAW_READ_SUBCODE_SIZE);
					OutputDiscLogA("======== Check CD+G for track[%02d] ========\n", i + 1);
					OutputMmcCDSub96Align(lpSubcode, nTmpLBA);

					SUB_R_TO_W scRW[4] = { 0 };
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
						pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::Fill;
						OutputDiscLogA("Track[%02u]: RtoW all 0xff\n", i + 1);
					}
					else if (bCDG && nRtoW > 0 && nRtoW != 0x200) {
						pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::CDG;
						OutputDiscLogA("Track[%02u]: CD+G\n", i + 1);
					}
					else if (bCDEG && nRtoW > 0 && nRtoW != 0x200) {
						pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::CDG;
						OutputDiscLogA("Track[%02u]: CD+EG\n", i + 1);
					}
					else {
						pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::Zero;
						OutputDiscLogA("Track[%02u]: Nothing\n", i + 1);
					}
				}
			}
			else {
				pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::Zero;
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
	PDEVICE pDevice,
	PEXT_ARG pExtArg,
	PDRIVE_DATA_ORDER pOrder
	)
{
	UINT newBufLen = 
		CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE + pDevice->TRANSFER.dwAdditionalBufLen;
	LPBYTE pBuf = (LPBYTE)calloc(newBufLen + pDevice->AlignmentMask, sizeof(BYTE));
	if (!pBuf) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevice, pBuf);

	BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
	if (pDevice->byPlexType) {
		CDB::_PLXTR_READ_CDDA cdb = { 0 };
		SetReadD8Command(pDevice, pExtArg, &cdb, 1, TRUE, FALSE);
		memcpy(lpCmd, &cdb, sizeof(lpCmd));
	}
	else {
		// non plextor && support scrambled ripping
		CDB::_READ_CD cdb = { 0 };
		SetReadCDCommand(pDevice, pExtArg, &cdb, READ_CD_FLAG::CDDA, 1, READ_CD_FLAG::Raw, FALSE);
		memcpy(lpCmd, &cdb, sizeof(lpCmd));
	}

	BOOL bRet = TRUE;
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pDevice, lpCmd, CDB12GENERIC_LENGTH, lpBuf, 
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
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszPath,
	READ_CD_FLAG::EXPECTED_SECTOR_TYPE flg
	)
{
	BOOL bRet = TRUE;
	if (pDisc->MAIN.nCombinedOffset < 0) {
		OutputString(_T("Checking reading lead-in\n"));
		bRet = ReadCDPartial(pExecType, pExtArg, pDevice,
			pDisc, pszPath, -1, 0, flg, TRUE);
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
	else if (0 < pDisc->MAIN.nCombinedOffset) {
		OutputString(_T("Checking reading lead-out\n"));
		bRet = ReadCDPartial(pExecType, pExtArg, pDevice, pDisc, pszPath, 
			pDisc->SCSI.nAllLength, pDisc->SCSI.nAllLength, flg, TRUE);
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
	PDEVICE pDevice,
	INT nLBA
	)
{
#if 1
	CDB::_READ12 cdb = { 0 };
	cdb.OperationCode = SCSIOP_READ12;
	cdb.ForceUnitAccess = CDB_FORCE_MEDIA_ACCESS;
	cdb.LogicalUnitNumber = pDevice->address.Lun;
	cdb.LogicalBlock[0] = HIBYTE(HIWORD(nLBA + 1));
	cdb.LogicalBlock[1] = LOBYTE(HIWORD(nLBA + 1));
	cdb.LogicalBlock[2] = HIBYTE(LOWORD(nLBA + 1));
	cdb.LogicalBlock[3] = LOBYTE(LOWORD(nLBA + 1));
#else
	CDB::_PLXTR_READ_CDDA cdb = { 0 };
	cdb.OperationCode = SCSIOP_PLEX_READ_CD;
	cdb.Reserved0 = CDB_FORCE_MEDIA_ACCESS;
	cdb.LogicalUnitNumber = pDevice->address.Lun;
	cdb.LogicalBlockByte0 = HIBYTE(HIWORD(nLBA + 1));
	cdb.LogicalBlockByte1 = LOBYTE(HIWORD(nLBA + 1));
	cdb.LogicalBlockByte2 = HIBYTE(LOWORD(nLBA + 1));
	cdb.LogicalBlockByte3 = LOBYTE(LOWORD(nLBA + 1));
	cdb.TransferBlockByte3 = 1;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH, NULL,
		0, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	return TRUE;
}

BOOL ReadCDForVolumeDescriptor(
	PDEVICE pDevice,
	PCDROM_TOC pToc,
	INT nFirstLBAofDataTrack
	)
{
	BOOL bRet = TRUE;
	LPBYTE pBuf = (LPBYTE)calloc(
		CD_RAW_SECTOR_SIZE + pDevice->AlignmentMask, sizeof(BYTE));
	if (!pBuf) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevice, pBuf);
	CDB::_READ_CD cdb = { 0 };
	SetReadCDCommand(pDevice, NULL, &cdb, READ_CD_FLAG::All, 1, READ_CD_FLAG::SubNone, TRUE);

	BYTE byScsiStatus = 0;
	try {
		// almost data track disc
		if ((pToc->TrackData[0].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
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
				RecuseDir(pDevice, &cdb, "/", rootDir);
			}
			else {
				cdb.StartingLBA[3] = 1;
				if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
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
					if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
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
			if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
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
			if (!ScsiPassThroughDirect(pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
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
	PDEVICE pDevice,
	PDISC pDisc,
	PC2_ERROR pC2Error,
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	UINT uiC2ErrorLBACnt,
	LPBYTE lpBuf,
	LPBYTE lpBufNext,
	LPBYTE lpCmd,
	PSUB_Q pPrevSubQ,
	LPBYTE lpSubcode,
	LPBYTE lpNextSubcode,
	FILE* fpImg
	)
{
	BOOL bProcessRet = TRUE;
	if (uiC2ErrorLBACnt > 0) {
		OutputString(
			_T("\nChange reading speed: %ux\n"), pExtArg->dwRereadSpeedNum);
		SetCDSpeed(pDevice, pExtArg->dwRereadSpeedNum);
	}
	UINT uiCnt = 0;

	while (uiC2ErrorLBACnt > 0) {
		if (uiCnt == pExtArg->dwMaxRereadNum) {
			OutputString(_T("Reread reached max: %u\n"), uiCnt);
			bProcessRet = RETURNED_FALSE;
			break;
		}
		UINT uiC2ErrorLBACntBackup = uiC2ErrorLBACnt;
		uiC2ErrorLBACnt = 0;
		SetC2ErrorBackup(pC2ErrorPerSector, 
			uiC2ErrorLBACntBackup, pDevice->TRANSFER.dwAllBufLen);

		UINT i = 0;
		for (INT nLBA = pC2ErrorPerSector[0].nErrorLBANumBackup; i < uiC2ErrorLBACntBackup; i++) {
			OutputString(
				_T("\rReread times %4u, ErrSectorNum %4u/%4u"), 
				uiCnt + 1, i + 1, uiC2ErrorLBACntBackup);
			nLBA = pC2ErrorPerSector[i].nErrorLBANumBackup;
			bProcessRet = ProcessReadCD(pExecType, pExtArg, pDevice, pDisc, 
				pC2Error, pC2ErrorPerSector, uiC2ErrorLBACnt, nLBA, lpBuf,
				lpBufNext, lpCmd, pPrevSubQ, lpSubcode, lpNextSubcode, fpImg, FALSE);

			if (bProcessRet == RETURNED_C2_ERROR_EXIST) {
				SetAndOutputc2ErrorDataPerSector(pC2Error, pC2ErrorPerSector,
					nLBA, pDevice->TRANSFER.dwAllBufLen, uiC2ErrorLBACnt);
				uiC2ErrorLBACnt++;
			}
			else if (bProcessRet == RETURNED_C2_ERROR_NO_1ST) {
				if (pC2ErrorPerSector[i].bErrorFlagBackup == RETURNED_C2_ERROR_EXIST ||
					pC2ErrorPerSector[i].bErrorFlagBackup == RETURNED_C2_ERROR_NO_BUT_BYTE_ERROR) {
					SetAndOutputC2NoErrorData(pC2ErrorPerSector, lpBuf, nLBA,
						pDevice->TRANSFER.dwAllBufLen, uiC2ErrorLBACnt);
					if (pC2ErrorPerSector[i].bErrorFlagBackup == RETURNED_C2_ERROR_NO_BUT_BYTE_ERROR) {
						if (CheckByteError(pC2ErrorPerSector, lpBuf, i)) {
							SetAndOutputC2NoErrorByteErrorData(pC2ErrorPerSector, lpBuf, nLBA,
								pDevice->TRANSFER.dwAllBufLen, uiC2ErrorLBACnt);
							uiC2ErrorLBACnt++;
						}
						else {
							LONG lPos = LONG(CD_RAW_SECTOR_SIZE * nLBA - pDisc->MAIN.nCombinedOffset);
							LONG lEndPos = lPos + CD_RAW_SECTOR_SIZE - 1;
							OutputErrorLogA(
								"LBA[%06d, %#07x], PrevBuf vs. PresentBuf matched: Fixed Main data from [%d, %#x] to [%d, %#x]\n",
								nLBA, nLBA, lPos, lPos, lEndPos, lEndPos);
							fseek(fpImg, lPos, SEEK_SET);
							WriteMainChannel(pExtArg, pDisc, lpBuf, nLBA, fpImg);
						}
					}
					else {
						uiC2ErrorLBACnt++;
					}
				}
				else {
					if (CheckByteError(pC2ErrorPerSector, lpBuf, i)) {
						SetAndOutputC2NoErrorByteErrorData(pC2ErrorPerSector, lpBuf, nLBA,
							pDevice->TRANSFER.dwAllBufLen, uiC2ErrorLBACnt);
						uiC2ErrorLBACnt++;
					}
					else {
						LONG lPos = LONG(CD_RAW_SECTOR_SIZE * nLBA - pDisc->MAIN.nCombinedOffset);
						LONG lEndPos = lPos + CD_RAW_SECTOR_SIZE - 1;
						OutputErrorLogA(
							"LBA[%06d, %#07x], Reread data matched: Fixed Main data from [%d, %#x] to [%d, %#x]\n",
							nLBA, nLBA, lPos, lPos, lEndPos, lEndPos);
						fseek(fpImg, lPos, SEEK_SET);
						// Write track to scrambled again
						WriteMainChannel(pExtArg, pDisc, lpBuf, nLBA, fpImg);
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
		OutputString(_T("No C2 errors\n"));
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
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszPath,
	FILE* fpCcd
	)
{
	if (pDisc->SCSI.toc.FirstTrack < 1 || 99 < pDisc->SCSI.toc.FirstTrack ||
		pDisc->SCSI.toc.LastTrack < 1 || 99 < pDisc->SCSI.toc.LastTrack) {
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
	LPBYTE pBufNext = NULL;
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector = NULL;
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

		size_t dwTrackAllocSize = (size_t)pDisc->SCSI.toc.LastTrack + 1;
		if (NULL == (pDisc->SUB.lpFirstLBAListOnSub = 
			(LPINT*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (pDisc->SUB.lpFirstLBAListOnSubSync = 
			(LPINT*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (pDisc->SUB.lpFirstLBAListOfDataTrackOnSub = 
			(LPINT)calloc(dwTrackAllocSize, sizeof(INT)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (pDisc->SUB.lpLastLBAListOfDataTrackOnSub = 
			(LPINT)calloc(dwTrackAllocSize, sizeof(INT)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (pDisc->SUB.lpISRCList = 
			(LPBOOL)calloc(dwTrackAllocSize, sizeof(BOOL)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (pDisc->SUB.lpCtlList = 
			(LPBYTE)calloc(dwTrackAllocSize, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (pDisc->SUB.lpModeList = 
			(LPBYTE)calloc(dwTrackAllocSize, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (pDisc->SUB.lpEndCtlList = 
			(LPBYTE)calloc(dwTrackAllocSize, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}

		size_t dwIndexAllocSize = (size_t)MAXIMUM_NUMBER_INDEXES * sizeof(UINT_PTR);
		for (INT h = 0; h < pDisc->SCSI.toc.LastTrack + 1; h++) {
			if (NULL == (pDisc->SUB.lpFirstLBAListOnSub[h] = (LPINT)malloc(dwIndexAllocSize))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			FillMemory(pDisc->SUB.lpFirstLBAListOnSub[h], dwIndexAllocSize, -1);
			if (NULL == (pDisc->SUB.lpFirstLBAListOnSubSync[h] = (LPINT)malloc(dwIndexAllocSize))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			FillMemory(pDisc->SUB.lpFirstLBAListOnSubSync[h], dwIndexAllocSize, -1);
			pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[h] = -1;
			pDisc->SUB.lpLastLBAListOfDataTrackOnSub[h] = -1;
		}

		SetCDTransferData(pDevice, DRIVE_DATA_ORDER::C2None);
		C2_ERROR pC2Error = { 0 };
		if (pDevice->bC2ErrorData && pExtArg->bC2) {
			if (NULL == (fpC2 = CreateOrOpenFileW(
				pszPath, NULL, NULL, NULL, NULL, _T(".c2"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			SetCDTransferData(pDevice, DRIVE_DATA_ORDER::MainC2Sub);
			DRIVE_DATA_ORDER dataOrder = DRIVE_DATA_ORDER::MainC2Sub;

			if (!InitC2ErrorData(pExtArg, pDisc, &pC2Error,
				&pC2ErrorPerSector, pDevice->TRANSFER.dwAllBufLen)) {
				throw FALSE;
			}
			if (!ReadCDForCheckingByteOrder(pDevice, pExtArg, &dataOrder)) {
				Terminatec2ErrorDataPerSector(pDevice, pExtArg, &pC2ErrorPerSector);
				pDevice->bC2ErrorData = FALSE;
				SetCDTransferData(pDevice, DRIVE_DATA_ORDER::C2None);
			}
			if (dataOrder == DRIVE_DATA_ORDER::MainSubC2) {
				OutputDriveLogA(
					"Byte order of this drive is main + sub + c2\n");
				SetCDTransferData(pDevice, DRIVE_DATA_ORDER::MainSubC2);
			}
			else {
				OutputDriveLogA(
					"Byte order of this drive is main + c2 + sub\n");
			}
		}
#ifdef _DEBUG
		OutputString(
			_T("TransferLen %u, BufLen %ubyte, AdditionalBufLen %ubyte, AllBufLen %ubyte, BufC2Offset %ubyte, BufSubOffset %ubyte\n"),
			pDevice->TRANSFER.uiTransferLen, pDevice->TRANSFER.dwBufLen,
			pDevice->TRANSFER.dwAdditionalBufLen, pDevice->TRANSFER.dwAllBufLen, 
			pDevice->TRANSFER.dwBufC2Offset, pDevice->TRANSFER.dwBufSubOffset);
#endif
		// store main + (c2) + sub data all
		if (NULL == (pBuf = (LPBYTE)calloc(
			pDevice->TRANSFER.dwAllBufLen + pDevice->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevice, pBuf);

		if (NULL == (pBufNext = (LPBYTE)calloc(pDevice->TRANSFER.dwBufLen + 
			pDevice->TRANSFER.dwAdditionalBufLen + pDevice->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBufNext = (LPBYTE)ConvParagraphBoundary(pDevice, pBufNext);

		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		_TCHAR szSubCode[5] = { 0 };
		if (pDevice->byPlexType && !pDisc->SCSI.bAudioOnly) {
			CDB::_PLXTR_READ_CDDA cdb = { 0 };
			SetReadD8Command(pDevice, pExtArg, &cdb, 1, TRUE, FALSE);
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
			SetReadCDCommand(pDevice, pExtArg, &cdb,
				READ_CD_FLAG::CDDA, 1, READ_CD_FLAG::Raw, FALSE);
			if (cdb.SubChannelSelection == READ_CD_FLAG::Raw) {
				_tcsncpy(szSubCode, _T("Raw"), 3);
			}
			memcpy(lpCmd, &cdb, sizeof(lpCmd));
		}
		OutputString(_T("Read command: %#x, Subcode reading mode: %s\n"), lpCmd[0], szSubCode);

		BYTE byScsiStatus = 0;
		BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		SUB_Q prevSubQ = { 0 };

		lpCmd[2] = HIBYTE(HIWORD(-1));
		lpCmd[3] = LOBYTE(HIWORD(-1));
		lpCmd[4] = HIBYTE(LOWORD(-1));
		lpCmd[5] = LOBYTE(LOWORD(-1));
		if (!ScsiPassThroughDirect(pDevice, lpCmd, CDB12GENERIC_LENGTH, lpBuf,
			pDevice->TRANSFER.dwAllBufLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw FALSE;
		}
		AlignRowSubcode(lpBuf + pDevice->TRANSFER.dwBufSubOffset, lpSubcode);
		SetSubQDataFromReadCD(pDisc, &prevSubQ, lpBuf, lpSubcode);
		if (prevSubQ.byAdr != ADR_ENCODES_CURRENT_POSITION) {
			// 1552 Tenka Tairan
			prevSubQ.byAdr = ADR_ENCODES_CURRENT_POSITION;
			prevSubQ.byTrackNum = 1;
		}
		if (prevSubQ.nAbsoluteTime != 149) {
			// 1552 Tenka Tairan
			prevSubQ.nAbsoluteTime = 149;
		}

		for (INT p = 0; p < pDisc->SCSI.toc.LastTrack; p++) {
			lpCmd[2] = HIBYTE(HIWORD(pDisc->SCSI.lpFirstLBAListOnToc[p]));
			lpCmd[3] = LOBYTE(HIWORD(pDisc->SCSI.lpFirstLBAListOnToc[p]));
			lpCmd[4] = HIBYTE(LOWORD(pDisc->SCSI.lpFirstLBAListOnToc[p]));
			lpCmd[5] = LOBYTE(LOWORD(pDisc->SCSI.lpFirstLBAListOnToc[p]));
			if (!ScsiPassThroughDirect(pDevice, lpCmd, CDB12GENERIC_LENGTH, lpBuf, 
				pDevice->TRANSFER.dwAllBufLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			AlignRowSubcode(lpBuf + pDevice->TRANSFER.dwBufSubOffset, lpSubcode);
			pDisc->SUB.lpEndCtlList[p] = (BYTE)((lpSubcode[12] >> 4) & 0x0F);
		}

		SetCDOffsetData(pDisc, 0, pDisc->SCSI.nAllLength);
		BYTE byCurrentTrackNum = pDisc->SCSI.toc.FirstTrack;
		INT nFirstLBAForSub = 0;
		INT nFirstLBA = pDisc->MAIN.nOffsetStart;
		INT nLastLBA = pDisc->SCSI.nAllLength + pDisc->MAIN.nOffsetEnd;
		INT nLBA = nFirstLBA;	// This value switches by /r option.

		if (pExtArg->bReverse) {
			SetCDOffsetData(pDisc, nLastLBA, pDisc->SCSI.nFirstLBAofDataTrack);
			byCurrentTrackNum = pDisc->SCSI.byLastDataTrack;
			nFirstLBA = pDisc->SCSI.nFirstLBAofDataTrack;
			nLastLBA = pDisc->SCSI.nLastLBAofDataTrack;
			nLBA = nLastLBA;
			if (pDisc->MAIN.nCombinedOffset > 0) {
				prevSubQ.nAbsoluteTime = 149 + nLastLBA;
			}
			else if (pDisc->MAIN.nCombinedOffset < 0) {
				prevSubQ.nAbsoluteTime = 150 + nLastLBA + pDisc->MAIN.nAdjustSectorNum - 1;
			}
			else {
				prevSubQ.nAbsoluteTime = 149 + nLastLBA;
			}

			prevSubQ.byCtl = pDisc->SUB.lpEndCtlList[pDisc->SCSI.byLastDataTrack - 1];
			prevSubQ.byAdr = ADR_ENCODES_CURRENT_POSITION;
			prevSubQ.byTrackNum = pDisc->SCSI.byLastDataTrack;
			prevSubQ.byIndex = pDisc->MAIN.nOffsetStart < 0 ? (BYTE)0 : (BYTE)1;
			prevSubQ.byMode = DATA_BLOCK_MODE0;
		}
		else if (pDisc->SUB.bIndex0InTrack1) {
			nFirstLBAForSub = -5000;
			nFirstLBA = -5000;
			nLBA = nFirstLBA;
			pDisc->MAIN.nOffsetStart = -5000;
		}
		// init end
#ifndef _DEBUG
		FlushLog();
#endif
		BYTE lpNextSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		SUB_Q prevPrevSubQ = { 0 };
		BOOL bCatalog = FALSE;
		UINT uiC2ErrorLBACnt = 0;
		BOOL bReadOK = pDisc->SUB.bIndex0InTrack1 ? FALSE : TRUE;

		while (nFirstLBA <= nLastLBA) {
			BOOL bProcessRet = ProcessReadCD(pExecType, pExtArg, pDevice,
				pDisc, &pC2Error, pC2ErrorPerSector, uiC2ErrorLBACnt, 
				nLBA, lpBuf, lpBufNext, lpCmd, &prevSubQ, lpSubcode, lpNextSubcode, fpImg, FALSE);
//#define C2TEST
#if defined C2TEST
			if (nLBA == 100) {
				memset(lpBuf, 0xff, 2352);
				bProcessRet = RETURNED_C2_ERROR_EXIST;
			}
#endif
			if (pC2ErrorPerSector && bProcessRet == RETURNED_C2_ERROR_EXIST) {
				INT nTmpLBA = nLBA/* + pC2Error.cSlideSectorNum*/;
				OutputErrorString(
					_T(" Detected C2 error. LBA[%06d, %#07x]\n"), nTmpLBA, nTmpLBA);
				SetAndOutputc2ErrorDataPerSector(&pC2Error, pC2ErrorPerSector,
					nTmpLBA, pDevice->TRANSFER.dwAllBufLen, uiC2ErrorLBACnt);
				uiC2ErrorLBACnt++;
				if (uiC2ErrorLBACnt == pExtArg->dwMaxC2ErrorNum) {
					OutputErrorString(_T("C2 error Max: %u\n"), uiC2ErrorLBACnt);
					throw FALSE;
				}
			}
			else if (bProcessRet == RETURNED_SKIP_LBA) {
				if (pExtArg->bReverse) {
					nLBA = pDisc->SCSI.nFirstLBAof2ndSession - 11400;
				}
				else {
					nLBA = pDisc->MAIN.nFixFirstLBAof2ndSession - 1;
					nFirstLBA = nLBA;
				}
			}
			else if (bProcessRet == RETURNED_FALSE) {
				throw FALSE;
			}
			if (bProcessRet != RETURNED_CONTINUE &&
				bProcessRet != RETURNED_SKIP_LBA) {
				SUB_Q subQ = { 0 };
				SetSubQDataFromReadCD(pDisc, &subQ, lpBuf, lpSubcode);
				if (pDisc->SUB.bIndex0InTrack1 && -5000 <= nLBA && nLBA <= -76) {
					if (subQ.byTrackNum == 1 && subQ.nAbsoluteTime == 0) {
						prevSubQ.nRelativeTime = subQ.nRelativeTime + 1;
						prevSubQ.nAbsoluteTime = -1;
						pDisc->MAIN.nFixStartLBA = nLBA;
						bReadOK = TRUE;
						if (pDisc->MAIN.nAdjustSectorNum < 0 ||
							1 < pDisc->MAIN.nAdjustSectorNum) {
							for (INT i = 0; i < abs(pDisc->MAIN.nAdjustSectorNum) * CD_RAW_SECTOR_SIZE; i++) {
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
					SUB_Q nextSubQ = { 0 };
					SetSubQDataFromReadCD(pDisc, &nextSubQ,
						lpBuf + pDevice->TRANSFER.dwBufLen, lpNextSubcode);
					if (nFirstLBAForSub <= nLBA && nLBA < pDisc->SCSI.nAllLength) {
						if (nLBA < pDisc->SCSI.nFirstLBAofLeadout ||
							pDisc->SCSI.nFirstLBAof2ndSession <= nLBA) {
							BOOL bLibCrypt = CheckLibCryptSector(pExtArg->bLibCrypt, nLBA);
							if (!pExtArg->bReverse) {
								CheckAndFixSubChannel(pExtArg, pDisc, lpSubcode,
									&nextSubQ, &subQ, &prevSubQ, &prevPrevSubQ,
									&byCurrentTrackNum, &bCatalog, nLBA, bLibCrypt);
								BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
								// fix raw subchannel
								AlignColumnSubcode(lpSubcode, lpSubcodeRaw);
#if 0
								OutputMmcCDSub96Align(lpSubcode, nLBA->fpError);
#endif
								WriteSubChannel(pDisc, lpSubcodeRaw,
									lpSubcode, nLBA, byCurrentTrackNum, fpSub, fpParse);
							}
							PreserveTrackAttribution(pExtArg, pDisc, nLBA,
								&byCurrentTrackNum, &subQ, &prevSubQ);
							UpdateSubQData(&subQ, &prevSubQ, &prevPrevSubQ, bLibCrypt);
						}
					}
					CheckMainChannel(pDisc, lpBuf, &subQ, 
						&prevSubQ, &prevPrevSubQ, byCurrentTrackNum, nLBA);
					// 4th:Write track to scrambled
					WriteMainChannel(pExtArg, pDisc, lpBuf, nLBA, fpImg);
					if (pDevice->bC2ErrorData && pExtArg->bC2) {
						WriteC2(pDisc, lpBuf + pDevice->TRANSFER.dwBufC2Offset, nLBA, fpC2);
						// TODO: exist an offset?
//						fwrite(lpBuf + pDevice->TRANSFER.dwBufC2Offset,
//							sizeof(BYTE), CD_RAW_READ_C2_294_SIZE, fpC2);
					}
				}
			}

			if (pExtArg->bReverse) {
				OutputString(_T("\rCreating img (LBA) %6d/%6d"),
					nLBA, pDisc->SCSI.nFirstLBAofDataTrack);
				nLBA--;
			}
			else {
				OutputString(_T("\rCreating img (LBA) %6d/%6d"),
					nFirstLBA - 1, pDisc->SCSI.nAllLength - 1);
				if (nFirstLBA == -76) {
					nLBA = nFirstLBA;
				}
				nLBA++;
			}
			nFirstLBA++;
		}
		OutputString(_T("\n"));
		if (pDevice->bC2ErrorData && pExtArg->bC2) {
			FcloseAndNull(fpC2);
		}
		FcloseAndNull(fpSub);
		FcloseAndNull(fpParse);
#ifndef _DEBUG
		FlushLog();
#endif

		if (pDisc->SCSI.toc.FirstTrack == pDisc->SCSI.toc.LastTrack) {
			pDisc->SUB.lpFirstLBAListOnSub[0][1] = pDisc->SCSI.lpLastLBAListOnToc[0];
		}
		for (INT i = pDisc->SCSI.toc.FirstTrack; i <= pDisc->SCSI.toc.LastTrack; i++) {
			BOOL bErr = FALSE;
			LONG lLine = 0;
			if (pDisc->SUB.lpFirstLBAListOnSub[i - 1][1] == -1) {
				bErr = TRUE;
				lLine = __LINE__;
			}
			else if ((pDisc->SCSI.toc.TrackData[i - 1].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				if (pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[i - 1] == -1) {
					bErr = TRUE;
					lLine = __LINE__;
				}
				else if (pDisc->SUB.lpLastLBAListOfDataTrackOnSub[i - 1] == -1) {
					bErr = TRUE;
					lLine = __LINE__;
				}
			}
			if (bErr) {
				OutputErrorString(
					_T("[L:%d] Internal error. Failed to analyze the subchannel. Track[%02u]/[%02u]\n"),
					lLine, i, pDisc->SCSI.toc.LastTrack);
				throw FALSE;
			}
		}
		// c2 error: reread sector 
		if (pDevice->bC2ErrorData && pExtArg->bC2) {
			if (!ReadCDForRereadingSector(pExecType, pExtArg, pDevice, pDisc,
				&pC2Error, pC2ErrorPerSector, uiC2ErrorLBACnt, lpBuf,
				lpBufNext, lpCmd, &prevSubQ, lpSubcode, lpNextSubcode, fpImg)) {
				throw FALSE;
			}
		}
		FcloseAndNull(fpImg);

		if (!pExtArg->bReverse) {
			OutputMmcTocWithPregap(pDisc);
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
			LONG lSeek = CD_RAW_SECTOR_SIZE - (LONG)pDisc->MAIN.uiMainDataSlideSize;
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
			fseek(fpImg_r, -(LONG)pDisc->MAIN.uiMainDataSlideSize, SEEK_CUR);
			fread(rBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpImg_r);
			fwrite(rBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpImg);
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
		if (pDisc->SCSI.bAudioOnly) {
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
			DescrambleMainChannel(pExtArg, pDisc, fpTbl, fpImg);
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
				_T("wb"), pDisc->SCSI.byFirstDataTrack, pDisc->SCSI.byFirstDataTrack);
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
			if (!CreateBinCueCcd(pDisc, pszPath,
				pszImgName, bCatalog, fpImg, fpCue, fpCueForImg, fpCcd)) {
				throw FALSE;
			}
		}
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpImg);
	if (pDevice->bC2ErrorData && pExtArg->bC2) {
		FcloseAndNull(fpC2);
	}
	FcloseAndNull(fpCueForImg);
	FcloseAndNull(fpCue);
	FcloseAndNull(fpParse);
	FcloseAndNull(fpSub);
	FcloseAndNull(fpTbl);
	for (INT h = 0; h < pDisc->SCSI.toc.LastTrack + 1; h++) {
		if (pDisc->SUB.lpFirstLBAListOnSub) {
			FreeAndNull(pDisc->SUB.lpFirstLBAListOnSub[h]);
		}
		if (pDisc->SUB.lpFirstLBAListOnSubSync) {
			FreeAndNull(pDisc->SUB.lpFirstLBAListOnSubSync[h]);
		}
	}
	FreeAndNull(pDisc->SUB.lpFirstLBAListOnSub);
	FreeAndNull(pDisc->SUB.lpFirstLBAListOnSubSync);
	FreeAndNull(pDisc->SUB.lpFirstLBAListOfDataTrackOnSub);
	FreeAndNull(pDisc->SUB.lpLastLBAListOfDataTrackOnSub);
	FreeAndNull(pDisc->SUB.lpCtlList);
	FreeAndNull(pDisc->SUB.lpEndCtlList);
	FreeAndNull(pDisc->SUB.lpISRCList);
	FreeAndNull(pDisc->SUB.lpModeList);
	FreeAndNull(pBuf);
	FreeAndNull(pBufNext);
	Terminatec2ErrorDataPerSector(pDevice, pExtArg, &pC2ErrorPerSector);

	return bRet;
}

BOOL ReadCDForSearchingOffset(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	)
{
	BOOL bRet = FALSE;
	INT nDriveSampleOffset = 0;
	BOOL bGetDrive = GetDriveOffset(pDevice->szProductId, &nDriveSampleOffset);
	if (!bGetDrive) {
		_TCHAR buf[6] = { 0 };
		OutputString(
			_T("This drive doesn't define in driveOffset.txt\n")
			_T("Please input Drive offset(Samples): "));
		INT b = _tscanf(_T("%6[^\n]%*[^\n]"), buf);
		b = _gettchar();
		nDriveSampleOffset = _ttoi(buf);
	}

	INT nDriveOffset = nDriveSampleOffset * 4;
	if (pDisc->SCSI.bAudioOnly) {
		pDisc->MAIN.nCombinedOffset = nDriveOffset;
		OutputMmcCDOffset(pExtArg, pDisc, bGetDrive, nDriveSampleOffset, nDriveOffset);
		bRet = TRUE;
	}
	else {
		LPBYTE pBuf = (LPBYTE)calloc(CD_RAW_SECTOR_SIZE + pDevice->AlignmentMask, sizeof(BYTE));
		if (!pBuf) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevice, pBuf);

		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		if (pDevice->byPlexType) {
			CDB::_PLXTR_READ_CDDA cdb = { 0 };
			SetReadD8Command(pDevice, NULL, &cdb, 1, FALSE, TRUE);
			memcpy(lpCmd, &cdb, sizeof(lpCmd));
		}
		else {
			CDB::_READ_CD cdb = { 0 };
			SetReadCDCommand(pDevice, NULL, &cdb, READ_CD_FLAG::CDDA, 1, READ_CD_FLAG::SubNone, TRUE);
			memcpy(lpCmd, &cdb, sizeof(lpCmd));
		}

		if (*pExecType == gd) {
			pDisc->SCSI.nFirstLBAofDataTrack = 45000;
		}
		lpCmd[2] = HIBYTE(HIWORD(pDisc->SCSI.nFirstLBAofDataTrack));
		lpCmd[3] = LOBYTE(HIWORD(pDisc->SCSI.nFirstLBAofDataTrack));
		lpCmd[4] = HIBYTE(LOWORD(pDisc->SCSI.nFirstLBAofDataTrack));
		lpCmd[5] = LOBYTE(LOWORD(pDisc->SCSI.nFirstLBAofDataTrack));
#ifdef _DEBUG
		for (INT j = 0; j < CDB12GENERIC_LENGTH; j++) {
			OutputString(_T("Command[%d]: %#04x\n"), j, lpCmd[j]);
		}
#endif
		BYTE byScsiStatus = 0;
		try {
			if (!ScsiPassThroughDirect(pDevice, lpCmd, CDB12GENERIC_LENGTH, lpBuf, 
				CD_RAW_SECTOR_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				// PLEXTOR(PX-W8432, PX-W1210T, PX-W2410T)
				// if Track1 is DataTrack (mostly game)
				// ==>Sense data, Key:Asc:Ascq: 05:64:00(ILLEGAL_REQUEST. ILLEGAL MODE FOR THIS TRACK)
				// else if Track1 isn't DataTrack (pc engine etc)
				// ==>no error.
				if (*pExecType == gd) {
					OutputErrorString(
						_T("Couldn't read a data sector at scrambled mode\n")
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
			OutputMmcCDMain2352(lpBuf, pDisc->SCSI.nFirstLBAofDataTrack);

			BYTE aBuf2[CD_RAW_SECTOR_SIZE * 2] = { 0 };
			memcpy(aBuf2, lpBuf, CD_RAW_SECTOR_SIZE);

			lpCmd[2] = HIBYTE(HIWORD(pDisc->SCSI.nFirstLBAofDataTrack + 1));
			lpCmd[3] = LOBYTE(HIWORD(pDisc->SCSI.nFirstLBAofDataTrack + 1));
			lpCmd[4] = HIBYTE(LOWORD(pDisc->SCSI.nFirstLBAofDataTrack + 1));
			lpCmd[5] = LOBYTE(LOWORD(pDisc->SCSI.nFirstLBAofDataTrack + 1));
			if (!ScsiPassThroughDirect(pDevice, lpCmd, CDB12GENERIC_LENGTH, lpBuf,
				CD_RAW_SECTOR_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				if (*pExecType == gd) {
					OutputErrorString(
						_T("Couldn't read a data sector at scrambled mode\n")
						_T("Please start it again from 1st step or after waiting a little\n"));
				}
				else {
					OutputErrorString(
						_T("This drive can't read a data sector at scrambled mode\n")
						_T("Please start it again at plextor drive (PX-708, 712, 716, 755, 760 etc)\n"));
				}
				throw FALSE;
			}
			memcpy(aBuf2 + CD_RAW_SECTOR_SIZE, lpBuf, CD_RAW_SECTOR_SIZE);
			if (!GetWriteOffset(pDisc, aBuf2)) {
				OutputErrorString(_T("Failed to get write-offset\n"));
				throw FALSE;
			}
			OutputMmcCDOffset(pExtArg, pDisc, bGetDrive, nDriveSampleOffset, nDriveOffset);
			bRet = TRUE;
		}
		catch (BOOL bErr) {
			bRet = bErr;
		}
		FreeAndNull(pBuf);
	}

	if (bRet) {
		if (0 < pDisc->MAIN.nCombinedOffset) {
			pDisc->MAIN.nAdjustSectorNum =
				pDisc->MAIN.nCombinedOffset / CD_RAW_SECTOR_SIZE + 1;
		}
		else if (pDisc->MAIN.nCombinedOffset < 0) {
			pDisc->MAIN.nAdjustSectorNum =
				pDisc->MAIN.nCombinedOffset / CD_RAW_SECTOR_SIZE - 1;
		}
		OutputDiscLogA(
			"\tOverread sector: %d\n", pDisc->MAIN.nAdjustSectorNum);
	}
	return bRet;
}

BOOL ReadCDPartial(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszPath,
	INT nStart,
	INT nEnd,
	READ_CD_FLAG::EXPECTED_SECTOR_TYPE flg,
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
	LPBYTE pBufNext = NULL;
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector = NULL;
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

		SetCDTransferData(pDevice, DRIVE_DATA_ORDER::C2None);
		C2_ERROR pC2Error = { 0 };
		if (!bCheckReading && pDevice->bC2ErrorData && pExtArg->bC2) {
			if (NULL == (fpC2 = CreateOrOpenFileW(
				pszPath, NULL, NULL, NULL, NULL, _T(".c2"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			SetCDTransferData(pDevice, DRIVE_DATA_ORDER::MainC2Sub);
			DRIVE_DATA_ORDER dataOrder = DRIVE_DATA_ORDER::MainC2Sub;

			if (!InitC2ErrorData(pExtArg, pDisc, &pC2Error, 
				&pC2ErrorPerSector, pDevice->TRANSFER.dwAllBufLen)) {
				throw FALSE;
			}
			if (!ReadCDForCheckingByteOrder(pDevice, pExtArg, &dataOrder)) {
				Terminatec2ErrorDataPerSector(pDevice, pExtArg, &pC2ErrorPerSector);
				pDevice->bC2ErrorData = FALSE;
				SetCDTransferData(pDevice, DRIVE_DATA_ORDER::C2None);
			}
			if (dataOrder == DRIVE_DATA_ORDER::MainSubC2) {
				OutputDriveLogA(
					"Byte order of this drive is main + sub + c2\n");
				SetCDTransferData(pDevice, DRIVE_DATA_ORDER::MainSubC2);
			}
			else {
				OutputDriveLogA(
					"Byte order of this drive is main + c2 + sub\n");
			}
		}

		// store main+(c2)+sub data
		if (NULL == (pBuf = (LPBYTE)calloc(
			pDevice->TRANSFER.dwAllBufLen + pDevice->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevice, pBuf);

		LPBYTE lpBufNext = NULL;
		if (pDevice->wDriveBufSize > 256) {
			if (NULL == (pBufNext = (LPBYTE)calloc(pDevice->TRANSFER.dwBufLen +
				pDevice->TRANSFER.dwAdditionalBufLen + pDevice->AlignmentMask, sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			lpBufNext = (LPBYTE)ConvParagraphBoundary(pDevice, pBufNext);
		}

		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		_TCHAR szSubCode[5] = { 0 };
		if (pDevice->byPlexType && !pDisc->SCSI.bAudioOnly) {
			CDB::_PLXTR_READ_CDDA cdb = { 0 };
			SetReadD8Command(pDevice, pExtArg, &cdb, 1, TRUE, bCheckReading);
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
			SetReadCDCommand(pDevice, pExtArg, &cdb, 
				flg, 1, READ_CD_FLAG::Raw, bCheckReading);
			if (cdb.SubChannelSelection == READ_CD_FLAG::Raw) {
				_tcsncpy(szSubCode, _T("Raw"), 3);
			}
			memcpy(lpCmd, &cdb, sizeof(lpCmd));
		}
		OutputString(_T("Read command: %#x, Subcode reading mode: %s\n"), lpCmd[0], szSubCode);

		BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		SetCDOffsetData(pDisc, nStart, nEnd);
		BYTE byCurrentTrackNum =
			*pExecType == gd ? (BYTE)3 : pDisc->SCSI.toc.FirstTrack;
		if (*pExecType == gd) {
			// because address out of range
			pDisc->MAIN.nOffsetEnd = 0;
		}
		INT nFirstLBA = nStart + pDisc->MAIN.nOffsetStart;
		INT nLastLBA = nEnd + pDisc->MAIN.nOffsetEnd;
		INT nLBA = nFirstLBA;
#ifndef _DEBUG
		FlushLog();
#endif
		BYTE lpNextSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		UINT uiC2ErrorLBACnt = 0;

		while (nFirstLBA <= nLastLBA) {
			BOOL bProcessRet = ProcessReadCD(pExecType, pExtArg, pDevice,
				pDisc, &pC2Error, pC2ErrorPerSector, uiC2ErrorLBACnt,
				nLBA, lpBuf, lpBufNext, lpCmd, NULL, lpSubcode, lpNextSubcode, fpBin, bCheckReading);
			if (pC2ErrorPerSector && bProcessRet == RETURNED_C2_ERROR_EXIST) {
				INT nTmpLBA = nLBA + pC2Error.cSlideSectorNum;
				OutputErrorString(
					_T(" Detected C2 error. LBA[%06d, %#07x]\n"), nTmpLBA, nTmpLBA);
				SetAndOutputc2ErrorDataPerSector(&pC2Error, pC2ErrorPerSector,
					nTmpLBA, pDevice->TRANSFER.dwAllBufLen, uiC2ErrorLBACnt);
				uiC2ErrorLBACnt++;
				if (uiC2ErrorLBACnt == pExtArg->dwMaxC2ErrorNum) {
					OutputErrorString(_T("C2 error Max: %u\n"), uiC2ErrorLBACnt);
					throw FALSE;
				}
#ifdef _DEBUG
				OutputMmcCDMain2352(lpBuf, nLBA);
#endif
			}
			else if (bProcessRet == RETURNED_SKIP_LBA) {
				nLBA = pDisc->SCSI.nFirstLBAof2ndSession - 1;
			}
			else if (bProcessRet == RETURNED_FALSE) {
				throw FALSE;
			}
			if (nStart <= nLBA && nLBA < nEnd) {
				// TODO: Fix Subchannnel
#if 0
				SUB_Q subQ = { 0 };
				SetSubQDataFromReadCD(&subQ, lpBuf, lpSubcode);
				SUB_Q nextSubQ = { 0 };
				SetSubQDataFromReadCD(&nextSubQ, lpBuf, lpSubcode);
				CheckAndFixSubChannel(pExtArg, pDisc, lpSubcode, &nextSubQ,
					&subQ, &prevSubQ, &prevPrevSubQ, &byCurrentTrackNum, &bCatalog,
					lpISRCList,	lpEndCtlList, lpFirstLBAListOnSub, 
					lpFirstLBAListOfDataTrackOnSub,	nLBA->fpError);
#else
				byCurrentTrackNum = BcdToDec(lpSubcode[13]);
				if (byCurrentTrackNum < 1 || 99 < byCurrentTrackNum) {
					byCurrentTrackNum = 1;
				}
				BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
				// fix raw subchannel
				AlignColumnSubcode(lpSubcode, lpSubcodeRaw);
#endif
#if 0
				OutputMmcCDSub96Align(lpSubcode, nLBA->fpError);
#endif
				if (!pExtArg->bReverse) {
					WriteSubChannel(pDisc, lpSubcodeRaw, 
						lpSubcode, nLBA, byCurrentTrackNum, fpSub, fpParse);
				}
			}
			WriteMainChannel(pExtArg, pDisc, lpBuf, nLBA, fpBin);
			if (!bCheckReading && pDevice->bC2ErrorData && pExtArg->bC2) {
				WriteC2(pDisc, lpBuf + pDevice->TRANSFER.dwBufC2Offset, nLBA, fpC2);
				// TODO: exist an offset?
//				fwrite(lpBuf + pDevice->TRANSFER.dwBufC2Offset,
//					sizeof(BYTE), CD_RAW_READ_C2_294_SIZE, fpC2);
			}
			OutputString(_T("\rCreating bin from %d to %d (LBA) %6d"), 
				nStart + pDisc->MAIN.nOffsetStart, 
				nEnd + pDisc->MAIN.nOffsetEnd, nLBA);
			nLBA++;
			nFirstLBA++;
		}
		OutputString(_T("\n"));
		if (pDevice->bC2ErrorData && pExtArg->bC2) {
			FcloseAndNull(fpC2);
		}
		FcloseAndNull(fpParse);
		FcloseAndNull(fpSub);
#ifndef _DEBUG
		FlushLog();
#endif

		if (!bCheckReading) {
			if (pDevice->wDriveBufSize > 256 && pDevice->bC2ErrorData && pExtArg->bC2) {
				if (!ReadCDForRereadingSector(pExecType, pExtArg, 
					pDevice, pDisc, &pC2Error, pC2ErrorPerSector, uiC2ErrorLBACnt, 
					lpBuf, lpBufNext, lpCmd, NULL, lpSubcode, lpNextSubcode, fpBin)) {
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
	FreeAndNull(pBufNext);
	Terminatec2ErrorDataPerSector(pDevice, pExtArg, &pC2ErrorPerSector);

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
