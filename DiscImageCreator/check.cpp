/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"

BOOL IsValidMSF(
	CONST PUCHAR Subcode,
	INT m,
	INT s,
	INT f
	)
{
	BOOL bRet = TRUE;
	if(Subcode[m] > 0x99) {
		bRet = FALSE;
	}
	else if(Subcode[s] > 0x59) {
		bRet = FALSE;
	}
	else if(Subcode[f] > 0x74) {
		bRet = FALSE;
	}
	return bRet;
}

BOOL IsValidRelativeTime(
	CONST SUB_Q_DATA* prevSubQ,
	CONST SUB_Q_DATA* subQ,
	CONST PUCHAR Subcode
	)
{
	BOOL bRet = IsValidMSF(Subcode, 15, 16, 17);
	if(bRet) {
		INT tmpLBA = MSFtoLBA(BcdToDec(Subcode[17]), 
			BcdToDec(Subcode[16]), BcdToDec(Subcode[15]));
		if(subQ->byIndex > 0) {
			if(tmpLBA != 0 &&
				prevSubQ->nRelativeTime + 1 != subQ->nRelativeTime) {
				bRet = FALSE;
			}
		}
		else if(subQ->byIndex == 0) {
			if(tmpLBA != 0 &&
				prevSubQ->nRelativeTime != subQ->nRelativeTime + 1) {
				bRet = FALSE;
			}
		}
	}
	return bRet;
}

BOOL IsValidAbsoluteTime(
	CONST SUB_Q_DATA* prevSubQ,
	CONST SUB_Q_DATA* subQ,
	CONST PUCHAR Subcode,
	INT nLBA
	)
{
	BOOL bRet = IsValidMSF(Subcode, 19, 20, 21);
	if(bRet) {
		INT tmpLBA = MSFtoLBA(BcdToDec(Subcode[21]), 
			BcdToDec(Subcode[20]), BcdToDec(Subcode[19])) - 150;
		if(nLBA != tmpLBA ||
			prevSubQ->nAbsoluteTime + 1 != subQ->nAbsoluteTime) {
			bRet = FALSE;
		}
	}
	return bRet;
}

BOOL IsValidControl(
	CONST SUB_Q_DATA* prevPrevSubQ,
	CONST SUB_Q_DATA* prevSubQ,
	CONST SUB_Q_DATA* subQ,
	UCHAR byEndCtl
	)
{
	BOOL bRet = FALSE;
	switch(subQ->byCtl) {
	case 0:
		bRet = TRUE;
		break;
	case AUDIO_WITH_PREEMPHASIS:
		bRet = TRUE;
		break;
	case DIGITAL_COPY_PERMITTED:
		bRet = TRUE;
		break;
	case DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
		bRet = TRUE;
		break;
	case AUDIO_DATA_TRACK:
		bRet = TRUE;
		break;
	case AUDIO_DATA_TRACK | DIGITAL_COPY_PERMITTED:
		bRet = TRUE;
		break;
	case TWO_FOUR_CHANNEL_AUDIO:
		bRet = TRUE;
		break;
	case TWO_FOUR_CHANNEL_AUDIO | AUDIO_WITH_PREEMPHASIS:
		bRet = TRUE;
		break;
	case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED:
		bRet = TRUE;
		break;
	case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
		bRet = TRUE;
		break;
	default:
		return FALSE;
	}

	if(prevSubQ->byCtl != subQ->byCtl) {
		if((subQ->byCtl != byEndCtl) && subQ->byCtl != 0) {
			bRet = FALSE;
		}
		else {
			if(subQ->byAdr == ADR_ENCODES_CURRENT_POSITION) {
				if(prevSubQ->byTrackNum + 1 == subQ->byTrackNum) {
					if(prevSubQ->nRelativeTime + 1 == subQ->nRelativeTime) {
						bRet = FALSE;
					}
					else {
						bRet = TRUE;
					}
				}
				else if(prevSubQ->byTrackNum == subQ->byTrackNum &&
					prevSubQ->byIndex == subQ->byIndex) {
					if(prevSubQ->nRelativeTime + 1 == subQ->nRelativeTime) {
						bRet = FALSE;
					}
					else {
						bRet = TRUE;
					}
				}
			}
			if(prevPrevSubQ->byCtl == subQ->byCtl) {
				bRet = TRUE;
			}
		}
	}
	return bRet;
}

BOOL IsValidDataHeader(
	CONST PUCHAR src
	)
{
	UCHAR aHeader[] = { 
		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00
	};
	BOOL bRet = TRUE;
	for(INT c = 0; c < sizeof(aHeader); c++) {
		if(src[c] != aHeader[c]) {
			bRet = FALSE;
			break;
		}
	}
	return bRet;
}

BOOL IsValidIndex(
	CONST SUB_Q_DATA* prevPrevSubQ,
	CONST SUB_Q_DATA* prevSubQ,
	CONST SUB_Q_DATA* subQ,
	PBOOL bPrevIndex,
	PBOOL bPrevPrevIndex
	)
{
	BOOL bRet = TRUE;
	if(prevSubQ->byIndex != subQ->byIndex) {
		if(prevSubQ->byIndex + 1 < subQ->byIndex) {
			bRet = FALSE;
		}
		else if(MAXIMUM_NUMBER_INDEXES < subQ->byIndex) {
			bRet = FALSE;
		}
		else {
			bRet = FALSE;
			if(prevSubQ->byIndex == 0 &&
				subQ->byIndex  == 1 && 
				subQ->nRelativeTime == 0) {
				bRet = TRUE;
			}
			else if((subQ->byIndex  == 0 || subQ->byIndex  == 1) && 
				prevSubQ->byTrackNum + 1 == subQ->byTrackNum &&
				subQ->nRelativeTime < prevSubQ->nRelativeTime) {
				bRet = TRUE;
			}
			else if(prevSubQ->byIndex + 1 == subQ->byIndex && 
				prevSubQ->nRelativeTime + 1 == subQ->nRelativeTime) {
				bRet = TRUE;
			}
			if(prevPrevSubQ->byIndex == subQ->byIndex) {
				bRet = TRUE;
				if(prevSubQ->byIndex - 1 == subQ->byIndex) {
					*bPrevIndex = FALSE;
				}
			}
		}
	}
	else if(prevSubQ->byIndex == subQ->byIndex &&
		prevPrevSubQ->byIndex - 1 == subQ->byIndex &&
		prevPrevSubQ->byTrackNum + 1 != subQ->byTrackNum) {
		*bPrevPrevIndex = FALSE;
	}
	return bRet;
}

// reference
// https://isrc.jmd.ne.jp/about/pattern.html
// https://isrc.jmd.ne.jp/about/error.html
BOOL IsValidISRC(
	CONST PUCHAR Subcode
	)
{
	INT ch = ((Subcode[13] >> 2) & 0x3F) + 0x30;
	if(isupper(ch) == 0) {
		return FALSE;
	}

	ch = (((Subcode[13] << 4) & 0x30) | 
		((Subcode[14] >> 4) & 0x0F)) + 0x30;
	if(isupper(ch) == 0) {
		return FALSE;
	}

	ch = (((Subcode[14] << 2) & 0x3C) | 
		((Subcode[15] >> 6) & 0x03)) + 0x30;
	if(isupper(ch) == 0 && isdigit(ch) == 0) {
		return FALSE;
	}

	ch = (Subcode[15] & 0x3F) + 0x30;
	if(isupper(ch) == 0 && isdigit(ch) == 0) {
		return FALSE;
	}

	ch = ((Subcode[16] >> 2) & 0x3F) + 0x30;
	if(isupper(ch) == 0 && isdigit(ch) == 0) {
		return FALSE;
	}

	if((Subcode[16] & 0x03) != 0) {
		return FALSE;
	}

	for(INT i = 17; i <= 19; i++) {
		if(isdigit(((Subcode[i] >> 4) & 0x0F) + 0x30) == 0) {
			return FALSE;
		}
		if(isdigit((Subcode[i] & 0x0F) + 0x30) == 0) {
			return FALSE;
		}
	}

	if(isdigit(((Subcode[20] >> 4) & 0x0F) + 0x30) == 0) {
		return FALSE;
	}

	if((Subcode[20] & 0x0F) != 0) {
		return FALSE;
	}

	return TRUE;
}

BOOL IsValidMCN(
	CONST PUCHAR Subcode
	)
{
	BOOL bRet = TRUE;
	for(INT i = 13; i <= 19; i++) {
		if(isdigit(((Subcode[i] >> 4) & 0x0F) + 0x30) == 0) {
			bRet = FALSE;
			break;
		}
		if(isdigit((Subcode[i] & 0x0F) + 0x30) == 0) {
			bRet = FALSE;
			break;
		}
	}
	if((Subcode[19] & 0x0F) != 0 || Subcode[20] != 0) {
		bRet = FALSE;
	}
	return bRet;
}

BOOL IsValidTrackNumber(
	CONST SUB_Q_DATA* prevPrevSubQ,
	CONST SUB_Q_DATA* prevSubQ,
	CONST SUB_Q_DATA* subQ,
	UCHAR byFirstTrackNum,
	UCHAR byLastTrackNum,
	PBOOL bPrevTrackNum
	)
{
	BOOL bRet = TRUE;
	if(subQ->byTrackNum < byFirstTrackNum || 
		byLastTrackNum < subQ->byTrackNum) {
		bRet = FALSE;
	}
	else if(prevSubQ->byTrackNum != subQ->byTrackNum) {
		if(prevSubQ->byTrackNum + 2 <= subQ->byTrackNum) {
			bRet = FALSE;
		}
		else if(prevSubQ->byTrackNum > subQ->byTrackNum) {
			if(prevPrevSubQ->byTrackNum == subQ->byTrackNum) {
				*bPrevTrackNum = FALSE;
			}
			else {
				bRet = FALSE;
			}
		}
		else {
			if(prevSubQ->byTrackNum + 1 == subQ->byTrackNum) {
				if((prevSubQ->nRelativeTime + 1 <= subQ->nRelativeTime) ||
					(prevPrevSubQ->nRelativeTime + 2 <= subQ->nRelativeTime)) {
					bRet = FALSE;
				}
			}
			if(prevPrevSubQ->byTrackNum == subQ->byTrackNum) {
				bRet = TRUE;
			}
		}
	}
	return bRet;
}

BOOL Is3DOData(
	CONST PUCHAR src
	)
{
	UCHAR aHeader[] = { 
		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
		0x00, 0x02, 0x00, 0x01, 0x01, 0x5A,
		0x5A, 0x5A, 0x5A, 0x5A, 0x01, 0x00
	};
	BOOL bRet = TRUE;
	for(INT c = 0; c < sizeof(aHeader); c++) {
		if(src[c] != aHeader[c]) {
			bRet = FALSE;
			break;
		}
	}
	return bRet;
}

BOOL CheckAndFixPSubchannel(
	PUCHAR Subcode
	)
{
	BOOL bFF = FALSE;
	BOOL b00 = FALSE;
	for(INT i = 0; i < 12; i++) {
		if(Subcode[i] == 0xFF) {
			bFF = TRUE;
			break;
		}
		else if(Subcode[i] == 0x00) {
			b00 = TRUE;
			break;
		}
	}
	for(INT i = 0; i < 12; i++) {
		if(bFF && Subcode[i] != 0xFF) {
			Subcode[i] = 0xFF;
		}
		else if(b00 && Subcode[i] != 0x00) {
			Subcode[i] = 0x00;
		}
	}
	return 0;
}

BOOL CheckAndFixSubchannel(
	PCDROM_TOC toc,
	INT nLBA,
	PUCHAR Subcode,
	PSUB_Q_DATA subQ,
	PSUB_Q_DATA prevSubQ,
	PSUB_Q_DATA prevPrevSubQ,
	PUCHAR byCurrentTrackNum,
	PBOOL bCatalog,
	PBOOL aISRC,
	PUCHAR aEndCtl,
	PINT* aLBAStart,
	PINT* aLBAOfDataTrack,
	FILE* fpLog
	)
{
	CheckAndFixPSubchannel(Subcode);

	_TCHAR szCatalog[META_CATALOG_SIZE+1] = {0};
	_TCHAR szISRC[META_ISRC_SIZE+1] = {0};
	BOOL bBadAdr = FALSE;

	if(subQ->byAdr == ADR_ENCODES_MEDIA_CATALOG) {
		BOOL bMCN = IsValidMCN(Subcode);
		SetMCNToString(Subcode, szCatalog, bMCN);
		szCatalog[13] = '\0';
		// only once
		if(prevSubQ->byTrackNum == toc->FirstTrack) {
			*bCatalog = bMCN;
		}
		if(!bMCN) {
			OutputLogString(fpLog, _T("LBA %6d, MCN[%s]\n"), nLBA, szCatalog);
			subQ->byAdr = prevSubQ->byAdr;
			bBadAdr = TRUE;
		}
		else {
			//// Fix TrackNum, because don't exist.
			// Cosmic Fantasy 2
			// LBA[202749, 0x317FD], Data, Copy NG, TOC[TrackNum-80, Index-01, RelativeTime-00:06:63, AbsoluteTime-45:05:24] RtoW:ZERO mode
			// LBA[202750, 0x317FE], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number (MCN)[0000000000000        , AbsoluteTime-     :25] RtoW:ZERO mode
			// LBA[202751, 0x317FF], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-81, Index-00, RelativeTime-00:01:73, AbsoluteTime-45:05:26] RtoW:ZERO mode
			if((prevSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK &&
			(subQ->byCtl & AUDIO_DATA_TRACK) == 0) {
				subQ->byTrackNum = (UCHAR)(prevSubQ->byTrackNum + 1);
			}
			else {
				subQ->byTrackNum = prevSubQ->byTrackNum;
			}
			//// Fix Index, because don't exist.
			// Psychic Detective Series Vol. 5 - Nightmare (Japan)
			// LBA[080999, 0x13C67], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-02, Index-00, RelativeTime-00:00:00, AbsoluteTime-18:01:74] RtoW:ZERO mode
			// LBA[081000, 0x13C68], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number (MCN)[3010911111863        , AbsoluteTime-     :00] RtoW:ZERO mode
			// LBA[081001, 0x13C69], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-02, Index-01, RelativeTime-00:00:01, AbsoluteTime-18:02:01] RtoW:ZERO mode
			if(prevSubQ->byIndex == 0 && prevSubQ->nRelativeTime == 0) {
				subQ->byIndex = 1;
			}
			// Super Schwarzschild 2 (Japan)
			// LBA[234845, 0x3955D], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-19, Index-01, RelativeTime-01:50:09, AbsoluteTime-52:13:20] RtoW:ZERO mode
			// LBA[234846, 0x3955E], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-19, Index-02, RelativeTime-01:50:10, AbsoluteTime-52:13:21] RtoW:ZERO mode
			// LBA[234847, 0x3955F], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number (MCN)[0000000000000        , AbsoluteTime-     :22] RtoW:ZERO mode
			// LBA[234848, 0x39560], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-19, Index-01, RelativeTime-01:50:12, AbsoluteTime-52:13:23] RtoW:ZERO mode
			else if(prevSubQ->byIndex > 1) {
				subQ->byIndex = prevPrevSubQ->byIndex;
				prevSubQ->byIndex = prevPrevSubQ->byIndex;
			}
			// Cosmic Fantasy 2
			// LBA[202749, 0x317FD], Data, Copy NG, TOC[TrackNum-80, Index-01, RelativeTime-00:06:63, AbsoluteTime-45:05:24] RtoW:ZERO mode
			// LBA[202750, 0x317FE], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number (MCN)[0000000000000        , AbsoluteTime-     :25] RtoW:ZERO mode
			// LBA[202751, 0x317FF], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-81, Index-00, RelativeTime-00:01:73, AbsoluteTime-45:05:26] RtoW:ZERO mode
			else if((prevSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK &&
			(subQ->byCtl & AUDIO_DATA_TRACK) == 0) {
				subQ->byIndex = 0; // TODO
			}
			else {
				subQ->byIndex = prevSubQ->byIndex;
			}

			if(subQ->byIndex == 0) {
				subQ->nRelativeTime = prevSubQ->nRelativeTime - 1;
			}
			else if(subQ->byIndex > 0) {
				subQ->nRelativeTime = prevSubQ->nRelativeTime + 1;
			}

		}
	}
	else if(subQ->byAdr == ADR_ENCODES_ISRC) {
		BOOL bISRC = IsValidISRC(Subcode);
		SetISRCToString(Subcode, *byCurrentTrackNum, szISRC, bISRC);
		szISRC[12] = '\0';
		aISRC[*byCurrentTrackNum-1] = bISRC;
		if(!bISRC) {
			OutputLogString(fpLog, _T("LBA %6d, ISRC[%s]\n"), nLBA, szISRC);
			subQ->byAdr = prevSubQ->byAdr;
			bBadAdr = TRUE;
		}
		else {
			// because don't exist tracknum, index...
			subQ->byTrackNum = prevSubQ->byTrackNum;
			subQ->byIndex = prevSubQ->byIndex;
			if(subQ->byIndex == 0) {
				subQ->nRelativeTime = prevSubQ->nRelativeTime - 1;
			}
			else if(subQ->byIndex > 0) {
				subQ->nRelativeTime = prevSubQ->nRelativeTime + 1;
			}
		}
	}
	else if(subQ->byAdr == ADR_NO_MODE_INFORMATION || 
			subQ->byAdr > ADR_ENCODES_ISRC) {
			OutputLogString(fpLog, _T("LBA %6d, Adr[%d], correct[%d]\n"), 
				nLBA, subQ->byAdr, prevSubQ->byAdr);
			subQ->byAdr = prevSubQ->byAdr;
			bBadAdr = TRUE;
	}
	if(bBadAdr) {
		Subcode[12] = (UCHAR)(subQ->byCtl << 4 | subQ->byAdr);
	}
	if(subQ->byAdr == ADR_ENCODES_CURRENT_POSITION || bBadAdr) {
		BOOL bPrevTrackNum = TRUE;
		if(!IsValidTrackNumber(prevPrevSubQ, prevSubQ,
			subQ, toc->FirstTrack, toc->LastTrack, &bPrevTrackNum)) {
			OutputLogString(fpLog, _T("LBA %6d, TrackNum[%02d], correct[%02d]\n"), 
				nLBA, subQ->byTrackNum, prevSubQ->byTrackNum);
			// Bikkuriman Daijikai (Japan)
			// LBA[106402, 0x19FA2], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-70, Index-01, RelativeTime-00:16:39, AbsoluteTime-23:40:52] RtoW:ZERO mode
			// LBA[106403, 0x19FA3], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-79, Index-01, RelativeTime-00:00:00, AbsoluteTime-21:40:53] RtoW:ZERO mode
			// LBA[106404, 0x19FA4], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-71, Index-01, RelativeTime-00:00:01, AbsoluteTime-23:40:54] RtoW:ZERO mode
			// Kagami no Kuni no Legend (Japan)
			// LBA[004373, 0x01115], Data, Copy NG, TOC[TrackNum-02, Index-00, RelativeTime-00:00:01, AbsoluteTime-01:00:23] RtoW:ZERO mode
			// LBA[004374, 0x01116], Data, Copy NG, TOC[TrackNum-0a, Index-00, RelativeTime-00:00:00, AbsoluteTime-01:00:24] RtoW:ZERO mode
			// LBA[004375, 0x01117], Data, Copy NG, TOC[TrackNum-02, Index-01, RelativeTime-00:00:00, AbsoluteTime-01:00:25] RtoW:ZERO mode
			subQ->byTrackNum = prevSubQ->byTrackNum;
			if(prevSubQ->byTrackNum < toc->LastTrack &&
				subQ->byIndex == 1 &&
				subQ->nRelativeTime == 0) {
				subQ->byTrackNum += 1;
			}
			Subcode[13] = DecToBcd(subQ->byTrackNum);
		}
		else if(!bPrevTrackNum) {
			OutputLogString(fpLog, _T("LBA %6d, PrevTrackNum[%02d], correct[%02d]\n"), 
				nLBA, prevSubQ->byTrackNum, prevPrevSubQ->byTrackNum);
			aLBAStart[prevSubQ->byTrackNum-1][prevSubQ->byIndex] = -1;
			aLBAOfDataTrack[prevSubQ->byTrackNum-1][0] = -1;
			prevSubQ->byTrackNum = prevPrevSubQ->byTrackNum;
		}
		BOOL bPrevIndex = TRUE;
		BOOL bPrevPrevIndex = TRUE;
		if(!IsValidIndex(prevPrevSubQ, prevSubQ, subQ, &bPrevIndex, &bPrevPrevIndex)) {
			OutputLogString(fpLog, _T("LBA %6d, Index[%02d], correct[%02d]\n"), 
				nLBA, subQ->byIndex, prevSubQ->byIndex);
			subQ->byTrackNum = prevSubQ->byTrackNum;
			subQ->byIndex = prevSubQ->byIndex;
			Subcode[14] = DecToBcd(subQ->byIndex);
		}
		else if(!bPrevIndex) {
			OutputLogString(fpLog, _T("LBA %6d, PrevIndex[%02d], correct[%02d]\n"), 
				nLBA - 1 , prevSubQ->byIndex, prevPrevSubQ->byIndex);
			aLBAStart[prevSubQ->byTrackNum-1][prevSubQ->byIndex] = -1;
			prevSubQ->byTrackNum = prevPrevSubQ->byTrackNum;
			prevSubQ->byIndex = prevPrevSubQ->byIndex;
		}
		else if(!bPrevPrevIndex) {
			OutputLogString(fpLog, _T("LBA %6d, PrevPrevIndex[%02d], correct[%02d]\n"), 
				nLBA - 1 , prevPrevSubQ->byIndex, prevSubQ->byIndex);
			aLBAStart[prevPrevSubQ->byTrackNum-1][prevPrevSubQ->byIndex] = -1;
			prevPrevSubQ->byTrackNum = prevSubQ->byTrackNum;
			prevPrevSubQ->byIndex = prevSubQ->byIndex;
		}

		if(!IsValidRelativeTime(prevSubQ, subQ, Subcode)) {
			UCHAR byFrame = 0;
			UCHAR bySecond = 0;
			UCHAR byMinute = 0;
			if(!(prevSubQ->byIndex == 0 && subQ->byIndex == 1) &&
				!(prevSubQ->byIndex == 1 && subQ->byIndex == 0)) {
				if(subQ->byIndex > 0) {
					LBAtoMSF(prevSubQ->nRelativeTime + 1, &byFrame, &bySecond, &byMinute);
					subQ->nRelativeTime = prevSubQ->nRelativeTime + 1;
				}
				else if(subQ->byIndex == 0) {
					LBAtoMSF(prevSubQ->nRelativeTime - 1, &byFrame, &bySecond, &byMinute);
					subQ->nRelativeTime = prevSubQ->nRelativeTime - 1;
				}
				OutputLogString(fpLog, 
					_T("LBA %6d, RelativeTime[%02d:%02d:%02d], correct[%02d:%02d:%02d]\n"), 
					nLBA, BcdToDec(Subcode[15]), BcdToDec(Subcode[16]), 
					BcdToDec(Subcode[17]), byMinute, bySecond, byFrame);
				Subcode[15] = DecToBcd(byMinute);
				Subcode[16] = DecToBcd(bySecond);
				Subcode[17] = DecToBcd(byFrame);
			}
		}

		if(Subcode[18] != 0) {
			OutputLogString(fpLog, 
				_T("LBA %6d, Zero[%02d], correct[0]\n"), nLBA, Subcode[18]);
			Subcode[18] = 0;
		}

		if(!IsValidAbsoluteTime(prevSubQ, subQ, Subcode, nLBA)) {
			UCHAR byFrame, bySecond, byMinute;
			LBAtoMSF(nLBA + 150, &byFrame, &bySecond, &byMinute);
			OutputLogString(fpLog, 
				_T("LBA %6d, AbsoluteTime[%02d:%02d:%02d], correct[%02d:%02d:%02d]\n"), 
				nLBA, BcdToDec(Subcode[19]), BcdToDec(Subcode[20]), 
				BcdToDec(Subcode[21]), byMinute, bySecond, byFrame);
			Subcode[19] = DecToBcd(byMinute);
			Subcode[20] = DecToBcd(bySecond);
			Subcode[21] = DecToBcd(byFrame);
		}
	}
	if(!IsValidControl(prevPrevSubQ, prevSubQ, subQ, aEndCtl[subQ->byTrackNum-1])) {
		OutputLogString(fpLog, _T("LBA %6d, Ctl[%d], correct[%d]\n"),
			nLBA, subQ->byCtl, prevSubQ->byCtl);
		subQ->byCtl = prevSubQ->byCtl;
		Subcode[12] = (UCHAR)(subQ->byCtl << 4 | subQ->byAdr);
	}
	USHORT crc16 = GetCrc16CCITT(10, &Subcode[12]);
	UCHAR tmp1 = (UCHAR)(crc16 >> 8 & 0xFF);
	UCHAR tmp2 = (UCHAR)(crc16 & 0xFF);
	if(Subcode[22] != tmp1) {
		OutputLogString(fpLog, _T("LBA %6d, Crc high[%02x], correct[%02x]\n"),
			nLBA, Subcode[22], tmp1);
		Subcode[22] = tmp1;
	}
	if(Subcode[23] != tmp2) {
		OutputLogString(fpLog, _T("LBA %6d, Crc low[%02x], correct[%02x]\n"),
			nLBA, Subcode[23], tmp2);
		Subcode[23] = tmp2;
	}
	return TRUE;
}
