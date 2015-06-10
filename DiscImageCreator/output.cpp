/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"
#include "struct.h"
#include "check.h"
#include "convert.h"
#include "get.h"
#include "output.h"
#include "outputMMCLog.h"
#include "set.h"

#ifdef _DEBUG
_TCHAR logBuffer[2048];
#endif
// These global variable is set at pringAndSetArg().
extern _TCHAR g_szCurrentdir[_MAX_PATH];

FILE* CreateOrOpenFileW(
	LPCTSTR pszPath,
	LPTSTR pszOutPath,
	LPTSTR pszPathWithoutPath,
	LPTSTR pszPathWithoutPathAndExt,
	LPCTSTR pszExt,
	LPCTSTR pszMode,
	UINT uiTrackNum,
	UINT uiMaxTrackNum
	)
{
	_TCHAR szDstPath[_MAX_PATH] = { 0 };
	_TCHAR szTmpPath[_MAX_PATH] = { 0 };
	_TCHAR drive[_MAX_DRIVE] = { 0 };
	_TCHAR dir[_MAX_DIR] = { 0 };
	_TCHAR fname[_MAX_FNAME] = { 0 };
	_TCHAR ext[_MAX_EXT] = { 0 };

	_tsplitpath(pszPath, drive, dir, fname, ext);
	if (drive[0] == 0 || dir[0] == 0) {
		_tcsncpy(szTmpPath, g_szCurrentdir, _MAX_PATH);
		if (dir[0] != 0) {
			if (!PathAppend(szTmpPath, dir)) {
				return NULL;
			}
		}
		if (!PathAppend(szTmpPath, fname)) {
			return NULL;
		}
		memcpy(szDstPath, szTmpPath, _MAX_PATH * sizeof(_TCHAR));
		_tsplitpath(szDstPath, drive, dir, fname, NULL);
	}

	if (drive[0] != 0 && dir[0] != 0) {
		_tmakepath(szTmpPath, drive, dir, NULL, NULL);
		if (!PathIsDirectory(szTmpPath)) {
			if (!CreateDirectory(szTmpPath, NULL)) {
				return NULL;
			}
		}
	}

	if (uiMaxTrackNum <= 1) {
		_stprintf(szDstPath, _T("%s%s%s%s"), drive, dir, fname, pszExt);
	}
	else if (2 <= uiMaxTrackNum && uiMaxTrackNum <= 9) {
		_stprintf(szDstPath, 
			_T("%s%s%s (Track %d)%s"), drive, dir, fname, uiTrackNum, pszExt);
	}
	else if (10 <= uiMaxTrackNum) {
		_stprintf(szDstPath, 
			_T("%s%s%s (Track %02d)%s"), drive, dir, fname, uiTrackNum, pszExt);
	}

	if (pszPathWithoutPath != NULL) {
		// size of pszPathWithoutPath must be _MAX_PATH.
		ZeroMemory(pszPathWithoutPath, _MAX_FNAME);
		_tsplitpath(szDstPath, drive, dir, fname, ext);
		_stprintf(pszPathWithoutPath, _T("%s%s"), fname, ext);
	}
	if (pszPathWithoutPathAndExt != NULL) {
		// size of pszPathWithoutPathAndExt must be _MAX_PATH.
		ZeroMemory(pszPathWithoutPathAndExt, _MAX_FNAME);
		_tsplitpath(szDstPath, drive, dir, fname, ext);
		_stprintf(pszPathWithoutPathAndExt, _T("%s"), fname);
	}
	if (pszOutPath != NULL) {
		// size of pszOutPath must be _MAX_PATH.
		_tcsncpy(pszOutPath, szDstPath, _MAX_PATH);
	}
	FILE* fp = _tfopen(szDstPath, pszMode);
#ifdef UNICODE
	// delete bom
	fseek(fp, 0, SEEK_SET);
#endif
	return fp;
}

FILE* CreateOrOpenFileA(
	LPCSTR pszPath,
	LPSTR pszOutPath,
	LPSTR pszPathWithoutPath,
	LPCSTR pszExt,
	LPCSTR pszMode,
	UINT uiTrackNum,
	UINT uiMaxTrackNum
	)
{
	CHAR szDstPath[_MAX_PATH] = { 0 };
	CHAR drive[_MAX_DRIVE] = { 0 };
	CHAR dir[_MAX_DIR] = { 0 };
	CHAR fname[_MAX_FNAME] = { 0 };
	CHAR ext[_MAX_EXT] = { 0 };

	_splitpath(pszPath, drive, dir, fname, ext);
	if (uiMaxTrackNum <= 1) {
		sprintf(szDstPath, "%s%s%s%s", drive, dir, fname, pszExt);
	}
	else if (2 <= uiMaxTrackNum && uiMaxTrackNum <= 9) {
		sprintf(szDstPath, 
			"%s%s%s (Track %d)%s", drive, dir, fname, uiTrackNum, pszExt);
	}
	else if (10 <= uiMaxTrackNum) {
		sprintf(szDstPath, 
			"%s%s%s (Track %02d)%s", drive, dir, fname, uiTrackNum, pszExt);
	}

	if (pszPathWithoutPath != NULL) {
		// size of pszPathWithoutPath must be _MAX_PATH.
		ZeroMemory(pszPathWithoutPath, _MAX_FNAME);
		_splitpath(szDstPath, drive, dir, fname, ext);
		sprintf(pszPathWithoutPath, "%s%s", fname, ext);
	}
	if (pszOutPath != NULL) {
		// size of pszOutPath must be _MAX_PATH.
		strncpy(pszOutPath, szDstPath, _MAX_PATH);
	}
	return fopen(szDstPath, pszMode);
}

FILE* OpenProgrammabledFile(
	LPCTSTR pszPath,
	LPCTSTR pszMode
	)
{
	FILE* fp = NULL;
	_TCHAR dir[MAX_PATH] = { 0 };
	if (!::GetModuleFileName(NULL, dir, MAX_PATH)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return NULL;
	}
	_TCHAR* pdest = _tcsrchr(dir, '\\');
	if (pdest) {
		pdest[0] = NULL;
		_TCHAR lpBuf[MAX_PATH] = { 0 };
		_stprintf(lpBuf, _T("%s\\%s"), dir, pszPath);
		fp = _tfopen(lpBuf, pszMode);
	}
	return fp;
}

VOID WriteCcdFileForDisc(
	UINT uiTocEntries,
	BYTE LastCompleteSession,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd,
		_T("[CloneCD]\n")
		_T("Version=3\n")
		_T("[Disc]\n")
		_T("TocEntries=%d\n")
		_T("Sessions=%d\n")
		_T("DataTracksScrambled=%d\n"),
		uiTocEntries,
		LastCompleteSession,
		0); // TODO
}

VOID WriteCcdFileForDiscCDTextLength(
	UINT uiCDTextSize,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd, _T("CDTextLength=%d\n"), uiCDTextSize);
}

VOID WriteCcdFileForDiscCatalog(
	PDISC_DATA pDiscData,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd, _T("CATALOG=%s\n"), pDiscData->szCatalog);
}

VOID WriteCcdFileForCDText(
	UINT uiCDTextSize,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd,
		_T("[CDText]\n")
		_T("Entries=%d\n"),
		uiCDTextSize);
}

VOID WriteCcdFileForCDTextEntry(
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	UINT uiTocTextEntries,
	FILE* fpCcd
	)
{
	for (UINT t = 0; t < uiTocTextEntries; t++) {
		_ftprintf(fpCcd, 
			_T("Entry %d=%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"), 
			t,
			pDesc[t].PackType,
			pDesc[t].TrackNumber | (pDesc[t].ExtensionFlag << 7),
			pDesc[t].SequenceNumber, 
			pDesc[t].CharacterPosition | (pDesc[t].BlockNumber << 4) | (pDesc[t].Unicode << 7),
			pDesc[t].Text[0], pDesc[t].Text[1], pDesc[t].Text[2], pDesc[t].Text[3], 
			pDesc[t].Text[4], pDesc[t].Text[5],	pDesc[t].Text[6], pDesc[t].Text[7], 
			pDesc[t].Text[8], pDesc[t].Text[9], pDesc[t].Text[10], pDesc[t].Text[11]);
	}
}

VOID WriteCcdFileForSession(
	BYTE SessionNumber,
	BYTE byMode,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd,
		_T("[Session %d]\n")
		_T("PreGapMode=%d\n")
		_T("PreGapSubC=%d\n"),
		SessionNumber,
		byMode,
		SessionNumber == 1 ? byMode : 0); // TODO
}

VOID WriteCcdFileForEntry(
	PCDROM_TOC_FULL_TOC_DATA_BLOCK toc,
	UINT a,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd,
		_T("[Entry %d]\n")
		_T("Session=%d\n")
		_T("Point=0x%02x\n")
		_T("ADR=0x%02x\n")
		_T("Control=0x%02x\n")
		_T("TrackNo=%d\n")
		_T("AMin=%d\n")
		_T("ASec=%d\n")
		_T("AFrame=%d\n")
		_T("ALBA=%d\n")
		_T("Zero=%d\n")
		_T("PMin=%d\n")
		_T("PSec=%d\n")
		_T("PFrame=%d\n")
		_T("PLBA=%d\n"),
		a,
		toc[a].SessionNumber,
		toc[a].Point,
		toc[a].Adr,
		toc[a].Control,
		toc[a].Reserved1,
		toc[a].MsfExtra[0],
		toc[a].MsfExtra[1],
		toc[a].MsfExtra[2],
		MSFtoLBA(toc[a].MsfExtra[2], toc[a].MsfExtra[1], toc[a].MsfExtra[0]) - 150,
		toc[a].Zero,
		toc[a].Msf[0],
		toc[a].Msf[1],
		toc[a].Msf[2], 
		MSFtoLBA(toc[a].Msf[2], toc[a].Msf[1], toc[a].Msf[0]) - 150);
}

VOID WriteCcdFileForTrack(
	PDISC_DATA pDiscData,
	UINT nTrackNum,
	BYTE byModeNum,
	BOOL bISRC,
	BYTE byCtl,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd,
		_T("[TRACK %d]\n")
		_T("MODE=%d\n"),
		nTrackNum,
		byModeNum);
	if (bISRC) {
		_ftprintf(fpCcd, _T("ISRC=%s\n"), pDiscData->pszISRC[nTrackNum - 1]);
	}
	switch (byCtl & ~AUDIO_DATA_TRACK) {
	case AUDIO_WITH_PREEMPHASIS:
		_ftprintf(fpCcd, _T("FLAGS= PRE\n"));
		break;
	case DIGITAL_COPY_PERMITTED:
		_ftprintf(fpCcd, _T("FLAGS= DCP\n"));
		break;
	case DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
		_ftprintf(fpCcd, _T("FLAGS= DCP PRE\n"));
		break;
	case TWO_FOUR_CHANNEL_AUDIO:
		_ftprintf(fpCcd, _T("FLAGS= 4CH\n"));
		break;
	case TWO_FOUR_CHANNEL_AUDIO | AUDIO_WITH_PREEMPHASIS:
		_ftprintf(fpCcd, _T("FLAGS= 4CH PRE\n"));
		break;
	case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED:
		_ftprintf(fpCcd, _T("FLAGS= 4CH DCP\n"));
		break;
	case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
		_ftprintf(fpCcd, _T("FLAGS= 4CH DCP PRE\n"));
		break;
	}
}

VOID WriteCcdFileForTrackIndex(
	INT nIndex,
	INT nLba,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd, _T("INDEX %d=%d\n"), nIndex, nLba);
}

VOID WriteCueFileFirst(
	PDISC_DATA pDiscData,
	BOOL lpCatalog,
	FILE* fpCue
	)
{
	if (lpCatalog) {
		_ftprintf(fpCue, _T("CATALOG %s\n"), pDiscData->szCatalog);
	}
	if (pDiscData->pszTitle[0][0] != 0) {
		_ftprintf(fpCue, _T("TITLE \"%s\"\n"), pDiscData->pszTitle[0]);
	}
	if (pDiscData->pszPerformer[0][0] != 0) {
		_ftprintf(fpCue, _T("PERFORMER \"%s\"\n"), pDiscData->pszPerformer[0]);
	}
	if (pDiscData->pszSongWriter[0][0] != 0) {
		_ftprintf(fpCue, _T("SONGWRITER \"%s\"\n"), pDiscData->pszSongWriter[0]);
	}
}

VOID WriteCueFile(
	PDISC_DATA pDiscData,
	LPCTSTR pszPath,
	BOOL bCDG,
	UINT nTrackNum,
	BYTE byModeNum,
	BOOL bISRC,
	BYTE byCtl,
	FILE* fpCue
	)
{
	_ftprintf(fpCue, _T("FILE \"%s\" BINARY\n"), pszPath);

	if (byModeNum == DATA_BLOCK_MODE0) {
		if (bCDG == TRUE) {
			_ftprintf(fpCue, _T("  TRACK %02d CDG\n"), nTrackNum);
		}
		else {
			_ftprintf(fpCue, _T("  TRACK %02d AUDIO\n"), nTrackNum);
		}
		if (bISRC) {
			_ftprintf(fpCue, _T("    ISRC %s\n"), pDiscData->pszISRC[nTrackNum-1]);
		}
		if (pDiscData->pszTitle[nTrackNum][0] != 0) {
			_ftprintf(fpCue, _T("    TITLE \"%s\"\n"), pDiscData->pszTitle[nTrackNum]);
		}
		if (pDiscData->pszPerformer[nTrackNum][0] != 0) {
			_ftprintf(fpCue, _T("    PERFORMER \"%s\"\n"), pDiscData->pszPerformer[nTrackNum]);
		}
		if (pDiscData->pszSongWriter[nTrackNum][0] != 0) {
			_ftprintf(fpCue, _T("    SONGWRITER \"%s\"\n"), pDiscData->pszSongWriter[nTrackNum]);
		}
		switch (byCtl & ~AUDIO_DATA_TRACK) {
		case AUDIO_WITH_PREEMPHASIS:
			_ftprintf(fpCue, _T("    FLAGS PRE\n"));
			break;
		case DIGITAL_COPY_PERMITTED:
			_ftprintf(fpCue, _T("    FLAGS DCP\n"));
			break;
		case DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
			_ftprintf(fpCue, _T("    FLAGS DCP PRE\n"));
			break;
		case TWO_FOUR_CHANNEL_AUDIO:
			_ftprintf(fpCue, _T("    FLAGS 4CH\n"));
			break;
		case TWO_FOUR_CHANNEL_AUDIO | AUDIO_WITH_PREEMPHASIS:
			_ftprintf(fpCue, _T("    FLAGS 4CH PRE\n"));
			break;
		case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED:
			_ftprintf(fpCue, _T("    FLAGS 4CH DCP\n"));
			break;
		case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
			_ftprintf(fpCue, _T("    FLAGS 4CH DCP PRE\n"));
			break;
		}
	}
	else {
		_ftprintf(fpCue, _T("  TRACK %02d MODE%1d/2352\n"), nTrackNum, byModeNum);
		if (bISRC) {
			_ftprintf(fpCue, _T("    ISRC %s\n"), pDiscData->pszISRC[nTrackNum - 1]);
		}
		if ((byCtl & DIGITAL_COPY_PERMITTED) == DIGITAL_COPY_PERMITTED) {
			_ftprintf(fpCue, _T("    FLAGS DCP\n"));
		}
	}
}

VOID WriteCueFileForIndex(
	BYTE byIndex,
	BYTE byFrame, 
	BYTE bySecond,
	BYTE byMinute,
	FILE* fpCue
	)
{
	_ftprintf(fpCue, _T("    INDEX %02d %02d:%02d:%02d\n"), 
		byIndex, byMinute, bySecond, byFrame);
}

VOID WriteMainChannel(
	PDISC_DATA pDiscData,
	LPBYTE lpBuf,
	LPINT* lpLBAStartList,
	INT nLBA,
	PCD_OFFSET_DATA pCdOffsetData,
	FILE* fpImg
	)
{
	if (pCdOffsetData->nFixStartLBA <= nLBA && nLBA < pCdOffsetData->nFixEndLBA) {
		// first sector
		if (nLBA == pCdOffsetData->nFixStartLBA) {
			fwrite(lpBuf + pCdOffsetData->uiMainDataSlideSize, sizeof(BYTE), 
				CD_RAW_SECTOR_SIZE - pCdOffsetData->uiMainDataSlideSize, fpImg);
			if (lpLBAStartList) {
				lpLBAStartList[0][0] = -150;
				lpLBAStartList[0][1] = nLBA - pCdOffsetData->nFixStartLBA;
			}
		}
		// last sector
		else if (nLBA == pCdOffsetData->nFixEndLBA - 1) {
			fwrite(lpBuf, sizeof(BYTE), pCdOffsetData->uiMainDataSlideSize, fpImg);
		}
		else {
			if (pDiscData->nStartLBAof2ndSession != -1 &&
				nLBA == pDiscData->nStartLBAof2ndSession) {
				if (pDiscData->nCombinedOffset > 0) {
					ZeroMemory(lpBuf, (size_t)pDiscData->nCombinedOffset);
				}
				else if (pDiscData->nCombinedOffset < 0) {
					// TODO: session2 and minus offset disc
				}
			}
			fwrite(lpBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpImg);
		}
	}
}

VOID WriteSubChannel(
	PDISC_DATA pDiscData,
	PREAD_CD_TRANSFER_DATA pTransferData,
	LPBYTE lpBuf,
	LPBYTE lpSubcode,
	LPBYTE lpSubcodeRaw,
	INT nLBA,
	BYTE lpCurrentTrackNum,
	FILE* fpSub,
	FILE* fpParse,
	FILE* fpCdg
	)
{
	fwrite(lpSubcode, sizeof(BYTE), CD_RAW_READ_SUBCODE_SIZE, fpSub);
	OutputMmcCDSubToLog(pDiscData, lpSubcode, lpBuf + pTransferData->dwBufSubOffset,
		nLBA, lpCurrentTrackNum, fpParse);
	if (fpCdg != NULL) {
		fwrite(lpSubcodeRaw, sizeof(BYTE), CD_RAW_READ_SUBCODE_SIZE, fpCdg);
	}
}

BOOL WriteParsingSubfile(
	LPCTSTR pszSubfile
	)
{
	BOOL bRet = TRUE;
	FILE* fpParse = CreateOrOpenFileW(
		pszSubfile, NULL, NULL, NULL, _T(".sub.txt"), _T(WFLAG), 0, 0);
	if (!fpParse) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}

	LPBYTE data = NULL;
	DISC_DATA discData = { 0 };
	FILE* fpSub = NULL;
	DWORD dwTrackAllocSize = MAXIMUM_NUMBER_TRACKS + 1;
	try {
		fpSub = CreateOrOpenFileW(
			pszSubfile, NULL, NULL, NULL, _T(".sub"), _T("rb"), 0, 0);
		if (!fpSub) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		DWORD dwFileSize = GetFileSize(0, fpSub);
		data = (LPBYTE)calloc(dwFileSize, sizeof(BYTE));
		if (!data) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		size_t uiReadSize = fread(data, sizeof(BYTE), dwFileSize, fpSub);
		FcloseAndNull(fpSub);
		if (uiReadSize < CD_RAW_READ_SUBCODE_SIZE) {
			throw FALSE;
		}

		// TODO:RtoW don't use in present
		BYTE lpSubcodeRtoW[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		BYTE byTrackNum = 1;

		if (NULL == (discData.pszISRC = (LPTSTR*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		for (DWORD h = 0; h < dwTrackAllocSize; h++) {
			if (NULL == (discData.pszISRC[h] = (LPTSTR)calloc((META_ISRC_SIZE), sizeof(_TCHAR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
		}
		for (INT i = 0, j = 0; i < (INT)dwFileSize; i += CD_RAW_READ_SUBCODE_SIZE, j++) {
			BYTE byAdr = (BYTE)(*(data + i + 12) & 0x0F);
			if (byAdr == ADR_ENCODES_MEDIA_CATALOG) {
				SetMCNToString(&discData, &data[i], discData.szCatalog, FALSE);
			}
			else if (byAdr == ADR_ENCODES_ISRC) {
				if (0 < byTrackNum && byTrackNum < dwTrackAllocSize) {
					SetISRCToString(&discData, &data[i], discData.pszISRC[byTrackNum - 1], byTrackNum, FALSE);
				}
			}
			else {
				byTrackNum = BcdToDec(*(data + i + 13));
			}
			if (0 < byTrackNum && byTrackNum < dwTrackAllocSize) {
				OutputMmcCDSubToLog(&discData, &data[i], lpSubcodeRtoW, j, byTrackNum, fpParse);
			}
			else {
				_TCHAR str[128] = { 0 };
				_stprintf(str, 
					_T("LBA[%06d, 0x%05X], Unknown, TOC[TrackNum-%02x, Index-%02x, RelativeTime-%02x:%02x:%02x, AbsoluteTime-%02x:%02x:%02x] RtoW:ZERO\n"),
					j, j, *(data + i + 13), *(data + i + 14), *(data + i + 15), *(data + i + 16), 
					*(data + i + 17), *(data + i + 19), *(data + i + 20), *(data + i + 21));
				fwrite(str, sizeof(_TCHAR), _tcslen(str), fpParse);
			}
			OutputString(_T("\rParse sub(Size) %6d/%6d"), i, dwFileSize);
		}
		OutputString(_T("\n"));
	}
	catch(BOOL bErr) {
		bRet = bErr;
	}
	FcloseAndNull(fpParse);
	FcloseAndNull(fpSub);
	for (DWORD i = 0; i < dwTrackAllocSize; i++) {
		if (discData.pszISRC) {
			FreeAndNull(discData.pszISRC[i]);
		}
	}
	FreeAndNull(discData.pszISRC);
	FreeAndNull(data);
	return bRet;
}

BOOL DescrambleMainChannelForGD(
	LPCTSTR pszPath
	)
{
	BOOL bRet = TRUE;
	FILE* fpScm = CreateOrOpenFileW(
		pszPath, NULL, NULL, NULL, _T(".scm2"), _T("rb"), 0, 0);
	if (!fpScm) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	FILE* fpImg = NULL;
	FILE* fpTbl = NULL;
	try {
		if (NULL == (fpImg = CreateOrOpenFileW(
			pszPath, NULL, NULL, NULL, _T(".img2"), _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpTbl = OpenProgrammabledFile(_T("scramble.bin"), _T("rb")))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		BYTE bufTbl[CD_RAW_SECTOR_SIZE] = { 0 };
		fread(bufTbl, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpTbl);
		FcloseAndNull(fpTbl);

		DWORD dwFileSize = GetFileSize(0, fpScm);
		DWORD dwAllSectorVal = dwFileSize / CD_RAW_SECTOR_SIZE;
		BYTE bufScm[CD_RAW_SECTOR_SIZE] = { 0 };
		BYTE bufImg[CD_RAW_SECTOR_SIZE] = { 0 };
		for (DWORD i = 0; i < dwAllSectorVal; i++) {
			fread(bufScm, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpScm);
			if (IsValidDataHeader(bufScm)) {
				if (bufScm[0x0C] == 0xC3 && bufScm[0x0D] == 0x84 && bufScm[0x0E] >= 0x00) {
					break;
				}
				for (INT j = 0; j < CD_RAW_SECTOR_SIZE; j++) {
					bufImg[j] = (BYTE)(bufScm[j] ^ bufTbl[j]);
				}
				fwrite(bufImg, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpImg);
			}
			else {
				// copy audio data
				fwrite(bufScm, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpImg);
			}
			OutputString(_T("\rDescrambling File(LBA) %6d/%6d"), i, dwAllSectorVal);
		}
		OutputString(_T("\n"));
		FcloseAndNull(fpImg);
	}
	catch(BOOL bErr) {
		bRet = bErr;
	}
	FcloseAndNull(fpScm);
	FcloseAndNull(fpImg);
	return bRet;
}

BOOL SplitFileForGD(
	LPCTSTR pszPath
	)
{
	BOOL bRet = TRUE;
	_TCHAR pszPathWithoutPathAndExt[_MAX_PATH] = { 0 };
	FILE* fpImg = CreateOrOpenFileW(pszPath, 
		NULL, NULL, pszPathWithoutPathAndExt, _T(".img2"), _T("rb"), 0, 0);
	if (!fpImg) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	FILE* fpBin = NULL;
	FILE* fpGdi = NULL;
	LPLONG lpToc = NULL;
	LPBYTE lpBuf = NULL;
	try {
		if (NULL == (fpGdi = CreateOrOpenFileW(pszPath,
			NULL, NULL, NULL, _T(".gdi"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		DWORD dwFileSize = GetFileSize(0, fpImg);
		if (dwFileSize < 0x110 + 512) {
			OutputErrorString(
				_T("[F:%s][L:%d] Not GD-ROM data\n"), _T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		fseek(fpImg, 0x110, SEEK_SET);
		// 0x110 - 0x31F is toc data
		BYTE aToc[512] = { 0 };
		fread(aToc, sizeof(BYTE), sizeof(aToc), fpImg);
		if (aToc[0] != 'T' || aToc[1] != 'O' || aToc[2] != 'C' || aToc[3] != '1') {
			OutputErrorString(
				_T("[F:%s][L:%d] Not GD-ROM data\n"), _T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		INT nTrackNum = 3;
		UINT uiMaxToc = 98 * 4;
		UINT uiMaxTrackNum = aToc[uiMaxToc + 4 * 1 + 2];
		lpToc = (PLONG)calloc(uiMaxTrackNum, sizeof(UINT));
		if (!lpToc) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LONG lMaxLba = 0;
		for (UINT i = 0; i < uiMaxToc; i += 4) {
			if (aToc[7+i] == 0xFF) {
				lMaxLba = MAKELONG(
					MAKEWORD(aToc[4 + i - 4], aToc[5 + i - 4]), MAKEWORD(aToc[6 + i - 4], 0));
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
			if (aToc[7 + i] == 0xFF) {
				break;
			}
			BYTE byCtl = (BYTE)((aToc[7 + i] >> 4) & 0x0F);
			LONG lToc =
				MAKELONG(MAKEWORD(aToc[4 + i], aToc[5 + i]), MAKEWORD(aToc[6 + i], 0));
			lpToc[nTrackNum - 3] = lToc - 300;
			if (nTrackNum == 3) {
				lpToc[nTrackNum - 3] += 150;
				OutputDiscLog(
					_T("Track %2d, Ctl %d,              , Index1 %6d\n"), 
					nTrackNum, byCtl, lpToc[nTrackNum - 3]);
			}
			else {
				if ((byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					lpToc[nTrackNum - 3] -= 75;
					OutputDiscLog(
						_T("Track %2d, Ctl %d, Index0 %6d, Index1 %6d\n"), 
						nTrackNum, byCtl, lpToc[nTrackNum - 3], lpToc[nTrackNum - 3] + 225);
				}
				else {
					OutputDiscLog(
						_T("Track %2d, Ctl %d, Index0 %6d, Index1 %6d\n"), 
						nTrackNum, byCtl, lpToc[nTrackNum - 3], lpToc[nTrackNum - 3] + 150);
				}
			}
			if (uiMaxTrackNum <= 9 && lMaxLba <= 99999) {
				_ftprintf(fpGdi, _T("%d %5d %d 2352 \"%s (Track %d).bin\" 0\n"),
					nTrackNum, lpToc[nTrackNum - 3], byCtl, pszPathWithoutPathAndExt, nTrackNum);
			}
			else if (10 <= uiMaxTrackNum && lMaxLba <= 99999) {
				_ftprintf(fpGdi, _T("%2d %5d %d 2352 \"%s (Track %02d).bin\" 0\n"),
					nTrackNum, lpToc[nTrackNum - 3], byCtl, pszPathWithoutPathAndExt, nTrackNum);
			}
			else if (uiMaxTrackNum <= 9 && 100000 <= lMaxLba) {
				_ftprintf(fpGdi, _T("%d %6d %d 2352 \"%s (Track %d).bin\" 0\n"),
					nTrackNum, lpToc[nTrackNum - 3], byCtl, pszPathWithoutPathAndExt, nTrackNum);
			}
			else if (10 <= uiMaxTrackNum && 100000 <= lMaxLba) {
				_ftprintf(fpGdi, _T("%2d %6d %d 2352 \"%s (Track %02d).bin\" 0\n"),
					nTrackNum, lpToc[nTrackNum - 3], byCtl, pszPathWithoutPathAndExt, nTrackNum);
			}
		}
		LONG lToc = 
			MAKELONG(MAKEWORD(aToc[uiMaxToc + 4 * 2], aToc[uiMaxToc + 4 * 2 + 1]),
			MAKEWORD(aToc[uiMaxToc + 4 * 2 + 2], 0)) - 150;
		OutputDiscLog(
			_T("MaxTrackNum %2d\n")
			_T("Leadout, LBA %6d\n"),
			uiMaxTrackNum, lToc);
		lpToc[nTrackNum - 3] = lToc;

		rewind(fpImg);
		for (UINT i = 3; i <= uiMaxTrackNum; i++) {
			OutputString(_T("\rSplit File(num) %2d/%2d"), i, uiMaxTrackNum);
			if (NULL == (fpBin = CreateOrOpenFileW(pszPath, NULL,
				NULL, NULL, _T(".bin"), _T("wb"), i, uiMaxTrackNum))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			size_t size = 
				(size_t)(lpToc[i - 2] - lpToc[i - 3]) * CD_RAW_SECTOR_SIZE;
			if (NULL == (lpBuf = (LPBYTE)calloc(size, sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			fread(lpBuf, sizeof(BYTE), size, fpImg);
			fwrite(lpBuf, sizeof(BYTE), size, fpBin);
			FcloseAndNull(fpBin);
			FreeAndNull(lpBuf);
		}
		OutputString(_T("\n"));
	}
	catch(BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpImg);
	FcloseAndNull(fpBin);
	FreeAndNull(lpToc);
	return bRet;
}

VOID DescrambleMainChannel(
	PDISC_DATA pDiscData,
	LPINT* lpLBAOfDataTrackList,
	FILE* fpTbl,
	FILE* fpImg
	)
{
	BYTE aScrambledBuf[CD_RAW_SECTOR_SIZE] = { 0 };
	fread(aScrambledBuf, sizeof(BYTE), sizeof(aScrambledBuf), fpTbl);
	BYTE aSrcBuf[CD_RAW_SECTOR_SIZE] = { 0 };

	for (INT k = 0; k < pDiscData->toc.LastTrack; k++) {
		if (lpLBAOfDataTrackList[k][0] != -1) {
			OutputDiscLog(_T("\tData Sector, LBA %6d-%6d\n"), 
				lpLBAOfDataTrackList[k][0], lpLBAOfDataTrackList[k][1]);
			UINT nStartLBA = pDiscData->puiSessionNum[k] >= 2 
				? lpLBAOfDataTrackList[k][0] - (11400 * (pDiscData->puiSessionNum[k] - 1)) 
				: lpLBAOfDataTrackList[k][0];
			UINT nEndLBA = pDiscData->puiSessionNum[k] >= 2 
				? lpLBAOfDataTrackList[k][1] - (11400 * (pDiscData->puiSessionNum[k] - 1)) 
				: lpLBAOfDataTrackList[k][1];
			for (; nStartLBA <= nEndLBA; nStartLBA++) {
				// ファイルを読み書き両用モードで開いている時は 注意が必要です。
				// 読み込みを行った後に書き込みを行う場合やその逆を行う場合は、 
				// 必ずfseekを呼ばなければなりません。もしこれを忘れると、
				// 場合によってはバッファー内と 実際にディスクに描き込まれた
				// データに矛盾が生じ、正確に書き込まれない場合や、
				// 嘘の データを読み込む場合があります。
				fseek(fpImg, (long)nStartLBA * CD_RAW_SECTOR_SIZE, SEEK_SET);
				fread(aSrcBuf, sizeof(BYTE), sizeof(aSrcBuf), fpImg);
				if (IsValidDataHeader(aSrcBuf)) {
					fseek(fpImg, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
					for (INT n = 0; n < CD_RAW_SECTOR_SIZE; n++) {
						aSrcBuf[n] ^= aScrambledBuf[n];
					}
					fwrite(aSrcBuf, sizeof(BYTE), sizeof(aSrcBuf), fpImg);
				}
				OutputString(
					_T("\rDescrambling data sector of img(LBA) %6d/%6d"), nStartLBA, nEndLBA);
			}
			OutputString(_T("\n"));
		}
	}
}

BOOL MergeMainChannelAndCDG(
	LPCTSTR pszPath,
	BOOL bCDG,
	BOOL bAudioOnly,
	FILE* fpImg
	)
{
	BOOL bRet = TRUE;
	if (bCDG == TRUE && bAudioOnly) {
		FILE* fpBinAll = NULL;
		FILE* fpCdg =
			CreateOrOpenFileW(pszPath, NULL, NULL, NULL, _T(".cdg"), _T("rb"), 0, 0);
		if (fpCdg == NULL) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		try {
			fpBinAll =
				CreateOrOpenFileW(pszPath, NULL, NULL, NULL, _T(".bin"), _T("wb"), 0, 0);
			if (fpBinAll == NULL) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			DWORD dwFileSize = GetFileSize(0, fpImg);
			DWORD dwSectorSize = dwFileSize / CD_RAW_SECTOR_SIZE;
			BYTE lpBuf[CD_RAW_SECTOR_WITH_SUBCODE_SIZE] = { 0 };

			for (DWORD i = 0; i < dwSectorSize; i++) {
				fread(lpBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpImg);
				fread(lpBuf + CD_RAW_SECTOR_SIZE,
					sizeof(BYTE), CD_RAW_READ_SUBCODE_SIZE, fpCdg);
				fwrite(lpBuf, sizeof(BYTE), CD_RAW_SECTOR_WITH_SUBCODE_SIZE, fpBinAll);
				OutputString(
					_T("\rMerging img + cdg -> bin(LBA) %6d/%6d"), i, dwSectorSize - 1);
			}
			OutputString(_T("\n"));
			FcloseAndNull(fpBinAll);
		}
		catch(BOOL bErr) {
			bRet = bErr;
		}
		FcloseAndNull(fpCdg);
	}
	return bRet;
}

BOOL CreatingBinCueCcd(
	PDISC_DATA pDiscData,
	LPCTSTR pszPath,
	BOOL lpCatalog,
	BOOL bCDG,
	LPBYTE lpCtlList,
	LPBYTE lpModeList,
	LPBOOL lpISRCList,
	LPINT* lpLBAStartList,
	FILE* fpImg,
	FILE* fpCue,
	FILE* fpCcd
	)
{
	BOOL bRet = TRUE;
	_TCHAR pszPathWithoutPath[_MAX_FNAME] = { 0 };
	FILE* fpBinWithCDG = NULL;
	if (bCDG == TRUE && pDiscData->bAudioOnly) {
		fpBinWithCDG = 
			CreateOrOpenFileW(pszPath, NULL, NULL, NULL, _T(".bin"), _T("rb"), 0, 0);
		if (fpBinWithCDG == NULL) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}
	for (UINT i = pDiscData->toc.FirstTrack; i <= pDiscData->toc.LastTrack; i++) {
		FILE* fpBin = CreateOrOpenFileW(pszPath, NULL, pszPathWithoutPath,
			NULL, _T(".bin"), _T("wb"), i, pDiscData->toc.LastTrack);
		if (fpBin == NULL) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			bRet = FALSE;
			break;
		}
		else {
			OutputString(_T("\rCreating bin, cue, ccd(Track) %2d/%2d"),
				i, pDiscData->toc.LastTrack);
			if (i == pDiscData->toc.FirstTrack) {
				WriteCueFileFirst(pDiscData, lpCatalog, fpCue);
			}
			WriteCueFile(pDiscData, pszPathWithoutPath, bCDG,
				i, lpModeList[i - 1], lpISRCList[i - 1], lpCtlList[i - 1], fpCue);
			WriteCcdFileForTrack(pDiscData, i,
				lpModeList[i - 1], lpISRCList[i - 1], lpCtlList[i - 1], fpCcd);

			BYTE byFrame = 0, bySecond = 0, byMinute = 0;
			if (i == pDiscData->toc.FirstTrack) {
				LBAtoMSF(lpLBAStartList[i - 1][1] - lpLBAStartList[i - 1][0] - 150,
					&byFrame, &bySecond, &byMinute);
				if (lpLBAStartList[i - 1][1] > 0) {
					// index 0 in track 1
					//  Crow, The - Original Motion Picture Soundtrack (82519-2)
					//  Now on Never (Nick Carter) (ZJCI-10118)
					//  etc..
					WriteCueFileForIndex(0, 0, 0, 0, fpCue);
					WriteCcdFileForTrackIndex(0, 0, fpCcd);
				}
				WriteCueFileForIndex(1, byFrame, bySecond, byMinute, fpCue);
				WriteCcdFileForTrackIndex(1, lpLBAStartList[i - 1][1], fpCcd);
			}
			else if (lpLBAStartList[i - 1][0] == -1 && lpLBAStartList[i - 1][2] == -1) {
				LBAtoMSF(lpLBAStartList[i - 1][1] - lpLBAStartList[i - 1][0],
					&byFrame, &bySecond, &byMinute);
				WriteCueFileForIndex(1, 0, 0, 0, fpCue);
				WriteCcdFileForTrackIndex(1, lpLBAStartList[i - 1][1], fpCcd);
			}
			else if (lpLBAStartList[i - 1][0] != -1) {
				for (BYTE index = 0; index < MAXIMUM_NUMBER_INDEXES; index++) {
					if (lpLBAStartList[i - 1][index] != -1) {
						LBAtoMSF(lpLBAStartList[i - 1][index] - lpLBAStartList[i - 1][0],
							&byFrame, &bySecond, &byMinute);
						WriteCueFileForIndex(index, byFrame, bySecond, byMinute, fpCue);
						WriteCcdFileForTrackIndex(index, lpLBAStartList[i - 1][index], fpCcd);
					}
				}
			}
			else {
				for (BYTE index = 1; index < MAXIMUM_NUMBER_INDEXES; index++) {
					if (lpLBAStartList[i - 1][index] != -1) {
						LBAtoMSF(lpLBAStartList[i - 1][index] - lpLBAStartList[i - 1][1],
							&byFrame, &bySecond, &byMinute);
						WriteCueFileForIndex(index, byFrame, bySecond, byMinute, fpCue);
						WriteCcdFileForTrackIndex(index, lpLBAStartList[i - 1][index], fpCcd);
					}
				}
			}
			// write each track
			size_t uiBufsize = 0;
			INT nLBA =
				lpLBAStartList[i][0] == -1 ? lpLBAStartList[i][1] : lpLBAStartList[i][0];
			INT nPrevLba =
				lpLBAStartList[i - 1][0] == -1 ? lpLBAStartList[i - 1][1] : lpLBAStartList[i - 1][0];
			INT nWriteSectorSize =
				(bCDG == TRUE && pDiscData->bAudioOnly) ? CD_RAW_SECTOR_WITH_SUBCODE_SIZE : CD_RAW_SECTOR_SIZE;

			if (pDiscData->toc.LastTrack == pDiscData->toc.FirstTrack) {
				uiBufsize = (size_t)pDiscData->nAllLength * nWriteSectorSize;
				nPrevLba = 0;
			}
			else if (i == pDiscData->toc.FirstTrack) {
				uiBufsize = (size_t)nLBA * nWriteSectorSize;
				nPrevLba = 0;
			}
			else if (i == pDiscData->toc.LastTrack) {
				INT nTmpLength = pDiscData->nAllLength;
				if (pDiscData->puiSessionNum[i - 1] > 1) {
					UINT nLeadinoutSize = 11400 * (pDiscData->puiSessionNum[i - 1] - 1);
					nPrevLba -= nLeadinoutSize;
					nTmpLength -= nLeadinoutSize;
				}
				uiBufsize = (size_t)(nTmpLength - nPrevLba) * nWriteSectorSize;
			}
			else {
				if (i == pDiscData->toc.LastTrack - (UINT)1 && pDiscData->puiSessionNum[i] > 1) {
					nLBA -= 11400 * (pDiscData->puiSessionNum[i] - 1);
				}
				uiBufsize = (size_t)(nLBA - nPrevLba) * nWriteSectorSize;
			}
			if (!(bCDG == TRUE && pDiscData->bAudioOnly)) {
				fseek(fpImg, nPrevLba * nWriteSectorSize, SEEK_SET);
			}
			try {
				LPBYTE lpBuf = (LPBYTE)calloc(uiBufsize, sizeof(BYTE));
				if (lpBuf == NULL) {
					OutputString(_T("\n"));
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				else {
					if (bCDG == TRUE && pDiscData->bAudioOnly) {
						fread(lpBuf, sizeof(BYTE), uiBufsize, fpBinWithCDG);
					}
					else {
						fread(lpBuf, sizeof(BYTE), uiBufsize, fpImg);
					}
					fwrite(lpBuf, sizeof(BYTE), uiBufsize, fpBin);
					FreeAndNull(lpBuf);
				}
			}
			catch(BOOL bErr) {
				bRet = bErr;
			}
			FcloseAndNull(fpBin);
			if (!bRet) {
				break;
			}
		}
	}
	FcloseAndNull(fpBinWithCDG);
	OutputString(_T("\n"));
	return bRet;
}

VOID OutputHashData(
	FILE* fpHash,
	LPCTSTR filename,
	UINT64 ui64FileSize,
	DWORD crc32,
	LPBYTE digest,
	LPBYTE Message_Digest
	)
{
	_ftprintf(fpHash, 
		_T("\t\t<rom name=\"%s\" size=\"%llu\" crc=\"%08x\" md5=\""),
		filename, ui64FileSize, crc32);
	for (INT i = 0; i < 16; i++) {
		_ftprintf(fpHash, _T("%02x"), digest[i]);
	}
	_ftprintf(fpHash, _T("\" sha1=\""));
	for (INT i = 0; i < 20; i++) {
		_ftprintf(fpHash, _T("%02x"), Message_Digest[i]);
	}
	_ftprintf(fpHash, _T("\"/>\n"));
}

VOID OutputLastErrorNumAndString(
	LPCTSTR pszFuncName,
	LONG lLineNum
	)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

	OutputErrorString(_T("[F:%s][L:%d] GetLastError: %d, %s\n"), 
		pszFuncName, lLineNum, GetLastError(), (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
