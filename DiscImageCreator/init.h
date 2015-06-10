/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"
#include "enum.h"

BOOL InitC2ErrorData(
	PEXT_ARG pExtArg,
	PC2_ERROR_DATA* c2ErrorData,
	DWORD dwAllBufLen
	);

BOOL InitTocData(
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

VOID TerminateC2ErrorData(
	PDEVICE_DATA pDevData,
	PEXT_ARG pExtArg,
	PC2_ERROR_DATA* c2ErrorData
	);

VOID TerminateTocData(
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
