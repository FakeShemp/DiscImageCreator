/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"
#include "enum.h"

BOOL PreserveTrackAttribution(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	INT nLBA,
	LPBYTE lpCurrentTrackNum,
	PSUB_Q pSubQ,
	PSUB_Q pPrevSubQ
	);

VOID SetAndOutputc2ErrorDataPerSector(
	PC2_ERROR pC2Error,
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	INT nLBA,
	DWORD dwAllBufLen,
	UINT uiC2ErrorLBACnt
	);

VOID SetAndOutputC2NoErrorData(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	LPBYTE lpBuf,
	INT nLBA,
	DWORD dwAllBufLen,
	UINT uiC2ErrorLBACnt
	);

VOID SetAndOutputC2NoErrorByteErrorData(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	LPBYTE lpBuf,
	INT nLBA,
	DWORD dwAllBufLen,
	UINT uiC2ErrorLBACnt
	);

VOID SetC2ErrorBackup(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	UINT uiC2ErrorLBACntBackup,
	DWORD dwAllBufLen
	);

VOID SetAndOutputMmcToc(
	PEXEC_TYPE pExecType,
	PDISC pDisc
	);

VOID SetAndOutputMmcTocFull(
	PDISC pDisc,
	PCDROM_TOC_FULL_TOC_DATA fullToc,
	PCDROM_TOC_FULL_TOC_DATA_BLOCK pTocData,
	size_t uiTocEntries,
	FILE* fpCcd
	);

VOID SetAndOutputMmcTocCDText(
	PDISC pDisc,
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	PCHAR pTmpText,
	WORD wTocTextEntries,
	WORD wAllTextSize
	);

VOID SetAndOutputMmcTocCDWText(
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	PCHAR pTmpText,
	WORD wFirstEntries,
	WORD wTocTextEntries,
	WORD wAllTextSize
	);

VOID SetMmcFeatureCdRead(
	PFEATURE_DATA_CD_READ pCDRead,
	PDEVICE pDevice
	);

VOID SetCDOffsetData(
	PDISC pDisc,
	INT nStartLBA,
	INT nEndLBA
	);

VOID SetCDTransferData(
	PDEVICE pDevice,
	DRIVE_DATA_ORDER order
	);

VOID SetISRCToString(
	PDISC pDisc,
	LPBYTE lpSubcode,
	LPSTR pszOutString,
	BYTE byTrackNum,
	BOOL bCopy
	);

VOID SetMCNToString(
	PDISC pDisc,
	LPBYTE lpSubcode,
	LPSTR pszOutString,
	BOOL bCopy
	);

VOID SetReadCDCommand(
	PDEVICE pDevice,
	PEXT_ARG pExtArg,
	CDB::_READ_CD* cdb,
	READ_CD_FLAG::EXPECTED_SECTOR_TYPE type,
	UINT uiTransferLen,
	READ_CD_FLAG::SUB_CHANNEL_SELECTION Sub,
	BOOL bCheckReading
	);

VOID SetReadD8Command(
	PDEVICE pDevice,
	PEXT_ARG pExtArg,
	CDB::_PLXTR_READ_CDDA* cdb,
	UINT uiTransferLen,
	BOOL bSub,
	BOOL bCheckReading
	);

VOID SetSubQDataFromReadCD(
	PDISC pDisc,
	PSUB_Q pSubQ,
	LPBYTE lpBuf,
	LPBYTE lpSubcode
	);

BOOL UpdateSubQData(
	PSUB_Q pSubQ,
	PSUB_Q pPrevSubQ,
	PSUB_Q pPrevPrevSubQ,
	BOOL bLibCrypt
	);
