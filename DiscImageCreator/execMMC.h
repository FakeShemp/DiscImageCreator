/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"
#include "enum.h"

#define DRIVE_MAX_SPEED	(72)

BOOL GetConfiguration(
	PDEVICE pDevice,
	PDISC pDisc
	);

BOOL Inquiry(
	PDEVICE pDevice
	);

BOOL ModeSense10(
	PDEVICE pDevice
	);

BOOL ReadBufferCapacity(
	PDEVICE pDevice
	);

BOOL ReadDiscInformation(
	PDEVICE pDevice
	);

BOOL ReadTOC(
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	PDISC pDisc
	);

BOOL ReadTOCFull(
	PDEVICE pDevice,
	PDISC pDisc,
	FILE* fpCcd
	);

BOOL ReadTOCText(
	PDEVICE pDevice,
	PDISC pDisc,
	FILE* fpCcd
	);

BOOL SetCDSpeed(
	PDEVICE pDevice,
	DWORD dwCDSpeedNum
	);

BOOL StartStopUnit(
	PDEVICE pDevice,
    BYTE Start,
    BYTE LoadEject
	);

BOOL TestUnitReady(
	PDEVICE pDevice
	);

// feature Plextor drive below
BOOL Reset(
	PDEVICE pDevice
	);

BOOL ReadEeprom(
	PDEVICE pDevice
	);

BOOL SetSpeedRead(
	PDEVICE pDevice,
	BOOL bState
	);
