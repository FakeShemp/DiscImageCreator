/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once

BOOL ReadCDForSearchingOffset(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	);

BOOL ReadCDForCheckingReadInOut(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PMAIN_HEADER pMain,
	LPCTSTR pszPath,
	READ_CD_FLAG::EXPECTED_SECTOR_TYPE flg
	);

BOOL ReadCDForCheckingCommand(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	);

BOOL ReadCDForCheckingCDG(
	PDEVICE pDevice,
	PDISC pDisc
	);

BOOL ReadCDForFileSystem(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	);

BOOL ReadCDAll(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PMAIN_HEADER pMain,
	LPCTSTR pszPath,
	FILE* fpCcd
	);

BOOL ReadCDPartial(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PMAIN_HEADER pMain,
	LPCTSTR pszPath,
	INT nStart,
	INT nEnd,
	READ_CD_FLAG::EXPECTED_SECTOR_TYPE flg,
	BOOL bCheckReading
	);

BOOL ReadCDForGDTOC(
	PDEVICE pDevice,
	PDISC pDisc
	);
