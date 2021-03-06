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

BOOL IsValidPceSector(
	LPBYTE lpBuf
	);

BOOL IsValidPcfxSector(
	LPBYTE lpBuf
	);

BOOL IsValidPlextorDrive(
	PEXT_ARG pExtArg,
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
	BYTE byCurrentTrackNum,
	INT nLBA
	);

VOID CheckAndFixSubChannel(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpSubcode,
	PSUB_Q pSubQ,
	BYTE byCurrentTrackNum,
	INT nLBA,
	BOOL bLibCrypt
	);

BOOL ContainsC2Error(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE lpBuf,
	UINT uiC2ErrorLBACnt
	);

BOOL ContainsDiffByte(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	LPBYTE lpBuf,
	UINT i
	);

BOOL SupportIndex0InTrack1(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	);
