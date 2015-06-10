/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"

VOID OutputIoctlInfoScsiStatus(
	PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER swb,
	LPBYTE byScsiStatus,
	LPCTSTR pszFuncName,
	LONG lLineNum
	);

VOID OutputIoctlScsiAddress(
	PDEVICE_DATA pDevData
	);

VOID OutputIoctlStorageAdaptorDescriptor(
	PSTORAGE_ADAPTER_DESCRIPTOR pAdapterDescriptor
	);

VOID OutputIoctlFloppyInfo(
	PDISK_GEOMETRY pGeom
	);
