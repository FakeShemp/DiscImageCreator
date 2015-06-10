/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"
#include "check.h"
#include "convert.h"
#include "get.h"
#include "output.h"
#include "outputGD.h"

BOOL DescrambleDataSector(
	LPCTSTR pszPath,
	INT nLBA
	)
{
	FILE* fpScr = CreateOrOpenFileW(pszPath, NULL, NULL, NULL, _T(".bin"), _T("rb"), 0, 0);
	if (!fpScr) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	FILE* fpDescr = NULL;
	FILE* fptxt = NULL;
	FILE* fpTbl = NULL;
	BOOL bRet = TRUE;
	try {
		if (NULL == (fpDescr = CreateOrOpenFileW(pszPath, NULL, NULL, NULL, _T(".dec"), _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fptxt = CreateOrOpenFileW(pszPath, NULL, NULL, NULL, _T("_dc_imglog.txt"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpTbl = OpenProgrammabledFile(_T("scramble.bin"), _T("rb")))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		DWORD dwFileSize = GetFileSize(0, fpScr);
		BYTE bufScr[CD_RAW_SECTOR_SIZE] = { 0 };
		INT nOffset = 0;
		BOOL bRet2 = FALSE;
		for (DWORD i = 0; i < CD_RAW_SECTOR_SIZE; i++) {
			fread(bufScr, sizeof(BYTE), SYNC_SIZE, fpScr);
			if (IsValidDataHeader(bufScr)) {
				bRet2 = TRUE;
				break;
			}
			else {
				nOffset++;
				fseek(fpScr, -11, SEEK_CUR);
			}
		}
		if (!bRet2) {
			OutputErrorString(_T("[F:%s][L:%d] Not GD-ROM File\n"), _T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		// skip to 10:02:00(11:82:00)
		fread(bufScr + SYNC_SIZE, sizeof(BYTE), 3, fpScr);
		LONG lba1 = MSFtoLBA(00, 02, 10);
		LONG lba2 = MSFtoLBA(BcdToDec(bufScr[0x0E]), 
			BcdToDec((BYTE)(bufScr[0x0D] ^ 0x80)), 
			BcdToDec((BYTE)(bufScr[0x0C] ^ 0x01)));
		INT nVal = lba2 - 150 - nLBA;
		INT nCombinedOffset = 
			nVal > 0 ? -CD_RAW_SECTOR_SIZE * nVal + nOffset : nOffset;
		OutputDiscLog(
			_T(" Combined Offset(Byte) %6d, (Samples) %5d\n")
			_T("-   Drive Offset(Byte) see _dc_drivelog.txt\n")
			_T("----------------------------------------------\n")
			_T("   GD-ROM Offset\n"),
			nCombinedOffset, nCombinedOffset / 4);
		FcloseAndNull(fptxt);

		BYTE bufTbl[CD_RAW_SECTOR_SIZE] = { 0 };
		fread(bufTbl, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpTbl);
		FcloseAndNull(fpTbl);

		LONG lSeekSize = CD_RAW_SECTOR_SIZE * (lba1 - lba2) + nOffset;
		if (dwFileSize < (DWORD)lSeekSize) {
			OutputErrorString(
				_T("[F:%s][L:%d] Out of range! SeekSize %d, FileSize %d\n"),
				_T(__FUNCTION__), __LINE__, lSeekSize, dwFileSize);
			throw FALSE;
		}
		fseek(fpScr, lSeekSize, SEEK_SET);

		BYTE bufDescr[CD_RAW_SECTOR_SIZE] = { 0 };
		DWORD dwFileSize2 = GetFileSize(lSeekSize, fpScr);
		DWORD dwDecFilesize = dwFileSize2 - lSeekSize;
		DWORD dwRoop = dwDecFilesize / CD_RAW_SECTOR_SIZE;
		for (DWORD i = 1; i <= dwRoop; i++) {
			fread(bufScr, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpScr);
			if (IsValidDataHeader(bufScr)) {
				if (bufScr[0x0C] == 0xC3 && bufScr[0x0D] == 0x84 && bufScr[0x0E] >= 0x00) {
					break;
				}
				for (INT j = 0; j < CD_RAW_SECTOR_SIZE; j++) {
					bufDescr[j] = (BYTE)(bufScr[j] ^ bufTbl[j]);
				}
				fwrite(bufDescr, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpDescr);
			}
			else {
				// copy audio data
				fwrite(bufScr, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpDescr);
			}
			OutputString(_T("\rDescrambling File(size) %10d/%10d"), CD_RAW_SECTOR_SIZE * i, dwDecFilesize);
		}
		OutputString(_T("\n"));
	}
	catch(BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpScr);
	FcloseAndNull(fpDescr);
	FcloseAndNull(fptxt);
	FcloseAndNull(fpTbl);
	return bRet;
}

BOOL SplitDescrambledFile(
	LPCTSTR pszPath
	)
{
	_TCHAR pszPathWithoutPathAndExt[_MAX_PATH] = { 0 };
	FILE* fpDescr = CreateOrOpenFileW(pszPath, 
		NULL, NULL, pszPathWithoutPathAndExt, _T(".dec"), _T("rb"), 0, 0);
	if (!fpDescr) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	FILE* fptxt = NULL;
	FILE* fpGdi = NULL;
	FILE* fpBin = NULL;
#if 0
	FILE* fpCcd = NULL;
#endif
	PLONG pToc = NULL;
	LPBYTE lpBuf = NULL;
	BOOL bRet = TRUE;
	try {
		if (NULL == (fptxt = CreateOrOpenFileW(pszPath,
			NULL, NULL, NULL, _T("_dc_imglog.txt"), _T(AFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpGdi = CreateOrOpenFileW(pszPath,
			NULL, NULL, NULL, _T(".gdi"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
#if 0
		if (NULL == (fpCcd = CreateOrOpenFileW(pszPath,
			NULL, NULL, NULL, _T(".ccd"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
#endif

		DWORD dwFileSize = GetFileSize(0, fpDescr);
		if (dwFileSize < 0x110 + 512) {
			OutputErrorString(
				_T("[F:%s][L:%d] Not GD-ROM data\n"), _T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		fseek(fpDescr, 0x110, SEEK_SET);
		// 0x110 - 0x31F is toc data
		BYTE byToc[512] = { 0 };
		fread(byToc, sizeof(BYTE), sizeof(byToc), fpDescr);
		if (byToc[0] != 'T' || byToc[1] != 'O' || byToc[2] != 'C' || byToc[3] != '1') {
			OutputErrorString(
				_T("[F:%s][L:%d] Not GD-ROM data\n"), _T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		INT nTrackNum = 3;
#if 0
		fprintf(fpCcd, "[CloneCD]\n");
		fprintf(fpCcd, "Version=3\n");
		fprintf(fpCcd, "[Disc]\n");
		fprintf(fpCcd, "TocEntries=%d\n", nTrackNum + 6);
		fprintf(fpCcd, "Sessions=2\n");
		fprintf(fpCcd, "DataTracksScrambled=0\n");
		fprintf(fpCcd, "CDTextLength=0\n");
		fprintf(fpCcd, "[Session 1]\n");
		fprintf(fpCcd, "PreGapMode=0\n");
		fprintf(fpCcd, "PreGapSubC=0\n");
		fprintf(fpCcd, "[Session 2]\n");
		fprintf(fpCcd, "PreGapMode=0\n");
		fprintf(fpCcd, "PreGapSubC=0\n");
		//Entry 0 = SessionNum 1, FirstTrackNum
		//Entry 1 = SessionNum 1, LastTrackNum
		//Entry 2 = SessionNum 1, Leadout MSF
		//Entry 3 = SessionNum 1, TrackNum 1
		//Entry 4 = SessionNum 1, TrackNum 2
		//Entry 5 = SessionNum 2, FirstTrackNum
		//Entry 6 = SessionNum 2, LastTrackNum
		//Entry 7 = SessionNum 2, Leadout MSF
		//Entry 8 = SessionNum 2, TrackNum 1
		//					:
		//					:
		//					:
		INT aSession[] = {1, 1, 1, 1, 1, 2, 2, 2};
		INT aPoint[] = {0xa0, 0xa1, 0xa2, 0x01, 0x02, 0xa0, 0xa1, 0xa2};
		INT aControl[] = {0x04, 0x00, 0x00, 0x04, 0x00, 0x04, 0x04, 0x04};
		INT aPMin[] = {1, 2, 0, 0, 0, 3, nTrackNum, 122};
		INT aPSec[] = {0, 0, 0xffffffff, 2, 0xffffffff, 0, 0, 4};
		INT aPFrame[] = {0, 0, 0xffffffff, 0, 0xffffffff, 0, 0, 0};
		INT aPLBA[] = {4350, 8850, 0xffffffff, 0, 0xffffffff, 62850, 62850, 1650};
		for (INT c = 0; c < 8; c++) {
			fprintf(fpCcd, "[Entry %d]\n", c);
			fprintf(fpCcd, "Session=%d\n", aSession[c]);
			fprintf(fpCcd, "Point=0x%02x\n", aPoint[c]);
			fprintf(fpCcd, "ADR=0x01\n");
			fprintf(fpCcd, "Control=0x%02x\n", aControl[c]);
			fprintf(fpCcd, "TrackNo=0\n");
			fprintf(fpCcd, "AMin=0\n");
			fprintf(fpCcd, "ASec=0\n");
			fprintf(fpCcd, "AFrame=0\n");
			fprintf(fpCcd, "ALBA=-150\n"); 
			fprintf(fpCcd, "Zero=0\n");
			fprintf(fpCcd, "PMin=%d\n", aPMin[c]);
			fprintf(fpCcd, "PSec=%d\n", aPSec[c]);
			fprintf(fpCcd, "PFrame=%d\n", aPFrame[c]);
			fprintf(fpCcd, "PLBA=%d\n", aPLBA[c]);
		}
#endif
		UINT uiMaxToc = 98 * 4;
		UINT uiMaxTrackNum = byToc[uiMaxToc + 4 * 1 + 2];
		pToc = (PLONG)calloc(uiMaxTrackNum, sizeof(UINT));
		if (!pToc) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LONG lMaxLba = 0;
		for (UINT i = 0; i < uiMaxToc; i += 4) {
			if (byToc[7 + i] == 0xFF) {
				lMaxLba =
					MAKELONG(MAKEWORD(byToc[4 + i - 4], byToc[5 + i - 4]), MAKEWORD(byToc[6 + i - 4], 0));
				break;
			}
		}
		_ftprintf(fpGdi, _T("%d\n"), uiMaxTrackNum);
		if (uiMaxTrackNum <= 9 && lMaxLba <= 99999) {
			_ftprintf(fpGdi,
				_T("1 %5d 4 2352 \"%s (Track %d).bin\" 0\n")
				_T("2 [fix] 0 2352 \"%s (Track %d).bin\" 0\n"),
				0, pszPathWithoutPathAndExt, 1,
				pszPathWithoutPathAndExt, 2);
		}
		else if (10 <= uiMaxTrackNum && lMaxLba <= 99999) {
			_ftprintf(fpGdi,
				_T(" 1 %5d 4 2352 \"%s (Track %02d).bin\" 0\n")
				_T(" 2 [fix] 0 2352 \"%s (Track %02d).bin\" 0\n"),
				0, pszPathWithoutPathAndExt, 1,
				pszPathWithoutPathAndExt, 2);
		}
		else if (uiMaxTrackNum <= 9 && 100000 <= lMaxLba) {
			_ftprintf(fpGdi,
				_T("1 %6d 4 2352 \"%s (Track %d).bin\" 0\n")
				_T("2  [fix] 0 2352 \"%s (Track %d).bin\" 0\n"),
				0, pszPathWithoutPathAndExt, 1,
				pszPathWithoutPathAndExt, 2);
		}
		else if (10 <= uiMaxTrackNum && 100000 <= lMaxLba) {
			_ftprintf(fpGdi,
				_T(" 1 %6d 4 2352 \"%s (Track %02d).bin\" 0\n")
				_T(" 2  [fix] 0 2352 \"%s (Track %02d).bin\" 0\n"),
				0, pszPathWithoutPathAndExt, 1,
				pszPathWithoutPathAndExt, 2);
		}
		for (UINT i = 0; i < uiMaxToc; i += 4, nTrackNum++) {
			if (byToc[7 + i] == 0xFF) {
				break;
			}
			BYTE byCtl = (BYTE)((byToc[7 + i] >> 4) & 0x0F);
			LONG lToc =
				MAKELONG(MAKEWORD(byToc[4 + i], byToc[5 + i]), MAKEWORD(byToc[6 + i], 0));
#if 0
			BYTE byAdr = (BYTE)(byToc[7 + i] & 0x0F);
			fprintf(fpCcd, "[Entry %d]\n", nTrackNum + 5);
			fprintf(fpCcd, "Session=2\n");
			fprintf(fpCcd, "Point=0x%02x\n", nTrackNum);
			fprintf(fpCcd, "ADR=0x%02x\n", byAdr);
			fprintf(fpCcd, "Control=0x%02x\n", byCtl);
			fprintf(fpCcd, "TrackNo=0\n");
			fprintf(fpCcd, "AMin=0\n");
			fprintf(fpCcd, "ASec=0\n");
			fprintf(fpCcd, "AFrame=0\n");
			fprintf(fpCcd, "ALBA=-150\n"); 
			fprintf(fpCcd, "Zero=0\n");
			BYTE byFrame = 0;
			BYTE bySecond = 0;
			BYTE byMinute = 0;
			LBAtoMSF(lToc, &byFrame, &bySecond, &byMinute);
			fprintf(fpCcd, "PMin=%d\n", byMinute);
			fprintf(fpCcd, "PSec=%d\n", bySecond);
			fprintf(fpCcd, "PFrame=%d\n", byFrame);
			fprintf(fpCcd, "PLBA=%d\n", lToc);
#endif
			pToc[nTrackNum-3] = lToc - 300;
			if (nTrackNum == 3) {
				pToc[nTrackNum-3] += 150;
				OutputDiscLog(
					_T("Track %2d, Ctl %d,              , Index1 %6d\n"), 
					nTrackNum, byCtl, pToc[nTrackNum-3]);
			}
			else {
				if ((byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					pToc[nTrackNum-3] -= 75;
					OutputDiscLog(
						_T("Track %2d, Ctl %d, Index0 %6d, Index1 %6d\n"), 
						nTrackNum, byCtl, pToc[nTrackNum-3], pToc[nTrackNum-3] + 225);
				}
				else {
					OutputDiscLog(
						_T("Track %2d, Ctl %d, Index0 %6d, Index1 %6d\n"), 
						nTrackNum, byCtl, pToc[nTrackNum-3], pToc[nTrackNum-3] + 150);
				}
			}
			if (uiMaxTrackNum <= 9 && lMaxLba <= 99999) {
				_ftprintf(fpGdi, _T("%d %5d %d 2352 \"%s (Track %d).bin\" 0\n"),
					nTrackNum, pToc[nTrackNum-3], byCtl, pszPathWithoutPathAndExt, nTrackNum);
			}
			else if (10 <= uiMaxTrackNum && lMaxLba <= 99999) {
				_ftprintf(fpGdi, _T("%2d %5d %d 2352 \"%s (Track %02d).bin\" 0\n"),
					nTrackNum, pToc[nTrackNum-3], byCtl, pszPathWithoutPathAndExt, nTrackNum);
			}
			else if (uiMaxTrackNum <= 9 && 100000 <= lMaxLba) {
				_ftprintf(fpGdi, _T("%d %6d %d 2352 \"%s (Track %d).bin\" 0\n"),
					nTrackNum, pToc[nTrackNum-3], byCtl, pszPathWithoutPathAndExt, nTrackNum);
			}
			else if (10 <= uiMaxTrackNum && 100000 <= lMaxLba) {
				_ftprintf(fpGdi, _T("%2d %6d %d 2352 \"%s (Track %02d).bin\" 0\n"),
					nTrackNum, pToc[nTrackNum-3], byCtl, pszPathWithoutPathAndExt, nTrackNum);
			}
		}
		LONG lToc = 
			MAKELONG(MAKEWORD(byToc[uiMaxToc + 4 * 2], byToc[uiMaxToc + 4 * 2 + 1]),
			MAKEWORD(byToc[uiMaxToc + 4 * 2 + 2], 0)) - 150;
		OutputDiscLog(
			_T("MaxTrackNum %2d\n")
			_T("Leadout, LBA %6d\n"),
			uiMaxTrackNum, lToc);
		pToc[nTrackNum-3] = lToc;

		rewind(fpDescr);
		for (UINT i = 3; i <= uiMaxTrackNum; i++) {
			OutputString(_T("\rSplit File(num) %2d/%2d"), i, uiMaxTrackNum);
			if (NULL == (fpBin = CreateOrOpenFileW(pszPath, NULL,
				NULL, NULL, _T(".bin"), _T("wb"), i, uiMaxTrackNum))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			size_t size = 
				(size_t)(pToc[i-2] - pToc[i-3]) * CD_RAW_SECTOR_SIZE;
			if (NULL == (lpBuf = (LPBYTE)calloc(size, sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			fread(lpBuf, sizeof(BYTE), size, fpDescr);
			fwrite(lpBuf, sizeof(BYTE), size, fpBin);
			FcloseAndNull(fpBin);
			FreeAndNull(lpBuf);
		}
		OutputString(_T("\n"));
	}
	catch(BOOL ret) {
		bRet = ret;
	}
#if 0
	FcloseAndNull(fpCcd);
#endif
	FcloseAndNull(fpDescr);
	FcloseAndNull(fptxt);
	FcloseAndNull(fpBin);
	FreeAndNull(pToc);
	return bRet;
}
