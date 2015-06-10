/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"
#include "enum.h"

#define SYNC_SIZE				(12)
#define HEADER_SIZE				(4)
#define SUBHEADER_SIZE			(8)

BOOL ReadCDAll(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	LPCTSTR pszPath,
	FILE* fpCcd
	);

BOOL ReadCDForSearchingOffset(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData
	);

BOOL ReadCDPartial(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	LPCTSTR pszPath,
	INT nStart,
	INT nEnd,
	READ_CD_FLAG::SECTOR_TYPE flg,
	BOOL bCheckReading
	);
