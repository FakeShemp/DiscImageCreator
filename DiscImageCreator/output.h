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
	((_b_) ? _T("True") : _T("False"))

#define BOOLEAN_TO_STRING_TRUE_FALSE_A(_b_) \
	((_b_) ? "True" : "False")

#define BOOLEAN_TO_STRING_YES_NO(_b_) \
	((_b_) ? _T("Yes") : _T("No"))

#define BOOLEAN_TO_STRING_YES_NO_A(_b_) \
	((_b_) ? "Yes" : "No")

#define OutputString(str, ...) \
{ \
	_tprintf(str, __VA_ARGS__); \
}
#ifdef _DEBUG
extern _TCHAR logBuffer[2048];
extern CHAR logBufferA[2048];
#define OutputDebugStringEx(str, ...) \
{ \
	_sntprintf(logBuffer, 2048, str, __VA_ARGS__); \
	logBuffer[2047] = 0; \
	OutputDebugString(logBuffer); \
}
#define OutputDebugStringExA(str, ...) \
{ \
	_snprintf(logBufferA, 2048, str, __VA_ARGS__); \
	logBufferA[2047] = 0; \
	OutputDebugStringA(logBufferA); \
}
#define OutputErrorString(str, ...) \
{ \
	OutputDebugStringEx(str, __VA_ARGS__); \
}
#define OutputDiscLog(str, ...) \
{ \
	OutputDebugStringEx(str, __VA_ARGS__); \
}
#define OutputDiscLogA(str, ...) \
{ \
	OutputDebugStringExA(str, __VA_ARGS__); \
}
#define OutputDriveLog(str, ...) \
{ \
	OutputDebugStringEx(str, __VA_ARGS__); \
}
#define OutputDriveLogA(str, ...) \
{ \
	OutputDebugStringExA(str, __VA_ARGS__); \
}
#define OutputErrorLog(str, ...) \
{ \
	OutputDebugStringEx(str, __VA_ARGS__); \
}
#define OutputErrorLogA(str, ...) \
{ \
	OutputDebugStringExA(str, __VA_ARGS__); \
}
#define OutputInfoLog(str, ...) \
{ \
	OutputDebugStringEx(str, __VA_ARGS__); \
}
#define OutputInfoLogA(str, ...) \
{ \
	OutputDebugStringExA(str, __VA_ARGS__); \
}
#define OutputLog(type, str, ...) \
{ \
	OutputDebugStringEx(str, __VA_ARGS__); \
}
#define OutputLogA(type, str, ...) \
{ \
	OutputDebugStringExA(str, __VA_ARGS__); \
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
#define OutputDiscLogA(str, ...) \
{ \
	fprintf(g_LogFile.fpDisc, str, __VA_ARGS__); \
}
#define OutputDriveLog(str, ...) \
{ \
	_ftprintf(g_LogFile.fpDrive, str, __VA_ARGS__); \
}
#define OutputDriveLogA(str, ...) \
{ \
	fprintf(g_LogFile.fpDrive, str, __VA_ARGS__); \
}
#define OutputErrorLog(str, ...) \
{ \
	_ftprintf(g_LogFile.fpError, str, __VA_ARGS__); \
}
#define OutputErrorLogA(str, ...) \
{ \
	fprintf(g_LogFile.fpError, str, __VA_ARGS__); \
}
#define OutputInfoLog(str, ...) \
{ \
	_ftprintf(g_LogFile.fpInfo, str, __VA_ARGS__); \
}
#define OutputInfoLogA(str, ...) \
{ \
	fprintf(g_LogFile.fpInfo, str, __VA_ARGS__); \
}
#define OutputLog(type, str, ...) \
{ \
	if (type == disc) { \
		OutputDiscLog(str, __VA_ARGS__); \
	} \
	else if (type == drive) { \
		OutputDriveLog(str, __VA_ARGS__); \
	} \
	else if (type == error) { \
		OutputErrorLog(str, __VA_ARGS__); \
	} \
	else if (type == info) { \
		OutputInfoLog(str, __VA_ARGS__); \
	} \
}
#define OutputLogA(type, str, ...) \
{ \
	if (type == disc) { \
		OutputDiscLogA(str, __VA_ARGS__); \
	} \
	else if (type == drive) { \
		OutputDriveLogA(str, __VA_ARGS__); \
	} \
	else if (type == error) { \
		OutputErrorLogA(str, __VA_ARGS__); \
	} \
	else if (type == info) { \
		OutputInfoLogA(str, __VA_ARGS__); \
	} \
}
#define FlushLog() \
{ \
	fflush(g_LogFile.fpDisc); \
	fflush(g_LogFile.fpDrive); \
	fflush(g_LogFile.fpError); \
	fflush(g_LogFile.fpInfo); \
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
	LPCTSTR pszPlusFname,
	LPTSTR pszOutPath,
	LPTSTR pszPathWithoutPath,
	LPTSTR pszPathWithoutPathAndExt,
	LPCTSTR pszExt,
	LPCTSTR pszMode,
	BYTE byTrackNum,
	BYTE byMaxTrackNum
	);

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
	);

FILE* OpenProgrammabledFile(
	LPCTSTR pszPath,
	LPCTSTR pszMode
	);

VOID WriteCcdFileForDisc(
	WORD wTocEntries,
	BYTE LastCompleteSession,
	FILE* fpCcd
	);

VOID WriteCcdFileForDiscCDTextLength(
	WORD wCDTextLength,
	FILE* fpCcd
	);

VOID WriteCcdFileForDiscCatalog(
	PDISC pDisc,
	FILE* fpCcd
	);

VOID WriteCcdFileForCDText(
	WORD dwTocTextEntries,
	FILE* fpCcd
	);

VOID WriteCcdFileForCDTextEntry(
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	WORD dwTocTextEntries,
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

VOID WriteMainChannel(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	INT nLBA,
	FILE* fpImg
	);

VOID WriteC2(
	PDISC pDisc,
	LPBYTE lpBuf,
	INT nLBA,
	FILE* fpC2
	);

VOID WriteSubChannel(
	PDISC pDisc,
	LPBYTE lpSubcodeRaw,
	LPBYTE lpSubcode,
	INT nLBA,
	BYTE byCurrentTrackNum,
	FILE* fpSub,
	FILE* fpParse
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
	PEXT_ARG pExtArg,
	PDISC pDisc,
	FILE* fpTbl,
	FILE* fpImg
	);

BOOL CreateBinCueCcd(
	PDISC pDisc,
	LPCTSTR pszPath,
	LPCTSTR pszImgName,
	BOOL lpCatalog,
	FILE* fpImg,
	FILE* fpCue,
	FILE* fpCueForImg,
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

BOOL OutputWindowsVer(
	);

VOID OutputEeprom(
	LPBYTE pBuf,
	DWORD tLen,
	INT nRoop,
	INT nLife
	);
