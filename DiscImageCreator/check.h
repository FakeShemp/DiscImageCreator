/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"

BOOL CheckAndFixSubchannel(
	PEXT_ARG pExtArg,
	PDISC_DATA pDiscData,
	LPBYTE lpSubcode,
	PSUB_Q_DATA pNextSubQ,
	PSUB_Q_DATA pSubQ,
	PSUB_Q_DATA pPrevSubQ,
	PSUB_Q_DATA pPrevPrevSubQ,
	LPBYTE lpCurrentTrackNum,
	LPBOOL lpCatalog,
	LPBOOL lpISRCList,
	LPBYTE lpEndCtlList,
	LPINT* lpLBAStartList,
	LPINT* lpLBAOfDataTrackList,
	INT nLBA
	);

BOOL CheckC2Error(
	PDISC_DATA pDiscData,
	PC2_ERROR_DATA pC2ErrorData,
	PREAD_CD_TRANSFER_DATA pTransferData,
	LPBYTE lpBuf,
	UINT uiC2ErrorLBACnt
	);

BOOL CheckByteError(
	PC2_ERROR_DATA pC2ErrorData,
	LPBYTE lpBuf,
	DWORD dwBufLen,
	INT nSlideLBANum,
	UINT i
	);

BOOL Is3DOData(
	LPBYTE lpSrc
	);

BOOL IsMacData(
	LPBYTE lpSrc
	);

BOOL IsPlextorDrive(
	PDEVICE_DATA pDevData
	);

BOOL IsValidDataHeader(
	LPBYTE lpSrc
	);

BOOL IsValidMCN(
	LPBYTE lpSubcode
	);
