/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"
#include "enum.h"

#define SYNC_SIZE		(12)
#define HEADER_SIZE		(4)
#define SUBHEADER_SIZE	(8)

BOOL ReadCDAll(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszPath,
	FILE* fpCcd
	);

BOOL ReadCDForCheckingSubPtoW(
	PDEVICE pDevice,
	PEXEC_TYPE pExecType
	);

BOOL ReadCDForCheckingCDG(
	PDEVICE pDevice,
	PDISC pDisc
	);

BOOL ReadCDForCheckingIndex0InTrack1(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	);

BOOL ReadCDForCheckingReadInOut(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszPath,
	READ_CD_FLAG::EXPECTED_SECTOR_TYPE flg
	);

BOOL ReadCDForSearchingOffset(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	);

BOOL ReadCDForVolumeDescriptor(
	PDEVICE pDevice,
	PCDROM_TOC pToc,
	INT nFirstLBAofDataTrack
	);

BOOL ReadCDPartial(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszPath,
	INT nStart,
	INT nEnd,
	READ_CD_FLAG::EXPECTED_SECTOR_TYPE flg,
	BOOL bCheckReading
	);

BOOL ReadCDForFlushingDriveCache(
	PDEVICE pDevice,
	INT nLBA
	);
