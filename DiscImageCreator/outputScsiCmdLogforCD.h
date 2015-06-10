/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "enum.h"

VOID OutputFsVolumeDescriptor(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	INT nLBA
	);

VOID OutputFsVolumeDescriptorForISO9660(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf
	);

VOID OutputFsVolumeDescriptorForJoliet(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf
	);

VOID OutputFsDirectoryRecord(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	INT nDataLen,
	LPSTR fname
	);

VOID OutputFsPathTableRecord(
	LPBYTE lpBuf,
	INT nLBA,
	INT nPathTblSize,
	LPINT pDirTblPosList,
	LPSTR* pDirTblNameList,
	LPINT nDirPosNum
	);

VOID OutputFsMasterDirectoryBlocks(
	LPBYTE lpBuf,
	INT nLBA
	);

VOID OutputFs3doHeader(
	LPBYTE lpBuf,
	INT nLBA
	);

VOID OutputFs3doDirectoryRecord(
	LPBYTE lpBuf,
	INT nLBA,
	PCHAR path,
	LONG directorySize
	);

VOID OutputFsPceStuff(
	LPBYTE lpBuf,
	INT nLBA
	);

VOID OutputFsPceBootSector(
	LPBYTE lpBuf,
	INT nLBA
	);

VOID OutputTocForGD(
	PDISC pDisc
	);

VOID OutputTocWithPregap(
	PDISC pDisc
	);

VOID OutputCDOffset(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	BOOL bGetDrive,
	INT nDriveSampleOffset,
	INT nDriveOffset
	);

VOID OutputCDC2Error296(
	LOG_TYPE type,
	LPBYTE lpBuf,
	INT nLBA
	);

VOID OutputCDMain2352(
	LPBYTE lpBuf,
	INT nLBA
	);

VOID OutputCDSub96Align(
	LPBYTE lpBuf,
	INT nLBA
	);

VOID OutputCDSub96Raw(
	LOG_TYPE type,
	LPBYTE lpBuf,
	INT nLBA
	);

VOID OutputCDSubToLog(
	PDISC pDisc,
	LPBYTE lpSubcode,
	LPBYTE lpSubcodeRaw,
	INT nLBA,
	INT byTrackNum,
	FILE* fpParse
	);
