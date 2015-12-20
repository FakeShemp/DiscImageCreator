/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "check.h"
#include "convert.h"
#include "get.h"
#include "output.h"
#include "_external\crc16ccitt.h"

VOID PreserveTrackAttribution(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	INT nLBA,
	LPBYTE lpCurrentTrackNum,
	PMAIN_HEADER pMain,
	PSUB_Q pSubQ
	)
{
	if (0 <= nLBA && nLBA < pDisc->SCSI.nAllLength && pSubQ->present.byTrackNum > 0) {
		INT tIdx = *lpCurrentTrackNum - 1;
		// preserve nLBA
		if (pSubQ->prev.byTrackNum + 1 == pSubQ->present.byTrackNum) {
			*lpCurrentTrackNum = pSubQ->present.byTrackNum;
			tIdx = *lpCurrentTrackNum - 1;
			if (pSubQ->present.byIndex > 0) {
				if (pDisc->SCSI.nFirstLBAof2ndSession == -1 &&
					pSubQ->present.byIndex == 1 &&
					nLBA != pDisc->SCSI.lpFirstLBAListOnToc[tIdx]) {
					OutputSubInfoLogA(
						"LBA[%06d, %#07x], Track[%02u]: Subchannel & TOC isn't sync. LBA on TOC[%d, %#x], index[%02u]\n",
						nLBA, nLBA, pSubQ->present.byTrackNum, pDisc->SCSI.lpFirstLBAListOnToc[tIdx],
						pDisc->SCSI.lpFirstLBAListOnToc[tIdx], pSubQ->present.byIndex);
					pDisc->SUB.lpFirstLBAListOnSub[tIdx][1] = pDisc->SCSI.lpFirstLBAListOnToc[tIdx];
					if (pDisc->SUB.lpFirstLBAListOnSub[tIdx][0] == pDisc->SUB.lpFirstLBAListOnSub[tIdx][1]) {
						pDisc->SUB.lpFirstLBAListOnSub[tIdx][0] = -1;
					}
					pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][1] = nLBA;
					if (pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][0] == pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][1]) {
						pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][0] = -1;
					}
					pDisc->SUB.byDesync = TRUE;
				}
				else {
					pDisc->SUB.lpFirstLBAListOnSub[tIdx][pSubQ->present.byIndex] = nLBA;
					pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][pSubQ->present.byIndex] = nLBA;
				}
			}
			// preserve last LBA per data track
			if (pSubQ->prev.byTrackNum > 0) {
				if (pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[pSubQ->prev.byTrackNum - 1] != -1 &&
					pDisc->SUB.lpLastLBAListOfDataTrackOnSub[pSubQ->prev.byTrackNum - 1] == -1 &&
					(pSubQ->prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					pDisc->SUB.lpLastLBAListOfDataTrackOnSub[pSubQ->prev.byTrackNum - 1] = nLBA - 1;
				}
			}
		}
		// preserve mode, ctl
		if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[tIdx]) {
			pDisc->SUB.lpCtlList[tIdx] = pSubQ->present.byCtl;
			pDisc->MAIN.lpModeList[tIdx] = GetMode((LPBYTE)pMain->present, pMain->prev[15], pSubQ->present.byCtl, UNSCRAMBLED);
		}
		// preserve index
		if (pSubQ->prev.byIndex + 1 == pSubQ->present.byIndex && *lpCurrentTrackNum >= 0) {
			if (pSubQ->present.byIndex != 1) {
				pDisc->SUB.lpFirstLBAListOnSub[tIdx][pSubQ->present.byIndex] = nLBA;
				pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][pSubQ->present.byIndex] = nLBA;
			}
			else {
				if (nLBA != pDisc->SCSI.lpFirstLBAListOnToc[tIdx]) {
					OutputSubInfoLogA(
						"LBA[%06d, %#07x], Track[%02u]: Subchannel & TOC isn't sync. LBA on TOC[%d, %#x], prevIndex[%02u]\n",
						nLBA, nLBA, pSubQ->present.byTrackNum, pDisc->SCSI.lpFirstLBAListOnToc[tIdx],
						pDisc->SCSI.lpFirstLBAListOnToc[tIdx], pSubQ->prev.byIndex);
				}
				pDisc->SUB.lpFirstLBAListOnSub[tIdx][1] = pDisc->SCSI.lpFirstLBAListOnToc[tIdx];
				if (pDisc->SUB.lpFirstLBAListOnSub[tIdx][0] == pDisc->SUB.lpFirstLBAListOnSub[tIdx][1]) {
					// Crow, The - Original Motion Picture Soundtrack (82519-2)
					// LBA 108975, Track[06], Subchannel & TOC isn't sync. LBA on TOC: 108972, prevIndex[00]
					pDisc->SUB.lpFirstLBAListOnSub[tIdx][0] = -1;
				}
				pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][pSubQ->present.byIndex] = nLBA;
				if (pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][0] == pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][1]) {
					pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][0] = -1;
				}
			}
		}
		else if (pSubQ->prev.byIndex >= 1 && pSubQ->present.byIndex == 0) {
			pDisc->SUB.lpFirstLBAListOnSub[tIdx][pSubQ->present.byIndex] = nLBA;
			pDisc->SUB.lpFirstLBAListOnSubSync[tIdx][pSubQ->present.byIndex] = nLBA;
		}

		if ((pDisc->SCSI.toc.TrackData[tIdx].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK &&
			(pSubQ->present.byCtl & AUDIO_DATA_TRACK) == 0) {
			OutputMainInfoLogA(
				"LBA[%06d, %#07x], Track[%02u]: Data track, but this sector is audio\n",
				nLBA, nLBA, pSubQ->present.byTrackNum);
		}
		else if ((pDisc->SCSI.toc.TrackData[tIdx].Control & AUDIO_DATA_TRACK) == 0 &&
			(pSubQ->present.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			OutputMainInfoLogA(
				"LBA[%06d, %#07x], Track[%02u]: Audio track, but this sector is data\n",
				nLBA, nLBA, pSubQ->present.byTrackNum);
		}

		if (pExtArg->byReverse) {
			// preserve last LBA per data track
			if (nLBA == pDisc->SCSI.nLastLBAofDataTrack) {
				if (pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tIdx] == -1 &&
					(pSubQ->present.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
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
					if ((pSubQ->present.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
						pDisc->SUB.lpLastLBAListOfDataTrackOnSub[tIdx] = pDisc->SCSI.nAllLength - 1;
					}
				}
			}
		}
	}
}

VOID SetC2ErrorDataDetail(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	INT nLBA,
	PUINT puiC2ErrorLBACnt
	)
{
	BOOL bSame = FALSE;
	for (UINT i = 0; i < *puiC2ErrorLBACnt; i++) {
		if (pC2ErrorPerSector[i].nErrorLBANum == nLBA) {
			bSame = TRUE;
		}
	}
	if (!bSame) {
		pC2ErrorPerSector[*puiC2ErrorLBACnt].byErrorFlag = RETURNED_EXIST_C2_ERROR;
		pC2ErrorPerSector[*puiC2ErrorLBACnt].nErrorLBANum = nLBA;
		(*puiC2ErrorLBACnt)++;
	}
}

VOID SetC2ErrorData(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	INT nLBA,
	PUINT puiC2ErrorLBACnt,
	BOOL b1stRead
	)
{
	INT nBytePos = nLBA * CD_RAW_SECTOR_SIZE;
	INT nBytePosEnd = nBytePos + CD_RAW_SECTOR_SIZE - 1;
	OutputC2ErrorLogA(
		"LBA[%06d, %#07x], BytePos[%d-%d, %#x-%#x] C2 err exist. ErrByteCnt %u\n"
		"                      [ErrOfs:BytePos(dec), ErrOfs:BytePos(hex)]\n",
		nLBA, nLBA, nBytePos, nBytePosEnd, nBytePos, nBytePosEnd,
		pC2ErrorPerSector[*puiC2ErrorLBACnt].uiErrorBytePosCnt);
	UINT n = 0;
	for (;  n < pC2ErrorPerSector[*puiC2ErrorLBACnt].uiErrorBytePosCnt; n++) {
		INT nPos = nBytePos + pC2ErrorPerSector[*puiC2ErrorLBACnt].lpErrorBytePos[n];
		OutputC2ErrorLogA("                      [%d:%d, %#x:%#x]\n", 
			pC2ErrorPerSector[*puiC2ErrorLBACnt].lpErrorBytePos[n], nPos,
			pC2ErrorPerSector[*puiC2ErrorLBACnt].lpErrorBytePos[n], nPos);
	}
	if (b1stRead) {
		SetC2ErrorDataDetail(pC2ErrorPerSector, nLBA - 1, puiC2ErrorLBACnt);
		SetC2ErrorDataDetail(pC2ErrorPerSector, nLBA, puiC2ErrorLBACnt);
	}
	else {
		SetC2ErrorDataDetail(pC2ErrorPerSector, nLBA, puiC2ErrorLBACnt);
	}
}

VOID SetNoC2ErrorData(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	LPBYTE lpBuf,
	INT nLBA,
	DWORD dwAllBufLen,
	PUINT puiC2ErrorLBACnt
	)
{
	OutputC2ErrorLogA(
		"LBA[%06d, %#07x], C2 err doesn't exist. Next check 2352 byte.\n",
		nLBA, nLBA);
	memcpy(pC2ErrorPerSector[*puiC2ErrorLBACnt].lpBufNoC2Sector, lpBuf, dwAllBufLen);
	pC2ErrorPerSector[*puiC2ErrorLBACnt].byErrorFlag = RETURNED_NO_C2_ERROR_1ST;
	pC2ErrorPerSector[*puiC2ErrorLBACnt].nErrorLBANum = nLBA;
	(*puiC2ErrorLBACnt)++;
}

VOID SetNoC2ErrorExistsByteErrorData(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	INT nLBA,
	PUINT puiC2ErrorLBACnt
	)
{
	OutputC2ErrorLogA(
		"LBA[%06d, %#07x], C2 err doesn't exist. But byte doesn't match\n",
		nLBA, nLBA);
	pC2ErrorPerSector[*puiC2ErrorLBACnt].byErrorFlag = RETURNED_NO_C2_ERROR_BUT_BYTE_ERROR;
	pC2ErrorPerSector[*puiC2ErrorLBACnt].nErrorLBANum = nLBA;
	(*puiC2ErrorLBACnt)++;
}

VOID SetC2ErrorBackup(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	UINT uiC2ErrorLBACntBackup,
	DWORD dwAllBufLen
	)
{
	for (UINT c = 0; c < uiC2ErrorLBACntBackup; c++) {
		pC2ErrorPerSector[c].byErrorFlagBackup = pC2ErrorPerSector[c].byErrorFlag;
		pC2ErrorPerSector[c].byErrorFlag = RETURNED_NO_C2_ERROR_1ST;
		pC2ErrorPerSector[c].nErrorLBANumBackup = pC2ErrorPerSector[c].nErrorLBANum;
		pC2ErrorPerSector[c].nErrorLBANum = 0;
		for (UINT d = 0; d < pC2ErrorPerSector[c].uiErrorBytePosCnt; d++) {
			pC2ErrorPerSector[c].lpErrorBytePosBackup[d] = pC2ErrorPerSector[c].lpErrorBytePos[d];
			pC2ErrorPerSector[c].lpErrorBytePos[d] = 0;
		}
		pC2ErrorPerSector[c].uiErrorBytePosCntBackup = pC2ErrorPerSector[c].uiErrorBytePosCnt;
		pC2ErrorPerSector[c].uiErrorBytePosCnt = 0;
		memcpy(pC2ErrorPerSector[c].lpBufNoC2SectorBackup, 
			pC2ErrorPerSector[c].lpBufNoC2Sector, dwAllBufLen);
		ZeroMemory(pC2ErrorPerSector[c].lpBufNoC2Sector, dwAllBufLen);
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
	BYTE byAudioOnly = TRUE;
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
				byAudioOnly = FALSE;
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
			"\t%s Track %2u, LBA %8u-%8u, Length %8u\n", strType, i,
			pDisc->SCSI.lpFirstLBAListOnToc[i - 1], pDisc->SCSI.lpLastLBAListOnToc[i - 1],
			pDisc->SCSI.lpLastLBAListOnToc[i - 1] - pDisc->SCSI.lpFirstLBAListOnToc[i - 1] + 1);
	}
	OutputDiscLogA(
		"\t                                        Total  %8u\n", pDisc->SCSI.nAllLength);
	if (*pExecType != gd) {
		pDisc->SCSI.byAudioOnly = byAudioOnly;
	}
}

VOID SetAndOutputTocFull(
	PDISC pDisc,
	PCDROM_TOC_FULL_TOC_DATA fullToc,
	PCDROM_TOC_FULL_TOC_DATA_BLOCK pTocData,
	WORD wTocEntries,
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

	for (WORD a = 0; a < wTocEntries; a++) {
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
					pDisc->SCSI.byCdi = TRUE;
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
	pDevice->FEATURE.byCanCDText = pCDRead->CDText;
	pDevice->FEATURE.byC2ErrorData = pCDRead->C2ErrorData;
}

VOID SetFeatureRealTimeStreaming(
	PFEATURE_DATA_REAL_TIME_STREAMING pRTS,
	PDEVICE pDevice
	)
{
	pDevice->FEATURE.byModePage2a = pRTS->WriteSpeedInMP2A;
	pDevice->FEATURE.bySetCDSpeed = pRTS->SetCDSpeed;
	pDevice->FEATURE.byReadBufCapa = pRTS->ReadBufferCapacityBlock;
}

VOID SetCDOffset(
	BYTE byBe,
	PDISC pDisc,
	INT nStartLBA,
	INT nEndLBA
	)
{
	if (pDisc->MAIN.nCombinedOffset > 0) {
		if (byBe && !pDisc->SCSI.byAudioOnly) {
			pDisc->MAIN.uiMainDataSlideSize = 0;
			pDisc->MAIN.nOffsetStart = 0;
			pDisc->MAIN.nOffsetEnd = 0;
			pDisc->MAIN.nFixStartLBA = 0;
			pDisc->MAIN.nFixEndLBA = pDisc->SCSI.nAllLength;
		}
		else {
			pDisc->MAIN.uiMainDataSlideSize =
				(size_t)pDisc->MAIN.nCombinedOffset % CD_RAW_SECTOR_SIZE;
			pDisc->MAIN.nOffsetStart = 0;
			pDisc->MAIN.nOffsetEnd =
				pDisc->MAIN.nAdjustSectorNum;
			pDisc->MAIN.nFixStartLBA =
				nStartLBA + pDisc->MAIN.nAdjustSectorNum - 1;
			pDisc->MAIN.nFixEndLBA =
				nEndLBA + pDisc->MAIN.nAdjustSectorNum - 1;
		}
		if (pDisc->SCSI.nFirstLBAof2ndSession != -1) {
			pDisc->MAIN.nFixFirstLBAofLeadout =
				pDisc->SCSI.nFirstLBAofLeadout + pDisc->MAIN.nAdjustSectorNum;
			pDisc->MAIN.nFixFirstLBAof2ndSession = 
				pDisc->SCSI.nFirstLBAof2ndSession + pDisc->MAIN.nAdjustSectorNum - 1;
		}
	}
	else if (pDisc->MAIN.nCombinedOffset < 0) {
		if (byBe && !pDisc->SCSI.byAudioOnly) {
			pDisc->MAIN.uiMainDataSlideSize = 0;
			pDisc->MAIN.nOffsetStart = 0;
			pDisc->MAIN.nOffsetEnd = 0;
			pDisc->MAIN.nFixStartLBA = 0;
			pDisc->MAIN.nFixEndLBA = pDisc->SCSI.nAllLength;
		}
		else {
			pDisc->MAIN.uiMainDataSlideSize =
				(size_t)CD_RAW_SECTOR_SIZE + (pDisc->MAIN.nCombinedOffset % CD_RAW_SECTOR_SIZE);
			pDisc->MAIN.nOffsetStart =
				pDisc->MAIN.nAdjustSectorNum;
			pDisc->MAIN.nOffsetEnd = 0;
			pDisc->MAIN.nFixStartLBA =
				nStartLBA + pDisc->MAIN.nAdjustSectorNum;
			pDisc->MAIN.nFixEndLBA =
				nEndLBA + pDisc->MAIN.nAdjustSectorNum;
		}
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
	if (order == DRIVE_DATA_ORDER::NoC2) {
		pDevice->TRANSFER.dwBufLen = CD_RAW_SECTOR_WITH_SUBCODE_SIZE;
		pDevice->TRANSFER.dwAllBufLen = 
			pDevice->TRANSFER.dwBufLen * pDevice->TRANSFER.uiTransferLen;
		pDevice->TRANSFER.dwAdditionalBufLen = 0;
		pDevice->TRANSFER.dwBufC2Offset = 0;
		pDevice->TRANSFER.dwBufSubOffset = CD_RAW_SECTOR_SIZE;
	}
	else if (order == DRIVE_DATA_ORDER::MainC2Sub) {
		pDevice->TRANSFER.dwBufLen = CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE;
		if (pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PXW4824A) {
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

VOID SetFirstAdrSector(
	INT nFirstLBA[][2],
	INT nRangeLBA[][2],
	LPSTR strAdr,
	LPINT nAdrLBAList,
	BYTE bySessionIdx,
	BYTE byPlxtrType
	)
{
	INT first = nAdrLBAList[1] - nAdrLBAList[0];
	INT second = nAdrLBAList[2] - nAdrLBAList[1];
	INT third = nAdrLBAList[3] - nAdrLBAList[2];
	INT fourth = nAdrLBAList[4] - nAdrLBAList[3];
	INT betweenThirdOne = nAdrLBAList[2] - nAdrLBAList[0];
	INT betweenTourthTwo = nAdrLBAList[3] - nAdrLBAList[1];
	INT betweenFifthThird = nAdrLBAList[4] - nAdrLBAList[2];
	if (first == second && first == third) {
		nFirstLBA[0][bySessionIdx] = nAdrLBAList[0];
		nRangeLBA[0][bySessionIdx] = nAdrLBAList[1] - nAdrLBAList[0];
		if (byPlxtrType == PLXTR_DRIVE_TYPE::PXS88T) {
			// Somehow PX-S88T is sliding subchannel +1;
			nFirstLBA[0][bySessionIdx]++;
		}
		OutputDiscLogA(
			"\tSession %d, 1st %s sector is %d, %s sector exists per %d\n"
			, bySessionIdx + 1, strAdr, nFirstLBA[0][bySessionIdx]
			, strAdr, nRangeLBA[0][bySessionIdx]);
	}
	else if (second == third && third == fourth) {
		// Originally, MCN sector exists per same frame number, but in case of 1st sector or next idx of the track, MCN sector slides at the next sector
		//
		// Shake the fake [Kyosuke Himuro] [First MCN Sector: 1, MCN sector exists per 91 frame]
		// LBA[000000, 0000000], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:00:00], AMSF[00:02:00], RtoW[0, 0, 0, 0]
		// LBA[000001, 0x00001], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [4988006116269], AMSF[     :01], RtoW[0, 0, 0, 0]
		// LBA[000002, 0x00002], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:00:02], AMSF[00:02:02], RtoW[0, 0, 0, 0]
		//  :
		// LBA[000090, 0x0005a], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:01:15], AMSF[00:03:15], RtoW[0, 0, 0, 0]
		// LBA[000091, 0x0005b], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [4988006116269], AMSF[     :16], RtoW[0, 0, 0, 0]
		// LBA[000092, 0x0005c], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:01:17], AMSF[00:03:17], RtoW[0, 0, 0, 0]
		//  :
		// LBA[000181, 0x000b5], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:02:31], AMSF[00:04:31], RtoW[0, 0, 0, 0]
		// LBA[000182, 0x000b6], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [4988006116269], AMSF[     :32], RtoW[0, 0, 0, 0]
		// LBA[000183, 0x000b7], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:02:33], AMSF[00:04:33], RtoW[0, 0, 0, 0]
		nFirstLBA[0][bySessionIdx] = nAdrLBAList[0] - 1;
		nRangeLBA[0][bySessionIdx] = nAdrLBAList[1] - nAdrLBAList[0] + 1;
		if (byPlxtrType == PLXTR_DRIVE_TYPE::PXS88T) {
			// Somehow PX-S88T is sliding subchannel +1;
			nFirstLBA[0][bySessionIdx]++;
		}
		OutputDiscLogA(
			"\tSession %d, 1st %s sector is %d, %s sector exists per %d\n"
			, bySessionIdx + 1, strAdr, nFirstLBA[0][bySessionIdx]
			, strAdr, nRangeLBA[0][bySessionIdx]);
	}
	else if (betweenThirdOne == betweenTourthTwo && betweenThirdOne == betweenFifthThird) {
		nFirstLBA[0][bySessionIdx] = nAdrLBAList[0];
		nRangeLBA[0][bySessionIdx] = betweenThirdOne;
		nFirstLBA[1][bySessionIdx] = nAdrLBAList[1];
		nRangeLBA[1][bySessionIdx] = betweenTourthTwo;
		if (byPlxtrType == PLXTR_DRIVE_TYPE::PXS88T) {
			// Somehow PX-S88T is sliding subchannel +1;
			nFirstLBA[0][bySessionIdx]++;
			nFirstLBA[1][bySessionIdx]++;
		}
		OutputDiscLogA(
			"\tSession %d, 1st %s sector is %d, %s sector exists per %d\n"
			"\t            2nd %s sector is %d, %s sector exists per %d\n"
			, bySessionIdx + 1
			, strAdr, nFirstLBA[0][bySessionIdx], strAdr, nRangeLBA[0][bySessionIdx]
			, strAdr, nFirstLBA[1][bySessionIdx], strAdr, nRangeLBA[1][bySessionIdx]);
	}
	else if (first == second || first == third || second == third) {
		nFirstLBA[0][bySessionIdx] = nAdrLBAList[0];
		nRangeLBA[0][bySessionIdx] = nAdrLBAList[1] - nAdrLBAList[0];
		nFirstLBA[1][bySessionIdx] = nAdrLBAList[0];
		nRangeLBA[1][bySessionIdx] = nAdrLBAList[2] - nAdrLBAList[1];
		nFirstLBA[2][bySessionIdx] = nAdrLBAList[0];
		nRangeLBA[2][bySessionIdx] = nAdrLBAList[3] - nAdrLBAList[2];
		if (byPlxtrType == PLXTR_DRIVE_TYPE::PXS88T) {
			// Somehow PX-S88T is sliding subchannel +1;
			nFirstLBA[0][bySessionIdx]++;
			nFirstLBA[1][bySessionIdx]++;
			nFirstLBA[2][bySessionIdx]++;
		}
		OutputDiscLogA(
			"\tSession %d, 1st %s sector is %d, %s sector exists per %d, %d, %d\n"
			, bySessionIdx + 1, strAdr, nFirstLBA[0][bySessionIdx]
			, strAdr, nRangeLBA[0][bySessionIdx]
			, nRangeLBA[1][bySessionIdx]
			, nRangeLBA[2][bySessionIdx]);
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
	if (!bCheckReading && pDevice->FEATURE.byC2ErrorData &&
		(pExtArg && pExtArg->byC2)) {
		cdb->ErrorFlags = READ_CD_FLAG::byte294;
	}
	cdb->IncludeEDC = TRUE;
	cdb->IncludeUserData = TRUE;
	cdb->HeaderCode = READ_CD_FLAG::BothHeader;
	cdb->IncludeSyncData = TRUE;
	cdb->SubChannelSelection = Sub;
}

VOID SetReadD8Command(
	PDEVICE pDevice,
	CDB::_PLXTR_READ_CDDA* cdb,
	UINT uiTransferLen,
	PLXTR_READ_CDDA_FLAG::SUB_CHANNEL_SELECTION Sub
	)
{
	cdb->OperationCode = SCSIOP_PLXTR_READ_CDDA;
	cdb->LogicalUnitNumber = pDevice->address.Lun;
	cdb->TransferBlockByte0 = HIBYTE(HIWORD(uiTransferLen));
	cdb->TransferBlockByte1 = LOBYTE(HIWORD(uiTransferLen));
	cdb->TransferBlockByte2 = HIBYTE(LOWORD(uiTransferLen));
	cdb->TransferBlockByte3 = LOBYTE(LOWORD(uiTransferLen));
	cdb->SubCode = (UCHAR)Sub;
}

VOID SetTransferLength(
	CDB::_READ12* pCdb,
	DWORD dwSize,
	LPBYTE lpTransferLen
	)
{
	*lpTransferLen = (BYTE)(dwSize / DISC_RAW_READ_SIZE);
	// Generally, pDirTblSize is per 2048 byte
	// Exception: Commandos - Behind Enemy Lines (Europe) (Sold Out Software)
	if (dwSize % DISC_RAW_READ_SIZE != 0) {
		(*lpTransferLen)++;
	}
	pCdb->TransferLength[3] = *lpTransferLen;
}

VOID SetSubQDataFromBuffer(
	PSUB_Q_PER_SECTOR pSubQ,
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
}

VOID SetBufferFromSubQData(
	SUB_Q_PER_SECTOR subQ,
	LPBYTE lpSubcode,
	BYTE byPresent
	)
{
	lpSubcode[12] = BYTE(subQ.byCtl << 4 | subQ.byAdr);
	lpSubcode[13] = DecToBcd(subQ.byTrackNum);
	lpSubcode[14] = DecToBcd(subQ.byIndex);
	BYTE m, s, f;
	if (byPresent) {
		LBAtoMSF(subQ.nRelativeTime, &m, &s, &f);
	}
	else {
		LBAtoMSF(subQ.nRelativeTime + 1, &m, &s, &f);
	}
	lpSubcode[15] = DecToBcd(m);
	lpSubcode[16] = DecToBcd(s);
	lpSubcode[17] = DecToBcd(f);
	if (byPresent) {
		LBAtoMSF(subQ.nAbsoluteTime, &m, &s, &f);
	}
	else {
		LBAtoMSF(subQ.nAbsoluteTime + 1, &m, &s, &f);
	}
	lpSubcode[19] = DecToBcd(m);
	lpSubcode[20] = DecToBcd(s);
	lpSubcode[21] = DecToBcd(f);
	WORD crc16 = (WORD)GetCrc16CCITT(10, &lpSubcode[12]);
	lpSubcode[22] = HIBYTE(crc16);
	lpSubcode[23] = LOBYTE(crc16);
}

VOID UpdateSubQData(
	PSUB_Q pSubQ,
	BOOL bLibCrypt
	)
{
	// TODO: Doesn't need?
	if (pSubQ->prev.byIndex == 0 && pSubQ->prev.nRelativeTime == 0) {
		pSubQ->prev.byIndex = 1;
	}
	pSubQ->prevPrev.byCtl = pSubQ->prev.byCtl;
	if (pSubQ->prev.byAdr != ADR_ENCODES_MEDIA_CATALOG &&
		pSubQ->prev.byAdr != ADR_ENCODES_ISRC) {
		pSubQ->prevPrev.byAdr = pSubQ->prev.byAdr;
		pSubQ->prevPrev.nRelativeTime = pSubQ->prev.nRelativeTime;
	}
	pSubQ->prevPrev.byTrackNum = pSubQ->prev.byTrackNum;
	pSubQ->prevPrev.byIndex = pSubQ->prev.byIndex;
	pSubQ->prevPrev.nAbsoluteTime = pSubQ->prev.nAbsoluteTime;

	pSubQ->prev.byCtl = pSubQ->present.byCtl;
	pSubQ->prev.byAdr = pSubQ->present.byAdr;
	pSubQ->prev.byTrackNum = pSubQ->present.byTrackNum;
	pSubQ->prev.byIndex = pSubQ->present.byIndex;
	if (bLibCrypt) {
		pSubQ->prev.nRelativeTime++;
	}
	else {
		pSubQ->prev.nRelativeTime = pSubQ->present.nRelativeTime;
	}
	pSubQ->prev.nAbsoluteTime++;
}

VOID UpdateTmpMainHeader(
	PMAIN_HEADER pMain,
	LPBYTE lpBuf,
	BYTE byCtl,
	INT nType
	)
{
	memcpy(pMain->prev, pMain->present, MAINHEADER_MODE1_SIZE);
	BYTE tmp = (BYTE)(pMain->present[14] + 1);
	if ((tmp & 0x0f) == 0x0a) {
		tmp += 6;
	}
	if (tmp == 0x75) {
		pMain->present[14] = 0;
		if (nType == SCRAMBLED) {
			tmp = (BYTE)((pMain->present[13] ^ 0x80) + 1);
		}
		else {
			tmp = (BYTE)(pMain->present[13] + 1);
		}
		if ((tmp & 0x0f) == 0x0a) {
			tmp += 6;
		}
		if (tmp == 0x60) {
			if (nType == SCRAMBLED) {
				pMain->present[13] = 0x80;
				tmp = (BYTE)((pMain->present[12] ^ 0x01) + 1);
			}
			else {
				pMain->present[13] = 0;
				tmp = (BYTE)(pMain->present[12] + 1);
			}
			if ((tmp & 0x0f) == 0x0a) {
				tmp += 6;
			}
			if (nType == SCRAMBLED) {
				pMain->present[12] = (BYTE)(tmp ^ 0x01);
			}
			else {
				pMain->present[12] = tmp;
			}
		}
		else {
			if (nType == SCRAMBLED) {
				pMain->present[13] = (BYTE)(tmp ^ 0x80);
			}
			else {
				pMain->present[13] = tmp;
			}
		}
	}
	else {
		pMain->present[14] = tmp;
	}
	pMain->present[15] = GetMode(lpBuf, pMain->prev[15], byCtl, nType);
}
