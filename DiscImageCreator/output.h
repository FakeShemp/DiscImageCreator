/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"

#ifdef UNICODE
#define WFLAG "w, ccs=UTF-8"
#define AFLAG "a, ccs=UTF-8"
#else
#define WFLAG "w"
#define AFLAG "a"
#endif

#define BOOLEAN_TO_STRING_TRUE_FALSE(_b_) \
	( (_b_) ? _T("True") : _T("False") )

#define BOOLEAN_TO_STRING_YES_NO(_b_) \
	( (_b_) ? _T("Yes") : _T("No") )

#define OutputString(str, ...) \
{ \
	_tprintf(str, __VA_ARGS__); \
}
#ifdef _DEBUG
extern _TCHAR logBuffer[2048];
#define OutputErrorString(str, ...) \
{ \
	_stprintf(logBuffer, str, __VA_ARGS__); \
	OutputDebugString(logBuffer); \
}
#define OutputDiscLog(str, ...) \
{ \
	_stprintf(logBuffer, str, __VA_ARGS__); \
	OutputDebugString(logBuffer); \
}
#define OutputDriveLog(str, ...) \
{ \
	_stprintf(logBuffer, str, __VA_ARGS__); \
	OutputDebugString(logBuffer); \
}
#define OutputErrorLog(str, ...) \
{ \
	_stprintf(logBuffer, str, __VA_ARGS__); \
	OutputDebugString(logBuffer); \
}
#else
// If it uses g_LogFile, call InitLogFile()
extern _LOG_FILE g_LogFile;
#define OutputErrorString(str, ...) \
{ \
	_ftprintf(stderr, str, __VA_ARGS__); \
}
#define OutputDiscLog(str, ...) \
{ \
	_ftprintf(g_LogFile.fpDisc, str, __VA_ARGS__); \
}
#define OutputDriveLog(str, ...) \
{ \
	_ftprintf(g_LogFile.fpDrive, str, __VA_ARGS__); \
}
#define OutputErrorLog(str, ...) \
{ \
	_ftprintf(g_LogFile.fpError, str, __VA_ARGS__); \
}
#define FlushLog() \
{ \
	fflush(g_LogFile.fpDisc); \
	fflush(g_LogFile.fpDrive); \
	fflush(g_LogFile.fpError); \
}
#endif

#define FcloseAndNull(fp) \
{ \
	if (fp) { \
		fclose(fp); \
		fp = NULL; \
	} \
}

#define FreeAndNull(lpBuf) \
{ \
	if (lpBuf) { \
		free(lpBuf); \
		lpBuf = NULL; \
	} \
}

FILE* CreateOrOpenFileW(
	LPCTSTR pszPath,
	LPTSTR pszOutPath,
	LPTSTR pszPathWithoutPath,
	LPTSTR pszPathWithoutPathAndExt,
	LPCTSTR pszExt,
	LPCTSTR pszMode,
	UINT uiTrackNum,
	UINT uiMaxTrackNum
	);

FILE* CreateOrOpenFileA(
	LPCSTR pszPath,
	LPSTR pszOutPath,
	LPSTR pszPathWithoutPath,
	LPCSTR pszExt,
	LPCSTR pszMode,
	UINT uiTrackNum,
	UINT uiMaxTrackNum
	);

FILE* OpenProgrammabledFile(
	LPCTSTR pszPath,
	LPCTSTR pszMode
	);

VOID WriteCcdFileForDisc(
	UINT uiTocEntries,
	BYTE LastCompleteSession,
	FILE* fpCcd
	);

VOID WriteCcdFileForDiscCDTextLength(
	UINT uiCDTextSize,
	FILE* fpCcd
	);

VOID WriteCcdFileForDiscCatalog(
	PDISC_DATA pDiscData,
	FILE* fpCcd
	);

VOID WriteCcdFileForCDText(
	UINT uiCDTextSize,
	FILE* fpCcd
	);

VOID WriteCcdFileForCDTextEntry(
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	UINT uiTocTextEntries,
	FILE* fpCcd
	);

VOID WriteCcdFileForSession(
	BYTE SessionNumber,
	BYTE byMode,
	FILE* fpCcd
	);

VOID WriteCcdFileForEntry(
	PCDROM_TOC_FULL_TOC_DATA_BLOCK toc,
	UINT a,
	FILE* fpCcd
	);

VOID WriteCcdFileForTrack(
	PDISC_DATA pDiscData,
	UINT nTrackNum,
	BYTE byModeNum,
	BOOL bISRC,
	BYTE byCtl,
	FILE* fpCcd
	);

VOID WriteCcdFileForTrackIndex(
	INT nIndex,
	INT nLba,
	FILE* fpCcd
	);

VOID WriteCueFileFirst(
	PDISC_DATA pDiscData,
	BOOL lpCatalog,
	FILE* fpCue
	);

VOID WriteCueFile(
	PDISC_DATA pDiscData,
	LPCTSTR pszPath,
	BOOL bCDG,
	UINT nTrackNum,
	BYTE byModeNum, 
	BOOL bISRC,
	BYTE byCtl,
	FILE* fpCue
	);

VOID WriteCueFileForIndex(
	BYTE byIndex,
	BYTE byFrame, 
	BYTE bySecond,
	BYTE byMinute,
	FILE* fpCue
	);

VOID WriteMainChannel(
	PDISC_DATA pDiscData,
	LPBYTE lpBuf,
	LPINT* lpLBAStartList,
	INT nLBA,
	PCD_OFFSET_DATA pCdOffsetData,
	FILE* fpImg
	);

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
	);

BOOL WriteParsingSubfile(
	LPCTSTR pszSubfile
	);

BOOL DescrambleMainChannelForGD(
	LPCTSTR pszPath
	);

BOOL SplitFileForGD(
	LPCTSTR pszPath
	);

VOID DescrambleMainChannel(
	PDISC_DATA pDiscData,
	LPINT* lpLBAOfDataTrackList,
	FILE* fpTbl,
	FILE* fpImg
	);

BOOL MergeMainChannelAndCDG(
	LPCTSTR pszPath,
	BOOL bCDG,
	BOOL bAudioOnly,
	FILE* fpImg
	);

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
	);

VOID OutputHashData(
	FILE* fpHash,
	LPCTSTR filename,
	UINT64 ui64FileSize,
	DWORD crc32,
	LPBYTE digest,
	LPBYTE Message_Digest
	);

VOID OutputLastErrorNumAndString(
	LPCTSTR pszFuncName,
	LONG lLineNum
	);
