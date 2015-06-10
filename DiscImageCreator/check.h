/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"

BOOL CheckMainChannel(
	PDISC pDisc,
	LPBYTE lpBuf,
	PSUB_Q pSubQ,
	PSUB_Q pPrevSubQ,
	PSUB_Q pPrevPrevSubQ,
	BYTE CurrentTrackNum,
	INT nLBA
	);

BOOL CheckLibCryptSector(
	BOOL bLibCrypt,
	INT nLBA
	);

BOOL CheckAndFixSubChannel(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpSubcode,
	PSUB_Q pNextSubQ,
	PSUB_Q pSubQ,
	PSUB_Q pPrevSubQ,
	PSUB_Q pPrevPrevSubQ,
	LPBYTE lpCurrentTrackNum,
	LPBOOL lpCatalog,
	INT nLBA,
	BOOL bLibCrypt
	);

BOOL CheckC2Error(
	PC2_ERROR pC2Error,
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	PDEVICE pTransferData,
	LPBYTE lpBuf,
	UINT uiC2ErrorLBACnt
	);

BOOL CheckByteError(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
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
	PDEVICE pDevice
	);

BOOL IsValidSubQMcn(
	LPBYTE lpSubcode
	);
