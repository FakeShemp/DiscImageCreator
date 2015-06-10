/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"

VOID OutputIoctlSenseData(
	PSENSE_DATA pSenseData
	);

VOID OutputIoctlScsiStatus(
	BYTE byScsiStatus
	);

VOID OutputIoctlScsiAddress(
	PDEVICE pDevice
	);

VOID OutputIoctlStorageAdaptorDescriptor(
	PSTORAGE_ADAPTER_DESCRIPTOR pAdapterDescriptor,
	PBOOL pBusTypeUSB
	);

VOID OutputIoctlFloppyInfo(
	PDISK_GEOMETRY pGeom,
	DWORD dwGeomNum
	);
