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
	PDEVICE_DATA pDevData,
	LPBYTE lpConf,
	DWORD dwAllLen
	);

VOID OutputMmcFeatureProfileType(
	WORD wFeatureProfileType
	);

VOID OutputMmcInquiryData(
	PDEVICE_DATA pDevData,
	PINQUIRYDATA pInquiry
	);
