/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"

VOID OutputFsVolumeDescriptor(
	LPBYTE lpBuf,
	INT nIdx
	);

VOID OutputFsVolumeRecognitionSequence(
	LPBYTE lpBuf,
	INT nIdx
	);

VOID OutputFsVolumeDescriptorSequence(
	LPBYTE lpBuf,
	INT nIdx
	);

VOID OutputFsMasterDirectoryBlocks(
	LPBYTE lpBuf,
	INT nIdx
	);

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
	DWORD dwAllLen,
	DWORD dwSize
	);

VOID OutputMmcFeatureProfileType(
	WORD wFeatureProfileType
	);

VOID OutputMmcInquiryData(
	PDEVICE_DATA pDevData,
	PINQUIRYDATA pInquiry
	);

VOID OutputMmcTocWithPregap(
	PDISC_DATA pDiscData,
	LPBYTE lpCtlList,
	LPBYTE lpModeList,
	LPINT* lpLBAStartList
	);

VOID OutputMmcCDC2Error296(
	LPBYTE lpBuf,
	INT nLBA
	);

VOID OutputMmcCDMain2352(
	LPBYTE lpBuf,
	INT nLBA
	);

VOID OutputMmcCDSub96Align(
	LPBYTE lpBuf,
	INT nLBA
	);

VOID OutputMmcCDSub96Raw(
	LPBYTE lpBuf,
	INT nLBA
	);

VOID OutputMmcCDSubToLog(
	PDISC_DATA pDiscData,
	LPBYTE lpSubcode,
	LPBYTE lpSubcodeOrg,
	INT nLBA,
	INT nTrackNum,
	FILE* fpParse
	);

VOID OutputMmcDVDStructureFormat(
	PDISC_DATA pDiscData,
	LPBYTE lpStructure,
	LPWORD lpStructureLength,
	LPBYTE lpLayerNum,
	LPBYTE lpFormat, 
	size_t uiFormatCode,
	INT nNum
	);

VOID OutputMmcDVDCopyrightManagementInformation(
	PDVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR dvdCopyright,
	INT nLBA,
	INT nIdx
	);
