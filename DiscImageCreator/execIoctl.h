/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"

BOOL DiskGetMediaTypes(
	PDEVICE pDevice,
	LPCTSTR pszPath
	);

BOOL ScsiGetAddress(
	PDEVICE pDevice
	);

BOOL ScsiPassThroughDirect(
	PDEVICE pDevice,
	PVOID lpCdbCmd,
	BYTE byCdbCmdLength,
	PVOID pvBuffer,
	DWORD dwBufferLength,
	LPBYTE byScsiStatus,
	LPCTSTR pszFuncName,
	LONG lLineNum
	);

BOOL StorageQueryProperty(
	PDEVICE pDevice,
	PBOOL pBusTypeUSB
	);
