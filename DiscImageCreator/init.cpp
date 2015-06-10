/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "init.h"
#include "output.h"

// These global variable is set at DiscImageCreator.cpp
extern BYTE g_aSyncHeader[SYNC_SIZE];

BOOL InitC2ErrorData(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	PC2_ERROR_PER_SECTOR* pC2ErrorPerSector,
	DWORD dwAllBufLen
	)
{
	BOOL bRet = TRUE;
	if (NULL == (*pC2ErrorPerSector = (PC2_ERROR_PER_SECTOR)
		calloc(pExtArg->dwMaxC2ErrorNum, sizeof(C2_ERROR_PER_SECTOR)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	pDisc->C2.sC2Offset = (SHORT)(pDisc->MAIN.nCombinedOffset / CHAR_BIT);
	try {
		for (UINT n = 0; n < pExtArg->dwMaxC2ErrorNum; n++) {
			OutputString(_T("\rAllocating memory for C2 errors: %u/%u"), 
				n + 1, pExtArg->dwMaxC2ErrorNum);
			if (NULL == ((*pC2ErrorPerSector)[n].lpErrorBytePos = 
				(PSHORT)calloc(CD_RAW_SECTOR_SIZE, sizeof(WORD)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputString(_T("\n"));
				throw FALSE;
			}
			if (NULL == ((*pC2ErrorPerSector)[n].lpErrorBytePosBackup = 
				(PSHORT)calloc(CD_RAW_SECTOR_SIZE, sizeof(WORD)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputString(_T("\n"));
				throw FALSE;
			}
			if (NULL == ((*pC2ErrorPerSector)[n].lpBufNoC2Sector = 
				(LPBYTE)calloc(dwAllBufLen, sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputString(_T("\n"));
				throw FALSE;
			}
			if (NULL == ((*pC2ErrorPerSector)[n].lpBufNoC2SectorBackup = 
				(LPBYTE)calloc(dwAllBufLen, sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputString(_T("\n"));
				throw FALSE;
			}
			(*pC2ErrorPerSector)[n].byErrorFlag = RETURNED_NO_C2_ERROR_1ST;
		}
		OutputString(_T("\n"));
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	return bRet;
}

BOOL InitLBAPerTrack(
	PEXEC_TYPE pExecType,
	PDISC* pDisc
	)
{
	size_t dwTrackAllocSize = 
		*pExecType == gd ? 100 : (size_t)(*pDisc)->SCSI.toc.LastTrack + 1;
	if (NULL == ((*pDisc)->SCSI.lpFirstLBAListOnToc = 
		(LPINT)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	if (NULL == ((*pDisc)->SCSI.lpLastLBAListOnToc = 
		(LPINT)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	return TRUE;
}

BOOL InitTocFullData(
	PEXEC_TYPE pExecType,
	PDISC* pDisc
	)
{
	size_t dwTrackAllocSize =
		*pExecType == gd ? 100 : (size_t)(*pDisc)->SCSI.toc.LastTrack + 1;
	if (NULL == ((*pDisc)->SCSI.lpSessionNumList =
		(LPBYTE)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	return TRUE;
}

BOOL InitTocTextData(
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	PDISC* pDisc
	)
{
	BOOL bRet = TRUE;
	size_t dwTrackAllocSize =
		*pExecType == gd ? 100 : (size_t)(*pDisc)->SCSI.toc.LastTrack + 1;
	try {
		if (pDevice->FEATURE.byCanCDText || *pExecType == gd) {
			if (NULL == ((*pDisc)->SUB.pszISRC = 
				(LPSTR*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == ((*pDisc)->SCSI.pszTitle = 
				(LPSTR*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == ((*pDisc)->SCSI.pszPerformer = 
				(LPSTR*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == ((*pDisc)->SCSI.pszSongWriter = 
				(LPSTR*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}

			size_t isrcSize = META_ISRC_SIZE;
			size_t textSize = META_CDTEXT_SIZE;
			for (size_t h = 0; h < dwTrackAllocSize; h++) {
				if (NULL == ((*pDisc)->SUB.pszISRC[h] = 
					(LPSTR)calloc(isrcSize, sizeof(CHAR)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (NULL == ((*pDisc)->SCSI.pszTitle[h] = 
					(LPSTR)calloc(textSize, sizeof(CHAR)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (NULL == ((*pDisc)->SCSI.pszPerformer[h] = 
					(LPSTR)calloc(textSize, sizeof(CHAR)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (NULL == ((*pDisc)->SCSI.pszSongWriter[h] = 
					(LPSTR)calloc(textSize, sizeof(CHAR)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
			}
		}
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	return bRet;
}

VOID InitMainDataHeader(
	PMAIN_HEADER pMain
	)
{
	memcpy(pMain->header, g_aSyncHeader, sizeof(g_aSyncHeader));
	pMain->header[12] = 0x01;
	pMain->header[13] = 0x82;
	pMain->header[14] = (BYTE)-1;
}

BOOL InitSubData(
	PEXEC_TYPE pExecType,
	PDISC* pDisc
	)
{
	BOOL bRet = TRUE;
	size_t dwTrackAllocSize =
		*pExecType == gd ? 100 : (size_t)(*pDisc)->SCSI.toc.LastTrack + 1;
	(*pDisc)->SUB.byCatalog = FALSE;
	(*pDisc)->SUB.nFirstLBAForMCN = -1;
	(*pDisc)->SUB.nRangeLBAForMCN = -1;
	(*pDisc)->SUB.byISRC = FALSE;
	(*pDisc)->SUB.nFirstLBAForISRC = -1;
	(*pDisc)->SUB.nRangeLBAForISRC = -1;
	try {
		if (NULL == ((*pDisc)->SUB.lpRtoWList =
			(LPBYTE)calloc(dwTrackAllocSize, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->SUB.lpFirstLBAListOnSub =
			(LPINT*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->SUB.lpFirstLBAListOnSubSync = 
			(LPINT*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->SUB.lpFirstLBAListOfDataTrackOnSub = 
			(LPINT)calloc(dwTrackAllocSize, sizeof(INT)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->SUB.lpLastLBAListOfDataTrackOnSub = 
			(LPINT)calloc(dwTrackAllocSize, sizeof(INT)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->SUB.lpISRCList = 
			(LPBOOL)calloc(dwTrackAllocSize, sizeof(BOOL)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->SUB.lpCtlList = 
			(LPBYTE)calloc(dwTrackAllocSize, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->MAIN.lpModeList = 
			(LPBYTE)calloc(dwTrackAllocSize, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == ((*pDisc)->SUB.lpEndCtlList = 
			(LPBYTE)calloc(dwTrackAllocSize, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}

		size_t dwIndexAllocSize = (size_t)MAXIMUM_NUMBER_INDEXES * sizeof(UINT_PTR);
		for (size_t h = 0; h < dwTrackAllocSize; h++) {
			if (NULL == ((*pDisc)->SUB.lpFirstLBAListOnSub[h] = (LPINT)malloc(dwIndexAllocSize))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			FillMemory((*pDisc)->SUB.lpFirstLBAListOnSub[h], dwIndexAllocSize, -1);
			if (NULL == ((*pDisc)->SUB.lpFirstLBAListOnSubSync[h] = (LPINT)malloc(dwIndexAllocSize))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			FillMemory((*pDisc)->SUB.lpFirstLBAListOnSubSync[h], dwIndexAllocSize, -1);
			(*pDisc)->SUB.lpFirstLBAListOfDataTrackOnSub[h] = -1;
			(*pDisc)->SUB.lpLastLBAListOfDataTrackOnSub[h] = -1;
		}
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	return bRet;
}

#ifndef _DEBUG
LOG_FILE g_LogFile;

BOOL InitLogFile(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	_TCHAR* szFullPath
	)
{
	CHAR path[_MAX_PATH] = { 0 };
#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, szFullPath, _MAX_PATH, path, sizeof(path), NULL, NULL);
#else
	strncpy(path, szFullPath, sizeof(path));
#endif
	CONST INT size = 32;
	CHAR szDiscLogtxt[size] = { 0 };
	CHAR szDriveLogtxt[size] = { 0 };
	CHAR szInfoLogtxt[size] = { 0 };
	CHAR szSubErrorLogtxt[size] = { 0 };
	CHAR szC2ErrorLogtxt[size] = { 0 };

	if (*pExecType == floppy) {
		strncpy(szDiscLogtxt, "_disclog_fd", size);
	}
	else if (*pExecType == dvd) {
		strncpy(szDiscLogtxt, "_disclog_dvd", size);
		strncpy(szDriveLogtxt, "_drivelog_dvd", size);
		strncpy(szInfoLogtxt, "_infolog_dvd", size);
	}
	else if (*pExecType == gd) {
		strncpy(szDiscLogtxt, "_disclog_gd", size);
		strncpy(szDriveLogtxt, "_drivelog_gd", size);
		strncpy(szInfoLogtxt, "_infolog_gd", size);
		strncpy(szSubErrorLogtxt, "_suberrorlog_gd", size);
		strncpy(szC2ErrorLogtxt, "_c2errorlog_gd", size);
	}
	else {
		strncpy(szDiscLogtxt, "_disclog", size);
		strncpy(szDriveLogtxt, "_drivelog", size);
		strncpy(szInfoLogtxt, "_infolog", size);
		strncpy(szSubErrorLogtxt, "_suberrorlog", size);
		strncpy(szC2ErrorLogtxt, "_c2errorlog", size);
	}

	g_LogFile.fpDisc = CreateOrOpenFileA(
		path, szDiscLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0);
	if (!g_LogFile.fpDisc) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = TRUE;
	try {
		if (*pExecType != floppy) {
			g_LogFile.fpDrive = CreateOrOpenFileA(
				path, szDriveLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0);
			if (!g_LogFile.fpDrive) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			g_LogFile.fpInfo = CreateOrOpenFileA(
				path, szInfoLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0);
			if (!g_LogFile.fpInfo) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (*pExecType != dvd) {
				g_LogFile.fpSubError = CreateOrOpenFileA(
					path, szSubErrorLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0);
				if (!g_LogFile.fpSubError) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (pExtArg->byC2) {
					g_LogFile.fpC2Error = CreateOrOpenFileA(
						path, szC2ErrorLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0);
					if (!g_LogFile.fpC2Error) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						throw FALSE;
					}
				}
			}
		}
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	return bRet;
}
#endif

VOID TerminateC2ErrorData(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PC2_ERROR_PER_SECTOR* pC2ErrorPerSector
	)
{
	if (*pC2ErrorPerSector && pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
		for (UINT i = 0; i < pExtArg->dwMaxC2ErrorNum; i++) {
			OutputString(_T("\rFreeing allocated memory for C2 errors: %u/%u"), 
				i + 1, pExtArg->dwMaxC2ErrorNum);
			FreeAndNull((*pC2ErrorPerSector)[i].lpErrorBytePos);
			FreeAndNull((*pC2ErrorPerSector)[i].lpErrorBytePosBackup);
			FreeAndNull((*pC2ErrorPerSector)[i].lpBufNoC2Sector);
			FreeAndNull((*pC2ErrorPerSector)[i].lpBufNoC2SectorBackup);
		}
		OutputString(_T("\n"));
		FreeAndNull(*pC2ErrorPerSector);
	}
}

VOID TerminateLBAPerTrack(
	PDISC* pDisc
	)
{
	FreeAndNull((*pDisc)->SCSI.lpFirstLBAListOnToc);
	FreeAndNull((*pDisc)->SCSI.lpLastLBAListOnToc);
}

VOID TerminateTocFullData(
	PDISC* pDisc
	)
{
	FreeAndNull((*pDisc)->SCSI.lpSessionNumList);
}

VOID TerminateTocTextData(
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	PDISC* pDisc
	)
{
	size_t dwTrackAllocSize =
		*pExecType == gd ? 100 : (size_t)(*pDisc)->SCSI.toc.LastTrack + 1;
	if (pDevice->FEATURE.byCanCDText) {
		for (size_t i = 0; i < dwTrackAllocSize; i++) {
			FreeAndNull((*pDisc)->SUB.pszISRC[i]);
			FreeAndNull((*pDisc)->SCSI.pszTitle[i]);
			FreeAndNull((*pDisc)->SCSI.pszPerformer[i]);
			FreeAndNull((*pDisc)->SCSI.pszSongWriter[i]);
		}
		FreeAndNull((*pDisc)->SUB.pszISRC);
		FreeAndNull((*pDisc)->SCSI.pszTitle);
		FreeAndNull((*pDisc)->SCSI.pszPerformer);
		FreeAndNull((*pDisc)->SCSI.pszSongWriter);
	}
}

VOID TerminateSubData(
	PEXEC_TYPE pExecType,
	PDISC* pDisc
	)
{
	size_t dwTrackAllocSize =
		*pExecType == gd ? 100 : (size_t)(*pDisc)->SCSI.toc.LastTrack + 1;
	for (size_t h = 0; h < dwTrackAllocSize; h++) {
		if ((*pDisc)->SUB.lpFirstLBAListOnSub) {
			FreeAndNull((*pDisc)->SUB.lpFirstLBAListOnSub[h]);
		}
		if ((*pDisc)->SUB.lpFirstLBAListOnSubSync) {
			FreeAndNull((*pDisc)->SUB.lpFirstLBAListOnSubSync[h]);
		}
	}
	FreeAndNull((*pDisc)->SUB.lpRtoWList);
	FreeAndNull((*pDisc)->SUB.lpFirstLBAListOnSub);
	FreeAndNull((*pDisc)->SUB.lpFirstLBAListOnSubSync);
	FreeAndNull((*pDisc)->SUB.lpFirstLBAListOfDataTrackOnSub);
	FreeAndNull((*pDisc)->SUB.lpLastLBAListOfDataTrackOnSub);
	FreeAndNull((*pDisc)->SUB.lpCtlList);
	FreeAndNull((*pDisc)->SUB.lpEndCtlList);
	FreeAndNull((*pDisc)->SUB.lpISRCList);
	FreeAndNull((*pDisc)->MAIN.lpModeList);
}

#ifndef _DEBUG
VOID TerminateLogFile(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg
	)
{
	FcloseAndNull(g_LogFile.fpDisc);
	if (*pExecType != floppy) {
		FcloseAndNull(g_LogFile.fpDrive);
		FcloseAndNull(g_LogFile.fpInfo);
		if (*pExecType != dvd) {
			FcloseAndNull(g_LogFile.fpSubError);
			if (pExtArg->byC2) {
				FcloseAndNull(g_LogFile.fpC2Error);
			}
		}
	}
}
#endif
