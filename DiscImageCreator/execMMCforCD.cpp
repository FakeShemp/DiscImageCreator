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
#if 0
#include "outputGD.h"
#endif
#include "outputMMCLog.h"
#include "set.h"

BOOL ProcessReadCD(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	PCD_OFFSET_DATA pCdOffsetData,
	PC2_ERROR_DATA pC2ErrorData,
	PREAD_CD_TRANSFER_DATA pTransferData,
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
	BOOL bRet = RETURNED_C2_ERROR_1ST_NONE;
	if (*pExecType != rgd) {
		if (pDiscData->nStartLBAof2ndSession != -1 && pDiscData->nLastLBAof1stSession == nLBA) {
			INT nStartSession2 = pDiscData->nStartLBAof2ndSession - 1;
			OutputErrorLog(
				_T("Skip from Leadout of Session 1 [%d] to Leadin of Session 2 [%d]\n"),
				pDiscData->nLastLBAof1stSession, nStartSession2);
			pPrevSubQ->nAbsoluteTime = nLBA + 11400 + 150 - 1;
			return RETURNED_C2_ERROR_SKIP_LBA;
		}
	}
	try {
		for (INT i = 0; i < pTransferData->byTransferLen; i++) {
			// 1st:Read main & sub channel
			lpCmd[2] = HIBYTE(HIWORD(nLBA + i));
			lpCmd[3] = LOBYTE(HIWORD(nLBA + i));
			lpCmd[4] = HIBYTE(LOWORD(nLBA + i));
			lpCmd[5] = LOBYTE(LOWORD(nLBA + i));
			BYTE byScsiStatus = 0;
			if (!ScsiPassThroughDirect(pDevData, lpCmd, CDB12GENERIC_LENGTH, lpBufTmp,
				pTransferData->dwBufLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)) {
				throw RETURNED_C2_ERROR_FALSE;
			}

			if (byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				if (*pExecType != rgd && !bCheckReading) {
					UINT uiSize = 0;
					ZeroMemory(lpBufTmp, pTransferData->dwBufLen);
					if (nLBA == pCdOffsetData->nFixStartLBA) {
						uiSize = CD_RAW_SECTOR_SIZE - pCdOffsetData->uiMainDataSlideSize;
						fwrite(lpBufTmp + pCdOffsetData->uiMainDataSlideSize,
							sizeof(BYTE), uiSize, fpImg);
					}
					else if (nLBA == pDiscData->nAllLength + pCdOffsetData->nFixEndLBA - 1) {
						uiSize = pCdOffsetData->uiMainDataSlideSize;
						fwrite(lpBufTmp, sizeof(BYTE), uiSize, fpImg);
					}
					else {
						uiSize = CD_RAW_SECTOR_SIZE;
						fwrite(lpBufTmp, sizeof(BYTE), uiSize, fpImg);
					}
					OutputErrorString(
						_T("[F:%s][L:%d] LBA %6d Read error. Zero padding [%dbyte]\n"), 
						_T(__FUNCTION__), __LINE__, nLBA, uiSize);
					if (pPrevSubQ) {
						if (pPrevSubQ->byIndex == 0) {
							pPrevSubQ->nRelativeTime--;
						}
						else {
							pPrevSubQ->nRelativeTime++;
						}
						pPrevSubQ->nAbsoluteTime++;
					}
					throw RETURNED_C2_ERROR_CONTINUE;
				}
				else {
					throw RETURNED_C2_ERROR_FALSE;
				}
			}
			memcpy(lpBuf + pTransferData->dwBufLen * i, lpBufTmp, pTransferData->dwBufLen);
#if 0
			OutputMmcCDMain2352(
				lpBuf + pTransferData->dwBufLen * i, nLBA->fpError);
			OutputMmcCDC2Error296(
				lpBuf + pTransferData->dwBufLen * i + pTransferData->dwBufC2Offset,
				nLBA->fpError);
#endif
		}
	}
	catch(BOOL bErr) {
		bRet = bErr;
	}

	if (bRet == RETURNED_C2_ERROR_1ST_NONE && !bCheckReading) {
		if (pC2ErrorData && pDevData->bC2ErrorData && pExtArg->bC2) {
			bRet = CheckC2Error(pDiscData,
				pC2ErrorData, pTransferData, lpBuf, uiC2ErrorLBACnt);
		}
		AlignRowSubcode(lpBuf + pTransferData->dwBufSubOffset, lpSubcode);
		if (pTransferData->byTransferLen > 1) {
			AlignRowSubcode(lpBuf + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE
				+ pTransferData->dwBufSubOffset, lpNextSubcode);
		}
	}
	return bRet;
}

BOOL ReadCDForCheckingIndex0InTrack1(
	PDEVICE_DATA pDevData,
	PEXT_ARG pExtArg,
	LPBOOL bOK
	)
{
	BOOL bRet = TRUE;
	if (pExtArg->bPre) {
		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		SetReadCDCommand(pDevData, NULL, lpCmd, FALSE, READ_CD_FLAG::All, 1);
		LPBYTE pBuf = NULL;
		if (NULL == (pBuf = (LPBYTE)calloc(
			CD_RAW_SECTOR_WITH_SUBCODE_SIZE + pDevData->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevData, pBuf);

		BYTE byScsiStatus = 0;
		try {
			if (!ScsiPassThroughDirect(pDevData, lpCmd, CDB12GENERIC_LENGTH, lpBuf, 
				CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			else {
				BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
				AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
				SUB_Q_DATA pSubQ = { 0 };
				pSubQ.byIndex = BcdToDec(lpSubcode[14]);
				if (pSubQ.byIndex == 0) {
					*bOK = TRUE;
				}
			}
		}
		catch(BOOL bErr) {
			bRet = bErr;
		}
		FreeAndNull(pBuf);
	}
	return bRet;
}

BOOL ReadCDForCheckingCDG(
	PDEVICE_DATA pDevData,
	LPBOOL lpCDG
	)
{
	BOOL bRet = TRUE;
	BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
	SetReadCDCommand(pDevData, NULL, lpCmd, FALSE, READ_CD_FLAG::All, 1);

	LPBYTE pBuf = NULL;
	if (NULL == (pBuf = (LPBYTE)calloc(
		CD_RAW_SECTOR_WITH_SUBCODE_SIZE + pDevData->AlignmentMask, sizeof(BYTE)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevData, pBuf);
	lpCmd[5] = 75;

	BYTE byScsiStatus = 0;
	try {
		if (!ScsiPassThroughDirect(pDevData, lpCmd, CDB12GENERIC_LENGTH, lpBuf, 
			CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw FALSE;
		}
		else {
			INT nCDG = 0;
			for (INT i = 0; i < 4; i++) {
				nCDG += *(lpBuf + CD_RAW_SECTOR_SIZE + i * 24);
			}
			if (nCDG == 0xfc) {
				// Why R-W bit is full? If it isn't CD+G or CD-MIDI, R-W bit should be off...
				//  Alanis Morissette - Jagged Little Pill (UK)
				//  WipEout 2097: The Soundtrack
				//  and more..
				*lpCDG = FALSE;
			}
			else if (nCDG > 0 && nCDG != 0x200) {
				*lpCDG = TRUE;
			}
		}
	}
	catch(BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(pBuf);
	return bRet;
}

BOOL ReadCDForCheckingByteOrder(
	PDEVICE_DATA pDevData,
	PEXT_ARG pExtArg,
	READ_CD_FLAG::PDATA_ORDER pOrder
	)
{
	BOOL bRet = TRUE;
	BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
	if (pDevData->bPlextor) {
		SetReadD8Command(pDevData, pExtArg, lpCmd, 1, FALSE);
	}
	else {
		// non plextor && support scrambled ripping
		SetReadCDCommand(pDevData, pExtArg, lpCmd, FALSE, READ_CD_FLAG::CDDA, 1);
	}

	LPBYTE pBuf = NULL;
	if (NULL == (pBuf = (LPBYTE)calloc(
		CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE + pDevData->AlignmentMask, sizeof(BYTE)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevData, pBuf);
	BYTE byScsiStatus = 0;
	try {
		if (!ScsiPassThroughDirect(pDevData, lpCmd, CDB12GENERIC_LENGTH, 
			lpBuf, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, &byScsiStatus, 
			_T(__FUNCTION__), __LINE__) || byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			OutputString(_T("This drive don't support C2 error report\n"));
			throw FALSE;
		}
#if 0
		OutputDriveLog(_T("=====Check main+c2+sub=====\n"));
		OutputMmcCDC2Error296(lpBuf + CD_RAW_SECTOR_SIZE, 0);
		OutputMmcCDSub96Raw(lpBuf + CD_RAW_SECTOR_WITH_C2_294_SIZE, 0);

		OutputDriveLog(_T("=====Check main+sub+c2=====\n"));
		OutputMmcCDSub96Raw(lpBuf + CD_RAW_SECTOR_SIZE, 0);
		OutputMmcCDC2Error296(lpBuf + CD_RAW_SECTOR_WITH_SUBCODE_SIZE, 0);
#endif
		BYTE subcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		memcpy(subcode, lpBuf + CD_RAW_SECTOR_WITH_C2_294_SIZE, CD_RAW_READ_SUBCODE_SIZE);
		// check main+c2+sub order
		for (INT i = 0; i < CD_RAW_READ_SUBCODE_SIZE; i++) {
			if (!(subcode[i] & 0x80)) {
				*pOrder = READ_CD_FLAG::MainSubC2;
				break;
			}
		}

	}
	catch(BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(pBuf);
	return bRet;
}

BOOL ReadCDForCheckingReadInOut(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	LPCTSTR pszPath
	)
{
	BOOL bRet = TRUE;
	if (0 < pDiscData->nCombinedOffset) {
		OutputString(_T("Checking reading lead-out\n"));
		bRet = ReadCDPartial(pExecType, pExtArg, pDevData, pDiscData, 
			pszPath, pDiscData->nAllLength, pDiscData->nAllLength, READ_CD_FLAG::CDDA, TRUE);
		if (bRet) {
			OutputDriveLog(_T("Reading lead-out: OK\n"));
			OutputString(_T("Reading lead-out: OK\n"));
		}
		else {
			OutputDriveLog(_T("Reading lead-out: NG\n"));
			OutputString(_T("Reading lead-out: NG\n"));
			bRet = FALSE;
		}
	}
	else if (pDiscData->nCombinedOffset < 0) {
		OutputString(_T("Checking reading lead-in\n"));
		bRet = ReadCDPartial(pExecType, pExtArg, pDevData, pDiscData, 
			pszPath, 0, -1, READ_CD_FLAG::CDDA, TRUE);
		if (bRet) {
			OutputDriveLog(_T("Reading lead-in: OK\n"));
			OutputString(_T("Reading lead-in: OK\n"));
		}
		else {
			OutputDriveLog(_T("Reading lead-in: NG\n"));
			OutputString(_T("Reading lead-in: NG\n"));
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
	BOOL bRet = TRUE;
	BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
	// Drive cache flush??
	lpCmd[0] = SCSIOP_READ12;
	lpCmd[1] = CDB_FORCE_MEDIA_ACCESS;
	lpCmd[2] = HIBYTE(HIWORD(pDiscData->nFirstDataLBA + 675));
	lpCmd[3] = LOBYTE(HIWORD(pDiscData->nFirstDataLBA + 675));
	lpCmd[4] = HIBYTE(LOWORD(pDiscData->nFirstDataLBA + 675));
	lpCmd[5] = LOBYTE(LOWORD(pDiscData->nFirstDataLBA + 675));
	lpCmd[9] = 32;
	LPBYTE pBuf = NULL;
	if (NULL == (pBuf = (LPBYTE)calloc(
		DISC_RAW_READ * 32 + pDevData->AlignmentMask, sizeof(BYTE)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevData, pBuf);
	BYTE byScsiStatus = 0;
	try {
		if (!ScsiPassThroughDirect(pDevData, lpCmd, CDB12GENERIC_LENGTH, lpBuf, 
			(DWORD)DISC_RAW_READ * 32, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw FALSE;
		}
	}
	catch(BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(pBuf);
	return bRet;
}

BOOL ReadCDForVolumeDescriptor(
	PDEVICE_DATA pDevData,
	PCDROM_TOC pToc
	)
{
	BOOL bRet = TRUE;
	if ((pToc->TrackData[0].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		SetReadCDCommand(pDevData, NULL, lpCmd, FALSE, READ_CD_FLAG::All, 1);

		LPBYTE pBuf = NULL;
		if (NULL == (pBuf = (LPBYTE)calloc(
			CD_RAW_SECTOR_WITH_SUBCODE_SIZE + pDevData->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevData, pBuf);
		BYTE byScsiStatus = 0;
		try {
			if (!ScsiPassThroughDirect(pDevData, lpCmd, CDB12GENERIC_LENGTH, lpBuf, 
				CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			if (Is3DOData(lpBuf)) {
				throw TRUE;
			}
			else {
				lpCmd[5] = 1;
				if (!ScsiPassThroughDirect(pDevData, lpCmd, CDB12GENERIC_LENGTH, lpBuf, 
					CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
					|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
					throw FALSE;
				}
				if (IsMacData(lpBuf + 1040)) {
					OutputFsMasterDirectoryBlocks(lpBuf, 1040);
				}
				else if (IsMacData(lpBuf + 528)) {
					OutputFsMasterDirectoryBlocks(lpBuf, 528);
				}
				else {
					lpCmd[5] = 16;
					if (!ScsiPassThroughDirect(pDevData, lpCmd, CDB12GENERIC_LENGTH, lpBuf, 
						CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
						|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
						throw FALSE;
					}
					if (lpBuf[15] == DATA_BLOCK_MODE1) {
						OutputFsVolumeDescriptor(lpBuf, SYNC_SIZE + HEADER_SIZE);
						if (lpBuf[SYNC_SIZE + HEADER_SIZE] == 0xFF) {
							throw TRUE;
						}
					}
					else if (lpBuf[15] == DATA_BLOCK_MODE2) {
						OutputFsVolumeDescriptor(lpBuf,
							SYNC_SIZE + HEADER_SIZE + SUBHEADER_SIZE);
						if (lpBuf[SYNC_SIZE + HEADER_SIZE + SUBHEADER_SIZE] == 0xFF) {
							throw TRUE;
						}
					}
				}
			}
		}
		catch(BOOL bErr) {
			bRet = bErr;
		}
		FreeAndNull(pBuf);
	}
	return bRet;
}

BOOL ReadCDForRereadingSector(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	PCD_OFFSET_DATA pCdOffsetData,
	PC2_ERROR_DATA pC2ErrorData,
	PREAD_CD_TRANSFER_DATA pTransferData,
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
			_T("\nChange reading Speed: %dx\n"), pExtArg->uiRereadSpeedNum);
		SetCDSpeed(pDevData, pExtArg->uiRereadSpeedNum);
	}
	UINT uiCnt = 1;
	while(uiC2ErrorLBACnt > 0) {
		if (uiCnt == pExtArg->uiMaxRereadNum) {
			OutputString(_T("Reread reached Max: %d\n"), uiCnt++);
			bProcessRet = RETURNED_C2_ERROR_FALSE;
			break;
		}
		if (!pDiscData->bAudioOnly) {
			ReadCDForFlushingDriveCache(pDevData, pDiscData);
		}
		UINT uiC2ErrorLBACntBackup = uiC2ErrorLBACnt;
		uiC2ErrorLBACnt = 0;
		SetC2ErrorBackup(pC2ErrorData, 
			uiC2ErrorLBACntBackup, pTransferData->dwAllBufLen);
				
		UINT i = 0;
		for (INT nLBA = pC2ErrorData[0].nErrorLBANumBackup; i < uiC2ErrorLBACntBackup; i++) {
			OutputString(
				_T("\rReread times %4d, ErrSectorNum %4d/%4d"), 
				uiCnt, i + 1, uiC2ErrorLBACntBackup);
			nLBA = pC2ErrorData[i].nErrorLBANumBackup;
			bProcessRet = ProcessReadCD(pExecType, pExtArg, pDevData, pDiscData, 
				pCdOffsetData, pC2ErrorData, pTransferData, uiC2ErrorLBACnt, nLBA, lpBuf,
				lpBufTmp, lpCmd, pPrevSubQ, lpSubcode, lpNextSubcode, fpImg, FALSE);
			INT nTmpLBA = nLBA - pDiscData->nAdjustSectorNum;

			if (bProcessRet == RETURNED_C2_ERROR_EXIST) {
				SetAndOutputC2ErrorData(pC2ErrorData, nTmpLBA, uiC2ErrorLBACnt);
				uiC2ErrorLBACnt++;
			}
			else if (bProcessRet == RETURNED_C2_ERROR_1ST_NONE) {
				if (pC2ErrorData[i].bErrorFlagBackup == RETURNED_C2_ERROR_EXIST) {
					SetAndOutputC2NoErrorData(pC2ErrorData, lpBuf, nTmpLBA,
						pTransferData->dwAllBufLen, uiC2ErrorLBACnt, i);
					uiC2ErrorLBACnt++;
				}
				else {
					INT nSlideLBANum = 
						pC2ErrorData[i].cSlideSectorNumBackup - pDiscData->nAdjustSectorNum;
					if (CheckByteError(pC2ErrorData, lpBuf, pTransferData->dwBufLen, nSlideLBANum, i)) {
						SetAndOutputC2NoErrorByteErrorData(pC2ErrorData, lpBuf, nTmpLBA,
							pTransferData->dwAllBufLen, uiC2ErrorLBACnt, i);
						uiC2ErrorLBACnt++;
					}
					else {
						INT nFixLBA = nLBA + nSlideLBANum;
						LONG lPos = LONG(CD_RAW_SECTOR_SIZE * nFixLBA - pDiscData->nCombinedOffset);
						OutputErrorLog(
							_T("LBA %6d, Reread OK: Fix Main data from 0x%08x to 0x%08x\n"),
							nTmpLBA, lPos, lPos + CD_RAW_SECTOR_SIZE - 1);
#ifdef _DEBUG
//						INT nSlideLBANum = 
//							pC2ErrorData[uiC2ErrorLBACnt].cSlideSectorNum - pDiscData->nAdjustSectorNum;
//						INT nFixLBA = nLBA + nSlideLBANum;
						OutputMmcCDMain2352(
							lpBuf + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * abs(pDiscData->nAdjustSectorNum),
							nLBA);
						OutputMmcCDC2Error296(
							lpBuf + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * abs(pDiscData->nAdjustSectorNum) + pTransferData->dwBufC2Offset,
							nLBA);
#endif
						fseek(fpImg, lPos, SEEK_SET);
						// Write track to scrambled again
						WriteMainChannel(pDiscData, 
							lpBuf + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * nSlideLBANum,
							NULL, nFixLBA, pCdOffsetData, fpImg);
#if 0
						OutputMmcCDMain2352(
							lpBuf + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * nSlideLBANum,
							nFixLBA->fpError);
#endif
					}
				}
			}
			else if (bProcessRet == RETURNED_C2_ERROR_FALSE) {
				break;
			}
		}
		OutputString(_T("\n"));
		uiCnt++;
	}
	if (uiCnt == 1 && uiC2ErrorLBACnt == 0) {
		OutputString(_T("C2 error didn't exist\n"));
	}
	else if (uiCnt > 1 && uiC2ErrorLBACnt == 0) {
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
	if (pDiscData->toc.FirstTrack < 1 || 99 < pDiscData->toc.FirstTrack ||
		pDiscData->toc.LastTrack < 1 || 99 < pDiscData->toc.LastTrack) {
		return FALSE;
	}
#if 1
	if (!ReadCDForCheckingReadInOut(pExecType,
		pExtArg, pDevData, pDiscData, pszPath)) {
		return  FALSE;
	}
#endif
	FILE* fpImg = NULL;
	_TCHAR pszOutScmFile[_MAX_PATH] = { 0 };
	if (NULL == (fpImg = CreateOrOpenFileW(
		pszPath, pszOutScmFile, NULL, NULL, _T(".scm"), _T("wb"), 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = TRUE;
	FILE* fpC2 = NULL;
	FILE* fpCdg = NULL;
	FILE* fpCue = NULL;
	FILE* fpParse = NULL;
	FILE* fpSub = NULL;
	FILE* fpTbl = NULL;
	LPINT* lpLBAStartList = NULL;
	LPINT* lpLBAOfDataTrackList = NULL;
	LPBYTE lpCtlList = NULL;
	LPBYTE lpEndCtlList = NULL;
	LPBYTE lpModeList = NULL;
	LPBOOL lpISRCList = NULL;
	LPBYTE pBuf = NULL;
	LPBYTE pBufTmp = NULL;
	PC2_ERROR_DATA pC2ErrorData = NULL;
	try {
		// init start
		if (NULL == (fpCue = CreateOrOpenFileW(
			pszPath, NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpParse = CreateOrOpenFileW(
			pszPath, NULL, NULL, NULL, _T(".sub.txt"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpSub = CreateOrOpenFileW(
			pszPath, NULL, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpTbl = OpenProgrammabledFile(
			_T("scramble.bin"), _T("rb")))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}

		size_t dwTrackAllocSize = (size_t)pDiscData->toc.LastTrack + 1;
		if (NULL == (lpLBAStartList = (LPINT*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (lpLBAOfDataTrackList = (LPINT*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (lpISRCList = (LPBOOL)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (lpCtlList = (LPBYTE)calloc(dwTrackAllocSize, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (lpModeList = (LPBYTE)calloc(dwTrackAllocSize, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (lpEndCtlList = (LPBYTE)calloc(dwTrackAllocSize, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}

		size_t dwIndexAllocSize = (size_t)MAXIMUM_NUMBER_INDEXES * sizeof(UINT_PTR);
		size_t dwRangeAllocSize = (size_t)2 * sizeof(UINT_PTR);
		for (INT h = 0; h < pDiscData->toc.LastTrack + 1; h++) {
			if (NULL == (lpLBAStartList[h] = (LPINT)malloc(dwIndexAllocSize))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == (lpLBAOfDataTrackList[h] = (LPINT)malloc(dwRangeAllocSize))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			FillMemory(lpLBAStartList[h], dwIndexAllocSize, -1);
			FillMemory(lpLBAOfDataTrackList[h], dwRangeAllocSize, -1);
		}

		BOOL bIndex0InTrack1 = FALSE;
		if (!ReadCDForCheckingIndex0InTrack1(pDevData, pExtArg, &bIndex0InTrack1)) {
			OutputErrorString(_T("[F:%s][L:%d] Failed to check Index0 In Track1\n"),
				_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}

		READ_CD_TRANSFER_DATA transferData = { 0 };
		transferData.byTransferLen = (BYTE)(1 + abs(pDiscData->nAdjustSectorNum));
		BOOL bCDG = FALSE;
		if (pDiscData->bAudioOnly) {
			if (!ReadCDForCheckingCDG(pDevData, &bCDG)) {
				throw FALSE;
			}
			if (bCDG == TRUE) {
				if (NULL == (fpCdg = 
					CreateOrOpenFileW(pszPath, NULL, NULL, NULL, _T(".cdg"), _T("wb"), 0, 0))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
			}
		}
		else {
			if (!ReadCDForVolumeDescriptor(pDevData, &pDiscData->toc)) {
				throw FALSE;
			}
		}
		transferData.dwBufLen = CD_RAW_SECTOR_WITH_SUBCODE_SIZE;
		transferData.dwAllBufLen = transferData.dwBufLen * transferData.byTransferLen;
		transferData.dwBufSubOffset = CD_RAW_SECTOR_SIZE;

		if (pDevData->bC2ErrorData && pExtArg->bC2) {
			if (NULL == (fpC2 = CreateOrOpenFileW(
				pszPath, NULL, NULL, NULL, _T(".c2"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			READ_CD_FLAG::DATA_ORDER dataOrder = READ_CD_FLAG::MainC2Sub;
			transferData.dwBufLen = CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE;
			transferData.dwAllBufLen = transferData.dwBufLen * transferData.byTransferLen;
			// default order: main+c2+sub
			transferData.dwBufC2Offset = CD_RAW_SECTOR_SIZE;
			transferData.dwBufSubOffset = CD_RAW_SECTOR_WITH_C2_294_SIZE;

			if (!InitC2ErrorData(pExtArg, &pC2ErrorData, transferData.dwAllBufLen)) {
				throw FALSE;
			}
			if (!ReadCDForCheckingByteOrder(pDevData, pExtArg, &dataOrder)) {
				TerminateC2ErrorData(pDevData, pExtArg, &pC2ErrorData);
				pDevData->bC2ErrorData = FALSE;
				transferData.dwBufLen = CD_RAW_SECTOR_WITH_SUBCODE_SIZE;
				transferData.dwAllBufLen = transferData.dwBufLen * transferData.byTransferLen;
				transferData.dwBufSubOffset = CD_RAW_SECTOR_SIZE;
			}
			if (dataOrder == READ_CD_FLAG::MainSubC2) {
				OutputDriveLog(
					_T("Byte order of this drive is main+sub+c2\n"));
				transferData.dwBufSubOffset = CD_RAW_SECTOR_SIZE;
				transferData.dwBufC2Offset = CD_RAW_SECTOR_WITH_SUBCODE_SIZE;
			}
			else {
				OutputDriveLog(
					_T("Byte order of this drive is main+c2+sub\n"));
			}
		}
		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		if (pDevData->bPlextor) {
			SetReadD8Command(pDevData, pExtArg, lpCmd, 1, FALSE);
		}
		else {
			// non plextor && support scrambled ripping
			SetReadCDCommand(pDevData, pExtArg, lpCmd, bCDG, READ_CD_FLAG::CDDA, 1);
		}

		// store main+(c2)+sub data all
		if (NULL == (pBuf = (LPBYTE)calloc(
			transferData.dwAllBufLen + pDevData->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevData, pBuf);

		if (NULL == (pBufTmp = (LPBYTE)calloc(
			transferData.dwBufLen + pDevData->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBufTmp = (LPBYTE)ConvParagraphBoundary(pDevData, pBufTmp);

		BYTE byScsiStatus = 0;
		BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		BYTE lpNextSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		for (INT p = 0; p < pDiscData->toc.LastTrack; p++) {
			lpCmd[2] = HIBYTE(HIWORD(pDiscData->pnTocStartLBA[p]));
			lpCmd[3] = LOBYTE(HIWORD(pDiscData->pnTocStartLBA[p]));
			lpCmd[4] = HIBYTE(LOWORD(pDiscData->pnTocStartLBA[p]));
			lpCmd[5] = LOBYTE(LOWORD(pDiscData->pnTocStartLBA[p]));
			if (!ScsiPassThroughDirect(pDevData, lpCmd, CDB12GENERIC_LENGTH,
				lpBuf, transferData.dwAllBufLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			AlignRowSubcode(lpBuf + transferData.dwBufSubOffset, lpSubcode);
			lpEndCtlList[p] = (BYTE)((lpSubcode[12] >> 4) & 0x0F);
		}

		CD_OFFSET_DATA cdOffsetData = { 0 };
		SetCDOffsetData(pDiscData, &cdOffsetData, 0, pDiscData->nAllLength);

		SUB_Q_DATA prevPrevSubQ = { 0 };
		SUB_Q_DATA prevSubQ = { 0 };
		if (pDiscData->nCombinedOffset > 0) {
			prevSubQ.nRelativeTime = 0;
			prevSubQ.nAbsoluteTime = 149;
		}
		else if (pDiscData->nCombinedOffset < 0) {
			prevSubQ.nRelativeTime = 150 + pDiscData->nAdjustSectorNum - 1;
			prevSubQ.nAbsoluteTime = 150 + pDiscData->nAdjustSectorNum - 1;
		}
		else {
			prevSubQ.nAbsoluteTime = 149;
		}
		// read pregap of track1
		if (bIndex0InTrack1) {
			cdOffsetData.nOffsetStart = -150;
			prevSubQ.nRelativeTime = pDiscData->pnTocStartLBA[0] + 150;
			prevSubQ.nAbsoluteTime = -1;
		}
		prevSubQ.byCtl = lpEndCtlList[0];
		prevSubQ.byAdr = ADR_ENCODES_CURRENT_POSITION;
		prevSubQ.byTrackNum = pDiscData->toc.FirstTrack;
		prevSubQ.byIndex = cdOffsetData.nOffsetStart < 0 ? (BYTE)0 : (BYTE)1;
		prevSubQ.byMode = DATA_BLOCK_MODE0;
		// init end

		BYTE byCurrentTrackNum = pDiscData->toc.FirstTrack;
		BOOL bCatalog = FALSE;
		BOOL bProcessRet = TRUE;
		UINT uiC2ErrorLBACnt = 0;

		for (INT nLBA = cdOffsetData.nOffsetStart; 
			nLBA < pDiscData->nAllLength + cdOffsetData.nOffsetEnd; nLBA++) {
			bProcessRet = ProcessReadCD(pExecType, pExtArg, pDevData, pDiscData,
				&cdOffsetData, pC2ErrorData, &transferData, uiC2ErrorLBACnt, nLBA,
				lpBuf, lpBufTmp, lpCmd,	&prevSubQ, lpSubcode, lpNextSubcode, fpImg,
				FALSE);
			INT nTmpLBA = nLBA - pDiscData->nAdjustSectorNum;
#if 0
			if (nLBA == 2529) {
				bProcessRet = RETURNED_C2_ERROR_EXIST;
				pC2ErrorData[uiC2ErrorLBACnt].cSlideSectorNum = 2;
				pC2ErrorData[uiC2ErrorLBACnt].uiErrorBytePosCnt = 1;
				pC2ErrorData[uiC2ErrorLBACnt].lpErrorBytePos[0] = 563;
			}
#endif
			if (pC2ErrorData != NULL && bProcessRet == RETURNED_C2_ERROR_EXIST) {
				OutputString(_T(" Detected C2 error. LBA %6d\n"), nTmpLBA);
#ifdef _DEBUG
//				INT nSlideLBANum = 
//					pC2ErrorData[uiC2ErrorLBACnt].cSlideSectorNum - pDiscData->nAdjustSectorNum;
//				INT nFixLBA = nLBA + nSlideLBANum;
				OutputMmcCDMain2352(
					lpBuf + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * abs(pDiscData->nAdjustSectorNum),
					nLBA);
				OutputMmcCDC2Error296(
					lpBuf + CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * abs(pDiscData->nAdjustSectorNum) + transferData.dwBufC2Offset,
					nLBA);
#endif
				SetAndOutputC2ErrorData(pC2ErrorData, nTmpLBA, uiC2ErrorLBACnt);
				uiC2ErrorLBACnt++;
				if (uiC2ErrorLBACnt == pExtArg->uiMaxC2ErrorNum) {
					OutputString(_T("C2 error Max: %d\n"), uiC2ErrorLBACnt);
					throw FALSE;
				}
			}
			else if (bProcessRet == RETURNED_C2_ERROR_CONTINUE) {
				continue;
			}
			else if (bProcessRet == RETURNED_C2_ERROR_SKIP_LBA) {
				nLBA = pDiscData->nStartLBAof2ndSession - 1;
				continue;
			}
			else if (bProcessRet == RETURNED_C2_ERROR_FALSE) {
				throw FALSE;
			}

			SUB_Q_DATA subQ = { 0 };
			SetSubQDataFromReadCD(&subQ, &cdOffsetData, lpBuf, lpSubcode);
			SUB_Q_DATA nextSubQ = { 0 };
			SetSubQDataFromReadCD(&nextSubQ, &cdOffsetData, lpBuf, lpSubcode);

			INT nStart = 0;
			if (bIndex0InTrack1) {
				nStart = -150;
			}
			if (nStart <= nLBA && nLBA < pDiscData->nAllLength) {
				// 2nd:Verification subchannel
				CheckAndFixSubchannel(pExtArg, pDiscData, lpSubcode, &nextSubQ,
					&subQ, &prevSubQ, &prevPrevSubQ, &byCurrentTrackNum, &bCatalog,
					lpISRCList,	lpEndCtlList, lpLBAStartList, lpLBAOfDataTrackList,
					nLBA);
				BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
				// fix raw subchannel
				AlignColumnSubcode(lpSubcode, lpSubcodeRaw);
#if 0
				OutputMmcCDSub96Align(lpSubcode, nLBA->fpError);
#endif
				// 3rd:Write subchannel
				WriteSubChannel(pDiscData, &transferData, lpBuf, lpSubcode,
					lpSubcodeRaw, nLBA, byCurrentTrackNum, fpSub, fpParse, fpCdg);
				PreserveTrackAttribution(pDiscData, nLBA, &byCurrentTrackNum,
					&subQ, &prevSubQ, &prevPrevSubQ, lpCtlList, lpModeList,
					lpLBAStartList, lpLBAOfDataTrackList);
			}
			// 4th:Write track to scrambled
			WriteMainChannel(pDiscData, lpBuf,
				lpLBAStartList, nLBA, &cdOffsetData, fpImg);
			if (pDevData->bC2ErrorData && pExtArg->bC2) {
				// TODO: exist an offset?
				fwrite(lpBuf + transferData.dwBufC2Offset,
					sizeof(BYTE), CD_RAW_READ_C2_294_SIZE, fpC2);
			}
			UpdateSubchannelQData(&subQ, &prevSubQ, &prevPrevSubQ);
			OutputString(_T("\rCreating img(LBA) %6d/%6d"), 
				nLBA - cdOffsetData.nOffsetEnd, pDiscData->nAllLength - 1);
		}
		OutputString(_T("\n"));
		FcloseAndNull(fpC2);
		FcloseAndNull(fpCdg);
		FcloseAndNull(fpSub);
		FcloseAndNull(fpParse);
#if 1
		// c2 error: reread sector 
		if (pDevData->bC2ErrorData && pExtArg->bC2) {
			if (!ReadCDForRereadingSector(pExecType, pExtArg, pDevData, pDiscData,
				&cdOffsetData, pC2ErrorData, &transferData, uiC2ErrorLBACnt, lpBuf,
				lpBufTmp, lpCmd, &prevSubQ, lpSubcode, lpNextSubcode, fpImg)) {
				throw FALSE;
			}
		}
#endif
		FcloseAndNull(fpImg);

		OutputMmcTocWithPregap(pDiscData, lpCtlList,
			lpModeList, lpLBAStartList);

		// 5th:audio only -> from .scm to .img. other descramble img.
		_TCHAR pszNewPath[_MAX_PATH] = { 0 };
		_tcscpy(pszNewPath, pszPath);
		if (!PathRenameExtension(pszNewPath, _T(".img"))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (pDiscData->bAudioOnly) {
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
			if (NULL == (fpImg = CreateOrOpenFileW(
				pszPath, NULL, NULL, NULL, _T(".img"), _T("rb+"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			DescrambleMainChannel(pDiscData,
				lpLBAOfDataTrackList, fpTbl, fpImg);
			FcloseAndNull(fpImg);
			FcloseAndNull(fpTbl);
		}
		if (NULL == (fpImg = CreateOrOpenFileW(
			pszPath, NULL, NULL, NULL, _T(".img"), _T("rb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (!MergeMainChannelAndCDG(pszPath, bCDG, pDiscData->bAudioOnly, fpImg)) {
			throw FALSE;
		}
		if (!CreatingBinCueCcd(pDiscData, pszPath, bCatalog, bCDG, 
			 lpCtlList, lpModeList, lpISRCList, lpLBAStartList, fpImg, fpCue, fpCcd)) {
			throw FALSE;
		}
	}
	catch(BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpImg);
	FcloseAndNull(fpC2);
	FcloseAndNull(fpCdg);
	FcloseAndNull(fpCue);
	FcloseAndNull(fpParse);
	FcloseAndNull(fpSub);
	FcloseAndNull(fpTbl);
	for (INT h = 0; h < pDiscData->toc.LastTrack + 1; h++) {
		if (lpLBAStartList) {
			FreeAndNull(lpLBAStartList[h]);
		}
		if (lpLBAOfDataTrackList) {
			FreeAndNull(lpLBAOfDataTrackList[h]);
		}
	}
	FreeAndNull(lpLBAStartList);
	FreeAndNull(lpLBAOfDataTrackList);
	FreeAndNull(lpCtlList);
	FreeAndNull(lpEndCtlList);
	FreeAndNull(lpISRCList);
	FreeAndNull(lpModeList);
	FreeAndNull(pBuf);
	FreeAndNull(pBufTmp);
	TerminateC2ErrorData(pDevData, pExtArg, &pC2ErrorData);

	return bRet;
}

BOOL ReadCDForSearchingOffset(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData
	)
{
	if (*pExecType != rgd) {
		if (pDiscData->toc.FirstTrack < 1 || 99 < pDiscData->toc.FirstTrack ||
			pDiscData->toc.LastTrack < 1 || 99 < pDiscData->toc.LastTrack) {
			return FALSE;
		}
		pDiscData->bAudioOnly = TRUE;
		if ((pDiscData->toc.TrackData[0].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			pDiscData->bAudioOnly = FALSE;
		}
		else {
			for (INT i = pDiscData->toc.FirstTrack + 1; i <= pDiscData->toc.LastTrack; i++) {
				if (pDiscData->toc.TrackData[0].Control != pDiscData->toc.TrackData[i - 1].Control) {
					pDiscData->bAudioOnly = FALSE;
					break;
				}
			}
		}
	}
	BOOL bRet = FALSE;
	INT nDriveSampleOffset = 0;
	BOOL bGetDrive = GetDriveOffset(pDevData->szProductId, &nDriveSampleOffset);
	if (!bGetDrive) {
		CHAR buf[6] = { 0 };
		OutputString(
			_T("This drive isn't defined in driveOffset.txt.\n")
			_T("Please input Drive offset(Samples) -> "));
		INT b = scanf("%6[^\n]%*[^\n]", buf);
		b = getchar();
		nDriveSampleOffset = atoi(buf) * 4;
	}

	INT nDriveOffset = nDriveSampleOffset * 4;
	if (pDiscData->bAudioOnly) {
		pDiscData->nCombinedOffset = nDriveOffset;
		OutputDiscLog(_T("Offset"));
		if (bGetDrive) {
			OutputDiscLog(
				_T("(Drive offset data referes to http://www.accuraterip.com)"));
		}
		if (pExtArg->bAdd) {
			pDiscData->nCombinedOffset += pExtArg->nAudioCDOffsetNum * 4;
			OutputDiscLog(
				_T("\n")
				_T("\t       Combined Offset(Byte) %6d, (Samples) %5d\n")
				_T("\t-         Drive Offset(Byte) %6d, (Samples) %5d\n")
				_T("\t----------------------------------------------------\n")
				_T("\t User Specified Offset(Byte) %6d, (Samples) %5d\n"),
				pDiscData->nCombinedOffset, pDiscData->nCombinedOffset / 4,
				nDriveOffset, nDriveSampleOffset,
				pDiscData->nCombinedOffset - nDriveOffset, 
				(pDiscData->nCombinedOffset - nDriveOffset) / 4);
		}
		else {
			OutputDiscLog(
				_T("\n")
				_T("\t Combined Offset(Byte) %6d, (Samples) %5d\n")
				_T("\t-   Drive Offset(Byte) %6d, (Samples) %5d\n")
				_T("\t----------------------------------------------\n")
				_T("\t       CD Offset(Byte) %6d, (Samples) %5d\n"),
				pDiscData->nCombinedOffset, pDiscData->nCombinedOffset / 4,
				nDriveOffset, nDriveSampleOffset,
				pDiscData->nCombinedOffset - nDriveOffset, 
				(pDiscData->nCombinedOffset - nDriveOffset) / 4);
		}
		bRet = TRUE;
	}
	else {
		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		LPBYTE ppBuf = (LPBYTE)calloc(
			CD_RAW_SECTOR_WITH_SUBCODE_SIZE * 2 + pDevData->AlignmentMask, sizeof(BYTE));
		if (!ppBuf) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevData, ppBuf);
		BYTE byTransferLen = 2;
		if (pDevData->bPlextor) {
			SetReadD8Command(pDevData, NULL, lpCmd, byTransferLen, TRUE);
		}
		else {
			SetReadCDCommand(pDevData, NULL, lpCmd, FALSE, READ_CD_FLAG::CDDA, byTransferLen);
		}
		if (*pExecType == rgd) {
			pDiscData->nFirstDataLBA = 45000;
		}
		lpCmd[2] = HIBYTE(HIWORD(pDiscData->nFirstDataLBA));
		lpCmd[3] = LOBYTE(HIWORD(pDiscData->nFirstDataLBA));
		lpCmd[4] = HIBYTE(LOWORD(pDiscData->nFirstDataLBA));
		lpCmd[5] = LOBYTE(LOWORD(pDiscData->nFirstDataLBA));
		BYTE byScsiStatus = 0;
		try {
			if (!ScsiPassThroughDirect(pDevData, lpCmd, CDB12GENERIC_LENGTH, lpBuf, 
				CD_RAW_SECTOR_WITH_SUBCODE_SIZE * 2, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				// PLEXTOR(PX-W8432, PX-W1210T, PX-W2410T)
				// if Track1 is DataTrack (mostly game)
				// ==>Sense data, Key:Asc:Ascq: 05:64:00(ILLEGAL_REQUEST. ILLEGAL MODE FOR THIS TRACK)
				// else if Track1 isn't DataTrack (pc engine etc)
				// ==>no error.
				OutputString(
					_T("This drive can't read data sector in scrambled mode.\n"));
				throw FALSE;
			}
			OutputMmcCDMain2352(lpBuf, pDiscData->nFirstDataLBA);
			BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
			AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
			OutputMmcCDSub96Align(lpSubcode, pDiscData->nFirstDataLBA);

			BYTE aBuf2[CD_RAW_SECTOR_SIZE * 2] = { 0 };
			memcpy(aBuf2, lpBuf, CD_RAW_SECTOR_SIZE);
			memcpy(aBuf2 + CD_RAW_SECTOR_SIZE,
				lpBuf + CD_RAW_SECTOR_WITH_SUBCODE_SIZE, CD_RAW_SECTOR_SIZE);
			if (!GetWriteOffset(pDiscData, aBuf2)) {
				OutputErrorString(
					_T("[F:%s][L:%d] Failed to get write-offset\n"),
					_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			OutputDiscLog(_T("Offset"));
			if (bGetDrive) {
				OutputDiscLog(
					_T("(Drive offset data referes to http://www.accuraterip.com)"));
			}
			OutputDiscLog(
				_T("\n")
				_T("\t Combined Offset(Byte) %6d, (Samples) %5d\n")
				_T("\t-   Drive Offset(Byte) %6d, (Samples) %5d\n")
				_T("\t----------------------------------------------\n")
				_T("\t       CD Offset(Byte) %6d, (Samples) %5d\n"),
				pDiscData->nCombinedOffset, pDiscData->nCombinedOffset / 4,
				nDriveOffset, nDriveSampleOffset,
				pDiscData->nCombinedOffset - nDriveOffset, 
				(pDiscData->nCombinedOffset - nDriveOffset) / 4);
			bRet = TRUE;
		}
		catch(BOOL bErr) {
			bRet = bErr;
		}
		FreeAndNull(ppBuf);
	}

	if (bRet) {
		if (0 < pDiscData->nCombinedOffset) {
			pDiscData->nAdjustSectorNum =
				pDiscData->nCombinedOffset / CD_RAW_SECTOR_SIZE + 1;
		}
		else if (pDiscData->nCombinedOffset < 0) {
			pDiscData->nAdjustSectorNum =
				pDiscData->nCombinedOffset / CD_RAW_SECTOR_SIZE - 1;
		}
		OutputDiscLog(
			_T("\tNeed overread sector: %d\n"), pDiscData->nAdjustSectorNum);
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
	_TCHAR szSub[8] = { 0 };
	_TCHAR szSubtxt[12] = { 0 };
	_TCHAR szExt[6] = { 0 };

	if (*pExecType == rgd) {
		_tcscpy(szSub, _T("_dc.sub"));
		_tcscpy(szSubtxt, _T("_dc.sub.txt"));
		_tcscpy(szExt, _T(".scm2"));
	}
	else {
		_tcscpy(szSub, _T(".sub"));
		_tcscpy(szSubtxt, _T(".sub.txt"));
		_tcscpy(szExt, _T(".bin"));
	}
	FILE* fpBin = CreateOrOpenFileW(pszPath, NULL, NULL, NULL, szExt, _T("wb"), 0, 0);
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
	PC2_ERROR_DATA pC2ErrorData = NULL;
	try {
		if (NULL == (fpParse = CreateOrOpenFileW(
			pszPath, NULL, NULL, NULL, szSubtxt, _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpSub = CreateOrOpenFileW(
			pszPath, NULL, NULL, NULL, szSub, _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		READ_CD_TRANSFER_DATA transferData = { 0 };
#if 1
		transferData.byTransferLen = (BYTE)(1 + abs(pDiscData->nAdjustSectorNum));
#else
		transferData.byTransferLen = 1;
#endif
		transferData.dwBufLen = CD_RAW_SECTOR_WITH_SUBCODE_SIZE;
		transferData.dwAllBufLen = transferData.dwBufLen * transferData.byTransferLen;
		transferData.dwBufSubOffset = CD_RAW_SECTOR_SIZE;

		if (!bCheckReading && pDevData->bC2ErrorData && pExtArg->bC2) {
			if (NULL == (fpC2 = CreateOrOpenFileW(
				pszPath, NULL, NULL, NULL, _T(".c2"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			READ_CD_FLAG::DATA_ORDER dataOrder = READ_CD_FLAG::MainC2Sub;
			transferData.dwBufLen = CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE;
			transferData.dwAllBufLen = transferData.dwBufLen * transferData.byTransferLen;
			// default order: main+c2+sub
			transferData.dwBufC2Offset = CD_RAW_SECTOR_SIZE;
			transferData.dwBufSubOffset = CD_RAW_SECTOR_WITH_C2_294_SIZE;

			if (!InitC2ErrorData(pExtArg, &pC2ErrorData, transferData.dwAllBufLen)) {
				throw FALSE;
			}
			if (!ReadCDForCheckingByteOrder(pDevData, pExtArg, &dataOrder)) {
				TerminateC2ErrorData(pDevData, pExtArg, &pC2ErrorData);
				pDevData->bC2ErrorData = FALSE;
				transferData.dwBufLen = CD_RAW_SECTOR_WITH_SUBCODE_SIZE;
				transferData.dwAllBufLen = transferData.dwBufLen * transferData.byTransferLen;
				transferData.dwBufSubOffset = CD_RAW_SECTOR_SIZE;
			}
			if (dataOrder == READ_CD_FLAG::MainSubC2) {
				OutputDriveLog(
					_T("Byte order of this drive is main+sub+c2\n"));
				transferData.dwBufSubOffset = CD_RAW_SECTOR_SIZE;
				transferData.dwBufC2Offset = CD_RAW_SECTOR_WITH_SUBCODE_SIZE;
			}
			else {
				OutputDriveLog(
					_T("Byte order of this drive is main+c2+sub\n"));
			}
		}
		if (pDevData->bPlextor) {
			SetReadD8Command(pDevData, pExtArg, lpCmd, 1, FALSE);
		}
		else {
			// non plextor && support scrambled ripping
			SetReadCDCommand(pDevData, pExtArg, lpCmd, FALSE, flg, 1);
		}

		// store main+(c2)+sub data
		if (NULL == (pBuf = (LPBYTE)calloc(
			transferData.dwAllBufLen + pDevData->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevData, pBuf);

		if (NULL == (pBufTmp = (LPBYTE)calloc(
			transferData.dwBufLen + pDevData->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBufTmp = (LPBYTE)ConvParagraphBoundary(pDevData, pBufTmp);

		BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		BYTE lpNextSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		CD_OFFSET_DATA cdOffsetData = { 0 };
		SetCDOffsetData(pDiscData, &cdOffsetData, nStart, nEnd);

		BYTE byCurrentTrackNum = *pExecType == rgd ? (BYTE)3 : pDiscData->toc.FirstTrack;
		UINT uiC2ErrorLBACnt = 0;
		BOOL bProcessRet = TRUE;

		for (INT nLBA = nStart + cdOffsetData.nOffsetStart; 
			nLBA < nEnd + cdOffsetData.nOffsetEnd; nLBA++) {
			bProcessRet = ProcessReadCD(pExecType, pExtArg, pDevData, pDiscData,
				&cdOffsetData, pC2ErrorData, &transferData, uiC2ErrorLBACnt, nLBA, lpBuf,
				lpBufTmp, lpCmd, NULL, lpSubcode, lpNextSubcode, fpBin, bCheckReading);
			INT nTmpLBA = nLBA - pDiscData->nAdjustSectorNum;

			if (pC2ErrorData != NULL && bProcessRet == RETURNED_C2_ERROR_EXIST) {
				OutputString(_T(" Detected C2 error. LBA %6d\n"), nTmpLBA);
				SetAndOutputC2ErrorData(pC2ErrorData, nLBA, uiC2ErrorLBACnt);
				uiC2ErrorLBACnt++;
				if (uiC2ErrorLBACnt == pExtArg->uiMaxC2ErrorNum) {
					OutputString(_T("C2 error Max: %d\n"), uiC2ErrorLBACnt);
					throw FALSE;
				}
			}
			else if (bProcessRet == RETURNED_C2_ERROR_CONTINUE) {
				continue;
			}
			else if (bProcessRet == RETURNED_C2_ERROR_SKIP_LBA) {
				nLBA = pDiscData->nStartLBAof2ndSession - 1;
				continue;
			}
			else if (bProcessRet == RETURNED_C2_ERROR_FALSE) {
				throw FALSE;
			}
			if (nStart <= nLBA && nLBA < nEnd) {
				// TODO: Fix Subchannnel
#if 0
				SUB_Q_DATA subQ = { 0 };
				SetSubQDataFromReadCD(&subQ, &cdOffsetData, lpBuf, lpSubcode);
				SUB_Q_DATA nextSubQ = { 0 };
				SetSubQDataFromReadCD(&nextSubQ, &cdOffsetData, lpBuf, lpSubcode);
				CheckAndFixSubchannel(pExtArg, pDiscData, lpSubcode, &nextSubQ,
					&subQ, &prevSubQ, &prevPrevSubQ, &byCurrentTrackNum, &bCatalog,
					lpISRCList,	lpEndCtlList, lpLBAStartList, lpLBAOfDataTrackList,
					nLBA->fpError);
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
				WriteSubChannel(pDiscData, &transferData, lpBuf, lpSubcode, 
					NULL, nLBA, byCurrentTrackNum, fpSub, fpParse, NULL);
			}
			WriteMainChannel(pDiscData, lpBuf, NULL, nLBA, &cdOffsetData, fpBin);
			if (!bCheckReading && pDevData->bC2ErrorData && pExtArg->bC2) {
				// TODO: exist an offset?
				fwrite(lpBuf + transferData.dwBufC2Offset,
					sizeof(BYTE), CD_RAW_READ_C2_294_SIZE, fpC2);
			}
			OutputString(_T("\rCreating bin from %d to %d (LBA) %6d"), 
				nStart, nEnd + cdOffsetData.nOffsetEnd - 1, nLBA);
		}
		OutputString(_T("\n"));
		FcloseAndNull(fpC2);
		FcloseAndNull(fpParse);
		FcloseAndNull(fpSub);
		if (!bCheckReading) {
			if (pC2ErrorData && pDevData->bC2ErrorData && pExtArg->bC2) {
				if (!ReadCDForRereadingSector(pExecType, pExtArg, pDevData, pDiscData,
					&cdOffsetData, pC2ErrorData, &transferData, uiC2ErrorLBACnt, lpBuf,
					lpBufTmp, lpCmd, NULL, lpSubcode, lpNextSubcode, fpBin)) {
					throw FALSE;
				}
			}
		}
	}
	catch(BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpBin);
	FcloseAndNull(fpC2);
	FcloseAndNull(fpParse);
	FcloseAndNull(fpSub);
	FreeAndNull(pBuf);
	FreeAndNull(pBufTmp);
	TerminateC2ErrorData(pDevData, pExtArg, &pC2ErrorData);

	if (bRet && *pExecType == rgd) {
		if (!DescrambleMainChannelForGD(pszPath)) {
			return FALSE;
		}
		if (!SplitFileForGD(pszPath)) {
			return FALSE;
		}
	}
	return bRet;
}
