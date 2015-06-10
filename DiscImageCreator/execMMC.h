/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"
#include "enum.h"

#define DRIVE_MAX_SPEED	(72)

BOOL GetConfiguration(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData
	);

BOOL Inquiry(
	PDEVICE_DATA pDevData
	);

BOOL ReadBufferCapacity(
	PDEVICE_DATA pDevData
	);

BOOL ReadDiscInformation(
	PDEVICE_DATA pDevData
	);

BOOL ReadTOC(
	PEXEC_TYPE pExecType,
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData
	);

BOOL ReadTOCFull(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	FILE* fpCcd
	);

BOOL ReadTOCText(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	FILE* fpCcd
	);

BOOL SetCDSpeed(
	PDEVICE_DATA pDevData,
	UINT uiCDSpeedNum
	);

BOOL StartStopUnit(
	PDEVICE_DATA pDevData,
    BYTE Start,
    BYTE LoadEject
	);

BOOL TestUnitReady(
	PDEVICE_DATA pDevData
	);
