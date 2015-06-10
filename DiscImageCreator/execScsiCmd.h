/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once

#define DRIVE_MAX_SPEED	(72)

BOOL TestUnitReady(
	PDEVICE pDevice
	);

BOOL Inquiry(
	PDEVICE pDevice
	);

BOOL StartStopUnit(
	PDEVICE pDevice,
	BYTE Start,
	BYTE LoadEject
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

BOOL GetConfiguration(
	PDEVICE pDevice,
	PDISC pDisc
	);

BOOL ReadDiscInformation(
	PDEVICE pDevice
	);

BOOL ModeSense10(
	PDEVICE pDevice
	);

BOOL ReadBufferCapacity(
	PDEVICE pDevice
	);

BOOL SetCDSpeed(
	PDEVICE pDevice,
	DWORD dwCDSpeedNum
	);

// feature PLEXTOR drive below
BOOL SetSpeedRead(
	PDEVICE pDevice,
	BOOL bState
	);

BOOL Reset(
	PDEVICE pDevice
	);

BOOL ReadEeprom(
	PDEVICE pDevice
	);
