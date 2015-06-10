/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"

BOOL CheckMainChannel(
	PDISC_DATA pDiscData,
	LPBYTE lpBuf,
	PSUB_Q_DATA pNextSubQ,
	PSUB_Q_DATA pSubQ,
	PSUB_Q_DATA pPrevSubQ,
	PSUB_Q_DATA pPrevPrevSubQ,
	BYTE CurrentTrackNum,
	INT nLBA
	);

BOOL CheckLibCryptSector(
	BOOL bLibCrypt,
	INT nLBA
	);

BOOL CheckAndFixSubChannel(
	PEXT_ARG pExtArg,
	PDISC_DATA pDiscData,
	LPBYTE lpSubcode,
	PSUB_Q_DATA pNextSubQ,
	PSUB_Q_DATA pSubQ,
	PSUB_Q_DATA pPrevSubQ,
	PSUB_Q_DATA pPrevPrevSubQ,
	LPBYTE lpCurrentTrackNum,
	LPBOOL lpCatalog,
	INT nLBA,
	BOOL bLibCrypt
	);

BOOL CheckC2Error(
	PC2_ERROR_DATA pC2ErrorData,
	PC2_ERROR_DATA_PER_SECTOR pC2ErrorDataPerSector,
	PDEVICE_DATA pTransferData,
	LPBYTE lpBuf,
	UINT uiC2ErrorLBACnt
	);

BOOL CheckByteError(
	PC2_ERROR_DATA_PER_SECTOR pC2ErrorDataPerSector,
	LPBYTE lpBuf,
	UINT i
	);

BOOL IsValid3doDataHeader(
	LPBYTE lpBuf
	);

BOOL IsValidDataHeader(
	LPBYTE lpBuf
	);

BOOL IsValidMacDataHeader(
	LPBYTE lpBuf
	);

BOOL IsValidPlextorDrive(
	PDEVICE_DATA pDevData
	);

BOOL IsValidSubQMcn(
	LPBYTE lpSubcode
	);
