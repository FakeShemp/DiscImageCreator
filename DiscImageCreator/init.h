/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "enum.h"

BOOL InitC2ErrorData(
	PEXT_ARG pExtArg,
	PC2_ERROR_PER_SECTOR* pC2ErrorPerSector,
	DWORD dwAllBufLen
	);

BOOL InitLBAPerTrack(
	PEXEC_TYPE pExecType,
	PDISC* pDisc
	);

BOOL InitTocFullData(
	PEXEC_TYPE pExecType,
	PDISC* pDisc
	);

BOOL InitTocTextData(
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	PDISC* pDisc
	);

VOID InitMainDataHeader(
	PEXT_ARG pExtArg,
	PMAIN_HEADER pMain
	);

BOOL InitSubData(
	PEXEC_TYPE pExecType,
	PDISC* pDisc
	);

#ifndef _DEBUG
BOOL InitLogFile(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	_TCHAR* szFullPath
	);
#endif

VOID TerminateC2ErrorData(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PC2_ERROR_PER_SECTOR* pC2ErrorPerSector
	);

VOID TerminateLBAPerTrack(
	PDISC* pDisc
	);

VOID TerminateTocFullData(
	PDISC* pDisc
	);

VOID TerminateTocTextData(
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	PDISC* pDisc
	);

VOID TerminateSubData(
	PEXEC_TYPE pExecType,
	PDISC* pDisc
	);

#ifndef _DEBUG
VOID TerminateLogFile(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg
	);
#endif
