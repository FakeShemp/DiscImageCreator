/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "enum.h"

VOID PreserveTrackAttribution(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	INT nLBA,
	LPBYTE lpCurrentTrackNum,
	PMAIN_HEADER pMain,
	PSUB_Q pSubQ
	);

VOID SetC2ErrorData(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	INT nLBA,
	PUINT puiC2ErrorLBACnt,
	BOOL b1stRead
	);

VOID SetNoC2ErrorData(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	LPBYTE lpBuf,
	INT nLBA,
	DWORD dwAllBufLen,
	PUINT puiC2ErrorLBACnt
	);

VOID SetNoC2ErrorExistsByteErrorData(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	INT nLBA,
	PUINT puiC2ErrorLBACnt
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
	WORD wTocEntries,
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

VOID SetFeatureRealTimeStreaming(
	PFEATURE_DATA_REAL_TIME_STREAMING pRTS,
	PDEVICE pDevice
	);

VOID SetCDOffset(
	BYTE byBe,
	PDISC pDisc,
	INT nStartLBA,
	INT nEndLBA
	);

VOID SetCDTransfer(
	PDEVICE pDevice,
	DRIVE_DATA_ORDER order
	);

VOID SetFirstAdrSector(
	INT nFirstLBA[][2],
	INT nRangeLBA[][2],
	LPSTR strAdr,
	LPINT nAdrLBAList,
	BYTE bySessionIdx,
	BYTE byPlxtrType
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
	PDEVICE pDevice,
	CDB::_PLXTR_READ_CDDA* cdb,
	UINT uiTransferLen,
	PLXTR_READ_CDDA_FLAG::SUB_CHANNEL_SELECTION Sub
	);

VOID SetTransferLength(
	CDB::_READ12* pCdb,
	DWORD dwSize,
	LPBYTE lpTransferLen
	);

VOID SetSubQDataFromBuffer(
	PSUB_Q_PER_SECTOR pSubQ,
	LPBYTE lpSubcode
	);

VOID SetBufferFromSubQData(
	SUB_Q_PER_SECTOR pSubQ,
	LPBYTE lpSubcode,
	BYTE byPresent
	);

VOID UpdateSubQData(
	PSUB_Q pSubQ,
	BOOL bLibCrypt
	);

VOID UpdateTmpMainHeader(
	PMAIN_HEADER pMain,
	LPBYTE lpBuf,
	BYTE byCtl,
	INT nType
	);
