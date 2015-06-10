/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"

#define DVD_RAW_READ	(2064)

BOOL ReadDVD(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	PEXT_ARG pExtArg,
	LPCTSTR pszPath
	);

BOOL ReadDVDRaw(
	PDEVICE_DATA pDevData,
	LPCSTR szVendorId,
	LPCTSTR pszPath
	);

BOOL ReadDVDStructure(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData
	);
