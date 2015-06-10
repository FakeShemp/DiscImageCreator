/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"
#include "enum.h"

BOOL GetCreatedFileList(
	PHANDLE h,
	PWIN32_FIND_DATA lp,
	LPTSTR szPathWithoutFileName,
	size_t szPathSize
	);

BOOL GetDriveOffset(
	LPCSTR szProductId,
	LPINT lpDriveOffset
	);

DWORD GetFileSize(
	LONG lOffset,
	FILE *fp
	);

UINT64 GetFileSize64(
	INT64 n64Offset,
	FILE *fp
	);

BYTE GetMode(
	LPBYTE lpBuf,
	BYTE byCtl
	);

BOOL GetWriteOffset(
	PDISC_DATA pDiscData,
	LPBYTE lpBuf
	);

BOOL GetEccEdcCheckCmd(
	LPTSTR pszCmd,
	size_t cmdSize,
	LPCTSTR pszImgPath
	);
