/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"
#include "enum.h"

BOOL PreserveTrackAttribution(
	PEXT_ARG pExtArg,
	PDISC_DATA pDiscData,
	INT nLBA,
	LPBYTE lpCurrentTrackNum,
	PSUB_Q_DATA pSubQ,
	PSUB_Q_DATA pPrevSubQ,
	PSUB_Q_DATA pPrevPrevSubQ
	);

VOID SetAndOutputc2ErrorDataPerSector(
	PC2_ERROR_DATA pC2ErrorData,
	PC2_ERROR_DATA_PER_SECTOR pC2ErrorDataPerSector,
	INT nLBA,
	DWORD dwAllBufLen,
	UINT uiC2ErrorLBACnt
	);

VOID SetAndOutputC2NoErrorData(
	PC2_ERROR_DATA_PER_SECTOR pC2ErrorDataPerSector,
	LPBYTE lpBuf,
	INT nLBA,
	DWORD dwAllBufLen,
	UINT uiC2ErrorLBACnt
	);

VOID SetAndOutputC2NoErrorByteErrorData(
	PC2_ERROR_DATA_PER_SECTOR pC2ErrorDataPerSector,
	LPBYTE lpBuf,
	INT nLBA,
	DWORD dwAllBufLen,
	UINT uiC2ErrorLBACnt
	);

VOID SetC2ErrorBackup(
	PC2_ERROR_DATA_PER_SECTOR pC2ErrorDataPerSector,
	UINT uiC2ErrorLBACntBackup,
	DWORD dwAllBufLen
	);

VOID SetAndOutputMmcToc(
	PEXEC_TYPE pExecType,
	PDISC_DATA pDiscData
	);

VOID SetAndOutputMmcTocFull(
	PDISC_DATA pDiscData,
	PCDROM_TOC_FULL_TOC_DATA fullToc,
	PCDROM_TOC_FULL_TOC_DATA_BLOCK pTocData,
	size_t uiTocEntries,
	FILE* fpCcd
	);

VOID SetAndOutputMmcTocCDText(
	PDISC_DATA pDiscData,
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	PCHAR pTmpText,
	WORD wTocTextEntries,
	WORD wAllTextSize
	);

VOID SetAndOutputMmcTocCDWText(
	PDISC_DATA pDiscData,
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	PCHAR pTmpText,
	WORD wFirstEntries,
	WORD wTocTextEntries,
	WORD wAllTextSize
	);

VOID SetMmcFeatureCdRead(
	PFEATURE_DATA_CD_READ pCDRead,
	PDEVICE_DATA pDevData
	);

VOID SetCDOffsetData(
	PDISC_DATA pDiscData,
	INT nStartLBA,
	INT nEndLBA
	);

VOID SetCDTransferData(
	PDEVICE_DATA pDevData,
	DRIVE_DATA_ORDER order
	);

VOID SetISRCToString(
	PDISC_DATA pDiscData,
	LPBYTE lpSubcode,
	LPTSTR pszOutString,
	BYTE byTrackNum,
	BOOL bCopy
	);

VOID SetMCNToString(
	PDISC_DATA pDiscData,
	LPBYTE lpSubcode,
	LPTSTR pszOutString,
	BOOL bCopy
	);

VOID SetReadCDCommand(
	PDEVICE_DATA pDevData,
	PEXT_ARG pExtArg,
	CDB::_READ_CD* cdb,
	BOOL bSub,
	READ_CD_FLAG::SECTOR_TYPE type,
	UINT uiTransferLen,
	BOOL bCheckReading
	);

VOID SetReadD8Command(
	PDEVICE_DATA pDevData,
	PEXT_ARG pExtArg,
	CDB::_PLXTR_READ_CDDA* cdb,
	UINT uiTransferLen,
	BOOL bSub,
	BOOL bCheckReading
	);

VOID SetSubQDataFromReadCD(
	PDISC_DATA pDiscData,
	PSUB_Q_DATA pSubQ,
	LPBYTE lpBuf,
	LPBYTE lpSubcode
	);

BOOL UpdateSubQData(
	PSUB_Q_DATA pSubQ,
	PSUB_Q_DATA pPrevSubQ,
	PSUB_Q_DATA pPrevPrevSubQ,
	BOOL bLibCrypt
	);
