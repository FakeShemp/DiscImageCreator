/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"
#include "struct.h"
#include "convert.h"
#include "get.h"
#include "output.h"

BOOL PreserveTrackAttribution(
	PEXT_ARG pExtArg,
	PDISC_DATA pDiscData,
	INT nLBA,
	LPBYTE lpCurrentTrackNum,
	PSUB_Q_DATA pSubQ,
	PSUB_Q_DATA pPrevSubQ,
	PSUB_Q_DATA pPrevPrevSubQ
	)
{
	UNREFERENCED_PARAMETER(pPrevPrevSubQ);
	if (pSubQ->byTrackNum > 0) {
		INT tIdx = *lpCurrentTrackNum - 1;
		// preserve nLBA
		if (pPrevSubQ->byTrackNum + 1 == pSubQ->byTrackNum) {
			*lpCurrentTrackNum = pSubQ->byTrackNum;
			tIdx = *lpCurrentTrackNum - 1;
			if (pSubQ->byIndex > 0) {
				if (pDiscData->SCSI.nFirstLBAof2ndSession == -1 &&
					pSubQ->byIndex == 1 && 
					nLBA != pDiscData->SCSI.lpFirstLBAListOnToc[tIdx]) {
					OutputInfoLogA(
						"LBA[%06d, %#07x], Track[%02u]: Subchannel & TOC isn't sync. LBA on TOC[%d, %#x], index[%02u]\n",
						nLBA, nLBA, pSubQ->byTrackNum, pDiscData->SCSI.lpFirstLBAListOnToc[tIdx], 
						pDiscData->SCSI.lpFirstLBAListOnToc[tIdx], pSubQ->byIndex);
					pDiscData->SUB_CHANNEL.lpFirstLBAListOnSub[tIdx][1] = pDiscData->SCSI.lpFirstLBAListOnToc[tIdx];
					if (pDiscData->SUB_CHANNEL.lpFirstLBAListOnSub[tIdx][0] == pDiscData->SUB_CHANNEL.lpFirstLBAListOnSub[tIdx][1]) {
						pDiscData->SUB_CHANNEL.lpFirstLBAListOnSub[tIdx][0] = -1;
					}
					pDiscData->SUB_CHANNEL.lpFirstLBAListOnSubSync[tIdx][1] = nLBA;
					if (pDiscData->SUB_CHANNEL.lpFirstLBAListOnSubSync[tIdx][0] == pDiscData->SUB_CHANNEL.lpFirstLBAListOnSubSync[tIdx][1]) {
						pDiscData->SUB_CHANNEL.lpFirstLBAListOnSubSync[tIdx][0] = -1;
					}
					pDiscData->SUB_CHANNEL.bDesync = TRUE;
				}
				else {
					pDiscData->SUB_CHANNEL.lpFirstLBAListOnSub[tIdx][pSubQ->byIndex] = nLBA;
					pDiscData->SUB_CHANNEL.lpFirstLBAListOnSubSync[tIdx][pSubQ->byIndex] = nLBA;
				}
			}
			// preserve last LBA per data track
			if (pPrevSubQ->byTrackNum > 0) {
				if (pDiscData->SUB_CHANNEL.lpFirstLBAListOfDataTrackOnSub[pPrevSubQ->byTrackNum - 1] != -1 &&
					pDiscData->SUB_CHANNEL.lpLastLBAListOfDataTrackOnSub[pPrevSubQ->byTrackNum - 1] == -1 &&
					(pPrevSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					pDiscData->SUB_CHANNEL.lpLastLBAListOfDataTrackOnSub[pPrevSubQ->byTrackNum - 1] = nLBA - 1;
				}
			}
		}
		// preserve mode, ctl
		if (nLBA == pDiscData->SCSI.lpFirstLBAListOnToc[tIdx]) {
			pDiscData->SUB_CHANNEL.lpCtlList[tIdx] = pSubQ->byCtl;
			pDiscData->SUB_CHANNEL.lpModeList[tIdx] = pSubQ->byMode;
		}
		// preserve index
		if (pPrevSubQ->byIndex + 1 == pSubQ->byIndex && *lpCurrentTrackNum >= 0) {
			if (pSubQ->byIndex != 1) {
				pDiscData->SUB_CHANNEL.lpFirstLBAListOnSub[tIdx][pSubQ->byIndex] = nLBA;
				pDiscData->SUB_CHANNEL.lpFirstLBAListOnSubSync[tIdx][pSubQ->byIndex] = nLBA;
			}
			else {
				if (nLBA != pDiscData->SCSI.lpFirstLBAListOnToc[tIdx]) {
					OutputInfoLogA(
						"LBA[%06d, %#07x], Track[%02u]: Subchannel & TOC isn't sync. LBA on TOC[%d, %#x], prevIndex[%02u]\n",
						nLBA, nLBA, pSubQ->byTrackNum, pDiscData->SCSI.lpFirstLBAListOnToc[tIdx],
						pDiscData->SCSI.lpFirstLBAListOnToc[tIdx], pPrevSubQ->byIndex);
				}
				pDiscData->SUB_CHANNEL.lpFirstLBAListOnSub[tIdx][1] = pDiscData->SCSI.lpFirstLBAListOnToc[tIdx];
				if (pDiscData->SUB_CHANNEL.lpFirstLBAListOnSub[tIdx][0] == pDiscData->SUB_CHANNEL.lpFirstLBAListOnSub[tIdx][1]) {
					// Crow, The - Original Motion Picture Soundtrack (82519-2)
					// LBA 108975, Track[06], Subchannel & TOC isn't sync. LBA on TOC: 108972, prevIndex[00]
					pDiscData->SUB_CHANNEL.lpFirstLBAListOnSub[tIdx][0] = -1;
				}
				pDiscData->SUB_CHANNEL.lpFirstLBAListOnSubSync[tIdx][pSubQ->byIndex] = nLBA;
				if (pDiscData->SUB_CHANNEL.lpFirstLBAListOnSubSync[tIdx][0] == pDiscData->SUB_CHANNEL.lpFirstLBAListOnSubSync[tIdx][1]) {
					pDiscData->SUB_CHANNEL.lpFirstLBAListOnSubSync[tIdx][0] = -1;
				}
			}
		}
		else if (pPrevSubQ->byIndex >= 1 && pSubQ->byIndex == 0) {
			pDiscData->SUB_CHANNEL.lpFirstLBAListOnSub[tIdx][pSubQ->byIndex] = nLBA;
			pDiscData->SUB_CHANNEL.lpFirstLBAListOnSubSync[tIdx][pSubQ->byIndex] = nLBA;
		}

		if ((pDiscData->SCSI.toc.TrackData[tIdx].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK &&
			(pSubQ->byCtl & AUDIO_DATA_TRACK) == 0) {
			OutputInfoLogA(
				"LBA[%06d, %#07x], Track[%02u]: Data track, but this sector is audio\n",
				nLBA, nLBA, pSubQ->byTrackNum);
		}
		else if ((pDiscData->SCSI.toc.TrackData[tIdx].Control & AUDIO_DATA_TRACK) == 0 &&
			(pSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			OutputInfoLogA(
				"LBA[%06d, %#07x], Track[%02u]: Audio track, but this sector is data\n",
				nLBA, nLBA, pSubQ->byTrackNum);
		}

		if (pExtArg->bReverse) {
			// preserve last LBA per data track
			if (nLBA == pDiscData->SCSI.nLastLBAofDataTrack) {
				if (pDiscData->SUB_CHANNEL.lpLastLBAListOfDataTrackOnSub[tIdx] == -1 &&
					(pSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					pDiscData->SUB_CHANNEL.lpLastLBAListOfDataTrackOnSub[tIdx] = pDiscData->SCSI.nLastLBAofDataTrack;
				}
			}
			else if ((pDiscData->SCSI.toc.TrackData[tIdx].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				pDiscData->SUB_CHANNEL.lpFirstLBAListOfDataTrackOnSub[tIdx] = nLBA;
			}
		}
		else {
			// preserve first LBA per data track
			if (pDiscData->SUB_CHANNEL.lpFirstLBAListOfDataTrackOnSub[tIdx] == -1 &&
				(pDiscData->SCSI.toc.TrackData[tIdx].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				pDiscData->SUB_CHANNEL.lpFirstLBAListOfDataTrackOnSub[tIdx] = nLBA;
			}
			// preserve last LBA per data track
			else if (nLBA == pDiscData->SCSI.nAllLength - 1) {
				if (pDiscData->SUB_CHANNEL.lpLastLBAListOfDataTrackOnSub[tIdx] == -1) {
					if ((pSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
						pDiscData->SUB_CHANNEL.lpLastLBAListOfDataTrackOnSub[tIdx] = pDiscData->SCSI.nAllLength - 1;
					}
				}
			}
		}
	}
	return TRUE;
}

VOID SetAndOutputc2ErrorDataPerSector(
	PC2_ERROR_DATA pC2ErrorData,
	PC2_ERROR_DATA_PER_SECTOR pC2ErrorDataPerSector,
	INT nLBA,
	DWORD dwAllBufLen,
	UINT uiC2ErrorLBACnt
	)
{
	INT nBytePos = nLBA * CD_RAW_SECTOR_SIZE;
	INT nBytePosEnd = nBytePos + CD_RAW_SECTOR_SIZE - 1;
	OutputErrorLogA(
		"LBA[%06d, %#07x], BytePos[%d-%d, %#x-%#x] C2 err exist. SlideSectorNum %d, ErrByteCnt %u\n"
		"                      [ErrOfs:BytePos(dec), ErrOfs:BytePos(hex)]\n",
		nLBA, nLBA, nBytePos, nBytePosEnd, nBytePos, nBytePosEnd, pC2ErrorData->cSlideSectorNum,
		pC2ErrorDataPerSector[uiC2ErrorLBACnt].uiErrorBytePosCnt);
	for (UINT n = 0; n < pC2ErrorDataPerSector[uiC2ErrorLBACnt].uiErrorBytePosCnt; n++) {
		INT nPos = nBytePos + pC2ErrorDataPerSector[uiC2ErrorLBACnt].lpErrorBytePos[n];
		OutputErrorLogA("                      [%u:%d, %#x:%#x]\n", 
			pC2ErrorDataPerSector[uiC2ErrorLBACnt].lpErrorBytePos[n], nPos, 
			pC2ErrorDataPerSector[uiC2ErrorLBACnt].lpErrorBytePos[n], nPos);
	}
	if (pC2ErrorDataPerSector[uiC2ErrorLBACnt].bErrorFlagBackup == RETURNED_C2_ERROR_NO_1ST ||
		pC2ErrorDataPerSector[uiC2ErrorLBACnt].bErrorFlagBackup == RETURNED_C2_ERROR_NO_BUT_BYTE_ERROR) {
		memcpy(pC2ErrorDataPerSector[uiC2ErrorLBACnt].lpBufC2NoneSector,
			pC2ErrorDataPerSector[uiC2ErrorLBACnt].lpBufC2NoneSectorBackup, dwAllBufLen);
	}
	pC2ErrorDataPerSector[uiC2ErrorLBACnt].bErrorFlag = RETURNED_C2_ERROR_EXIST;
	pC2ErrorDataPerSector[uiC2ErrorLBACnt].nErrorLBANum = nLBA;
}

VOID SetAndOutputC2NoErrorData(
	PC2_ERROR_DATA_PER_SECTOR pC2ErrorDataPerSector,
	LPBYTE lpBuf,
	INT nLBA,
	DWORD dwAllBufLen,
	UINT uiC2ErrorLBACnt
	)
{
	OutputErrorLogA(
		"LBA[%06d, %#07x], C2 err doesn't exist. Next check 2352 byte.\n",
		nLBA, nLBA);
	memcpy(pC2ErrorDataPerSector[uiC2ErrorLBACnt].lpBufC2NoneSector, lpBuf, dwAllBufLen);
	pC2ErrorDataPerSector[uiC2ErrorLBACnt].bErrorFlag = RETURNED_C2_ERROR_NO_1ST;
	pC2ErrorDataPerSector[uiC2ErrorLBACnt].nErrorLBANum = nLBA;
}

VOID SetAndOutputC2NoErrorByteErrorData(
	PC2_ERROR_DATA_PER_SECTOR pC2ErrorDataPerSector,
	LPBYTE lpBuf,
	INT nLBA,
	DWORD dwAllBufLen,
	UINT uiC2ErrorLBACnt
	)
{
	OutputErrorLogA(
		"LBA[%06d, %#07x], C2 err doesn't exist. But byte doesn't match\n",
		nLBA, nLBA);
	memcpy(pC2ErrorDataPerSector[uiC2ErrorLBACnt].lpBufC2NoneSector, lpBuf, dwAllBufLen);
	pC2ErrorDataPerSector[uiC2ErrorLBACnt].bErrorFlag = RETURNED_C2_ERROR_NO_BUT_BYTE_ERROR;
	pC2ErrorDataPerSector[uiC2ErrorLBACnt].nErrorLBANum = nLBA;
}

VOID SetC2ErrorBackup(
	PC2_ERROR_DATA_PER_SECTOR pC2ErrorDataPerSector,
	UINT uiC2ErrorLBACntBackup,
	DWORD dwAllBufLen
	)
{
	for (UINT c = 0; c < uiC2ErrorLBACntBackup; c++) {
		pC2ErrorDataPerSector[c].bErrorFlagBackup = pC2ErrorDataPerSector[c].bErrorFlag;
		pC2ErrorDataPerSector[c].bErrorFlag = RETURNED_C2_ERROR_NO_1ST;
		pC2ErrorDataPerSector[c].nErrorLBANumBackup = pC2ErrorDataPerSector[c].nErrorLBANum;
		pC2ErrorDataPerSector[c].nErrorLBANum = 0;
		for (UINT d = 0; d < pC2ErrorDataPerSector[c].uiErrorBytePosCnt; d++) {
			pC2ErrorDataPerSector[c].lpErrorBytePosBackup[d] = pC2ErrorDataPerSector[c].lpErrorBytePos[d];
			pC2ErrorDataPerSector[c].lpErrorBytePos[d] = 0;
		}
		pC2ErrorDataPerSector[c].uiErrorBytePosCntBackup = pC2ErrorDataPerSector[c].uiErrorBytePosCnt;
		pC2ErrorDataPerSector[c].uiErrorBytePosCnt = 0;
		memcpy(pC2ErrorDataPerSector[c].lpBufC2NoneSectorBackup, 
			pC2ErrorDataPerSector[c].lpBufC2NoneSector, dwAllBufLen);
		ZeroMemory(pC2ErrorDataPerSector[c].lpBufC2NoneSector, dwAllBufLen);
	}
}

VOID SetAndOutputMmcToc(
	PEXEC_TYPE pExecType,
	PDISC_DATA pDiscData
	)
{
	OutputDiscLogA("TOC on SCSIOP_READ_TOC\n");
	CONST INT typeSize = 7;
	_TCHAR strType[typeSize] = { 0 };
	BOOL bFirstData = TRUE;
	BOOL bAudioOnly = TRUE;
	for (BYTE i = pDiscData->SCSI.toc.FirstTrack; i <= pDiscData->SCSI.toc.LastTrack; i++) {
		for (INT j = 0, k = 24; j < 4; j++, k -= 8) {
			pDiscData->SCSI.lpFirstLBAListOnToc[i - 1] |= 
				pDiscData->SCSI.toc.TrackData[i - 1].Address[j] << k;
			pDiscData->SCSI.lpLastLBAListOnToc[i - 1] |= 
				pDiscData->SCSI.toc.TrackData[i].Address[j] << k;
		}
		pDiscData->SCSI.lpLastLBAListOnToc[i - 1] -= 1;
		pDiscData->SCSI.nAllLength += 
			pDiscData->SCSI.lpLastLBAListOnToc[i - 1] - pDiscData->SCSI.lpFirstLBAListOnToc[i - 1] + 1;

		if ((pDiscData->SCSI.toc.TrackData[i - 1].Control & AUDIO_DATA_TRACK) == 0) {
			_tcsncpy(strType, _T(" Audio"), typeSize);
		}
		else if ((pDiscData->SCSI.toc.TrackData[i - 1].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			_tcsncpy(strType, _T("  Data"), typeSize);
			if (bFirstData) {
				pDiscData->SCSI.nFirstLBAofDataTrack = 
					pDiscData->SCSI.lpFirstLBAListOnToc[i - 1];
				pDiscData->SCSI.byFirstDataTrack = i;
				bFirstData = FALSE;
				bAudioOnly = FALSE;
			}
			pDiscData->SCSI.nLastLBAofDataTrack = 
				pDiscData->SCSI.lpLastLBAListOnToc[i - 1];
			pDiscData->SCSI.byLastDataTrack = i;
		}
		if (i == pDiscData->SCSI.toc.FirstTrack && 
			pDiscData->SCSI.lpFirstLBAListOnToc[i - 1] > 0) {
			pDiscData->SCSI.nAllLength += pDiscData->SCSI.lpFirstLBAListOnToc[i - 1];
			OutputDiscLogA(
				"\tPregap Track   , LBA %8u-%8u, Length %8u\n",
				0, pDiscData->SCSI.lpFirstLBAListOnToc[i - 1] - 1, 
				pDiscData->SCSI.lpFirstLBAListOnToc[i - 1]);
		}
		OutputDiscLogA(
			"\t%s Track %2u, LBA %8u-%8u, Length %8u\n", 
			strType, i, pDiscData->SCSI.lpFirstLBAListOnToc[i - 1],
			pDiscData->SCSI.lpLastLBAListOnToc[i - 1],
			pDiscData->SCSI.lpLastLBAListOnToc[i - 1] - pDiscData->SCSI.lpFirstLBAListOnToc[i - 1] + 1);
	}
	OutputDiscLogA(
		"\t                                        Total  %8u\n", pDiscData->SCSI.nAllLength);
	if (*pExecType != gd) {
		pDiscData->SCSI.bAudioOnly = bAudioOnly;
	}
}

VOID SetAndOutputMmcTocFull(
	PDISC_DATA pDiscData,
	PCDROM_TOC_FULL_TOC_DATA fullToc,
	PCDROM_TOC_FULL_TOC_DATA_BLOCK pTocData,
	size_t uiTocEntries,
	FILE* fpCcd
	)
{
	OutputDiscLogA(
		"FULL TOC on SCSIOP_READ_TOC\n"
		"\tFirstCompleteSession: %u\n"
		"\t LastCompleteSession: %u\n",
		fullToc->FirstCompleteSession,
		fullToc->LastCompleteSession);
	BOOL bFirst2ndSession = TRUE;
	pDiscData->SCSI.nFirstLBAofLeadout = -1;
	pDiscData->SCSI.nFirstLBAof2ndSession = -1;

	for (UINT a = 0; a < uiTocEntries; a++) {
		INT nTmpLBAExt = 0;
		INT nTmpLBA = 0;
		if (fpCcd) {
			WriteCcdFileForEntry(pTocData, a, fpCcd);
		}
		switch (pTocData[a].Point) {
		case 0xA0:
			OutputDiscLogA("\tSession %u, FirstTrack %2u, ", 
				pTocData[a].SessionNumber, pTocData[a].Msf[0]);
			switch (pTocData[a].Msf[1]) {
				case 0x00:
					OutputDiscLogA("Format: CD-DA or CD-ROM\n");
					break;
				case 0x10:
					OutputDiscLogA("Format: CD-I\n");
					pDiscData->SCSI.bCdi = TRUE;
					break;
				case 0x20:
					OutputDiscLogA("Format: CD-ROM-XA\n");
					break;
				default:
					OutputDiscLogA("Format: Other\n");
					break;
			}
			break;
		case 0xA1:
			OutputDiscLogA("\tSession %u,  LastTrack %2u\n", 
				pTocData[a].SessionNumber, pTocData[a].Msf[0]);
			break;
		case 0xA2:
			nTmpLBA = 
				MSFtoLBA(pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
			OutputDiscLogA(
				"\tSession %u,      Leadout, MSF %02u:%02u:%02u (LBA[%06d, %#07x])\n", 
				pTocData[a].SessionNumber, pTocData[a].Msf[0], pTocData[a].Msf[1],
				pTocData[a].Msf[2], nTmpLBA, nTmpLBA);
			pDiscData->SCSI.lpLastLBAListOnToc[pDiscData->SCSI.toc.LastTrack - 1] = nTmpLBA - 150 - 1;
			if (pTocData[a].SessionNumber == 1) {
				pDiscData->SCSI.nFirstLBAofLeadout = nTmpLBA - 150;
			}
			break;
		case 0xB0: // (multi-session disc)
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
		case 0xB1: // (Audio only: This identifies the presence of skip intervals)
			OutputDiscLogA(
				"\tThe number of skip interval pointers %2u\n"
				"\tThe number of skip track assignments %2u\n",
				pTocData[a].Msf[0], pTocData[a].Msf[1]);
			break;
		case 0xB2: // (Audio only: This identifies tracks that should be skipped during playback)
		case 0xB3:
		case 0xB4:
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
		case 0xC0: // (Together with POINT=B0h, this is used to identify a multi-session disc)
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
				pDiscData->SCSI.lpFirstLBAListOnToc[pTocData[a].Point - 1] = nTmpLBA - 150;
			}
			else if (pTocData[a].Point >= 2 && pTocData[a].Point <= 100) {
				pDiscData->SCSI.lpLastLBAListOnToc[pTocData[a].Point - 2] = nTmpLBA - 150 - 1;
				pDiscData->SCSI.lpFirstLBAListOnToc[pTocData[a].Point - 1] = nTmpLBA - 150;
			}
			if (pTocData[a].SessionNumber == 2 && bFirst2ndSession) {
				pDiscData->SCSI.nFirstLBAof2ndSession = nTmpLBA - 150;
				bFirst2ndSession = FALSE;
			}
			pDiscData->SCSI.lpSessionNumList[pTocData[a].Point - 1] = pTocData[a].SessionNumber;
			break;
		}
	}
}

VOID SetAndOutputMmcTocCDText(
	PDISC_DATA pDiscData,
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
				_tcsncpy(pDiscData->SCSI.pszTitle[nTitleCnt], tmp, _tcslen(tmp));
				if (nTitleCnt == 0) {
					OutputDiscLogA(
						"\tAlbum Name: %s\n", 
						pDiscData->SCSI.pszTitle[nTitleCnt]);
				}
				else {
					OutputDiscLogA(
						"\t Song Name: %s\n", 
						pDiscData->SCSI.pszTitle[nTitleCnt]);
				}
				nTitleCnt++;
			}
			else if (byPerformerCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt)) {
				_tcsncpy(pDiscData->SCSI.pszPerformer[nPerformerCnt], tmp, _tcslen(tmp));
				if (nPerformerCnt == 0) {
					OutputDiscLogA(
						"\tAlbum Performer: %s\n", 
						pDiscData->SCSI.pszPerformer[nPerformerCnt]);
				}
				else {
					OutputDiscLogA(
						"\t Song Performer: %s\n", 
						pDiscData->SCSI.pszPerformer[nPerformerCnt]);
				}
				nPerformerCnt++;
			}
			else if (bySongwriterCnt != 0 &&
				z < (size_t)(byAlbumCnt + byPerformerCnt + bySongwriterCnt)) {
				_tcsncpy(pDiscData->SCSI.pszSongWriter[nSongwriterCnt], tmp, _tcslen(tmp));
				if (nSongwriterCnt == 0) {
					OutputDiscLogA(
						"\tAlbum SongWriter: %s\n", 
						pDiscData->SCSI.pszSongWriter[nSongwriterCnt]);
				}
				else {
					OutputDiscLogA(
						"\t      SongWriter: %s\n", 
						pDiscData->SCSI.pszSongWriter[nSongwriterCnt]);
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

VOID SetAndOutputMmcTocCDWText(
	PDISC_DATA pDiscData,
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	PCHAR pTmpText,
	WORD wFirstEntries,
	WORD wTocTextEntries,
	WORD wAllTextSize
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

VOID SetMmcFeatureCdRead(
	PFEATURE_DATA_CD_READ pCDRead,
	PDEVICE_DATA pDevData
	)
{
	pDevData->bCanCDText = (BOOL)(pCDRead->CDText);
	pDevData->bC2ErrorData = (BOOL)(pCDRead->C2ErrorData);
}

VOID SetCDOffsetData(
	PDISC_DATA pDiscData,
	INT nStartLBA,
	INT nEndLBA
	)
{
	if (pDiscData->MAIN_CHANNEL.nCombinedOffset > 0) {
		pDiscData->MAIN_CHANNEL.uiMainDataSlideSize = 
			(size_t)pDiscData->MAIN_CHANNEL.nCombinedOffset % CD_RAW_SECTOR_SIZE;
		pDiscData->MAIN_CHANNEL.nOffsetStart = 0;
		pDiscData->MAIN_CHANNEL.nOffsetEnd = 
			pDiscData->MAIN_CHANNEL.nAdjustSectorNum;
		pDiscData->MAIN_CHANNEL.nFixStartLBA = 
			nStartLBA + pDiscData->MAIN_CHANNEL.nAdjustSectorNum - 1;
		pDiscData->MAIN_CHANNEL.nFixEndLBA = 
			nEndLBA + pDiscData->MAIN_CHANNEL.nAdjustSectorNum - 1;
		if (pDiscData->SCSI.nFirstLBAof2ndSession != -1) {
			pDiscData->MAIN_CHANNEL.nFixFirstLBAofLeadout =
				pDiscData->SCSI.nFirstLBAofLeadout + pDiscData->MAIN_CHANNEL.nAdjustSectorNum;
			pDiscData->MAIN_CHANNEL.nFixFirstLBAof2ndSession = 
				pDiscData->SCSI.nFirstLBAof2ndSession + pDiscData->MAIN_CHANNEL.nAdjustSectorNum - 1;
		}
	}
	else if (pDiscData->MAIN_CHANNEL.nCombinedOffset < 0) {
		pDiscData->MAIN_CHANNEL.uiMainDataSlideSize = 
			(size_t)CD_RAW_SECTOR_SIZE + (pDiscData->MAIN_CHANNEL.nCombinedOffset % CD_RAW_SECTOR_SIZE);
		pDiscData->MAIN_CHANNEL.nOffsetStart = 
			pDiscData->MAIN_CHANNEL.nAdjustSectorNum;
		pDiscData->MAIN_CHANNEL.nOffsetEnd = 0;
		pDiscData->MAIN_CHANNEL.nFixStartLBA = 
			nStartLBA + pDiscData->MAIN_CHANNEL.nAdjustSectorNum;
		pDiscData->MAIN_CHANNEL.nFixEndLBA = 
			nEndLBA + pDiscData->MAIN_CHANNEL.nAdjustSectorNum;
		if (pDiscData->SCSI.nFirstLBAof2ndSession != -1) {
			pDiscData->MAIN_CHANNEL.nFixFirstLBAofLeadout = 
				pDiscData->SCSI.nFirstLBAofLeadout + pDiscData->MAIN_CHANNEL.nAdjustSectorNum + 1;
			pDiscData->MAIN_CHANNEL.nFixFirstLBAof2ndSession =
				pDiscData->SCSI.nFirstLBAof2ndSession + pDiscData->MAIN_CHANNEL.nAdjustSectorNum;
		}
	}
}

VOID SetCDTransferData(
	PDEVICE_DATA pDevData,
	DRIVE_DATA_ORDER order
	)
{
	pDevData->TRANSFER_DATA.uiTransferLen = 1;
	if (order == DRIVE_DATA_ORDER::C2None) {
		pDevData->TRANSFER_DATA.dwBufLen = CD_RAW_SECTOR_WITH_SUBCODE_SIZE;
		pDevData->TRANSFER_DATA.dwAllBufLen = 
			pDevData->TRANSFER_DATA.dwBufLen * pDevData->TRANSFER_DATA.uiTransferLen;
		pDevData->TRANSFER_DATA.dwAdditionalBufLen = 0;
		pDevData->TRANSFER_DATA.dwBufC2Offset = 0;
		pDevData->TRANSFER_DATA.dwBufSubOffset = CD_RAW_SECTOR_SIZE;
	}
	else if (order == DRIVE_DATA_ORDER::MainC2Sub) {
		pDevData->TRANSFER_DATA.dwBufLen = CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE;
		if (pDevData->byPlexType == PLEX_DRIVE_TYPE::PXW4824) {
			pDevData->TRANSFER_DATA.dwAdditionalBufLen = 34; // unknown size
		}
		else {
			pDevData->TRANSFER_DATA.dwAdditionalBufLen = 0;
		}
		pDevData->TRANSFER_DATA.dwAllBufLen = 
			(pDevData->TRANSFER_DATA.dwBufLen + pDevData->TRANSFER_DATA.dwAdditionalBufLen) * pDevData->TRANSFER_DATA.uiTransferLen;
		pDevData->TRANSFER_DATA.dwBufC2Offset = CD_RAW_SECTOR_SIZE;
		pDevData->TRANSFER_DATA.dwBufSubOffset = CD_RAW_SECTOR_WITH_C2_294_SIZE;
	}
	else if (order == DRIVE_DATA_ORDER::MainSubC2) {
		pDevData->TRANSFER_DATA.dwBufLen = CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE;
		pDevData->TRANSFER_DATA.dwAdditionalBufLen = 0;
		pDevData->TRANSFER_DATA.dwAllBufLen =
			pDevData->TRANSFER_DATA.dwBufLen * pDevData->TRANSFER_DATA.uiTransferLen;
		pDevData->TRANSFER_DATA.dwBufSubOffset = CD_RAW_SECTOR_SIZE;
		pDevData->TRANSFER_DATA.dwBufC2Offset = CD_RAW_SECTOR_WITH_SUBCODE_SIZE;
	}
}

VOID SetISRCToString(
	PDISC_DATA pDiscData,
	LPBYTE lpSubcode,
	LPTSTR pszOutString,
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
	_sntprintf(pszOutString, META_ISRC_SIZE, _T("%c%c%c%c%c%c%c%c%c%c%c%c"),
		((lpSubcode[13] >> 2) & 0x3F) + 0x30, 
		(((lpSubcode[13] << 4) & 0x30) | ((lpSubcode[14] >> 4) & 0x0F)) + 0x30, 
		(((lpSubcode[14] << 2) & 0x3C) | ((lpSubcode[15] >> 6) & 0x03)) + 0x30, 
		(lpSubcode[15] & 0x3F) + 0x30, 
		((lpSubcode[16] >> 2) & 0x3F) + 0x30, 
		((lpSubcode[17] >> 4) & 0x0F) + 0x30, (lpSubcode[17] & 0x0F) + 0x30, 
		((lpSubcode[18] >> 4) & 0x0F) + 0x30, (lpSubcode[18] & 0x0F) + 0x30, 
		((lpSubcode[19] >> 4) & 0x0F) + 0x30, (lpSubcode[19] & 0x0F) + 0x30,
		((lpSubcode[20] >> 4) & 0x0F) + 0x30);
	pszOutString[META_ISRC_SIZE - 1] = 0;
	if (bCopy) {
		_tcsncpy(pDiscData->SUB_CHANNEL.pszISRC[byTrackNum - 1], pszOutString, META_ISRC_SIZE);
	}
}

VOID SetMCNToString(
	PDISC_DATA pDiscData,
	LPBYTE lpSubcode,
	LPTSTR pszOutString,
	BOOL bCopy
	)
{
	_sntprintf(pszOutString, META_CATALOG_SIZE, _T("%c%c%c%c%c%c%c%c%c%c%c%c%c"),
		((lpSubcode[13] >> 4) & 0x0F) + 0x30, (lpSubcode[13] & 0x0F) + 0x30, 
		((lpSubcode[14] >> 4) & 0x0F) + 0x30, (lpSubcode[14] & 0x0F) + 0x30, 
		((lpSubcode[15] >> 4) & 0x0F) + 0x30, (lpSubcode[15] & 0x0F) + 0x30, 
		((lpSubcode[16] >> 4) & 0x0F) + 0x30, (lpSubcode[16] & 0x0F) + 0x30, 
		((lpSubcode[17] >> 4) & 0x0F) + 0x30, (lpSubcode[17] & 0x0F) + 0x30, 
		((lpSubcode[18] >> 4) & 0x0F) + 0x30, (lpSubcode[18] & 0x0F) + 0x30, 
		((lpSubcode[19] >> 4) & 0x0F) + 0x30);
	pszOutString[META_CATALOG_SIZE - 1] = 0;
	if (bCopy) {
		_tcsncpy(pDiscData->SUB_CHANNEL.szCatalog, pszOutString, META_CATALOG_SIZE);
	}
}

VOID SetReadCDCommand(
	PDEVICE_DATA pDevData,
	PEXT_ARG pExtArg,
	CDB::_READ_CD* cdb,
	BOOL bSub,
	READ_CD_FLAG::SECTOR_TYPE type,
	UINT uiTransferLen,
	BOOL bCheckReading
	)
{
	cdb->OperationCode = SCSIOP_READ_CD;
	cdb->ExpectedSectorType = type;
	cdb->Lun = pDevData->address.Lun;
	cdb->TransferBlocks[0] = LOBYTE(HIWORD(uiTransferLen));
	cdb->TransferBlocks[1] = HIBYTE(LOWORD(uiTransferLen));
	cdb->TransferBlocks[2] = LOBYTE(LOWORD(uiTransferLen));
	if (!bCheckReading && pDevData->bC2ErrorData &&
		(pExtArg && pExtArg->bC2)) {
		cdb->ErrorFlags = READ_CD_FLAG::byte294;
	}
	cdb->IncludeEDC = TRUE;
	cdb->IncludeUserData = TRUE;
	cdb->HeaderCode = READ_CD_FLAG::BothHeader;
	cdb->IncludeSyncData = TRUE;
	if (bSub) {
		cdb->SubChannelSelection = READ_CD_FLAG::Raw;
	}
	else {
		cdb->SubChannelSelection = READ_CD_FLAG::SubNone;
	}
}

VOID SetReadD8Command(
	PDEVICE_DATA pDevData,
	PEXT_ARG pExtArg,
	CDB::_PLXTR_READ_CDDA* cdb,
	UINT uiTransferLen,
	BOOL bSub,
	BOOL bCheckReading
	)
{
	cdb->OperationCode = SCSIOP_PLEX_READ_CD;
	cdb->LogicalUnitNumber = pDevData->address.Lun;
	cdb->TransferBlockByte0 = HIBYTE(HIWORD(uiTransferLen));
	cdb->TransferBlockByte1 = LOBYTE(HIWORD(uiTransferLen));
	cdb->TransferBlockByte2 = HIBYTE(LOWORD(uiTransferLen));
	cdb->TransferBlockByte3 = LOBYTE(LOWORD(uiTransferLen));
	if (bSub) {
		if (!bCheckReading && pDevData->bC2ErrorData &&
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
	PDISC_DATA pDiscData,
	PSUB_Q_DATA pSubQ,
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
	pSubQ->byMode = 
		GetMode(lpBuf + pDiscData->MAIN_CHANNEL.uiMainDataSlideSize, pSubQ->byCtl);
}

BOOL UpdateSubQData(
	PSUB_Q_DATA pSubQ,
	PSUB_Q_DATA pPrevSubQ,
	PSUB_Q_DATA pPrevPrevSubQ,
	BOOL bLibCrypt
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
	if (bLibCrypt) {
		pPrevSubQ->nRelativeTime++;
	}
	else {
		pPrevSubQ->nRelativeTime = pSubQ->nRelativeTime;
	}
	pPrevSubQ->nAbsoluteTime++;

	return TRUE;
}
