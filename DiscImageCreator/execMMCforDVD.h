/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"

#define DVD_RAW_READ	(2064)

BOOL ReadDVD(
	PDEVICE pDevice,
	PDISC pDisc,
	PEXT_ARG pExtArg,
	LPCTSTR pszPath
	);

BOOL ReadDVDForCMI(
	PDEVICE pDevice,
	PDISC pDisc
	);

BOOL ReadDVDRaw(
	PDEVICE pDevice,
	LPCSTR szVendorId,
	LPCTSTR pszPath
	);

BOOL ReadDVDStructure(
	PDEVICE pDevice,
	PDISC pDisc
	);
