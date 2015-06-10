/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"
#include "struct.h"
#include "check.h"
#include "convert.h"
#include "execMMC.h"
#include "execMMCforCD.h"
#include "output.h"
#include "set.h"
#include "_external/crc16ccitt.h"

BOOL IsValidMSF(
	LPBYTE lpSubcode,
	INT m,
	INT s,
	INT f
	)
{
	BOOL bRet = TRUE;
	if (lpSubcode[m] > 0x99) {
		bRet = FALSE;
	}
	else if (lpSubcode[s] > 0x59) {
		bRet = FALSE;
	}
	else if (lpSubcode[f] > 0x74) {
		bRet = FALSE;
	}
	return bRet;
}

BOOL IsValidRelativeTime(
	PSUB_Q_DATA pPrevSubQ,
	PSUB_Q_DATA pSubQ,
	LPBYTE lpSubcode,
	INT nLBA
	)
{
	BOOL bRet = IsValidMSF(lpSubcode, 15, 16, 17);
	if (bRet) {
		INT tmpLBA = MSFtoLBA(BcdToDec(lpSubcode[17]), 
			BcdToDec(lpSubcode[16]), BcdToDec(lpSubcode[15]));
		if (pSubQ->byIndex > 0) {
			if (tmpLBA != 0 &&
				pPrevSubQ->nRelativeTime + 1 != pSubQ->nRelativeTime) {
				bRet = FALSE;
			}
		}
		else if (pSubQ->byIndex == 0) {
			// SagaFrontier Original Sound Track (Disc 3)
			// LBA[009948, 0x026DC], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-01, Index-00, RelativeTime-00:00:01, AbsoluteTime-02:14:48] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
			// LBA[009949, 0x026DD], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-01, Index-00, RelativeTime-00:00:00, AbsoluteTime-02:14:49] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
			// LBA[009950, 0x026DE], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-01, Index-01, RelativeTime-00:00:00, AbsoluteTime-02:14:50] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
			// LBA[009951, 0x026DF], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-01, Index-01, RelativeTime-00:00:01, AbsoluteTime-02:14:51] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
			if (tmpLBA != 0 && pPrevSubQ->byTrackNum == pSubQ->byTrackNum &&
				pPrevSubQ->nRelativeTime != pSubQ->nRelativeTime + 1) {
				// Now on Never (Nick Carter) (ZJCI-10118)
				// LBA[000599, 0x00257], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-01, Index-00, RelativeTime-00:00:01, AbsoluteTime-00:09:74] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
				// LBA[000600, 0x00258], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-01, Index-01, RelativeTime-00:00:00, AbsoluteTime-00:10:00] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
				// LBA[000601, 0x00259], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-01, Index-01, RelativeTime-00:00:01, AbsoluteTime-00:10:01] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
				if (tmpLBA != 0 && pPrevSubQ->byTrackNum == pSubQ->byTrackNum &&
					pPrevSubQ->nRelativeTime != pSubQ->nRelativeTime) {
					bRet = FALSE;
				}
			}
		}
		if (nLBA != 0 && tmpLBA == 0 && pPrevSubQ->byTrackNum == pSubQ->byTrackNum &&
			pPrevSubQ->byIndex == pSubQ->byIndex && pSubQ->byIndex != 0) {
			if (pPrevSubQ->nRelativeTime + 1 != pSubQ->nRelativeTime ||
				pPrevSubQ->nRelativeTime != pSubQ->nRelativeTime + 1) {
				bRet = FALSE;
			}
		}
	}
	return bRet;
}

BOOL IsValidAbsoluteTime(
	PSUB_Q_DATA pPrevSubQ,
	PSUB_Q_DATA pSubQ,
	LPBYTE lpSubcode,
	INT nLBA
	)
{
	BOOL bRet = IsValidMSF(lpSubcode, 19, 20, 21);
	if (bRet) {
		INT tmpLBA = MSFtoLBA(BcdToDec(lpSubcode[21]), 
			BcdToDec(lpSubcode[20]), BcdToDec(lpSubcode[19])) - 150;
		if (nLBA != tmpLBA ||
			pPrevSubQ->nAbsoluteTime + 1 != pSubQ->nAbsoluteTime) {
			bRet = FALSE;
		}
	}
	return bRet;
}

BOOL IsValidControl(
	PSUB_Q_DATA pPrevPrevSubQ,
	PSUB_Q_DATA pPrevSubQ,
	PSUB_Q_DATA pSubQ,
	BYTE byEndCtl
	)
{
	BOOL bRet = FALSE;
	switch (pSubQ->byCtl) {
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

	if (pPrevSubQ->byCtl != pSubQ->byCtl) {
		if ((pSubQ->byCtl != byEndCtl) && pSubQ->byCtl != 0) {
			bRet = FALSE;
		}
		else {
			if (pSubQ->byAdr == ADR_ENCODES_CURRENT_POSITION) {
				if (pPrevSubQ->byTrackNum + 1 == pSubQ->byTrackNum) {
					if (pPrevSubQ->nRelativeTime + 1 == pSubQ->nRelativeTime) {
						bRet = FALSE;
					}
					else {
						bRet = TRUE;
					}
				}
				else if (pPrevSubQ->byTrackNum == pSubQ->byTrackNum &&
					pPrevSubQ->byIndex == pSubQ->byIndex) {
					if (pPrevSubQ->nRelativeTime + 1 == pSubQ->nRelativeTime) {
						bRet = FALSE;
					}
					else {
						bRet = TRUE;
					}
				}
			}
			// EVE - burst error (Disc 3) (Terror Disc)
			// LBA[188021, 0x2DE75], Data, Copy NG, TOC[TrackNum-01, Index-01, RelativeTime-41:46:71, AbsoluteTime-41:48:71] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
			// LBA[188022, 0x2DE76], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number (MCN)[0000000000000        , AbsoluteTime-     :72] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
			// LBA[188023, 0x2DE77], Data, Copy NG, TOC[TrackNum-01, Index-01, RelativeTime-41:46:73, AbsoluteTime-41:48:73] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
			else if (pSubQ->byAdr == ADR_ENCODES_MEDIA_CATALOG) {
				bRet = FALSE;
			}
			if (pPrevPrevSubQ->byCtl == pSubQ->byCtl) {
				bRet = TRUE;
			}
		}
	}
	return bRet;
}

BOOL IsValidDataHeader(
	LPBYTE lpSrc
	)
{
	BYTE aHeader[] = { 
		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00
	};
	BOOL bRet = TRUE;
	for (INT c = 0; c < sizeof(aHeader); c++) {
		if (lpSrc[c] != aHeader[c]) {
			bRet = FALSE;
			break;
		}
	}
	return bRet;
}

BOOL IsValidIndex(
	PSUB_Q_DATA pPrevPrevSubQ,
	PSUB_Q_DATA pPrevSubQ,
	PSUB_Q_DATA pSubQ,
	LPBOOL bPrevIndex,
	LPBOOL bPrevPrevIndex
	)
{
	BOOL bRet = TRUE;
	if (pPrevSubQ->byIndex != pSubQ->byIndex) {
		if (pPrevSubQ->byIndex + 1 < pSubQ->byIndex) {
			bRet = FALSE;
		}
		else if (MAXIMUM_NUMBER_INDEXES < pSubQ->byIndex) {
			bRet = FALSE;
		}
		else {
			bRet = FALSE;
			if (pPrevSubQ->byIndex == 0 &&
				pSubQ->byIndex  == 1 && 
				pSubQ->nRelativeTime == 0) {
				bRet = TRUE;
			}
			else if ((pSubQ->byIndex  == 0 || pSubQ->byIndex  == 1) && 
				pPrevSubQ->byTrackNum + 1 == pSubQ->byTrackNum &&
				pSubQ->nRelativeTime < pPrevSubQ->nRelativeTime) {
				bRet = TRUE;
			}
			else if (pPrevSubQ->byIndex + 1 == pSubQ->byIndex && 
				pPrevSubQ->nRelativeTime + 1 == pSubQ->nRelativeTime) {
				bRet = TRUE;
			}
			if (pPrevPrevSubQ->byIndex == pSubQ->byIndex) {
				bRet = TRUE;
				if (pPrevSubQ->byIndex - 1 == pSubQ->byIndex) {
					*bPrevIndex = FALSE;
				}
			}
		}
	}
	else if (pPrevSubQ->byIndex == pSubQ->byIndex &&
		pPrevPrevSubQ->byIndex - 1 == pSubQ->byIndex &&
		pPrevPrevSubQ->byTrackNum + 1 != pSubQ->byTrackNum) {
		*bPrevPrevIndex = FALSE;
	}
	return bRet;
}

// reference
// https://isrc.jmd.ne.jp/about/pattern.html
// https://isrc.jmd.ne.jp/about/error.html
BOOL IsValidISRC(
	LPBYTE lpSubcode
	)
{
	INT ch = ((lpSubcode[13] >> 2) & 0x3F) + 0x30;
	if (isupper(ch) == 0) {
		return FALSE;
	}

	ch = (((lpSubcode[13] << 4) & 0x30) | 
		((lpSubcode[14] >> 4) & 0x0F)) + 0x30;
	if (isupper(ch) == 0) {
		return FALSE;
	}

	ch = (((lpSubcode[14] << 2) & 0x3C) | 
		((lpSubcode[15] >> 6) & 0x03)) + 0x30;
	if (isupper(ch) == 0 && isdigit(ch) == 0) {
		return FALSE;
	}

	ch = (lpSubcode[15] & 0x3F) + 0x30;
	if (isupper(ch) == 0 && isdigit(ch) == 0) {
		return FALSE;
	}

	ch = ((lpSubcode[16] >> 2) & 0x3F) + 0x30;
	if (isupper(ch) == 0 && isdigit(ch) == 0) {
		return FALSE;
	}

	if ((lpSubcode[16] & 0x03) != 0) {
		return FALSE;
	}

	for (INT i = 17; i <= 19; i++) {
		if (isdigit(((lpSubcode[i] >> 4) & 0x0F) + 0x30) == 0) {
			return FALSE;
		}
		if (isdigit((lpSubcode[i] & 0x0F) + 0x30) == 0) {
			return FALSE;
		}
	}

	if (isdigit(((lpSubcode[20] >> 4) & 0x0F) + 0x30) == 0) {
		return FALSE;
	}

	if ((lpSubcode[20] & 0x0F) != 0) {
		return FALSE;
	}
	return TRUE;
}

BOOL IsValidMCN(
	LPBYTE lpSubcode
	)
{
	BOOL bRet = TRUE;
	for (INT i = 13; i <= 19; i++) {
		if (isdigit(((lpSubcode[i] >> 4) & 0x0F) + 0x30) == 0) {
			bRet = FALSE;
			break;
		}
		if (isdigit((lpSubcode[i] & 0x0F) + 0x30) == 0) {
			bRet = FALSE;
			break;
		}
	}
	if ((lpSubcode[19] & 0x0F) != 0 || lpSubcode[20] != 0) {
		bRet = FALSE;
	}
	return bRet;
}

BOOL IsValidTrackNumber(
	PSUB_Q_DATA pPrevPrevSubQ,
	PSUB_Q_DATA pPrevSubQ,
	PSUB_Q_DATA pSubQ,
	BYTE byFirstTrackNum,
	BYTE byLastTrackNum,
	LPBOOL bPrevTrackNum
	)
{
	BOOL bRet = TRUE;
	if (pSubQ->byTrackNum < byFirstTrackNum || 
		byLastTrackNum < pSubQ->byTrackNum) {
		bRet = FALSE;
	}
	else if (pPrevSubQ->byTrackNum != pSubQ->byTrackNum) {
		if (pPrevSubQ->byTrackNum + 2 <= pSubQ->byTrackNum) {
			bRet = FALSE;
		}
		else if (pPrevSubQ->byTrackNum > pSubQ->byTrackNum) {
			if (pPrevPrevSubQ->byTrackNum == pSubQ->byTrackNum) {
				*bPrevTrackNum = FALSE;
			}
			else {
				bRet = FALSE;
			}
		}
		else {
			if (pPrevSubQ->byTrackNum + 1 == pSubQ->byTrackNum) {
				if ((pPrevSubQ->nRelativeTime + 1 <= pSubQ->nRelativeTime) ||
					(pPrevPrevSubQ->nRelativeTime + 2 <= pSubQ->nRelativeTime)) {
					bRet = FALSE;
				}
			}
			if (pPrevPrevSubQ->byTrackNum == pSubQ->byTrackNum) {
				bRet = TRUE;
			}
		}
	}
	return bRet;
}

BOOL IsValidPregapSector(
	PDISC_DATA pDiscData,
	PSUB_Q_DATA pNextSubQ,
	PSUB_Q_DATA pSubQ,
	PSUB_Q_DATA pPrevSubQ,
	INT nLBA
	)
{
	BOOL bRet = FALSE;
	if ((nLBA == pDiscData->pnTocStartLBA[pPrevSubQ->byTrackNum] - 225 ||
		nLBA == pDiscData->pnTocStartLBA[pPrevSubQ->byTrackNum] - 150) &&
		(pSubQ->byCtl & AUDIO_DATA_TRACK) == 0 &&
		(pNextSubQ->byCtl & AUDIO_DATA_TRACK) == 0 &&
		pNextSubQ->byIndex == 0
		) {
		bRet = TRUE;
	}
	return bRet;
}

BOOL Is3DOData(
	LPBYTE lpSrc
	)
{
	BYTE aHeader[] = { 
		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
		0x00, 0x02, 0x00, 0x01, 0x01, 0x5A,
		0x5A, 0x5A, 0x5A, 0x5A, 0x01, 0x00
	};
	BOOL bRet = TRUE;
	for (INT c = 0; c < sizeof(aHeader); c++) {
		if (lpSrc[c] != aHeader[c]) {
			bRet = FALSE;
			break;
		}
	}
	return bRet;
}

BOOL IsMacData(
	LPBYTE lpSrc
	)
{
	BOOL bRet = TRUE;
	if (lpSrc[0] != 0x42 || lpSrc[1] != 0x44) {
		bRet = FALSE;
	}
	return bRet;
}

BOOL IsPlextorDrive(
	PDEVICE_DATA pDevData
	)
{
	if (!strncmp(pDevData->szVendorId, "PLEXTOR", 7)) {
		// PX-504A, PX-740A, PX-750A, PX-751A don't support...
		// Sense data, Key:Asc:Ascq: 05:20:00(ILLEGAL_REQUEST. INVALID COMMAND OPERATION CODE)
		if (!strncmp(pDevData->szProductId, "DVDR   PX-760A", 14)) {
			pDevData->bPlextor = TRUE;
			pDevData->PLEX_DRIVE_TYPE.bPlextorPX760A = TRUE;
		}
		else if (!strncmp(pDevData->szProductId, "DVDR   PX-755A", 14)) {
			pDevData->bPlextor = TRUE;
			pDevData->PLEX_DRIVE_TYPE.bPlextorPX755A = TRUE;
		}
		else if (!strncmp(pDevData->szProductId, "DVDR   PX-716A", 14)) {
			pDevData->bPlextor = TRUE;
			pDevData->PLEX_DRIVE_TYPE.bPlextorPX716A = TRUE;
		}
		else if (!strncmp(pDevData->szProductId, "DVDR   PX-712A", 14)) {
			pDevData->bPlextor = TRUE;
			pDevData->PLEX_DRIVE_TYPE.bPlextorPX712A = TRUE;
		}
		else if (!strncmp(pDevData->szProductId, "DVDR   PX-708A", 14)) {
			pDevData->bPlextor = TRUE;
			pDevData->PLEX_DRIVE_TYPE.bPlextorPX708A = TRUE;
		}
		else if (!strncmp(pDevData->szProductId, "DVDR   PX-320A", 14)) {
			pDevData->bPlextor = TRUE;
			pDevData->PLEX_DRIVE_TYPE.bPlextorPX320A = TRUE;
		}
		else if (!strncmp(pDevData->szProductId, "CD-R   PX-W5232A", 16)) {
			pDevData->bPlextor = TRUE;
			pDevData->PLEX_DRIVE_TYPE.bPlextorPXW5232A = TRUE;
		}
		else if (!strncmp(pDevData->szProductId, "CD-R   PX-W5224A", 16)) {
			pDevData->bPlextor = TRUE;
			pDevData->PLEX_DRIVE_TYPE.bPlextorPXW5224A = TRUE;
		}
		else if (!strncmp(pDevData->szProductId, "CD-R   PX-W4824A", 16)) {
			pDevData->bPlextor = TRUE;
			pDevData->PLEX_DRIVE_TYPE.bPlextorPXW4824A = TRUE;
		}
		else if (!strncmp(pDevData->szProductId, "CD-R   PX-W4012A", 16)) {
			pDevData->bPlextor = TRUE;
			pDevData->PLEX_DRIVE_TYPE.bPlextorPXW4012A = TRUE;
		}
		else if (!strncmp(pDevData->szProductId, "CD-R   PX-W2410A", 16)) {
			pDevData->bPlextor = TRUE;
			pDevData->PLEX_DRIVE_TYPE.bPlextorPXW2410A = TRUE;
		}
		else if (!strncmp(pDevData->szProductId, "CD-R   PX-W1610A", 16)) {
			pDevData->bPlextor = TRUE;
			pDevData->PLEX_DRIVE_TYPE.bPlextorPXW1610A = TRUE;
		}
		else if (!strncmp(pDevData->szProductId, "CD-R   PX-W1210A", 16)) {
			pDevData->bPlextor = TRUE;
			pDevData->PLEX_DRIVE_TYPE.bPlextorPXW1210A = TRUE;
		}
		else if (!strncmp(pDevData->szProductId, "CD-R   PX-W8432T", 16)) {
			pDevData->bPlextor = TRUE;
			pDevData->PLEX_DRIVE_TYPE.bPlextorPXW8432T = TRUE;
		}
	}
	return TRUE;
}

BOOL CheckAndFixPSubchannel(
	LPBYTE lpSubcode
	)
{
	BOOL bFF = FALSE;
	BOOL b00 = FALSE;
	for (INT i = 0; i < 12; i++) {
		if (lpSubcode[i] == 0xFF) {
			bFF = TRUE;
			break;
		}
		else if (lpSubcode[i] == 0x00) {
			b00 = TRUE;
			break;
		}
	}
	for (INT i = 0; i < 12; i++) {
		if (bFF && lpSubcode[i] != 0xFF) {
			lpSubcode[i] = 0xFF;
		}
		else if (b00 && lpSubcode[i] != 0x00) {
			lpSubcode[i] = 0x00;
		}
	}
	return TRUE;
}

BOOL CheckAndFixSubchannel(
	PEXT_ARG pExtArg,
	PDISC_DATA pDiscData,
	LPBYTE lpSubcode,
	PSUB_Q_DATA pNextSubQ,
	PSUB_Q_DATA pSubQ,
	PSUB_Q_DATA pPrevSubQ,
	PSUB_Q_DATA pPrevPrevSubQ,
	LPBYTE lpCurrentTrackNum,
	LPBOOL lpCatalog,
	LPBOOL lpISRCList,
	LPBYTE lpEndCtlList,
	LPINT* lpLBAStartList,
	LPINT* lpLBAOfDataTrackList,
	INT nLBA
	)
{
	CheckAndFixPSubchannel(lpSubcode);

	_TCHAR szCatalog[META_CATALOG_SIZE] = { 0 };
	_TCHAR szISRC[META_ISRC_SIZE] = { 0 };
	BOOL bBadAdr = FALSE;

	if (pSubQ->byAdr == ADR_ENCODES_MEDIA_CATALOG) {
		BOOL bMCN = IsValidMCN(lpSubcode);
		SetMCNToString(pDiscData, lpSubcode, szCatalog, bMCN);
		// only once
		if (pPrevSubQ->byTrackNum == pDiscData->toc.FirstTrack) {
			*lpCatalog = bMCN;
		}
		if (!bMCN) {
			OutputErrorLog(_T("LBA %6d, Track[%02d]: MCN[%s]\n"),
				nLBA, *lpCurrentTrackNum, szCatalog);
			pSubQ->byAdr = pPrevSubQ->byAdr;
			bBadAdr = TRUE;
		}
		else {
			// Subchannel pattern on EAN Sector
			// pattern 1: pregap sector.
			if (IsValidPregapSector(pDiscData, pNextSubQ, pSubQ, pPrevSubQ, nLBA)) {
				// pattern 1-1: prev sector is audio.
				if ((pPrevSubQ->byCtl & AUDIO_DATA_TRACK) == 0) {
					// pattern 1-1-1: present sector is audio.
					if ((pSubQ->byCtl & AUDIO_DATA_TRACK) == 0) {
						if (nLBA == pDiscData->pnTocStartLBA[pPrevSubQ->byTrackNum] - 225) {
							// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
							// LBA[261585, 0x3FDD1], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-95, Index-01, RelativeTime-00:13:69, AbsoluteTime-58:09:60] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
							// LBA[261586, 0x3FDD2], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number (MCN)[0000000000000        , AbsoluteTime-     :61] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
							// LBA[261587, 0x3FDD3], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-96, Index-00, RelativeTime-00:02:73, AbsoluteTime-58:09:62] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
							pSubQ->nRelativeTime = 224;
							// pattern 1-1-1-1: change track.
							pSubQ->byTrackNum = (BYTE)(pPrevSubQ->byTrackNum + 1);
							// pattern 1-1-1-1-1: change index. (pregap sector is 0)
							pSubQ->byIndex = 0;
						}
						else if (nLBA == pDiscData->pnTocStartLBA[pPrevSubQ->byTrackNum] - 150) {
							pSubQ->nRelativeTime = 149;
							// pattern 1-1-1-1: change track.
							pSubQ->byTrackNum = (BYTE)(pPrevSubQ->byTrackNum + 1);
							// pattern 1-1-1-1-1: change index. (pregap sector is 0)
							pSubQ->byIndex = 0;
						}
						else {
							// pattern 1-1-1-2: not change track.
							pSubQ->byTrackNum = pPrevSubQ->byTrackNum;
							// pattern 1-1-1-2-2: not change index.
							pSubQ->byIndex = pPrevSubQ->byIndex;
						}
					}
					// pattern 1-1-2: present sector is data.
					else if ((pSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
						if (nLBA == pDiscData->pnTocStartLBA[pPrevSubQ->byTrackNum] - 225) {
							pSubQ->nRelativeTime = 224;
							// pattern 1-1-2-1: change track.
							pSubQ->byTrackNum = (BYTE)(pPrevSubQ->byTrackNum + 1);
							// pattern 1-1-2-1-1: change index. (pregap sector is 0)
							pSubQ->byIndex = 0;
						}
						else if (nLBA == pDiscData->pnTocStartLBA[pPrevSubQ->byTrackNum] - 150) {
							pSubQ->nRelativeTime = 149;
							// pattern 1-1-2-1: change track.
							pSubQ->byTrackNum = (BYTE)(pPrevSubQ->byTrackNum + 1);
							// pattern 1-1-2-1-1: change index. (pregap sector is 0)
							pSubQ->byIndex = 0;
						}
						else {
							// pattern 1-1-2-2: not change track.
							pSubQ->byTrackNum = pPrevSubQ->byTrackNum;
							// pattern 1-1-2-2-2: not change index.
							pSubQ->byIndex = pPrevSubQ->byIndex;
						}
					}
				}
				// pattern 1-2: prev sector is data.
				else if ((pPrevSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					// pattern 1-2-1: present sector is audio.
					if ((pSubQ->byCtl & AUDIO_DATA_TRACK) == 0) {
						if (nLBA == pDiscData->pnTocStartLBA[pPrevSubQ->byTrackNum] - 225) {
							pSubQ->nRelativeTime = 224;
							// pattern 1-2-1-1: change track.
							pSubQ->byTrackNum = (BYTE)(pPrevSubQ->byTrackNum + 1);
							// pattern 1-2-1-1-1: change index. (pregap sector is 0)
							pSubQ->byIndex = 0;
						}
						else if (nLBA == pDiscData->pnTocStartLBA[pPrevSubQ->byTrackNum] - 150) {
							// Valis IV (Japan)
							// LBA[157830, 0x26886], Data, Copy NG, TOC[TrackNum-44, Index-01, RelativeTime-00:06:27, AbsoluteTime-35:06:30] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
							// LBA[157831, 0x26887], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number (MCN)[0000000000000        , AbsoluteTime-     :31] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
							// LBA[157832, 0x26888], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-45, Index-00, RelativeTime-00:01:73, AbsoluteTime-35:06:32] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
							// Cosmic Fantasy 2
							// LBA[202749, 0x317FD], Data, Copy NG, TOC[TrackNum-80, Index-01, RelativeTime-00:06:63, AbsoluteTime-45:05:24] RtoW:ZERO byMode
							// LBA[202750, 0x317FE], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number (MCN)[0000000000000        , AbsoluteTime-     :25] RtoW:ZERO byMode
							// LBA[202751, 0x317FF], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-81, Index-00, RelativeTime-00:01:73, AbsoluteTime-45:05:26] RtoW:ZERO byMode
							pSubQ->nRelativeTime = 149;
							// pattern 1-2-1-1: change track.
							pSubQ->byTrackNum = (BYTE)(pPrevSubQ->byTrackNum + 1);
							// pattern 1-2-1-1-1: change index. (pregap sector is 0)
							pSubQ->byIndex = 0;
						}
						else {
							// pattern 1-2-1-2: not change track.
							pSubQ->byTrackNum = pPrevSubQ->byTrackNum;
							// pattern 1-2-1-2-2: not change index.
							pSubQ->byIndex = pPrevSubQ->byIndex;
						}
					}
					// pattern 1-2-2: present sector is data.
					else if ((pSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
						if (nLBA == pDiscData->pnTocStartLBA[pPrevSubQ->byTrackNum] - 225) {
							pSubQ->nRelativeTime = 224;
							// pattern 1-2-2-1: change track.
							pSubQ->byTrackNum = (BYTE)(pPrevSubQ->byTrackNum + 1);
							// pattern 1-2-2-1-1: change index. (pregap sector is 0)
							pSubQ->byIndex = 0;
						}
						else if (nLBA == pDiscData->pnTocStartLBA[pPrevSubQ->byTrackNum] - 150) {
							pSubQ->nRelativeTime = 149;
							// pattern 1-2-2-1: change track.
							pSubQ->byTrackNum = (BYTE)(pPrevSubQ->byTrackNum + 1);
							// pattern 1-2-2-1-1: change index. (pregap sector is 0)
							pSubQ->byIndex = 0;
						}
						else {
							// pattern 1-2-2-2: not change track.
							pSubQ->byTrackNum = pPrevSubQ->byTrackNum;
							// pattern 1-2-2-2-2: not change index.
							pSubQ->byIndex = pPrevSubQ->byIndex;
						}
					}
				}
			}
			// pattern 2: not pregap sector.
			else {
				// pattern 2-1: prev sector is audio.
				if ((pPrevSubQ->byCtl & AUDIO_DATA_TRACK) == 0) {
					// pattern 2-1-1: present sector is audio.
					if ((pSubQ->byCtl & AUDIO_DATA_TRACK) == 0) {
						// 1st sector
						if (nLBA == pDiscData->pnTocStartLBA[pPrevSubQ->byTrackNum]) {
							// Madou Monogatari I - Honoo no Sotsuenji (Japan)
							// LBA[183031, 0x2CAF7], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-21, Index-01, RelativeTime-00:31:70, AbsoluteTime-40:42:31] RtoW:ZERO byMode
							// LBA[183032, 0x2CAF8], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number (MCN)[0000000000000        , AbsoluteTime-     :32] RtoW:ZERO byMode
							// LBA[183033, 0x2CAF9], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-22, Index-01, RelativeTime-00:00:01, AbsoluteTime-40:42:33] RtoW:ZERO byMode
							pSubQ->nRelativeTime = 0;
							// pattern 2-1-1-1: change track.
							pSubQ->byTrackNum = (BYTE)(pPrevSubQ->byTrackNum + 1);
							// pattern 2-1-1-1-2: not change index.
							pSubQ->byIndex = pPrevSubQ->byIndex;
						}
						else if (pPrevSubQ->byIndex == 0 && pPrevSubQ->nRelativeTime == 0) {
							// Psychic Detective Series Vol. 5 - Nightmare (Japan)
							// LBA[080999, 0x13C67], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-02, Index-00, RelativeTime-00:00:00, AbsoluteTime-18:01:74] RtoW:ZERO byMode
							// LBA[081000, 0x13C68], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number (MCN)[3010911111863        , AbsoluteTime-     :00] RtoW:ZERO byMode
							// LBA[081001, 0x13C69], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-02, Index-01, RelativeTime-00:00:01, AbsoluteTime-18:02:01] RtoW:ZERO byMode
							pSubQ->nRelativeTime = 0;
							// pattern 2-1-1-2: not change track.
							pSubQ->byTrackNum = pPrevSubQ->byTrackNum;
							// pattern 2-1-1-2-1: change index.
							pSubQ->byIndex = (BYTE)(pPrevSubQ->byIndex + 1);
						}
						else if (pPrevSubQ->byIndex > 1 && pPrevSubQ->byIndex != pNextSubQ->byIndex) {
							// Super Schwarzschild 2 (Japan)
							// LBA[234845, 0x3955D], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-19, Index-01, RelativeTime-01:50:09, AbsoluteTime-52:13:20] RtoW:ZERO byMode
							// LBA[234846, 0x3955E], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-19, Index-02, RelativeTime-01:50:10, AbsoluteTime-52:13:21] RtoW:ZERO byMode
							// LBA[234847, 0x3955F], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number (MCN)[0000000000000        , AbsoluteTime-     :22] RtoW:ZERO byMode
							// LBA[234848, 0x39560], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-19, Index-01, RelativeTime-01:50:12, AbsoluteTime-52:13:23] RtoW:ZERO byMode
							// pattern 2-1-1-2: not change track.
							pSubQ->byTrackNum = pPrevSubQ->byTrackNum;
							pSubQ->byIndex = pPrevPrevSubQ->byIndex;
							pPrevSubQ->byIndex = pPrevPrevSubQ->byIndex;
						}
						else {
							if (pPrevSubQ->byIndex == 0) {
								// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
								// LBA[003413, 0x00D55], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-02, Index-00, RelativeTime-00:02:26, AbsoluteTime-00:47:38] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
								// LBA[003414, 0x00D56], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number (MCN)[0000000000000        , AbsoluteTime-     :39] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
								// LBA[003415, 0x00D57], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-02, Index-00, RelativeTime-00:02:24, AbsoluteTime-00:47:40] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
								pSubQ->nRelativeTime = pPrevSubQ->nRelativeTime - 1;
							}
							else if (pPrevSubQ->byIndex > 0) {
								// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
								// LBA[081541, 0x13E85], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-18, Index-01, RelativeTime-00:12:57, AbsoluteTime-18:09:16] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
								// LBA[081542, 0x13E86], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number (MCN)[0000000000000        , AbsoluteTime-     :17] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
								// LBA[081543, 0x13E87], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-18, Index-01, RelativeTime-00:12:59, AbsoluteTime-18:09:18] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
								pSubQ->nRelativeTime = pPrevSubQ->nRelativeTime + 1;
							}
							// pattern 2-1-1-2: not change track.
							pSubQ->byTrackNum = pPrevSubQ->byTrackNum;
							// pattern 2-1-1-2-2: not change index.
							pSubQ->byIndex = pPrevSubQ->byIndex;
						}
					}
					// pattern 2-1-2: present sector is data.
					else if ((pSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
						// 1st sector
						if (nLBA == pDiscData->pnTocStartLBA[pPrevSubQ->byTrackNum]) {
							pSubQ->nRelativeTime = 0;
							// pattern 2-1-2-1: change track.
							pSubQ->byTrackNum = (BYTE)(pPrevSubQ->byTrackNum + 1);
							// pattern 2-1-2-1-2: not change index.
							pSubQ->byIndex = pPrevSubQ->byIndex;
						}
						else if (pPrevSubQ->byIndex == 0 && pPrevSubQ->nRelativeTime == 0) {
							pSubQ->nRelativeTime = 0;
							// pattern 2-1-2-2: not change track.
							pSubQ->byTrackNum = pPrevSubQ->byTrackNum;
							// pattern 2-1-2-2-1: change index.
							pSubQ->byIndex = (BYTE)(pPrevSubQ->byIndex + 1);
						}
						else {
							if (pPrevSubQ->byIndex == 0) {
								pSubQ->nRelativeTime = pPrevSubQ->nRelativeTime - 1;
							}
							else if (pPrevSubQ->byIndex > 0) {
								pSubQ->nRelativeTime = pPrevSubQ->nRelativeTime + 1;
							}
							// pattern 2-1-2-2: not change track.
							pSubQ->byTrackNum = pPrevSubQ->byTrackNum;
							// pattern 2-1-2-2-2: not change index.
							pSubQ->byIndex = pPrevSubQ->byIndex;
						}
					}
				}
				// pattern 2-2: prev sector is data.
				else if ((pPrevSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					// pattern 2-2-1: present sector is audio.
					if ((pSubQ->byCtl & AUDIO_DATA_TRACK) == 0) {
						// 1st sector
						if (nLBA == pDiscData->pnTocStartLBA[pPrevSubQ->byTrackNum]) {
							pSubQ->nRelativeTime = 0;
							// pattern 2-2-1-1: change track.
							pSubQ->byTrackNum = (BYTE)(pPrevSubQ->byTrackNum + 1);
							// pattern 2-2-1-1-2: not change index.
							pSubQ->byIndex = pPrevSubQ->byIndex;
						}
						else if (pPrevSubQ->byIndex == 0 && pPrevSubQ->nRelativeTime == 0) {
							pSubQ->nRelativeTime = 0;
							// pattern 2-2-1-2: not change track.
							pSubQ->byTrackNum = pPrevSubQ->byTrackNum;
							// pattern 2-2-1-2-1: change index.
							pSubQ->byIndex = (BYTE)(pPrevSubQ->byIndex + 1);
						}
						else {
							if (pPrevSubQ->byIndex == 0) {
								pSubQ->nRelativeTime = pPrevSubQ->nRelativeTime - 1;
							}
							else if (pPrevSubQ->byIndex > 0) {
								// EVE - burst error (Disc 3) (Terror Disc)
								// LBA[188021, 0x2DE75], Data, Copy NG, TOC[TrackNum-01, Index-01, RelativeTime-41:46:71, AbsoluteTime-41:48:71] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
								// LBA[188022, 0x2DE76], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number (MCN)[0000000000000        , AbsoluteTime-     :72] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
								// LBA[188023, 0x2DE77], Data, Copy NG, TOC[TrackNum-01, Index-01, RelativeTime-41:46:73, AbsoluteTime-41:48:73] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
								pSubQ->nRelativeTime = pPrevSubQ->nRelativeTime + 1;
							}
							// pattern 2-2-1-2: not change track.
							pSubQ->byTrackNum = pPrevSubQ->byTrackNum;
							// pattern 2-2-1-2-2: not change index.
							pSubQ->byIndex = pPrevSubQ->byIndex;
						}
					}
					// pattern 2-2-2: present sector is data.
					else if ((pSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
						// 1st sector
						if (nLBA == pDiscData->pnTocStartLBA[pPrevSubQ->byTrackNum]) {
							pSubQ->nRelativeTime = 0;
							// pattern 2-2-2-1: change track.
							pSubQ->byTrackNum = (BYTE)(pPrevSubQ->byTrackNum + 1);
							// pattern 2-2-2-1-2: not change index.
							pSubQ->byIndex = pPrevSubQ->byIndex;
						}
						else if (pPrevSubQ->byIndex == 0 && pPrevSubQ->nRelativeTime == 0) {
							// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
							// LBA[142873, 0x22E19], Data, Copy NG, TOC[TrackNum-37, Index-00, RelativeTime-00:00:00, AbsoluteTime-31:46:73] RtoW:ZERO byMode
							// LBA[142874, 0x22E1A], Data, Copy NG, Media Catalog Number (MCN)[0000000000000        , AbsoluteTime-     :74] RtoW:ZERO byMode
							// LBA[142875, 0x22E1B], Data, Copy NG, TOC[TrackNum-37, Index-01, RelativeTime-00:00:01, AbsoluteTime-31:47:00] RtoW:ZERO byMode
							pSubQ->nRelativeTime = 0;
							// pattern 2-2-2-2: not change track.
							pSubQ->byTrackNum = pPrevSubQ->byTrackNum;
							// pattern 2-2-2-2-1: change index.
							pSubQ->byIndex = (BYTE)(pPrevSubQ->byIndex + 1);
						}
						else {
							if (pPrevSubQ->byIndex == 0) {
								pSubQ->nRelativeTime = pPrevSubQ->nRelativeTime - 1;
							}
							else if (pPrevSubQ->byIndex > 0) {
								// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
								// LBA[174261, 0x2A8B5], Data, Copy NG, TOC[TrackNum-60, Index-01, RelativeTime-00:06:19, AbsoluteTime-38:45:36] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
								// LBA[174262, 0x2A8B6], Data, Copy NG, Media Catalog Number (MCN)[0000000000000        , AbsoluteTime-     :37] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
								// LBA[174263, 0x2A8B7], Data, Copy NG, TOC[TrackNum-60, Index-01, RelativeTime-00:06:21, AbsoluteTime-38:45:38] RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode, RtoW:ZERO byMode
								pSubQ->nRelativeTime = pPrevSubQ->nRelativeTime + 1;
							}
							// pattern 2-2-2-2: not change track.
							pSubQ->byTrackNum = pPrevSubQ->byTrackNum;
							// pattern 2-2-2-2-2: not change index.
							pSubQ->byIndex = pPrevSubQ->byIndex;
						}
					}
				}
			}
		}
	}
	else if (pSubQ->byAdr == ADR_ENCODES_ISRC) {
		BOOL bISRC = IsValidISRC(lpSubcode);
		if (!bISRC && pExtArg->bIsrc) {
			// force a invalid ISRC to valid ISRC
			bISRC = pExtArg->bIsrc;
		}
		SetISRCToString(pDiscData, lpSubcode, szISRC, *lpCurrentTrackNum, bISRC);
		lpISRCList[*lpCurrentTrackNum-1] = bISRC;
		if (!bISRC) {
			OutputErrorLog(_T("LBA %6d, Track[%02d]: ISRC[%s]\n"),
				nLBA, *lpCurrentTrackNum, szISRC);
			pSubQ->byAdr = pPrevSubQ->byAdr;
			bBadAdr = TRUE;
		}
		else {
			// because don't exist tracknum, index...
			pSubQ->byTrackNum = pPrevSubQ->byTrackNum;
			pSubQ->byIndex = pPrevSubQ->byIndex;
			if (pSubQ->byIndex == 0) {
				pSubQ->nRelativeTime = pPrevSubQ->nRelativeTime - 1;
			}
			else if (pSubQ->byIndex > 0) {
				pSubQ->nRelativeTime = pPrevSubQ->nRelativeTime + 1;
			}
		}
	}
	else if (pSubQ->byAdr == ADR_NO_MODE_INFORMATION || 
		pSubQ->byAdr > ADR_ENCODES_ISRC) {
		OutputErrorLog(
			_T("LBA %6d, Track[%02d]: Adr[%d], correct[%d]\n"), 
			nLBA, *lpCurrentTrackNum, pSubQ->byAdr, pPrevSubQ->byAdr);
		pSubQ->byAdr = pPrevSubQ->byAdr;
		bBadAdr = TRUE;
	}
	if (bBadAdr) {
		lpSubcode[12] = (BYTE)(pSubQ->byCtl << 4 | pSubQ->byAdr);
	}
	if (pSubQ->byAdr == ADR_ENCODES_CURRENT_POSITION || bBadAdr) {
		BOOL bPrevTrackNum = TRUE;
		if (!IsValidTrackNumber(pPrevPrevSubQ, pPrevSubQ, pSubQ,
			pDiscData->toc.FirstTrack, pDiscData->toc.LastTrack, &bPrevTrackNum)) {
			OutputErrorLog(
				_T("LBA %6d, Track[%02d]: TrackNum[%02d], correct[%02d]\n"), 
				nLBA, *lpCurrentTrackNum, pSubQ->byTrackNum, pPrevSubQ->byTrackNum);
			// Bikkuriman Daijikai (Japan)
			// LBA[106402, 0x19FA2], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-70, Index-01, RelativeTime-00:16:39, AbsoluteTime-23:40:52] RtoW:ZERO byMode
			// LBA[106403, 0x19FA3], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-79, Index-01, RelativeTime-00:00:00, AbsoluteTime-21:40:53] RtoW:ZERO byMode
			// LBA[106404, 0x19FA4], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-71, Index-01, RelativeTime-00:00:01, AbsoluteTime-23:40:54] RtoW:ZERO byMode
			// Kagami no Kuni no Legend (Japan)
			// LBA[004373, 0x01115], Data, Copy NG, TOC[TrackNum-02, Index-00, RelativeTime-00:00:01, AbsoluteTime-01:00:23] RtoW:ZERO byMode
			// LBA[004374, 0x01116], Data, Copy NG, TOC[TrackNum-0a, Index-00, RelativeTime-00:00:00, AbsoluteTime-01:00:24] RtoW:ZERO byMode
			// LBA[004375, 0x01117], Data, Copy NG, TOC[TrackNum-02, Index-01, RelativeTime-00:00:00, AbsoluteTime-01:00:25] RtoW:ZERO byMode
			pSubQ->byTrackNum = pPrevSubQ->byTrackNum;
			if (pPrevSubQ->byTrackNum < pDiscData->toc.LastTrack &&
				pSubQ->byIndex == 1 &&
				pSubQ->nRelativeTime == 0) {
				pSubQ->byTrackNum += 1;
			}
			lpSubcode[13] = DecToBcd(pSubQ->byTrackNum);
		}
		else if (!bPrevTrackNum) {
			OutputErrorLog(
				_T("LBA %6d, Track[%02d]: PrevTrackNum[%02d], correct[%02d]\n"), 
				nLBA, *lpCurrentTrackNum, pPrevSubQ->byTrackNum, pPrevPrevSubQ->byTrackNum);
			lpLBAStartList[pPrevSubQ->byTrackNum-1][pPrevSubQ->byIndex] = -1;
			lpLBAOfDataTrackList[pPrevSubQ->byTrackNum-1][0] = -1;
			pPrevSubQ->byTrackNum = pPrevPrevSubQ->byTrackNum;
		}
		BOOL bPrevIndex = TRUE;
		BOOL bPrevPrevIndex = TRUE;
		if (!IsValidIndex(pPrevPrevSubQ, pPrevSubQ, pSubQ, &bPrevIndex, &bPrevPrevIndex)) {
			OutputErrorLog(
				_T("LBA %6d, Track[%02d]: Index[%02d], correct[%02d]\n"), 
				nLBA, *lpCurrentTrackNum, pSubQ->byIndex, pPrevSubQ->byIndex);
			pSubQ->byTrackNum = pPrevSubQ->byTrackNum;
			pSubQ->byIndex = pPrevSubQ->byIndex;
			lpSubcode[14] = DecToBcd(pSubQ->byIndex);
		}
		else if (!bPrevIndex) {
			OutputErrorLog(
				_T("LBA %6d, Track[%02d]: PrevIndex[%02d], correct[%02d]\n"), 
				nLBA - 1 , *lpCurrentTrackNum, pPrevSubQ->byIndex, pPrevPrevSubQ->byIndex);
			lpLBAStartList[pPrevSubQ->byTrackNum-1][pPrevSubQ->byIndex] = -1;
			pPrevSubQ->byTrackNum = pPrevPrevSubQ->byTrackNum;
			pPrevSubQ->byIndex = pPrevPrevSubQ->byIndex;
		}
		else if (!bPrevPrevIndex) {
			OutputErrorLog(
				_T("LBA %6d, Track[%02d]: PrevPrevIndex[%02d], correct[%02d]\n"), 
				nLBA - 1 , *lpCurrentTrackNum, pPrevPrevSubQ->byIndex, pPrevSubQ->byIndex);
			lpLBAStartList[pPrevPrevSubQ->byTrackNum-1][pPrevPrevSubQ->byIndex] = -1;
			pPrevPrevSubQ->byTrackNum = pPrevSubQ->byTrackNum;
			pPrevPrevSubQ->byIndex = pPrevSubQ->byIndex;
		}

		if (!IsValidRelativeTime(pPrevSubQ, pSubQ, lpSubcode, nLBA)) {
			BYTE byFrame = 0;
			BYTE bySecond = 0;
			BYTE byMinute = 0;
			if (!(pPrevSubQ->byIndex == 0 && pSubQ->byIndex == 1) &&
				!(pPrevSubQ->byIndex >= 1 && pSubQ->byIndex == 0)) {
				if (pSubQ->byIndex > 0) {
					LBAtoMSF(pPrevSubQ->nRelativeTime + 1, &byFrame, &bySecond, &byMinute);
					pSubQ->nRelativeTime = pPrevSubQ->nRelativeTime + 1;
				}
				else if (pSubQ->byIndex == 0) {
					LBAtoMSF(pPrevSubQ->nRelativeTime - 1, &byFrame, &bySecond, &byMinute);
					pSubQ->nRelativeTime = pPrevSubQ->nRelativeTime - 1;
				}
				OutputErrorLog(
					_T("LBA %6d, Track[%02d]: RelativeTime[%02d:%02d:%02d], correct[%02d:%02d:%02d]\n"), 
					nLBA, *lpCurrentTrackNum, BcdToDec(lpSubcode[15]), BcdToDec(lpSubcode[16]), 
					BcdToDec(lpSubcode[17]), byMinute, bySecond, byFrame);
				lpSubcode[15] = DecToBcd(byMinute);
				lpSubcode[16] = DecToBcd(bySecond);
				lpSubcode[17] = DecToBcd(byFrame);
			}
		}

		if (lpSubcode[18] != 0) {
			OutputErrorLog(
				_T("LBA %6d, Track[%02d]: Zero[%02d], correct[0]\n"),
				nLBA, *lpCurrentTrackNum, lpSubcode[18]);
			lpSubcode[18] = 0;
		}

		if (!IsValidAbsoluteTime(pPrevSubQ, pSubQ, lpSubcode, nLBA)) {
			BYTE byFrame, bySecond, byMinute;
			LBAtoMSF(nLBA + 150, &byFrame, &bySecond, &byMinute);
			OutputErrorLog(
				_T("LBA %6d, Track[%02d]: AbsoluteTime[%02d:%02d:%02d], correct[%02d:%02d:%02d]\n"), 
				nLBA, *lpCurrentTrackNum, BcdToDec(lpSubcode[19]), BcdToDec(lpSubcode[20]), 
				BcdToDec(lpSubcode[21]), byMinute, bySecond, byFrame);
			lpSubcode[19] = DecToBcd(byMinute);
			lpSubcode[20] = DecToBcd(bySecond);
			lpSubcode[21] = DecToBcd(byFrame);
		}
	}
	if (!IsValidControl(pPrevPrevSubQ, pPrevSubQ, pSubQ, lpEndCtlList[pSubQ->byTrackNum-1])) {
		OutputErrorLog(
			_T("LBA %6d, Track[%02d]: Ctl[%d], correct[%d]\n"),
			nLBA, *lpCurrentTrackNum, pSubQ->byCtl, pPrevSubQ->byCtl);
		pSubQ->byCtl = pPrevSubQ->byCtl;
		lpSubcode[12] = (BYTE)(pSubQ->byCtl << 4 | pSubQ->byAdr);
	}
	WORD crc16 = (WORD)GetCrc16CCITT(10, &lpSubcode[12]);
	BYTE tmp1 = HIBYTE(crc16);
	BYTE tmp2 = LOBYTE(crc16);
	if (lpSubcode[22] != tmp1) {
		OutputErrorLog(
			_T("LBA %6d, Track[%02d]: Crc high[%02x], correct[%02x]\n"),
			nLBA, *lpCurrentTrackNum, lpSubcode[22], tmp1);
		lpSubcode[22] = tmp1;
	}
	if (lpSubcode[23] != tmp2) {
		OutputErrorLog(
			_T("LBA %6d, Track[%02d]: Crc low[%02x], correct[%02x]\n"),
			nLBA, *lpCurrentTrackNum, lpSubcode[23], tmp2);
		lpSubcode[23] = tmp2;
	}
	return TRUE;
}

BOOL CheckByteError(
	PC2_ERROR_DATA pC2ErrorData,
	LPBYTE lpBuf,
	DWORD dwBufLen,
	INT nSlideLBANum,
	UINT i
	)
{
	BOOL bByteError = FALSE;
	DWORD dwPos = dwBufLen * nSlideLBANum;
	for (INT j = 0; j < CD_RAW_SECTOR_SIZE; j++) {
		if (pC2ErrorData[i].lpBufC2NoneSectorBackup[dwPos + j] != lpBuf[dwPos + j]) {
			bByteError = TRUE;
			break;
		}
	}
#if 0
	for (INT k = 0; k < byTransferLen; k++) {
		for (INT j = 0; j < CD_RAW_SECTOR_SIZE; j++) {
			DWORD nPos1 = dwBufLen * k + j;
			if (c2ErrorData[i].lpBufC2NoneSectorBackup[nPos1] != lpBuf[nPos1]) {
				nByteError = TRUE;
				break;
			}
		}
	}
#endif
	return bByteError;
}

BOOL CheckC2Error(
	PDISC_DATA pDiscData,
	PC2_ERROR_DATA pC2ErrorData,
	PREAD_CD_TRANSFER_DATA pTransferData,
	LPBYTE lpBuf,
	UINT uiC2ErrorLBACnt
	)
{
	BOOL bRet = RETURNED_C2_ERROR_1ST_NONE;
	DWORD dwC2ErrorPosPtr = pTransferData->dwBufC2Offset + 
		CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE * abs(pDiscData->nAdjustSectorNum);
	SHORT sC2Offset = (SHORT)(pDiscData->nCombinedOffset / CHAR_BIT);

	for (WORD wC2ErrorPos = 0; wC2ErrorPos < CD_RAW_READ_C2_294_SIZE; wC2ErrorPos++) {
		// Ricoh based drives (+97 read offset, like the Aopen CD-RW CRW5232)
		// use lsb points to 1st byte of main. 
		// But almost drive is msb points to 1st byte of main.
//		INT nBit = 0x01;
		INT nBit = 0x80;
		DWORD dwPos = dwC2ErrorPosPtr + wC2ErrorPos;
		for (INT n = 0; n < CHAR_BIT; n++) {
			// exist C2 error
			if (lpBuf[dwPos] & nBit) {
				// Big endian
				// +12/+13‚Ì‚¸‚ê (real < pos)
				pC2ErrorData[uiC2ErrorLBACnt].lpErrorBytePos[pC2ErrorData[uiC2ErrorLBACnt].uiErrorBytePosCnt] =
					(SHORT)(wC2ErrorPos * CHAR_BIT + n - sC2Offset);
				pC2ErrorData[uiC2ErrorLBACnt].uiErrorBytePosCnt++;
				if (pC2ErrorData[uiC2ErrorLBACnt].uiErrorBytePosCnt == 1) {
					bRet = RETURNED_C2_ERROR_EXIST;
					if (pDiscData->nCombinedOffset > 0) {
						//	Nonomura Byouin no Hitobito
						//	 Combined Offset(Byte)   1680, (Samples)   420
						//	-   Drive Offset(Byte)    120, (Samples)    30
						//	----------------------------------------------
						//	       CD Offset(Byte)   1560, (Samples)   390
						//	Need overread sector: 1

						// Policenauts
						//	 Combined Offset(Byte)   5136, (Samples)  1284
						//	-   Drive Offset(Byte)    120, (Samples)    30
						//	----------------------------------------------
						//	       CD Offset(Byte)   5016, (Samples)  1254
						//	Need overread sector: 3
						pC2ErrorData[uiC2ErrorLBACnt].cSlideSectorNum = (CHAR)(abs(pDiscData->nAdjustSectorNum));
					}
					else {
						// Anesan
						//	 Combined Offset(Byte)  -1932, (Samples)  -483
						//	-   Drive Offset(Byte)    120, (Samples)    30
						//	----------------------------------------------
						//	       CD Offset(Byte)  -2052, (Samples)  -513
						//	Need overread sector: -1

						// F1 Circus Special - Pole to Win
						//	 Combined Offset(Byte)  -2712, (Samples)  -678
						//	-   Drive Offset(Byte)    120, (Samples)    30
						//	----------------------------------------------
						//	       CD Offset(Byte)  -2832, (Samples)  -708
						//	Need overread sector: -2
						INT nSlideNum = pDiscData->nCombinedOffset / CD_RAW_SECTOR_SIZE;
						if (nSlideNum == 0) {
							pC2ErrorData[uiC2ErrorLBACnt].cSlideSectorNum = -1;
						}
						else {
							pC2ErrorData[uiC2ErrorLBACnt].cSlideSectorNum = (CHAR)(nSlideNum);
						}
					}
				}
			}
//			nBit <<= 1;
			nBit >>= 1;
		}
	}
	return bRet;
}
