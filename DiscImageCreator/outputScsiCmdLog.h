/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once

VOID OutputBufferCapacity(
	PREAD_BUFFER_CAPACITY_DATA pReadBufCapaData
	);

VOID OutputDiscInformation(
	PDISC_INFORMATION pDiscInformation
	);

VOID OutputDriveSpeed(
	PCDROM_SET_SPEED pSetspeed
	);

VOID OutputFeatureNumber(
	PDEVICE pDevice,
	LPBYTE lpConf,
	DWORD dwAllLen
	);

VOID OutputFeatureProfileType(
	WORD wFeatureProfileType
	);

VOID OutputInquiry(
	PDEVICE pDevice,
	PINQUIRYDATA pInquiry
	);

VOID OutputModeSense10(
	PDEVICE pDevice,
	PSENSE pModesense
	);

VOID OutputEeprom(
	LPBYTE pBuf,
	DWORD tLen,
	INT nRoop,
	INT nLife
	);
