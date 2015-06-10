/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"

BOOL PreserveTrackAttribution(
	PDISC_DATA pDiscData,
	INT nLBA,
	LPBYTE lpCurrentTrackNum,
	PSUB_Q_DATA pSubQ,
	PSUB_Q_DATA pPrevSubQ,
	PSUB_Q_DATA pPrevPrevSubQ,
	LPBYTE lpCtlList,
	LPBYTE lpModeList,
	LPINT* lpLBAStartList,
	LPINT* lpLBAOfDataTrackList
	);

VOID SetAndOutputC2ErrorData(
	PC2_ERROR_DATA c2ErrorData,
	INT nLBA,
	UINT uiC2ErrorLBACnt
	);

VOID SetAndOutputC2NoErrorData(
	PC2_ERROR_DATA c2ErrorData,
	LPBYTE lpBuf,
	INT nLBA,
	DWORD dwAllBufLen,
	UINT uiC2ErrorLBACnt,
	UINT i
	);

VOID SetAndOutputC2NoErrorByteErrorData(
	PC2_ERROR_DATA c2ErrorData,
	LPBYTE lpBuf,
	INT nLBA,
	DWORD dwAllBufLen,
	UINT uiC2ErrorLBACnt,
	UINT i
	);

VOID SetC2ErrorBackup(
	PC2_ERROR_DATA c2ErrorData,
	UINT uiC2ErrorLBACntBackup,
	DWORD dwAllBufLen
	);

VOID SetAndOutputMmcToc(
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
	size_t uiTocTextEntries,
	size_t uiAllTextSize
	);

VOID SetAndOutputMmcTocCDWText(
	PDISC_DATA pDiscData,
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	PCHAR pTmpText,
	size_t uiFirstEntries,
	size_t uiTocTextEntries,
	size_t uiAllTextSize
	);

VOID SetCDOffsetData(
	PDISC_DATA pDiscData,
	PCD_OFFSET_DATA pCdOffsetData,
	INT nStartLBA,
	INT nEndLBA
	);

VOID SetISRCToString(
	PDISC_DATA pDiscData,
	LPBYTE lpSubcode,
	LPTSTR pszOutString,
	INT nTrackNum,
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
	LPBYTE lpCmd,
	BOOL bCDG,
	READ_CD_FLAG::SECTOR_TYPE flg,
	BYTE byTransferLen
	);

VOID SetReadD8Command(
	PDEVICE_DATA pDevData,
	PEXT_ARG pExtArg,
	LPBYTE lpCmd,
	BYTE byTransferLen,
	BOOL bCheckReading
	);

VOID SetSubQDataFromReadCD(
	PSUB_Q_DATA pSubQ,
	PCD_OFFSET_DATA pCdOffsetData,
	LPBYTE lpBuf,
	LPBYTE lpSubcode
	);

BOOL UpdateSubchannelQData(
	PSUB_Q_DATA pSubQ,
	PSUB_Q_DATA pPrevSubQ,
	PSUB_Q_DATA pPrevPrevSubQ
	);
