/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"
#include "struct.h"
#include "init.h"
#include "output.h"

BOOL InitC2ErrorData(
	PEXT_ARG pExtArg,
	PDISC_DATA pDiscData,
	PC2_ERROR_DATA pC2ErrorData,
	PC2_ERROR_DATA_PER_SECTOR* c2ErrorDataPerSector,
	DWORD dwAllBufLen
	)
{
	BOOL bRet = TRUE;
	if (NULL == (*c2ErrorDataPerSector = (PC2_ERROR_DATA_PER_SECTOR)
		calloc(pExtArg->dwMaxC2ErrorNum, sizeof(C2_ERROR_DATA_PER_SECTOR)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	pC2ErrorData->sC2Offset =
		(SHORT)((pDiscData->MAIN_CHANNEL.nCombinedOffset - CD_RAW_READ_SUBCODE_SIZE) / CHAR_BIT);
	if (pDiscData->MAIN_CHANNEL.nCombinedOffset > 0) {
		//	Nonomura Byouin no Hitobito
		//	 Combined Offset(Byte)   1680, (Samples)   420
		//	-   Drive Offset(Byte)    120, (Samples)    30
		//	----------------------------------------------
		//	       CD Offset(Byte)   1560, (Samples)   390
		//	Need overread sector: 1

		// Policenauts
		//	 Combined Offset(Byte)   5136, (Samples)  1284
		//	-   Drive Offset(Byte)    120, (Samples)    30
		//	----------------------------------------------
		//	       CD Offset(Byte)   5016, (Samples)  1254
		//	Need overread sector: 3
		pC2ErrorData->cSlideSectorNum = (CHAR)(abs(pDiscData->MAIN_CHANNEL.nAdjustSectorNum));
	}
	else {
		// Anesan
		//	 Combined Offset(Byte)  -1932, (Samples)  -483
		//	-   Drive Offset(Byte)    120, (Samples)    30
		//	----------------------------------------------
		//	       CD Offset(Byte)  -2052, (Samples)  -513
		//	Need overread sector: -1

		// F1 Circus Special - Pole to Win
		//	 Combined Offset(Byte)  -2712, (Samples)  -678
		//	-   Drive Offset(Byte)    120, (Samples)    30
		//	----------------------------------------------
		//	       CD Offset(Byte)  -2832, (Samples)  -708
		//	Need overread sector: -2
		INT nSlideNum = pDiscData->MAIN_CHANNEL.nCombinedOffset / CD_RAW_SECTOR_SIZE;
		if (nSlideNum == 0) {
			pC2ErrorData->cSlideSectorNum = -1;
		}
		else {
			pC2ErrorData->cSlideSectorNum = (CHAR)(nSlideNum);
		}
	}
	try {
		for (UINT n = 0; n < pExtArg->dwMaxC2ErrorNum; n++) {
			OutputString(_T("\rAllocating memory for C2 error: %u/%u"), 
				n + 1, pExtArg->dwMaxC2ErrorNum);
			if (NULL == ((*c2ErrorDataPerSector)[n].lpErrorBytePos = 
				(PSHORT)calloc(CD_RAW_SECTOR_SIZE, sizeof(WORD)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputString(_T("\n"));
				throw FALSE;
			}
			if (NULL == ((*c2ErrorDataPerSector)[n].lpErrorBytePosBackup = 
				(PSHORT)calloc(CD_RAW_SECTOR_SIZE, sizeof(WORD)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputString(_T("\n"));
				throw FALSE;
			}
			if (NULL == ((*c2ErrorDataPerSector)[n].lpBufC2NoneSector = 
				(LPBYTE)calloc(dwAllBufLen, sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputString(_T("\n"));
				throw FALSE;
			}
			if (NULL == ((*c2ErrorDataPerSector)[n].lpBufC2NoneSectorBackup = 
				(LPBYTE)calloc(dwAllBufLen, sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputString(_T("\n"));
				throw FALSE;
			}
			(*c2ErrorDataPerSector)[n].bErrorFlag = RETURNED_C2_ERROR_NO_1ST;
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
	PDISC_DATA* pDiscData
	)
{
	size_t dwTrackAllocSize = 
		*pExecType == gd ? 100 : (size_t)(*pDiscData)->SCSI.toc.LastTrack + 1;
	if (NULL == ((*pDiscData)->SCSI.lpFirstLBAListOnToc = 
		(LPINT)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	if (NULL == ((*pDiscData)->SCSI.lpLastLBAListOnToc = 
		(LPINT)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
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
	size_t dwTrackAllocSize = 
		*pExecType == gd ? 100 : (size_t)(*pDiscData)->SCSI.toc.LastTrack + 1;
	if (NULL == ((*pDiscData)->SCSI.lpSessionNumList = 
		(LPBYTE)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	try {
		if (pDevData->bCanCDText || *pExecType == gd) {
			if (NULL == ((*pDiscData)->SUB_CHANNEL.pszISRC = 
				(LPTSTR*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == ((*pDiscData)->SCSI.pszTitle = 
				(LPTSTR*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == ((*pDiscData)->SCSI.pszPerformer = 
				(LPTSTR*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == ((*pDiscData)->SCSI.pszSongWriter = 
				(LPTSTR*)calloc(dwTrackAllocSize, sizeof(UINT_PTR)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}

			size_t isrcSize = META_ISRC_SIZE;
			size_t textSize = META_CDTEXT_SIZE;
			for (size_t h = 0; h < dwTrackAllocSize; h++) {
				if (NULL == ((*pDiscData)->SUB_CHANNEL.pszISRC[h] = 
					(LPTSTR)calloc(isrcSize, sizeof(_TCHAR)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (NULL == ((*pDiscData)->SCSI.pszTitle[h] = 
					(LPTSTR)calloc(textSize, sizeof(_TCHAR)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (NULL == ((*pDiscData)->SCSI.pszPerformer[h] = 
					(LPTSTR)calloc(textSize, sizeof(_TCHAR)))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
				if (NULL == ((*pDiscData)->SCSI.pszSongWriter[h] = 
					(LPTSTR)calloc(textSize, sizeof(_TCHAR)))) {
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

#ifndef _DEBUG
LOG_FILE g_LogFile;

BOOL InitLogFile(
	PEXEC_TYPE pExecType,
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
	CHAR szDriveLogtxt[size] = { 0 };
	CHAR szDiscLogtxt[size] = { 0 };
	CHAR szErrorLogtxt[size] = { 0 };
	CHAR szInfoLogtxt[size] = { 0 };
	if (*pExecType == gd) {
		strncpy(szDiscLogtxt, "_dc_disclog", size);
		strncpy(szDriveLogtxt, "_dc_drivelog", size);
		strncpy(szErrorLogtxt, "_dc_errorlog", size);
		strncpy(szInfoLogtxt, "_dc_infolog", size);
	}
	else if (*pExecType == floppy) {
		strncpy(szDiscLogtxt, "_fd_disclog", size);
	}
	else {
		strncpy(szDiscLogtxt, "_disclog", size);
		strncpy(szDriveLogtxt, "_drivelog", size);
		strncpy(szErrorLogtxt, "_errorlog", size);
		strncpy(szInfoLogtxt, "_infolog", size);
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
			g_LogFile.fpError = CreateOrOpenFileA(
				path, szErrorLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0);
			if (!g_LogFile.fpError) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			g_LogFile.fpInfo = CreateOrOpenFileA(
				path, szInfoLogtxt, NULL, NULL, NULL, ".txt", "w", 0, 0);
			if (!g_LogFile.fpInfo) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
		}
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	return bRet;
}
#endif

VOID Terminatec2ErrorDataPerSector(
	PDEVICE_DATA pDevData,
	PEXT_ARG pExtArg,
	PC2_ERROR_DATA_PER_SECTOR* c2ErrorDataPerSector
	)
{
	if (*c2ErrorDataPerSector && pDevData->bC2ErrorData && pExtArg->bC2) {
		for (UINT i = 0; i < pExtArg->dwMaxC2ErrorNum; i++) {
			OutputString(_T("\rFree memory for C2 error: %u/%u"), 
				i + 1, pExtArg->dwMaxC2ErrorNum);
			FreeAndNull((*c2ErrorDataPerSector)[i].lpErrorBytePos);
			FreeAndNull((*c2ErrorDataPerSector)[i].lpErrorBytePosBackup);
			FreeAndNull((*c2ErrorDataPerSector)[i].lpBufC2NoneSector);
			FreeAndNull((*c2ErrorDataPerSector)[i].lpBufC2NoneSectorBackup);
		}
		OutputString(_T("\n"));
		FreeAndNull(*c2ErrorDataPerSector);
	}
}

VOID TerminateLBAPerTrack(
	PDISC_DATA* pDiscData
	)
{
	FreeAndNull((*pDiscData)->SCSI.lpFirstLBAListOnToc);
	FreeAndNull((*pDiscData)->SCSI.lpLastLBAListOnToc);
}

VOID TerminateTocFullData(
	PEXEC_TYPE pExecType,
	PDEVICE_DATA pDevData,
	PDISC_DATA* pDiscData
	)
{
	size_t dwTrackAllocSize = 
		*pExecType == gd ? 100 : (size_t)(*pDiscData)->SCSI.toc.LastTrack + 1;
	FreeAndNull((*pDiscData)->SCSI.lpSessionNumList);
	if (pDevData->bCanCDText || *pExecType == gd) {
		for (size_t i = 0; i < dwTrackAllocSize; i++) {
			FreeAndNull((*pDiscData)->SUB_CHANNEL.pszISRC[i]);
			FreeAndNull((*pDiscData)->SCSI.pszTitle[i]);
			FreeAndNull((*pDiscData)->SCSI.pszPerformer[i]);
			FreeAndNull((*pDiscData)->SCSI.pszSongWriter[i]);
		}
		FreeAndNull((*pDiscData)->SUB_CHANNEL.pszISRC);
		FreeAndNull((*pDiscData)->SCSI.pszTitle);
		FreeAndNull((*pDiscData)->SCSI.pszPerformer);
		FreeAndNull((*pDiscData)->SCSI.pszSongWriter);
	}
}

#ifndef _DEBUG
VOID TerminateLogFile(
	PEXEC_TYPE pExecType
	)
{
	FcloseAndNull(g_LogFile.fpDisc);
	if (*pExecType != floppy) {
		FcloseAndNull(g_LogFile.fpDrive);
		FcloseAndNull(g_LogFile.fpError);
		FcloseAndNull(g_LogFile.fpInfo);
	}
}
#endif
