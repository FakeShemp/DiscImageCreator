/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "enum.h"

VOID OutputFs3doStructure(
	LPBYTE lpBuf,
	INT nLBA
	);

VOID OutputFsVolumeDescriptor(
	LPBYTE lpBuf,
	INT nIdx,
	INT nLBA
	);

VOID OutputFsPrimaryVolumeDescriptorForISO9660(
	LPBYTE lpBuf,
	INT nIdx
	);

VOID OutputFsPrimaryVolumeDescriptorForJoliet(
	LPBYTE lpBuf,
	INT nIdx
	);

VOID OutputFsMasterDirectoryBlocks(
	LPBYTE lpBuf,
	INT nIdx,
	INT nLBA
	);

VOID OutputFsPceStuff(
	LPBYTE lpBuf,
	INT nLBA
	);

VOID OutputFsPceBootSector(
	LPBYTE lpBuf,
	INT nLBA
	);

VOID OutputMmcTocWithPregap(
	PDISC pDisc
	);

VOID OutputMmcCDOffset(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	BOOL bGetDrive,
	INT nDriveSampleOffset,
	INT nDriveOffset
	);

VOID OutputMmcCDC2Error296(
	LOG_TYPE type,
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
	LOG_TYPE type,
	LPBYTE lpBuf,
	INT nLBA
	);

VOID OutputMmcCDSubToLog(
	PDISC pDisc,
	LPBYTE lpSubcode,
	LPBYTE lpSubcodeRaw,
	INT nLBA,
	INT byTrackNum,
	FILE* fpParse
	);
