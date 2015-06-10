/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"
#include "struct.h"
#include "init.h"
#include "output.h"

BOOL InitC2ErrorData(
	PEXT_ARG pExtArg,
	PC2_ERROR_DATA* c2ErrorData,
	DWORD dwAllBufLen
	)
{
	BOOL bRet = TRUE;
	if (NULL == (*c2ErrorData = 
		(PC2_ERROR_DATA)calloc(pExtArg->uiMaxC2ErrorNum, sizeof(C2_ERROR_DATA)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	try {
		for (UINT n = 0; n < pExtArg->uiMaxC2ErrorNum; n++) {
			OutputString(_T("\rAllocating memory for C2 error: %d/%d"), 
				n + 1, pExtArg->uiMaxC2ErrorNum);
			if (NULL == ((*c2ErrorData)[n].lpErrorBytePos = 
				(PSHORT)calloc(CD_RAW_SECTOR_SIZE, sizeof(WORD)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputString(_T("\n"));
				throw FALSE;
			}
			if (NULL == ((*c2ErrorData)[n].lpErrorBytePosBackup = 
				(PSHORT)calloc(CD_RAW_SECTOR_SIZE, sizeof(WORD)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputString(_T("\n"));
				throw FALSE;
			}
			if (NULL == ((*c2ErrorData)[n].lpBufC2NoneSector = 
				(LPBYTE)calloc(dwAllBufLen, sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputString(_T("\n"));
				throw FALSE;
			}
			if (NULL == ((*c2ErrorData)[n].lpBufC2NoneSectorBackup = 
				(LPBYTE)calloc(dwAllBufLen, sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputString(_T("\n"));
				throw FALSE;
			}
			(*c2ErrorData)[n].bErrorFlag = RETURNED_C2_ERROR_1ST_NONE;
		}
		OutputString(_T("\n"));
	}
	catch(BOOL bErr) {
		bRet = bErr;
	}
	return bRet;
}

BOOL InitTocData(
	PEXEC_TYPE pExecType,
	PDISC_DATA* pDiscData
	)
{
	size_t dwTrackAllocSize = *pExecType == rgd ? 100 : (size_t)(*pDiscData)->toc.LastTrack + 1;
	if (NULL == ((*pDiscData)->pnTocStartLBA = (PINT)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	if (NULL == ((*pDiscData)->pnTocEndLBA = (PINT)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	return TRUE;
}

BOOL InitTocFullData(
	PEXEC_TYPE pExecType,
	PDEVICE_DATA pDevData,
	PDISC_DATA* pDiscData
	)
{
	BOOL bRet = TRUE;
	size_t dwTrackAllocSize = *pExecType == rgd ? 100 : (size_t)(*pDiscData)->toc.LastTrack + 1;
	if (NULL == ((*pDiscData)->puiSessionNum = (PUINT)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	try {
		if (pDevData->bCanCDText || *pExecType == rgd) {
			if (NULL == ((*pDiscData)->pszISRC = (LPTSTR*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == ((*pDiscData)->pszTitle = (LPTSTR*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == ((*pDiscData)->pszPerformer = (LPTSTR*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == ((*pDiscData)->pszSongWriter = (LPTSTR*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}

			size_t isrcSize = META_ISRC_SIZE;
			size_t textSize = META_CDTEXT_SIZE;
			for (size_t h = 0; h < dwTrackAllocSize; h++) {
				if (NULL == ((*pDiscData)->pszISRC[h] = (LPTSTR)calloc(isrcSize, sizeof(_TCHAR)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (NULL == ((*pDiscData)->pszTitle[h] = (LPTSTR)calloc(textSize, sizeof(_TCHAR)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (NULL == ((*pDiscData)->pszPerformer[h] = (LPTSTR)calloc(textSize, sizeof(_TCHAR)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (NULL == ((*pDiscData)->pszSongWriter[h] = (LPTSTR)calloc(textSize, sizeof(_TCHAR)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
			}
		}
	}
	catch(BOOL bErr) {
		bRet = bErr;
	}
	return bRet;
}

#ifndef _DEBUG
LOG_FILE g_LogFile;

BOOL InitLogFile(
	PEXEC_TYPE pExecType,
	_TCHAR* szFullPath
	)
{
	_TCHAR szDriveLogtxt[32] = { 0 };
	_TCHAR szDiscLogtxt[32] = { 0 };
	_TCHAR szErrorLogtxt[32] = { 0 };
	if (*pExecType == rgd) {
		_tcscpy(szDiscLogtxt, _T("_dc_disclog.txt"));
		_tcscpy(szDriveLogtxt, _T("_dc_drivelog.txt"));
		_tcscpy(szErrorLogtxt, _T("_dc_errorlog.txt"));
	}
	else if (*pExecType == f) {
		_tcscpy(szDiscLogtxt, _T("_fd_disclog.txt"));
	}
	else {
		_tcscpy(szDiscLogtxt, _T("_disclog.txt"));
		_tcscpy(szDriveLogtxt, _T("_drivelog.txt"));
		_tcscpy(szErrorLogtxt, _T("_errorlog.txt"));
	}

	g_LogFile.fpDisc =
		CreateOrOpenFileW(szFullPath, NULL, NULL, NULL, szDiscLogtxt, _T(WFLAG), 0, 0);
	if (!g_LogFile.fpDisc) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = TRUE;
	try {
		if (*pExecType != f) {
			g_LogFile.fpDrive = CreateOrOpenFileW(szFullPath, NULL,
				NULL, NULL, szDriveLogtxt, _T(WFLAG), 0, 0);
			if (!g_LogFile.fpDrive) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			g_LogFile.fpError = CreateOrOpenFileW(szFullPath, NULL, NULL,
				NULL, szErrorLogtxt, _T(WFLAG), 0, 0);
			if (!g_LogFile.fpError) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
		}
	}
	catch(BOOL bErr) {
		bRet = bErr;
	}
	return bRet;
}
#endif

VOID TerminateC2ErrorData(
	PDEVICE_DATA pDevData,
	PEXT_ARG pExtArg,
	PC2_ERROR_DATA* c2ErrorData
	)
{
	if (*c2ErrorData && pDevData->bC2ErrorData && pExtArg->bC2) {
		for (UINT i = 0; i < pExtArg->uiMaxC2ErrorNum; i++) {
			OutputString(_T("\rFree memory for C2 error: %d/%d"), 
				i + 1, pExtArg->uiMaxC2ErrorNum);
			FreeAndNull((*c2ErrorData)[i].lpErrorBytePos);
			FreeAndNull((*c2ErrorData)[i].lpErrorBytePosBackup);
			FreeAndNull((*c2ErrorData)[i].lpBufC2NoneSector);
			FreeAndNull((*c2ErrorData)[i].lpBufC2NoneSectorBackup);
		}
		OutputString(_T("\n"));
		FreeAndNull(*c2ErrorData);
	}
}

VOID TerminateTocData(
	PDISC_DATA* pDiscData
	)
{
	FreeAndNull((*pDiscData)->pnTocStartLBA);
	FreeAndNull((*pDiscData)->pnTocEndLBA);
}

VOID TerminateTocFullData(
	PEXEC_TYPE pExecType,
	PDEVICE_DATA pDevData,
	PDISC_DATA* pDiscData
	)
{
	size_t dwTrackAllocSize = *pExecType == rgd ? 100 : (size_t)(*pDiscData)->toc.LastTrack + 1;
	FreeAndNull((*pDiscData)->puiSessionNum);
	if (pDevData->bCanCDText || *pExecType == rgd) {
		for (size_t i = 0; i < dwTrackAllocSize; i++) {
			FreeAndNull((*pDiscData)->pszISRC[i]);
			FreeAndNull((*pDiscData)->pszTitle[i]);
			FreeAndNull((*pDiscData)->pszPerformer[i]);
			FreeAndNull((*pDiscData)->pszSongWriter[i]);
		}
		FreeAndNull((*pDiscData)->pszISRC);
		FreeAndNull((*pDiscData)->pszTitle);
		FreeAndNull((*pDiscData)->pszPerformer);
		FreeAndNull((*pDiscData)->pszSongWriter);
	}
}

#ifndef _DEBUG
VOID TerminateLogFile(
	PEXEC_TYPE pExecType
	)
{
	FcloseAndNull(g_LogFile.fpDisc);
	if (*pExecType != f) {
		FcloseAndNull(g_LogFile.fpDrive);
		FcloseAndNull(g_LogFile.fpError);
	}
}
#endif
