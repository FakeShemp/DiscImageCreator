/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "convert.h"
#include "get.h"
#include "output.h"

BOOL PreserveTrackAttribution(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	INT nLBA,
	LPBYTE lpCurrentTrackNum,
	PMAIN_HEADER pMainHeader,
	PSUB_Q pSubQ,
	PSUB_Q pPrevSubQ
	)
{
	if (pSubQ->byTrackNum > 0) {
		INT tIdx = *lpCurrentTrackNum - 1;
		// preserve nLBA
		if (pPrevSubQ->byTrackNum + 1 == pSubQ->byTrackNum) {
			*lpCurrentTrackNum = pSubQ->byTrackNum;
			tIdx = *lpCurrentTrackNum - 1;
			if (pSubQ->byIndex > 0) {
				if (pDisc->SCSI.nFirstLBAof2ndSession == -1 &&
					pSubQ->byIndex == 1 && 
					nLBA != pDisc->SCSI.lpFirstLBAListOnToc[tIdx]) {
					OutputInfoLogA(
						"LBA[%06d, %#07x], Track[%02u]: Subchannel & TOC isn't sync. LBA on TOC[%d, %#x], index[%02u]\n",
						nLBA, nLBA, pSubQ->byTrackNum, pDisc->SCSI.lpFirstLBAListOnToc[tIdx], 
						pDisc->SCSI.lpFirstLBAListOnToc[tIdx], pSubQ->byIndex);
					pDisc->SUB.lpFirstLBAListOnSub[tIdx][1] = pDisc->SCSI.lpFirstLBAListOnToc[tIdx];
					if (pDisc->SUB.lpFirstLBAListOnSub[tIdx][0] == pDisc->SUB.lpFirstLBAListOnSub[tIdx][1]) {
						pDisc->SUB.lpFirstLBAListOnSub[tIdx][0] = -1;
					}
					pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][1] = nLBA;
					if (pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][0] == pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][1]) {
						pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][0] = -1;
					}
					pDisc->SUB.bDesync = TRUE;
				}
				else {
					pDisc->SUB.lpFirstLBAListOnSub[tIdx][pSubQ->byIndex] = nLBA;
					pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][pSubQ->byIndex] = nLBA;
				}
			}
			// preserve last LBA per data track
			if (pPrevSubQ->byTrackNum > 0) {
				if (pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[pPrevSubQ->byTrackNum - 1] != -1 &&
					pDisc->SUB.lpLastLBAListOfDataTrackOnSub[pPrevSubQ->byTrackNum - 1] == -1 &&
					(pPrevSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					pDisc->SUB.lpLastLBAListOfDataTrackOnSub[pPrevSubQ->byTrackNum - 1] = nLBA - 1;
				}
			}
		}
		// preserve mode, ctl
		if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[tIdx]) {
			pDisc->SUB.lpCtlList[tIdx] = pSubQ->byCtl;
			pDisc->MAIN.lpModeList[tIdx] = pMainHeader->byMode;
		}
		// preserve index
		if (pPrevSubQ->byIndex + 1 == pSubQ->byIndex && *lpCurrentTrackNum >= 0) {
			if (pSubQ->byIndex != 1) {
				pDisc->SUB.lpFirstLBAListOnSub[tIdx][pSubQ->byIndex] = nLBA;
				pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][pSubQ->byIndex] = nLBA;
			}
			else {
				if (nLBA != pDisc->SCSI.lpFirstLBAListOnToc[tIdx]) {
					OutputInfoLogA(
						"LBA[%06d, %#07x], Track[%02u]: Subchannel & TOC isn't sync. LBA on TOC[%d, %#x], prevIndex[%02u]\n",
						nLBA, nLBA, pSubQ->byTrackNum, pDisc->SCSI.lpFirstLBAListOnToc[tIdx],
						pDisc->SCSI.lpFirstLBAListOnToc[tIdx], pPrevSubQ->byIndex);
				}
				pDisc->SUB.lpFirstLBAListOnSub[tIdx][1] = pDisc->SCSI.lpFirstLBAListOnToc[tIdx];
				if (pDisc->SUB.lpFirstLBAListOnSub[tIdx][0] == pDisc->SUB.lpFirstLBAListOnSub[tIdx][1]) {
					// Crow, The - Original Motion Picture Soundtrack (82519-2)
					// LBA 108975, Track[06], Subchannel & TOC isn't sync. LBA on TOC: 108972, prevIndex[00]
					pDisc->SUB.lpFirstLBAListOnSub[tIdx][0] = -1;
				}
				pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][pSubQ->byIndex] = nLBA;
				if (pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][0] == pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][1]) {
					pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][0] = -1;
				}
			}
		}
		else if (pPrevSubQ->byIndex >= 1 && pSubQ->byIndex == 0) {
			pDisc->SUB.lpFirstLBAListOnSub[tIdx][pSubQ->byIndex] = nLBA;
			pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][pSubQ->byIndex] = nLBA;
		}

		if ((pDisc->SCSI.toc.TrackData[tIdx].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK &&
			(pSubQ->byCtl & AUDIO_DATA_TRACK) == 0) {
			OutputInfoLogA(
				"LBA[%06d, %#07x], Track[%02u]: Data track, but this sector is audio\n",
				nLBA, nLBA, pSubQ->byTrackNum);
		}
		else if ((pDisc->SCSI.toc.TrackData[tIdx].Control & AUDIO_DATA_TRACK) == 0 &&
			(pSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			OutputInfoLogA(
				"LBA[%06d, %#07x], Track[%02u]: Audio track, but this sector is data\n",
				nLBA, nLBA, pSubQ->byTrackNum);
		}

		if (pExtArg->bReverse) {
			// preserve last LBA per data track
			if (nLBA == pDisc->SCSI.nLastLBAofDataTrack) {
				if (pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tIdx] == -1 &&
					(pSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tIdx] = pDisc->SCSI.nLastLBAofDataTrack;
				}
			}
			else if ((pDisc->SCSI.toc.TrackData[tIdx].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[tIdx] = nLBA;
			}
		}
		else {
			// preserve first LBA per data track
			if (pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[tIdx] == -1 &&
				(pDisc->SCSI.toc.TrackData[tIdx].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[tIdx] = nLBA;
			}
			// preserve last LBA per data track
			else if (nLBA == pDisc->SCSI.nAllLength - 1) {
				if (pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tIdx] == -1) {
					if ((pSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
						pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tIdx] = pDisc->SCSI.nAllLength - 1;
					}
				}
			}
		}
	}
	return TRUE;
}

VOID SetAndOutputC2ErrorDataPerSector(
#if 0
	PC2_ERROR pC2Error,
#endif
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	INT nLBA,
	DWORD dwAllBufLen,
	UINT uiC2ErrorLBACnt
	)
{
	INT nBytePos = nLBA * CD_RAW_SECTOR_SIZE;
	INT nBytePosEnd = nBytePos + CD_RAW_SECTOR_SIZE - 1;
#if 0
	OutputErrorLogA(
		"LBA[%06d, %#07x], BytePos[%d-%d, %#x-%#x] C2 err exist. SlideSectorNum %d, ErrByteCnt %u\n"
		"                      [ErrOfs:BytePos(dec), ErrOfs:BytePos(hex)]\n",
		nLBA, nLBA, nBytePos, nBytePosEnd, nBytePos, nBytePosEnd, pC2Error->cSlideSectorNum,
		pC2ErrorPerSector[uiC2ErrorLBACnt].uiErrorBytePosCnt);
#else
	OutputErrorLogA(
		"LBA[%06d, %#07x], BytePos[%d-%d, %#x-%#x] C2 err exist. ErrByteCnt %u\n"
		"                      [ErrOfs:BytePos(dec), ErrOfs:BytePos(hex)]\n",
		nLBA, nLBA, nBytePos, nBytePosEnd, nBytePos, nBytePosEnd,
		pC2ErrorPerSector[uiC2ErrorLBACnt].uiErrorBytePosCnt);
#endif
	for (UINT n = 0; n < pC2ErrorPerSector[uiC2ErrorLBACnt].uiErrorBytePosCnt; n++) {
		INT nPos = nBytePos + pC2ErrorPerSector[uiC2ErrorLBACnt].lpErrorBytePos[n];
		OutputErrorLogA("                      [%u:%d, %#x:%#x]\n", 
			pC2ErrorPerSector[uiC2ErrorLBACnt].lpErrorBytePos[n], nPos, 
			pC2ErrorPerSector[uiC2ErrorLBACnt].lpErrorBytePos[n], nPos);
	}
	if (pC2ErrorPerSector[uiC2ErrorLBACnt].bErrorFlagBackup == RETURNED_NO_C2_ERROR_1ST ||
		pC2ErrorPerSector[uiC2ErrorLBACnt].bErrorFlagBackup == RETURNED_NO_C2_ERROR_BUT_BYTE_ERROR) {
		memcpy(pC2ErrorPerSector[uiC2ErrorLBACnt].lpBufC2NoneSector,
			pC2ErrorPerSector[uiC2ErrorLBACnt].lpBufC2NoneSectorBackup, dwAllBufLen);
	}
	pC2ErrorPerSector[uiC2ErrorLBACnt].bErrorFlag = RETURNED_EXIST_C2_ERROR;
	pC2ErrorPerSector[uiC2ErrorLBACnt].nErrorLBANum = nLBA;
}

VOID SetAndOutputC2NoErrorData(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	LPBYTE lpBuf,
	INT nLBA,
	DWORD dwAllBufLen,
	UINT uiC2ErrorLBACnt
	)
{
	OutputErrorLogA(
		"LBA[%06d, %#07x], C2 err doesn't exist. Next check 2352 byte.\n",
		nLBA, nLBA);
	memcpy(pC2ErrorPerSector[uiC2ErrorLBACnt].lpBufC2NoneSector, lpBuf, dwAllBufLen);
	pC2ErrorPerSector[uiC2ErrorLBACnt].bErrorFlag = RETURNED_NO_C2_ERROR_1ST;
	pC2ErrorPerSector[uiC2ErrorLBACnt].nErrorLBANum = nLBA;
}

VOID SetAndOutputC2NoErrorByteErrorData(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	LPBYTE lpBuf,
	INT nLBA,
	DWORD dwAllBufLen,
	UINT uiC2ErrorLBACnt
	)
{
	OutputErrorLogA(
		"LBA[%06d, %#07x], C2 err doesn't exist. But byte doesn't match\n",
		nLBA, nLBA);
	memcpy(pC2ErrorPerSector[uiC2ErrorLBACnt].lpBufC2NoneSector, lpBuf, dwAllBufLen);
	pC2ErrorPerSector[uiC2ErrorLBACnt].bErrorFlag = RETURNED_NO_C2_ERROR_BUT_BYTE_ERROR;
	pC2ErrorPerSector[uiC2ErrorLBACnt].nErrorLBANum = nLBA;
}

VOID SetC2ErrorBackup(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	UINT uiC2ErrorLBACntBackup,
	DWORD dwAllBufLen
	)
{
	for (UINT c = 0; c < uiC2ErrorLBACntBackup; c++) {
		pC2ErrorPerSector[c].bErrorFlagBackup = pC2ErrorPerSector[c].bErrorFlag;
		pC2ErrorPerSector[c].bErrorFlag = RETURNED_NO_C2_ERROR_1ST;
		pC2ErrorPerSector[c].nErrorLBANumBackup = pC2ErrorPerSector[c].nErrorLBANum;
		pC2ErrorPerSector[c].nErrorLBANum = 0;
		for (UINT d = 0; d < pC2ErrorPerSector[c].uiErrorBytePosCnt; d++) {
			pC2ErrorPerSector[c].lpErrorBytePosBackup[d] = pC2ErrorPerSector[c].lpErrorBytePos[d];
			pC2ErrorPerSector[c].lpErrorBytePos[d] = 0;
		}
		pC2ErrorPerSector[c].uiErrorBytePosCntBackup = pC2ErrorPerSector[c].uiErrorBytePosCnt;
		pC2ErrorPerSector[c].uiErrorBytePosCnt = 0;
		memcpy(pC2ErrorPerSector[c].lpBufC2NoneSectorBackup, 
			pC2ErrorPerSector[c].lpBufC2NoneSector, dwAllBufLen);
		ZeroMemory(pC2ErrorPerSector[c].lpBufC2NoneSector, dwAllBufLen);
	}
}

VOID SetAndOutputToc(
	PEXEC_TYPE pExecType,
	PDISC pDisc
	)
{
	OutputDiscLogA(
		"============================ TOC on SCSIOP_READ_TOC ===========================\n");
	CONST INT typeSize = 7;
	CHAR strType[typeSize] = { 0 };
	BOOL bFirstData = TRUE;
	BOOL bAudioOnly = TRUE;
	for (BYTE i = pDisc->SCSI.toc.FirstTrack; i <= pDisc->SCSI.toc.LastTrack; i++) {
		for (INT j = 0, k = 24; j < 4; j++, k -= 8) {
			pDisc->SCSI.lpFirstLBAListOnToc[i - 1] |= 
				pDisc->SCSI.toc.TrackData[i - 1].Address[j] << k;
			pDisc->SCSI.lpLastLBAListOnToc[i - 1] |= 
				pDisc->SCSI.toc.TrackData[i].Address[j] << k;
		}
		pDisc->SCSI.lpLastLBAListOnToc[i - 1] -= 1;
		pDisc->SCSI.nAllLength += 
			pDisc->SCSI.lpLastLBAListOnToc[i - 1] - pDisc->SCSI.lpFirstLBAListOnToc[i - 1] + 1;

		if ((pDisc->SCSI.toc.TrackData[i - 1].Control & AUDIO_DATA_TRACK) == 0) {
			strncpy(strType, " Audio", typeSize);
		}
		else if ((pDisc->SCSI.toc.TrackData[i - 1].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			strncpy(strType, "  Data", typeSize);
			if (bFirstData) {
				pDisc->SCSI.nFirstLBAofDataTrack = 
					pDisc->SCSI.lpFirstLBAListOnToc[i - 1];
				pDisc->SCSI.byFirstDataTrack = i;
				bFirstData = FALSE;
				bAudioOnly = FALSE;
			}
			pDisc->SCSI.nLastLBAofDataTrack = 
				pDisc->SCSI.lpLastLBAListOnToc[i - 1];
			pDisc->SCSI.byLastDataTrack = i;
		}
		if (i == pDisc->SCSI.toc.FirstTrack && 
			pDisc->SCSI.lpFirstLBAListOnToc[i - 1] > 0) {
			pDisc->SCSI.nAllLength += pDisc->SCSI.lpFirstLBAListOnToc[i - 1];
			OutputDiscLogA(
				"\tPregap Track   , LBA %8u-%8u, Length %8u\n",
				0, pDisc->SCSI.lpFirstLBAListOnToc[i - 1] - 1, 
				pDisc->SCSI.lpFirstLBAListOnToc[i - 1]);
		}
		OutputDiscLogA(
			"\t%s Track %2u, LBA %8u-%8u, Length %8u\n", 
			strType, i, pDisc->SCSI.lpFirstLBAListOnToc[i - 1],
			pDisc->SCSI.lpLastLBAListOnToc[i - 1],
			pDisc->SCSI.lpLastLBAListOnToc[i - 1] - pDisc->SCSI.lpFirstLBAListOnToc[i - 1] + 1);
	}
	OutputDiscLogA(
		"\t                                        Total  %8u\n", pDisc->SCSI.nAllLength);
	if (*pExecType != gd) {
		pDisc->SCSI.bAudioOnly = bAudioOnly;
	}
}

VOID SetAndOutputTocFull(
	PDISC pDisc,
	PCDROM_TOC_FULL_TOC_DATA fullToc,
	PCDROM_TOC_FULL_TOC_DATA_BLOCK pTocData,
	size_t uiTocEntries,
	FILE* fpCcd
	)
{
	OutputDiscLogA(
		"========================= FULL TOC on SCSIOP_READ_TOC =========================\n"
		"\tFirstCompleteSession: %u\n"
		"\t LastCompleteSession: %u\n",
		fullToc->FirstCompleteSession,
		fullToc->LastCompleteSession);
	BOOL bFirst2ndSession = TRUE;
	pDisc->SCSI.nFirstLBAofLeadout = -1;
	pDisc->SCSI.nFirstLBAof2ndSession = -1;

	for (UINT a = 0; a < uiTocEntries; a++) {
		INT nTmpLBAExt = 0;
		INT nTmpLBA = 0;
		if (fpCcd) {
			WriteCcdFileForEntry(pTocData, a, fpCcd);
		}
		switch (pTocData[a].Point) {
		case 0xa0:
			OutputDiscLogA("\tSession %u, FirstTrack %2u, ", 
				pTocData[a].SessionNumber, pTocData[a].Msf[0]);
			switch (pTocData[a].Msf[1]) {
				case 0x00:
					OutputDiscLogA("Format: CD-DA or CD-ROM\n");
					break;
				case 0x10:
					OutputDiscLogA("Format: CD-I\n");
					pDisc->SCSI.bCdi = TRUE;
					break;
				case 0x20:
					OutputDiscLogA("Format: CD-ROM-XA\n");
					break;
				default:
					OutputDiscLogA("Format: Other\n");
					break;
			}
			break;
		case 0xa1:
			OutputDiscLogA("\tSession %u,  LastTrack %2u\n", 
				pTocData[a].SessionNumber, pTocData[a].Msf[0]);
			break;
		case 0xa2:
			nTmpLBA = 
				MSFtoLBA(pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
			OutputDiscLogA(
				"\tSession %u,      Leadout, MSF %02u:%02u:%02u (LBA[%06d, %#07x])\n", 
				pTocData[a].SessionNumber, pTocData[a].Msf[0], pTocData[a].Msf[1],
				pTocData[a].Msf[2], nTmpLBA, nTmpLBA);
			pDisc->SCSI.lpLastLBAListOnToc[pDisc->SCSI.toc.LastTrack - 1] = nTmpLBA - 150 - 1;
			if (pTocData[a].SessionNumber == 1) {
				pDisc->SCSI.nFirstLBAofLeadout = nTmpLBA - 150;
			}
			break;
		case 0xb0: // (multi-session disc)
			nTmpLBAExt =
				MSFtoLBA(pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2]);
			nTmpLBA =
				MSFtoLBA(pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
			OutputDiscLogA(
				"\tSession %u,  NextSession, MSF %02u:%02u:%02u (LBA[%06d, %#07x])\n"
				"\t     Outermost Lead-out, MSF %02u:%02u:%02u (LBA[%06d, %#07x])\n" 
				"\tThe number of different Mode-5 pointers present %02u\n", 
				pTocData[a].SessionNumber, 
				pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2], 
				nTmpLBAExt, nTmpLBAExt,
				pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2],
				nTmpLBA, nTmpLBA, pTocData[a].Zero);
			break;
		case 0xb1: // (Audio only: This identifies the presence of skip intervals)
			OutputDiscLogA(
				"\tThe number of skip interval pointers %2u\n"
				"\tThe number of skip track assignments %2u\n",
				pTocData[a].Msf[0], pTocData[a].Msf[1]);
			break;
		case 0xb2: // (Audio only: This identifies tracks that should be skipped during playback)
		case 0xb3:
		case 0xb4:
			OutputDiscLogA(
				"\tTrack number to skip upon playback %2u\n"
				"\tTrack number to skip upon playback %2u\n"
				"\tTrack number to skip upon playback %2u\n"
				"\tTrack number to skip upon playback %2u\n"
				"\tTrack number to skip upon playback %2u\n"
				"\tTrack number to skip upon playback %2u\n",
				pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2],
				pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
			break;
		case 0xc0: // (Together with POINT=B0h, this is used to identify a multi-session disc)
			nTmpLBAExt =
				MSFtoLBA(pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2]);
			nTmpLBA =
				MSFtoLBA(pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
			OutputDiscLogA(
				"\tSession %u,  ATIP values, MSF %02u:%02u:%02u (LBA[%06d, %#07x])\n"
				"\t          First Lead-in, MSF %02u:%02u:%02u (LBA[%06d, %#07x])\n", 
				pTocData[a].SessionNumber, 
				pTocData[a].MsfExtra[0], pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2], 
				nTmpLBAExt, nTmpLBAExt,
				pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2],
				nTmpLBA, nTmpLBA);
			break;
		default:
			nTmpLBA = 
				MSFtoLBA(pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
			OutputDiscLogA(
				"\tSession %u,     Track %2u, MSF %02u:%02u:%02u (LBA[%06d, %#07x])\n", 
				pTocData[a].SessionNumber, pTocData[a].Point, pTocData[a].Msf[0], 
				pTocData[a].Msf[1], pTocData[a].Msf[2], nTmpLBA, nTmpLBA);
			if (pTocData[a].Point == 1) {
				pDisc->SCSI.lpFirstLBAListOnToc[pTocData[a].Point - 1] = nTmpLBA - 150;
			}
			else if (pTocData[a].Point >= 2 && pTocData[a].Point <= 100) {
				pDisc->SCSI.lpLastLBAListOnToc[pTocData[a].Point - 2] = nTmpLBA - 150 - 1;
				pDisc->SCSI.lpFirstLBAListOnToc[pTocData[a].Point - 1] = nTmpLBA - 150;
			}
			if (pTocData[a].SessionNumber == 2 && bFirst2ndSession) {
				pDisc->SCSI.nFirstLBAof2ndSession = nTmpLBA - 150;
				bFirst2ndSession = FALSE;
			}
			pDisc->SCSI.lpSessionNumList[pTocData[a].Point - 1] = pTocData[a].SessionNumber;
			break;
		}
	}
}

VOID SetAndOutputTocCDText(
	PDISC pDisc,
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	PCHAR pTmpText,
	WORD wTocTextEntries,
	WORD wAllTextSize
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

	for (size_t t = 0; t < wTocTextEntries; t++) {
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
	for (size_t z = 0; z < wTocTextEntries; z++) {
		if (uiIdx == wAllTextSize) {
			break;
		}
		size_t len1 = strlen(pTmpText + uiIdx);
		if (len1 == 0 || len1 >= META_CDTEXT_SIZE) {
			z--;
		}
		else {
			CHAR tmp[META_CDTEXT_SIZE] = { 0 };
			strncpy(tmp, pTmpText + uiIdx, len1);

			if (byAlbumCnt != 0 && z < byAlbumCnt) {
				strncpy(pDisc->SCSI.pszTitle[nTitleCnt], tmp, strlen(tmp));
				if (nTitleCnt == 0) {
					OutputDiscLogA(
						"\tAlbum Name: %s\n", 
						pDisc->SCSI.pszTitle[nTitleCnt]);
				}
				else {
					OutputDiscLogA(
						"\t Song Name: %s\n", 
						pDisc->SCSI.pszTitle[nTitleCnt]);
				}
				nTitleCnt++;
			}
			else if (byPerformerCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt)) {
				strncpy(pDisc->SCSI.pszPerformer[nPerformerCnt], tmp, strlen(tmp));
				if (nPerformerCnt == 0) {
					OutputDiscLogA(
						"\tAlbum Performer: %s\n", 
						pDisc->SCSI.pszPerformer[nPerformerCnt]);
				}
				else {
					OutputDiscLogA(
						"\t Song Performer: %s\n", 
						pDisc->SCSI.pszPerformer[nPerformerCnt]);
				}
				nPerformerCnt++;
			}
			else if (bySongwriterCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt)) {
				strncpy(pDisc->SCSI.pszSongWriter[nSongwriterCnt], tmp, strlen(tmp));
				if (nSongwriterCnt == 0) {
					OutputDiscLogA(
						"\tAlbum SongWriter: %s\n", 
						pDisc->SCSI.pszSongWriter[nSongwriterCnt]);
				}
				else {
					OutputDiscLogA(
						"\t      SongWriter: %s\n", 
						pDisc->SCSI.pszSongWriter[nSongwriterCnt]);
				}
				nSongwriterCnt++;
			}
			else if (byComposerCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt + byComposerCnt)) {
				OutputDiscLogA("\tComposer: %s\n", tmp);
			}
			else if (byArrangerCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt + byComposerCnt + byArrangerCnt)) {
				OutputDiscLogA("\tArranger: %s\n", tmp);
			}
			else if (byMessagesCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt)) {
				OutputDiscLogA("\tMessages: %s\n", tmp);
			}
			else if (byDiscIdCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt + byDiscIdCnt)) {
				OutputDiscLogA("\tDiscId: %s\n", tmp);
			}
			else if (byGenreCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt + byDiscIdCnt + byGenreCnt)) {
				OutputDiscLogA("\tGenre: %s\n", tmp);
			}
			else if (byUpcEanCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt + byDiscIdCnt +
				byGenreCnt + byTocInfoCnt + byTocInfo2Cnt + byUpcEanCnt)) {
				OutputDiscLogA("\tUpcEan: %s\n", tmp);
			}
		}
		uiIdx += len1 + 1;
	}
	INT nTocInfoCnt = 0;
	INT nSizeInfoCnt = 0;
	for (size_t z = 0; z <= bySizeInfoIdx; z++) {
		if (pDesc[z].PackType == CDROM_CD_TEXT_PACK_TOC_INFO) {
			// detail in Page 54-55 of EN 60908:1999
			OutputDiscLogA("\tTocInfo\n");
			if (nTocInfoCnt == 0) {
				OutputDiscLogA(
					"\t\t  First track number: %u\n"
					"\t\t   Last track number: %u\n"
					"\t\t   Lead-out(minutes): %u\n"
					"\t\t   Lead-out(seconds): %u\n"
					"\t\t    Lead-out(frames): %u\n",
					pDesc[z].Text[0],
					pDesc[z].Text[1],
					pDesc[z].Text[3],
					pDesc[z].Text[4],
					pDesc[z].Text[5]);
			}
			if (nTocInfoCnt == 1) {
				OutputDiscLogA(
					"\t\t    Track 1(minutes): %u\n"
					"\t\t    Track 1(seconds): %u\n"
					"\t\t     Track 1(frames): %u\n"
					"\t\t    Track 2(minutes): %u\n"
					"\t\t    Track 2(seconds): %u\n"
					"\t\t     Track 2(frames): %u\n"
					"\t\t    Track 3(minutes): %u\n"
					"\t\t    Track 3(seconds): %u\n"
					"\t\t     Track 3(frames): %u\n"
					"\t\t    Track 4(minutes): %u\n"
					"\t\t    Track 4(seconds): %u\n"
					"\t\t     Track 4(frames): %u\n",
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
			OutputDiscLogA("\tTocInfo2\n");
			OutputDiscLogA(
				"\t\t         Priority number: %u\n"
				"\t\t     Number of intervals: %u\n"
				"\t\t    Start point(minutes): %u\n"
				"\t\t    Start point(seconds): %u\n"
				"\t\t     Start point(frames): %u\n"
				"\t\t      End point(minutes): %u\n"
				"\t\t      End point(seconds): %u\n"
				"\t\t       End point(frames): %u\n",
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
			OutputDiscLogA("\tSizeInfo\n");
			if (nSizeInfoCnt == 0) {
				OutputDiscLogA(
					"\t\t  Charactor Code for this BLOCK: %u\n"
					"\t\t             First track Number: %u\n"
					"\t\t              Last track Number: %u\n"
					"\t\t  Mode2 & copy protection flags: %u\n"
					"\t\tNumber of PACKS with ALBUM_NAME: %u\n"
					"\t\t Number of PACKS with PERFORMER: %u\n"
					"\t\tNumber of PACKS with SONGWRITER: %u\n"
					"\t\t  Number of PACKS with COMPOSER: %u\n"
					"\t\t  Number of PACKS with ARRANGER: %u\n"
					"\t\t  Number of PACKS with MESSAGES: %u\n"
					"\t\t   Number of PACKS with DISC_ID: %u\n"
					"\t\t     Number of PACKS with GENRE: %u\n",
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[0],
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[1],
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[2],
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[3],
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[4],
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[5],
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[6],
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[7],
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[8],
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[9],
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[10],
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[11]);
			}
			else if (nSizeInfoCnt == 1) {
				OutputDiscLogA(
					"\t\t  Number of PACKS with TOC_INFO: %u\n"
					"\t\t Number of PACKS with TOC_INFO2: %u\n"
					"\t\t       Number of PACKS with $8a: %u\n"
					"\t\t       Number of PACKS with $8b: %u\n"
					"\t\t       Number of PACKS with $8c: %u\n"
					"\t\t       Number of PACKS with $8d: %u\n"
					"\t\t   Number of PACKS with UPC_EAN: %u\n"
					"\t\t Number of PACKS with SIZE_INFO: %u\n"
					"\t\tLast Sequence number of BLOCK 0: %u\n"
					"\t\tLast Sequence number of BLOCK 1: %u\n"
					"\t\tLast Sequence number of BLOCK 2: %u\n"
					"\t\tLast Sequence number of BLOCK 3: %u\n",
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[0],
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[1],
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[2],
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[3],
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[4],
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[5],
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[6],
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[7],
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[8],
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[9],
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[10],
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[11]);
			}
			else if (nSizeInfoCnt == 2) {
				OutputDiscLogA(
					"\t\tLast Sequence number of BLOCK 4: %u\n"
					"\t\tLast Sequence number of BLOCK 5: %u\n"
					"\t\tLast Sequence number of BLOCK 6: %u\n"
					"\t\tLast Sequence number of BLOCK 7: %u\n"
					"\t\t          Language code BLOCK 0: %u\n"
					"\t\t          Language code BLOCK 1: %u\n"
					"\t\t          Language code BLOCK 2: %u\n"
					"\t\t          Language code BLOCK 3: %u\n"
					"\t\t          Language code BLOCK 4: %u\n"
					"\t\t          Language code BLOCK 5: %u\n"
					"\t\t          Language code BLOCK 6: %u\n"
					"\t\t          Language code BLOCK 7: %u\n",
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[0],
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[1],
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[2],
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[3],
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[4],
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[5],
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[6],
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[7],
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[8],
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[9],
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[10],
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[11]);
			}
			nSizeInfoCnt++;
		}
	}
}

VOID SetAndOutputTocCDWText(
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	PCHAR pTmpText,
	WORD wFirstEntries,
	WORD wTocTextEntries,
	WORD wAllTextSize
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

	for (size_t t = wFirstEntries; t < wTocTextEntries; t++) {
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
			memcpy(pTmpText + 12 * (t - wFirstEntries), (WCHAR*)(pDesc[t].Text), 12);
		}
	}

	size_t uiIdx = 0;
	INT nTitleCnt = 0;
	INT nPerformerCnt = 0;
	INT nSongwriterCnt = 0;
	for (size_t z = 0; z < wTocTextEntries; z++) {
		if (uiIdx == wAllTextSize) {
			break;
		}
		size_t len1 = strlen(pTmpText + uiIdx);
		if (len1 == 0 || len1 >= META_CDTEXT_SIZE) {
			z--;
		}
		else {
			CHAR tmp[META_CDTEXT_SIZE] = { 0 };
			strncpy(tmp, pTmpText + uiIdx, len1);

			if (byAlbumCnt != 0 && z < byAlbumCnt) {
				if (nTitleCnt == 0) {
					OutputDiscLogA("\tAlbum Name: %s\n", tmp);
				}
				else {
					OutputDiscLogA("\t Song Name: %s\n", tmp);
				}
				nTitleCnt++;
			}
			else if (byPerformerCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt)) {
				if (nPerformerCnt == 0) {
					OutputDiscLogA("\tAlbum Performer: %s\n", tmp);
				}
				else {
					OutputDiscLogA("\t Song Performer: %s\n", tmp);
				}
				nPerformerCnt++;
			}
			else if (bySongwriterCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt)) {
				if (nSongwriterCnt == 0) {
					OutputDiscLogA("\tAlbum SongWriter: %s\n", tmp);
				}
				else {
					OutputDiscLogA("\t      SongWriter: %s\n", tmp);
				}
				nSongwriterCnt++;
			}
			else if (byComposerCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt + byComposerCnt)) {
				OutputDiscLogA("\tComposer: %s\n", tmp);
			}
			else if (byArrangerCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt + byComposerCnt + byArrangerCnt)) {
				OutputDiscLogA("\tArranger: %s\n", tmp);
			}
			else if (byMessagesCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt)) {
				OutputDiscLogA("\tMessages: %s\n", tmp);
			}
			else if (byDiscIdCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt + byDiscIdCnt)) {
				OutputDiscLogA("\tDiscId: %s\n", tmp);
			}
			else if (byGenreCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt + byDiscIdCnt + byGenreCnt)) {
				OutputDiscLogA("\tGenre: %s\n", tmp);
			}
			else if (byUpcEanCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt +
				byComposerCnt + byArrangerCnt + byMessagesCnt + byDiscIdCnt +
				byGenreCnt + byTocInfoCnt + byTocInfo2Cnt + byUpcEanCnt)) {
				OutputDiscLogA("\tUpcEan: %s\n", tmp);
			}
		}
		uiIdx += len1 + 1;
	}
	INT nTocInfoCnt = 0;
	INT nSizeInfoCnt = 0;
	for (size_t z = 0; z <= bySizeInfoIdx; z++) {
		if (pDesc[z].PackType == CDROM_CD_TEXT_PACK_TOC_INFO) {
			// detail in Page 54-55 of EN 60908:1999
			OutputDiscLogA("\tTocInfo\n");
			if (nTocInfoCnt == 0) {
				OutputDiscLogA(
					"\t\t  First track number: %u\n"
					"\t\t   Last track number: %u\n"
					"\t\t   Lead-out(minutes): %u\n"
					"\t\t   Lead-out(seconds): %u\n"
					"\t\t    Lead-out(frames): %u\n",
					pDesc[z].Text[0],
					pDesc[z].Text[1],
					pDesc[z].Text[3],
					pDesc[z].Text[4],
					pDesc[z].Text[5]);
			}
			if (nTocInfoCnt == 1) {
				OutputDiscLogA(
					"\t\t    Track 1(minutes): %u\n"
					"\t\t    Track 1(seconds): %u\n"
					"\t\t     Track 1(frames): %u\n"
					"\t\t    Track 2(minutes): %u\n"
					"\t\t    Track 2(seconds): %u\n"
					"\t\t     Track 2(frames): %u\n"
					"\t\t    Track 3(minutes): %u\n"
					"\t\t    Track 3(seconds): %u\n"
					"\t\t     Track 3(frames): %u\n"
					"\t\t    Track 4(minutes): %u\n"
					"\t\t    Track 4(seconds): %u\n"
					"\t\t     Track 4(frames): %u\n",
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
			OutputDiscLogA("\tTocInfo2\n");
			OutputDiscLogA(
				"\t\t         Priority number: %u\n"
				"\t\t     Number of intervals: %u\n"
				"\t\t    Start point(minutes): %u\n"
				"\t\t    Start point(seconds): %u\n"
				"\t\t     Start point(frames): %u\n"
				"\t\t      End point(minutes): %u\n"
				"\t\t      End point(seconds): %u\n"
				"\t\t       End point(frames): %u\n",
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
			OutputDiscLogA("\tSizeInfo\n");
			if (nSizeInfoCnt == 0) {
				OutputDiscLogA(
					"\t\t  Charactor Code for this BLOCK: %u\n"
					"\t\t             First track Number: %u\n"
					"\t\t              Last track Number: %u\n"
					"\t\t  Mode2 & copy protection flags: %u\n"
					"\t\tNumber of PACKS with ALBUM_NAME: %u\n"
					"\t\t Number of PACKS with PERFORMER: %u\n"
					"\t\tNumber of PACKS with SONGWRITER: %u\n"
					"\t\t  Number of PACKS with COMPOSER: %u\n"
					"\t\t  Number of PACKS with ARRANGER: %u\n"
					"\t\t  Number of PACKS with MESSAGES: %u\n"
					"\t\t   Number of PACKS with DISC_ID: %u\n"
					"\t\t     Number of PACKS with GENRE: %u\n",
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[0],
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[1],
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[2],
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[3],
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[4],
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[5],
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[6],
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[7],
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[8],
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[9],
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[10],
					pDesc[wTocTextEntries - bySizeInfoCnt].Text[11]);
			}
			else if (nSizeInfoCnt == 1) {
				OutputDiscLogA(
					"\t\t  Number of PACKS with TOC_INFO: %u\n"
					"\t\t Number of PACKS with TOC_INFO2: %u\n"
					"\t\t       Number of PACKS with $8a: %u\n"
					"\t\t       Number of PACKS with $8b: %u\n"
					"\t\t       Number of PACKS with $8c: %u\n"
					"\t\t       Number of PACKS with $8d: %u\n"
					"\t\t   Number of PACKS with UPC_EAN: %u\n"
					"\t\t Number of PACKS with SIZE_INFO: %u\n"
					"\t\tLast Sequence number of BLOCK 0: %u\n"
					"\t\tLast Sequence number of BLOCK 1: %u\n"
					"\t\tLast Sequence number of BLOCK 2: %u\n"
					"\t\tLast Sequence number of BLOCK 3: %u\n",
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[0],
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[1],
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[2],
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[3],
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[4],
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[5],
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[6],
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[7],
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[8],
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[9],
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[10],
					pDesc[wTocTextEntries - bySizeInfoCnt + 1].Text[11]);
			}
			else if (nSizeInfoCnt == 2) {
				OutputDiscLogA(
					"\t\tLast Sequence number of BLOCK 4: %u\n"
					"\t\tLast Sequence number of BLOCK 5: %u\n"
					"\t\tLast Sequence number of BLOCK 6: %u\n"
					"\t\tLast Sequence number of BLOCK 7: %u\n"
					"\t\t          Language code BLOCK 0: %u\n"
					"\t\t          Language code BLOCK 1: %u\n"
					"\t\t          Language code BLOCK 2: %u\n"
					"\t\t          Language code BLOCK 3: %u\n"
					"\t\t          Language code BLOCK 4: %u\n"
					"\t\t          Language code BLOCK 5: %u\n"
					"\t\t          Language code BLOCK 6: %u\n"
					"\t\t          Language code BLOCK 7: %u\n",
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[0],
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[1],
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[2],
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[3],
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[4],
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[5],
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[6],
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[7],
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[8],
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[9],
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[10],
					pDesc[wTocTextEntries - bySizeInfoCnt + 2].Text[11]);
			}
			nSizeInfoCnt++;
		}
	}
}

VOID SetFeatureCdRead(
	PFEATURE_DATA_CD_READ pCDRead,
	PDEVICE pDevice
	)
{
	pDevice->bCanCDText = (BOOL)(pCDRead->CDText);
	pDevice->bC2ErrorData = (BOOL)(pCDRead->C2ErrorData);
}

VOID SetCDOffset(
	PDISC pDisc,
	INT nStartLBA,
	INT nEndLBA
	)
{
	if (pDisc->MAIN.nCombinedOffset > 0) {
		pDisc->MAIN.uiMainDataSlideSize = 
			(size_t)pDisc->MAIN.nCombinedOffset % CD_RAW_SECTOR_SIZE;
		pDisc->MAIN.nOffsetStart = 0;
		pDisc->MAIN.nOffsetEnd = 
			pDisc->MAIN.nAdjustSectorNum;
		pDisc->MAIN.nFixStartLBA = 
			nStartLBA + pDisc->MAIN.nAdjustSectorNum - 1;
		pDisc->MAIN.nFixEndLBA = 
			nEndLBA + pDisc->MAIN.nAdjustSectorNum - 1;
		if (pDisc->SCSI.nFirstLBAof2ndSession != -1) {
			pDisc->MAIN.nFixFirstLBAofLeadout =
				pDisc->SCSI.nFirstLBAofLeadout + pDisc->MAIN.nAdjustSectorNum;
			pDisc->MAIN.nFixFirstLBAof2ndSession = 
				pDisc->SCSI.nFirstLBAof2ndSession + pDisc->MAIN.nAdjustSectorNum - 1;
		}
	}
	else if (pDisc->MAIN.nCombinedOffset < 0) {
		pDisc->MAIN.uiMainDataSlideSize = 
			(size_t)CD_RAW_SECTOR_SIZE + (pDisc->MAIN.nCombinedOffset % CD_RAW_SECTOR_SIZE);
		pDisc->MAIN.nOffsetStart = 
			pDisc->MAIN.nAdjustSectorNum;
		pDisc->MAIN.nOffsetEnd = 0;
		pDisc->MAIN.nFixStartLBA = 
			nStartLBA + pDisc->MAIN.nAdjustSectorNum;
		pDisc->MAIN.nFixEndLBA = 
			nEndLBA + pDisc->MAIN.nAdjustSectorNum;
		if (pDisc->SCSI.nFirstLBAof2ndSession != -1) {
			pDisc->MAIN.nFixFirstLBAofLeadout = 
				pDisc->SCSI.nFirstLBAofLeadout + pDisc->MAIN.nAdjustSectorNum + 1;
			pDisc->MAIN.nFixFirstLBAof2ndSession =
				pDisc->SCSI.nFirstLBAof2ndSession + pDisc->MAIN.nAdjustSectorNum;
		}
	}
}

VOID SetCDTransfer(
	PDEVICE pDevice,
	DRIVE_DATA_ORDER order
	)
{
	pDevice->TRANSFER.uiTransferLen = 1;
	if (order == DRIVE_DATA_ORDER::C2None) {
		pDevice->TRANSFER.dwBufLen = CD_RAW_SECTOR_WITH_SUBCODE_SIZE;
		pDevice->TRANSFER.dwAllBufLen = 
			pDevice->TRANSFER.dwBufLen * pDevice->TRANSFER.uiTransferLen;
		pDevice->TRANSFER.dwAdditionalBufLen = 0;
		pDevice->TRANSFER.dwBufC2Offset = 0;
		pDevice->TRANSFER.dwBufSubOffset = CD_RAW_SECTOR_SIZE;
	}
	else if (order == DRIVE_DATA_ORDER::MainC2Sub) {
		pDevice->TRANSFER.dwBufLen = CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE;
		if (pDevice->byPlexType == PLEX_DRIVE_TYPE::PXW4824) {
			pDevice->TRANSFER.dwAdditionalBufLen = 34; // unknown size
		}
		else {
			pDevice->TRANSFER.dwAdditionalBufLen = 0;
		}
		pDevice->TRANSFER.dwAllBufLen = 
			(pDevice->TRANSFER.dwBufLen + pDevice->TRANSFER.dwAdditionalBufLen) * pDevice->TRANSFER.uiTransferLen;
		pDevice->TRANSFER.dwBufC2Offset = CD_RAW_SECTOR_SIZE;
		pDevice->TRANSFER.dwBufSubOffset = CD_RAW_SECTOR_WITH_C2_294_SIZE;
	}
	else if (order == DRIVE_DATA_ORDER::MainSubC2) {
		pDevice->TRANSFER.dwBufLen = CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE;
		pDevice->TRANSFER.dwAdditionalBufLen = 0;
		pDevice->TRANSFER.dwAllBufLen =
			pDevice->TRANSFER.dwBufLen * pDevice->TRANSFER.uiTransferLen;
		pDevice->TRANSFER.dwBufSubOffset = CD_RAW_SECTOR_SIZE;
		pDevice->TRANSFER.dwBufC2Offset = CD_RAW_SECTOR_WITH_SUBCODE_SIZE;
	}
}

VOID SetISRCToString(
	PDISC pDisc,
	LPBYTE lpSubcode,
	LPSTR pszOutString,
	BYTE byTrackNum,
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
	_snprintf(pszOutString, META_ISRC_SIZE - 1, "%c%c%c%c%c%c%c%c%c%c%c%c",
		((lpSubcode[13] >> 2) & 0x3f) + 0x30, 
		(((lpSubcode[13] << 4) & 0x30) | ((lpSubcode[14] >> 4) & 0x0f)) + 0x30, 
		(((lpSubcode[14] << 2) & 0x3c) | ((lpSubcode[15] >> 6) & 0x03)) + 0x30, 
		(lpSubcode[15] & 0x3f) + 0x30, 
		((lpSubcode[16] >> 2) & 0x3f) + 0x30, 
		((lpSubcode[17] >> 4) & 0x0f) + 0x30, (lpSubcode[17] & 0x0f) + 0x30, 
		((lpSubcode[18] >> 4) & 0x0f) + 0x30, (lpSubcode[18] & 0x0f) + 0x30, 
		((lpSubcode[19] >> 4) & 0x0f) + 0x30, (lpSubcode[19] & 0x0f) + 0x30,
		((lpSubcode[20] >> 4) & 0x0f) + 0x30);
	pszOutString[META_ISRC_SIZE - 1] = 0;
	if (bCopy) {
		strncpy(pDisc->SUB.pszISRC[byTrackNum - 1], pszOutString, META_ISRC_SIZE);
	}
}

VOID SetMCNToString(
	PDISC pDisc,
	LPBYTE lpSubcode,
	LPSTR pszOutString,
	BOOL bCopy
	)
{
	_snprintf(pszOutString, META_CATALOG_SIZE - 1, "%c%c%c%c%c%c%c%c%c%c%c%c%c",
		((lpSubcode[13] >> 4) & 0x0f) + 0x30, (lpSubcode[13] & 0x0f) + 0x30, 
		((lpSubcode[14] >> 4) & 0x0f) + 0x30, (lpSubcode[14] & 0x0f) + 0x30, 
		((lpSubcode[15] >> 4) & 0x0f) + 0x30, (lpSubcode[15] & 0x0f) + 0x30, 
		((lpSubcode[16] >> 4) & 0x0f) + 0x30, (lpSubcode[16] & 0x0f) + 0x30, 
		((lpSubcode[17] >> 4) & 0x0f) + 0x30, (lpSubcode[17] & 0x0f) + 0x30, 
		((lpSubcode[18] >> 4) & 0x0f) + 0x30, (lpSubcode[18] & 0x0f) + 0x30, 
		((lpSubcode[19] >> 4) & 0x0f) + 0x30);
	pszOutString[META_CATALOG_SIZE - 1] = 0;
	if (bCopy) {
		strncpy(pDisc->SUB.szCatalog, pszOutString, META_CATALOG_SIZE);
	}
}

VOID SetReadCDCommand(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	CDB::_READ_CD* cdb,
	READ_CD_FLAG::EXPECTED_SECTOR_TYPE type,
	UINT uiTransferLen,
	READ_CD_FLAG::SUB_CHANNEL_SELECTION Sub,
	BOOL bCheckReading
	)
{
	cdb->OperationCode = SCSIOP_READ_CD;
	cdb->ExpectedSectorType = type;
	cdb->Lun = pDevice->address.Lun;
	cdb->TransferBlocks[0] = LOBYTE(HIWORD(uiTransferLen));
	cdb->TransferBlocks[1] = HIBYTE(LOWORD(uiTransferLen));
	cdb->TransferBlocks[2] = LOBYTE(LOWORD(uiTransferLen));
	if (!bCheckReading && pDevice->bC2ErrorData &&
		(pExtArg && pExtArg->bC2)) {
		cdb->ErrorFlags = READ_CD_FLAG::byte294;
	}
	cdb->IncludeEDC = TRUE;
	cdb->IncludeUserData = TRUE;
	cdb->HeaderCode = READ_CD_FLAG::BothHeader;
	cdb->IncludeSyncData = TRUE;
	cdb->SubChannelSelection = Sub;
}

VOID SetReadD8Command(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	CDB::_PLXTR_READ_CDDA* cdb,
	UINT uiTransferLen,
	BOOL bSub,
	BOOL bCheckReading
	)
{
	cdb->OperationCode = SCSIOP_PLEX_READ_CD;
	cdb->LogicalUnitNumber = pDevice->address.Lun;
	cdb->TransferBlockByte0 = HIBYTE(HIWORD(uiTransferLen));
	cdb->TransferBlockByte1 = LOBYTE(HIWORD(uiTransferLen));
	cdb->TransferBlockByte2 = HIBYTE(LOWORD(uiTransferLen));
	cdb->TransferBlockByte3 = LOBYTE(LOWORD(uiTransferLen));
	if (bSub) {
		if (!bCheckReading && pDevice->bC2ErrorData &&
			(pExtArg && pExtArg->bC2)) {
			cdb->SubCode = READ_D8_FLAG::MainC2Raw;
		}
		else {
			cdb->SubCode = READ_D8_FLAG::MainPack;
		}
	}
	else {
		cdb->SubCode = READ_D8_FLAG::SubNone;
	}
}

VOID SetSubQDataFromReadCD(
	WORD wDriveBufSize,
	UINT uiMainDataSlideSize,
	PMAIN_HEADER pMainHeader,
	PSUB_Q pSubQ,
	LPBYTE lpBuf,
	LPBYTE lpSubcode
	)
{
	pSubQ->byCtl = (BYTE)((lpSubcode[12] >> 4) & 0x0f);
	pSubQ->byAdr = (BYTE)(lpSubcode[12] & 0x0f);
	pSubQ->byTrackNum = BcdToDec(lpSubcode[13]);
	pSubQ->byIndex = BcdToDec(lpSubcode[14]);
	pSubQ->nRelativeTime = MSFtoLBA(BcdToDec(lpSubcode[15]), 
		BcdToDec(lpSubcode[16]), BcdToDec(lpSubcode[17]));
	pSubQ->nAbsoluteTime = MSFtoLBA(BcdToDec(lpSubcode[19]), 
		BcdToDec(lpSubcode[20]), BcdToDec(lpSubcode[21]));
	if (wDriveBufSize > MINIMUM_DRIVE_BUF_SIZE) {
		pMainHeader->byMode =
			GetMode(lpBuf + uiMainDataSlideSize, pSubQ->byCtl);
	}
	else {
		pMainHeader->byMode = (BYTE)(pSubQ->byCtl & 0x0f);
	}
}

BOOL UpdateSubQData(
	PSUB_Q pSubQ,
	PSUB_Q pPrevSubQ,
	PSUB_Q pPrevPrevSubQ,
	BOOL bLibCrypt
	)
{
	if (pPrevSubQ->byAdr != ADR_ENCODES_MEDIA_CATALOG && 
		pPrevSubQ->byAdr != ADR_ENCODES_ISRC) {
		pPrevPrevSubQ->byAdr = pPrevSubQ->byAdr;
		pPrevPrevSubQ->nRelativeTime = pPrevSubQ->nRelativeTime;
	}
	else if ((pPrevSubQ->byAdr == ADR_ENCODES_MEDIA_CATALOG || 
		pPrevSubQ->byAdr == ADR_ENCODES_ISRC) && pPrevSubQ->byIndex == 0) {
			pSubQ->nRelativeTime = pPrevSubQ->nRelativeTime + 1;
	}

	if (pSubQ->byAdr != ADR_ENCODES_MEDIA_CATALOG && 
		pSubQ->byAdr != ADR_ENCODES_ISRC) {
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
	if (bLibCrypt) {
		pPrevSubQ->nRelativeTime++;
	}
	else {
		pPrevSubQ->nRelativeTime = pSubQ->nRelativeTime;
	}
	pPrevSubQ->nAbsoluteTime++;

	return TRUE;
}

VOID UpdateTmpMainHeader(
	PMAIN_HEADER pMainHeader
	)
{
	BYTE tmp = (BYTE)(pMainHeader->header[14] + 1);
	if ((tmp & 0x0f) == 0x0a) {
		tmp += 6;
	}
	if (tmp == 0x75) {
		pMainHeader->header[14] = 0;
		tmp = (BYTE)((pMainHeader->header[13] ^ 0x80) + 1);
		if ((tmp & 0x0f) == 0x0a) {
			tmp += 6;
		}
		if (tmp == 0x60) {
			pMainHeader->header[13] = 0x80;
			tmp = (BYTE)((pMainHeader->header[12] ^ 0x01) + 1);
			if ((tmp & 0x0f) == 0x0a) {
				tmp += 6;
			}
			pMainHeader->header[12] = (BYTE)(tmp ^ 0x01);
		}
		else {
			pMainHeader->header[13] = (BYTE)(tmp ^ 0x80);
		}
	}
	else {
		pMainHeader->header[14] = tmp;
	}
}
