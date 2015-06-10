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
	PC2_ERROR pC2Error,
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
	pC2Error->sC2Offset =
		(SHORT)((pDisc->MAIN.nCombinedOffset - CD_RAW_READ_SUBCODE_SIZE) / CHAR_BIT);
#if 0
	if (pDisc->MAIN.nCombinedOffset > 0) {
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
		pC2Error->cSlideSectorNum = (CHAR)(abs(pDisc->MAIN.nAdjustSectorNum));
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
		INT nSlideNum = pDisc->MAIN.nCombinedOffset / CD_RAW_SECTOR_SIZE;
		if (nSlideNum == 0) {
			pC2Error->cSlideSectorNum = -1;
		}
		else {
			pC2Error->cSlideSectorNum = (CHAR)(nSlideNum);
		}
	}
#endif
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
			if (NULL == ((*pC2ErrorPerSector)[n].lpBufC2NoneSector = 
				(LPBYTE)calloc(dwAllBufLen, sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputString(_T("\n"));
				throw FALSE;
			}
			if (NULL == ((*pC2ErrorPerSector)[n].lpBufC2NoneSectorBackup = 
				(LPBYTE)calloc(dwAllBufLen, sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputString(_T("\n"));
				throw FALSE;
			}
			(*pC2ErrorPerSector)[n].bErrorFlag = RETURNED_NO_C2_ERROR_1ST;
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
		if (pDevice->bCanCDText || *pExecType == gd) {
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
	PMAIN_HEADER pMainHeader
	)
{
	memcpy(pMainHeader->header, g_aSyncHeader, sizeof(g_aSyncHeader));
	pMainHeader->header[12] = 0x01;
	pMainHeader->header[13] = 0x82;
	pMainHeader->header[14] = (BYTE)-1;
}

BOOL InitSubData(
	PEXEC_TYPE pExecType,
	PDISC* pDisc
	)
{
	BOOL bRet = TRUE;
	size_t dwTrackAllocSize =
		*pExecType == gd ? 100 : (size_t)(*pDisc)->SCSI.toc.LastTrack + 1;
	(*pDisc)->SUB.bCatalog = FALSE;
	(*pDisc)->SUB.nFirstLBAForMCN = -1;
	(*pDisc)->SUB.nRangeLBAForMCN = -1;
	(*pDisc)->SUB.bISRC = FALSE;
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

VOID TerminateC2ErrorDataPerSector(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PC2_ERROR_PER_SECTOR* pC2ErrorPerSector
	)
{
	if (*pC2ErrorPerSector && pExtArg->bC2 && pDevice->bC2ErrorData) {
		for (UINT i = 0; i < pExtArg->dwMaxC2ErrorNum; i++) {
			OutputString(_T("\rFreeing allocated memory for C2 errors: %u/%u"), 
				i + 1, pExtArg->dwMaxC2ErrorNum);
			FreeAndNull((*pC2ErrorPerSector)[i].lpErrorBytePos);
			FreeAndNull((*pC2ErrorPerSector)[i].lpErrorBytePosBackup);
			FreeAndNull((*pC2ErrorPerSector)[i].lpBufC2NoneSector);
			FreeAndNull((*pC2ErrorPerSector)[i].lpBufC2NoneSectorBackup);
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
	if (pDevice->bCanCDText) {
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
