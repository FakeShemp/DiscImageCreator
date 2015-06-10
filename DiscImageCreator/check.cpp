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
#include "outputMMCLogforCD.h"
#include "set.h"
#include "_external/crc16ccitt.h"

BOOL IsValid3doDataHeader(
	LPBYTE lpBuf
	)
{
	// judge it from the 1st sector(=LBA 0).
	BOOL bRet = TRUE;
	if (IsValidDataHeader(lpBuf)) {
		// ZZZZZZ
		BYTE a3doHeader1[] = {
			0x01, 0x5a, 0x5a, 0x5a, 
			0x5a, 0x5a, 0x01, 0x00
		};
		UINT i = 16;
		for (UINT c = 0; c < sizeof(a3doHeader1); i++, c++) {
			if (lpBuf[i] != a3doHeader1[c]) {
				bRet = FALSE;
				break;
			}
		}
		if (bRet) {
			BYTE reserved1[32] = { 0 };
			for (UINT c = 0; c < sizeof(reserved1); i++, c++) {
				if (lpBuf[i] != reserved1[c]) {
					bRet = FALSE;
					break;
				}
			}
		}
		if (bRet) {
			// CD-ROM
			BYTE a3doHeader2[] = {
				0x43, 0x44, 0x2d, 0x52,
				0x4f, 0x4d, 0x00, 0x00
			};
			for (UINT c = 0; c < sizeof(a3doHeader2); i++, c++) {
				if (lpBuf[i] != a3doHeader2[c]) {
					bRet = FALSE;
					break;
				}
			}
		}
		if (bRet) {
			BYTE reserved2[24] = { 0 };
			for (UINT c = 0; c < sizeof(reserved2); i++, c++) {
				if (lpBuf[i] != reserved2[c]) {
					bRet = FALSE;
					break;
				}
			}
		}
		if (bRet) {
			i = 124;
			BYTE reserved3[24] = { 0 };
			for (UINT c = 0; c < sizeof(reserved3); i++, c++) {
				if (lpBuf[i] != reserved3[c]) {
					bRet = FALSE;
					break;
				}
			}
		}
		if (bRet) {
			for (; i < 2060; i+=8) {				
				if (strncmp((CHAR*)&lpBuf[i], "duckiama", 8)) {
					bRet = FALSE;
					break;
				}
			}
		}
	}
	return bRet;
}

BOOL IsValidDataHeader(
	LPBYTE lpBuf
	)
{
	BYTE aHeader[] = {
		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00
	};
	BOOL bRet = TRUE;
	for (INT c = 0; c < sizeof(aHeader); c++) {
		if (lpBuf[c] != aHeader[c]) {
			bRet = FALSE;
			break;
		}
	}
	return bRet;
}

BOOL IsValidMacDataHeader(
	LPBYTE lpBuf
	)
{
	// judge it from the 2nd sector(=LBA 1).
	BOOL bRet = TRUE;
	if (lpBuf[0] != 0x42 || lpBuf[1] != 0x44) {
		bRet = FALSE;
	}
	return bRet;
}

BOOL IsValidPlextorDrive(
	PDEVICE pDevice
	)
{
	if (!strncmp(pDevice->szVendorId, "PLEXTOR ", DRIVE_VENDER_ID_SIZE)) {
		// PX-504A, PX-740A, PX-750A, PX-751A doesn't support...
		// Sense data, Key:Asc:Ascq: 05:20:00(ILLEGAL_REQUEST. INVALID COMMAND OPERATION CODE)
		if (!strncmp(pDevice->szProductId, "DVDR   PX-760A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlexType = PLEX_DRIVE_TYPE::PX760;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-755A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlexType = PLEX_DRIVE_TYPE::PX755;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-716A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlexType = PLEX_DRIVE_TYPE::PX716;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-716AL ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlexType = PLEX_DRIVE_TYPE::PX716AL;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-712A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlexType = PLEX_DRIVE_TYPE::PX712;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-708A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlexType = PLEX_DRIVE_TYPE::PX708;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-708A2 ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlexType = PLEX_DRIVE_TYPE::PX708A2;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-320A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlexType = PLEX_DRIVE_TYPE::PX320;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PREMIUM2 ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlexType = PLEX_DRIVE_TYPE::Premium2;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PREMIUM  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlexType = PLEX_DRIVE_TYPE::PXW5232;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W5224A", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlexType = PLEX_DRIVE_TYPE::PXW5224;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W4824A", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlexType = PLEX_DRIVE_TYPE::PXW4824;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W4012A", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlexType = PLEX_DRIVE_TYPE::PXW4012;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W2410A", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlexType = PLEX_DRIVE_TYPE::PXW2410;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W1610A", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlexType = PLEX_DRIVE_TYPE::PXW1610;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W1210A", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlexType = PLEX_DRIVE_TYPE::PXW1210;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W8432T", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlexType = PLEX_DRIVE_TYPE::PXW8432;
		}
		else {
			pDevice->byPlexType = PLEX_DRIVE_TYPE::No;
		}
	}
	return TRUE;
}

BOOL IsValidPregapSector(
	PDISC pDisc,
	PSUB_Q pNextSubQ,
	PSUB_Q pSubQ,
	PSUB_Q pPrevSubQ,
	INT nLBA
	)
{
	BOOL bRet = FALSE;
	if ((nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pPrevSubQ->byTrackNum] - 225 ||
		nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pPrevSubQ->byTrackNum] - 150 ||
		nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pPrevSubQ->byTrackNum] - 149) &&
		(pSubQ->byCtl & AUDIO_DATA_TRACK) == 0 &&
		(pNextSubQ->byCtl & AUDIO_DATA_TRACK) == 0 &&
		pNextSubQ->byIndex == 0
		) {
		bRet = TRUE;
	}
	return bRet;
}

BOOL IsValidSubQCtl(
	PSUB_Q pPrevPrevSubQ,
	PSUB_Q pPrevSubQ,
	PSUB_Q pSubQ,
	BYTE byEndCtl
	)
{
	BOOL bRet = TRUE;
	switch (pSubQ->byCtl) {
	case 0:
		break;
	case AUDIO_WITH_PREEMPHASIS:
		break;
	case DIGITAL_COPY_PERMITTED:
		break;
	case DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
		break;
	case AUDIO_DATA_TRACK:
		break;
	case AUDIO_DATA_TRACK | DIGITAL_COPY_PERMITTED:
		break;
	case TWO_FOUR_CHANNEL_AUDIO:
		break;
	case TWO_FOUR_CHANNEL_AUDIO | AUDIO_WITH_PREEMPHASIS:
		break;
	case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED:
		break;
	case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
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
				}
				else if (pPrevSubQ->byTrackNum == pSubQ->byTrackNum &&
					pPrevSubQ->byIndex == pSubQ->byIndex) {
					if (pPrevSubQ->nRelativeTime + 1 == pSubQ->nRelativeTime) {
						bRet = FALSE;
					}
				}
			}
			// EVE - burst error (Disc 3) (Terror Disc)
			// LBA[188021, 0x2DE75], Data, Copy NG, Track[01], Idx[01], RelTime[41:46:71], AbsTime[41:48:71] RtoW[Zero, Zero, Zero, Zero]
			// LBA[188022, 0x2DE76], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number  [0000000000000], AbsTime[     :72] RtoW[Zero, Zero, Zero, Zero]
			// LBA[188023, 0x2DE77], Data, Copy NG, Track[01], Idx[01], RelTime[41:46:73], AbsTime[41:48:73] RtoW[Zero, Zero, Zero, Zero]
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

BOOL IsValidSubQTrack(
	PDISC pDisc,
	PSUB_Q pPrevPrevSubQ,
	PSUB_Q pPrevSubQ,
	PSUB_Q pSubQ,
	PSUB_Q pNextSubQ,
	INT nLBA,
	LPBOOL bPrevTrackNum
	)
{
	if (nLBA > pDisc->SCSI.nAllLength - 10 && pSubQ->byTrackNum == 110) {
		// skip leadout
		return TRUE;
	}
	if (pDisc->SCSI.toc.LastTrack < pSubQ->byTrackNum) {
		return FALSE;
	}
	else if (pNextSubQ->byAdr == ADR_NO_MODE_INFORMATION &&
		pNextSubQ->byTrackNum < pSubQ->byTrackNum) {
		return FALSE;
	}
	BOOL bRet = TRUE;
	if (pPrevSubQ->byTrackNum != pSubQ->byTrackNum) {
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
				else if (pSubQ->byTrackNum != pNextSubQ->byTrackNum) {
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
BOOL IsValidSubQIdx(
	PSUB_Q pPrevPrevSubQ,
	PSUB_Q pPrevSubQ,
	PSUB_Q pSubQ,
	PSUB_Q pNextSubQ,
	LPBOOL bPrevIndex,
	LPBOOL bPrevPrevIndex
	)
{
	BOOL bRet = TRUE;
	if (pSubQ->byTrackNum != 110) {
		if (pPrevSubQ->byIndex != pSubQ->byIndex) {
			if (pPrevSubQ->byIndex + 1 < pSubQ->byIndex) {
				bRet = FALSE;
			}
			else if (MAXIMUM_NUMBER_INDEXES < pSubQ->byIndex) {
				bRet = FALSE;
			}
			else if (pSubQ->byIndex - 1 == pNextSubQ->byIndex && 
				pNextSubQ->byAdr == ADR_ENCODES_CURRENT_POSITION) {
				// Davis Cup Tennis, The (Japan)
				// LBA[004110, 0x0100e], Audio, 2ch, Copy NG, Pre-emphasis No, Track[03], Idx[00], RelTime[00:00:01], AbsTime[00:56:60], RtoW[Zero, Zero, Zero, Zero]
				// LBA[004111, 0x0100f], Audio, 2ch, Copy NG, Pre-emphasis No, Track[03], Idx[00], RelTime[00:00:00], AbsTime[00:56:61], RtoW[Zero, Zero, Zero, Zero]
				// LBA[004112, 0x01010], Audio, 2ch, Copy NG, Pre-emphasis No, Track[03], Idx[01], RelTime[00:00:00], AbsTime[00:56:62], RtoW[Zero, Zero, Zero, Zero]
				// LBA[004113, 0x01011], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number  [0000000000000], AbsTime[     :63], RtoW[Zero, Zero, Zero, Zero]
				// LBA[004114, 0x01012], Audio, 2ch, Copy NG, Pre-emphasis No, Track[03], Idx[01], RelTime[00:00:02], AbsTime[00:56:64], RtoW[Zero, Zero, Zero, Zero]
				// 1552 Tenka Tairan
				// LBA[126959, 0x1efef], Audio, 2ch, Copy NG, Pre-emphasis No, Track[14], Idx[01], RelTime[01:50:13], AbsTime[28:14:59], RtoW[Zero, Zero, Zero, Zero]
				// LBA[126960, 0x1eff0], Audio, 2ch, Copy NG, Pre-emphasis No, Track[14], Idx[02], RelTime[01:50:14], AbsTime[28:14:60], RtoW[Zero, Zero, Zero, Zero]
				// LBA[126961, 0x1eff1], Audio, 2ch, Copy NG, Pre-emphasis No, Track[14], Idx[01], RelTime[01:50:15], AbsTime[28:14:61], RtoW[Zero, Zero, Zero, Zero]
				bRet = FALSE;
			}
			else {
				bRet = FALSE;
				if (pPrevSubQ->byIndex == 0 &&
					pSubQ->byIndex == 1 &&
					pSubQ->nRelativeTime == 0) {
					bRet = TRUE;
				}
				else if ((pSubQ->byIndex == 0 || pSubQ->byIndex == 1) &&
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
	}
	return bRet;
}

BOOL IsValidSubQMsf(
	LPBYTE lpSubcode,
	BYTE m,
	BYTE s,
	BYTE f
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

BOOL IsValidSubQRelTime(
	PSUB_Q pPrevSubQ,
	PSUB_Q pSubQ,
	LPBYTE lpSubcode,
	INT nLBA
	)
{
	if (pPrevSubQ->byTrackNum == 110 || pSubQ->byTrackNum == 110) {
		return TRUE;
	}
	BOOL bRet = IsValidSubQMsf(lpSubcode, 15, 16, 17);
	if (!bRet) {
		return bRet;
	}
	INT tmpLBA = MSFtoLBA(BcdToDec(lpSubcode[15]), 
		BcdToDec(lpSubcode[16]), BcdToDec(lpSubcode[17]));
	if (tmpLBA != 0) {
		if (pSubQ->byIndex > 0) {
			if (pPrevSubQ->nRelativeTime != 0 && pSubQ->nRelativeTime != 0 &&
				pPrevSubQ->nRelativeTime + 1 != pSubQ->nRelativeTime) {
				bRet = FALSE;
			}
			else if (pPrevSubQ->byIndex > 0 &&
				pPrevSubQ->nRelativeTime + 1 != pSubQ->nRelativeTime) {
				// LBA[015496, 0x03c88], Audio, 2ch, Copy NG, Pre-emphasis No, Track[09], Idx[01], RelTime[00:13:42], AbsTime[03:28:46], RtoW[Zero, Zero, Zero, Zero]
				// LBA[015497, 0x03c89], Audio, 2ch, Copy NG, Pre-emphasis No, Track[10], Idx[01], RelTime[00:00:00], AbsTime[03:28:47], RtoW[Zero, Zero, Zero, Zero]
				// LBA[015498, 0x03c8a], Audio, 2ch, Copy NG, Pre-emphasis No, Track[10], Idx[01], RelTime[08:00:01], AbsTime[03:28:48], RtoW[Zero, Zero, Zero, Zero]
				bRet = FALSE;
			}
		}
		else if (pSubQ->byIndex == 0) {
			// SagaFrontier Original Sound Track (Disc 3)
			// LBA[009948, 0x026DC], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[00], RelTime[00:00:01], AbsTime[02:14:48] RtoW[Zero, Zero, Zero, Zero]
			// LBA[009949, 0x026DD], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[00], RelTime[00:00:00], AbsTime[02:14:49] RtoW[Zero, Zero, Zero, Zero]
			// LBA[009950, 0x026DE], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RelTime[00:00:00], AbsTime[02:14:50] RtoW[Zero, Zero, Zero, Zero]
			// LBA[009951, 0x026DF], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RelTime[00:00:01], AbsTime[02:14:51] RtoW[Zero, Zero, Zero, Zero]
			// Now on Never (Nick Carter) (ZJCI-10118)
			// LBA[000598, 0x00256], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[00], RelTime[00:00:02], AbsTime[00:09:73, RtoW[Zero, Zero, Zero, Zero]
			// LBA[000599, 0x00257], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[00], RelTime[00:00:01], AbsTime[00:09:74] RtoW[Zero, Zero, Zero, Zero]
			// LBA[000600, 0x00258], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RelTime[00:00:00], AbsTime[00:10:00] RtoW[Zero, Zero, Zero, Zero]
			// LBA[000601, 0x00259], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RelTime[00:00:01], AbsTime[00:10:01] RtoW[Zero, Zero, Zero, Zero]
			if (pPrevSubQ->byTrackNum == pSubQ->byTrackNum &&
				pPrevSubQ->nRelativeTime != pSubQ->nRelativeTime + 1) {
				bRet = FALSE;
			}
		}
	}
	else if (tmpLBA == 0) {
		// ???
		if (nLBA != 0 && pSubQ->byIndex != 0 &&
			pPrevSubQ->byTrackNum == pSubQ->byTrackNum &&
			pPrevSubQ->byIndex == pSubQ->byIndex) {
			if (pPrevSubQ->nRelativeTime + 1 != pSubQ->nRelativeTime ||
				pPrevSubQ->nRelativeTime != pSubQ->nRelativeTime + 1) {
				bRet = FALSE;
			}
		}
	}
	return bRet;
}

BOOL IsValidSubQAbsTimeFrame(
	LPBYTE lpSubcode,
	INT nLBA
	)
{
	BYTE byFrame = 0;
	BYTE bySecond = 0;
	BYTE byMinute = 0;
	LBAtoMSF(nLBA + 150, &byMinute, &bySecond, &byFrame);
	if (BcdToDec(lpSubcode[21]) != byFrame) {
		return FALSE;
	}
	return TRUE;
}

BOOL IsValidSubQAbsTime(
	BOOL bRipPregap,
	PSUB_Q pPrevSubQ,
	PSUB_Q pSubQ,
	LPBYTE lpSubcode,
	INT nLBA
	)
{
	BOOL bRet = IsValidSubQMsf(lpSubcode, 19, 20, 21);
	if (bRet) {
		INT tmpLBA = MSFtoLBA(BcdToDec(lpSubcode[19]), 
			BcdToDec(lpSubcode[20]), BcdToDec(lpSubcode[21])) - 150;
		if (bRipPregap) {
			if (pPrevSubQ->nAbsoluteTime + 1 != pSubQ->nAbsoluteTime) {
				bRet = FALSE;
			}
		}
		else if (nLBA != tmpLBA ||
			pPrevSubQ->nAbsoluteTime + 1 != pSubQ->nAbsoluteTime) {
			bRet = FALSE;
		}
	}
	return bRet;
}

// reference
// https://isrc.jmd.ne.jp/about/pattern.html
// https://isrc.jmd.ne.jp/about/error.html
BOOL IsValidSubQIsrc(
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

BOOL IsValidSubQMcn(
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

BOOL CheckMainChannel(
	PDISC pDisc,
	LPBYTE lpBuf,
	PSUB_Q pSubQ,
	PSUB_Q pPrevSubQ,
	PSUB_Q pPrevPrevSubQ,
	BYTE CurrentTrackNum,
	INT nLBA
	)
{
	INT nOfs = pDisc->MAIN.nCombinedOffset;
	BYTE ctl = 0;
	INT nAdd = 0;

	if (0 <= nOfs && nOfs < 2352) {
		ctl = pSubQ->byCtl;
	}
	else if (2352 <= nOfs && nOfs < 4704) {
		ctl = pPrevSubQ->byCtl;
		nAdd--;
	}
	else if (4704 <= nOfs && nOfs < 7056) {
		ctl = pPrevPrevSubQ->byCtl;
		nAdd -= 2;
	}

	if ((ctl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
		if (!IsValidDataHeader(lpBuf + pDisc->MAIN.uiMainDataSlideSize)) {
			OutputInfoLogA(
				"LBA[%06d, %#07x], Track[%02u]: This sector is data, but it doesn't exist a header\n",
				nLBA + nAdd, nLBA + nAdd, CurrentTrackNum);
		}
	}
	return TRUE;
}

BOOL CheckAndFixSubP(
	LPBYTE lpSubcode,
	BYTE CurrentTrackNum,
	INT nLBA
	)
{
	BOOL bFF = FALSE;
	BOOL b00 = FALSE;
	for (INT i = 0; i < 12; i++) {
		if (lpSubcode[i] == 0xff) {
			bFF = TRUE;
			break;
		}
		else if (lpSubcode[i] == 0x00) {
			b00 = TRUE;
			break;
		}
	}
	for (INT i = 0; i < 12; i++) {
		if (bFF && lpSubcode[i] != 0xff) {
			OutputErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: Sub[%02d]:[%#04x] -> [0xff]\n",
				nLBA, nLBA, CurrentTrackNum, i, lpSubcode[i]);
			lpSubcode[i] = 0xff;
		}
		else if (b00 && lpSubcode[i] != 0x00) {
			OutputErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: Sub[%02d]:[%#04x] -> [0x00]\n",
				nLBA, nLBA, CurrentTrackNum, i, lpSubcode[i]);
			lpSubcode[i] = 0x00;
		}
	}
	return TRUE;
}

BOOL CheckLibCryptSector(
	BOOL bLibCrypt,
	INT nLBA
	)
{
	BOOL bRet = FALSE;
	if (((13500 <= nLBA && nLBA < 18000) || (40500 <= nLBA && nLBA < 45000)) && bLibCrypt) {
		bRet = TRUE;
	}
	return bRet;
}

BOOL CheckAndFixSubQMcn(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpSubcode,
	PSUB_Q pNextSubQ,
	PSUB_Q pSubQ,
	PSUB_Q pPrevSubQ,
	PSUB_Q pPrevPrevSubQ,
	LPBYTE lpCurrentTrackNum,
	LPBOOL lpCatalog,
	INT nLBA
	)
{
	BOOL bRet = TRUE;
	CHAR szCatalog[META_CATALOG_SIZE] = { 0 };
	BOOL bMCN = IsValidSubQMcn(lpSubcode);
	SetMCNToString(pDisc, lpSubcode, szCatalog, FALSE);
	// only once
	if (pPrevSubQ->byTrackNum == pDisc->SCSI.toc.FirstTrack) {
		*lpCatalog = bMCN;
	}
	if (strncmp(pDisc->SUB.szCatalog, szCatalog, META_CATALOG_SIZE) || !bMCN) {
		OutputErrorLogA("LBA[%06d, %#07x], Track[%02u]: MCN[%13s], Sub[19]Lo:%x, Sub[20]:%02x",
			nLBA, nLBA, *lpCurrentTrackNum, szCatalog, lpSubcode[19] & 0x0f, lpSubcode[20]);
		BOOL bFake = FALSE;
		BOOL bTrack = IsValidSubQTrack(pDisc, pPrevPrevSubQ, pPrevSubQ, pSubQ, pNextSubQ, nLBA, &bFake);
		BOOL bIdx = IsValidSubQIdx(pPrevPrevSubQ, pPrevSubQ, pSubQ, pNextSubQ, &bFake, &bFake);
		BOOL bAbs = IsValidSubQAbsTime(bFake, pPrevSubQ, pSubQ, lpSubcode, nLBA);
		if (*lpCatalog && (!bTrack && !bIdx && !bAbs)) {
			OutputErrorLogA(" -> Valid adr, fixes to[%13s], Sub[19]Lo:0, Sub[20]:0x00\n",
				pDisc->SUB.szCatalog);
#ifdef _DEBUG
			OutputErrorLogA("%02x %02x %02x %02x %02x %02x %02x %02x\n",
				lpSubcode[13], lpSubcode[14], lpSubcode[15], lpSubcode[16],
				lpSubcode[17], lpSubcode[18], lpSubcode[19], lpSubcode[20]);
#endif
			for (INT i = 13, j = 0; i < 19; i++, j += 2) {
				lpSubcode[i] = (BYTE)(pDisc->SUB.szCatalog[j] - 0x30);
				lpSubcode[i] <<= 4;
				lpSubcode[i] |= (BYTE)(pDisc->SUB.szCatalog[j + 1] - 0x30);
			}
			lpSubcode[19] = (BYTE)(pDisc->SUB.szCatalog[12] - 0x30);
			lpSubcode[19] <<= 4;
			lpSubcode[20] = 0;
#ifdef _DEBUG
			OutputErrorLogA("%02x %02x %02x %02x %02x %02x %02x %02x\n",
				lpSubcode[13], lpSubcode[14], lpSubcode[15], lpSubcode[16],
				lpSubcode[17], lpSubcode[18], lpSubcode[19], lpSubcode[20]);
#endif
#if 0
			// because tracknum, index... doesn't exist
			pSubQ->byTrackNum = pPrevSubQ->byTrackNum;
			pSubQ->byIndex = pPrevSubQ->byIndex;
			if (pSubQ->byIndex == 0) {
				pSubQ->nRelativeTime = pPrevSubQ->nRelativeTime - 1;
			}
			else if (pSubQ->byIndex > 0) {
				pSubQ->nRelativeTime = pPrevSubQ->nRelativeTime + 1;
			}
#endif
		}
		else {
			OutputErrorLogA(" -> Invalid adr\n");
			bRet = FALSE;
		}
	}

	if(bRet) {
		if (pSubQ->byAdr != ADR_ENCODES_MEDIA_CATALOG) {
			pSubQ->byAdr = ADR_ENCODES_MEDIA_CATALOG;
			lpSubcode[12] = (BYTE)(pSubQ->byCtl << 4 | pSubQ->byAdr);
		}
		// Subchannel pattern on MCN Sector
		// pattern 1: pregap sector.
		if (IsValidPregapSector(pDisc, pNextSubQ, pSubQ, pPrevSubQ, nLBA)) {
			BOOL bValidPre = FALSE;
			// pattern 1-1: prev sector is audio.
			if ((pPrevSubQ->byCtl & AUDIO_DATA_TRACK) == 0) {
				// pattern 1-1-1: present sector is audio.
				if ((pSubQ->byCtl & AUDIO_DATA_TRACK) == 0) {
					if (pPrevSubQ->byTrackNum > 0 &&
						nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pPrevSubQ->byTrackNum] - 225) {
						// Atlas, The - Renaissance Voyager (Japan)
						// LBA[003364, 0x00d24], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RelTime[00:44:64], AbsTime[00:46:64], RtoW[Zero, Zero, Zero, Zero]
						// LBA[003365, 0x00d25], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number  [0000000000000], AbsTime[     :65], RtoW[Zero, Zero, Zero, Zero]
						// LBA[003366, 0x00d26], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[00], RelTime[00:02:73], AbsTime[00:46:66], RtoW[Zero, Zero, Zero, Zero]
						// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
						// LBA[261585, 0x3FDD1], Audio, 2ch, Copy NG, Pre-emphasis No, Track[95], Idx[01], RelTime[00:13:69], AbsTime[58:09:60], RtoW[Zero, Zero, Zero, Zero]
						// LBA[261586, 0x3FDD2], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number  [0000000000000], AbsTime[     :61], RtoW[Zero, Zero, Zero, Zero]
						// LBA[261587, 0x3FDD3], Audio, 2ch, Copy NG, Pre-emphasis No, Track[96], Idx[00], RelTime[00:02:73], AbsTime[58:09:62], RtoW[Zero, Zero, Zero, Zero]
						pSubQ->nRelativeTime = 224;
						bValidPre = TRUE;
					}
					else if (pPrevSubQ->byTrackNum > 0 &&
						nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pPrevSubQ->byTrackNum] - 150) {
						pSubQ->nRelativeTime = 149;
						bValidPre = TRUE;
					}
					else if (pPrevSubQ->byTrackNum > 0 &&
						nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pPrevSubQ->byTrackNum] - 149) {
						pSubQ->nRelativeTime = 148;
						bValidPre = TRUE;
					}
					if (bValidPre && pExtArg->bMCN) {
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
					if (pPrevSubQ->byTrackNum > 0 &&
						nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pPrevSubQ->byTrackNum] - 225) {
						pSubQ->nRelativeTime = 224;
						bValidPre = TRUE;
					}
					else if (pPrevSubQ->byTrackNum > 0 &&
						nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pPrevSubQ->byTrackNum] - 150) {
						pSubQ->nRelativeTime = 149;
						bValidPre = TRUE;
					}
					else if (pPrevSubQ->byTrackNum > 0 &&
						nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pPrevSubQ->byTrackNum] - 149) {
						pSubQ->nRelativeTime = 148;
						bValidPre = TRUE;
					}
					if (bValidPre && pExtArg->bMCN) {
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
					if (pPrevSubQ->byTrackNum > 0 &&
						nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pPrevSubQ->byTrackNum] - 225) {
						pSubQ->nRelativeTime = 224;
						bValidPre = TRUE;
					}
					else if (pPrevSubQ->byTrackNum > 0 &&
						nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pPrevSubQ->byTrackNum] - 150) {
						// Valis IV (Japan)
						// LBA[157830, 0x26886], Data, Copy NG, Track[44], Idx[01], RelTime[00:06:27], AbsTime[35:06:30] RtoW[Zero, Zero, Zero, Zero]
						// LBA[157831, 0x26887], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number  [0000000000000], AbsTime[     :31] RtoW[Zero, Zero, Zero, Zero]
						// LBA[157832, 0x26888], Audio, 2ch, Copy NG, Pre-emphasis No, Track[45], Idx[00], RelTime[00:01:73], AbsTime[35:06:32] RtoW[Zero, Zero, Zero, Zero]
						// Cosmic Fantasy 2
						// LBA[202749, 0x317FD], Data, Copy NG, Track[80], Idx[01], RelTime[00:06:63], AbsTime[45:05:24] RtoW[Zero, Zero, Zero, Zero]
						// LBA[202750, 0x317FE], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number  [0000000000000], AbsTime[     :25] RtoW[Zero, Zero, Zero, Zero]
						// LBA[202751, 0x317FF], Audio, 2ch, Copy NG, Pre-emphasis No, Track[81], Idx[00], RelTime[00:01:73], AbsTime[45:05:26] RtoW[Zero, Zero, Zero, Zero]
						pSubQ->nRelativeTime = 149;
						bValidPre = TRUE;
					}
					else if (pPrevSubQ->byTrackNum > 0 &&
						nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pPrevSubQ->byTrackNum] - 149) {
						pSubQ->nRelativeTime = 148;
						bValidPre = TRUE;
					}
					if (bValidPre && pExtArg->bMCN) {
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
					if (pPrevSubQ->byTrackNum > 0 &&
						nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pPrevSubQ->byTrackNum] - 225) {
						pSubQ->nRelativeTime = 224;
						bValidPre = TRUE;
					}
					else if (pPrevSubQ->byTrackNum > 0 &&
						nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pPrevSubQ->byTrackNum] - 150) {
						pSubQ->nRelativeTime = 149;
						bValidPre = TRUE;
					}
					else if (pPrevSubQ->byTrackNum > 0 &&
						nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pPrevSubQ->byTrackNum] - 149) {
						pSubQ->nRelativeTime = 148;
						bValidPre = TRUE;
					}
					if (bValidPre && pExtArg->bMCN) {
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
					if (pPrevSubQ->byTrackNum > 0 &&
						nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pPrevSubQ->byTrackNum]) {
						// Madou Monogatari I - Honoo no Sotsuenji (Japan)
						// LBA[183031, 0x2CAF7], Audio, 2ch, Copy NG, Pre-emphasis No, Track[21], Idx[01], RelTime[00:31:70], AbsTime[40:42:31] RtoW[Zero, Zero, Zero, Zero]
						// LBA[183032, 0x2CAF8], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number  [0000000000000], AbsTime[     :32] RtoW[Zero, Zero, Zero, Zero]
						// LBA[183033, 0x2CAF9], Audio, 2ch, Copy NG, Pre-emphasis No, Track[22], Idx[01], RelTime[00:00:01], AbsTime[40:42:33] RtoW[Zero, Zero, Zero, Zero]
						pSubQ->nRelativeTime = 0;
						// pattern 2-1-1-1: change track.
						pSubQ->byTrackNum = (BYTE)(pPrevSubQ->byTrackNum + 1);
						// pattern 2-1-1-1-2: not change index.
						pSubQ->byIndex = pPrevSubQ->byIndex;
					}
					else if (pPrevSubQ->byIndex == 0 && pPrevSubQ->nRelativeTime == 0) {
						// Psychic Detective Series Vol. 5 - Nightmare (Japan)
						// LBA[080999, 0x13C67], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[00], RelTime[00:00:00], AbsTime[18:01:74] RtoW[Zero, Zero, Zero, Zero]
						// LBA[081000, 0x13C68], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number  [3010911111863], AbsTime[     :00] RtoW[Zero, Zero, Zero, Zero]
						// LBA[081001, 0x13C69], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[01], RelTime[00:00:01], AbsTime[18:02:01] RtoW[Zero, Zero, Zero, Zero]
						pSubQ->nRelativeTime = 0;
						// pattern 2-1-1-2: not change track.
						pSubQ->byTrackNum = pPrevSubQ->byTrackNum;
						// pattern 2-1-1-2-1: change index.
						pSubQ->byIndex = (BYTE)(pPrevSubQ->byIndex + 1);
					}
					else if (pPrevSubQ->byIndex > 1 && pPrevSubQ->byIndex != pNextSubQ->byIndex) {
						// Super Schwarzschild 2 (Japan)
						// LBA[234845, 0x3955D], Audio, 2ch, Copy NG, Pre-emphasis No, Track[19], Idx[01], RelTime[01:50:09], AbsTime[52:13:20] RtoW[Zero, Zero, Zero, Zero]
						// LBA[234846, 0x3955E], Audio, 2ch, Copy NG, Pre-emphasis No, Track[19], Idx[02], RelTime[01:50:10], AbsTime[52:13:21] RtoW[Zero, Zero, Zero, Zero]
						// LBA[234847, 0x3955F], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number  [0000000000000], AbsTime[     :22] RtoW[Zero, Zero, Zero, Zero]
						// LBA[234848, 0x39560], Audio, 2ch, Copy NG, Pre-emphasis No, Track[19], Idx[01], RelTime[01:50:12], AbsTime[52:13:23] RtoW[Zero, Zero, Zero, Zero]
						// pattern 2-1-1-2: not change track.
						pSubQ->byTrackNum = pPrevSubQ->byTrackNum;
						pSubQ->byIndex = pPrevPrevSubQ->byIndex;
						pPrevSubQ->byIndex = pPrevPrevSubQ->byIndex;
					}
					else {
						if (pPrevSubQ->byIndex == 0) {
							// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
							// LBA[003413, 0x00D55], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[00], RelTime[00:02:26], AbsTime[00:47:38] RtoW[Zero, Zero, Zero, Zero]
							// LBA[003414, 0x00D56], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number  [0000000000000], AbsTime[     :39] RtoW[Zero, Zero, Zero, Zero]
							// LBA[003415, 0x00D57], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[00], RelTime[00:02:24], AbsTime[00:47:40] RtoW[Zero, Zero, Zero, Zero]
							pSubQ->nRelativeTime = pPrevSubQ->nRelativeTime - 1;
						}
						else if (pPrevSubQ->byIndex > 0) {
							// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
							// LBA[081541, 0x13E85], Audio, 2ch, Copy NG, Pre-emphasis No, Track[18], Idx[01], RelTime[00:12:57], AbsTime[18:09:16] RtoW[Zero, Zero, Zero, Zero]
							// LBA[081542, 0x13E86], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number  [0000000000000], AbsTime[     :17] RtoW[Zero, Zero, Zero, Zero]
							// LBA[081543, 0x13E87], Audio, 2ch, Copy NG, Pre-emphasis No, Track[18], Idx[01], RelTime[00:12:59], AbsTime[18:09:18] RtoW[Zero, Zero, Zero, Zero]
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
					if (pPrevSubQ->byTrackNum > 0 &&
						nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pPrevSubQ->byTrackNum]) {
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
					if (pPrevSubQ->byTrackNum > 0 &&
						nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pPrevSubQ->byTrackNum]) {
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
							// LBA[188021, 0x2DE75], Data, Copy NG, Track[01], Idx[01], RelTime[41:46:71], AbsTime[41:48:71] RtoW[Zero, Zero, Zero, Zero]
							// LBA[188022, 0x2DE76], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number  [0000000000000], AbsTime[     :72] RtoW[Zero, Zero, Zero, Zero]
							// LBA[188023, 0x2DE77], Data, Copy NG, Track[01], Idx[01], RelTime[41:46:73], AbsTime[41:48:73] RtoW[Zero, Zero, Zero, Zero]
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
					if (pPrevSubQ->byTrackNum > 0 &&
						nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pPrevSubQ->byTrackNum]) {
						pSubQ->nRelativeTime = 0;
						// pattern 2-2-2-1: change track.
						pSubQ->byTrackNum = (BYTE)(pPrevSubQ->byTrackNum + 1);
						// pattern 2-2-2-1-2: not change index.
						pSubQ->byIndex = pPrevSubQ->byIndex;
					}
					else if (pPrevSubQ->byIndex == 0 && pPrevSubQ->nRelativeTime == 0) {
						// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
						// LBA[142873, 0x22E19], Data, Copy NG, Track[37], Idx[00], RelTime[00:00:00], AbsTime[31:46:73] RtoW[Zero, Zero, Zero, Zero]
						// LBA[142874, 0x22E1A], Data, Copy NG, Media Catalog Number  [0000000000000], AbsTime[     :74] RtoW[Zero, Zero, Zero, Zero]
						// LBA[142875, 0x22E1B], Data, Copy NG, Track[37], Idx[01], RelTime[00:00:01], AbsTime[31:47:00] RtoW[Zero, Zero, Zero, Zero]
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
							// LBA[174261, 0x2A8B5], Data, Copy NG, Track[60], Idx[01], RelTime[00:06:19], AbsTime[38:45:36] RtoW[Zero, Zero, Zero, Zero]
							// LBA[174262, 0x2A8B6], Data, Copy NG, Media Catalog Number  [0000000000000], AbsTime[     :37] RtoW[Zero, Zero, Zero, Zero]
							// LBA[174263, 0x2A8B7], Data, Copy NG, Track[60], Idx[01], RelTime[00:06:21], AbsTime[38:45:38] RtoW[Zero, Zero, Zero, Zero]
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
	return bRet;
}

BOOL CheckAndFixSubQIsrc(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpSubcode,
	PSUB_Q pSubQ,
	PSUB_Q pPrevSubQ,
	LPBYTE lpCurrentTrackNum,
	INT nLBA
	)
{
	BOOL bRet = TRUE;
	CHAR szISRC[META_ISRC_SIZE] = { 0 };
	BOOL bISRC = IsValidSubQIsrc(lpSubcode);
	if (!bISRC && pExtArg->bISRC) {
		// force a invalid ISRC to valid ISRC
		bISRC = pExtArg->bISRC;
	}
	SetISRCToString(pDisc, lpSubcode, szISRC, *lpCurrentTrackNum, bISRC);
	pDisc->SUB.lpISRCList[*lpCurrentTrackNum - 1] = bISRC;

	if (strncmp(pDisc->SUB.pszISRC[*lpCurrentTrackNum - 1], szISRC, META_ISRC_SIZE) || !bISRC) {
		OutputErrorLogA("LBA[%06d, %#07x], Track[%02u]: ISRC[%s], Sub[20]Lo:%x",
			nLBA, nLBA, *lpCurrentTrackNum, szISRC, lpSubcode[20] & 0x0f);
		if (bISRC) {
			OutputErrorLogA(" -> Valid adr, fixes to[%12s], Sub[20]Lo:0\n",
				pDisc->SUB.pszISRC[*lpCurrentTrackNum - 1]);
#ifdef _DEBUG
			OutputErrorLogA("%02x %02x %02x %02x %02x %02x %02x %02x\n",
				lpSubcode[13], lpSubcode[14], lpSubcode[15], lpSubcode[16],
				lpSubcode[17], lpSubcode[18], lpSubcode[19], lpSubcode[20]);
#endif
			CHAR tmpISRC[META_ISRC_SIZE] = { 0 };
			strncpy(tmpISRC, pDisc->SUB.pszISRC[*lpCurrentTrackNum - 1], META_ISRC_SIZE);
			lpSubcode[13] = (BYTE)(((tmpISRC[0] - 0x30) << 2) | ((tmpISRC[1] - 0x30) >> 4));
			lpSubcode[14] = (BYTE)(((tmpISRC[1] - 0x30) << 4) | ((tmpISRC[2] - 0x30) >> 2));
			lpSubcode[15] = (BYTE)(((tmpISRC[2] - 0x30) << 6) | (tmpISRC[3] - 0x30));
			lpSubcode[16] = (BYTE)((tmpISRC[4] - 0x30) << 6);
			for (INT i = 17, j = 5; i < 20; i++, j += 2) {
				lpSubcode[i] = (BYTE)(tmpISRC[j] - 0x30);
				lpSubcode[i] <<= 4;
				lpSubcode[i] |= (BYTE)(tmpISRC[j + 1] - 0x30);
			}
			lpSubcode[20] = (BYTE)(tmpISRC[11] - 0x30);
			lpSubcode[20] <<= 4;
#ifdef _DEBUG
			OutputErrorLogA("%02x %02x %02x %02x %02x %02x %02x %02x\n",
				lpSubcode[13], lpSubcode[14], lpSubcode[15], lpSubcode[16],
				lpSubcode[17], lpSubcode[18], lpSubcode[19], lpSubcode[20]);
#endif
		}
		else {
			OutputErrorLogA(" -> Invalid adr\n");
			if (pDisc->SUB.lpISRCList[*lpCurrentTrackNum - 1]) {
				pDisc->SUB.lpISRCList[*lpCurrentTrackNum - 1] = FALSE;
			}
			bRet = FALSE;
		}
	}
	if (bRet) {
		// because tracknum, index... doesn't exist
		if (pSubQ->byAdr != ADR_ENCODES_ISRC) {
			pSubQ->byAdr = ADR_ENCODES_ISRC;
			lpSubcode[12] = (BYTE)(pSubQ->byCtl << 4 | pSubQ->byAdr);
		}
		pSubQ->byTrackNum = pPrevSubQ->byTrackNum;
		pSubQ->byIndex = pPrevSubQ->byIndex;
		if (pSubQ->byIndex == 0) {
			pSubQ->nRelativeTime = pPrevSubQ->nRelativeTime - 1;
		}
		else if (pSubQ->byIndex > 0) {
			pSubQ->nRelativeTime = pPrevSubQ->nRelativeTime + 1;
		}
		if (!pDisc->SUB.lpISRCList[*lpCurrentTrackNum - 1]) {
			pDisc->SUB.lpISRCList[*lpCurrentTrackNum - 1] = TRUE;
		}
	}
	return bRet;
}

BOOL CheckAndFixSubQ(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpSubcode,
	PSUB_Q pNextSubQ,
	PSUB_Q pSubQ,
	PSUB_Q pPrevSubQ,
	PSUB_Q pPrevPrevSubQ,
	LPBYTE lpCurrentTrackNum,
	LPBOOL lpCatalog,
	INT nLBA,
	BOOL bLibCrypt
	)
{
	if (pPrevPrevSubQ->byTrackNum == 110 || pPrevSubQ->byTrackNum == 110 || pSubQ->byTrackNum == 110) {
		// skip lead-out
		return TRUE;
	}

	BOOL bBadAdr = FALSE;
	if (pSubQ->byAdr == ADR_ENCODES_MEDIA_CATALOG) {
		// first check adr:2
		if (!CheckAndFixSubQMcn(pExtArg, pDisc, lpSubcode, pNextSubQ, pSubQ,
			pPrevSubQ, pPrevPrevSubQ, lpCurrentTrackNum, lpCatalog, nLBA)) {
			// Next check adr:3
			if (!CheckAndFixSubQIsrc(pExtArg, pDisc, lpSubcode, 
				pSubQ, pPrevSubQ, lpCurrentTrackNum, nLBA)) {
				bBadAdr = TRUE;
			}
		}
	}
	else if (pSubQ->byAdr == ADR_ENCODES_ISRC) {
		// first check adr:3
		if (!CheckAndFixSubQIsrc(pExtArg, pDisc, lpSubcode,
			pSubQ, pPrevSubQ, lpCurrentTrackNum, nLBA)) {
			// Next check adr:2
			if (!CheckAndFixSubQMcn(pExtArg, pDisc, lpSubcode, pNextSubQ, pSubQ,
				pPrevSubQ, pPrevPrevSubQ, lpCurrentTrackNum, lpCatalog, nLBA)) {
				bBadAdr = TRUE;
			}
		}
	}
	else if (pSubQ->byAdr == ADR_NO_MODE_INFORMATION ||
		pSubQ->byAdr > ADR_ENCODES_ISRC) {
		// first check adr:2
		if (!CheckAndFixSubQMcn(pExtArg, pDisc, lpSubcode, pNextSubQ, pSubQ,
			pPrevSubQ, pPrevPrevSubQ, lpCurrentTrackNum, lpCatalog, nLBA)) {
			// Next check adr:3
			if (!CheckAndFixSubQIsrc(pExtArg, pDisc, lpSubcode,
				pSubQ, pPrevSubQ, lpCurrentTrackNum, nLBA)) {
				bBadAdr = TRUE;
			}
		}
	}
	if (bBadAdr) {
		pSubQ->byAdr = ADR_ENCODES_CURRENT_POSITION;
		lpSubcode[12] = (BYTE)(pSubQ->byCtl << 4 | pSubQ->byAdr);
	}

	BYTE SubQcodeForLibCrypt[12] = { 0 };
	if (bLibCrypt) {
		OutputInfoLogA(
			"LBA[%06d, %#07x], Track[%02u]: Sub[12-23]: %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02x %02x\n",
			nLBA, nLBA, *lpCurrentTrackNum,
			BcdToDec(lpSubcode[12]), BcdToDec(lpSubcode[13]), BcdToDec(lpSubcode[14]), BcdToDec(lpSubcode[15]),
			BcdToDec(lpSubcode[16]), BcdToDec(lpSubcode[17]), BcdToDec(lpSubcode[18]), BcdToDec(lpSubcode[19]),
			BcdToDec(lpSubcode[20]), BcdToDec(lpSubcode[21]), BcdToDec(lpSubcode[22]), BcdToDec(lpSubcode[23]));
		memcpy(SubQcodeForLibCrypt, &lpSubcode[12], sizeof(SubQcodeForLibCrypt));
	}
	if (pSubQ->byAdr == ADR_ENCODES_CURRENT_POSITION || bBadAdr) {
		BOOL bPrevTrackNum = TRUE;
		if (!IsValidSubQTrack(pDisc, pPrevPrevSubQ, pPrevSubQ, pSubQ, pNextSubQ, nLBA, &bPrevTrackNum)) {
			OutputErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: TrackNum[%02u] -> [%02u]\n",
				nLBA, nLBA, *lpCurrentTrackNum, pSubQ->byTrackNum, pPrevSubQ->byTrackNum);
			if (*lpCurrentTrackNum == pDisc->SCSI.toc.LastTrack) {
				pSubQ->byTrackNum = pDisc->SCSI.toc.LastTrack;
			}
			else if (pDisc->SCSI.lpFirstLBAListOnToc[*lpCurrentTrackNum] < nLBA) {
				pSubQ->byTrackNum = (BYTE)(pPrevSubQ->byTrackNum + 1);
			}
			else {
				// Bikkuriman Daijikai (Japan)
				// LBA[106402, 0x19FA2], Audio, 2ch, Copy NG, Pre-emphasis No, Track[70], Idx[01], RelTime[00:16:39], AbsTime[23:40:52] RtoW[Zero, Zero, Zero, Zero]
				// LBA[106403, 0x19FA3], Audio, 2ch, Copy NG, Pre-emphasis No, Track[79], Idx[01], RelTime[00:00:00], AbsTime[21:40:53] RtoW[Zero, Zero, Zero, Zero]
				// LBA[106404, 0x19FA4], Audio, 2ch, Copy NG, Pre-emphasis No, Track[71], Idx[01], RelTime[00:00:01], AbsTime[23:40:54] RtoW[Zero, Zero, Zero, Zero]
				// Kagami no Kuni no Legend (Japan)
				// LBA[004373, 0x01115], Data, Copy NG, Track[02], Idx[00], RelTime[00:00:01], AbsTime[01:00:23] RtoW[Zero, Zero, Zero, Zero]
				// LBA[004374, 0x01116], Data, Copy NG, Track[0a], Idx[00], RelTime[00:00:00], AbsTime[01:00:24] RtoW[Zero, Zero, Zero, Zero]
				// LBA[004375, 0x01117], Data, Copy NG, Track[02], Idx[01], RelTime[00:00:00], AbsTime[01:00:25] RtoW[Zero, Zero, Zero, Zero]
				pSubQ->byTrackNum = pPrevSubQ->byTrackNum;
				if (pPrevSubQ->byTrackNum < pDisc->SCSI.toc.LastTrack &&
					pSubQ->byIndex == 1 && pSubQ->nRelativeTime == 0) {
					pSubQ->byTrackNum += 1;
				}
			}
			lpSubcode[13] = DecToBcd(pSubQ->byTrackNum);
		}
		else if (!bPrevTrackNum) {
			OutputErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: PrevTrackNum[%02u] -> [%02u]\n",
				nLBA, nLBA, *lpCurrentTrackNum, pPrevSubQ->byTrackNum, pPrevPrevSubQ->byTrackNum);
			pDisc->SUB.lpFirstLBAListOnSub[pPrevSubQ->byTrackNum - 1][pPrevSubQ->byIndex] = -1;
			pDisc->SUB.lpFirstLBAListOnSubSync[pPrevSubQ->byTrackNum - 1][pPrevSubQ->byIndex] = -1;
			pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[pPrevSubQ->byTrackNum - 1] = -1;
			pPrevSubQ->byTrackNum = pPrevPrevSubQ->byTrackNum;
			pPrevSubQ->byIndex = pPrevPrevSubQ->byIndex;
			pPrevSubQ->nRelativeTime = pPrevPrevSubQ->nRelativeTime + 1;
		}
		BOOL bPrevIndex = TRUE;
		BOOL bPrevPrevIndex = TRUE;
		if (!IsValidSubQIdx(pPrevPrevSubQ, pPrevSubQ, pSubQ, pNextSubQ, &bPrevIndex, &bPrevPrevIndex)) {
			OutputErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: Idx[%02u] -> [%02u]\n",
				nLBA, nLBA, *lpCurrentTrackNum, pSubQ->byIndex, pPrevSubQ->byIndex);
			pSubQ->byTrackNum = pPrevSubQ->byTrackNum;
			pSubQ->byIndex = pPrevSubQ->byIndex;
			lpSubcode[14] = DecToBcd(pSubQ->byIndex);
		}
		else if (!bPrevIndex) {
			OutputErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: PrevIdx[%02u] -> [%02u]\n",
				nLBA - 1, nLBA - 1, *lpCurrentTrackNum, pPrevSubQ->byIndex, pPrevPrevSubQ->byIndex);
			pDisc->SUB.lpFirstLBAListOnSub[pPrevSubQ->byTrackNum - 1][pPrevSubQ->byIndex] = -1;
			pDisc->SUB.lpFirstLBAListOnSubSync[pPrevSubQ->byTrackNum - 1][pPrevSubQ->byIndex] = -1;
			pPrevSubQ->byTrackNum = pPrevPrevSubQ->byTrackNum;
			pPrevSubQ->byIndex = pPrevPrevSubQ->byIndex;
		}
		else if (!bPrevPrevIndex) {
			OutputErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: PrevPrevIdx[%02u] -> [%02u]\n",
				nLBA - 1, nLBA - 1, *lpCurrentTrackNum, pPrevPrevSubQ->byIndex, pPrevSubQ->byIndex);
			pDisc->SUB.lpFirstLBAListOnSub[pPrevPrevSubQ->byTrackNum - 1][pPrevPrevSubQ->byIndex] = -1;
			pDisc->SUB.lpFirstLBAListOnSubSync[pPrevPrevSubQ->byTrackNum - 1][pPrevPrevSubQ->byIndex] = -1;
			pPrevPrevSubQ->byTrackNum = pPrevSubQ->byTrackNum;
			pPrevPrevSubQ->byIndex = pPrevSubQ->byIndex;
		}

		if (!bLibCrypt && !IsValidSubQRelTime(pPrevSubQ, pSubQ, lpSubcode, nLBA)) {
			if (!(pPrevSubQ->byIndex == 0 && pSubQ->byIndex == 1) &&
				!(pPrevSubQ->byIndex >= 1 && pSubQ->byIndex == 0)) {
				BYTE byFrame = 0;
				BYTE bySecond = 0;
				BYTE byMinute = 0;
				INT tmpRel = 0;
				if (pSubQ->byIndex > 0) {
					tmpRel = pPrevSubQ->nRelativeTime + 1;
				}
				else if (pSubQ->byIndex == 0) {
					tmpRel = pPrevSubQ->nRelativeTime - 1;
				}
				LBAtoMSF(tmpRel, &byMinute, &bySecond, &byFrame);
				BYTE byPrevFrame = 0;
				BYTE byPrevSecond = 0;
				BYTE byPrevMinute = 0;
				LBAtoMSF(pPrevSubQ->nRelativeTime, &byPrevMinute, &byPrevSecond, &byPrevFrame);
				OutputErrorLogA(
					"LBA[%06d, %#07x], Track[%02u]: PrevRel[%d, %02u:%02u:%02u], Rel[%d, %02u:%02u:%02u] -> [%d, %02u:%02u:%02u]\n",
					nLBA, nLBA, *lpCurrentTrackNum, pPrevSubQ->nRelativeTime,
					byPrevMinute, byPrevSecond, byPrevFrame, pSubQ->nRelativeTime,
					BcdToDec(lpSubcode[15]), BcdToDec(lpSubcode[16]), BcdToDec(lpSubcode[17]),
					tmpRel, byMinute, bySecond, byFrame);
				BOOL b1 = FALSE;
#if 0
				BOOL b2 = FALSE;
#endif
				// A Bigger Bang (TOCP-66400)
				// LBA[310026, 0x4bb0a], Track[17], PrevRel[  8898], Rel[  8898], RelTime[01:58:48] -> [01:58:49]
				if (pPrevSubQ->nRelativeTime == pSubQ->nRelativeTime) {
					b1 = TRUE;
				}
				// A Bigger Bang (TOCP-66400)
				// LBA[332700, 0x5139c], Track[17], PrevRel[31571], Rel[149], RelTime[00:01:74] -> [07:00:72]
#if 0
				else if (pSubQ->nRelativeTime <= 149 &&
					pSubQ->nRelativeTime < pPrevSubQ->nRelativeTime) {
					b2 = TRUE;
					LBAtoMSF(pSubQ->nRelativeTime, &byMinute, &bySecond, &byFrame);
					//					pSubQ->byTrackNum = 18;
				}
				if (!b2) {
					pSubQ->nRelativeTime = tmpRel;
				}
#endif
				if (b1) {
					pSubQ->nRelativeTime--;
				}
				pSubQ->nRelativeTime = tmpRel;
				lpSubcode[15] = DecToBcd(byMinute);
				lpSubcode[16] = DecToBcd(bySecond);
				lpSubcode[17] = DecToBcd(byFrame);
			}
		}

		if (lpSubcode[18] != 0) {
			OutputErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: Sub[18]:[%#04x] -> [0x00]\n",
				nLBA, nLBA, *lpCurrentTrackNum, lpSubcode[18]);
			lpSubcode[18] = 0;
		}

		if (!bLibCrypt && !IsValidSubQAbsTime(pDisc->SUB.bIndex0InTrack1,
			pPrevSubQ, pSubQ, lpSubcode, nLBA)) {
			BYTE byFrame = 0;
			BYTE bySecond = 0;
			BYTE byMinute = 0;
			INT tmpAbs = nLBA + 150;
			LBAtoMSF(tmpAbs, &byMinute, &bySecond, &byFrame);
			BYTE byPrevFrame = 0;
			BYTE byPrevSecond = 0;
			BYTE byPrevMinute = 0;
			LBAtoMSF(pPrevSubQ->nAbsoluteTime, &byPrevMinute, &byPrevSecond, &byPrevFrame);
			OutputErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: PrevAbs[%d, %02u:%02u:%02u], Abs[%d, %02u:%02u:%02u] -> [%d, %02u:%02u:%02u]\n",
				nLBA, nLBA, *lpCurrentTrackNum, pPrevSubQ->nAbsoluteTime,
				byPrevMinute, byPrevSecond, byPrevFrame, pSubQ->nAbsoluteTime,
				BcdToDec(lpSubcode[19]), BcdToDec(lpSubcode[20]), BcdToDec(lpSubcode[21]),
				tmpAbs, byMinute, bySecond, byFrame);
			pSubQ->nAbsoluteTime = MSFtoLBA(byMinute, bySecond, byFrame) + 150;
			lpSubcode[19] = DecToBcd(byMinute);
			lpSubcode[20] = DecToBcd(bySecond);
			lpSubcode[21] = DecToBcd(byFrame);
		}
	}
	else if (pSubQ->byAdr == ADR_ENCODES_MEDIA_CATALOG ||
		pSubQ->byAdr == ADR_ENCODES_ISRC) {
		if (!IsValidSubQAbsTimeFrame(lpSubcode, nLBA)) {
			BYTE byFrame = 0;
			BYTE bySecond = 0;
			BYTE byMinute = 0;
			LBAtoMSF(nLBA + 150, &byMinute, &bySecond, &byFrame);
			BYTE byPrevFrame = 0;
			BYTE byPrevSecond = 0;
			BYTE byPrevMinute = 0;
			LBAtoMSF(pPrevSubQ->nAbsoluteTime, &byPrevMinute, &byPrevSecond, &byPrevFrame);
			OutputErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: PrevAbsFrame[%02u], AbsFrame[%02u] -> [%02u]\n",
				nLBA, nLBA, *lpCurrentTrackNum, byPrevFrame, BcdToDec(lpSubcode[21]), byFrame);
			pSubQ->nAbsoluteTime = MSFtoLBA(byMinute, bySecond, byFrame);
			lpSubcode[21] = DecToBcd(byFrame);
		}
	}
	if (!IsValidSubQCtl(pPrevPrevSubQ, pPrevSubQ, pSubQ, 
		pDisc->SUB.lpEndCtlList[pSubQ->byTrackNum - 1])) {
		OutputErrorLogA(
			"LBA[%06d, %#07x], Track[%02u]: Ctl[%u] -> [%u]\n",
			nLBA, nLBA, *lpCurrentTrackNum, pSubQ->byCtl, pPrevSubQ->byCtl);
		pSubQ->byCtl = pPrevSubQ->byCtl;
		lpSubcode[12] = (BYTE)(pSubQ->byCtl << 4 | pSubQ->byAdr);
	}
	WORD crc16 = (WORD)GetCrc16CCITT(10, &lpSubcode[12]);
	if (bLibCrypt) {
		// lpSubcode is fixed (= original crc)
		WORD xorCrc16 = (WORD)(crc16 ^ 0x8001);
		BYTE tmp3 = HIBYTE(xorCrc16);
		BYTE tmp4 = LOBYTE(xorCrc16);
		if (SubQcodeForLibCrypt[10] == tmp3 &&
			SubQcodeForLibCrypt[11] == tmp4) {
			OutputInfoLogA(
				"LBA[%06d, %#07x], Track[%02u]: LibCrypt exists, CRC-16 is original:[%04x] and XORed with 0x8001:[%04x]\n",
				nLBA, nLBA, *lpCurrentTrackNum, crc16, xorCrc16);
		}
		else {
			// lpSubcode isn't fixed (= recalc crc)
			WORD reCalcCrc16 = (WORD)GetCrc16CCITT(10, &SubQcodeForLibCrypt[0]);
			WORD reCalcXorCrc16 = (WORD)(reCalcCrc16 ^ 0x0080);
			BYTE tmp5 = HIBYTE(reCalcXorCrc16);
			BYTE tmp6 = LOBYTE(reCalcXorCrc16);
			if (SubQcodeForLibCrypt[10] == tmp5 &&
				SubQcodeForLibCrypt[11] == tmp6) {
				OutputInfoLogA(
					"LBA[%06d, %#07x], Track[%02u]: LibCrypt exists, CRC-16 is recalculated:[%04x] and XORed with 0x0080:[%04x]\n",
					nLBA, nLBA, *lpCurrentTrackNum, reCalcCrc16, reCalcXorCrc16);
			}
			else {
				OutputInfoLogA(
					"LBA[%06d, %#07x], Track[%02u]: LibCrypt doesn't exist\n",
					nLBA, nLBA, *lpCurrentTrackNum);
			}
		}
		lpSubcode[22] = SubQcodeForLibCrypt[10];
		lpSubcode[23] = SubQcodeForLibCrypt[11];
	}
	else {
		BYTE tmp1 = HIBYTE(crc16);
		BYTE tmp2 = LOBYTE(crc16);
		if (lpSubcode[22] != tmp1) {
			OutputErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: CrcHigh[%#04x] -> [%#04x]\n",
				nLBA, nLBA, *lpCurrentTrackNum, lpSubcode[22], tmp1);
			lpSubcode[22] = tmp1;
		}
		if (lpSubcode[23] != tmp2) {
			OutputErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: CrcLow[%#04x] -> [%#04x]\n",
				nLBA, nLBA, *lpCurrentTrackNum, lpSubcode[23], tmp2);
			lpSubcode[23] = tmp2;
		}
	}
	return TRUE;
}

BOOL CheckAndFixSubRtoW(
	PDISC pDisc,
	LPBYTE lpSubcode,
	BYTE CurrentTrackNum,
	INT nLBA
	)
{
	if (pDisc->SUB.lpRtoWList[CurrentTrackNum - 1] == SUB_RTOW_TYPE::Zero) {
		for (INT i = 24; i < CD_RAW_READ_SUBCODE_SIZE; i++) {
			if (lpSubcode[i] != 0) {
				OutputErrorLogA(
					"LBA[%06d, %#07x], Track[%02u]: Sub[%02d]:[%#04x] -> [0x00]\n",
					nLBA, nLBA, CurrentTrackNum, i, lpSubcode[i]);
				lpSubcode[i] = 0;
			}
		}
	}
	return TRUE;
}

BOOL CheckAndFixSubChannel(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpSubcode,
	PSUB_Q pNextSubQ,
	PSUB_Q pSubQ,
	PSUB_Q pPrevSubQ,
	PSUB_Q pPrevPrevSubQ,
	LPBYTE lpCurrentTrackNum,
	LPBOOL lpCatalog,
	INT nLBA,
	BOOL bLibCrypt
	)
{
	CheckAndFixSubP(lpSubcode, *lpCurrentTrackNum, nLBA);
	CheckAndFixSubQ(pExtArg, pDisc, lpSubcode, pNextSubQ, pSubQ, 
		pPrevSubQ, pPrevPrevSubQ, lpCurrentTrackNum, lpCatalog, nLBA, bLibCrypt);
	CheckAndFixSubRtoW(pDisc, lpSubcode, *lpCurrentTrackNum, nLBA);
	return TRUE;
}

BOOL CheckByteError(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	LPBYTE lpBuf,
	UINT i
	)
{
	BOOL bByteError = FALSE;
	for (INT j = 0; j < CD_RAW_SECTOR_SIZE; j++) {
		if (pC2ErrorPerSector[i].lpBufC2NoneSectorBackup[j] != lpBuf[j]) {
			bByteError = TRUE;
			break;
		}
	}
	return bByteError;
}

BOOL CheckC2Error(
	PC2_ERROR pC2Error,
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	PDEVICE pDevice,
	LPBYTE lpBuf,
	UINT uiC2ErrorLBACnt
	)
{
	BOOL bRet = RETURNED_C2_ERROR_NO_1ST;
	for (WORD wC2ErrorPos = 0; wC2ErrorPos < CD_RAW_READ_C2_294_SIZE; wC2ErrorPos++) {
		// Ricoh based drives (+97 read offset, like the Aopen CD-RW CRW5232)
		// use lsb points to 1st byte of main. 
		// But almost drive is msb points to 1st byte of main.
//		INT nBit = 0x01;
		INT nBit = 0x80;
		DWORD dwPos = pDevice->TRANSFER.dwBufC2Offset + wC2ErrorPos;
		for (INT n = 0; n < CHAR_BIT; n++) {
			// exist C2 error
			if (lpBuf[dwPos] & nBit) {
				// wC2ErrorPos * CHAR_BIT => position of byte
				// (position of byte) + n => position of bit
				pC2ErrorPerSector[uiC2ErrorLBACnt].lpErrorBytePos[pC2ErrorPerSector[uiC2ErrorLBACnt].uiErrorBytePosCnt] =
					(SHORT)(wC2ErrorPos * CHAR_BIT + n - pC2Error->sC2Offset);
				pC2ErrorPerSector[uiC2ErrorLBACnt].uiErrorBytePosCnt++;
				if (pC2ErrorPerSector[uiC2ErrorLBACnt].uiErrorBytePosCnt == 1) {
					bRet = RETURNED_C2_ERROR_EXIST;
				}
			}
//			nBit <<= 1;
			nBit >>= 1;
		}
	}
	return bRet;
}
