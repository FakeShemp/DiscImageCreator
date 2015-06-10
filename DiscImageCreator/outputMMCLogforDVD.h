/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once

VOID OutputFsVolumeDescriptorSequence(
	LPBYTE lpBuf,
	INT nIdx
	);

VOID OutputFsVolumeRecognitionSequence(
	LPBYTE lpBuf,
	INT nIdx
	);

VOID OutputMmcDVDStructureFormat(
	PDISC pDisc,
	BYTE byFormatCode,
	WORD wFormatLength,
	LPBYTE lpFormat,
	LPBYTE lpLayerNum,
	UINT uiNum,
	BOOL bSuccesssReadToc
	);

VOID OutputMmcDVDCopyrightManagementInformation(
	PDVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR dvdCopyright,
	INT nLBA
	);
