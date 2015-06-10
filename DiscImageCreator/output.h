/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once

#define BOOLEAN_TO_STRING_TRUE_FALSE_W(_b_) \
	((_b_) ? _T("True") : _T("False"))

#define BOOLEAN_TO_STRING_TRUE_FALSE_A(_b_) \
	((_b_) ? "True" : "False")

#define BOOLEAN_TO_STRING_YES_NO_W(_b_) \
	((_b_) ? _T("Yes") : _T("No"))

#define BOOLEAN_TO_STRING_YES_NO_A(_b_) \
	((_b_) ? "Yes" : "No")

#define OutputStringW(str, ...) \
{ \
	_tprintf(str, __VA_ARGS__); \
}
#define OutputStringA(str, ...) \
{ \
	printf(str, __VA_ARGS__); \
}

#ifdef _DEBUG
extern _TCHAR logBufferW[DISC_RAW_READ_SIZE];
extern CHAR logBufferA[DISC_RAW_READ_SIZE];
#define OutputDebugStringExW(str, ...) \
{ \
	_sntprintf(logBufferW, DISC_RAW_READ_SIZE, str, __VA_ARGS__); \
	logBufferW[2047] = 0; \
	OutputDebugString(logBufferW); \
}
#define OutputDebugStringExA(str, ...) \
{ \
	_snprintf(logBufferA, DISC_RAW_READ_SIZE, str, __VA_ARGS__); \
	logBufferA[2047] = 0; \
	OutputDebugStringA(logBufferA); \
}

#define OutputErrorStringW(str, ...) \
{ \
	OutputDebugStringExW(str, __VA_ARGS__); \
}
#define OutputErrorStringA(str, ...) \
{ \
	OutputDebugStringExA(str, __VA_ARGS__); \
}
#define OutputDiscLogW(str, ...) \
{ \
	OutputDebugStringExW(str, __VA_ARGS__); \
}
#define OutputDiscLogA(str, ...) \
{ \
	OutputDebugStringExA(str, __VA_ARGS__); \
}
#define OutputDriveLogW(str, ...) \
{ \
	OutputDebugStringExW(str, __VA_ARGS__); \
}
#define OutputDriveLogA(str, ...) \
{ \
	OutputDebugStringExA(str, __VA_ARGS__); \
}
#define OutputSubErrorLogW(str, ...) \
{ \
	OutputDebugStringExW(str, __VA_ARGS__); \
}
#define OutputSubErrorLogA(str, ...) \
{ \
	OutputDebugStringExA(str, __VA_ARGS__); \
}
#define OutputC2ErrorLogW(str, ...) \
{ \
	OutputDebugStringExW(str, __VA_ARGS__); \
}
#define OutputC2ErrorLogA(str, ...) \
{ \
	OutputDebugStringExA(str, __VA_ARGS__); \
}
#define OutputInfoLogW(str, ...) \
{ \
	OutputDebugStringExW(str, __VA_ARGS__); \
}
#define OutputInfoLogA(str, ...) \
{ \
	OutputDebugStringExA(str, __VA_ARGS__); \
}
#define OutputLogW(type, str, ...) \
{ \
	OutputDebugStringExW(str, __VA_ARGS__); \
}
#define OutputLogA(type, str, ...) \
{ \
	OutputDebugStringExA(str, __VA_ARGS__); \
}
#else
// If it uses g_LogFile, call InitLogFile()
extern _LOG_FILE g_LogFile;
#define FlushLog() \
{ \
	fflush(g_LogFile.fpDisc); \
	fflush(g_LogFile.fpDrive); \
	fflush(g_LogFile.fpSubError); \
	fflush(g_LogFile.fpC2Error); \
	fflush(g_LogFile.fpInfo); \
}

#define OutputErrorStringW(str, ...) \
{ \
	_ftprintf(stderr, str, __VA_ARGS__); \
}
#define OutputErrorStringA(str, ...) \
{ \
	fprintf(stderr, str, __VA_ARGS__); \
}
#define OutputDiscLogW(str, ...) \
{ \
	_ftprintf(g_LogFile.fpDisc, str, __VA_ARGS__); \
}
#define OutputDiscLogA(str, ...) \
{ \
	fprintf(g_LogFile.fpDisc, str, __VA_ARGS__); \
}
#define OutputDriveLogW(str, ...) \
{ \
	_ftprintf(g_LogFile.fpDrive, str, __VA_ARGS__); \
}
#define OutputDriveLogA(str, ...) \
{ \
	fprintf(g_LogFile.fpDrive, str, __VA_ARGS__); \
}
#define OutputSubErrorLogW(str, ...) \
{ \
	_ftprintf(g_LogFile.fpSubError, str, __VA_ARGS__); \
}
#define OutputSubErrorLogA(str, ...) \
{ \
	fprintf(g_LogFile.fpSubError, str, __VA_ARGS__); \
}
#define OutputC2ErrorLogW(str, ...) \
{ \
	_ftprintf(g_LogFile.fpC2Error, str, __VA_ARGS__); \
}
#define OutputC2ErrorLogA(str, ...) \
{ \
	fprintf(g_LogFile.fpC2Error, str, __VA_ARGS__); \
}
#define OutputInfoLogW(str, ...) \
{ \
	_ftprintf(g_LogFile.fpInfo, str, __VA_ARGS__); \
}
#define OutputInfoLogA(str, ...) \
{ \
	fprintf(g_LogFile.fpInfo, str, __VA_ARGS__); \
}
#define OutputLogW(type, str, ...) \
{ \
	INT t = type; \
	if ((t & standardOut) == standardOut) { \
		OutputStringW(str, __VA_ARGS__); \
	} \
	if ((t & standardErr) == standardErr) { \
		OutputErrorStringW(str, __VA_ARGS__); \
	} \
	if ((t & fileDisc) == fileDisc) { \
		OutputDiscLogW(str, __VA_ARGS__); \
	} \
	if ((t & fileDrive) == fileDrive) { \
		OutputDriveLogW(str, __VA_ARGS__); \
	} \
	if ((t & fileSubError) == fileSubError) { \
		OutputSubErrorLogW(str, __VA_ARGS__); \
	} \
	if ((t & fileC2Error) == fileC2Error) { \
		OutputC2ErrorLogW(str, __VA_ARGS__); \
	} \
	if ((t & fileInfo) == fileInfo) { \
		OutputInfoLogW(str, __VA_ARGS__); \
	} \
}
#define OutputLogA(type, str, ...) \
{ \
	INT t = type; \
	if ((t & standardOut) == standardOut) { \
		OutputStringA(str, __VA_ARGS__); \
	} \
	if ((t & standardErr) == standardErr) { \
		OutputErrorStringA(str, __VA_ARGS__); \
	} \
	if ((t & fileDisc) == fileDisc) { \
		OutputDiscLogA(str, __VA_ARGS__); \
	} \
	if ((t & fileDrive) == fileDrive) { \
		OutputDriveLogA(str, __VA_ARGS__); \
	} \
	if ((t & fileSubError) == fileSubError) { \
		OutputSubErrorLogA(str, __VA_ARGS__); \
	} \
	if ((t & fileC2Error) == fileC2Error) { \
		OutputC2ErrorLogA(str, __VA_ARGS__); \
	} \
	if ((t & fileInfo) == fileInfo) { \
		OutputInfoLogA(str, __VA_ARGS__); \
	} \
}
#endif

#ifdef UNICODE
#define WFLAG "w, ccs=UTF-8"
#define AFLAG "a, ccs=UTF-8"
#define BOOLEAN_TO_STRING_TRUE_FALSE BOOLEAN_TO_STRING_TRUE_FALSE_W
#define BOOLEAN_TO_STRING_YES_NO BOOLEAN_TO_STRING_YES_NO_W
#define OutputString OutputStringW
#define OutputErrorString OutputErrorStringW
#define OutputDiscLog OutputDiscLogW
#define OutputDriveLog OutputDriveLogW
#define OutputSubErrorLog OutputSubErrorLogW
#define OutputC2ErrorLog OutputC2ErrorLogW
#define OutputInfoLog OutputInfoLogW
#else
#define WFLAG "w"
#define AFLAG "a"
#define BOOLEAN_TO_STRING_TRUE_FALSE BOOLEAN_TO_STRING_TRUE_FALSE_A
#define BOOLEAN_TO_STRING_YES_NO BOOLEAN_TO_STRING_YES_NO_A
#define OutputString OutputStringA
#define OutputErrorString OutputErrorStringA
#define OutputDiscLog OutputDiscLogA
#define OutputDriveLog OutputDriveLogA
#define OutputSubErrorLog OutputSubErrorLogA
#define OutputC2ErrorLog OutputC2ErrorLogA
#define OutputInfoLog OutputInfoLogA
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

VOID WriteErrorBuffer(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PMAIN_HEADER pMain,
	LPBYTE lpBuf,
	LPBYTE lpScrambledBuf,
	LPBYTE lpSubcode,
	INT nLBA,
	FILE* fpImg,
	FILE* fpSub,
	FILE* fpC2
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
	LPBYTE lpScrambledBuf,
	FILE* fpImg
	);

BOOL CreateBinCueCcd(
	PDISC pDisc,
	LPCTSTR pszPath,
	LPCTSTR pszImgName,
	BOOL bCanCDText,
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
