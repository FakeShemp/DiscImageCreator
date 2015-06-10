/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"
#include "enum.h"

BOOL InitC2ErrorData(
	PEXT_ARG pExtArg,
	PDISC_DATA pDiscData,
	PC2_ERROR_DATA c2ErrorData,
	PC2_ERROR_DATA_PER_SECTOR* c2ErrorDataPerSector,
	DWORD dwAllBufLen
	);

BOOL InitLBAPerTrack(
	PEXEC_TYPE pExecType,
	PDISC_DATA* pDiscData
	);

BOOL InitTocFullData(
	PEXEC_TYPE pExecType,
	PDEVICE_DATA pDevData,
	PDISC_DATA* pDiscData
	);

#ifndef _DEBUG
BOOL InitLogFile(
	PEXEC_TYPE pExecType,
	_TCHAR* szFullPath
	);
#endif

VOID Terminatec2ErrorDataPerSector(
	PDEVICE_DATA pDevData,
	PEXT_ARG pExtArg,
	PC2_ERROR_DATA_PER_SECTOR* c2ErrorDataPerSector
	);

VOID TerminateLBAPerTrack(
	PDISC_DATA* pDiscData
	);

VOID TerminateTocFullData(
	PEXEC_TYPE pExecType,
	PDEVICE_DATA pDevData,
	PDISC_DATA* pDiscData
	);

#ifndef _DEBUG
VOID TerminateLogFile(
	PEXEC_TYPE pExecType
	);
#endif
