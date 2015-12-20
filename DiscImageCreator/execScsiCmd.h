/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once

#define DRIVE_MAX_SPEED	(72)

BOOL TestUnitReady(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
	);

BOOL Inquiry(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
	);

BOOL StartStopUnit(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	BYTE Start,
	BYTE LoadEject
	);

BOOL ReadTOC(
	PEXT_ARG pExtArg,
	PEXEC_TYPE pExecType,
	PDEVICE pDevice,
	PDISC pDisc
	);

BOOL ReadTOCFull(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	FILE* fpCcd
	);

BOOL ReadTOCText(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	FILE* fpCcd
	);

BOOL GetConfiguration(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	);

BOOL ReadDiscInformation(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
	);

BOOL ModeSense10(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
	);

BOOL ReadBufferCapacity(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
	);

BOOL SetCDSpeed(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	DWORD dwCDSpeedNum
	);

// feature PLEXTOR drive below
BOOL SetSpeedRead(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	BOOL bState
	);

BOOL Reset(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
	);

BOOL ReadEeprom(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
	);
