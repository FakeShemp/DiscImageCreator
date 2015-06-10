/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "check.h"
#include "convert.h"
#include "execScsiCmd.h"
#include "execScsiCmdforCD.h"
#include "execIoctl.h"
#include "get.h"
#include "init.h"
#include "output.h"
#include "outputScsiCmdLog.h"
#include "outputScsiCmdLogforCD.h"
#include "set.h"
#include "_external/crc16ccitt.h"

BOOL ExecReadCD(
	PDEVICE pDevice,
	LPBYTE lpCmd,
	INT nLBA,
	LPBYTE lpBuf,
	DWORD dwBufSize,
	LPCTSTR pszFuncName,
	LONG lLineNum
	)
{
	lpCmd[2] = HIBYTE(HIWORD(nLBA));
	lpCmd[3] = LOBYTE(HIWORD(nLBA));
	lpCmd[4] = HIBYTE(LOWORD(nLBA));
	lpCmd[5] = LOBYTE(LOWORD(nLBA));
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pDevice, lpCmd, CDB12GENERIC_LENGTH, lpBuf,
		dwBufSize, &byScsiStatus, pszFuncName, lLineNum)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	return TRUE;
}

BOOL ExecReadCDForC2(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	LPBYTE lpCmd,
	INT nLBA,
	LPBYTE lpBuf,
	BOOL bCheckReading,
	LPCTSTR pszFuncName,
	LONG lLineNum
	)
{
	lpCmd[2] = HIBYTE(HIWORD(nLBA));
	lpCmd[3] = LOBYTE(HIWORD(nLBA));
	lpCmd[4] = HIBYTE(LOWORD(nLBA));
	lpCmd[5] = LOBYTE(LOWORD(nLBA));
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pDevice, lpCmd, CDB12GENERIC_LENGTH, lpBuf,
		pDevice->TRANSFER.dwBufLen + pDevice->TRANSFER.dwAdditionalBufLen,
		&byScsiStatus, pszFuncName, lLineNum)) {
		if (pExtArg->bReadContinue) {
			return RETURNED_CONTINUE;
		}
		else {
			return RETURNED_FALSE;
		}
	}

	if (byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		if (*pExecType != gd && !bCheckReading) {
			return RETURNED_CONTINUE;
		}
		else {
			return RETURNED_FALSE;
		}
	}
	return RETURNED_NO_C2_ERROR_1ST;
}

BOOL ProcessReadCD(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PMAIN_HEADER pMainHeader,
	PC2_ERROR pC2Error,
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	UINT uiC2ErrorLBACnt,
	LPBYTE lpCmd,
	INT nLBA,
	LPBYTE lpBuf,
	LPBYTE lpNextBuf,
	PSUB_Q pPrevSubQ,
	LPBYTE lpSubcode,
	LPBYTE lpNextSubcode,
	BOOL bCheckReading
	)
{
	BOOL bRet = RETURNED_NO_C2_ERROR_1ST;
	if (*pExecType != gd) {
		if (pDisc->SCSI.nFirstLBAof2ndSession != -1) {
			if (pExtArg->bReverse) {
				if (pDisc->SCSI.nFirstLBAof2ndSession == nLBA + 1) {
					OutputInfoLogA(
						"Skip from Leadout of Session 1 [%d, %#x] to Leadin of Session 2 [%d, %#x]\n",
						pDisc->SCSI.nFirstLBAofLeadout, pDisc->SCSI.nFirstLBAofLeadout, 
						pDisc->SCSI.nFirstLBAof2ndSession - 1, pDisc->SCSI.nFirstLBAof2ndSession - 1);
					pPrevSubQ->nAbsoluteTime = nLBA - SESSION_TO_SESSION_SKIP_LBA - 150;
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
							nLBA + SESSION_TO_SESSION_SKIP_LBA + 150 - pDisc->MAIN.nAdjustSectorNum - 1;
					}
					else if (pDisc->MAIN.nCombinedOffset < 0) {
						pPrevSubQ->nAbsoluteTime = 
							nLBA + SESSION_TO_SESSION_SKIP_LBA + 150 + pDisc->MAIN.nAdjustSectorNum;
					}
					return RETURNED_SKIP_LBA;
				}
			}
		}
	}
	if (pExtArg->bFua) {
		ReadCDForFlushingDriveCache(pDevice, nLBA);
	}
	bRet = ExecReadCDForC2(pExecType, pExtArg, pDevice,
		lpCmd, nLBA, lpBuf, bCheckReading, _T(__FUNCTION__), __LINE__);
	if (bRet == RETURNED_NO_C2_ERROR_1ST) {
		if (!bCheckReading) {
			UpdateTmpMainHeader(pMainHeader);
			if (IsValidMainDataHeader(lpBuf + pDisc->MAIN.uiMainDataSlideSize)) {
				if (lpBuf[pDisc->MAIN.uiMainDataSlideSize + 12] == pMainHeader->header[12] &&
					lpBuf[pDisc->MAIN.uiMainDataSlideSize + 13] == pMainHeader->header[13] &&
					lpBuf[pDisc->MAIN.uiMainDataSlideSize + 14] == pMainHeader->header[14]) {
					memcpy(pMainHeader->header, 
						lpBuf + pDisc->MAIN.uiMainDataSlideSize, sizeof(pMainHeader->header));
				}
			}
		}
		if (pDevice->wDriveBufSize > MINIMUM_DRIVE_BUF_SIZE) {
			// Read next sector to get next subchannel.
			// A drive of buffer size under 256 is very slow, so unread.
			bRet = ExecReadCDForC2(pExecType, pExtArg, pDevice,
				lpCmd, nLBA + 1, lpNextBuf, bCheckReading, _T(__FUNCTION__), __LINE__);
		}
		if (!bCheckReading) {
			if (pC2ErrorPerSector && pExtArg->bC2 && pDevice->bC2ErrorData) {
				if (pExtArg->bReadContinue && pDisc->PROTECT.bExist &&
					(pDisc->PROTECT.ERROR_SECTOR.nExtentPos <= nLBA &&
						nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize)) {
					// skip check c2 error
					ZeroMemory(lpBuf + pDevice->TRANSFER.dwBufC2Offset, CD_RAW_READ_C2_294_SIZE);
				}
				else {
					bRet = ContainsC2Error(pC2Error,
						pC2ErrorPerSector, pDevice, lpBuf, uiC2ErrorLBACnt);
				}
			}
			AlignRowSubcode(lpBuf + pDevice->TRANSFER.dwBufSubOffset, lpSubcode);
			if (pDevice->wDriveBufSize > MINIMUM_DRIVE_BUF_SIZE) {
				AlignRowSubcode(lpNextBuf + pDevice->TRANSFER.dwBufSubOffset, lpNextSubcode);
			}
		}
	}
	if (!bCheckReading) {
		if (pExtArg->bReadContinue && pDisc->PROTECT.bExist &&
			(pDisc->PROTECT.ERROR_SECTOR.nExtentPos <= nLBA &&
				nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize)) {
			if (bRet == RETURNED_CONTINUE) {
				if (pC2ErrorPerSector && pExtArg->bC2 && pDevice->bC2ErrorData) {
					// skip check c2 error
					ZeroMemory(lpBuf + pDevice->TRANSFER.dwBufC2Offset, CD_RAW_READ_C2_294_SIZE);
				}
			}
			// replace sub to sub of prev
			ZeroMemory(lpBuf + pDevice->TRANSFER.dwBufSubOffset, CD_RAW_READ_SUBCODE_SIZE);
			ZeroMemory(lpSubcode, CD_RAW_READ_SUBCODE_SIZE);
			lpSubcode[12] = BYTE(pPrevSubQ->byCtl << 4 | pPrevSubQ->byAdr);
			lpSubcode[13] = pPrevSubQ->byTrackNum;
			lpSubcode[14] = pPrevSubQ->byIndex;
			BYTE m, s, f;
			LBAtoMSF(pPrevSubQ->nRelativeTime + 1, &m, &s, &f);
			lpSubcode[15] = DecToBcd(m);
			lpSubcode[16] = DecToBcd(s);
			lpSubcode[17] = DecToBcd(f);
			LBAtoMSF(pPrevSubQ->nAbsoluteTime + 1, &m, &s, &f);
			lpSubcode[19] = DecToBcd(m);
			lpSubcode[20] = DecToBcd(s);
			lpSubcode[21] = DecToBcd(f);
			WORD crc16 = (WORD)GetCrc16CCITT(10, &lpSubcode[12]);
			lpSubcode[22] = HIBYTE(crc16);
			lpSubcode[23] = LOBYTE(crc16);
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
	PDEVICE pDevice
	)
{
	BOOL bRet = TRUE;
	if (pDevice->byPlexType) {
		LPBYTE pBuf = NULL;
		LPBYTE lpBuf = NULL;
		if (!GetAlignedAllocatedBuffer(pDevice, &pBuf,
			CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, &lpBuf, _T(__FUNCTION__), __LINE__)) {
			return FALSE;
		}
		CDB::_READ_CD cdb = { 0 };
		BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		SetReadCDCommand(NULL, pDevice, &cdb, READ_CD_FLAG::All, 1, READ_CD_FLAG::Raw, TRUE);
		try {
			if (!ExecReadCD(pDevice, (LPBYTE)&cdb, 0, lpBuf,
				CD_RAW_SECTOR_WITH_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
			OutputDiscLogA(
				"============ Check SubP to W, OperationCode: 0xbe, Subcode: 1(=Raw) ===========\n");
			OutputCDSub96Align(lpSubcode, 0);

			SetReadCDCommand(NULL, pDevice, &cdb, READ_CD_FLAG::All, 1, READ_CD_FLAG::Pack, TRUE);
			if (!ExecReadCD(pDevice, (LPBYTE)&cdb, 0, lpBuf,
				CD_RAW_SECTOR_WITH_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
			OutputDiscLogA(
				"=========== Check SubP to W, OperationCode: 0xbe, Subcode: 4(=Pack) ===========\n");
			OutputCDSub96Align(lpSubcode, 0);

			CDB::_PLXTR_READ_CDDA cdb2 = { 0 };
			SetReadD8Command(NULL, pDevice, &cdb2, 1, TRUE, TRUE);
			cdb2.SubCode = READ_D8_FLAG::MainPack;
			if (!ExecReadCD(pDevice, (LPBYTE)&cdb2, 0, lpBuf,
				CD_RAW_SECTOR_WITH_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
			OutputDiscLogA(
				"========== Check SubP to W, OperationCode: 0xd8, Subcode: 0x02(=Pack) =========\n");
			OutputCDSub96Align(lpSubcode, 0);

			cdb2.SubCode = READ_D8_FLAG::Raw;
			if (!ExecReadCD(pDevice, (LPBYTE)&cdb2, 0, lpBuf,
				CD_RAW_READ_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(lpBuf, lpSubcode);
			OutputDiscLogA(
				"========== Check SubP to W, OperationCode: 0xd8, Subcode: 0x03(=Pack) =========\n");
			OutputCDSub96Align(lpSubcode, 0);

			cdb2.SubCode = READ_D8_FLAG::MainC2Raw;
			if (!ExecReadCD(pDevice, (LPBYTE)&cdb2, 0, lpBuf,
				CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE + CD_RAW_READ_C2_294_SIZE, lpSubcode);
			OutputDiscLogA(
				"========== Check SubP to W, OperationCode: 0xd8, Subcode: 0x08(=Raw) ==========\n");
			OutputCDSub96Align(lpSubcode, 0);
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
	LPBYTE lpBuf = NULL;
	if (!GetAlignedAllocatedBuffer(pDevice, &pBuf,
		CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	CDB::_READ_CD cdb = { 0 };
	SetReadCDCommand(NULL, pDevice, &cdb, READ_CD_FLAG::All, 1, READ_CD_FLAG::Raw, TRUE);
	for (BYTE i = 0; i < pDisc->SCSI.toc.LastTrack; i++) {
		if ((pDisc->SCSI.toc.TrackData[i].Control & AUDIO_DATA_TRACK) == 0) {
			try {
				INT nTmpLBA = pDisc->SCSI.lpFirstLBAListOnToc[i] + 100;
				if (!ExecReadCD(pDevice, (LPBYTE)&cdb, nTmpLBA, lpBuf,
					CD_RAW_SECTOR_WITH_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
				BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
				AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
				BYTE lpSubcodeOrg[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
				memcpy(lpSubcodeOrg, lpBuf + CD_RAW_SECTOR_SIZE, CD_RAW_READ_SUBCODE_SIZE);
				OutputDiscLogA(
					"=========================== Check RtoW for track[%02d] ==========================\n"
					, i + 1);
				OutputCDSub96Align(lpSubcode, nTmpLBA);

				SUB_R_TO_W scRW[4] = { 0 };
				BYTE tmpCode[24] = { 0 };
				INT nRtoW = 0;
				BOOL bCDG = FALSE;
				BOOL bCDEG = FALSE;
				for (INT k = 0; k < 4; k++) {
					for (INT j = 0; j < 24; j++) {
						tmpCode[j] = (BYTE)(*(lpSubcodeOrg + (k * 24 + j)) & 0x3f);
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
				INT nR = 0;
				INT nS = 0;
				INT nT = 0;
				INT nU = 0;
				INT nV = 0;
				INT nW = 0;
				for (INT j = 24; j < CD_RAW_READ_SUBCODE_SIZE; j++) {
					if (24 <= j && j < 36) {
						nR += lpSubcode[j];
					}
					else if (36 <= j && j < 48) {
						nS += lpSubcode[j];
					}
					else if (48 <= j && j < 60) {
						nT += lpSubcode[j];
					}
					else if (60 <= j && j < 72) {
						nU += lpSubcode[j];
					}
					else if (72 <= j && j < 84) {
						nV += lpSubcode[j];
					}
					else if (84 <= j && j < CD_RAW_READ_SUBCODE_SIZE) {
						nW += lpSubcode[j];
					}
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
					pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::Full;
					OutputDiscLogA("Track[%02u]: RtoW is 0xff\n", i + 1);
				}
				else {
					BOOL bFull = FALSE;
					// 0xff * 12 = 0xbf4
					if (nR == 0xbf4) {
						pDisc->SUB.lpRtoWList[i] |= SUB_RTOW_TYPE::RFull;
						OutputDiscLogA("Track[%02u]: All R is 0xff\n", i + 1);
						bFull = TRUE;
					}
					if (nS == 0xbf4) {
						pDisc->SUB.lpRtoWList[i] |= SUB_RTOW_TYPE::SFull;
						OutputDiscLogA("Track[%02u]: All S is 0xff\n", i + 1);
						bFull = TRUE;
					}
					if (nT == 0xbf4) {
						pDisc->SUB.lpRtoWList[i] |= SUB_RTOW_TYPE::TFull;
						OutputDiscLogA("Track[%02u]: All T is 0xff\n", i + 1);
						bFull = TRUE;
					}
					if (nU == 0xbf4) {
						pDisc->SUB.lpRtoWList[i] |= SUB_RTOW_TYPE::UFull;
						OutputDiscLogA("Track[%02u]: All U is 0xff\n", i + 1);
						bFull = TRUE;
					}
					if (nV == 0xbf4) {
						pDisc->SUB.lpRtoWList[i] |= SUB_RTOW_TYPE::VFull;
						OutputDiscLogA("Track[%02u]: All V is 0xff\n", i + 1);
						bFull = TRUE;
					}
					if (nW == 0xbf4) {
						pDisc->SUB.lpRtoWList[i] |= SUB_RTOW_TYPE::WFull;
						OutputDiscLogA("Track[%02u]: All W is 0xff\n", i + 1);
						bFull = TRUE;
					}
					if (!bFull) {
						if (bCDG && nRtoW > 0 && nRtoW != 0x200) {
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
			}
			catch (BOOL bErr) {
				bRet = bErr;
			}
		}
		else {
			pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::Zero;
		}
	}
	FreeAndNull(pBuf);
	return bRet;
}

BOOL ReadCDForCheckingByteOrder(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDRIVE_DATA_ORDER pOrder
	)
{
	UINT newBufLen = 
		CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE + pDevice->TRANSFER.dwAdditionalBufLen;
	LPBYTE pBuf = NULL;
	LPBYTE lpBuf = NULL;
	if (!GetAlignedAllocatedBuffer(pDevice, &pBuf,
		newBufLen, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}

	BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
	if (pDevice->byPlexType) {
		CDB::_PLXTR_READ_CDDA cdb = { 0 };
		SetReadD8Command(pExtArg, pDevice, &cdb, 1, TRUE, FALSE);
		memcpy(lpCmd, &cdb, sizeof(lpCmd));
	}
	else {
		// non plextor && support scrambled ripping
		CDB::_READ_CD cdb = { 0 };
		SetReadCDCommand(pExtArg, pDevice, &cdb, READ_CD_FLAG::CDDA, 1, READ_CD_FLAG::Raw, FALSE);
		memcpy(lpCmd, &cdb, sizeof(lpCmd));
	}

	BOOL bRet = TRUE;
	if (!ExecReadCD(pDevice, lpCmd, 0, lpBuf, newBufLen, _T(__FUNCTION__), __LINE__)) {
		OutputErrorString(_T("This drive doesn't support C2 error report\n"));
		bRet = FALSE;
	}
	else {
#ifdef _DEBUG
		OutputCDMain2352(lpBuf, 0);
#endif
		OutputDriveLogA(
			"============================== Check main+c2+sub ==============================\n");
		OutputCDC2Error296(fileDrive, lpBuf + CD_RAW_SECTOR_SIZE, 0);
		OutputCDSub96Raw(fileDrive, lpBuf + CD_RAW_SECTOR_WITH_C2_294_SIZE, 0);

		OutputDriveLogA(
			"============================== Check main+sub+c2 ==============================\n");
		OutputCDSub96Raw(fileDrive, lpBuf + CD_RAW_SECTOR_SIZE, 0);
		OutputCDC2Error296(fileDrive, lpBuf + CD_RAW_SECTOR_WITH_SUBCODE_SIZE, 0);

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
	PMAIN_HEADER pMainHeader,
	LPCTSTR pszPath,
	READ_CD_FLAG::EXPECTED_SECTOR_TYPE flg
	)
{
	BOOL bRet = TRUE;
	if (pDisc->MAIN.nCombinedOffset < 0) {
		OutputString(_T("Checking reading lead-in\n"));
		bRet = ReadCDPartial(pExecType, pExtArg,
			pDevice, pDisc, pMainHeader, pszPath, 0, 0, flg, TRUE);
		if (bRet) {
			OutputLogA(standardOut | fileDrive, "Reading lead-in: OK\n");
		}
		else {
			OutputLogA(standardOut | fileDrive, "Reading lead-in: NG\n");
			bRet = FALSE;
		}
	}
	else if (0 < pDisc->MAIN.nCombinedOffset) {
		OutputString(_T("Checking reading lead-out\n"));
		bRet = ReadCDPartial(pExecType, pExtArg, pDevice, pDisc, pMainHeader,
			pszPath, pDisc->SCSI.nAllLength + 1, pDisc->SCSI.nAllLength, flg, TRUE);
		if (bRet) {
			OutputLogA(standardOut | fileDrive, "Reading lead-out: OK\n");
		}
		else {
			OutputLogA(standardOut | fileDrive, "Reading lead-out: NG\n");
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
	if (!ExecReadCD(pDevice, (LPBYTE)&cdb, 0, NULL, 0, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	return TRUE;
}

BOOL ReadCDForVolumeDescriptor(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	BYTE byIdx,
	CDB::_READ_CD* pCdb,
	LPBYTE* ppBuf,
	LPBYTE lpBuf,
	LPBOOL pPVD
	)
{
	BOOL bRet = TRUE;
	INT nPVD = pDisc->SCSI.lpFirstLBAListOnToc[byIdx] + 16;
	INT nTmpLBA = nPVD;
	INT PathTblSize = 0;
	INT PathTblPos = 0;
	INT nRootDataLen = 0;
	for (;;) {
		if (!ExecReadCD(pDevice, (LPBYTE)pCdb, nTmpLBA, lpBuf,
			DISC_RAW_READ, _T(__FUNCTION__), __LINE__)) {
			throw FALSE;
		}
		if (!strncmp((PCHAR)&lpBuf[1], "CD001", 5)) {
			if (nTmpLBA == nPVD) {
				PathTblSize = MAKELONG(MAKEWORD(lpBuf[132], lpBuf[133]),
					MAKEWORD(lpBuf[134], lpBuf[135]));
				PathTblPos = MAKELONG(MAKEWORD(lpBuf[140], lpBuf[141]),
					MAKEWORD(lpBuf[142], lpBuf[143]));
				nRootDataLen = MAKELONG(MAKEWORD(lpBuf[166], lpBuf[167]),
					MAKEWORD(lpBuf[168], lpBuf[169]));
				*pPVD = TRUE;
			}
			OutputFsVolumeDescriptor(pExtArg, pDisc, lpBuf, nTmpLBA++);
		}
		else {
			break;
		}
	}
	if (*pPVD) {
		BYTE transferLen = 1;
		LPBYTE lpBuf2 = lpBuf;
		if (PathTblSize > DISC_RAW_READ) {
			transferLen += (BYTE)(PathTblSize / DISC_RAW_READ);
			LPBYTE pBuf2 = (LPBYTE)realloc(
				*ppBuf, DISC_RAW_READ * transferLen + pDevice->AlignmentMask);
			if (!pBuf2) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
			*ppBuf = pBuf2;
			lpBuf2 = (LPBYTE)ConvParagraphBoundary(pDevice, pBuf2);
			pCdb->TransferBlocks[2] = (BYTE)transferLen;
		}
		if (!ExecReadCD(pDevice, (LPBYTE)pCdb, PathTblPos, lpBuf2,
			(DWORD)(DISC_RAW_READ * transferLen), _T(__FUNCTION__), __LINE__)) {
			return FALSE;
		}
		// TODO: buf size
		CONST INT nMaxDirNum = 4096;
		LPINT pDirTblPosList = (LPINT)calloc(nMaxDirNum, sizeof(INT));
		if (!pDirTblPosList) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		LPINT pDirTblSizeList = NULL;
		LPSTR* pDirTblNameList = NULL;
		try {
			pDirTblSizeList = (LPINT)calloc(nMaxDirNum, sizeof(INT));
			if (!pDirTblSizeList) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			pDirTblNameList = (LPSTR*)calloc(nMaxDirNum, sizeof(UINT_PTR));
			if (!pDirTblNameList) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			CONST INT nMaxFname = 64;
			for (INT p = 0; p < nMaxDirNum; p++) {
				pDirTblNameList[p] = (LPSTR)calloc(nMaxFname, sizeof(CHAR));
				if (!(pDirTblNameList[p])) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
			}
			INT nDirPosNum = 0;
			OutputFsPathTableRecord(lpBuf2,
				PathTblPos, PathTblSize, pDirTblPosList, pDirTblNameList, &nDirPosNum);
			OutputInfoLogA("Dir Num: %u\n", nDirPosNum);

			LPBYTE lpBuf3 = lpBuf2;
			LPBYTE pBuf3 = NULL;
			if (nRootDataLen != DISC_RAW_READ * transferLen) {
				pBuf3 = (LPBYTE)realloc(*ppBuf, nRootDataLen + pDevice->AlignmentMask);
				if (!pBuf3) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				*ppBuf = pBuf3;
				lpBuf3 = (LPBYTE)ConvParagraphBoundary(pDevice, pBuf3);
				transferLen = (BYTE)(nRootDataLen / DISC_RAW_READ);
				pCdb->TransferBlocks[2] = transferLen;
			}
			pDirTblSizeList[0] = nRootDataLen;
//			INT nDirIdx = 1;
			for (INT i = 0; i < nDirPosNum; i++) {
				UINT nOfs = 0;
				if (pDirTblSizeList[i] != pDirTblSizeList[i - 1]) {
					pBuf3 = (LPBYTE)realloc(*ppBuf, pDirTblSizeList[i] + pDevice->AlignmentMask);
					if (!pBuf3) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						throw FALSE;
					}
					*ppBuf = pBuf3;
					lpBuf3 = (LPBYTE)ConvParagraphBoundary(pDevice, pBuf3);
					transferLen = (BYTE)(pDirTblSizeList[i] / DISC_RAW_READ);
					pCdb->TransferBlocks[2] = transferLen;
				}
				if (!ExecReadCD(pDevice, (LPBYTE)pCdb, pDirTblPosList[i], lpBuf3,
					(DWORD)(DISC_RAW_READ * transferLen), _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
				OutputInfoLogA(
					"==================== Directory Record, LBA[%06d, %#07x] ===================\n"
					, pDirTblPosList[i], pDirTblPosList[i]);
				for (INT nSectorNum = 0; nSectorNum < transferLen;) {
					for (;;) {
						CHAR curDirName[nMaxDirNum] = { 0 };
						LPBYTE lpDirRec = lpBuf3 + nOfs;
						if (lpDirRec[0] >= 0x22) {
							INT nDataLen = MAKELONG(MAKEWORD(lpDirRec[10], lpDirRec[11]),
								MAKEWORD(lpDirRec[12], lpDirRec[13]));
							OutputFsDirectoryRecord(pExtArg, pDisc, lpDirRec, nDataLen, curDirName);
							OutputInfoLogA("\n");
							nOfs += lpDirRec[0];
							for (INT b = 1; b < nDirPosNum; b++) {
								if (pDirTblSizeList[b] == 0 && !strncmp(curDirName, pDirTblNameList[b], nMaxDirNum)) {
									pDirTblSizeList[b] = nDataLen;
									break;
								}
							}
							if (nOfs == (UINT)(DISC_RAW_READ * (nSectorNum + 1))) {
								nSectorNum++;
							}
						}
						else {
							UINT zeroPaddingNum = DISC_RAW_READ * (nSectorNum + 1) - nOfs;
							if (nSectorNum < transferLen) {
								UINT j = 0;
								for (j = 0; j < zeroPaddingNum; j++) {
									if (lpDirRec[j] != 0) {
										break;
									}
								}
								if (j == zeroPaddingNum) {
									nOfs += zeroPaddingNum;
									nSectorNum++;
									break;
								}
							}
							else {
								break;
							}
						}
					}
				}
			}
			if (pDisc->PROTECT.bExist) {
				OutputLogA(standardOut | fileDisc, "Detected %s, Skip error from %d to %d\n"
					, pDisc->PROTECT.name, pDisc->PROTECT.ERROR_SECTOR.nExtentPos,
					pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize);
			}
		}
		catch (BOOL bErr) {
			bRet = bErr;
		}
		for (INT p = 0; p < nMaxDirNum; p++) {
			FreeAndNull(pDirTblNameList[p]);
		}
		FreeAndNull(pDirTblPosList);
		FreeAndNull(pDirTblNameList);
		FreeAndNull(pDirTblSizeList);
	}
	return bRet;
}

BOOL ReadCDFor3DODirectory(
	PDEVICE pDevice,
	CDB::_READ_CD* pCdb,
	PCHAR path,
	INT nLBA
	)
{
	BOOL bRet = TRUE;
	LPBYTE pBuf = NULL;
	LPBYTE lpBuf = NULL;
	if (!GetAlignedAllocatedBuffer(pDevice, &pBuf,
		DISC_RAW_READ, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	try {
		if (!ExecReadCD(pDevice, (LPBYTE)pCdb, nLBA, lpBuf,
			DISC_RAW_READ, _T(__FUNCTION__), __LINE__)) {
			throw FALSE;
		}
		LONG cur = THREEDO_DIR_HEADER_SIZE;
		LONG directorySize =
			MAKELONG(MAKEWORD(lpBuf[15], lpBuf[14]), MAKEWORD(lpBuf[13], lpBuf[12]));
		OutputFs3doDirectoryRecord(lpBuf, nLBA, path, directorySize);

		// next dir
		CHAR szNewPath[_MAX_PATH] = { 0 };
		CHAR fname[32] = { 0 };
		while (cur < directorySize) {
			LPBYTE dirEnt = lpBuf + cur;
			LONG flags = MAKELONG(MAKEWORD(dirEnt[3], dirEnt[2]), MAKEWORD(dirEnt[1], dirEnt[0]));
			strncpy(fname, (PCHAR)&dirEnt[32], sizeof(fname));
			LONG lastCopy = MAKELONG(MAKEWORD(dirEnt[67], dirEnt[66]), MAKEWORD(dirEnt[65], dirEnt[64]));
			cur += THREEDO_DIR_ENTRY_SIZE;

			if ((flags & 0xff) == 7) {
				sprintf(szNewPath, "%s%s/", path, fname);
				if (!ReadCDFor3DODirectory(pDevice, pCdb, szNewPath,
					MAKELONG(MAKEWORD(dirEnt[71], dirEnt[70]), MAKEWORD(dirEnt[69], dirEnt[68])))) {
					return FALSE;
				}
			}
			for (LONG i = 0; i < lastCopy; i++) {
				cur += sizeof(LONG);
			}
		}
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(pBuf);
	return bRet;
}

BOOL ReadCDForFileSystem(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	)
{
	BOOL bRet = TRUE;

	for (BYTE i = 0; i < pDisc->SCSI.toc.LastTrack; i++) {
		if ((pDisc->SCSI.toc.TrackData[i].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			LPBYTE pBuf = NULL;
			LPBYTE lpBuf = NULL;
			if (!GetAlignedAllocatedBuffer(pDevice, &pBuf,
				DISC_RAW_READ, &lpBuf, _T(__FUNCTION__), __LINE__)) {
				return FALSE;
			}
			CDB::_READ_CD cdb = { 0 };
			SetReadCDCommand(NULL, pDevice, &cdb,
				READ_CD_FLAG::All, 1, READ_CD_FLAG::SubNone, TRUE);
			cdb.ErrorFlags = 0;
			cdb.IncludeEDC = 0;
			cdb.HeaderCode = 0;
			cdb.IncludeSyncData = 0;
			BOOL bPVD = FALSE;
			try {
				// general data track disc
				if (!ReadCDForVolumeDescriptor(pExtArg,
					pDevice, pDisc, i, &cdb, &pBuf, lpBuf, &bPVD)) {
					throw FALSE;
				}
				if (!bPVD) {
					// for 3DO
					if (!ExecReadCD(pDevice, (LPBYTE)&cdb, 0, lpBuf,
						DISC_RAW_READ, _T(__FUNCTION__), __LINE__)) {
						throw FALSE;
					}
					if (IsValid3doDataHeader(lpBuf)) {
						OutputFs3doHeader(lpBuf, 0);
						if (!ReadCDFor3DODirectory(pDevice, &cdb, "/",
							MAKELONG(MAKEWORD(lpBuf[103], lpBuf[102]),
								MAKEWORD(lpBuf[101], lpBuf[100])))) {
							throw FALSE;
						}
						break;
					}
					// for MAC
					if (!ExecReadCD(pDevice, (LPBYTE)&cdb, 1, lpBuf,
						DISC_RAW_READ, _T(__FUNCTION__), __LINE__)) {
						throw FALSE;
					}
					if (IsValidMacDataHeader(lpBuf + 1024)) {
						OutputFsMasterDirectoryBlocks(lpBuf + 1024, 1);
						break;
					}
					else if (IsValidMacDataHeader(lpBuf + 512)) {
						OutputFsMasterDirectoryBlocks(lpBuf + 512, 1);
						break;
					}
					// other (pce, pc-fx)
					INT nLBA = pDisc->SCSI.nFirstLBAofDataTrack;
					if (!ExecReadCD(pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
						DISC_RAW_READ, _T(__FUNCTION__), __LINE__)) {
						throw FALSE;
					}
					OutputFsPceStuff(lpBuf, nLBA);

					nLBA = pDisc->SCSI.nFirstLBAofDataTrack + 1;
					if (!ExecReadCD(pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
						DISC_RAW_READ, _T(__FUNCTION__), __LINE__)) {
						throw FALSE;
					}
					OutputFsPceBootSector(lpBuf, nLBA);
					break;
				}
			}
			catch (BOOL bErr) {
				bRet = bErr;
			}
			FreeAndNull(pBuf);
		}
	}
	return bRet;
}

BOOL ReadCDForRereadingSector(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PMAIN_HEADER pMainHeader,
	PC2_ERROR pC2Error,
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	UINT uiC2ErrorLBACnt,
	LPBYTE lpCmd,
	LPBYTE lpBuf,
	LPBYTE lpNextBuf,
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
				pMainHeader, pC2Error, pC2ErrorPerSector, uiC2ErrorLBACnt, lpCmd, 
				nLBA, lpBuf, lpNextBuf, pPrevSubQ, lpSubcode, lpNextSubcode, FALSE);

			if (bProcessRet == RETURNED_EXIST_C2_ERROR) {
#if 0
				SetAndOutputC2ErrorDataPerSector(pC2Error, pC2ErrorPerSector,
					nLBA, pDevice->TRANSFER.dwAllBufLen, uiC2ErrorLBACnt);
#else
				SetAndOutputC2ErrorDataPerSector(pC2ErrorPerSector,
					nLBA, pDevice->TRANSFER.dwAllBufLen, uiC2ErrorLBACnt);
#endif
				uiC2ErrorLBACnt++;
			}
			else if (bProcessRet == RETURNED_NO_C2_ERROR_1ST) {
				if (pC2ErrorPerSector[i].bErrorFlagBackup == RETURNED_EXIST_C2_ERROR ||
					pC2ErrorPerSector[i].bErrorFlagBackup == RETURNED_NO_C2_ERROR_BUT_BYTE_ERROR) {
					SetAndOutputC2NoErrorData(pC2ErrorPerSector, lpBuf, nLBA,
						pDevice->TRANSFER.dwAllBufLen, uiC2ErrorLBACnt);
					if (pC2ErrorPerSector[i].bErrorFlagBackup == RETURNED_NO_C2_ERROR_BUT_BYTE_ERROR) {
						if (ContainsDiffByte(pC2ErrorPerSector, lpBuf, i)) {
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
					if (ContainsDiffByte(pC2ErrorPerSector, lpBuf, i)) {
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
	PMAIN_HEADER pMainHeader,
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
		if (NULL == (fpImg = CreateOrOpenFileW(pszPath, _T("_reverse"),
			pszOutReverseScmFile, NULL, NULL, _T(".scm"), _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}
	else {
		if (NULL == (fpImg = CreateOrOpenFileW(pszPath, NULL,
			pszOutScmFile, NULL, NULL, _T(".scm"), _T("wb"), 0, 0))) {
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
	LPBYTE pBuf = NULL;
	LPBYTE pNextBuf = NULL;
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector = NULL;
	BYTE aScrambledBuf[CD_RAW_SECTOR_SIZE] = { 0 };
	try {
		FILE* fpTbl = NULL;
		// init start
		if (NULL == (fpTbl = OpenProgrammabledFile(_T("scramble.bin"), _T("rb")))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		fread(aScrambledBuf, sizeof(BYTE), sizeof(aScrambledBuf), fpTbl);
		FcloseAndNull(fpTbl);

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

		SetCDTransfer(pDevice, DRIVE_DATA_ORDER::C2None);
		C2_ERROR pC2Error = { 0 };
		if (pExtArg->bC2 && pDevice->bC2ErrorData) {
			if (NULL == (fpC2 = CreateOrOpenFileW(
				pszPath, NULL, NULL, NULL, NULL, _T(".c2"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			SetCDTransfer(pDevice, DRIVE_DATA_ORDER::MainC2Sub);
			DRIVE_DATA_ORDER dataOrder = DRIVE_DATA_ORDER::MainC2Sub;

			if (!InitC2ErrorData(pExtArg, pDisc, &pC2Error,
				&pC2ErrorPerSector, pDevice->TRANSFER.dwAllBufLen)) {
				throw FALSE;
			}
			if (!ReadCDForCheckingByteOrder(pExtArg, pDevice, &dataOrder)) {
				TerminateC2ErrorDataPerSector(pExtArg, pDevice, &pC2ErrorPerSector);
				pDevice->bC2ErrorData = FALSE;
				SetCDTransfer(pDevice, DRIVE_DATA_ORDER::C2None);
			}
			if (dataOrder == DRIVE_DATA_ORDER::MainSubC2) {
				OutputDriveLogA(
					"=> Byte order of this drive is main + sub + c2\n");
				SetCDTransfer(pDevice, DRIVE_DATA_ORDER::MainSubC2);
			}
			else {
				OutputDriveLogA(
					"=> Byte order of this drive is main + c2 + sub\n");
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
		LPBYTE lpBuf = NULL;
		if (!GetAlignedAllocatedBuffer(pDevice, &pBuf,
			pDevice->TRANSFER.dwAllBufLen, &lpBuf, _T(__FUNCTION__), __LINE__)) {
			return FALSE;
		}
		LPBYTE lpNextBuf = NULL;
		if (!GetAlignedAllocatedBuffer(pDevice, &pNextBuf,
			pDevice->TRANSFER.dwAllBufLen, &lpNextBuf, _T(__FUNCTION__), __LINE__)) {
			return FALSE;
		}
		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		_TCHAR szSubCode[5] = { 0 };
		if (pDevice->byPlexType && !pDisc->SCSI.bAudioOnly) {
			CDB::_PLXTR_READ_CDDA cdb = { 0 };
			SetReadD8Command(pExtArg, pDevice, &cdb, 1, TRUE, FALSE);
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
			SetReadCDCommand(pExtArg, pDevice, &cdb,
				READ_CD_FLAG::CDDA, 1, READ_CD_FLAG::Raw, FALSE);
			if (cdb.SubChannelSelection == READ_CD_FLAG::Raw) {
				_tcsncpy(szSubCode, _T("Raw"), 3);
			}
			memcpy(lpCmd, &cdb, sizeof(lpCmd));
		}
		OutputString(_T("Read command: %#x, Subcode reading mode: %s\n"), lpCmd[0], szSubCode);

		BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		SUB_Q prevSubQ = { 0 };
		// read -1 because get prevSubQ
		if (!ExecReadCD(pDevice, lpCmd, -1, lpBuf,
			pDevice->TRANSFER.dwAllBufLen, _T(__FUNCTION__), __LINE__)) {
			throw FALSE;
		}
		AlignRowSubcode(lpBuf + pDevice->TRANSFER.dwBufSubOffset, lpSubcode);
		SetSubQDataFromReadCD(pDevice->wDriveBufSize,
			pDisc->MAIN.uiMainDataSlideSize, pMainHeader, &prevSubQ, lpBuf, lpSubcode);
		if (prevSubQ.byAdr != ADR_ENCODES_CURRENT_POSITION) {
			// 1552 Tenka Tairan
			prevSubQ.byAdr = ADR_ENCODES_CURRENT_POSITION;
			prevSubQ.byTrackNum = 1;
		}
		if (prevSubQ.nAbsoluteTime != 149) {
			// 1552 Tenka Tairan
			prevSubQ.nAbsoluteTime = 149;
		}

		for (UINT p = 0; p < pDisc->SCSI.toc.LastTrack; p++) {
			if (!ExecReadCD(pDevice, lpCmd, pDisc->SCSI.lpFirstLBAListOnToc[p], lpBuf,
				pDevice->TRANSFER.dwAllBufLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(lpBuf + pDevice->TRANSFER.dwBufSubOffset, lpSubcode);
			pDisc->SUB.lpEndCtlList[p] = (BYTE)((lpSubcode[12] >> 4) & 0x0f);
		}

		SetCDOffset(pDisc, 0, pDisc->SCSI.nAllLength);
		BYTE byCurrentTrackNum = pDisc->SCSI.toc.FirstTrack;
		INT nFirstLBAForSub = 0;
		INT nFirstLBA = pDisc->MAIN.nOffsetStart;
		INT nLastLBA = pDisc->SCSI.nAllLength + pDisc->MAIN.nOffsetEnd;
		INT nLBA = nFirstLBA;	// This value switches by /r option.

		if (pExtArg->bReverse) {
			SetCDOffset(pDisc, nLastLBA, pDisc->SCSI.nFirstLBAofDataTrack);
			byCurrentTrackNum = pDisc->SCSI.byLastDataTrack;
			nFirstLBA = pDisc->SCSI.nFirstLBAofDataTrack;
			nLastLBA = pDisc->SCSI.nLastLBAofDataTrack + 1;
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
			pMainHeader->byMode = DATA_BLOCK_MODE0;
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
		UINT uiC2ErrorLBACnt = 0;
		BOOL bReadOK = pDisc->SUB.bIndex0InTrack1 ? FALSE : TRUE;

		while (nFirstLBA <= nLastLBA) {
			BOOL bProcessRet = ProcessReadCD(pExecType, pExtArg, pDevice,
				pDisc, pMainHeader, &pC2Error, pC2ErrorPerSector, uiC2ErrorLBACnt,
				lpCmd, nLBA, lpBuf, lpNextBuf, &prevSubQ, lpSubcode, lpNextSubcode, FALSE);
//#define C2TEST
#if defined C2TEST
			if (nLBA == 100) {
				memset(lpBuf, 0xff, 2352);
				bProcessRet = RETURNED_EXIST_C2_ERROR;
			}
#endif
			if (pC2ErrorPerSector && bProcessRet == RETURNED_EXIST_C2_ERROR) {
#if 0
				INT nTmpLBA = nLBA + pC2Error.cSlideSectorNum;
				OutputErrorString(
					_T(" Detected C2 error. LBA[%06d, %#07x]\n"), nTmpLBA, nTmpLBA);
				SetAndOutputC2ErrorDataPerSector(&pC2Error, pC2ErrorPerSector,
					nTmpLBA, pDevice->TRANSFER.dwAllBufLen, uiC2ErrorLBACnt);
#else
				OutputErrorString(
					_T(" Detected C2 error. LBA[%06d, %#07x]\n"), nLBA, nLBA);
				SetAndOutputC2ErrorDataPerSector(pC2ErrorPerSector,
					nLBA, pDevice->TRANSFER.dwAllBufLen, uiC2ErrorLBACnt);
#endif
				uiC2ErrorLBACnt++;
				if (uiC2ErrorLBACnt == pExtArg->dwMaxC2ErrorNum) {
					OutputErrorString(_T("C2 error Max: %u\n"), uiC2ErrorLBACnt);
					throw FALSE;
				}
			}
			else if (bProcessRet == RETURNED_SKIP_LBA) {
				if (pExtArg->bReverse) {
					nLBA = pDisc->SCSI.nFirstLBAof2ndSession - SESSION_TO_SESSION_SKIP_LBA;
				}
				else {
					nLBA = pDisc->MAIN.nFixFirstLBAof2ndSession - 1;
					nFirstLBA = nLBA;
				}
			}
			else if (bProcessRet == RETURNED_CONTINUE) {
				WriteErrorBuffer(pExtArg, pDevice, pDisc, pMainHeader, lpBuf,
					aScrambledBuf, lpSubcode, nLBA, fpImg, fpSub, fpC2);
				UpdateTmpMainHeader(pMainHeader);
				if (prevSubQ.byIndex == 0) {
					prevSubQ.nRelativeTime--;
				}
				else {
					prevSubQ.nRelativeTime++;
				}
				prevSubQ.nAbsoluteTime++;
			}
			else if (bProcessRet == RETURNED_FALSE) {
				throw FALSE;
			}
			if (bProcessRet != RETURNED_CONTINUE &&
				bProcessRet != RETURNED_SKIP_LBA) {
				SUB_Q subQ = { 0 };
				SetSubQDataFromReadCD(pDevice->wDriveBufSize, 
					pDisc->MAIN.uiMainDataSlideSize, pMainHeader, &subQ, lpBuf, lpSubcode);
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
					SetSubQDataFromReadCD(pDevice->wDriveBufSize,
						pDisc->MAIN.uiMainDataSlideSize, pMainHeader, &nextSubQ, lpNextBuf, lpNextSubcode);
					if (nFirstLBAForSub <= nLBA && nLBA < pDisc->SCSI.nAllLength) {
						if (nLBA < pDisc->SCSI.nFirstLBAofLeadout ||
							pDisc->SCSI.nFirstLBAof2ndSession <= nLBA) {
							BOOL bLibCrypt = IsValidLibCryptSector(pExtArg->bLibCrypt, nLBA);
							if (!pExtArg->bReverse) {
								CheckAndFixSubChannel(pExecType, pExtArg, pDisc, lpSubcode,
									&nextSubQ, &subQ, &prevSubQ, &prevPrevSubQ,
									byCurrentTrackNum, nLBA, bLibCrypt);
								BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
								// fix raw subchannel
								AlignColumnSubcode(lpSubcode, lpSubcodeRaw);
#if 0
								OutputCDSub96Align(lpSubcode, nLBA->fpError);
#endif
								WriteSubChannel(pDisc, lpSubcodeRaw,
									lpSubcode, nLBA, byCurrentTrackNum, fpSub, fpParse);
							}
							PreserveTrackAttribution(pExtArg, pDisc, nLBA,
								&byCurrentTrackNum, pMainHeader, &subQ, &prevSubQ);
							UpdateSubQData(&subQ, &prevSubQ, &prevPrevSubQ, bLibCrypt);
						}
					}
					CheckMainChannel(pDisc, lpBuf, &subQ, 
						&prevSubQ, &prevPrevSubQ, byCurrentTrackNum, nLBA);
					// Write track to scrambled
					WriteMainChannel(pExtArg, pDisc, lpBuf, nLBA, fpImg);
					if (pExtArg->bC2 && pDevice->bC2ErrorData) {
						WriteC2(pDisc, lpBuf + pDevice->TRANSFER.dwBufC2Offset, nLBA, fpC2);
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
		if (pExtArg->bC2 && pDevice->bC2ErrorData) {
			FcloseAndNull(fpC2);
		}
		FcloseAndNull(fpSub);
		FcloseAndNull(fpParse);
#ifndef _DEBUG
		FlushLog();
#endif
		if (!pExtArg->bReverse) {
			if (pDisc->SCSI.toc.FirstTrack == pDisc->SCSI.toc.LastTrack) {
				pDisc->SUB.lpFirstLBAListOnSub[0][1] = pDisc->SCSI.lpLastLBAListOnToc[0];
			}
			for (INT i = 0; i < pDisc->SCSI.toc.LastTrack; i++) {
				BOOL bErr = FALSE;
				LONG lLine = 0;
				if (pDisc->SUB.lpFirstLBAListOnSub[i][1] == -1) {
					bErr = TRUE;
					lLine = __LINE__;
				}
				else if ((pDisc->SCSI.toc.TrackData[i].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					if (pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[i] == -1) {
						bErr = TRUE;
						lLine = __LINE__;
					}
					else if (pDisc->SUB.lpLastLBAListOfDataTrackOnSub[i] == -1) {
						bErr = TRUE;
						lLine = __LINE__;
					}
				}
				if (bErr) {
					OutputErrorString(
						_T("[L:%d] Internal error. Failed to analyze the subchannel. Track[%02u]/[%02u]\n"),
						lLine, i + 1, pDisc->SCSI.toc.LastTrack);
					throw FALSE;
				}
			}
		}
		// c2 error: reread sector 
		if (pExtArg->bC2 && pDevice->bC2ErrorData) {
			if (!ReadCDForRereadingSector(pExecType, pExtArg, pDevice, pDisc,
				pMainHeader, &pC2Error, pC2ErrorPerSector, uiC2ErrorLBACnt, lpBuf, lpNextBuf,
				lpCmd, &prevSubQ, lpSubcode, lpNextSubcode, fpImg)) {
				throw FALSE;
			}
		}
		FcloseAndNull(fpImg);

		if (!pExtArg->bReverse) {
			OutputTocWithPregap(pDisc);
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
			DWORD dwRoop = GetFileSize(0, fpImg_r) - CD_RAW_SECTOR_SIZE * 2;
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
			rewind(fpImg_r);
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
		// audio only -> from .scm to .img. other descramble img.
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
			DescrambleMainChannel(pExtArg, pDisc, aScrambledBuf, fpImg);
			FcloseAndNull(fpImg);
			_TCHAR str[_MAX_PATH] = { 0 };
			_TCHAR cmd[6] = { _T("check") };
			if (pExtArg->bReadContinue) {
				ZeroMemory(cmd, sizeof(cmd));
				_tcsncpy(cmd, _T("fix"), 3);
			}
			if (GetEccEdcCmd(str, _MAX_PATH, cmd, pszImgPath)) {
				_tsystem(str);
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
				pszImgName, fpImg, fpCue, fpCueForImg, fpCcd)) {
				throw FALSE;
			}
		}
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpImg);
	if (pExtArg->bC2 && pDevice->bC2ErrorData) {
		FcloseAndNull(fpC2);
	}
	FcloseAndNull(fpCueForImg);
	FcloseAndNull(fpCue);
	FcloseAndNull(fpParse);
	FcloseAndNull(fpSub);
	FreeAndNull(pBuf);
	FreeAndNull(pNextBuf);
	TerminateC2ErrorDataPerSector(pExtArg, pDevice, &pC2ErrorPerSector);

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
		OutputCDOffset(pExtArg, pDisc, bGetDrive, nDriveSampleOffset, nDriveOffset);
		bRet = TRUE;
	}
	else {
		LPBYTE pBuf = NULL;
		LPBYTE lpBuf = NULL;
		if (!GetAlignedAllocatedBuffer(pDevice, &pBuf,
			CD_RAW_SECTOR_SIZE, &lpBuf, _T(__FUNCTION__), __LINE__)) {
			return FALSE;
		}
		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		if (pDevice->byPlexType) {
			CDB::_PLXTR_READ_CDDA cdb = { 0 };
			SetReadD8Command(NULL, pDevice, &cdb, 1, FALSE, TRUE);
			memcpy(lpCmd, &cdb, sizeof(lpCmd));
		}
		else {
			CDB::_READ_CD cdb = { 0 };
			SetReadCDCommand(NULL, pDevice, &cdb, READ_CD_FLAG::CDDA, 1, READ_CD_FLAG::SubNone, TRUE);
			memcpy(lpCmd, &cdb, sizeof(lpCmd));
		}

		if (*pExecType == gd) {
			pDisc->SCSI.nFirstLBAofDataTrack = FIRST_LBA_FOR_GD;
		}

		try {
			if (!ExecReadCD(pDevice, lpCmd, pDisc->SCSI.nFirstLBAofDataTrack, lpBuf,
				CD_RAW_SECTOR_SIZE, _T(__FUNCTION__), __LINE__)) {
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
			OutputDiscLogA(
				"============================ Check Drive+CD offset ============================\n");
			OutputCDMain2352(lpBuf, pDisc->SCSI.nFirstLBAofDataTrack);

			BYTE aBuf2[CD_RAW_SECTOR_SIZE * 2] = { 0 };
			memcpy(aBuf2, lpBuf, CD_RAW_SECTOR_SIZE);

			if (!ExecReadCD(pDevice, lpCmd, pDisc->SCSI.nFirstLBAofDataTrack + 1, lpBuf,
				CD_RAW_SECTOR_SIZE, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			memcpy(aBuf2 + CD_RAW_SECTOR_SIZE, lpBuf, CD_RAW_SECTOR_SIZE);
			if (!GetWriteOffset(pDisc, aBuf2)) {
				OutputErrorString(_T("Failed to get write-offset\n"));
				throw FALSE;
			}
			OutputCDOffset(pExtArg, pDisc, bGetDrive, nDriveSampleOffset, nDriveOffset);
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
	PMAIN_HEADER pMainHeader,
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
	FILE* fpBin = 
		CreateOrOpenFileW(pszPath, NULL, NULL, NULL, NULL, szExt, _T("wb"), 0, 0);
	if (!fpBin) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	FILE* fpC2 = NULL;
	FILE* fpParse = NULL;
	FILE* fpSub = NULL;
	BOOL bRet = TRUE;
	LPBYTE pBuf = NULL;
	LPBYTE pNextBuf = NULL;
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector = NULL;
	BYTE aScrambledBuf[CD_RAW_SECTOR_SIZE] = { 0 };
	try {
		FILE* fpTbl = NULL;
		if (NULL == (fpTbl = OpenProgrammabledFile(_T("scramble.bin"), _T("rb")))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		fread(aScrambledBuf, sizeof(BYTE), sizeof(aScrambledBuf), fpTbl);
		FcloseAndNull(fpTbl);

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

		SetCDTransfer(pDevice, DRIVE_DATA_ORDER::C2None);
		C2_ERROR pC2Error = { 0 };
		if (!bCheckReading && pExtArg->bC2 && pDevice->bC2ErrorData) {
			if (NULL == (fpC2 = CreateOrOpenFileW(
				pszPath, NULL, NULL, NULL, NULL, _T(".c2"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			SetCDTransfer(pDevice, DRIVE_DATA_ORDER::MainC2Sub);
			DRIVE_DATA_ORDER dataOrder = DRIVE_DATA_ORDER::MainC2Sub;

			if (!InitC2ErrorData(pExtArg, pDisc, &pC2Error, 
				&pC2ErrorPerSector, pDevice->TRANSFER.dwAllBufLen)) {
				throw FALSE;
			}
			if (!ReadCDForCheckingByteOrder(pExtArg, pDevice, &dataOrder)) {
				TerminateC2ErrorDataPerSector(pExtArg, pDevice, &pC2ErrorPerSector);
				pDevice->bC2ErrorData = FALSE;
				SetCDTransfer(pDevice, DRIVE_DATA_ORDER::C2None);
			}
			if (dataOrder == DRIVE_DATA_ORDER::MainSubC2) {
				OutputDriveLogA(
					"Byte order of this drive is main + sub + c2\n");
				SetCDTransfer(pDevice, DRIVE_DATA_ORDER::MainSubC2);
			}
			else {
				OutputDriveLogA(
					"Byte order of this drive is main + c2 + sub\n");
			}
		}

		// store main+(c2)+sub data
		LPBYTE lpBuf = NULL;
		if (!GetAlignedAllocatedBuffer(pDevice, &pBuf,
			pDevice->TRANSFER.dwAllBufLen, &lpBuf, _T(__FUNCTION__), __LINE__)) {
			return FALSE;
		}

		LPBYTE lpNextBuf = NULL;
		if (pDevice->wDriveBufSize > MINIMUM_DRIVE_BUF_SIZE) {
			if (!GetAlignedAllocatedBuffer(pDevice, &pNextBuf,
				pDevice->TRANSFER.dwAllBufLen, &lpNextBuf, _T(__FUNCTION__), __LINE__)) {
				return FALSE;
			}

		}

		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		_TCHAR szSubCode[5] = { 0 };
		if (pDevice->byPlexType && !pDisc->SCSI.bAudioOnly) {
			CDB::_PLXTR_READ_CDDA cdb = { 0 };
			SetReadD8Command(pExtArg, pDevice, &cdb, 1, TRUE, bCheckReading);
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
			SetReadCDCommand(pExtArg, pDevice, &cdb,
				flg, 1, READ_CD_FLAG::Raw, bCheckReading);
			if (cdb.SubChannelSelection == READ_CD_FLAG::Raw) {
				_tcsncpy(szSubCode, _T("Raw"), 3);
			}
			memcpy(lpCmd, &cdb, sizeof(lpCmd));
		}
		OutputString(_T("Read command: %#x, Subcode reading mode: %s\n"), lpCmd[0], szSubCode);

		BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		SUB_Q prevSubQ = { 0 };

		if (!ExecReadCD(pDevice, lpCmd, nStart - 1, lpBuf,
			pDevice->TRANSFER.dwAllBufLen, _T(__FUNCTION__), __LINE__)) {
			throw FALSE;
		}
		AlignRowSubcode(lpBuf + pDevice->TRANSFER.dwBufSubOffset, lpSubcode);
		SetSubQDataFromReadCD(pDevice->wDriveBufSize,
			pDisc->MAIN.uiMainDataSlideSize, pMainHeader, &prevSubQ, lpBuf, lpSubcode);

		if (*pExecType == gd) {
			for (INT p = pDisc->GDROM_TOC.FirstTrack - 1; p < pDisc->GDROM_TOC.LastTrack; p++) {
				pDisc->SUB.lpEndCtlList[p] = pDisc->GDROM_TOC.TrackData[p].Control;
			}
		}
		else {
			for (UINT p = 0; p < pDisc->SCSI.toc.LastTrack; p++) {
				if (!ExecReadCD(pDevice, lpCmd, pDisc->SCSI.lpFirstLBAListOnToc[p], lpBuf,
					pDevice->TRANSFER.dwAllBufLen, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
				AlignRowSubcode(lpBuf + pDevice->TRANSFER.dwBufSubOffset, lpSubcode);
				pDisc->SUB.lpEndCtlList[p] = (BYTE)((lpSubcode[12] >> 4) & 0x0f);
			}
		}
		SetCDOffset(pDisc, nStart, nEnd);
		BYTE byCurrentTrackNum = prevSubQ.byTrackNum;
		if (*pExecType == gd) {
			byCurrentTrackNum = pDisc->GDROM_TOC.FirstTrack;
			// because address out of range
			pDisc->MAIN.nOffsetEnd = 0;
		}
		INT nLBA = nStart + pDisc->MAIN.nOffsetStart;
		INT nLastLBA = nEnd + pDisc->MAIN.nOffsetEnd;
#ifndef _DEBUG
		FlushLog();
#endif
		BYTE lpNextSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		UINT uiC2ErrorLBACnt = 0;
		SUB_Q prevPrevSubQ = { 0 };

		while (nLBA <= nLastLBA) {
			BOOL bProcessRet = ProcessReadCD(pExecType, pExtArg, pDevice, pDisc, 
				pMainHeader, &pC2Error, pC2ErrorPerSector, uiC2ErrorLBACnt, lpCmd,
				nLBA, lpBuf, lpNextBuf,&prevSubQ, lpSubcode, lpNextSubcode, bCheckReading);
			if (pC2ErrorPerSector && bProcessRet == RETURNED_EXIST_C2_ERROR) {
#if 0
				INT nTmpLBA = nLBA + pC2Error.cSlideSectorNum;
				OutputErrorString(
					_T(" Detected C2 error. LBA[%06d, %#07x]\n"), nTmpLBA, nTmpLBA);
				SetAndOutputC2ErrorDataPerSector(&pC2Error, pC2ErrorPerSector,
					nTmpLBA, pDevice->TRANSFER.dwAllBufLen, uiC2ErrorLBACnt);
#else
				OutputErrorString(
					_T(" Detected C2 error. LBA[%06d, %#07x]\n"), nLBA, nLBA);
				SetAndOutputC2ErrorDataPerSector(pC2ErrorPerSector,
					nLBA, pDevice->TRANSFER.dwAllBufLen, uiC2ErrorLBACnt);
#endif
				uiC2ErrorLBACnt++;
				if (uiC2ErrorLBACnt == pExtArg->dwMaxC2ErrorNum) {
					OutputErrorString(_T("C2 error Max: %u\n"), uiC2ErrorLBACnt);
					throw FALSE;
				}
#ifdef _DEBUG
				OutputCDMain2352(lpBuf, nLBA);
#endif
			}
			else if (bProcessRet == RETURNED_SKIP_LBA) {
				nLBA = pDisc->MAIN.nFixFirstLBAof2ndSession - 1;
			}
			else if (bProcessRet == RETURNED_FALSE) {
				throw FALSE;
			}

			if (bProcessRet != RETURNED_CONTINUE &&
				bProcessRet != RETURNED_SKIP_LBA) {
				SUB_Q subQ = { 0 };
				SetSubQDataFromReadCD(pDevice->wDriveBufSize,
					pDisc->MAIN.uiMainDataSlideSize, pMainHeader, &subQ, lpBuf, lpSubcode);
				if (nStart <= nLBA && nLBA < nEnd) {
					if (nLBA < pDisc->SCSI.nFirstLBAofLeadout ||
						pDisc->SCSI.nFirstLBAof2ndSession <= nLBA) {
						SUB_Q nextSubQ = { 0 };
						SetSubQDataFromReadCD(pDevice->wDriveBufSize,
							pDisc->MAIN.uiMainDataSlideSize, pMainHeader, &nextSubQ, lpNextBuf, lpNextSubcode);
						CheckAndFixSubChannel(pExecType, pExtArg, pDisc, lpSubcode, &nextSubQ,
							&subQ, &prevSubQ, &prevPrevSubQ, byCurrentTrackNum, nLBA, FALSE);
						BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
						// fix raw subchannel
						AlignColumnSubcode(lpSubcode, lpSubcodeRaw);
#if 0
						OutputCDSub96Align(lpSubcode, nLBA->fpError);
#endif
						if (!pExtArg->bReverse) {
							WriteSubChannel(pDisc, lpSubcodeRaw,
								lpSubcode, nLBA, byCurrentTrackNum, fpSub, fpParse);
						}
						if (*pExecType == gd) {
							byCurrentTrackNum = subQ.byTrackNum;
						}
						else {
							PreserveTrackAttribution(pExtArg, pDisc, nLBA,
								&byCurrentTrackNum, pMainHeader, &subQ, &prevSubQ);
						}
						UpdateSubQData(&subQ, &prevSubQ, &prevPrevSubQ, FALSE);
					}
				}
				CheckMainChannel(pDisc, lpBuf, &subQ,
					&prevSubQ, &prevPrevSubQ, byCurrentTrackNum, nLBA);
				WriteMainChannel(pExtArg, pDisc, lpBuf, nLBA, fpBin);
				if (!bCheckReading && pExtArg->bC2 && pDevice->bC2ErrorData) {
					WriteC2(pDisc, lpBuf + pDevice->TRANSFER.dwBufC2Offset, nLBA, fpC2);
				}
			}
			OutputString(_T("\rCreating bin from %d to %d (LBA) %6d"), 
				nStart + pDisc->MAIN.nOffsetStart, nEnd + pDisc->MAIN.nOffsetEnd, nLBA);
			nLBA++;
		}
		OutputString(_T("\n"));
		if (pExtArg->bC2 && pDevice->bC2ErrorData) {
			FcloseAndNull(fpC2);
		}
		FcloseAndNull(fpParse);
		FcloseAndNull(fpSub);
#ifndef _DEBUG
		FlushLog();
#endif
		if (!bCheckReading) {
			if (pDevice->wDriveBufSize > MINIMUM_DRIVE_BUF_SIZE &&
				pExtArg->bC2 && pDevice->bC2ErrorData) {
				if (!ReadCDForRereadingSector(pExecType, pExtArg, pDevice,
					pDisc, pMainHeader, &pC2Error, pC2ErrorPerSector, uiC2ErrorLBACnt,
					lpBuf, lpNextBuf, lpCmd, NULL, lpSubcode, lpNextSubcode, fpBin)) {
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
	FreeAndNull(pNextBuf);
	TerminateC2ErrorDataPerSector(pExtArg, pDevice, &pC2ErrorPerSector);

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

BOOL ReadCDForGDTOC(
	PDEVICE pDevice,
	PDISC pDisc
	)
{
	CDB::_READ_CD cdb = { 0 };
	SetReadCDCommand(NULL, pDevice, &cdb,
		READ_CD_FLAG::All, 1, READ_CD_FLAG::SubNone, TRUE);
	BYTE aToc[CD_RAW_SECTOR_SIZE] = { 0 };
	FILE* fpTbl = NULL;
	if (NULL == (fpTbl = OpenProgrammabledFile(_T("scramble.bin"), _T("rb")))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BYTE bufTbl[CD_RAW_SECTOR_SIZE] = { 0 };
	fread(bufTbl, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpTbl);
	FcloseAndNull(fpTbl);

	if (!ExecReadCD(pDevice, (LPBYTE)&cdb, FIRST_LBA_FOR_GD, aToc,
		CD_RAW_SECTOR_SIZE, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}

	BYTE bufDec[CD_RAW_SECTOR_SIZE] = { 0 };
	INT idx = pDisc->MAIN.nCombinedOffset;
	for (INT j = 0; j < CD_RAW_SECTOR_SIZE; j++) {
		bufDec[j] = (BYTE)(aToc[idx + j] ^ bufTbl[j]);
	}
	/*
		0x110 - 0x113: TOC1
		0x114 - 0x116: LBA(little) |
		0x117        : Ctl/Adr     |
		:                          |-> 100 track
		:                          |
		0x294 - 0x296: LBA(little) |
		0x297        : Ctl/Adr     |
		0x298 - 0x299: Zero
		0x29a        : First track |-> alway "3"
		0x29b        : Ctl/Adr     |-> alway "41"
		0x29c - 0x29d: Zero
		0x29e        : Last track
		0x29f        : Ctl/Adr
		0x2a0 - 0x2a2: Max LBA     |-> alway "b4 61 08" (549300)
		0x2a3        : Ctl/Adr     |-> alway "41"
	*/
	if (bufDec[0x110] != 'T' || bufDec[0x111] != 'O' ||
		bufDec[0x112] != 'C' || bufDec[0x113] != '1') {
		OutputErrorString(_T("Not GD-ROM data\n"));
		return FALSE;
	}
	pDisc->GDROM_TOC.FirstTrack = bufDec[0x29a];
	pDisc->GDROM_TOC.LastTrack = bufDec[0x29e];
	pDisc->GDROM_TOC.Length = MAKELONG(
		MAKEWORD(bufDec[0x2a0], bufDec[0x2a1]), MAKEWORD(bufDec[0x2a2], 0));

	for (INT i = pDisc->GDROM_TOC.FirstTrack - 1, j = 0; i < pDisc->GDROM_TOC.LastTrack; i++, j += 4) {
		pDisc->GDROM_TOC.TrackData[i].Address = MAKELONG(
			MAKEWORD(bufDec[0x114 + j], bufDec[0x115 + j]), MAKEWORD(bufDec[0x116 + j], 0));
		pDisc->GDROM_TOC.TrackData[i].Control = BYTE((bufDec[0x117 + j]) >> 4 & 0x0f);
		pDisc->GDROM_TOC.TrackData[i].Adr = BYTE((bufDec[0x117 + j]) & 0x0f);
		pDisc->GDROM_TOC.TrackData[i].TrackNumber = (BYTE)(i + 1);
	}
	OutputTocForGD(pDisc);
	return TRUE;
}
