/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "enum.h"

BOOL PreserveTrackAttribution(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	INT nLBA,
	LPBYTE lpCurrentTrackNum,
	PMAIN_HEADER pMainHeader,
	PSUB_Q pSubQ,
	PSUB_Q pPrevSubQ
	);

VOID SetAndOutputC2ErrorDataPerSector(
#if 0
	PC2_ERROR pC2Error,
#endif
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

VOID SetAndOutputToc(
	PEXEC_TYPE pExecType,
	PDISC pDisc
	);

VOID SetAndOutputTocFull(
	PDISC pDisc,
	PCDROM_TOC_FULL_TOC_DATA fullToc,
	PCDROM_TOC_FULL_TOC_DATA_BLOCK pTocData,
	size_t uiTocEntries,
	FILE* fpCcd
	);

VOID SetAndOutputTocCDText(
	PDISC pDisc,
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	PCHAR pTmpText,
	WORD wTocTextEntries,
	WORD wAllTextSize
	);

VOID SetAndOutputTocCDWText(
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	PCHAR pTmpText,
	WORD wFirstEntries,
	WORD wTocTextEntries,
	WORD wAllTextSize
	);

VOID SetFeatureCdRead(
	PFEATURE_DATA_CD_READ pCDRead,
	PDEVICE pDevice
	);

VOID SetCDOffset(
	PDISC pDisc,
	INT nStartLBA,
	INT nEndLBA
	);

VOID SetCDTransfer(
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
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	CDB::_READ_CD* cdb,
	READ_CD_FLAG::EXPECTED_SECTOR_TYPE type,
	UINT uiTransferLen,
	READ_CD_FLAG::SUB_CHANNEL_SELECTION Sub,
	BOOL bCheckReading
	);

VOID SetReadD8Command(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	CDB::_PLXTR_READ_CDDA* cdb,
	UINT uiTransferLen,
	BOOL bSub,
	BOOL bCheckReading
	);

VOID SetSubQDataFromReadCD(
	WORD wDriveBufSize,
	UINT uiMainDataSlideSize,
	PMAIN_HEADER pMainHeader,
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

VOID UpdateTmpMainHeader(
	PMAIN_HEADER pMainHeader
	);
