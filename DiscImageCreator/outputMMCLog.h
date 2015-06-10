/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"

VOID OutputMmcBufferCapacity(
	PREAD_BUFFER_CAPACITY_DATA pReadBufCapaData
	);

VOID OutputMmcDiscInformation(
	PDISC_INFORMATION pDiscInformation
	);

VOID OutputMmcDriveSpeed(
	PCDROM_SET_SPEED pSetspeed
	);

VOID OutputMmcFeatureNumber(
	PDEVICE pDevice,
	LPBYTE lpConf,
	DWORD dwAllLen
	);

VOID OutputMmcFeatureProfileType(
	WORD wFeatureProfileType
	);

VOID OutputMmcInquiryData(
	PDEVICE pDevice,
	PINQUIRYDATA pInquiry
	);

VOID OutputMmcModeSense10(
	PDEVICE pDevice,
	PSENSE pModesense
	);
