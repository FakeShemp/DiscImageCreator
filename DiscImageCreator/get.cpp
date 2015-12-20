/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "check.h"
#include "convert.h"
#include "get.h"
#include "output.h"

// These global variable is set at printAndSetArg().
extern _TCHAR g_szCurrentdir[_MAX_PATH];
extern _TCHAR g_drive[_MAX_DRIVE];
extern _TCHAR g_dir[_MAX_DIR];

BOOL GetAlignedCallocatedBuffer(
	PDEVICE pDevice,
	LPBYTE* ppSrcBuf,
	DWORD dwSize,
	LPBYTE* ppOutBuf,
	LPCTSTR pszFuncName,
	LONG lLineNum
	)
{
	*ppSrcBuf = (LPBYTE)calloc(dwSize + pDevice->AlignmentMask, sizeof(BYTE));
	if (!*ppSrcBuf) {
		OutputLastErrorNumAndString(pszFuncName, lLineNum);
		return FALSE;
	}
	*ppOutBuf = (LPBYTE)ConvParagraphBoundary(pDevice, *ppSrcBuf);
	return TRUE;
}

BOOL GetAlignedReallocatedBuffer(
	PDEVICE pDevice,
	LPBYTE* ppSrcBuf,
	DWORD dwSize,
	BYTE byTransferLen,
	LPBYTE* ppOutBuf,
	LPCTSTR pszFuncName,
	LONG lLineNum
	)
{
	LPBYTE pBuf = (LPBYTE)realloc(
		*ppSrcBuf, dwSize * byTransferLen + pDevice->AlignmentMask);
	if (!pBuf) {
		OutputLastErrorNumAndString(pszFuncName, lLineNum);
		return FALSE;
	}
	*ppSrcBuf = pBuf;
	*ppOutBuf = (LPBYTE)ConvParagraphBoundary(pDevice, pBuf);
	return TRUE;
}

BOOL GetCreatedFileList(
	PHANDLE h,
	PWIN32_FIND_DATA lp,
	LPTSTR szPathWithoutFileName,
	size_t szPathSize
	)
{
	_TCHAR drive[_MAX_DRIVE] = { 0 };
	_TCHAR dir[_MAX_DIR] = { 0 };
	if (!g_drive[0] || !g_dir[0]) {
		_TCHAR szTmpPath[_MAX_PATH + 1] = { 0 };
		_tcsncpy(szTmpPath, g_szCurrentdir, _MAX_PATH);
		szTmpPath[_MAX_PATH] = 0;
		if (!PathAddBackslash(szTmpPath)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		_tsplitpath(szTmpPath, drive, dir, NULL, NULL);
	}
	else {
		_tcsncpy(drive, g_drive, _MAX_DRIVE);
		_tcsncpy(dir, g_dir, _MAX_DIR);
	}
	// "*" is wild card
	_sntprintf(szPathWithoutFileName, szPathSize, _T("%s\\%s\\*"), drive, dir);
	szPathWithoutFileName[szPathSize - 1] = 0;

	*h = FindFirstFile(szPathWithoutFileName, lp);
	if (*h == INVALID_HANDLE_VALUE) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("%s\n"), szPathWithoutFileName);
			return FALSE;
	}
	else {
		// delete '*'
		szPathWithoutFileName[_tcslen(szPathWithoutFileName) - 1] = '\0';
	}
	return TRUE;
}

BOOL GetDriveOffset(
	LPCSTR szProductId,
	LPINT lpDriveOffset
	)
{
	BOOL bGetOffset = FALSE;
	FILE* fpDrive = OpenProgrammabledFile(_T("driveOffset.txt"), _T("r"));
	if (!fpDrive) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}

	CHAR szProduct[DRIVE_PRODUCT_ID_SIZE + 1] = { 0 };
	for (INT src = 0, dst = 0; dst < sizeof(szProduct) - 1; dst++) {
		if (szProductId[dst] == ' ' && (szProductId[dst + 1] == ' ' ||
			szProductId[dst + 1] == '\0')) {
			continue;
		}
		szProduct[src++] = szProductId[dst];
	}

	PCHAR pTrimId[5] = { 0 };
	PCHAR pId = NULL;
	pTrimId[0] = strtok(szProduct, " ");
	// get model string (ex. PX-755A)
	for (INT nRoop = 1; nRoop < 5; nRoop++) {
		pTrimId[nRoop] = strtok(NULL, " ");
		if (pTrimId[nRoop] != NULL) {
			pId = pTrimId[nRoop];
		}
		else {
			if (pTrimId[1] == NULL) {
				pId = pTrimId[0];
			}
			break;
		}
	}
	if (pId) {
		PCHAR pTrimBuf[10] = { 0 };
		CHAR lpBuf[1024] = { 0 };

		while ((fgets(lpBuf, sizeof(lpBuf), fpDrive))) {
			pTrimBuf[0] = strtok(lpBuf, " 	"); // space & tab
			for (INT nRoop = 1; nRoop < 10; nRoop++) {
				pTrimBuf[nRoop] = strtok(NULL, " 	"); // space & tab
			}
			if (pTrimBuf[0] == NULL || pTrimBuf[1] == NULL || pTrimBuf[2] == NULL) {
				continue;
			}
			else if (*pTrimBuf[0] == '\n' || (*pTrimBuf[1] != '-' && *pTrimBuf[2] != '-')) {
				continue;
			}
			for (INT nRoop = 0; nRoop < 10 && pTrimBuf[nRoop] != NULL; nRoop++) {
				if (strstr(pTrimBuf[nRoop], pId) != NULL) {
					*lpDriveOffset = atoi(pTrimBuf[nRoop+1]);
					bGetOffset = TRUE;
					break;
				}
			}
			if (bGetOffset) {
				break;
			}
		}
	}
	fclose(fpDrive);
	return bGetOffset;
}

DWORD GetFileSize(
	LONG lOffset,
	FILE *fp
	)
{
	DWORD dwFileSize = 0;
	if (fp != NULL) {
		fseek(fp, 0, SEEK_END);
		dwFileSize = (DWORD)ftell(fp);
		fseek(fp, lOffset, SEEK_SET);
	}
	return dwFileSize;
}

UINT64 GetFileSize64(
	INT64 n64Offset,
	FILE *fp
	)
{
	UINT64 ui64FileSize = 0;
	if (fp != NULL) {
		_fseeki64(fp, 0, SEEK_END);
		ui64FileSize = (UINT64)_ftelli64(fp);
		_fseeki64(fp, n64Offset, SEEK_SET);
	}
	return ui64FileSize;
}

BYTE GetMode(
	LPBYTE lpBuf,
	BYTE byPrevMode,
	BYTE byCtl,
	INT nType
	)
{
	BYTE byMode = byPrevMode;
	if ((byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
		if (IsValidMainDataHeader(lpBuf)) {
			if ((lpBuf[15] & 0x60) == 0x60 && nType == UNSCRAMBLED) {
				byMode = BcdToDec((BYTE)(lpBuf[15] ^ 0x60));
			}
			else {
				byMode = lpBuf[15];
			}
		}
		else {
			if ((byPrevMode & 0x60) == 0x60 && nType == UNSCRAMBLED) {
				byMode = BcdToDec(byPrevMode);
			}
			else {
				byMode = byPrevMode;
			}
		}
	}
	else if ((byCtl & AUDIO_DATA_TRACK) == 0) {
		byMode = DATA_BLOCK_MODE0;
	}
	else {
		if ((byPrevMode & 0x60) == 0x60 && nType == UNSCRAMBLED) {
			byMode = BcdToDec(byPrevMode);
		}
		else {
			byMode = byPrevMode;
		}
	}
	return byMode;
}

BOOL GetWriteOffset(
	PDISC pDisc,
	LPBYTE lpBuf
	)
{
	BOOL bRet = FALSE;
	for (INT i = 0; i < CD_RAW_SECTOR_SIZE * 2; i++) {
		if (IsValidMainDataHeader(lpBuf + i)) {
			BYTE sm = BcdToDec((BYTE)(lpBuf[i + 12] ^ 0x01));
			BYTE ss = BcdToDec((BYTE)(lpBuf[i + 13] ^ 0x80));
			BYTE sf = BcdToDec((BYTE)(lpBuf[i + 14]));
			INT tmpLBA = MSFtoLBA(sm, ss, sf) - 150;
			pDisc->MAIN.nCombinedOffset = 
				CD_RAW_SECTOR_SIZE * -(tmpLBA - pDisc->SCSI.nFirstLBAofDataTrack) + i;
			bRet = TRUE;
			break;
		}
	}
	return bRet;
}

BOOL GetEccEdcCmd(
	LPTSTR pszStr,
	size_t cmdSize,
	LPCTSTR pszCmd,
	LPCTSTR pszImgPath,
	INT nStartLBA,
	INT nEndLBA
	)
{
	_TCHAR path[_MAX_PATH] = { 0 };
	if (!::GetModuleFileName(NULL, path, _MAX_PATH)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = FALSE;
	_TCHAR drive[_MAX_DRIVE] = { 0 };
	_TCHAR dir[_MAX_DIR] = { 0 };
	_tsplitpath(path, drive, dir, NULL, NULL);
	_tmakepath(path, drive, dir, _T("EccEdc"), _T("exe"));
	if (PathFileExists(path)) {
		if (!_tcscmp(pszCmd, _T("check"))) {
			_sntprintf(pszStr, cmdSize,
				_T("\"\"%s\" %s \"%s\"\""), path, pszCmd, pszImgPath);
		}
		else if (!_tcscmp(pszCmd, _T("fix"))) {
			_sntprintf(pszStr, cmdSize,
				_T("\"\"%s\" %s \"%s\"\" %d %d"),
				path, pszCmd, pszImgPath, nStartLBA, nEndLBA);
		}
		bRet = TRUE;
	}
	else {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T(" => %s\n"), path);
	}
	return bRet;
}
