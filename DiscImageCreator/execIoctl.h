/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"

BOOL DiskGetMediaTypes(
	PDEVICE_DATA pDevData,
	LPCTSTR pszPath
	);

BOOL ScsiGetAddress(
	PDEVICE_DATA pDevData
	);

BOOL ScsiPassThroughDirect(
	PDEVICE_DATA pDevData,
	PVOID lpCdbCmd,
	BYTE byCdbCmdLength,
	PVOID pvBuffer,
	DWORD dwBufferLength,
	LPBYTE byScsiStatus,
	LPCTSTR pszFuncName,
	LONG lLineNum
	);

BOOL StorageQueryProperty(
	PDEVICE_DATA pDevData
	);
