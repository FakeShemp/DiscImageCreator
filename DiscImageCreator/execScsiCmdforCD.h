/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once

BOOL ReadCDAll(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PMAIN_HEADER pMainHeader,
	LPCTSTR pszPath,
	FILE* fpCcd
	);

BOOL ReadCDForCheckingSubPtoW(
	PDEVICE pDevice
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
	PMAIN_HEADER pMainHeader,
	LPCTSTR pszPath,
	READ_CD_FLAG::EXPECTED_SECTOR_TYPE flg
	);

BOOL ReadCDForSearchingOffset(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	);

BOOL ReadCDForFileSystem(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	);

BOOL ReadCDPartial(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PMAIN_HEADER pMainHeader,
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

BOOL ReadCDForGDTOC(
	PDEVICE pDevice,
	PDISC pDisc
	);
