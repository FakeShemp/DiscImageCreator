/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "enum.h"

BOOL GetAlignedCallocatedBuffer(
	PDEVICE pDevice,
	LPBYTE* ppSrcBuf,
	DWORD dwSize,
	LPBYTE* ppOutBuf,
	LPCTSTR pszFuncName,
	LONG lLineNum
	);

BOOL GetAlignedReallocatedBuffer(
	PDEVICE pDevice,
	LPBYTE* ppSrcBuf,
	DWORD dwSize,
	BYTE byTransferLen,
	LPBYTE* ppOutBuf,
	LPCTSTR pszFuncName,
	LONG lLineNum
	);

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
	BYTE byPrevMode,
	BYTE byCtl,
	INT nType
	);

BOOL GetWriteOffset(
	PDISC pDisc,
	LPBYTE lpBuf
	);

BOOL GetEccEdcCmd(
	LPTSTR pszStr,
	size_t cmdSize,
	LPCTSTR pszCmd,
	LPCTSTR pszImgPath,
	INT nStartLBA,
	INT nEndLBA
	);
