/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "enum.h"

BOOL IsValidMainDataHeader(
	LPBYTE lpBuf
	);

BOOL IsValid3doDataHeader(
	LPBYTE lpBuf
	);

BOOL IsValidMacDataHeader(
	LPBYTE lpBuf
	);

BOOL IsValidPlextorDrive(
	PDEVICE pDevice
	);

BOOL IsValidLibCryptSector(
	BOOL bLibCrypt,
	INT nLBA
	);

BOOL IsValidSubQMCN(
	LPBYTE lpSubcode
	);

BOOL IsValidSubQISRC(
	LPBYTE lpSubcode
	);

VOID CheckMainChannel(
	PDISC pDisc,
	LPBYTE lpBuf,
	PSUB_Q pSubQ,
	PSUB_Q pPrevSubQ,
	PSUB_Q pPrevPrevSubQ,
	BYTE byCurrentTrackNum,
	INT nLBA
	);

VOID CheckAndFixSubChannel(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpSubcode,
	PSUB_Q pNextSubQ,
	PSUB_Q pSubQ,
	PSUB_Q pPrevSubQ,
	PSUB_Q pPrevPrevSubQ,
	BYTE byCurrentTrackNum,
	INT nLBA,
	BOOL bLibCrypt
	);

BOOL ContainsC2Error(
	PC2_ERROR pC2Error,
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	PDEVICE pTransferData,
	LPBYTE lpBuf,
	UINT uiC2ErrorLBACnt
	);

BOOL ContainsDiffByte(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	LPBYTE lpBuf,
	UINT i
	);
