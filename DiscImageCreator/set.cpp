/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"
#include "struct.h"
#include "convert.h"
#include "get.h"
#include "output.h"

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
	)
{
	if (pSubQ->byTrackNum > 0) {
		// preserve byMode, ctl
		if (nLBA == pDiscData->pnTocStartLBA[pSubQ->byTrackNum - 1]) {
			lpCtlList[pSubQ->byTrackNum - 1] = pSubQ->byCtl;
			lpModeList[pSubQ->byTrackNum - 1] = pSubQ->byMode;
		}
		// preserve nLBA
		if (pPrevSubQ->byTrackNum + 1 == pSubQ->byTrackNum) {
			if (pSubQ->byIndex > 0) {
				// index 1 is prior to TOC
				if (pSubQ->byIndex == 1 && nLBA != pDiscData->pnTocStartLBA[pSubQ->byTrackNum - 1]) {
					OutputErrorLog(
						_T("Subchannel & TOC isn't sync. Track %2d, LBA on subchannel: %6d, index: %2d, LBA on TOC: %6d\n"),
						pSubQ->byTrackNum, nLBA, pSubQ->byIndex, pDiscData->pnTocStartLBA[pSubQ->byTrackNum - 1]);
					lpLBAStartList[pSubQ->byTrackNum - 1][1] = pDiscData->pnTocStartLBA[pSubQ->byTrackNum - 1];
					if (lpLBAStartList[pSubQ->byTrackNum - 1][0] == lpLBAStartList[pSubQ->byTrackNum - 1][1]) {
						lpLBAStartList[pSubQ->byTrackNum - 1][0] = -1;
					}
				}
				else {
					lpLBAStartList[pSubQ->byTrackNum - 1][pSubQ->byIndex] = nLBA;
				}
			}
			// preserve end lba of data track
			if (pPrevSubQ->byTrackNum > 0) {
				if (lpLBAOfDataTrackList[pPrevSubQ->byTrackNum - 1][0] != -1 &&
					lpLBAOfDataTrackList[pPrevSubQ->byTrackNum - 1][1] == -1 &&
					(pPrevSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					lpLBAOfDataTrackList[pPrevSubQ->byTrackNum - 1][1] = nLBA - 1;
				}
			}
			*lpCurrentTrackNum = pSubQ->byTrackNum;
		}
		// Hyper Wars (Japan)
		// LBA[098484, 0x180B4], Data, Copy NG, TOC[TrackNum-03, Index-00, RelativeTime-00:02:01, AbsoluteTime-21:55:09] RtoW:ZERO byMode
		// LBA[098485, 0x180B5], Data, Copy NG, TOC[TrackNum-03, Index-00, RelativeTime-00:02:00, AbsoluteTime-21:55:10] RtoW:ZERO byMode
		// LBA[098486, 0x180B6], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-03, Index-00, RelativeTime-00:01:74, AbsoluteTime-21:55:11] RtoW:ZERO byMode
		// LBA[098487, 0x180B7], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-03, Index-00, RelativeTime-00:01:73, AbsoluteTime-21:55:12] RtoW:ZERO byMode
		if ((pPrevPrevSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK &&
			(pPrevSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK &&
			(pSubQ->byCtl & AUDIO_DATA_TRACK) == 0 &&
			pSubQ->byIndex == 0 && pPrevSubQ->byIndex == 0 && pPrevPrevSubQ->byIndex == 0) {
			lpLBAOfDataTrackList[pSubQ->byTrackNum - 1][pSubQ->byIndex + 1] = nLBA - 1;
		}
		// preserve index
		if (pPrevSubQ->byIndex + 1 == pSubQ->byIndex && *lpCurrentTrackNum >= 0) {
			if (pSubQ->byIndex != 1) {
				lpLBAStartList[pSubQ->byTrackNum - 1][pSubQ->byIndex] = nLBA;
			}
			else {
				// index 1 is prior to TOC
				if (nLBA != pDiscData->pnTocStartLBA[pSubQ->byTrackNum - 1]) {
					OutputErrorLog(
						_T("Subchannel & TOC isn't sync. Track %2d, LBA on subchannel: %6d, prevIndex: %2d, LBA on TOC: %6d\n"),
						pSubQ->byTrackNum, nLBA, pPrevSubQ->byIndex, pDiscData->pnTocStartLBA[pSubQ->byTrackNum - 1]);
				}
				lpLBAStartList[pSubQ->byTrackNum - 1][1] = pDiscData->pnTocStartLBA[pSubQ->byTrackNum - 1];
				if (lpLBAStartList[pSubQ->byTrackNum - 1][0] == lpLBAStartList[pSubQ->byTrackNum - 1][1]) {
					// Crow, The - Original Motion Picture Soundtrack (82519-2)
					lpLBAStartList[pSubQ->byTrackNum-1][0] = -1;
				}
			}
		}
		else if (pPrevSubQ->byIndex >= 1 && pSubQ->byIndex == 0) {
			lpLBAStartList[pSubQ->byTrackNum - 1][pSubQ->byIndex] = nLBA;
		}

		if ((pDiscData->toc.TrackData[pSubQ->byTrackNum - 1].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK &&
			(pSubQ->byCtl & AUDIO_DATA_TRACK) == 0) {
			OutputErrorLog(
				_T("LBA %6d, Track[%02d]: data track, but this sector is audio\n"),
				nLBA, pSubQ->byTrackNum);
		}
		else if ((pDiscData->toc.TrackData[pSubQ->byTrackNum - 1].Control & AUDIO_DATA_TRACK) == 0 &&
			(pSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			OutputErrorLog(
				_T("LBA %6d, Track[%02d]: audio track, but this sector is data\n"),
				nLBA, pSubQ->byTrackNum);
		}

		// preserve first lba of data track offset
		if (lpLBAOfDataTrackList[pSubQ->byTrackNum - 1][0] == -1 &&
			(pDiscData->toc.TrackData[pSubQ->byTrackNum - 1].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			lpLBAOfDataTrackList[pSubQ->byTrackNum - 1][0] = nLBA;
		}
		// preserve end data track offset
		else if (nLBA == pDiscData->nAllLength - 1) {
			// preserve end data track offset
			if (lpLBAOfDataTrackList[pSubQ->byTrackNum - 1][0] != -1 &&
				lpLBAOfDataTrackList[pSubQ->byTrackNum - 1][1] == -1 &&
				(pSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				lpLBAOfDataTrackList[pSubQ->byTrackNum - 1][1] = pDiscData->nAllLength - 1;
			}
		}
	}
	return TRUE;
}

VOID SetAndOutputC2ErrorData(
	PC2_ERROR_DATA c2ErrorData,
	INT nLBA,
	UINT uiC2ErrorLBACnt
	)
{
	OutputErrorLog(
		_T("LBA %6d, C2 error exist. SlideSectorNum %d, ErrorByteCnt %d\n")
		_T("            Pos: "),
		nLBA, c2ErrorData[uiC2ErrorLBACnt].cSlideSectorNum, c2ErrorData[uiC2ErrorLBACnt].uiErrorBytePosCnt);
	for (UINT n = 0; n < c2ErrorData[uiC2ErrorLBACnt].uiErrorBytePosCnt; n++) {
		OutputErrorLog(
			_T("%d, "), c2ErrorData[uiC2ErrorLBACnt].lpErrorBytePos[n]);
	}
	OutputErrorLog(_T("\n"));
	c2ErrorData[uiC2ErrorLBACnt].bErrorFlag = RETURNED_C2_ERROR_EXIST;
	c2ErrorData[uiC2ErrorLBACnt].nErrorLBANum = nLBA;
}

VOID SetAndOutputC2NoErrorData(
	PC2_ERROR_DATA c2ErrorData,
	LPBYTE lpBuf,
	INT nLBA,
	DWORD dwAllBufLen,
	UINT uiC2ErrorLBACnt,
	UINT i
	)
{
	OutputErrorLog(
		_T("LBA %6d, C2 error don't exist. Next check 2352 byte.\n"), nLBA);
	// store c2error none main data
	memcpy(c2ErrorData[uiC2ErrorLBACnt].lpBufC2NoneSector,
		lpBuf, dwAllBufLen);
#if 0
	for (INT b = 0; b < 2352; b++) {
		c2ErrorData[uiC2ErrorLBACnt].lpBufC2NoneSector[b] = 0xff;
		c2ErrorData[uiC2ErrorLBACnt].lpBufC2NoneSector[CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE+b] = 0xff;
	}
#endif
	c2ErrorData[uiC2ErrorLBACnt].bErrorFlag = RETURNED_C2_ERROR_1ST_NONE;
	c2ErrorData[uiC2ErrorLBACnt].cSlideSectorNum = c2ErrorData[i].cSlideSectorNumBackup;
	c2ErrorData[uiC2ErrorLBACnt].nErrorLBANum = nLBA;
}

VOID SetAndOutputC2NoErrorByteErrorData(
	PC2_ERROR_DATA c2ErrorData,
	LPBYTE lpBuf,
	INT nLBA,
	DWORD dwAllBufLen,
	UINT uiC2ErrorLBACnt,
	UINT i
	)
{
	OutputErrorLog(
		_T("LBA %6d, C2 error don't exist. But byte don't match\n"), nLBA);
	memcpy(c2ErrorData[uiC2ErrorLBACnt].lpBufC2NoneSector,
		lpBuf, dwAllBufLen);
	c2ErrorData[uiC2ErrorLBACnt].bErrorFlag =
		RETURNED_C2_ERROR_1ST_NONE;
	c2ErrorData[uiC2ErrorLBACnt].cSlideSectorNum =
		c2ErrorData[i].cSlideSectorNumBackup;
	c2ErrorData[uiC2ErrorLBACnt].nErrorLBANum = nLBA;
}

VOID SetC2ErrorBackup(
	PC2_ERROR_DATA c2ErrorData,
	UINT uiC2ErrorLBACntBackup,
	DWORD dwAllBufLen
	)
{
	for (UINT c = 0; c < uiC2ErrorLBACntBackup; c++) {
		c2ErrorData[c].bErrorFlagBackup = c2ErrorData[c].bErrorFlag;
		c2ErrorData[c].bErrorFlag = RETURNED_C2_ERROR_1ST_NONE;
		c2ErrorData[c].cSlideSectorNumBackup =	c2ErrorData[c].cSlideSectorNum;
		c2ErrorData[c].cSlideSectorNum = 0;
		c2ErrorData[c].nErrorLBANumBackup = c2ErrorData[c].nErrorLBANum;
		c2ErrorData[c].nErrorLBANum = 0;
		for (UINT d = 0; d < c2ErrorData[c].uiErrorBytePosCnt; d++) {
			c2ErrorData[c].lpErrorBytePosBackup[d] = c2ErrorData[c].lpErrorBytePos[d];
			c2ErrorData[c].lpErrorBytePos[d] = 0;
		}
		c2ErrorData[c].uiErrorBytePosCntBackup = c2ErrorData[c].uiErrorBytePosCnt;
		c2ErrorData[c].uiErrorBytePosCnt = 0;
		memcpy(c2ErrorData[c].lpBufC2NoneSectorBackup, 
			c2ErrorData[c].lpBufC2NoneSector, dwAllBufLen);
		ZeroMemory(c2ErrorData[c].lpBufC2NoneSector, dwAllBufLen);
	}
}

VOID SetAndOutputMmcToc(
	PDISC_DATA pDiscData
	)
{
	OutputDiscLog(_T("TOC on SCSIOP_READ_TOC\n"));
	_TCHAR strType[7] = { 0 };
	BOOL bFirstData = TRUE;
	for (INT i = pDiscData->toc.FirstTrack; i <= pDiscData->toc.LastTrack; i++) {
		for (INT j = 0, k = 24; j < 4; j++, k -= 8) {
			pDiscData->pnTocStartLBA[i - 1] |= pDiscData->toc.TrackData[i - 1].Address[j] << k;
			pDiscData->pnTocEndLBA[i - 1] |= pDiscData->toc.TrackData[i].Address[j] << k;
		}
		pDiscData->pnTocEndLBA[i - 1] -= 1;
		pDiscData->nAllLength += pDiscData->pnTocEndLBA[i - 1] - pDiscData->pnTocStartLBA[i - 1] + 1;

		if ((pDiscData->toc.TrackData[i - 1].Control & AUDIO_DATA_TRACK) == 0) {
			_tcscpy(strType, _T(" Audio"));
		}
		else if ((pDiscData->toc.TrackData[i - 1].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			if (bFirstData) {
				pDiscData->nFirstDataLBA = pDiscData->pnTocStartLBA[i - 1];
				bFirstData = FALSE;
			}
			_tcscpy(strType, _T("  Data"));
		}
		if (i == pDiscData->toc.FirstTrack && pDiscData->pnTocStartLBA[i - 1] > 0) {
			pDiscData->nAllLength += pDiscData->pnTocStartLBA[i - 1];
			OutputDiscLog(_T("\tPregap Track   , LBA %8u-%8u, Length %8u\n"),
				0, pDiscData->pnTocStartLBA[i - 1] - 1, pDiscData->pnTocStartLBA[i - 1]);
		}
		OutputDiscLog(_T("\t%s Track %2u, LBA %8u-%8u, Length %8u\n"), 
			strType, i, pDiscData->pnTocStartLBA[i - 1], pDiscData->pnTocEndLBA[i - 1],
			pDiscData->pnTocEndLBA[i - 1] - pDiscData->pnTocStartLBA[i - 1] + 1);
	}
	OutputDiscLog(
		_T("\t                                        Total  %8u\n"), pDiscData->nAllLength);
}

VOID SetAndOutputMmcTocFull(
	PDISC_DATA pDiscData,
	PCDROM_TOC_FULL_TOC_DATA fullToc,
	PCDROM_TOC_FULL_TOC_DATA_BLOCK pTocData,
	size_t uiTocEntries,
	FILE* fpCcd
	)
{
	OutputDiscLog(
		_T("FULL TOC on SCSIOP_READ_TOC\n")
		_T("\tFirstCompleteSession: %d\n")
		_T("\t LastCompleteSession: %d\n"),
		fullToc->FirstCompleteSession,
		fullToc->LastCompleteSession);
	BOOL bFirst2ndSession = TRUE;
	pDiscData->nLastLBAof1stSession = -1;
	pDiscData->nStartLBAof2ndSession = -1;

	for (UINT a = 0; a < uiTocEntries; a++) {
		INT nTmpLBA = 0;
		if (fpCcd) {
			WriteCcdFileForEntry(pTocData, a, fpCcd);
		}
		switch (pTocData[a].Point) {
		case 0xA0:
			OutputDiscLog(_T("\tSession %d, FirstTrack %2d, "), 
				pTocData[a].SessionNumber, pTocData[a].Msf[0]);
			switch (pTocData[a].Msf[1]) {
				case 0x00:
					OutputDiscLog(_T("Format: CD-DA or CD-ROM\n"));
					break;
				case 0x10:
					OutputDiscLog(_T("Format: CD-I\n"));
					break;
				case 0x20:
					OutputDiscLog(_T("Format: CD-ROM-XA\n"));
					break;
				default:
					OutputDiscLog(_T("Format: Other\n"));
					break;
			}
			break;
		case 0xA1:
			OutputDiscLog(_T("\tSession %d,  LastTrack %2d\n"), 
				pTocData[a].SessionNumber, pTocData[a].Msf[0]);
			pDiscData->toc.LastTrack = pTocData[a].Msf[0];
			break;
		case 0xA2:
			nTmpLBA = MSFtoLBA(pTocData[a].Msf[2], pTocData[a].Msf[1], pTocData[a].Msf[0]);
			OutputDiscLog(
				_T("\tSession %d,      Leadout, MSF %02d:%02d:%02d (LBA %6d)\n"), 
				pTocData[a].SessionNumber, pTocData[a].Msf[0], pTocData[a].Msf[1],
				pTocData[a].Msf[2], nTmpLBA);
			pDiscData->pnTocEndLBA[pDiscData->toc.LastTrack - 1] = nTmpLBA - 150 - 1;
			if (pTocData[a].SessionNumber == 1) {
				pDiscData->nLastLBAof1stSession = nTmpLBA - 150;
			}
			break;
		case 0xB0: // (multi-session disc)
			OutputDiscLog(
				_T("\tSession %d,  NextSession, MSF %02d:%02d:%02d (LBA %6d)\n")
				_T("\t     Outermost Lead-out, MSF %02d:%02d:%02d (LBA %6d)\n") 
				_T("\tThe number of different Mode-5 pointers present %02d\n"), 
				pTocData[a].SessionNumber, pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1],
				pTocData[a].MsfExtra[2], 
				MSFtoLBA(pTocData[a].MsfExtra[2], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[0]),
				pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2],
				MSFtoLBA(pTocData[a].Msf[2], pTocData[a].Msf[1], pTocData[a].Msf[0]),
				pTocData[a].Zero);
			break;
		case 0xB1: // (Audio only: This identifies the presence of skip intervals)
			OutputDiscLog(
				_T("\tThe number of skip interval pointers %2d\n")
				_T("\tThe number of skip track assignments %2d\n"),
				pTocData[a].Msf[0], pTocData[a].Msf[1]);
			break;
		case 0xB2: // (Audio only: This identifies tracks that should be skipped during playback)
		case 0xB3:
		case 0xB4:
			OutputDiscLog(
				_T("\tTrack number to skip upon playback %2d\n")
				_T("\tTrack number to skip upon playback %2d\n")
				_T("\tTrack number to skip upon playback %2d\n")
				_T("\tTrack number to skip upon playback %2d\n")
				_T("\tTrack number to skip upon playback %2d\n")
				_T("\tTrack number to skip upon playback %2d\n"),
				pTocData[a].MsfExtra[2], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[0],
				pTocData[a].Msf[2], pTocData[a].Msf[1], pTocData[a].Msf[0]);
			break;
		case 0xC0: // (Together with POINT=B0h, this is used to identify a multi-session disc)
			OutputDiscLog(
				_T("\tSession %d,  ATIP values, MSF %02d:%02d:%02d\n")
				_T("\t          First Lead-in, MSF %02d:%02d:%02d (LBA %6d)\n"), 
				pTocData[a].SessionNumber, pTocData[a].MsfExtra[0],	
				pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2], 
				pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2],
				MSFtoLBA(pTocData[a].Msf[2], pTocData[a].Msf[1], pTocData[a].Msf[0]));
			break;
		default:
			nTmpLBA = MSFtoLBA(pTocData[a].Msf[2], pTocData[a].Msf[1], pTocData[a].Msf[0]);
			OutputDiscLog(
				_T("\tSession %d,     Track %2d, MSF %02d:%02d:%02d (LBA %6d)\n"), 
				pTocData[a].SessionNumber, pTocData[a].Point, pTocData[a].Msf[0], 
				pTocData[a].Msf[1], pTocData[a].Msf[2], nTmpLBA);
			if (pTocData[a].Point == 1) {
				pDiscData->pnTocStartLBA[pTocData[a].Point - 1] = nTmpLBA - 150;
			}
			if (pTocData[a].Point >= 2 && pTocData[a].Point <= 100) {
				pDiscData->pnTocEndLBA[pTocData[a].Point - 2] = nTmpLBA - 150 - 1;
				pDiscData->pnTocStartLBA[pTocData[a].Point - 1] = nTmpLBA - 150;
			}
			if (pTocData[a].SessionNumber == 2 && bFirst2ndSession) {
				pDiscData->nStartLBAof2ndSession = nTmpLBA - 150;
				bFirst2ndSession = FALSE;
			}
			pDiscData->puiSessionNum[pTocData[a].Point - 1] = pTocData[a].SessionNumber;
			break;
		}
	}
}

VOID SetAndOutputMmcTocCDText(
	PDISC_DATA pDiscData,
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	PCHAR pTmpText,
	size_t uiTocTextEntries,
	size_t uiAllTextSize
	)
{
	BYTE byAlbumCnt = 0;
	BYTE byPerformerCnt = 0;
	BYTE bySongwriterCnt = 0;
	BYTE byComposerCnt = 0;
	BYTE byArrangerCnt = 0;
	BYTE byMessagesCnt = 0;
	BYTE byDiscIdCnt = 0;
	BYTE byGenreCnt = 0;
	BYTE byTocInfoCnt = 0;
	BYTE byTocInfo2Cnt = 0;
	BYTE byUpcEanCnt = 0;
	BYTE bySizeInfoCnt = 0;

	BYTE byAlbumIdx = 0;
	BYTE byPerformerIdx = 0;
	BYTE bySongwriterIdx = 0;
	BYTE byComposerIdx = 0;
	BYTE byArrangerIdx = 0;
	BYTE byMessagesIdx = 0;
	BYTE byDiscIdIdx = 0;
	BYTE byGenreIdx = 0;
	BYTE byTocInfoIdx = 0;
	BYTE byTocInfo2Idx = 0;
	BYTE byUpcEanIdx = 0;
	BYTE bySizeInfoIdx = 0;

	for (size_t t = 0; t < uiTocTextEntries; t++) {
		BYTE bRet = 0;
		BYTE bCnt = 0;
		for (INT k = 0; k < 12; k++) {
			if (pDesc[t].Text[k] == 0) {
				bRet++;
				if (k < 11 && pDesc[t].Text[k+1] == 0) {
					bRet--;
					bCnt++;
				}
				if (k == 11 && bCnt == 11 && pDesc[t].CharacterPosition == 0) {
					bRet--;
				}
			}
		}

		if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_ALBUM_NAME) {
			if (bRet) {
				byAlbumCnt += bRet;
			}
			byAlbumIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_PERFORMER) {
			if (bRet) {
				byPerformerCnt += bRet;
			}
			byPerformerIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_SONGWRITER) {
			if (bRet) {
				bySongwriterCnt += bRet;
			}
			bySongwriterIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_COMPOSER) {
			if (bRet) {
				byComposerCnt += bRet;
			}
			byComposerIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_ARRANGER) {
			if (bRet) {
				byArrangerCnt += bRet;
			}
			byArrangerIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_MESSAGES) {
			if (bRet) {
				byMessagesCnt += bRet;
			}
			byMessagesIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_DISC_ID) {
			if (bRet) {
				byDiscIdCnt += bRet;
			}
			byDiscIdIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_GENRE) {
			if (bRet) {
				byGenreCnt += bRet;
			}
			byGenreIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_TOC_INFO) {
			byTocInfoCnt++;
			byTocInfoIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_TOC_INFO2) {
			byTocInfo2Cnt++;
			byTocInfo2Idx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_UPC_EAN) {
			if (bRet) {
				byUpcEanCnt += bRet;
			}
			byUpcEanIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_SIZE_INFO) {
			bySizeInfoCnt++;
			bySizeInfoIdx = pDesc[t].SequenceNumber;
		}
		if (pDesc[t].PackType != CDROM_CD_TEXT_PACK_TOC_INFO &&
			pDesc[t].PackType != CDROM_CD_TEXT_PACK_TOC_INFO2 &&
			pDesc[t].PackType != CDROM_CD_TEXT_PACK_SIZE_INFO) {
			memcpy(pTmpText + 12 * t, (pDesc[t].Text), 12);
		}
	}

	size_t uiIdx = 0;
	INT nTitleCnt = 0;
	INT nPerformerCnt = 0;
	INT nSongwriterCnt = 0;
	for (size_t z = 0; z < uiTocTextEntries; z++) {
		if (uiIdx == uiAllTextSize) {
			break;
		}
		size_t len1 = strlen(pTmpText + uiIdx);
		if (len1 == 0 || len1 >= META_CDTEXT_SIZE) {
			z--;
		}
		else {
			CHAR ctmp[META_CDTEXT_SIZE] = { 0 };
			_TCHAR tmp[META_CDTEXT_SIZE] = { 0 };
			strncpy(ctmp, pTmpText + uiIdx, len1);
			ctmp[META_CDTEXT_SIZE - 1] = 0;
#ifdef UNICODE
			INT len = MultiByteToWideChar(CP_ACP, 0, ctmp, -1, NULL, 0);
			if (!len) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			}
			if (!MultiByteToWideChar(CP_ACP, 0, ctmp, -1, tmp, len)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			}
#else
			strncpy(tmp, ctmp, len1);
#endif
			if (byAlbumCnt != 0 && z < byAlbumCnt) {
				_tcsncpy(pDiscData->pszTitle[nTitleCnt], tmp, _tcslen(tmp));
				if (nTitleCnt == 0) {
					OutputDiscLog(_T("\tAlbum Name: %s\n"), pDiscData->pszTitle[nTitleCnt]);
				}
				else {
					OutputDiscLog(_T("\t Song Name: %s\n"), pDiscData->pszTitle[nTitleCnt]);
				}
				nTitleCnt++;
			}
			else if (byPerformerCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt)) {
				_tcsncpy(pDiscData->pszPerformer[nPerformerCnt], tmp, _tcslen(tmp));
				if (nPerformerCnt == 0) {
					OutputDiscLog(_T("\tAlbum Performer: %s\n"), pDiscData->pszPerformer[nPerformerCnt]);
				}
				else {
					OutputDiscLog(_T("\t Song Performer: %s\n"), pDiscData->pszPerformer[nPerformerCnt]);
				}
				nPerformerCnt++;
			}
			else if (bySongwriterCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt)) {
				_tcsncpy(pDiscData->pszSongWriter[nSongwriterCnt], tmp, _tcslen(tmp));
				if (nSongwriterCnt == 0) {
					OutputDiscLog(_T("\tAlbum SongWriter: %s\n"), pDiscData->pszSongWriter[nSongwriterCnt]);
				}
				else {
					OutputDiscLog(_T("\t      SongWriter: %s\n"), pDiscData->pszSongWriter[nSongwriterCnt]);
				}
				nSongwriterCnt++;
			}
			else if (byComposerCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt + byComposerCnt)) {
				OutputDiscLog(_T("\tComposer: %s\n"), tmp);
			}
			else if (byArrangerCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt + byComposerCnt + byArrangerCnt)) {
				OutputDiscLog(_T("\tArranger: %s\n"), tmp);
			}
			else if (byMessagesCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt)) {
				OutputDiscLog(_T("\tMessages: %s\n"), tmp);
			}
			else if (byDiscIdCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt + byDiscIdCnt)) {
				OutputDiscLog(_T("\tDiscId: %s\n"), tmp);
			}
			else if (byGenreCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt + byDiscIdCnt + byGenreCnt)) {
				OutputDiscLog(_T("\tGenre: %s\n"), tmp);
			}
			else if (byUpcEanCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt + byDiscIdCnt +
				byGenreCnt + byTocInfoCnt + byTocInfo2Cnt + byUpcEanCnt)) {
				OutputDiscLog(_T("\tUpcEan: %s\n"), tmp);
			}
		}
		uiIdx += len1 + 1;
	}
	INT nTocInfoCnt = 0;
	INT nSizeInfoCnt = 0;
	for (size_t z = 0; z <= bySizeInfoIdx; z++) {
		if (pDesc[z].PackType == CDROM_CD_TEXT_PACK_TOC_INFO) {
			// detail in Page 54-55 of EN 60908:1999
			OutputDiscLog(_T("\tTocInfo\n"));
			if (nTocInfoCnt == 0) {
				OutputDiscLog(
					_T("\t\t  First track number: %d\n")
					_T("\t\t   Last track number: %d\n")
					_T("\t\t   Lead-out(minutes): %d\n")
					_T("\t\t   Lead-out(seconds): %d\n")
					_T("\t\t    Lead-out(frames): %d\n"),
					pDesc[z].Text[0],
					pDesc[z].Text[1],
					pDesc[z].Text[3],
					pDesc[z].Text[4],
					pDesc[z].Text[5]);
			}
			if (nTocInfoCnt == 1) {
				OutputDiscLog(
					_T("\t\t    Track 1(minutes): %d\n")
					_T("\t\t    Track 1(seconds): %d\n")
					_T("\t\t     Track 1(frames): %d\n")
					_T("\t\t    Track 2(minutes): %d\n")
					_T("\t\t    Track 2(seconds): %d\n")
					_T("\t\t     Track 2(frames): %d\n")
					_T("\t\t    Track 3(minutes): %d\n")
					_T("\t\t    Track 3(seconds): %d\n")
					_T("\t\t     Track 3(frames): %d\n")
					_T("\t\t    Track 4(minutes): %d\n")
					_T("\t\t    Track 4(seconds): %d\n")
					_T("\t\t     Track 4(frames): %d\n"),
					pDesc[z].Text[0],
					pDesc[z].Text[1],
					pDesc[z].Text[2],
					pDesc[z].Text[3],
					pDesc[z].Text[4],
					pDesc[z].Text[5],
					pDesc[z].Text[6],
					pDesc[z].Text[7],
					pDesc[z].Text[8],
					pDesc[z].Text[9],
					pDesc[z].Text[10],
					pDesc[z].Text[11]);
			}
			nTocInfoCnt++;
		}
		else if (pDesc[z].PackType == CDROM_CD_TEXT_PACK_TOC_INFO2) {
			OutputDiscLog(_T("\tTocInfo2\n"));
			OutputDiscLog(
				_T("\t\t         Priority number: %d\n")
				_T("\t\t     Number of intervals: %d\n")
				_T("\t\t    Start point(minutes): %d\n")
				_T("\t\t    Start point(seconds): %d\n")
				_T("\t\t     Start point(frames): %d\n")
				_T("\t\t      End point(minutes): %d\n")
				_T("\t\t      End point(seconds): %d\n")
				_T("\t\t       End point(frames): %d\n"),
				pDesc[z].Text[0],
				pDesc[z].Text[1],
				pDesc[z].Text[6],
				pDesc[z].Text[7],
				pDesc[z].Text[8],
				pDesc[z].Text[9],
				pDesc[z].Text[10],
				pDesc[z].Text[11]);
		}
		else if (pDesc[z].PackType == CDROM_CD_TEXT_PACK_SIZE_INFO) {
			// detail in Page 56 of EN 60908:1999
			OutputDiscLog(_T("\tSizeInfo\n"));
			if (nSizeInfoCnt == 0) {
				OutputDiscLog(
					_T("\t\t  Charactor Code for this BLOCK: %d\n")
					_T("\t\t             First track Number: %d\n")
					_T("\t\t              Last track Number: %d\n")
					_T("\t\t  Mode2 & copy protection flags: %d\n")
					_T("\t\tNumber of PACKS with ALBUM_NAME: %d\n")
					_T("\t\t Number of PACKS with PERFORMER: %d\n")
					_T("\t\tNumber of PACKS with SONGWRITER: %d\n")
					_T("\t\t  Number of PACKS with COMPOSER: %d\n")
					_T("\t\t  Number of PACKS with ARRANGER: %d\n")
					_T("\t\t  Number of PACKS with MESSAGES: %d\n")
					_T("\t\t   Number of PACKS with DISC_ID: %d\n")
					_T("\t\t     Number of PACKS with GENRE: %d\n"),
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[0],
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[1],
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[2],
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[3],
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[4],
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[5],
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[6],
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[7],
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[8],
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[9],
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[10],
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[11]);
			}
			else if (nSizeInfoCnt == 1) {
				OutputDiscLog(
					_T("\t\t  Number of PACKS with TOC_INFO: %d\n")
					_T("\t\t Number of PACKS with TOC_INFO2: %d\n")
					_T("\t\t       Number of PACKS with $8a: %d\n")
					_T("\t\t       Number of PACKS with $8b: %d\n")
					_T("\t\t       Number of PACKS with $8c: %d\n")
					_T("\t\t       Number of PACKS with $8d: %d\n")
					_T("\t\t   Number of PACKS with UPC_EAN: %d\n")
					_T("\t\t Number of PACKS with SIZE_INFO: %d\n")
					_T("\t\tLast Sequence number of BLOCK 0: %d\n")
					_T("\t\tLast Sequence number of BLOCK 1: %d\n")
					_T("\t\tLast Sequence number of BLOCK 2: %d\n")
					_T("\t\tLast Sequence number of BLOCK 3: %d\n"),
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[0],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[1],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[2],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[3],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[4],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[5],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[6],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[7],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[8],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[9],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[10],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[11]);
			}
			else if (nSizeInfoCnt == 2) {
				OutputDiscLog(
					_T("\t\tLast Sequence number of BLOCK 4: %d\n")
					_T("\t\tLast Sequence number of BLOCK 5: %d\n")
					_T("\t\tLast Sequence number of BLOCK 6: %d\n")
					_T("\t\tLast Sequence number of BLOCK 7: %d\n")
					_T("\t\t          Language code BLOCK 0: %d\n")
					_T("\t\t          Language code BLOCK 1: %d\n")
					_T("\t\t          Language code BLOCK 2: %d\n")
					_T("\t\t          Language code BLOCK 3: %d\n")
					_T("\t\t          Language code BLOCK 4: %d\n")
					_T("\t\t          Language code BLOCK 5: %d\n")
					_T("\t\t          Language code BLOCK 6: %d\n")
					_T("\t\t          Language code BLOCK 7: %d\n"),
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[0],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[1],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[2],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[3],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[4],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[5],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[6],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[7],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[8],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[9],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[10],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[11]);
			}
			nSizeInfoCnt++;
		}
	}
}

VOID SetAndOutputMmcTocCDWText(
	PDISC_DATA pDiscData,
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	PCHAR pTmpText,
	size_t uiFirstEntries,
	size_t uiTocTextEntries,
	size_t uiAllTextSize
	)
{
	UNREFERENCED_PARAMETER(pDiscData);
	BYTE byAlbumCnt = 0;
	BYTE byPerformerCnt = 0;
	BYTE bySongwriterCnt = 0;
	BYTE byComposerCnt = 0;
	BYTE byArrangerCnt = 0;
	BYTE byMessagesCnt = 0;
	BYTE byDiscIdCnt = 0;
	BYTE byGenreCnt = 0;
	BYTE byTocInfoCnt = 0;
	BYTE byTocInfo2Cnt = 0;
	BYTE byUpcEanCnt = 0;
	BYTE bySizeInfoCnt = 0;

	BYTE byAlbumIdx = 0;
	BYTE byPerformerIdx = 0;
	BYTE bySongwriterIdx = 0;
	BYTE byComposerIdx = 0;
	BYTE byArrangerIdx = 0;
	BYTE byMessagesIdx = 0;
	BYTE byDiscIdIdx = 0;
	BYTE byGenreIdx = 0;
	BYTE byTocInfoIdx = 0;
	BYTE byTocInfo2Idx = 0;
	BYTE byUpcEanIdx = 0;
	BYTE bySizeInfoIdx = 0;

	for (size_t t = uiFirstEntries; t < uiTocTextEntries; t++) {
		BOOL bRet = FALSE;
		for (INT k = 0; k < 6; k++) {
			if (pDesc[t].WText[k] == 0) {
				bRet++;
				if (k < 5 && pDesc[t].WText[k + 1] == 0) {
					bRet--;
				}
			}
		}

		if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_ALBUM_NAME) {
			if (bRet) {
				byAlbumCnt++;
			}
			byAlbumIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_PERFORMER) {
			if (bRet) {
				byPerformerCnt++;
			}
			byPerformerIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_SONGWRITER) {
			if (bRet) {
				bySongwriterCnt++;
			}
			bySongwriterIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_COMPOSER) {
			if (bRet) {
				byComposerCnt++;
			}
			byComposerIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_ARRANGER) {
			if (bRet) {
				byArrangerCnt++;
			}
			byArrangerIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_MESSAGES) {
			if (bRet) {
				byMessagesCnt++;
			}
			byMessagesIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_DISC_ID) {
			if (bRet) {
				byDiscIdCnt++;
			}
			byDiscIdIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_GENRE) {
			if (bRet) {
				byGenreCnt++;
			}
			byGenreIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_TOC_INFO) {
			byTocInfoCnt++;
			byTocInfoIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_TOC_INFO2) {
			byTocInfo2Cnt++;
			byTocInfo2Idx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_UPC_EAN) {
			if (bRet) {
				byUpcEanCnt++;
			}
			byUpcEanIdx = pDesc[t].SequenceNumber;
		}
		else if (pDesc[t].PackType == CDROM_CD_TEXT_PACK_SIZE_INFO) {
			bySizeInfoCnt++;
			bySizeInfoIdx = pDesc[t].SequenceNumber;
		}
		if (pDesc[t].PackType != CDROM_CD_TEXT_PACK_TOC_INFO &&
			pDesc[t].PackType != CDROM_CD_TEXT_PACK_TOC_INFO2 &&
			pDesc[t].PackType != CDROM_CD_TEXT_PACK_SIZE_INFO) {
			memcpy(pTmpText + 12 * (t - uiFirstEntries), (WCHAR*)(pDesc[t].Text), 12);
		}
	}

	size_t uiIdx = 0;
	INT nTitleCnt = 0;
	INT nPerformerCnt = 0;
	INT nSongwriterCnt = 0;
	for (size_t z = 0; z < uiTocTextEntries; z++) {
		if (uiIdx == uiAllTextSize) {
			break;
		}
		size_t len1 = strlen(pTmpText + uiIdx);
		if (len1 == 0 || len1 >= META_CDTEXT_SIZE) {
			z--;
		}
		else {
			CHAR ctmp[META_CDTEXT_SIZE] = { 0 };
			_TCHAR tmp[META_CDTEXT_SIZE] = { 0 };
			strncpy(ctmp, pTmpText + uiIdx, len1);
			ctmp[META_CDTEXT_SIZE - 1] = 0;
#ifdef UNICODE
			INT len = MultiByteToWideChar(CP_ACP, 0, ctmp, -1, NULL, 0);
			if (!len) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			}
			if (!MultiByteToWideChar(CP_ACP, 0, ctmp, -1, tmp, len)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			}
#else
			strncpy(tmp, ctmp, len1);
#endif
			if (byAlbumCnt != 0 && z < byAlbumCnt) {
				if (nTitleCnt == 0) {
					OutputDiscLog(_T("\tAlbum Name: %s\n"), tmp);
				}
				else {
					OutputDiscLog(_T("\t Song Name: %s\n"), tmp);
				}
				nTitleCnt++;
			}
			else if (byPerformerCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt)) {
				if (nPerformerCnt == 0) {
					OutputDiscLog(_T("\tAlbum Performer: %s\n"), tmp);
				}
				else {
					OutputDiscLog(_T("\t Song Performer: %s\n"), tmp);
				}
				nPerformerCnt++;
			}
			else if (bySongwriterCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt)) {
				if (nSongwriterCnt == 0) {
					OutputDiscLog(_T("\tAlbum SongWriter: %s\n"), tmp);
				}
				else {
					OutputDiscLog(_T("\t      SongWriter: %s\n"), tmp);
				}
				nSongwriterCnt++;
			}
			else if (byComposerCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt + byComposerCnt)) {
				OutputDiscLog(_T("\tComposer: %s\n"), tmp);
			}
			else if (byArrangerCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt + byComposerCnt + byArrangerCnt)) {
				OutputDiscLog(_T("\tArranger: %s\n"), tmp);
			}
			else if (byMessagesCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt)) {
				OutputDiscLog(_T("\tMessages: %s\n"), tmp);
			}
			else if (byDiscIdCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt + byDiscIdCnt)) {
				OutputDiscLog(_T("\tDiscId: %s\n"), tmp);
			}
			else if (byGenreCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt + byDiscIdCnt + byGenreCnt)) {
				OutputDiscLog(_T("\tGenre: %s\n"), tmp);
			}
			else if (byUpcEanCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt + byDiscIdCnt +
				byGenreCnt + byTocInfoCnt + byTocInfo2Cnt + byUpcEanCnt)) {
				OutputDiscLog(_T("\tUpcEan: %s\n"), tmp);
			}
		}
		uiIdx += len1 + 1;
	}
	INT nTocInfoCnt = 0;
	INT nSizeInfoCnt = 0;
	for (size_t z = 0; z <= bySizeInfoIdx; z++) {
		if (pDesc[z].PackType == CDROM_CD_TEXT_PACK_TOC_INFO) {
			// detail in Page 54-55 of EN 60908:1999
			OutputDiscLog(_T("\tTocInfo\n"));
			if (nTocInfoCnt == 0) {
				OutputDiscLog(
					_T("\t\t  First track number: %d\n")
					_T("\t\t   Last track number: %d\n")
					_T("\t\t   Lead-out(minutes): %d\n")
					_T("\t\t   Lead-out(seconds): %d\n")
					_T("\t\t    Lead-out(frames): %d\n"),
					pDesc[z].Text[0],
					pDesc[z].Text[1],
					pDesc[z].Text[3],
					pDesc[z].Text[4],
					pDesc[z].Text[5]);
			}
			if (nTocInfoCnt == 1) {
				OutputDiscLog(
					_T("\t\t    Track 1(minutes): %d\n")
					_T("\t\t    Track 1(seconds): %d\n")
					_T("\t\t     Track 1(frames): %d\n")
					_T("\t\t    Track 2(minutes): %d\n")
					_T("\t\t    Track 2(seconds): %d\n")
					_T("\t\t     Track 2(frames): %d\n")
					_T("\t\t    Track 3(minutes): %d\n")
					_T("\t\t    Track 3(seconds): %d\n")
					_T("\t\t     Track 3(frames): %d\n")
					_T("\t\t    Track 4(minutes): %d\n")
					_T("\t\t    Track 4(seconds): %d\n")
					_T("\t\t     Track 4(frames): %d\n"),
					pDesc[z].Text[0],
					pDesc[z].Text[1],
					pDesc[z].Text[2],
					pDesc[z].Text[3],
					pDesc[z].Text[4],
					pDesc[z].Text[5],
					pDesc[z].Text[6],
					pDesc[z].Text[7],
					pDesc[z].Text[8],
					pDesc[z].Text[9],
					pDesc[z].Text[10],
					pDesc[z].Text[11]);
			}
			nTocInfoCnt++;
		}
		else if (pDesc[z].PackType == CDROM_CD_TEXT_PACK_TOC_INFO2) {
			OutputDiscLog(_T("\tTocInfo2\n"));
			OutputDiscLog(
				_T("\t\t         Priority number: %d\n")
				_T("\t\t     Number of intervals: %d\n")
				_T("\t\t    Start point(minutes): %d\n")
				_T("\t\t    Start point(seconds): %d\n")
				_T("\t\t     Start point(frames): %d\n")
				_T("\t\t      End point(minutes): %d\n")
				_T("\t\t      End point(seconds): %d\n")
				_T("\t\t       End point(frames): %d\n"),
				pDesc[z].Text[0],
				pDesc[z].Text[1],
				pDesc[z].Text[6],
				pDesc[z].Text[7],
				pDesc[z].Text[8],
				pDesc[z].Text[9],
				pDesc[z].Text[10],
				pDesc[z].Text[11]);
		}
		else if (pDesc[z].PackType == CDROM_CD_TEXT_PACK_SIZE_INFO) {
			// detail in Page 56 of EN 60908:1999
			OutputDiscLog(_T("\tSizeInfo\n"));
			if (nSizeInfoCnt == 0) {
				OutputDiscLog(
					_T("\t\t  Charactor Code for this BLOCK: %d\n")
					_T("\t\t             First track Number: %d\n")
					_T("\t\t              Last track Number: %d\n")
					_T("\t\t  Mode2 & copy protection flags: %d\n")
					_T("\t\tNumber of PACKS with ALBUM_NAME: %d\n")
					_T("\t\t Number of PACKS with PERFORMER: %d\n")
					_T("\t\tNumber of PACKS with SONGWRITER: %d\n")
					_T("\t\t  Number of PACKS with COMPOSER: %d\n")
					_T("\t\t  Number of PACKS with ARRANGER: %d\n")
					_T("\t\t  Number of PACKS with MESSAGES: %d\n")
					_T("\t\t   Number of PACKS with DISC_ID: %d\n")
					_T("\t\t     Number of PACKS with GENRE: %d\n"),
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[0],
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[1],
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[2],
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[3],
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[4],
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[5],
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[6],
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[7],
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[8],
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[9],
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[10],
					pDesc[uiTocTextEntries - bySizeInfoCnt].Text[11]);
			}
			else if (nSizeInfoCnt == 1) {
				OutputDiscLog(
					_T("\t\t  Number of PACKS with TOC_INFO: %d\n")
					_T("\t\t Number of PACKS with TOC_INFO2: %d\n")
					_T("\t\t       Number of PACKS with $8a: %d\n")
					_T("\t\t       Number of PACKS with $8b: %d\n")
					_T("\t\t       Number of PACKS with $8c: %d\n")
					_T("\t\t       Number of PACKS with $8d: %d\n")
					_T("\t\t   Number of PACKS with UPC_EAN: %d\n")
					_T("\t\t Number of PACKS with SIZE_INFO: %d\n")
					_T("\t\tLast Sequence number of BLOCK 0: %d\n")
					_T("\t\tLast Sequence number of BLOCK 1: %d\n")
					_T("\t\tLast Sequence number of BLOCK 2: %d\n")
					_T("\t\tLast Sequence number of BLOCK 3: %d\n"),
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[0],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[1],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[2],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[3],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[4],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[5],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[6],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[7],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[8],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[9],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[10],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 1].Text[11]);
			}
			else if (nSizeInfoCnt == 2) {
				OutputDiscLog(
					_T("\t\tLast Sequence number of BLOCK 4: %d\n")
					_T("\t\tLast Sequence number of BLOCK 5: %d\n")
					_T("\t\tLast Sequence number of BLOCK 6: %d\n")
					_T("\t\tLast Sequence number of BLOCK 7: %d\n")
					_T("\t\t          Language code BLOCK 0: %d\n")
					_T("\t\t          Language code BLOCK 1: %d\n")
					_T("\t\t          Language code BLOCK 2: %d\n")
					_T("\t\t          Language code BLOCK 3: %d\n")
					_T("\t\t          Language code BLOCK 4: %d\n")
					_T("\t\t          Language code BLOCK 5: %d\n")
					_T("\t\t          Language code BLOCK 6: %d\n")
					_T("\t\t          Language code BLOCK 7: %d\n"),
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[0],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[1],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[2],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[3],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[4],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[5],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[6],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[7],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[8],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[9],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[10],
					pDesc[uiTocTextEntries - bySizeInfoCnt + 2].Text[11]);
			}
			nSizeInfoCnt++;
		}
	}
}

VOID SetCDOffsetData(
	PDISC_DATA pDiscData,
	PCD_OFFSET_DATA pCdOffsetData,
	INT nStartLBA,
	INT nEndLBA
	)
{
	if (pDiscData->nCombinedOffset > 0) {
		pCdOffsetData->uiMainDataSlideSize = 
			(size_t)pDiscData->nCombinedOffset % CD_RAW_SECTOR_SIZE;
		pCdOffsetData->nOffsetStart = 0;
		pCdOffsetData->nOffsetEnd = pDiscData->nAdjustSectorNum;
		pCdOffsetData->nFixStartLBA = nStartLBA + pDiscData->nAdjustSectorNum - 1;
		pCdOffsetData->nFixEndLBA = nEndLBA + pDiscData->nAdjustSectorNum;
	}
	else if (pDiscData->nCombinedOffset < 0) {
		pCdOffsetData->uiMainDataSlideSize = 
			(size_t)CD_RAW_SECTOR_SIZE + (pDiscData->nCombinedOffset % CD_RAW_SECTOR_SIZE);
		pCdOffsetData->nOffsetStart = pDiscData->nAdjustSectorNum;
		pCdOffsetData->nOffsetEnd = 0;
		pCdOffsetData->nFixStartLBA = nStartLBA + pDiscData->nAdjustSectorNum;
		pCdOffsetData->nFixEndLBA = nEndLBA + pDiscData->nAdjustSectorNum + 1;
	}
}

VOID SetISRCToString(
	PDISC_DATA pDiscData,
	LPBYTE lpSubcode,
	LPTSTR pszOutString,
	INT nTrackNum,
	BOOL bCopy
	)
{
	/*
	BYTE＼bit |	7	 6	  5	  4	  3	  2	  1	  0
	0	      |      Ctrl	    |		ADR
	1		  |		I01				    |(MSB) I02
	2		  |		I02　(LSB)  |(MSB)　　I03
	3		  |I03 (LSB)|			I04
	4		  |I05						|ZERO
	5		  |I06			    |I07
	6		  |I08			    |I09
	7		  |I10			    |I11
	8		  |I12			    |ZERO
	9	フレーム (=1/75秒) (CD全体の経過時間)(BCD)
	10	
	(MSB)
	CRC　P(x)=x16+x12+x5+x1
	(LSB)
	11

	I01 ～ I02 : 国名コード (6ビット)
	I03 ～ I05 : 登録者コード (6ビット)
	I06 ～ I07 : 記録年 (4ビット)
	I08 ～ I12 : シリアルナンバー (4ビット)

	これをASCIIコードに変換するには、それぞれに 0x30 を足してやります。
	*/
	_stprintf(pszOutString, _T("%c%c%c%c%c%c%c%c%c%c%c%c"),
		((lpSubcode[13] >> 2) & 0x3F) + 0x30, 
		(((lpSubcode[13] << 4) & 0x30) | ((lpSubcode[14] >> 4) & 0x0F)) + 0x30, 
		(((lpSubcode[14] << 2) & 0x3C) | ((lpSubcode[15] >> 6) & 0x03)) + 0x30, 
		(lpSubcode[15] & 0x3F) + 0x30, 
		((lpSubcode[16] >> 2) & 0x3F) + 0x30, 
		((lpSubcode[17] >> 4) & 0x0F) + 0x30, (lpSubcode[17] & 0x0F) + 0x30, 
		((lpSubcode[18] >> 4) & 0x0F) + 0x30, (lpSubcode[18] & 0x0F) + 0x30, 
		((lpSubcode[19] >> 4) & 0x0F) + 0x30, (lpSubcode[19] & 0x0F) + 0x30,
		((lpSubcode[20] >> 4) & 0x0F) + 0x30);
	if (bCopy) {
		_tcsncpy(pDiscData->pszISRC[nTrackNum - 1], pszOutString, META_ISRC_SIZE);
	}
}

VOID SetMCNToString(
	PDISC_DATA pDiscData,
	LPBYTE lpSubcode,
	LPTSTR pszOutString,
	BOOL bCopy
	)
{
	_stprintf(pszOutString, _T("%c%c%c%c%c%c%c%c%c%c%c%c%c"), 
		((lpSubcode[13] >> 4) & 0x0F) + 0x30, (lpSubcode[13] & 0x0F) + 0x30, 
		((lpSubcode[14] >> 4) & 0x0F) + 0x30, (lpSubcode[14] & 0x0F) + 0x30, 
		((lpSubcode[15] >> 4) & 0x0F) + 0x30, (lpSubcode[15] & 0x0F) + 0x30, 
		((lpSubcode[16] >> 4) & 0x0F) + 0x30, (lpSubcode[16] & 0x0F) + 0x30, 
		((lpSubcode[17] >> 4) & 0x0F) + 0x30, (lpSubcode[17] & 0x0F) + 0x30, 
		((lpSubcode[18] >> 4) & 0x0F) + 0x30, (lpSubcode[18] & 0x0F) + 0x30, 
		((lpSubcode[19] >> 4) & 0x0F) + 0x30);
	if (bCopy) {
		_tcsncpy(pDiscData->szCatalog, pszOutString, META_CATALOG_SIZE);
	}
}

VOID SetReadCDCommand(
	PDEVICE_DATA pDevData,
	PEXT_ARG pExtArg,
	LPBYTE lpCmd,
	BOOL bCDG,
	READ_CD_FLAG::SECTOR_TYPE flg,
	BYTE byTransferLen
	)
{
	lpCmd[0] = SCSIOP_READ_CD;
	lpCmd[1] = (BYTE)flg;
	lpCmd[8] = byTransferLen;
	lpCmd[9] = READ_CD_FLAG::SyncData | READ_CD_FLAG::SubHeader | 
		READ_CD_FLAG::UserData | READ_CD_FLAG::MainHeader | READ_CD_FLAG::Edc;
	if (pDevData->bC2ErrorData && 
		(pExtArg != NULL && pExtArg->bC2)) {
		lpCmd[9] |= READ_CD_FLAG::C2ErrorBlockData;
	}
	if (bCDG) {
		// memo CD+G ripping
		// raw mode(001b) don't play CDG
		// plextor:D8                  -> play CDG OK
		// plextor:READ_CD_FLAG::Pack  -> play CDG OK
		// plextor:READ_CD_FLAG::Raw   -> play CDG NG (PQ is OK)
		// slimtype:READ_CD_FLAG::Pack -> play CDG NG (PQ is None)
		// slimtype:READ_CD_FLAG::Raw  -> play CDG NG (PQ is OK)
		lpCmd[10] = READ_CD_FLAG::Pack;
	}
	else {
		lpCmd[10] = READ_CD_FLAG::Raw;
	}
}

VOID SetReadD8Command(
	PDEVICE_DATA pDevData,
	PEXT_ARG pExtArg,
	LPBYTE lpCmd,
	BYTE byTransferLen,
	BOOL bCheckReading
	)
{
	if (pDevData->bPlextor) {
		lpCmd[0] = SCSIOP_READ_D8;
		if (pExtArg != NULL && pExtArg->bFua) {
			lpCmd[1] = CDB_FORCE_MEDIA_ACCESS;
		}
		else {
			lpCmd[1] = 0x00;
		}
		lpCmd[8] = 0x00;
		lpCmd[9] = byTransferLen;
		if (!bCheckReading && pDevData->bC2ErrorData && 
			(pExtArg != NULL && pExtArg->bC2)) {
			lpCmd[10] = READ_D8_FLAG::MainAndC2AndSub96;
		}
		else {
			lpCmd[10] = READ_D8_FLAG::MainAndSub96;
		}
	}
}

VOID SetSubQDataFromReadCD(
	PSUB_Q_DATA pSubQ,
	PCD_OFFSET_DATA pCdOffsetData,
	LPBYTE lpBuf,
	LPBYTE lpSubcode
	)
{
	pSubQ->byCtl = (BYTE)((lpSubcode[12] >> 4) & 0x0F);
	pSubQ->byAdr = (BYTE)(lpSubcode[12] & 0x0F);
	pSubQ->byTrackNum = BcdToDec(lpSubcode[13]);
	pSubQ->byIndex = BcdToDec(lpSubcode[14]);
	pSubQ->nRelativeTime = MSFtoLBA(BcdToDec(lpSubcode[17]), 
		BcdToDec(lpSubcode[16]), BcdToDec(lpSubcode[15]));
	pSubQ->nAbsoluteTime = MSFtoLBA(BcdToDec(lpSubcode[21]), 
		BcdToDec(lpSubcode[20]), BcdToDec(lpSubcode[19]));
	pSubQ->byMode = GetMode(lpBuf + pCdOffsetData->uiMainDataSlideSize);
}

BOOL UpdateSubchannelQData(
	PSUB_Q_DATA pSubQ,
	PSUB_Q_DATA pPrevSubQ,
	PSUB_Q_DATA pPrevPrevSubQ
	)
{
	if (pPrevSubQ->byAdr != ADR_ENCODES_MEDIA_CATALOG && 
		pPrevSubQ->byAdr != ADR_ENCODES_ISRC) {
		pPrevPrevSubQ->byMode = pPrevSubQ->byMode;
		pPrevPrevSubQ->byAdr = pPrevSubQ->byAdr;
		pPrevPrevSubQ->nRelativeTime = pPrevSubQ->nRelativeTime;
	}
	else if ((pPrevSubQ->byAdr == ADR_ENCODES_MEDIA_CATALOG || 
		pPrevSubQ->byAdr == ADR_ENCODES_ISRC) && pPrevSubQ->byIndex == 0) {
			pSubQ->nRelativeTime = pPrevSubQ->nRelativeTime + 1;
	}

	if (pSubQ->byAdr != ADR_ENCODES_MEDIA_CATALOG && 
		pSubQ->byAdr != ADR_ENCODES_ISRC) {
		pPrevSubQ->byMode = pSubQ->byMode;
		pPrevSubQ->byAdr = pSubQ->byAdr;
	}
	else if (pPrevSubQ->byIndex == 0 && pPrevSubQ->nRelativeTime == 0) {
		pPrevSubQ->byIndex = 1;
	}

	pPrevPrevSubQ->byTrackNum = pPrevSubQ->byTrackNum;
	pPrevPrevSubQ->byIndex = pPrevSubQ->byIndex;
	pPrevPrevSubQ->byCtl = pPrevSubQ->byCtl;
	pPrevPrevSubQ->nAbsoluteTime = pPrevSubQ->nAbsoluteTime;
	pPrevSubQ->byTrackNum = pSubQ->byTrackNum;
	pPrevSubQ->byIndex = pSubQ->byIndex;
	pPrevSubQ->byCtl = pSubQ->byCtl;
	pPrevSubQ->nRelativeTime = pSubQ->nRelativeTime;
	pPrevSubQ->nAbsoluteTime++;

	return TRUE;
}
