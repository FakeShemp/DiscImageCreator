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
#include "outputMMCLogforCD.h"
#include "set.h"

#ifdef _DEBUG
_TCHAR logBuffer[2048];
CHAR logBufferA[2048];
#endif
// These global variable is set at printAndSetArg().
extern _TCHAR g_szCurrentdir[_MAX_PATH];

FILE* CreateOrOpenFileW(
	LPCTSTR pszPath,
	LPCTSTR pszPlusFname,
	LPTSTR pszOutPath,
	LPTSTR pszPathWithoutPath,
	LPTSTR pszPathWithoutPathAndExt,
	LPCTSTR pszExt,
	LPCTSTR pszMode,
	BYTE byTrackNum,
	BYTE byMaxTrackNum
	)
{
	_TCHAR szDstPath[_MAX_PATH] = { 0 };
	_TCHAR szTmpPath[_MAX_PATH + 1] = { 0 };
	_TCHAR drive[_MAX_DRIVE] = { 0 };
	_TCHAR dir[_MAX_DIR] = { 0 };
	_TCHAR fname[_MAX_FNAME] = { 0 };
	_TCHAR ext[_MAX_EXT] = { 0 };

	_tsplitpath(pszPath, drive, dir, fname, ext);
	if (pszPlusFname) {
		size_t plusFnameLen = _tcslen(pszPlusFname);
		DWORD pathSize = DWORD(_tcslen(dir) + _tcslen(fname) + plusFnameLen + _tcslen(pszExt));
		if (pathSize > _MAX_FNAME) {
			OutputErrorString(_T("File Size is too long: %u\n"), pathSize);
			return NULL;
		}
		_tcsncat(fname, pszPlusFname, plusFnameLen);
	}
	if (!drive[0] || !dir[0]) {
		_tcsncpy(szTmpPath, g_szCurrentdir, _MAX_PATH);
		szTmpPath[_MAX_PATH] = 0;
		if (dir[0]) {
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

	if (drive[0] && dir[0]) {
		_tmakepath(szTmpPath, drive, dir, NULL, NULL);
		if (!PathIsDirectory(szTmpPath)) {
			if (!CreateDirectory(szTmpPath, NULL)) {
				return NULL;
			}
		}
	}

	if (byMaxTrackNum <= 1) {
		_sntprintf(szDstPath, _MAX_PATH,
			_T("%s%s%s%s"), drive, dir, fname, pszExt);
	}
	else if (2 <= byMaxTrackNum && byMaxTrackNum <= 9) {
		_sntprintf(szDstPath, _MAX_PATH,
			_T("%s%s%s (Track %u)%s"), drive, dir, fname, byTrackNum, pszExt);
	}
	else if (10 <= byMaxTrackNum) {
		_sntprintf(szDstPath, _MAX_PATH,
			_T("%s%s%s (Track %02u)%s"), drive, dir, fname, byTrackNum, pszExt);
	}
	szDstPath[_MAX_PATH - 1] = 0;

	if (pszPathWithoutPath) {
		// size of pszPathWithoutPath must be _MAX_PATH.
		ZeroMemory(pszPathWithoutPath, _MAX_FNAME);
		_tsplitpath(szDstPath, drive, dir, fname, ext);
		_sntprintf(pszPathWithoutPath, _MAX_FNAME, _T("%s%s"), fname, ext);
	}
	if (pszPathWithoutPathAndExt) {
		// size of pszPathWithoutPathAndExt must be _MAX_PATH.
		ZeroMemory(pszPathWithoutPathAndExt, _MAX_FNAME);
		_tsplitpath(szDstPath, drive, dir, fname, ext);
		_sntprintf(pszPathWithoutPathAndExt, _MAX_FNAME, _T("%s"), fname);
	}
	if (pszOutPath) {
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
	LPCSTR pszPlusFname,
	LPSTR pszOutPath,
	LPSTR pszPathWithoutPath,
	LPSTR pszPathWithoutPathAndExt,
	LPCSTR pszExt,
	LPCSTR pszMode,
	BYTE byTrackNum,
	BYTE byMaxTrackNum
	)
{
	CHAR szDstPath[_MAX_PATH] = { 0 };
	CHAR szTmpPath[_MAX_PATH + 1] = { 0 };
	CHAR drive[_MAX_DRIVE] = { 0 };
	CHAR dir[_MAX_DIR] = { 0 };
	CHAR fname[_MAX_FNAME] = { 0 };
	CHAR ext[_MAX_EXT] = { 0 };

	_splitpath(pszPath, drive, dir, fname, ext);
	if (pszPlusFname) {
		size_t plusFnameLen = strlen(pszPlusFname);
		DWORD pathSize = DWORD(strlen(dir) + strlen(fname) + plusFnameLen + strlen(pszExt));
		if (pathSize > _MAX_FNAME) {
			OutputErrorString(_T("File Size is too long: %u\n"), pathSize);
			return NULL;
		}
		strncat(fname, pszPlusFname, plusFnameLen);
	}
	if (!drive[0] || !dir[0]) {
		CHAR curpath[_MAX_PATH] = { 0 };
#ifdef UNICODE
		WideCharToMultiByte(CP_ACP, 0, g_szCurrentdir, _MAX_PATH, curpath, sizeof(curpath), NULL, NULL);
#else
		strncpy(curpath, g_szCurrentdir, sizeof(curpath));
#endif
		curpath[_MAX_PATH - 1] = 0;
		strncpy(szTmpPath, curpath, _MAX_PATH);
		szTmpPath[_MAX_PATH] = 0;
		if (dir[0]) {
			if (!PathAppendA(szTmpPath, dir)) {
				return NULL;
			}
		}
		if (!PathAppendA(szTmpPath, fname)) {
			return NULL;
		}
		memcpy(szDstPath, szTmpPath, _MAX_PATH);
		_splitpath(szDstPath, drive, dir, fname, NULL);
	}

	if (drive[0] && dir[0]) {
		_makepath(szTmpPath, drive, dir, NULL, NULL);
		if (!PathIsDirectoryA(szTmpPath)) {
			if (!CreateDirectoryA(szTmpPath, NULL)) {
				return NULL;
			}
		}
	}

	if (byMaxTrackNum <= 1) {
		_snprintf(szDstPath, _MAX_PATH,
			"%s%s%s%s", drive, dir, fname, pszExt);
	}
	else if (2 <= byMaxTrackNum && byMaxTrackNum <= 9) {
		_snprintf(szDstPath, _MAX_PATH,
			"%s%s%s (Track %u)%s", drive, dir, fname, byTrackNum, pszExt);
	}
	else if (10 <= byMaxTrackNum) {
		_snprintf(szDstPath, _MAX_PATH,
			"%s%s%s (Track %02u)%s", drive, dir, fname, byTrackNum, pszExt);
	}
	szDstPath[_MAX_PATH - 1] = 0;

	if (pszPathWithoutPath) {
		// size of pszPathWithoutPath must be _MAX_PATH.
		ZeroMemory(pszPathWithoutPath, _MAX_FNAME);
		_splitpath(szDstPath, drive, dir, fname, ext);
		_snprintf(pszPathWithoutPath, _MAX_FNAME, "%s%s", fname, ext);
	}
	if (pszPathWithoutPathAndExt) {
		// size of pszPathWithoutPathAndExt must be _MAX_PATH.
		ZeroMemory(pszPathWithoutPathAndExt, _MAX_FNAME);
		_splitpath(szDstPath, drive, dir, fname, ext);
		_snprintf(pszPathWithoutPathAndExt, _MAX_FNAME, "%s", fname);
	}
	if (pszOutPath) {
		// size of pszOutPath must be _MAX_PATH.
		strncpy(pszOutPath, szDstPath, _MAX_PATH);
	}
	FILE* fp = fopen(szDstPath, pszMode);
	return fp;
}

FILE* OpenProgrammabledFile(
	LPCTSTR pszPath,
	LPCTSTR pszMode
	)
{
	_TCHAR path[_MAX_PATH] = { 0 };
	if (!::GetModuleFileName(NULL, path, _MAX_PATH)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return NULL;
	}
	FILE* fp = NULL;
	_TCHAR* pdest = _tcsrchr(path, '\\');
	if (pdest) {
		pdest[0] = NULL;
		_TCHAR lpBuf[_MAX_PATH] = { 0 };
		_sntprintf(lpBuf, _MAX_PATH, _T("%s\\%s"), path, pszPath);
		lpBuf[_MAX_PATH - 1] = 0;
		fp = _tfopen(lpBuf, pszMode);
	}
	return fp;
}

VOID WriteCcdFileForDisc(
	WORD wTocEntries,
	BYTE LastCompleteSession,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd,
		_T("[CloneCD]\n")
		_T("Version=3\n")
		_T("[Disc]\n")
		_T("TocEntries=%u\n")
		_T("Sessions=%u\n")
		_T("DataTracksScrambled=%u\n"),
		wTocEntries,
		LastCompleteSession,
		0); // TODO
}

VOID WriteCcdFileForDiscCDTextLength(
	WORD wCDTextLength,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd, _T("CDTextLength=%u\n"), wCDTextLength);
}

VOID WriteCcdFileForDiscCatalog(
	PDISC pDisc,
	FILE* fpCcd
	)
{
	_TCHAR str[META_CATALOG_SIZE] = { 0 };
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, 
		pDisc->SUB.szCatalog, META_CATALOG_SIZE, str, META_CATALOG_SIZE);
#else
	strncpy(str, pDisc->SUB.szCatalog, META_CATALOG_SIZE);
#endif
	_ftprintf(fpCcd, _T("CATALOG=%s\n"), str);
}

VOID WriteCcdFileForCDText(
	WORD wTocTextEntries,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd,
		_T("[CDText]\n")
		_T("Entries=%u\n"),
		wTocTextEntries);
}

VOID WriteCcdFileForCDTextEntry(
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	WORD wTocTextEntries,
	FILE* fpCcd
	)
{
	for (WORD t = 0; t < wTocTextEntries; t++) {
		_ftprintf(fpCcd, 
			_T("Entry %u=%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"), 
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
		_T("[Session %u]\n")
		_T("PreGapMode=%u\n")
		_T("PreGapSubC=%u\n"),
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
		_T("[Entry %u]\n")
		_T("Session=%u\n")
		_T("Point=0x%02x\n")
		_T("ADR=0x%02x\n")
		_T("Control=0x%02x\n")
		_T("TrackNo=%u\n")
		_T("AMin=%u\n")
		_T("ASec=%u\n")
		_T("AFrame=%u\n")
		_T("ALBA=%d\n")
		_T("Zero=%u\n")
		_T("PMin=%u\n")
		_T("PSec=%u\n")
		_T("PFrame=%u\n")
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
		MSFtoLBA(toc[a].MsfExtra[0], toc[a].MsfExtra[1], toc[a].MsfExtra[2]) - 150,
		toc[a].Zero,
		toc[a].Msf[0],
		toc[a].Msf[1],
		toc[a].Msf[2], 
		MSFtoLBA(toc[a].Msf[0], toc[a].Msf[1], toc[a].Msf[2]) - 150);
}

VOID WriteCcdFileForTrack(
	PDISC pDisc,
	BYTE byTrackNum,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd,
		_T("[TRACK %u]\n")
		_T("MODE=%u\n"),
		byTrackNum,
		pDisc->SUB.lpModeList[byTrackNum - 1]);
	if (pDisc->SUB.lpISRCList[byTrackNum - 1]) {
		_TCHAR str[META_ISRC_SIZE] = { 0 };
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, 
			pDisc->SUB.pszISRC[byTrackNum - 1], META_ISRC_SIZE, str, META_ISRC_SIZE);
#else
		strncpy(str, pDisc->SUB.pszISRC[byTrackNum - 1], META_ISRC_SIZE);
#endif
		_ftprintf(fpCcd, _T("ISRC=%s\n"), str);
	}
	switch (pDisc->SUB.lpCtlList[byTrackNum - 1] & ~AUDIO_DATA_TRACK) {
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
	BYTE byIndex,
	INT nLBA,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd, _T("INDEX %u=%d\n"), byIndex, nLBA);
}

VOID WriteCueForFirst(
	PDISC pDisc,
	BOOL lpCatalog,
	FILE* fpCue
	)
{
	if (lpCatalog) {
		_TCHAR str[META_CATALOG_SIZE] = { 0 };
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, 
			pDisc->SUB.szCatalog, META_CATALOG_SIZE, str, META_CATALOG_SIZE);
#else
		strncpy(str, pDisc->SUB.szCatalog, META_CATALOG_SIZE);
#endif
		_ftprintf(fpCue, _T("CATALOG %s\n"), str);
	}
	if (pDisc->SCSI.pszTitle[0][0] != 0) {
		_TCHAR str[META_CDTEXT_SIZE] = { 0 };
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, 
			pDisc->SCSI.pszTitle[0], META_CDTEXT_SIZE, str, META_CDTEXT_SIZE);
#else
		strncpy(str, pDisc->SUB.szCatalog, META_CATALOG_SIZE);
#endif
		_ftprintf(fpCue, _T("TITLE \"%s\"\n"), str);
	}
	if (pDisc->SCSI.pszPerformer[0][0] != 0) {
		_TCHAR str[META_CDTEXT_SIZE] = { 0 };
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0,
			pDisc->SCSI.pszPerformer[0], META_CDTEXT_SIZE, str, META_CDTEXT_SIZE);
#else
		strncpy(str, pDisc->SCSI.pszPerformer[0], META_CATALOG_SIZE);
#endif
		_ftprintf(fpCue, _T("PERFORMER \"%s\"\n"), str);
	}
	if (pDisc->SCSI.pszSongWriter[0][0] != 0) {
		_TCHAR str[META_CDTEXT_SIZE] = { 0 };
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0,
			pDisc->SCSI.pszSongWriter[0], META_CDTEXT_SIZE, str, META_CDTEXT_SIZE);
#else
		strncpy(str, pDisc->SCSI.pszSongWriter[0], META_CATALOG_SIZE);
#endif
		_ftprintf(fpCue, _T("SONGWRITER \"%s\"\n"), str);
	}
}

VOID WriteCueForFileDirective(
	LPCTSTR pszPath,
	FILE* fpCue
	)
{
	_ftprintf(fpCue, _T("FILE \"%s\" BINARY\n"), pszPath);
}

VOID WriteCueForUnderFileDirective(
	PDISC pDisc,
	BYTE byTrackNum,
	FILE* fpCue
	)
{
	if (pDisc->SUB.lpModeList[byTrackNum - 1] == DATA_BLOCK_MODE0) {
		_ftprintf(fpCue, _T("  TRACK %02u AUDIO\n"), byTrackNum);
		if (pDisc->SUB.lpISRCList[byTrackNum - 1]) {
			_TCHAR str[META_ISRC_SIZE] = { 0 };
#ifdef UNICODE
			MultiByteToWideChar(CP_ACP, 0,
				pDisc->SUB.pszISRC[byTrackNum - 1], META_ISRC_SIZE, str, META_ISRC_SIZE);
#else
			strncpy(str, pDisc->SUB.pszISRC[byTrackNum - 1], META_ISRC_SIZE);
#endif
			_ftprintf(fpCue, _T("    ISRC %s\n"), str);
		}
		if (pDisc->SCSI.pszTitle[byTrackNum][0] != 0) {
			_TCHAR str[META_CDTEXT_SIZE] = { 0 };
#ifdef UNICODE
			MultiByteToWideChar(CP_ACP, 0,
				pDisc->SCSI.pszTitle[byTrackNum], META_CDTEXT_SIZE, str, META_CDTEXT_SIZE);
#else
			strncpy(str, pDisc->SCSI.pszTitle[byTrackNum], META_CATALOG_SIZE);
#endif
			_ftprintf(fpCue, _T("    TITLE \"%s\"\n"), str);
		}
		if (pDisc->SCSI.pszPerformer[byTrackNum][0] != 0) {
			_TCHAR str[META_CDTEXT_SIZE] = { 0 };
#ifdef UNICODE
			MultiByteToWideChar(CP_ACP, 0,
				pDisc->SCSI.pszPerformer[byTrackNum], META_CDTEXT_SIZE, str, META_CDTEXT_SIZE);
#else
			strncpy(str, pDisc->SCSI.pszPerformer[byTrackNum], META_CATALOG_SIZE);
#endif
			_ftprintf(fpCue, _T("    PERFORMER \"%s\"\n"), str);
		}
		if (pDisc->SCSI.pszSongWriter[byTrackNum][0] != 0) {
			_TCHAR str[META_CDTEXT_SIZE] = { 0 };
#ifdef UNICODE
			MultiByteToWideChar(CP_ACP, 0,
				pDisc->SCSI.pszSongWriter[byTrackNum], META_CDTEXT_SIZE, str, META_CDTEXT_SIZE);
#else
			strncpy(str, pDisc->SCSI.pszSongWriter[byTrackNum], META_CATALOG_SIZE);
#endif
			_ftprintf(fpCue, _T("    SONGWRITER \"%s\"\n"), str);
		}
		switch (pDisc->SUB.lpCtlList[byTrackNum - 1] & ~AUDIO_DATA_TRACK) {
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
		if (pDisc->SCSI.bCdi) {
			_ftprintf(fpCue, _T("  TRACK %02u CDI/2352\n"), byTrackNum);
		}
		else {
			_ftprintf(fpCue, _T("  TRACK %02u MODE%1u/2352\n"), 
				byTrackNum, pDisc->SUB.lpModeList[byTrackNum - 1]);
		}
		if (pDisc->SUB.lpISRCList[byTrackNum - 1]) {
			_TCHAR str[META_ISRC_SIZE] = { 0 };
#ifdef UNICODE
			MultiByteToWideChar(CP_ACP, 0,
				pDisc->SUB.pszISRC[byTrackNum - 1], META_ISRC_SIZE, str, META_ISRC_SIZE);
#else
			strncpy(str, pDisc->SUB.pszISRC[byTrackNum - 1], META_ISRC_SIZE);
#endif
			_ftprintf(fpCue, _T("    ISRC %s\n"), str);
		}
		if ((pDisc->SUB.lpCtlList[byTrackNum - 1] & DIGITAL_COPY_PERMITTED) == DIGITAL_COPY_PERMITTED) {
			_ftprintf(fpCue, _T("    FLAGS DCP\n"));
		}
	}
}

VOID WriteCueForIndexDirective(
	BYTE byIndex,
	BYTE byMinute,
	BYTE bySecond,
	BYTE byFrame,
	FILE* fpCue
	)
{
	_ftprintf(fpCue, _T("    INDEX %02u %02u:%02u:%02u\n"), 
		byIndex, byMinute, bySecond, byFrame);
}

VOID WriteMainChannel(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	INT nLBA,
	FILE* fpImg
	)
{
	INT sLBA = pDisc->MAIN.nFixStartLBA;
	INT eLBA = pDisc->MAIN.nFixEndLBA;
	if (pExtArg->bReverse) {
		sLBA = pDisc->MAIN.nFixEndLBA;
		eLBA = pDisc->MAIN.nFixStartLBA;
	}
	if (sLBA <= nLBA && nLBA <= eLBA) {
		// first sector
		if (nLBA == sLBA) {
			fwrite(lpBuf + pDisc->MAIN.uiMainDataSlideSize, sizeof(BYTE), 
				CD_RAW_SECTOR_SIZE - pDisc->MAIN.uiMainDataSlideSize, fpImg);
			if (!pExtArg->bReverse && pDisc->SUB.lpFirstLBAListOnSub) {
				pDisc->SUB.lpFirstLBAListOnSub[0][0] = -150;
				pDisc->SUB.lpFirstLBAListOnSub[0][1] = nLBA - sLBA;
			}
			if (!pExtArg->bReverse && pDisc->SUB.lpFirstLBAListOnSubSync) {
				pDisc->SUB.lpFirstLBAListOnSubSync[0][0] = -150;
				pDisc->SUB.lpFirstLBAListOnSubSync[0][1] = nLBA - sLBA;
			}
		}
		// last sector in 1st session (when exists session 2)
		else if (pDisc->SCSI.nFirstLBAof2ndSession != -1 &&
			nLBA == pDisc->MAIN.nFixFirstLBAofLeadout - 1) {
			fwrite(lpBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpImg);
		}
		// first sector in 2nd Session
		else if (pDisc->SCSI.nFirstLBAof2ndSession != -1 &&
			nLBA == pDisc->MAIN.nFixFirstLBAof2ndSession) {
			fwrite(lpBuf + pDisc->MAIN.uiMainDataSlideSize, sizeof(BYTE),
				CD_RAW_SECTOR_SIZE - pDisc->MAIN.uiMainDataSlideSize, fpImg);
		}
		// last sector
		else if (nLBA == eLBA) {
			fwrite(lpBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpImg);
		}
		else {
			if (pDisc->SCSI.nFirstLBAof2ndSession != -1 &&
				nLBA == pDisc->SCSI.nFirstLBAof2ndSession) {
				if (pDisc->MAIN.nCombinedOffset > 0) {
					ZeroMemory(lpBuf, (size_t)pDisc->MAIN.nCombinedOffset);
				}
				else if (pDisc->MAIN.nCombinedOffset < 0) {
					// TODO: session2 and minus offset disc
				}
			}
			fwrite(lpBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpImg);
		}
	}
}

VOID WriteC2(
	PDISC pDisc,
	LPBYTE lpBuf,
	INT nLBA,
	FILE* fpC2
	)
{
	INT sLBA = pDisc->MAIN.nFixStartLBA;
	INT eLBA = pDisc->MAIN.nFixEndLBA;
	UINT nC2SlideSize = pDisc->MAIN.uiMainDataSlideSize / 8;
	if (sLBA <= nLBA && nLBA <= eLBA) {
		// first sector
		if (nLBA == sLBA) {
			fwrite(lpBuf + nC2SlideSize, sizeof(BYTE),
				CD_RAW_READ_C2_294_SIZE - nC2SlideSize, fpC2);
		}
		// last sector in 1st session (when exists session 2)
		else if (pDisc->SCSI.nFirstLBAof2ndSession != -1 &&
			nLBA == pDisc->MAIN.nFixFirstLBAofLeadout - 1) {
			fwrite(lpBuf, sizeof(BYTE), nC2SlideSize, fpC2);
		}
		// first sector in 2nd Session
		else if (pDisc->SCSI.nFirstLBAof2ndSession != -1 &&
			nLBA == pDisc->MAIN.nFixFirstLBAof2ndSession) {
			fwrite(lpBuf + nC2SlideSize, sizeof(BYTE),
				CD_RAW_READ_C2_294_SIZE - nC2SlideSize, fpC2);
		}
		// last sector
		else if (nLBA == eLBA) {
			fwrite(lpBuf, sizeof(BYTE), nC2SlideSize, fpC2);
		}
		else {
			if (pDisc->SCSI.nFirstLBAof2ndSession != -1 &&
				nLBA == pDisc->SCSI.nFirstLBAof2ndSession) {
				if (pDisc->MAIN.nCombinedOffset > 0) {
					ZeroMemory(lpBuf, (size_t)pDisc->MAIN.nCombinedOffset);
				}
				else if (pDisc->MAIN.nCombinedOffset < 0) {
					// TODO: session2 and minus offset disc
				}
			}
			fwrite(lpBuf, sizeof(BYTE), CD_RAW_READ_C2_294_SIZE, fpC2);
		}
	}
}

VOID WriteSubChannel(
	PDISC pDisc,
	LPBYTE lpSubcodeRaw,
	LPBYTE lpSubcode,
	INT nLBA,
	BYTE byCurrentTrackNum,
	FILE* fpSub,
	FILE* fpParse
	)
{
	fwrite(lpSubcode, sizeof(BYTE), CD_RAW_READ_SUBCODE_SIZE, fpSub);
	OutputMmcCDSubToLog(pDisc, lpSubcode, lpSubcodeRaw, nLBA, byCurrentTrackNum, fpParse);
}

BOOL WriteParsingSubfile(
	LPCTSTR pszSubfile
	)
{
	BOOL bRet = TRUE;
	FILE* fpParse = CreateOrOpenFileW(
		pszSubfile, _T("_sub"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0);
	if (!fpParse) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}

	LPBYTE data = NULL;
	DISC discData = { 0 };
	FILE* fpSub = NULL;
	DWORD dwTrackAllocSize = MAXIMUM_NUMBER_TRACKS + 1;
	try {
		fpSub = CreateOrOpenFileW(
			pszSubfile, NULL, NULL, NULL, NULL, _T(".sub"), _T("rb"), 0, 0);
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

		// TODO: it doesn't use RtoW in present
		BYTE lpSubcodeRtoW[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		BYTE byTrackNum = 1;

		if (NULL == (discData.SUB.pszISRC =
			(LPSTR*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		for (DWORD h = 0; h < dwTrackAllocSize; h++) {
			if (NULL == (discData.SUB.pszISRC[h] =
				(LPSTR)calloc((META_ISRC_SIZE), sizeof(_TCHAR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
		}
		for (INT i = 0, j = 0; i < (INT)dwFileSize; i += CD_RAW_READ_SUBCODE_SIZE, j++) {
			BYTE byAdr = (BYTE)(*(data + i + 12) & 0x0F);
			if (byAdr == ADR_ENCODES_MEDIA_CATALOG) {
				SetMCNToString(&discData, &data[i],
					discData.SUB.szCatalog, FALSE);
			}
			else if (byAdr == ADR_ENCODES_ISRC) {
				if (0 < byTrackNum && byTrackNum < dwTrackAllocSize) {
					SetISRCToString(&discData, &data[i], 
						discData.SUB.pszISRC[byTrackNum - 1], byTrackNum, FALSE);
				}
			}
			else {
				byTrackNum = BcdToDec(*(data + i + 13));
			}
			if (0 < byTrackNum && byTrackNum < 0xaa) {
				OutputMmcCDSubToLog(&discData, 
					&data[i], lpSubcodeRtoW, j, byTrackNum, fpParse);
			}
			else {
				_TCHAR str[128] = { 0 };
				_sntprintf(str, sizeof(str),
					_T("LBA[%06d, %#07x], Unknown,                               "),
					j, j);
				_ftprintf(fpParse, str);
				_sntprintf(str, sizeof(str),
					_T("Track[%02x], Idx[%02x], RelTime[%02x:%02x:%02x], AbsTime[%02x:%02x:%02x], RtoW[Zero, Zero, Zero, Zero]\n"),
					*(data + i + 13), *(data + i + 14), *(data + i + 15), *(data + i + 16),
					*(data + i + 17), *(data + i + 19), *(data + i + 20), *(data + i + 21));
				_ftprintf(fpParse, str);
			}
			OutputString(_T("\rParse sub(Size) %6d/%6u"), i, dwFileSize);
		}
		OutputString(_T("\n"));
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FcloseAndNull(fpParse);
	FcloseAndNull(fpSub);
	for (DWORD i = 0; i < dwTrackAllocSize; i++) {
		if (discData.SUB.pszISRC) {
			FreeAndNull(discData.SUB.pszISRC[i]);
		}
	}
	FreeAndNull(discData.SUB.pszISRC);
	FreeAndNull(data);
	return bRet;
}

BOOL DescrambleMainChannelForGD(
	LPCTSTR pszPath
	)
{
	BOOL bRet = TRUE;
	FILE* fpScm = CreateOrOpenFileW(
		pszPath, NULL, NULL, NULL, NULL, _T(".scm2"), _T("rb"), 0, 0);
	if (!fpScm) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	FILE* fpImg = NULL;
	FILE* fpTbl = NULL;
	try {
		if (NULL == (fpImg = CreateOrOpenFileW(
			pszPath, NULL, NULL, NULL, NULL, _T(".img2"), _T("wb"), 0, 0))) {
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
			OutputString(_T("\rDescrambling File(LBA) %6u/%6u"), i, dwAllSectorVal);
		}
		OutputString(_T("\n"));
		FcloseAndNull(fpImg);
	}
	catch (BOOL bErr) {
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
	FILE* fpImg = CreateOrOpenFileW(pszPath, NULL,
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
		if (NULL == (fpGdi = CreateOrOpenFileW(pszPath, NULL,
			NULL, NULL, NULL, _T(".gdi"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		DWORD dwFileSize = GetFileSize(0, fpImg);
		if (dwFileSize < 0x110 + 512) {
			OutputErrorString(_T("Not GD-ROM data. Size: %u\n"), dwFileSize);
			throw FALSE;
		}
		fseek(fpImg, 0x110, SEEK_SET);
		// 0x110 - 0x31F is toc data
		BYTE aToc[512] = { 0 };
		fread(aToc, sizeof(BYTE), sizeof(aToc), fpImg);
		if (aToc[0] != 'T' || aToc[1] != 'O' || aToc[2] != 'C' || aToc[3] != '1') {
			OutputErrorString(_T("Not GD-ROM data. Header: %c%c%c%c\n"),
				aToc[0], aToc[1], aToc[2], aToc[3]);
			throw FALSE;
		}

		UINT uiMaxToc = 98 * 4;
		BYTE byMaxTrackNum = aToc[uiMaxToc + 4 * 1 + 2];
		lpToc = (PLONG)calloc(byMaxTrackNum, sizeof(UINT));
		if (!lpToc) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LONG lMaxLBA = 0;
		for (UINT i = 0; i < uiMaxToc; i += 4) {
			if (aToc[7+i] == 0xFF) {
				lMaxLBA = MAKELONG(
					MAKEWORD(aToc[4 + i - 4], aToc[5 + i - 4]), MAKEWORD(aToc[6 + i - 4], 0));
				break;
			}
		}
		_ftprintf(fpGdi, _T("%u\n"), byMaxTrackNum);
		if (byMaxTrackNum <= 9 && lMaxLBA <= 99999) {
			_ftprintf(fpGdi,
				_T("1 %5d 4 2352 \"%s (Track %u).bin\" 0\n")
				_T("2 [fix] 0 2352 \"%s (Track %u).bin\" 0\n"),
				0, pszPathWithoutPathAndExt, 1,
				pszPathWithoutPathAndExt, 2);
		}
		else if (10 <= byMaxTrackNum && lMaxLBA <= 99999) {
			_ftprintf(fpGdi,
				_T(" 1 %5d 4 2352 \"%s (Track %02u).bin\" 0\n")
				_T(" 2 [fix] 0 2352 \"%s (Track %02u).bin\" 0\n"),
				0, pszPathWithoutPathAndExt, 1,
				pszPathWithoutPathAndExt, 2);
		}
		else if (byMaxTrackNum <= 9 && 100000 <= lMaxLBA) {
			_ftprintf(fpGdi,
				_T("1 %6d 4 2352 \"%s (Track %u).bin\" 0\n")
				_T("2  [fix] 0 2352 \"%s (Track %u).bin\" 0\n"),
				0, pszPathWithoutPathAndExt, 1,
				pszPathWithoutPathAndExt, 2);
		}
		else if (10 <= byMaxTrackNum && 100000 <= lMaxLBA) {
			_ftprintf(fpGdi,
				_T(" 1 %6d 4 2352 \"%s (Track %02u).bin\" 0\n")
				_T(" 2  [fix] 0 2352 \"%s (Track %02u).bin\" 0\n"),
				0, pszPathWithoutPathAndExt, 1,
				pszPathWithoutPathAndExt, 2);
		}

		BYTE byTrackNum = 3;
		for (UINT i = 0; i < uiMaxToc; i += 4, byTrackNum++) {
			if (aToc[7 + i] == 0xFF) {
				break;
			}
			BYTE byCtl = (BYTE)((aToc[7 + i] >> 4) & 0x0F);
			LONG lToc =
				MAKELONG(MAKEWORD(aToc[4 + i], aToc[5 + i]), MAKEWORD(aToc[6 + i], 0));
			lpToc[byTrackNum - 3] = lToc - 300;
			if (byTrackNum == 3) {
				lpToc[byTrackNum - 3] += 150;
				OutputDiscLogA(
					"Track %2u, Ctl %u,              , Index1 %6d\n", 
					byTrackNum, byCtl, lpToc[byTrackNum - 3]);
			}
			else {
				if ((byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					lpToc[byTrackNum - 3] -= 75;
					OutputDiscLogA(
						"Track %2u, Ctl %u, Index0 %6d, Index1 %6d\n", 
						byTrackNum, byCtl, lpToc[byTrackNum - 3], lpToc[byTrackNum - 3] + 225);
				}
				else {
					OutputDiscLogA(
						"Track %2u, Ctl %u, Index0 %6d, Index1 %6d\n", 
						byTrackNum, byCtl, lpToc[byTrackNum - 3], lpToc[byTrackNum - 3] + 150);
				}
			}
			if (byMaxTrackNum <= 9 && lMaxLBA <= 99999) {
				_ftprintf(fpGdi, _T("%u %5d %u 2352 \"%s (Track %u).bin\" 0\n"),
					byTrackNum, lpToc[byTrackNum - 3], byCtl, pszPathWithoutPathAndExt, byTrackNum);
			}
			else if (10 <= byMaxTrackNum && lMaxLBA <= 99999) {
				_ftprintf(fpGdi, _T("%2u %5d %u 2352 \"%s (Track %02u).bin\" 0\n"),
					byTrackNum, lpToc[byTrackNum - 3], byCtl, pszPathWithoutPathAndExt, byTrackNum);
			}
			else if (byMaxTrackNum <= 9 && 100000 <= lMaxLBA) {
				_ftprintf(fpGdi, _T("%u %6d %u 2352 \"%s (Track %u).bin\" 0\n"),
					byTrackNum, lpToc[byTrackNum - 3], byCtl, pszPathWithoutPathAndExt, byTrackNum);
			}
			else if (10 <= byMaxTrackNum && 100000 <= lMaxLBA) {
				_ftprintf(fpGdi, _T("%2u %6d %u 2352 \"%s (Track %02u).bin\" 0\n"),
					byTrackNum, lpToc[byTrackNum - 3], byCtl, pszPathWithoutPathAndExt, byTrackNum);
			}
		}
		LONG lToc = 
			MAKELONG(MAKEWORD(aToc[uiMaxToc + 4 * 2], aToc[uiMaxToc + 4 * 2 + 1]),
			MAKEWORD(aToc[uiMaxToc + 4 * 2 + 2], 0)) - 150;
		OutputDiscLogA(
			"MaxTrackNum %2u\n"
			"Leadout, LBA[%06d, %#07x]\n",
			byMaxTrackNum, lToc, lToc);
		lpToc[byTrackNum - 3] = lToc;

		rewind(fpImg);
		for (BYTE i = 3; i <= byMaxTrackNum; i++) {
			OutputString(_T("\rSplit File(num) %2u/%2u"), i, byMaxTrackNum);
			if (NULL == (fpBin = CreateOrOpenFileW(pszPath, NULL, NULL,
				NULL, NULL, _T(".bin"), _T("wb"), i, byMaxTrackNum))) {
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
	catch (BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpImg);
	FcloseAndNull(fpBin);
	FreeAndNull(lpToc);
	return bRet;
}

VOID DescrambleMainChannel(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	FILE* fpTbl,
	FILE* fpImg
	)
{
	BYTE aScrambledBuf[CD_RAW_SECTOR_SIZE] = { 0 };
	fread(aScrambledBuf, sizeof(BYTE), sizeof(aScrambledBuf), fpTbl);
	BYTE aSrcBuf[CD_RAW_SECTOR_SIZE] = { 0 };
	LONG lSeekPtr = 0;

	for (INT k = pDisc->SCSI.byFirstDataTrack - 1; k < pDisc->SCSI.byLastDataTrack; k++) {
		if (pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[k] != -1) {
			OutputDiscLogA("\tData Sector, LBA %6d-%6d (%#07x-%#07x)\n", 
				pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[k], 
				pDisc->SUB.lpLastLBAListOfDataTrackOnSub[k],
				pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[k], 
				pDisc->SUB.lpLastLBAListOfDataTrackOnSub[k]);
			INT nFirstLBA = pDisc->SCSI.lpSessionNumList[k] >= 2 
				? pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[k] - (11400 * (INT)(pDisc->SCSI.lpSessionNumList[k] - 1))
				: pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[k];
			INT nLastLBA = pDisc->SCSI.lpSessionNumList[k] >= 2 
				? pDisc->SUB.lpLastLBAListOfDataTrackOnSub[k] - (11400 * (INT)(pDisc->SCSI.lpSessionNumList[k] - 1))
				: pDisc->SUB.lpLastLBAListOfDataTrackOnSub[k];
			if (pDisc->SUB.bIndex0InTrack1) {
				nFirstLBA += 150;
				nLastLBA += 150;
			}
			if (!pExtArg->bReverse) {
				lSeekPtr = nFirstLBA;
			}
			for (; nFirstLBA <= nLastLBA; nFirstLBA++, lSeekPtr++) {
				// ファイルを読み書き両用モードで開いている時は 注意が必要です。
				// 読み込みを行った後に書き込みを行う場合やその逆を行う場合は、 
				// 必ずfseekを呼ばなければなりません。もしこれを忘れると、
				// 場合によってはバッファー内と 実際にディスクに描き込まれた
				// データに矛盾が生じ、正確に書き込まれない場合や、
				// 嘘の データを読み込む場合があります。
				fseek(fpImg, lSeekPtr * CD_RAW_SECTOR_SIZE, SEEK_SET);
				fread(aSrcBuf, sizeof(BYTE), sizeof(aSrcBuf), fpImg);
				if (IsValidDataHeader(aSrcBuf)) {
					fseek(fpImg, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
					for (INT n = 0; n < CD_RAW_SECTOR_SIZE; n++) {
						aSrcBuf[n] ^= aScrambledBuf[n];
					}
					fwrite(aSrcBuf, sizeof(BYTE), sizeof(aSrcBuf), fpImg);
				}
				OutputString(
					_T("\rDescrambling data sector of img(LBA) %6d/%6d"), nFirstLBA, nLastLBA);
			}
			OutputString(_T("\n"));
		}
	}
}

BOOL CreateBin(
	PDISC pDisc,
	BYTE i,
	INT nLBA,
	INT nPrevLBA,
	FILE* fpImg,
	FILE* fpBin
	)
{
	size_t uiBufSize = 0;
	INT nWriteSectorSize = CD_RAW_SECTOR_SIZE;

	if (pDisc->SCSI.toc.LastTrack == pDisc->SCSI.toc.FirstTrack) {
		uiBufSize = (size_t)pDisc->SCSI.nAllLength * nWriteSectorSize;
		nPrevLBA = 0;
	}
	else if (i == pDisc->SCSI.toc.FirstTrack) {
		uiBufSize = (size_t)nLBA * nWriteSectorSize;
		nPrevLBA = 0;
	}
	else if (i == pDisc->SCSI.toc.LastTrack) {
		INT nTmpLength = pDisc->SCSI.nAllLength;
		if (pDisc->SCSI.lpSessionNumList[i - 1] > 1) {
			UINT nLeadinoutSize =
				11400 * (UINT)(pDisc->SCSI.lpSessionNumList[i - 1] - 1);
			nPrevLBA -= nLeadinoutSize;
			nTmpLength -= nLeadinoutSize;
		}
		uiBufSize = (size_t)(nTmpLength - nPrevLBA) * nWriteSectorSize;
	}
	else {
		if (i == pDisc->SCSI.toc.LastTrack - (UINT)1 &&
			pDisc->SCSI.lpSessionNumList[i] > 1) {
			nLBA -= 11400 * (pDisc->SCSI.lpSessionNumList[i] - 1);
		}
		uiBufSize = (size_t)(nLBA - nPrevLBA) * nWriteSectorSize;
	}
	fseek(fpImg, nPrevLBA * nWriteSectorSize, SEEK_SET);
	LPBYTE lpBuf = (LPBYTE)calloc(uiBufSize, sizeof(BYTE));
	if (!lpBuf) {
		OutputString(_T("\n"));
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	fread(lpBuf, sizeof(BYTE), uiBufSize, fpImg);
	fwrite(lpBuf, sizeof(BYTE), uiBufSize, fpBin);
	FreeAndNull(lpBuf);

	return TRUE;
}

BOOL CreateBinCueCcd(
	PDISC pDisc,
	LPCTSTR pszPath,
	LPCTSTR pszImgName,
	BOOL lpCatalog,
	FILE* fpImg,
	FILE* fpCue,
	FILE* fpCueForImg,
	FILE* fpCcd
	)
{
	WriteCueForFirst(pDisc, lpCatalog, fpCueForImg);
	WriteCueForFileDirective(pszImgName, fpCueForImg);
	WriteCueForFirst(pDisc, lpCatalog, fpCue);

	FILE* fpCueSyncForImg = NULL;
	FILE* fpCueSync = NULL;
	if (pDisc->SUB.bDesync) {
		if (NULL == (fpCueSyncForImg = CreateOrOpenFileW(
			pszPath, _T(" (Subs indexes)_img"), NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		if (NULL == (fpCueSync = CreateOrOpenFileW(
			pszPath, _T(" (Subs indexes)"), NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		WriteCueForFirst(pDisc, lpCatalog, fpCueSyncForImg);
		WriteCueForFileDirective(pszImgName, fpCueSyncForImg);
		WriteCueForFirst(pDisc, lpCatalog, fpCueSync);
	}

	BOOL bRet = TRUE;
	_TCHAR pszPathWithoutPath[_MAX_FNAME] = { 0 };
	for (BYTE i = pDisc->SCSI.toc.FirstTrack; i <= pDisc->SCSI.toc.LastTrack; i++) {
		FILE* fpBin = CreateOrOpenFileW(pszPath, NULL, NULL, pszPathWithoutPath,
			NULL, _T(".bin"), _T("wb"), i, pDisc->SCSI.toc.LastTrack);
		if (!fpBin) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			bRet = FALSE;
			break;
		}
		WriteCueForUnderFileDirective(pDisc, i, fpCueForImg);
		WriteCueForFileDirective(pszPathWithoutPath, fpCue);
		WriteCueForUnderFileDirective(pDisc, i, fpCue);
		WriteCcdFileForTrack(pDisc, i, fpCcd);

		FILE* fpBinSync = NULL;
		_TCHAR pszPathWithoutPathSync[_MAX_FNAME] = { 0 };
		if (pDisc->SUB.bDesync) {
			fpBinSync = CreateOrOpenFileW(pszPath, _T(" (Subs indexes)"), NULL, 
				pszPathWithoutPathSync, NULL, _T(".bin"), _T("wb"), i, pDisc->SCSI.toc.LastTrack);
			if (!fpBinSync) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				bRet = FALSE;
				break;
			}
			WriteCueForUnderFileDirective(pDisc, i, fpCueSyncForImg);
			WriteCueForFileDirective(pszPathWithoutPathSync, fpCueSync);
			WriteCueForUnderFileDirective(pDisc, i, fpCueSync);
		}
		OutputString(
			_T("\rCreating bin, cue and ccd (Track) %2u/%2u"), i, pDisc->SCSI.toc.LastTrack);

		BYTE index = 0;
		INT nLBAofFirstIdx = pDisc->SUB.lpFirstLBAListOnSub[i - 1][0];
		// nothing or index 0 in track 1
		if (nLBAofFirstIdx == -1 || nLBAofFirstIdx == -150) {
			nLBAofFirstIdx = pDisc->SUB.lpFirstLBAListOnSub[i - 1][1];
			index++;
		}
		INT nLBAofFirstIdxSync = pDisc->SUB.lpFirstLBAListOnSubSync[i - 1][0];
		if (nLBAofFirstIdxSync == -1 || nLBAofFirstIdxSync == -150) {
			nLBAofFirstIdxSync = pDisc->SUB.lpFirstLBAListOnSubSync[i - 1][1];
		}

		BYTE byFrame = 0, bySecond = 0, byMinute = 0;
		if (i == pDisc->SCSI.toc.FirstTrack) { 
			if (0 == nLBAofFirstIdx ||
				i == pDisc->SCSI.toc.LastTrack) {
				WriteCueForIndexDirective(index, 0, 0, 0, fpCueForImg);
				WriteCueForIndexDirective(index, 0, 0, 0, fpCue);
				WriteCcdFileForTrackIndex(index, 0, fpCcd);
				if (pDisc->SUB.bDesync) {
					WriteCueForIndexDirective(index, 0, 0, 0, fpCueSyncForImg);
					WriteCueForIndexDirective(index, 0, 0, 0, fpCueSync);
				}
			}
			else if (0 < nLBAofFirstIdx) {
				// index 0 in track 1
				//  Crow, The - Original Motion Picture Soundtrack (82519-2)
				//  Now on Never (Nick Carter) (ZJCI-10118)
				//  SaGa Frontier Original Sound Track (Disc 3)
				//  etc..
				WriteCueForIndexDirective(0, 0, 0, 0, fpCueForImg);
				WriteCueForIndexDirective(0, 0, 0, 0, fpCue);
				WriteCcdFileForTrackIndex(0, 0, fpCcd);

				LBAtoMSF(nLBAofFirstIdx, &byMinute, &bySecond, &byFrame);
				WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCueForImg);
				WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCue);
				WriteCcdFileForTrackIndex(index, nLBAofFirstIdx, fpCcd);
				if (pDisc->SUB.bDesync) {
					WriteCueForIndexDirective(0, 0, 0, 0, fpCueSyncForImg);
					WriteCueForIndexDirective(0, 0, 0, 0, fpCueSync);
					WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCueSyncForImg);
					WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCueSync);
				}
			}
			index++;
		}

		for (; index < MAXIMUM_NUMBER_INDEXES; index++) {
			INT nLBAofNextIdx = pDisc->SUB.lpFirstLBAListOnSub[i - 1][index];
			if (nLBAofNextIdx != -1) {
				LBAtoMSF(nLBAofNextIdx,	&byMinute, &bySecond, &byFrame);
				WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCueForImg);

				LBAtoMSF(nLBAofNextIdx - nLBAofFirstIdx, &byMinute, &bySecond, &byFrame);
				WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCue);

				WriteCcdFileForTrackIndex(index, nLBAofNextIdx, fpCcd);
			}
			else {
				if (index >= 2) {
					break;
				}
			}
			if (pDisc->SUB.bDesync) {
				INT nLBAofNextIdxSync = pDisc->SUB.lpFirstLBAListOnSubSync[i - 1][index];
				if (nLBAofNextIdxSync != -1) {
					LBAtoMSF(nLBAofNextIdxSync,	&byMinute, &bySecond, &byFrame);
					WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCueSyncForImg);

					LBAtoMSF(nLBAofNextIdxSync - nLBAofFirstIdxSync, &byMinute, &bySecond, &byFrame);
					WriteCueForIndexDirective(index, byMinute, bySecond, byFrame, fpCueSync);
				}
			}
		}
		// write each track
		INT nLBA = pDisc->SUB.lpFirstLBAListOnSub[i - 1][0] == -1 ?
			pDisc->SUB.lpFirstLBAListOnSub[i - 1][1] : 
			pDisc->SUB.lpFirstLBAListOnSub[i - 1][0];
		if (pDisc->SUB.bIndex0InTrack1) {
			if (i == pDisc->SCSI.toc.FirstTrack) {
				nLBA += 150 - abs(pDisc->MAIN.nAdjustSectorNum);
			}
			else {
				nLBA += 150;
			}
			if (i == pDisc->SCSI.toc.LastTrack) {
				pDisc->SCSI.nAllLength += 150;
			}
		}
		INT nNextLBA = pDisc->SUB.lpFirstLBAListOnSub[i][0] == -1 ?
			pDisc->SUB.lpFirstLBAListOnSub[i][1] : 
			pDisc->SUB.lpFirstLBAListOnSub[i][0];
		if (pDisc->SUB.bIndex0InTrack1) {
			nNextLBA += 150;
		}
		bRet = CreateBin(pDisc, i, nNextLBA, nLBA, fpImg, fpBin);
		FcloseAndNull(fpBin);
		if (!bRet) {
			break;
		}
		if (pDisc->SUB.bDesync) {
			nLBA = pDisc->SUB.lpFirstLBAListOnSubSync[i - 1][0] == -1 ?
				pDisc->SUB.lpFirstLBAListOnSubSync[i - 1][1] : 
				pDisc->SUB.lpFirstLBAListOnSubSync[i - 1][0];
			if (pDisc->SUB.bIndex0InTrack1) {
				if (i == pDisc->SCSI.toc.FirstTrack) {
					nLBA += 150 - abs(pDisc->MAIN.nAdjustSectorNum);
				}
				else {
					nLBA += 150;
				}
				if (i == pDisc->SCSI.toc.LastTrack) {
					pDisc->SCSI.nAllLength += 150;
				}
			}
			nNextLBA = pDisc->SUB.lpFirstLBAListOnSubSync[i][0] == -1 ?
				pDisc->SUB.lpFirstLBAListOnSubSync[i][1] :
				pDisc->SUB.lpFirstLBAListOnSubSync[i][0];
			if (pDisc->SUB.bIndex0InTrack1) {
				nNextLBA += 150;
			}
			bRet = CreateBin(pDisc, i, nNextLBA, nLBA, fpImg, fpBinSync);
			FcloseAndNull(fpBinSync);
			if (!bRet) {
				break;
			}
		}
	}
	FcloseAndNull(fpCueSyncForImg);
	FcloseAndNull(fpCueSync);
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

	OutputErrorString(_T("[F:%s][L:%d] GetLastError: %u, %s\n"), 
		pszFuncName, lLineNum, GetLastError(), (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

BOOL OutputWindowsVer(
	)
{
	OSVERSIONINFOEX OSver;
	OSver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (!GetVersionEx((LPOSVERSIONINFO)&OSver)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	OutputString(_T("OS\n"));
	switch (OSver.dwMajorVersion) {
	case 5:
		switch (OSver.dwMinorVersion) {
		case 0:
			OutputString(_T("\tWindows 2000 "));
			switch (OSver.wProductType) {
			case VER_NT_WORKSTATION:
				OutputString(_T("Professional"));
				break;
			case VER_NT_DOMAIN_CONTROLLER:
			case VER_NT_SERVER:
				if (OSver.wSuiteMask & VER_SUITE_ENTERPRISE) {
					OutputString(_T("Advanced Server"));
				}
				else if (OSver.wSuiteMask & VER_SUITE_DATACENTER) {
					OutputString(_T("Datacenter Server"));
				}
				else {
					OutputString(_T("Server"));
				}
				break;
			}
			break;
		case 1:
			OutputString(_T("\tWindows XP "));
			switch (OSver.wProductType) {
			case VER_NT_WORKSTATION:
				if (OSver.wSuiteMask & VER_SUITE_PERSONAL) {
					OutputString(_T("Home Edition"));
				}
				else {
					OutputString(_T("Professional"));
				}
				break;
			case VER_NT_DOMAIN_CONTROLLER:
			case VER_NT_SERVER:
				break;
			}
			break;
		case 2:
			OutputString(_T("\tWindows Server 2003 "));
			switch (OSver.wProductType) {
			case VER_NT_WORKSTATION:
				break;
			case VER_NT_DOMAIN_CONTROLLER:
			case VER_NT_SERVER:
				if (OSver.wSuiteMask & VER_SUITE_SMALLBUSINESS) {
					OutputString(_T("Small Business Server"));
				}
				else if (OSver.wSuiteMask & VER_SUITE_ENTERPRISE) {
					OutputString(_T("Enterprise Edition"));
				}
				else if (OSver.wSuiteMask & VER_SUITE_SMALLBUSINESS_RESTRICTED) {
					OutputString(_T("Small Business Server with the restrictive client license"));
				}
				else if (OSver.wSuiteMask & VER_SUITE_DATACENTER) {
					OutputString(_T("Datacenter Edition"));
				}
				else if (OSver.wSuiteMask & VER_SUITE_BLADE) {
					OutputString(_T("Web Edition"));
				}
				else if (OSver.wSuiteMask & VER_SUITE_STORAGE_SERVER) {
					OutputString(_T("Storage Server Edition"));
				}
				else if (OSver.wSuiteMask & VER_SUITE_COMPUTE_SERVER) {
					OutputString(_T("Compute Cluster Edition"));
				}
				else {
					OutputString(_T("Other"));
				}
				break;
			}
			break;
		}
		break;
	case 6:
		BOOL(CALLBACK* pfnGetProductInfo)
			(DWORD dwOSMajorVersion, DWORD dwOSMinorVersion, 
			DWORD dwSpMajorVersion, DWORD dwSpMinorVersion, PDWORD pdwReturnedProductType);
		HMODULE	hModule = ::LoadLibrary(_T("kernel32.dll"));
		if (!hModule) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		(*(FARPROC*)&pfnGetProductInfo) = ::GetProcAddress(hModule, "GetProductInfo");
		if (!pfnGetProductInfo) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		DWORD dwProductType = PRODUCT_UNDEFINED;
		pfnGetProductInfo(OSver.dwMajorVersion, OSver.dwMinorVersion, 
			OSver.wServicePackMajor, OSver.wServicePackMinor, &dwProductType);
		if (!::FreeLibrary(hModule)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}

		switch (OSver.dwMinorVersion) {
		case 0:
			switch (OSver.wProductType) {
			case VER_NT_WORKSTATION:
				OutputString(_T("\tWindows Vista "));
				switch (dwProductType) {
				case PRODUCT_ULTIMATE:
					OutputString(_T("Ultimate"));
					break;
				case PRODUCT_HOME_BASIC:
					OutputString(_T("Home Basic"));
					break;
				case PRODUCT_HOME_PREMIUM:
					OutputString(_T("Home Premium"));
					break;
				case PRODUCT_ENTERPRISE:
					OutputString(_T("Enterprise"));
					break;
				case PRODUCT_HOME_BASIC_N:
					OutputString(_T("Home Basic N"));
					break;
				case PRODUCT_BUSINESS:
					OutputString(_T("Business"));
					break;
				case PRODUCT_STARTER:
					OutputString(_T("Starter"));
					break;
				case PRODUCT_BUSINESS_N:
					OutputString(_T("Business N"));
					break;
				default:
					OutputString(_T("Other"));
					break;
				}
				break;
			case VER_NT_DOMAIN_CONTROLLER:
			case VER_NT_SERVER:
				if (OSver.wSuiteMask & VER_SUITE_ENTERPRISE) {
					OutputString(_T("Windows Server 2008 Enterprise"));
				}
				else if (OSver.wSuiteMask & VER_SUITE_SMALLBUSINESS) {
					OutputString(_T("Windows Small Business Server 2008"));
				}
				else if (OSver.wSuiteMask & VER_SUITE_DATACENTER) {
					OutputString(_T("Windows Server 2008 Datacenter"));
				}
				else if (OSver.wSuiteMask & VER_SUITE_BLADE) {
					OutputString(_T("Windows Web Server 2008"));
				}
				else if (OSver.wSuiteMask & VER_SUITE_STORAGE_SERVER) {
					OutputString(_T("Windows Storage Server 2008"));
				}
				else {
					OutputString(_T("Other"));
				}
				break;
			}
			break;
		case 1:
			switch (OSver.wProductType) {
			case VER_NT_WORKSTATION:
				OutputString(_T("\tWindows 7 "));
				switch (dwProductType) {
				case PRODUCT_ULTIMATE:
					OutputString(_T("Ultimate"));
					break;
				case PRODUCT_HOME_BASIC:
					OutputString(_T("Home Basic"));
					break;
				case PRODUCT_HOME_PREMIUM:
					OutputString(_T("Home Premium"));
					break;
				case PRODUCT_ENTERPRISE:
					OutputString(_T("Enterprise"));
					break;
				case PRODUCT_STARTER:
					OutputString(_T("Starter"));
					break;
				case PRODUCT_ULTIMATE_N:
					OutputString(_T("Ultimate N"));
					break;
				case PRODUCT_HOME_PREMIUM_N:
					OutputString(_T("Home Premium N"));
					break;
				case PRODUCT_ENTERPRISE_N:
					OutputString(_T("Enterprise N"));
					break;
				case PRODUCT_PROFESSIONAL:
					OutputString(_T("Professional"));
					break;
				case PRODUCT_PROFESSIONAL_N:
					OutputString(_T("Professional N"));
					break;
				default:
					OutputString(_T("Other"));
					break;
				}
				break;
			case VER_NT_DOMAIN_CONTROLLER:
			case VER_NT_SERVER:
				if (OSver.wSuiteMask & VER_SUITE_ENTERPRISE) {
					OutputString(_T("Windows Server 2008 R2 Enterprise"));
				}
				else if (OSver.wSuiteMask & VER_SUITE_DATACENTER) {
					OutputString(_T("Windows Server 2008 R2 Datacenter"));
				}
				else if (OSver.wSuiteMask & VER_SUITE_BLADE) {
					OutputString(_T("Windows Web Server 2008 R2"));
				}
				else {
					OutputString(_T("Windows Server 2008"));
				}
				break;
			}
			break;
		case 2:
			switch (OSver.wProductType) {
			case VER_NT_WORKSTATION:
				OutputString(_T("\tWindows 8 "));
				switch (dwProductType) {
				case PRODUCT_ENTERPRISE:
					OutputString(_T("Enterprise"));
					break;
				case PRODUCT_PROFESSIONAL:
					OutputString(_T("Pro"));
					break;
				default:
					OutputString(_T("Other"));
					break;
				}
				break;
			case VER_NT_DOMAIN_CONTROLLER:
			case VER_NT_SERVER:
				if (OSver.wSuiteMask & VER_SUITE_DATACENTER) {
					OutputString(_T("Windows Server 2012 Datacenter"));
				}
				else {
					OutputString(_T("Windows Server 2012"));
				}
				break;
			}
			break;
		case 3:
			switch (OSver.wProductType) {
			case VER_NT_WORKSTATION:
				OutputString(_T("\tWindows 8.1 "));
				switch (dwProductType) {
				case PRODUCT_ENTERPRISE:
					OutputString(_T("Enterprise"));
					break;
				case PRODUCT_PROFESSIONAL:
					OutputString(_T("Pro"));
					break;
				default:
					OutputString(_T("Other"));
					break;
				}
				break;
			case VER_NT_DOMAIN_CONTROLLER:
			case VER_NT_SERVER:
				if (OSver.wSuiteMask & VER_SUITE_DATACENTER) {
					OutputString(_T("Windows Server 2012 R2 Datacenter"));
				}
				else {
					OutputString(_T("Windows Server 2012 R2"));
				}
				break;
			}
			break;
		}
		break;
	}
	OutputString(_T(" %s "), OSver.szCSDVersion);
	BOOL b64BitOS = TRUE;
#ifndef _WIN64
	if (!::IsWow64Process(GetCurrentProcess(), &b64BitOS)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
#endif
	INT bit = 32;
	if (b64BitOS) {
		bit = 64;
	}
	OutputString(_T("%dbit\n"), bit);
#ifdef _DEBUG
	OutputString(
		_T("\tMajorVersion: %u, MinorVersion: %u, BuildNumber: %u, PlatformId: %u\n")
		_T("\tServicePackMajor: %u, ServicePackMinor: %u, SuiteMask: %u, ProductType: %u\n"),
		OSver.dwMajorVersion, OSver.dwMinorVersion, OSver.dwBuildNumber, OSver.dwPlatformId,
		OSver.wServicePackMajor, OSver.wServicePackMinor, OSver.wSuiteMask, OSver.wProductType);
#endif
	return TRUE;
}

VOID OutputEepromOverPX708(
	LPBYTE pBuf,
	PDWORD idx
	)
{
	OutputDriveLogA("\t    Silent Mode: ");
	if (pBuf[*idx] == 1) {
		OutputDriveLogA(
			"Enabled\n"
			"\t\t       Access Time: ");
		if (pBuf[*idx + 1] == 0) {
			OutputDriveLogA("Fast\n");
		}
		else if (pBuf[*idx + 1] == 2) {
			OutputDriveLogA("Slow\n");
		}
		OutputDriveLogA(
			"\t\t    Max Read Speed: %dx\n"
			"\t\t           Unknown: %02x\n"
			"\t\t   Max Write Speed: %dx\n"
			"\t\t           Unknown: %02x\n"
			"\t\t           Unknown: %02x\n"
			"\t\t  Tray Speed Eject: %02x (Low d0 - 80 High)\n"
			"\t\tTray Speed Loading: %02x (Low 2f - 7f High)\n",
			pBuf[*idx + 2], pBuf[*idx + 3], pBuf[*idx + 4], 
			pBuf[*idx + 5], pBuf[*idx + 6], pBuf[*idx + 7], pBuf[*idx + 8]);
	}
	else {
		OutputDriveLogA("Disable\n");
	}
	*idx += 9;
	DWORD tmp = *idx;
	OutputDriveLogA("\t        SecuRec: ");
	while (*idx < tmp + 20) {
		OutputDriveLogA("%02x ", pBuf[*idx]);
		*idx += 1;
	}
	OutputDriveLogA(
		"\n\t        Unknown: %x"
		"\n\t      SpeedRead: "
		, pBuf[*idx] >> 4 & 0x0f);
	INT sp = pBuf[*idx] & 0x0f;
	if (sp == 0) {
		OutputDriveLogA("Enable");
	}
	else if (sp == 0xf) {
		OutputDriveLogA("Disable");
	}
	OutputDriveLogA(
		"\n\t        Unknown: %x"
		"\n\t  Spindown Time: ",
		pBuf[*idx + 1]);
	switch (pBuf[*idx + 2]) {
	case 0:
		OutputDriveLogA("Infinite\n");
		break;
	case 1:
		OutputDriveLogA("125 ms\n");
		break;
	case 2:
		OutputDriveLogA("250 ms\n");
		break;
	case 3:
		OutputDriveLogA("500 ms\n");
		break;
	case 4:
		OutputDriveLogA("1 second\n");
		break;
	case 5:
		OutputDriveLogA("2 seconds\n");
		break;
	case 6:
		OutputDriveLogA("4 seconds\n");
		break;
	case 7:
		OutputDriveLogA("8 seconds\n");
		break;
	case 8:
		OutputDriveLogA("16 seconds\n");
		break;
	case 9:
		OutputDriveLogA("32 seconds\n");
		break;
	case 10:
		OutputDriveLogA("1 minite\n");
		break;
	case 11:
		OutputDriveLogA("2 minites\n");
		break;
	case 12:
		OutputDriveLogA("4 minites\n");
		break;
	case 13:
		OutputDriveLogA("8 minites\n");
		break;
	case 14:
		OutputDriveLogA("16 minites\n");
		break;
	case 15:
		OutputDriveLogA("32 minites\n");
		break;
	default:
		OutputDriveLogA("Unset\n");
		break;
	}
	*idx += 3;
	LONG ucr = 
		MAKELONG(MAKEWORD(pBuf[*idx + 5], pBuf[*idx + 4]), MAKEWORD(pBuf[*idx + 3], pBuf[*idx + 2]));
	LONG ucw = 
		MAKELONG(MAKEWORD(pBuf[*idx + 9], pBuf[*idx + 8]), MAKEWORD(pBuf[*idx + 7], pBuf[*idx + 6]));
	LONG udr = 
		MAKELONG(MAKEWORD(pBuf[*idx + 13], pBuf[*idx + 12]), MAKEWORD(pBuf[*idx + 11], pBuf[*idx + 10]));
	LONG udw = 
		MAKELONG(MAKEWORD(pBuf[*idx + 17], pBuf[*idx + 16]), MAKEWORD(pBuf[*idx + 15], pBuf[*idx + 14]));
	OutputDriveLogA(
		"\tDisc load count: %u\n"
		"\t   CD read time: %02u:%02u:%02u\n"
		"\t  CD write time: %02u:%02u:%02u\n"
		"\t  DVD read time: %02u:%02u:%02u\n"
		"\t DVD write time: %02u:%02u:%02u\n"
		, MAKEWORD(pBuf[*idx + 1], pBuf[*idx])
		, ucr / 3600, ucr / 60 % 60, ucr % 60
		, ucw / 3600, ucw / 60 % 60, ucw % 60
		, udr / 3600, udr / 60 % 60, udr % 60
		, udw / 3600, udw / 60 % 60, udw % 60);
	*idx += 18;
}

VOID OutputEeprom(
	LPBYTE pBuf,
	DWORD tLen,
	INT nRoop,
	INT nLife
	)
{
	DWORD idx = 0;
	if (nRoop == 0) {
		OutputDriveLogA("\t        Unknown: ");
		while (idx < 2) {
			OutputDriveLogA("%02x ", pBuf[idx]);
			idx++;
		}
		OutputDriveLogA("\n\t         Vendor: ");
		while (idx < 10) {
			OutputDriveLogA("%c", pBuf[idx]);
			idx++;
		}
		OutputDriveLogA("\n\t        Product: ");
		while (idx < 26) {
			OutputDriveLogA("%c", pBuf[idx]);
			idx++;
		}
		OutputDriveLogA("\n\t  Serial number: %u\n", strtoul((PCHAR)&pBuf[26], NULL, 16));
		idx += 5;
		OutputDriveLogA("\t        Unknown: ");
		while (idx < 41) {
			OutputDriveLogA("%02x ", pBuf[idx]);
			idx++;
		}
		if (nLife >= 1) {
			OutputDriveLogA("\n\t            TLA: ");
			while (idx < 45) {
				OutputDriveLogA("%c", pBuf[idx]);
				idx++;
			}
		}
		else {
			OutputDriveLogA("\n\t        Unknown: ");
			while (idx < 45) {
				OutputDriveLogA("%02x ", pBuf[idx]);
				idx++;
			}
		}
		OutputDriveLogA("\n\t        Unknown: ");
		while (idx < 108) {
			OutputDriveLogA("%02x ", pBuf[idx]);
			idx++;
		}
		OutputDriveLogA("\n");
		if (nLife == 0) {
			LONG ucr = MAKELONG(MAKEWORD(pBuf[111], pBuf[110]), MAKEWORD(pBuf[109], pBuf[108]));
			LONG ucw = MAKELONG(MAKEWORD(pBuf[125], pBuf[124]), MAKEWORD(pBuf[123], pBuf[122]));
			OutputDriveLogA(
				"\t   CD read time: %02u:%02u:%02u\n"
				"\t        Unknown: %02x %02x %02x %02x %02x %02x %02x %02x\n"
				"\tDisc load count: %u\n"
				"\t  CD write time: %02u:%02u:%02u\n"
				, ucr / 3600, ucr / 60 % 60, ucr % 60,
				pBuf[112], pBuf[113], pBuf[114], pBuf[115], pBuf[116], pBuf[117], pBuf[118], pBuf[119]
				, MAKEWORD(pBuf[121], pBuf[120])
				, ucw / 3600, ucw / 60 % 60, ucw % 60);
			idx += 18;
		}
		else if (nLife == 1) {
			OutputDriveLogA("\t        Unknown: ");
			while (idx < 256) {
				OutputDriveLogA("%02x ", pBuf[idx]);
				idx++;
			}
			OutputDriveLogA("\n");
			OutputEepromOverPX708(pBuf, &idx);
		}
		OutputDriveLogA("\t        Unknown: ");
		while (idx < tLen) {
			OutputDriveLogA("%02x ", pBuf[idx]);
			idx++;
		}
		OutputDriveLogA("\n");
	}
	else if (nRoop == 1 && nLife == 2) {
		OutputEepromOverPX708(pBuf, &idx);
		OutputDriveLogA("\t        Unknown: ");
		while (idx < 256) {
			OutputDriveLogA("%02x ", pBuf[idx]);
			idx++;
		}
		OutputDriveLogA("\n");
	}
	else {
		OutputDriveLogA("\t        Unknown: ");
		while (idx < 256) {
			OutputDriveLogA("%02x ", pBuf[idx]);
			idx++;
		}
		OutputDriveLogA("\n");
	}
}
