/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"
#include "struct.h"
#include "check.h"
#include "convert.h"
#include "get.h"
#include "output.h"

// These global variable is set at pringAndSetArg().
extern _TCHAR g_szCurrentdir[_MAX_PATH];
extern _TCHAR g_drive[_MAX_DRIVE];
extern _TCHAR g_dir[_MAX_DIR];

BOOL GetCreatedFileList(
	PHANDLE h,
	PWIN32_FIND_DATA lp,
	LPTSTR szPathWithoutFileName
	)
{
	_TCHAR drive[_MAX_DRIVE] = { 0 };
	_TCHAR dir[_MAX_DIR] = { 0 };
	if (g_drive[0] == 0 || g_dir[0] == 0) {
		_TCHAR szTmpPath[_MAX_PATH] = { 0 };
		_tcsncpy(szTmpPath, g_szCurrentdir, _MAX_PATH);
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
	_stprintf(szPathWithoutFileName, _T("%s\\%s\\*"), drive, dir);

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
	if (pId != NULL) {
		PCHAR pTrimBuf[10] = { 0 };
		CHAR lpBuf[1024] = { 0 };

		while((fgets(lpBuf, sizeof(lpBuf), fpDrive)) != NULL) {
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
	LPBYTE lpBuf
	)
{
	BYTE byMode = DATA_BLOCK_MODE0;
	if (IsValidDataHeader(lpBuf)) {
		if ((lpBuf[15] & 0x60) == 0x60) {
			byMode = BcdToDec((BYTE)(lpBuf[15] ^ 0x60));
		}
		else {
			byMode = BcdToDec(lpBuf[15]);
		}
	}
	return byMode;
}

BOOL GetWriteOffset(
	PDISC_DATA pDiscData,
	LPBYTE lpBuf
	)
{
	BOOL bRet = FALSE;
	for (INT i = 0; i < CD_RAW_SECTOR_SIZE * 2; i++) {
		if (IsValidDataHeader(lpBuf + i)) {
			BYTE sm = BcdToDec((BYTE)(lpBuf[i + 12] ^ 0x01));
			BYTE ss = BcdToDec((BYTE)(lpBuf[i + 13] ^ 0x80));
			BYTE sf = BcdToDec((BYTE)(lpBuf[i + 14]));
			INT tmpLBA = MSFtoLBA(sf, ss, sm) - 150;
			pDiscData->nCombinedOffset = 
				CD_RAW_SECTOR_SIZE * -(tmpLBA - pDiscData->nFirstDataLBA) + i;
			bRet = TRUE;
			break;
		}
	}
	return bRet;
}
