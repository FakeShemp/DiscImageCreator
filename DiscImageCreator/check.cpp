/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "check.h"
#include "convert.h"
#include "execScsiCmd.h"
#include "execScsiCmdforCD.h"
#include "output.h"
#include "outputScsiCmdLogforCD.h"
#include "set.h"
#include "_external/crc16ccitt.h"

// These global variable is declared at DiscImageCreator.cpp
extern BYTE g_aSyncHeader[SYNC_SIZE];
// This static variable is set if function is error
static LONG s_lineNum;

BOOL IsValidMainDataHeader(
	LPBYTE lpBuf
	)
{
	BOOL bRet = TRUE;
	for (INT c = 0; c < sizeof(g_aSyncHeader); c++) {
		if (lpBuf[c] != g_aSyncHeader[c]) {
			bRet = FALSE;
			break;
		}
	}
	return bRet;
}

BOOL IsValid3doDataHeader(
	LPBYTE lpBuf
	)
{
	// judge it from the 1st sector(=LBA 0).
	BOOL bRet = TRUE;
	CONST BYTE a3doHeader[] = {
		0x01, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x01, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x43, 0x44, 0x2d, 0x52, 0x4f, 0x4d, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	INT i = 0;
	for (INT c = 0; c < sizeof(a3doHeader); i++, c++) {
		if (lpBuf[i] != a3doHeader[c]) {
			bRet = FALSE;
			break;
		}
	}
	if (bRet) {
		for (i = 132; i < 2048; i += 8) {
			if (strncmp((CHAR*)&lpBuf[i], "duck", 4) &&
				strncmp((CHAR*)&lpBuf[i + 4], "iama", 4)) {
				bRet = FALSE;
				break;
			}
		}
	}
	return bRet;
}

// http://d.hatena.ne.jp/zariganitosh/20130501/hfs_plus_struct
// http://www.opensource.apple.com/source/xnu/xnu-2050.18.24/bsd/hfs/hfs_format.h	
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

BOOL IsValidPceSector(
	LPBYTE lpBuf
	)
{
	BOOL bRet = TRUE;
	CONST BYTE warningStr[] = {
		0x82, 0xb1, 0x82, 0xcc, 0x83, 0x76, 0x83, 0x8d,
		0x83, 0x4f, 0x83, 0x89, 0x83, 0x80, 0x82, 0xcc,
		0x92, 0x98, 0x8d, 0xec, 0x8c, 0xa0, 0x82, 0xcd,
		0x8a, 0x94, 0x8e, 0xae, 0x89, 0xef, 0x8e, 0xd0,
		0x00, 0x83, 0x6e, 0x83, 0x68, 0x83, 0x5c, 0x83,
		0x93, 0x82, 0xaa, 0x8f, 0x8a, 0x97, 0x4c, 0x82,
		0xb5, 0x82, 0xc4, 0x82, 0xa8, 0x82, 0xe8, 0x82
	};
	for (INT i = 0; i < sizeof(warningStr); i++) {
		if (lpBuf[i] != warningStr[i]) {
			bRet = FALSE;
			break;
		}
	}
	return bRet;
}

BOOL IsValidPcfxSector(
	LPBYTE lpBuf
	)
{
	BOOL bRet = TRUE;
	if (strncmp((CHAR*)&lpBuf[0], "PC-FX:Hu_CD-ROM", 15)) {
		bRet = FALSE;
	}
	return bRet;
}

BOOL IsValidPlextorDrive(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
	)
{
	if (!strncmp(pDevice->szVendorId, "PLEXTOR ", DRIVE_VENDER_ID_SIZE)) {
		if (!strncmp(pDevice->szProductId, "DVDR   PX-760A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PX760A;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-755A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PX755A;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-716AL ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PX716AL;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-716A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PX716A;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-714A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PX714A;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-712A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PX712A;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-708A2 ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PX708A2;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-708A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PX708A;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-704A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PX704A;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-320A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PX320A;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PREMIUM2 ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PREMIUM2;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PREMIUM  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PREMIUM;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W5224A", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PXW5224A;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W4824A", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PXW4824A;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W4012A", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PXW4012A;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W4012S", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PXW4012S;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W2410A", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PXW2410A;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-S88T  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PXS88T;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W1610A", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PXW1610A;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W1210A", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PXW1210A;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W1210S", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PXW1210S;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W124TS", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PXW124TS;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W8432T", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PXW8432T;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W8220T", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PXW8220T;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W4220T", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PXW4220T;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-R820T ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PXR820T;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-R412C ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PXR412C;
		}
		else if (!strncmp(pDevice->szProductId, "CD-ROM PX-40TS  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PX40TS;
		}
		else if (!strncmp(pDevice->szProductId, "CD-ROM PX-40TSUW", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PX40TSUW;
		}
		else if (!strncmp(pDevice->szProductId, "CD-ROM PX-40TW  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PX40TW;
		}
		else if (!strncmp(pDevice->szProductId, "CD-ROM PX-32TS  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PX32TS;
		}
		else if (!strncmp(pDevice->szProductId, "CD-ROM PX-32CS  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PX32CS;
		}
		else if (!strncmp(pDevice->szProductId, "CD-ROM PX-20TS  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PX20TS;
		}
		else if (!strncmp(pDevice->szProductId, "CD-ROM PX-12TS  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PX12TS;
		}
		else if (!strncmp(pDevice->szProductId, "CD-ROM PX-12CS  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PX12CS;
		}
		else if (!strncmp(pDevice->szProductId, "CD-ROM PX-8XCS  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::PX8XCS;
		}
		else {
			pDevice->byPlxtrType = PLXTR_DRIVE_TYPE::No;
		}
	}
	if (pDevice->byPlxtrType) {
		pExtArg->byD8 = TRUE;
	}
	return TRUE;
}

BOOL IsValidPregapSector(
	PDISC pDisc,
	PSUB_Q pSubQ,
	INT nLBA
	)
{
	BOOL bRet = FALSE;
	if ((nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 225 ||
		nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 150 ||
		nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 149) &&
		(pSubQ->present.byCtl & AUDIO_DATA_TRACK) == 0 &&
		(pSubQ->next.byCtl & AUDIO_DATA_TRACK) == 0 &&
		pSubQ->next.byIndex == 0
		) {
		bRet = TRUE;
	}
	return bRet;
}

BOOL IsValidLibCryptSector(
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

BOOL IsValidSubQCtl(
	PSUB_Q pSubQ,
	BYTE byEndCtl
	)
{
	BOOL bRet = TRUE;
	switch (pSubQ->present.byCtl) {
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
		s_lineNum = __LINE__;
		return FALSE;
	}

	if (pSubQ->prev.byCtl != pSubQ->present.byCtl) {
		if ((pSubQ->present.byCtl != byEndCtl) && pSubQ->present.byCtl != 0) {
			s_lineNum = __LINE__;
			bRet = FALSE;
		}
		else {
			if (pSubQ->present.byAdr == ADR_ENCODES_CURRENT_POSITION) {
				if (pSubQ->prev.byTrackNum + 1 == pSubQ->present.byTrackNum) {
					if (pSubQ->prev.nRelativeTime + 1 == pSubQ->present.nRelativeTime) {
						s_lineNum = __LINE__;
						bRet = FALSE;
					}
				}
				else if (pSubQ->prev.byTrackNum == pSubQ->present.byTrackNum &&
					pSubQ->prev.byIndex == pSubQ->present.byIndex) {
					if (pSubQ->prev.nRelativeTime + 1 == pSubQ->present.nRelativeTime) {
						s_lineNum = __LINE__;
						bRet = FALSE;
					}
				}
			}
			// EVE - burst error (Disc 3) (Terror Disc)
			// LBA[188021, 0x2DE75],  Data,      Copy NG,                  Track[01], Idx[01], RMSF[41:46:71], AMSF[41:48:71], RtoW[0, 0, 0, 0]
			// LBA[188022, 0x2DE76], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :72], RtoW[0, 0, 0, 0]
			// LBA[188023, 0x2DE77],  Data,      Copy NG,                  Track[01], Idx[01], RMSF[41:46:73], AMSF[41:48:73], RtoW[0, 0, 0, 0]
			else if (pSubQ->present.byAdr == ADR_ENCODES_MEDIA_CATALOG) {
				s_lineNum = __LINE__;
				bRet = FALSE;
			}
			if (pSubQ->prevPrev.byCtl == pSubQ->present.byCtl) {
				bRet = TRUE;
			}
		}
	}
	return bRet;
}

BOOL IsValidSubQTrack(
	PEXEC_TYPE pExecType,
	PDISC pDisc,
	PSUB_Q pSubQ,
	INT nLBA,
	BYTE byCurrentTrackNum,
	LPBOOL bPrevTrackNum
	)
{
	if (*pExecType == gd) {
		if (pDisc->GDROM_TOC.LastTrack < pSubQ->present.byTrackNum) {
			s_lineNum = __LINE__;
			return FALSE;
		}
	}
	else {
		if (pDisc->SCSI.toc.LastTrack < pSubQ->present.byTrackNum) {
			s_lineNum = __LINE__;
			return FALSE;
		}
		else if (pSubQ->next.byAdr == ADR_NO_MODE_INFORMATION &&
			pSubQ->next.byTrackNum > 0 &&
			pSubQ->next.byTrackNum < pSubQ->present.byTrackNum) {
			s_lineNum = __LINE__;
			return FALSE;
		}
	}
	BOOL bRet = TRUE;
	if (pSubQ->prev.byTrackNum != pSubQ->present.byTrackNum) {
		if (pSubQ->prev.byTrackNum + 2 <= pSubQ->present.byTrackNum) {
			s_lineNum = __LINE__;
			return FALSE;
		}
		else if (pSubQ->present.byTrackNum < pSubQ->prev.byTrackNum) {
			if (pSubQ->prevPrev.byTrackNum == pSubQ->present.byTrackNum) {
				*bPrevTrackNum = FALSE;
			}
			else {
				s_lineNum = __LINE__;
				return FALSE;
			}
		}
		else {
			if (pSubQ->prev.byTrackNum + 1 == pSubQ->present.byTrackNum) {
				if (nLBA != pDisc->SCSI.lpFirstLBAListOnToc[byCurrentTrackNum]) {
					// Super CD-ROM^2 Taiken Soft Shuu (Japan)
					// LBA[139289, 0x22019], Audio, 2ch, Copy NG, Pre-emphasis No, Track[16], Idx[01], RMSF[01:19:00], AMSF[30:59:14], RtoW[0, 0, 0, 0]
					// LBA[139290, 0x2201a], Audio, 2ch, Copy NG, Pre-emphasis No, Track[16], Idx[01], RMSF[01:19:01], AMSF[30:59:15], RtoW[0, 0, 0, 0]
					// LBA[139291, 0x2201b], Audio, 2ch, Copy NG, Pre-emphasis No, Track[17], Idx[01], RMSF[01:19:02], AMSF[30:59:16], RtoW[0, 0, 0, 0]
					// LBA[139292, 0x2201c], Audio, 2ch, Copy NG, Pre-emphasis No, Track[16], Idx[01], RMSF[01:19:03], AMSF[30:59:17], RtoW[0, 0, 0, 0]
					if ((pSubQ->prev.byAdr == ADR_ENCODES_CURRENT_POSITION &&
						pSubQ->prev.nRelativeTime + 1 == pSubQ->present.nRelativeTime) ||
						(pSubQ->prevPrev.byAdr == ADR_ENCODES_CURRENT_POSITION &&
						pSubQ->prevPrev.nRelativeTime + 2 == pSubQ->present.nRelativeTime)) {
						s_lineNum = __LINE__;
						bRet = FALSE;
					}
					else if (pSubQ->next.byAdr == ADR_ENCODES_CURRENT_POSITION && pSubQ->next.byTrackNum > 0) {
						// Ys III (Japan)
						// LBA[226292, 0x373f4], Audio, 2ch, Copy NG, Pre-emphasis No, Track[20], Idx[01], RMSF[00:37:62], AMSF[50:19:17], RtoW[0, 0, 0, 0]
						// LBA[226293, 0x373f5], Audio, 2ch, Copy NG, Pre-emphasis No, Track[21], Idx[01], RMSF[00:37:63], AMSF[50:19:18], RtoW[0, 0, 0, 0]
						// LBA[226294, 0x373f6], Audio, 2ch, Copy NG, Pre-emphasis No, Track[20], Idx[01], RMSF[00:37:64], AMSF[50:19:19], RtoW[0, 0, 0, 0]
						if (pSubQ->present.byTrackNum != pSubQ->next.byTrackNum) {
							// Sega Flash Vol. 3 (Eu)
							// LBA[221184, 0x36000], Audio, 2ch, Copy NG, Pre-emphasis No, Track[30], Idx[01], RMSF[00:05:00], AMSF[49:11:09], RtoW[0, 0, 0, 0]
							// LBA[221185, 0x36001], Audio, 2ch, Copy NG, Pre-emphasis No, Track[31], Idx[01], RMSF[00:01:74], AMSF[49:11:10], RtoW[0, 0, 0, 0]
							// LBA[221186, 0x36002], Audio, 2ch, Copy NG, Pre-emphasis No, Track[11], Idx[01], RMSF[00:01:73], AMSF[49:11:11], RtoW[0, 0, 0, 0]
							// LBA[221187, 0x36003], Audio, 2ch, Copy NG, Pre-emphasis No, Track[31], Idx[00], RMSF[00:01:72], AMSF[49:11:12], RtoW[0, 0, 0, 0]
							if (pSubQ->prev.byTrackNum != pSubQ->next.byTrackNum) {
								if (pSubQ->nextNext.byAdr == ADR_ENCODES_CURRENT_POSITION && pSubQ->nextNext.byTrackNum > 0) {
									if (pSubQ->present.byTrackNum != pSubQ->nextNext.byTrackNum) {
										s_lineNum = __LINE__;
										bRet = FALSE;
									}
								}
							}
							else {
								s_lineNum = __LINE__;
								bRet = FALSE;
							}
						}
					}
					// Godzilla - Rettou Shinkan
					// LBA[125215, 0x1e91f], Audio, 2ch, Copy NG, Pre-emphasis No, Track[13], Idx[01], RelTime[00:54:41], AbsTime[27:51:40], RtoW[0, 0, 0, 0]
					// LBA[125216, 0x1e920], Audio, 2ch, Copy NG, Pre-emphasis No, Track[14], Idx[00], RelTime[00:01:73], AbsTime[27:51:41], RtoW[0, 0, 0, 0]
					// LBA[125217, 0x1e921], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number  [0000000000000], AbsTime[     :42], RtoW[0, 0, 0, 0]
					// LBA[125218, 0x1e922], Audio, 2ch, Copy NG, Pre-emphasis No, Track[14], Idx[00], RelTime[00:01:71], AbsTime[27:51:43], RtoW[0, 0, 0, 0]
					else if ((pSubQ->next.byAdr == ADR_ENCODES_MEDIA_CATALOG ||
						pSubQ->next.byAdr == ADR_ENCODES_ISRC) && pSubQ->nextNext.byTrackNum > 0) {
						if (pSubQ->present.byTrackNum != pSubQ->nextNext.byTrackNum) {
							s_lineNum = __LINE__;
							bRet = FALSE;
						}
					}
				}
			}
			if (pSubQ->prevPrev.byTrackNum == pSubQ->present.byTrackNum) {
				bRet = TRUE;
			}
		}
	}
	return bRet;
}

BOOL IsValidSubQIdx(
	PDISC pDisc,
	PSUB_Q pSubQ,
	INT nLBA,
	BYTE byCurrentTrackNum,
	LPBOOL bPrevIndex,
	LPBOOL bPrevPrevIndex
	)
{
	if (nLBA < 1) {
		return TRUE;
	}
	else if (MAXIMUM_NUMBER_INDEXES < pSubQ->present.byIndex) {
		s_lineNum = __LINE__;
		return FALSE;
	}
	BOOL bRet = TRUE;
	if (pSubQ->prev.byIndex != pSubQ->present.byIndex) {
		if (nLBA != pDisc->SCSI.lpFirstLBAListOnToc[byCurrentTrackNum]) {
			if (pSubQ->prev.byIndex + 1 < pSubQ->present.byIndex) {
				s_lineNum = __LINE__;
				bRet = FALSE;
			}
			else if (pSubQ->next.byTrackNum > 0 && 
				pSubQ->present.byIndex - 1 == pSubQ->next.byIndex &&
				pSubQ->prev.byIndex == pSubQ->next.byIndex &&
				pSubQ->next.byAdr == ADR_ENCODES_CURRENT_POSITION) {
				// 1552 Tenka Tairan
				// LBA[126959, 0x1efef], Audio, 2ch, Copy NG, Pre-emphasis No, Track[14], Idx[01], RMSF[01:50:13], AMSF[28:14:59], RtoW[0, 0, 0, 0]
				// LBA[126960, 0x1eff0], Audio, 2ch, Copy NG, Pre-emphasis No, Track[14], Idx[02], RMSF[01:50:14], AMSF[28:14:60], RtoW[0, 0, 0, 0]
				// LBA[126961, 0x1eff1], Audio, 2ch, Copy NG, Pre-emphasis No, Track[14], Idx[01], RMSF[01:50:15], AMSF[28:14:61], RtoW[0, 0, 0, 0]
				s_lineNum = __LINE__;
				bRet = FALSE;
			}
			else if (pSubQ->nextNext.byTrackNum > 0 &&
				pSubQ->prev.byIndex + 1 == pSubQ->present.byIndex &&
				pSubQ->prev.byIndex == pSubQ->nextNext.byIndex) {
				// Super Schwarzschild 2 (Japan)
				// LBA[234845, 0x3955D], Audio, 2ch, Copy NG, Pre-emphasis No, Track[19], Idx[01], RMSF[01:50:09], AMSF[52:13:20], RtoW[0, 0, 0, 0]
				// LBA[234846, 0x3955E], Audio, 2ch, Copy NG, Pre-emphasis No, Track[19], Idx[02], RMSF[01:50:10], AMSF[52:13:21], RtoW[0, 0, 0, 0]
				// LBA[234847, 0x3955F], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :22], RtoW[0, 0, 0, 0]
				// LBA[234848, 0x39560], Audio, 2ch, Copy NG, Pre-emphasis No, Track[19], Idx[01], RMSF[01:50:12], AMSF[52:13:23], RtoW[0, 0, 0, 0]
				s_lineNum = __LINE__;
				bRet = FALSE;
			}
			else {
				bRet = FALSE;
				// first sector on TOC
				if (pSubQ->prev.byIndex == 0 && pSubQ->present.byIndex == 1 &&
					pSubQ->present.nRelativeTime == 0) {
					bRet = TRUE;
				}
				else if ((pSubQ->present.byIndex == 0 || pSubQ->present.byIndex == 1) &&
					pSubQ->prev.byTrackNum + 1 == pSubQ->present.byTrackNum &&
					pSubQ->present.nRelativeTime < pSubQ->prev.nRelativeTime) {
					bRet = TRUE;
				}
				// multi index sector
				else if (pSubQ->prev.byIndex + 1 == pSubQ->present.byIndex &&
					pSubQ->prev.nRelativeTime + 1 == pSubQ->present.nRelativeTime) {
					bRet = TRUE;
				}
				// first pregap sector
				else if (pSubQ->prev.byIndex == 1 && pSubQ->present.byIndex == 0) {
					if (pSubQ->present.nRelativeTime - 1 == pSubQ->next.nRelativeTime) {
						bRet = TRUE;
					}
					// Shanghai - Matekibuyu (Japan)
					// LBA[016447, 0x0403f], Audio, 2ch, Copy NG, Pre-emphasis No, Track[04], Idx[01], RMSF[00:16:11], AMSF[03:41:22], RtoW[0, 0, 0, 0]
					// LBA[016448, 0x04040], Audio, 2ch, Copy NG, Pre-emphasis No, Track[05], Idx[00], RMSF[00:21:74], AMSF[03:41:23], RtoW[0, 0, 0, 0]
					// LBA[016449, 0x04041], Audio, 2ch, Copy NG, Pre-emphasis No, Track[05], Idx[00], RMSF[00:01:73], AMSF[03:41:24], RtoW[0, 0, 0, 0]
					else if (IsValidPregapSector(pDisc, pSubQ, nLBA)) {
						bRet = TRUE;
					}
				}
				if (pSubQ->prevPrev.byIndex == pSubQ->present.byIndex) {
					bRet = TRUE;
					if (pSubQ->prev.byIndex - 1 == pSubQ->present.byIndex) {
						*bPrevIndex = FALSE;
					}
				}
			}
		}
	}
	else if (pSubQ->prev.byIndex == pSubQ->present.byIndex &&
		pSubQ->prevPrev.byIndex - 1 == pSubQ->present.byIndex &&
		pSubQ->prevPrev.byTrackNum + 1 != pSubQ->present.byTrackNum) {
		*bPrevPrevIndex = FALSE;
	}
	return bRet;
}

BOOL IsValidSubQMSF(
	PEXEC_TYPE pExecType,
	LPBYTE lpSubcode,
	BYTE m,
	BYTE s,
	BYTE f
	)
{
	BOOL bRet = TRUE;
	if (*pExecType == gd) {
		if (lpSubcode[m] > 0xc2) {
			bRet = FALSE;
		}
	}
	else {
		if (lpSubcode[m] > 0x99) {
			bRet = FALSE;
		}
	}
	if (lpSubcode[s] > 0x59) {
		bRet = FALSE;
	}
	else if (lpSubcode[f] > 0x74) {
		bRet = FALSE;
	}
	return bRet;
}

BOOL IsValidSubQRMSF(
	PEXEC_TYPE pExecType,
	PSUB_Q pSubQ,
	LPBYTE lpSubcode,
	INT nLBA
	)
{
	BOOL bRet = IsValidSubQMSF(pExecType, lpSubcode, 15, 16, 17);
	if (!bRet) {
		return bRet;
	}
	INT tmpLBA = MSFtoLBA(BcdToDec(lpSubcode[15]), 
		BcdToDec(lpSubcode[16]), BcdToDec(lpSubcode[17]));
	if (tmpLBA != 0) {
		if (pSubQ->present.byIndex > 0) {
			if (pSubQ->prev.nRelativeTime != 0 && pSubQ->present.nRelativeTime != 0 &&
				pSubQ->prev.nRelativeTime + 1 != pSubQ->present.nRelativeTime) {
				s_lineNum = __LINE__;
				bRet = FALSE;
			}
			else if (pSubQ->prev.byIndex > 0 &&
				pSubQ->prev.nRelativeTime + 1 != pSubQ->present.nRelativeTime) {
				// ???
				// LBA[015496, 0x03c88], Audio, 2ch, Copy NG, Pre-emphasis No, Track[09], Idx[01], RMSF[00:13:42], AMSF[03:28:46], RtoW[0, 0, 0, 0]
				// LBA[015497, 0x03c89], Audio, 2ch, Copy NG, Pre-emphasis No, Track[10], Idx[01], RMSF[00:00:00], AMSF[03:28:47], RtoW[0, 0, 0, 0]
				// LBA[015498, 0x03c8a], Audio, 2ch, Copy NG, Pre-emphasis No, Track[10], Idx[01], RMSF[08:00:01], AMSF[03:28:48], RtoW[0, 0, 0, 0]
				s_lineNum = __LINE__;
				bRet = FALSE;
			}
			// Nights into Dreams (US)
			// LBA[201301, 0x31255], Audio, 2ch, Copy NG, Pre-emphasis No, Track[19], Idx[01], RMSF[01:26:00], AMSF[44:46:01], RtoW[0, 0, 0, 0]
			// LBA[201302, 0x31256], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :02], RtoW[0, 0, 0, 0]
			// LBA[201303, 0x31257], Audio, 2ch, Copy NG, Pre-emphasis No, Track[19], Idx[01], RMSF[00:26:02], AMSF[44:46:03], RtoW[0, 0, 0, 0]
			//  :
			// LBA[201528, 0x31338], Audio, 2ch, Copy NG, Pre-emphasis No, Track[20], Idx[01], RMSF[00:00:01], AMSF[44:49:03], RtoW[0, 0, 0, 0]
			if (pSubQ->prevPrev.nRelativeTime + 2 == pSubQ->present.nRelativeTime) {
				bRet = TRUE;
			}
		}
		else if (pSubQ->present.byIndex == 0) {
			// SagaFrontier Original Sound Track (Disc 3)
			// LBA[009948, 0x026DC], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[00], RMSF[00:00:01], AMSF[02:14:48], RtoW[0, 0, 0, 0]
			// LBA[009949, 0x026DD], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[00], RMSF[00:00:00], AMSF[02:14:49], RtoW[0, 0, 0, 0]
			// LBA[009950, 0x026DE], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:00:00], AMSF[02:14:50], RtoW[0, 0, 0, 0]
			// LBA[009951, 0x026DF], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:00:01], AMSF[02:14:51], RtoW[0, 0, 0, 0]
			// Now on Never (Nick Carter) (ZJCI-10118)
			// LBA[000598, 0x00256], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[00], RMSF[00:00:02], AMSF[00:09:73], RtoW[0, 0, 0, 0]
			// LBA[000599, 0x00257], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[00], RMSF[00:00:01], AMSF[00:09:74], RtoW[0, 0, 0, 0]
			// LBA[000600, 0x00258], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:00:00], AMSF[00:10:00], RtoW[0, 0, 0, 0]
			// LBA[000601, 0x00259], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:00:01], AMSF[00:10:01], RtoW[0, 0, 0, 0]
			if (pSubQ->prev.byTrackNum == pSubQ->present.byTrackNum &&
				pSubQ->prev.nRelativeTime != pSubQ->present.nRelativeTime + 1) {
				s_lineNum = __LINE__;
				bRet = FALSE;
			}
		}
	}
	else if (tmpLBA == 0) {
		// Midtown Madness (Japan)
		// LBA[198294, 0x30696], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :69], RtoW[0, 0, 0, 0]
		//  :
		// LBA[198342, 0x306c6], Audio, 2ch, Copy NG, Pre-emphasis No, Track[08], Idx[01], RMSF[02:34:74], AMSF[44:06:42], RtoW[0, 0, 0, 0]
		// LBA[198343, 0x306c7], Audio, 2ch, Copy NG, Pre-emphasis No, Track[09], Idx[00], RMSF[00:01:74], AMSF[44:06:43], RtoW[0, 0, 0, 0]
		// LBA[198344, 0x306c8], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :44], RtoW[0, 0, 0, 0]
		// LBA[198345, 0x306c9], Audio, 2ch, Copy NG, Pre-emphasis No, Track[09], Idx[00], RMSF[00:01:72], AMSF[44:06:45], RtoW[0, 0, 0, 0]
		//  :
		// LBA[198392, 0x306f8], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :17], RtoW[0, 0, 0, 0]
		if (nLBA != 0 &&
			pSubQ->prev.byTrackNum == pSubQ->present.byTrackNum &&
			pSubQ->prev.byIndex == pSubQ->present.byIndex) {
			if (pSubQ->present.byIndex != 0) {
				if (pSubQ->prev.nRelativeTime + 1 != pSubQ->present.nRelativeTime) {
					s_lineNum = __LINE__;
					bRet = FALSE;
				}
			}
			else if (pSubQ->present.byIndex == 0) {
				if (pSubQ->prev.nRelativeTime != pSubQ->present.nRelativeTime + 1) {
					s_lineNum = __LINE__;
					bRet = FALSE;
				}
			}
		}
	}
	return bRet;
}

BOOL IsValidSubQAFrame(
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

BOOL IsValidSubQAMSF(
	PEXEC_TYPE pExecType,
	BOOL bRipPregap,
	PSUB_Q pSubQ,
	LPBYTE lpSubcode,
	INT nLBA
	)
{
	BOOL bRet = IsValidSubQMSF(pExecType, lpSubcode, 19, 20, 21);
	if (bRet) {
		INT tmpLBA = MSFtoLBA(BcdToDec(lpSubcode[19]), 
			BcdToDec(lpSubcode[20]), BcdToDec(lpSubcode[21])) - 150;
		if (nLBA != tmpLBA || bRipPregap) {
			if (pSubQ->prev.nAbsoluteTime + 1 != pSubQ->present.nAbsoluteTime) {
				bRet = FALSE;
			}
		}
	}
	return bRet;
}

BOOL IsValidSubQMCN(
	LPBYTE lpSubcode
	)
{
	BOOL bRet = TRUE;
	for (INT i = 13; i <= 19; i++) {
		if (isdigit(((lpSubcode[i] >> 4) & 0x0f) + 0x30) == 0) {
			bRet = FALSE;
			break;
		}
		if (isdigit((lpSubcode[i] & 0x0f) + 0x30) == 0) {
			bRet = FALSE;
			break;
		}
	}
	return bRet;
}

// reference
// https://isrc.jmd.ne.jp/about/pattern.html
// https://isrc.jmd.ne.jp/about/error.html
BOOL IsValidSubQISRC(
	LPBYTE lpSubcode
	)
{
	INT ch = ((lpSubcode[13] >> 2) & 0x3f) + 0x30;
	if (isupper(ch) == 0) {
		return FALSE;
	}

	ch = (((lpSubcode[13] << 4) & 0x30) |
		((lpSubcode[14] >> 4) & 0x0f)) + 0x30;
	if (isupper(ch) == 0) {
		return FALSE;
	}

	ch = (((lpSubcode[14] << 2) & 0x3c) |
		((lpSubcode[15] >> 6) & 0x03)) + 0x30;
	if (isupper(ch) == 0 && isdigit(ch) == 0) {
		return FALSE;
	}

	ch = (lpSubcode[15] & 0x3f) + 0x30;
	if (isupper(ch) == 0 && isdigit(ch) == 0) {
		return FALSE;
	}

	ch = ((lpSubcode[16] >> 2) & 0x3f) + 0x30;
	if (isupper(ch) == 0 && isdigit(ch) == 0) {
		return FALSE;
	}

	for (INT i = 17; i <= 19; i++) {
		if (isdigit(((lpSubcode[i] >> 4) & 0x0f) + 0x30) == 0) {
			return FALSE;
		}
		if (isdigit((lpSubcode[i] & 0x0f) + 0x30) == 0) {
			return FALSE;
		}
	}

	if (isdigit(((lpSubcode[20] >> 4) & 0x0f) + 0x30) == 0) {
		return FALSE;
	}
	return TRUE;
}

BOOL IsValidSubQAdrSector(
	DWORD dwSubAdditionalNum,
	BYTE byPrevAdr,
	BYTE byNextAdr,
	INT nRangeLBA,
	INT nFirstLBA,
	INT nPrevAdrSector,
	INT nLBA
	)
{
	BOOL bRet = FALSE;
	if (nLBA < 0 ||
		(nLBA % nRangeLBA == nFirstLBA) ||
		(nLBA - nPrevAdrSector == nRangeLBA)) {
		if (1 <= dwSubAdditionalNum) {
			if (byNextAdr != ADR_ENCODES_MEDIA_CATALOG &&
				byNextAdr != ADR_ENCODES_ISRC) {
				bRet = TRUE;
			}
		}
		else {
			bRet = TRUE;
		}
	}
	else if ((nLBA % nRangeLBA == nFirstLBA + 1) && byPrevAdr == ADR_ENCODES_CURRENT_POSITION) {
		// Originally, MCN sector exists per same frame number, but in case of 1st sector or next idx of the track, MCN sector slides at the next sector
		//
		// SaGa Frontier Original Sound Track (Disc 3) [First MCN Sector: 33, MCN sector exists per 91 frame]
		// LBA[039709, 0x09b1d], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :34], RtoW[0, 0, 0, 0]
		//  :
		// LBA[039799, 0x09b77], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[01], RMSF[03:26:24], AMSF[08:52:49], RtoW[0, 0, 0, 0]
		// LBA[039800, 0x09b78], Audio, 2ch, Copy NG, Pre-emphasis No, Track[03], Idx[00], RMSF[00:01:36], AMSF[08:52:50], RtoW[0, 0, 0, 0]
		// LBA[039801, 0x09b79], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[08:52:51], RtoW[0, 0, 0, 0]
		// LBA[039802, 0x09b7a], Audio, 2ch, Copy NG, Pre-emphasis No, Track[03], Idx[00], RMSF[00:01:34], AMSF[08:52:51], RtoW[0, 0, 0, 0]
		//  :
		// LBA[039891, 0x09bd3], Audio, 2ch, Copy NG, Pre - emphasis No, MediaCatalogNumber[0000000000000], AMSF[    :66], RtoW[0, 0, 0, 0]

		// Super Real Marjang Special (Japan) [First MCN Sector: 71, MCN sector exists per: 98 frame]
		// LBA[090819, 0x162c3], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :69], RtoW[0, 0, 0, 0]
		//  :
		// LBA[090916, 0x16324], Audio, 2ch, Copy NG, Pre-emphasis No, Track[37], Idx[01], RMSF[00:09:12], AMSF[20:14:16], RtoW[0, 0, 0, 0]
		// LBA[090917, 0x16325], Audio, 2ch, Copy NG, Pre-emphasis No, Track[38], Idx[01], RMSF[00:00:00], AMSF[20:14:17], RtoW[0, 0, 0, 0]
		// LBA[090918, 0x16326], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :18], RtoW[0, 0, 0, 0]
		// LBA[090919, 0x16327], Audio, 2ch, Copy NG, Pre-emphasis No, Track[38], Idx[01], RMSF[00:00:02], AMSF[20:14:19], RtoW[0, 0, 0, 0]
		//  :
		// LBA[091015, 0x16387], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :40], RtoW[0, 0, 0, 0]

		// Midtown Madness (Japan) [First MCN sector: 40, MCN sector exists per 98 frame]
		// LBA[209270, 0x33176], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :20], RtoW[0, 0, 0, 0]
		//  :
		// LBA[209367, 0x331d7], Audio, 2ch, Copy NG, Pre-emphasis No, Track[10], Idx[00], RMSF[00:00:00], AMSF[46:33:42], RtoW[0, 0, 0, 0]
		// LBA[209368, 0x331d8], Audio, 2ch, Copy NG, Pre-emphasis No, Track[10], Idx[01], RMSF[00:00:00], AMSF[46:33:43], RtoW[0, 0, 0, 0]
		// LBA[209369, 0x331d9], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :44], RtoW[0, 0, 0, 0]
		// LBA[209370, 0x331da], Audio, 2ch, Copy NG, Pre-emphasis No, Track[10], Idx[01], RMSF[00:00:02], AMSF[46:33:45], RtoW[0, 0, 0, 0]
		//  :
		// LBA[209466, 0x3323a], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :66], RtoW[0, 0, 0, 0]
		bRet = TRUE;
	}
	else if (nFirstLBA <= nLBA && nLBA % nRangeLBA == 0 && nRangeLBA == nFirstLBA + 1) {
		// Tengai Makyou - Fuuun Kabuki Den (Japan) [First MCN sector: 97, MCN sector exists per 98 frame]
		// LBA[241373, 0x3aedd], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :23], RtoW[0, 0, 0, 0]
		//  :
		// LBA[241469, 0x3af3d], Audio, 2ch, Copy NG, Pre-emphasis No, Track[31], Idx[01], RMSF[00:48:03], AMSF[53:41:44], RtoW[0, 0, 0, 0]
		// LBA[241470, 0x3af3e], Audio, 2ch, Copy NG, Pre-emphasis No, Track[31], Idx[01], RMSF[00:48:04], AMSF[53:41:45], RtoW[0, 0, 0, 0]
		// LBA[241471, 0x3af3f], Audio, 2ch, Copy NG, Pre-emphasis No, Track[32], Idx[01], RMSF[00:00:00], AMSF[53:41:46], RtoW[0, 0, 0, 0]
		// LBA[241472, 0x3af40], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :47], RtoW[0, 0, 0, 0]
		// LBA[241473, 0x3af41], Audio, 2ch, Copy NG, Pre-emphasis No, Track[32], Idx[01], RMSF[00:00:02], AMSF[53:41:48], RtoW[0, 0, 0, 0]
		//  :
		// LBA[241569, 0x3afa1], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :69], RtoW[0, 0, 0, 0]
		bRet = TRUE;
	}
	return bRet;
}

VOID CheckMainChannel(
	PDISC pDisc,
	LPBYTE lpBuf,
	PSUB_Q pSubQ,
	BYTE byCurrentTrackNum,
	INT nLBA
	)
{
	INT nOfs = pDisc->MAIN.nCombinedOffset;
	BYTE ctl = 0;
	INT nAdd = 0;

	if (0 <= nOfs && nOfs < 2352) {
		ctl = pSubQ->present.byCtl;
	}
	else if (2352 <= nOfs && nOfs < 4704) {
		ctl = pSubQ->prev.byCtl;
		nAdd--;
	}
	else if (4704 <= nOfs && nOfs < 7056) {
		ctl = pSubQ->prevPrev.byCtl;
		nAdd -= 2;
	}

	if ((ctl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
		if (!IsValidMainDataHeader(lpBuf + pDisc->MAIN.uiMainDataSlideSize)) {
			OutputMainErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: This sector is data, but a header doesn't exist\n",
				nLBA + nAdd, nLBA + nAdd, byCurrentTrackNum);
		}
	}
	return;
}

VOID CheckAndFixSubP(
	LPBYTE lpSubcode,
	BYTE byCurrentTrackNum,
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
			OutputSubErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: SubP[%02d]:[%#04x] -> [0xff]\n",
				nLBA, nLBA, byCurrentTrackNum, i, lpSubcode[i]);
			lpSubcode[i] = 0xff;
		}
		else if (b00 && lpSubcode[i] != 0x00) {
			OutputSubErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: SubP[%02d]:[%#04x] -> [0x00]\n",
				nLBA, nLBA, byCurrentTrackNum, i, lpSubcode[i]);
			lpSubcode[i] = 0x00;
		}
		else if (!bFF && !b00) {
			OutputSubErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: SubP[%02d]:[%#04x] -> [0x00]\n",
				nLBA, nLBA, byCurrentTrackNum, i, lpSubcode[i]);
			lpSubcode[i] = 0x00;
		}
	}
	return;
}

BOOL IsValidSubQAdrMCN(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpSubcode,
	PSUB_Q pSubQ,
	BYTE byCurrentTrackNum,
	INT nLBA
	)
{
	if (!pDisc->SUB.byCatalog) {
		return FALSE;
	}
	INT session = pDisc->SCSI.lpSessionNumList[byCurrentTrackNum - 1];
	INT nRangeLBA = pDisc->SUB.nRangeLBAForMCN[0][session - 1];
	if (nRangeLBA == -1) {
		return FALSE;
	}

	INT nFirstLBA = pDisc->SUB.nFirstLBAForMCN[0][session - 1];
	INT nPrevMCNSector = pDisc->SUB.nPrevMCNSector;
	BOOL bRet = IsValidSubQAdrSector(pExtArg->dwSubAddionalNum, pSubQ->prev.byAdr,
		pSubQ->next.byAdr, nRangeLBA, nFirstLBA, nPrevMCNSector, nLBA);
	if (!bRet) {
		nRangeLBA = pDisc->SUB.nRangeLBAForMCN[1][session - 1];
		if (nRangeLBA != -1) {
			nFirstLBA = pDisc->SUB.nFirstLBAForMCN[1][session - 1];
			bRet = IsValidSubQAdrSector(pExtArg->dwSubAddionalNum, pSubQ->prev.byAdr,
				pSubQ->next.byAdr, nRangeLBA, nFirstLBA, nPrevMCNSector, nLBA);
		}
	}
	if (bRet) {
		if (pSubQ->present.byAdr != ADR_ENCODES_MEDIA_CATALOG &&
			pSubQ->prev.byAdr != ADR_ENCODES_MEDIA_CATALOG) {
			OutputSubErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: SubQ[12]:Adr[%d] -> [0x02]\n",
				nLBA, nLBA, byCurrentTrackNum, pSubQ->present.byAdr);
			pSubQ->present.byAdr = ADR_ENCODES_MEDIA_CATALOG;
			lpSubcode[12] = (BYTE)(pSubQ->present.byCtl << 4 | pSubQ->present.byAdr);
		}
		if (pSubQ->present.byAdr == ADR_ENCODES_MEDIA_CATALOG) {
			CHAR szCatalog[META_CATALOG_SIZE] = { 0 };
			BOOL bMCN = IsValidSubQMCN(lpSubcode);
			SetMCNToString(pDisc, lpSubcode, szCatalog, FALSE);

			if (strncmp(pDisc->SUB.szCatalog, szCatalog, META_CATALOG_SIZE) || !bMCN) {
				OutputSubErrorLogA(
					"LBA[%06d, %#07x], Track[%02u]: SubQ[13-19]:MCN[%13s], Sub[19]Lo:[%x], Sub[20]:[%#04x] -> [%13s], [19]Lo:[0], [20]:[0x00]\n",
					nLBA, nLBA, byCurrentTrackNum, szCatalog, lpSubcode[19] & 0x0f, lpSubcode[20], pDisc->SUB.szCatalog);
				for (INT i = 13, j = 0; i < 19; i++, j += 2) {
					lpSubcode[i] = (BYTE)(pDisc->SUB.szCatalog[j] - 0x30);
					lpSubcode[i] <<= 4;
					lpSubcode[i] |= (BYTE)(pDisc->SUB.szCatalog[j + 1] - 0x30);
				}
				lpSubcode[19] = (BYTE)(pDisc->SUB.szCatalog[12] - 0x30);
				lpSubcode[19] <<= 4;
				lpSubcode[20] = 0;
			}
			bRet = TRUE;
		}
	}
	else {
		if (pSubQ->present.byAdr == ADR_ENCODES_MEDIA_CATALOG) {
			CHAR szCatalog[META_CATALOG_SIZE] = { 0 };
			BOOL bMCN = IsValidSubQMCN(lpSubcode);
			SetMCNToString(pDisc, lpSubcode, szCatalog, FALSE);

			if (strncmp(pDisc->SUB.szCatalog, szCatalog, META_CATALOG_SIZE) || !bMCN) {
				OutputSubErrorLogA(
					"LBA[%06d, %#07x], Track[%02u]: SubQ[12]:Adr[%d] -> No MCN frame\n",
					nLBA, nLBA, byCurrentTrackNum, pSubQ->present.byAdr);
				return FALSE;
			}
			INT nTmpFirstLBA = nLBA % pDisc->SUB.nRangeLBAForMCN[0][session - 1];
			OutputMainInfoLogA("LBA[%06d, %#07x], Track[%02u]: Range of MCN is different [%d]\n"
				, nLBA, nLBA, byCurrentTrackNum, nLBA - pDisc->SUB.nPrevMCNSector);
			pDisc->SUB.nFirstLBAForMCN[0][session - 1] = nTmpFirstLBA;
			bRet = TRUE;
		}
	}

	if (bRet) {
		pDisc->SUB.nPrevMCNSector = nLBA;
		if ((lpSubcode[19] & 0x0f) != 0) {
			OutputSubErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: SubQ[19]:[%x] -> [%x]\n",
				nLBA, nLBA, byCurrentTrackNum, lpSubcode[19], lpSubcode[19] & 0xf0);
			lpSubcode[19] &= 0xf0;
		}
		if (lpSubcode[20] != 0) {
			OutputSubErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: SubQ[20]:[%x] -> [0x00]\n",
				nLBA, nLBA, byCurrentTrackNum, lpSubcode[20]);
			lpSubcode[20] = 0;
		}
		// Subchannel pattern on MCN Sector
		// pattern 1: pregap sector.
		if (IsValidPregapSector(pDisc, pSubQ, nLBA)) {
			BOOL bValidPre = FALSE;
			// pattern 1-1: prev sector is audio.
			if ((pSubQ->prev.byCtl & AUDIO_DATA_TRACK) == 0) {
				// pattern 1-1-1: present sector is audio.
				if ((pSubQ->present.byCtl & AUDIO_DATA_TRACK) == 0) {
					if (pSubQ->prev.byTrackNum > 0) {
						if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 225) {
							// Atlas, The - Renaissance Voyager (Japan)
							// LBA[003364, 0x00d24], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:44:64], AMSF[00:46:64], RtoW[0, 0, 0, 0]
							// LBA[003365, 0x00d25], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :65], RtoW[0, 0, 0, 0]
							// LBA[003366, 0x00d26], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[00], RMSF[00:02:73], AMSF[00:46:66], RtoW[0, 0, 0, 0]
							// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
							// LBA[261585, 0x3fDD1], Audio, 2ch, Copy NG, Pre-emphasis No, Track[95], Idx[01], RMSF[00:13:69], AMSF[58:09:60], RtoW[0, 0, 0, 0]
							// LBA[261586, 0x3fDD2], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :61], RtoW[0, 0, 0, 0]
							// LBA[261587, 0x3fDD3], Audio, 2ch, Copy NG, Pre-emphasis No, Track[96], Idx[00], RMSF[00:02:73], AMSF[58:09:62], RtoW[0, 0, 0, 0]
							pSubQ->present.nRelativeTime = 224;
							bValidPre = TRUE;
						}
						else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 150) {
							pSubQ->present.nRelativeTime = 149;
							bValidPre = TRUE;
						}
						else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 149) {
							pSubQ->present.nRelativeTime = 148;
							bValidPre = TRUE;
						}
					}
					if (bValidPre && pExtArg->byMCN) {
						// pattern 1-1-1-1: change track.
						pSubQ->present.byTrackNum = (BYTE)(pSubQ->prev.byTrackNum + 1);
						// pattern 1-1-1-1-1: change index. (pregap sector is 0)
						pSubQ->present.byIndex = 0;
						OutputMainInfoLogA(
							"LBA[%06d, %#07x], Track[%02u]: The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
							nLBA, nLBA, byCurrentTrackNum + 1, pSubQ->present.byTrackNum, pSubQ->prev.byTrackNum);
					}
					else {
						// pattern 1-1-1-2: not change track.
						pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
						// pattern 1-1-1-2-2: not change index.
						pSubQ->present.byIndex = pSubQ->prev.byIndex;
					}
				}
				// pattern 1-1-2: present sector is data.
				else if ((pSubQ->present.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					if (pSubQ->prev.byTrackNum > 0) {
						if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 225) {
							pSubQ->present.nRelativeTime = 224;
							bValidPre = TRUE;
						}
						else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 150) {
							pSubQ->present.nRelativeTime = 149;
							bValidPre = TRUE;
						}
						else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 149) {
							pSubQ->present.nRelativeTime = 148;
							bValidPre = TRUE;
						}
					}
					if (bValidPre && pExtArg->byMCN) {
						// pattern 1-1-2-1: change track.
						pSubQ->present.byTrackNum = (BYTE)(pSubQ->prev.byTrackNum + 1);
						// pattern 1-1-2-1-1: change index. (pregap sector is 0)
						pSubQ->present.byIndex = 0;
						OutputMainInfoLogA(
							"LBA[%06d, %#07x], Track[%02u]: The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
							nLBA, nLBA, byCurrentTrackNum + 1, pSubQ->present.byTrackNum, pSubQ->prev.byTrackNum);
					}
					else {
						// pattern 1-1-2-2: not change track.
						pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
						// pattern 1-1-2-2-2: not change index.
						pSubQ->present.byIndex = pSubQ->prev.byIndex;
					}
				}
			}
			// pattern 1-2: prev sector is data.
			else if ((pSubQ->prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				// pattern 1-2-1: present sector is audio.
				if ((pSubQ->present.byCtl & AUDIO_DATA_TRACK) == 0) {
					if (pSubQ->prev.byTrackNum > 0) {
						if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 225) {
							pSubQ->present.nRelativeTime = 224;
							bValidPre = TRUE;
						}
						else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 150) {
							// Valis IV (Japan)
							// LBA[157830, 0x26886],  Data,      Copy NG,                  Track[44], Idx[01], RMSF[00:06:27], AMSF[35:06:30], RtoW[0, 0, 0, 0]
							// LBA[157831, 0x26887], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :31], RtoW[0, 0, 0, 0]
							// LBA[157832, 0x26888], Audio, 2ch, Copy NG, Pre-emphasis No, Track[45], Idx[00], RMSF[00:01:73], AMSF[35:06:32], RtoW[0, 0, 0, 0]
							// Cosmic Fantasy 2
							// LBA[202749, 0x317FD],  Data,      Copy NG,                  Track[80], Idx[01], RMSF[00:06:63], AMSF[45:05:24], RtoW[0, 0, 0, 0]
							// LBA[202750, 0x317FE], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :25], RtoW[0, 0, 0, 0]
							// LBA[202751, 0x317FF], Audio, 2ch, Copy NG, Pre-emphasis No, Track[81], Idx[00], RMSF[00:01:73], AMSF[45:05:26], RtoW[0, 0, 0, 0]
							pSubQ->present.nRelativeTime = 149;
							bValidPre = TRUE;
						}
						else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 149) {
							pSubQ->present.nRelativeTime = 148;
							bValidPre = TRUE;
						}
					}
					if (bValidPre && pExtArg->byMCN) {
						// pattern 1-2-1-1: change track.
						pSubQ->present.byTrackNum = (BYTE)(pSubQ->prev.byTrackNum + 1);
						// pattern 1-2-1-1-1: change index. (pregap sector is 0)
						pSubQ->present.byIndex = 0;
						OutputMainInfoLogA(
							"LBA[%06d, %#07x], Track[%02u]: The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
							nLBA, nLBA, byCurrentTrackNum + 1, pSubQ->present.byTrackNum, pSubQ->prev.byTrackNum);
					}
					else {
						// pattern 1-2-1-2: not change track.
						pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
						// pattern 1-2-1-2-2: not change index.
						pSubQ->present.byIndex = pSubQ->prev.byIndex;
					}
				}
				// pattern 1-2-2: present sector is data.
				else if ((pSubQ->present.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					if (pSubQ->prev.byTrackNum > 0) {
						if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 225) {
							pSubQ->present.nRelativeTime = 224;
							bValidPre = TRUE;
						}
						else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 150) {
							pSubQ->present.nRelativeTime = 149;
							bValidPre = TRUE;
						}
						else if (nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 149) {
							pSubQ->present.nRelativeTime = 148;
							bValidPre = TRUE;
						}
					}
					if (bValidPre && pExtArg->byMCN) {
						// pattern 1-2-2-1: change track.
						pSubQ->present.byTrackNum = (BYTE)(pSubQ->prev.byTrackNum + 1);
						// pattern 1-2-2-1-1: change index. (pregap sector is 0)
						pSubQ->present.byIndex = 0;
						OutputMainInfoLogA(
							"LBA[%06d, %#07x], Track[%02u]: The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
							nLBA, nLBA, byCurrentTrackNum + 1, pSubQ->present.byTrackNum, pSubQ->prev.byTrackNum);
					}
					else {
						// pattern 1-2-2-2: not change track.
						pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
						// pattern 1-2-2-2-2: not change index.
						pSubQ->present.byIndex = pSubQ->prev.byIndex;
					}
				}
			}
		}
		// pattern 2: not pregap sector.
		else {
			// pattern 2-1: prev sector is audio.
			if ((pSubQ->prev.byCtl & AUDIO_DATA_TRACK) == 0) {
				// pattern 2-1-1: present sector is audio.
				if ((pSubQ->present.byCtl & AUDIO_DATA_TRACK) == 0) {
					// 1st sector of tracks
					if (pSubQ->prev.byTrackNum > 0 &&
						nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum]) {
						// Madou Monogatari I - Honoo no Sotsuenji (Japan)
						// LBA[183031, 0x2CAF7], Audio, 2ch, Copy NG, Pre-emphasis No, Track[21], Idx[01], RMSF[00:31:70], AMSF[40:42:31], RtoW[0, 0, 0, 0]
						// LBA[183032, 0x2CAF8], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :32], RtoW[0, 0, 0, 0]
						// LBA[183033, 0x2CAF9], Audio, 2ch, Copy NG, Pre-emphasis No, Track[22], Idx[01], RMSF[00:00:01], AMSF[40:42:33], RtoW[0, 0, 0, 0]
						pSubQ->present.nRelativeTime = 0;
						// pattern 2-1-1-1: change track.
						pSubQ->present.byTrackNum = (BYTE)(pSubQ->prev.byTrackNum + 1);
						// pattern 2-1-1-1-2: not change index.
						pSubQ->present.byIndex = pSubQ->prev.byIndex;
						OutputMainInfoLogA(
							"LBA[%06d, %#07x], Track[%02u]: The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
							nLBA, nLBA, byCurrentTrackNum + 1, pSubQ->present.byTrackNum, pSubQ->prev.byTrackNum);
					}
					// 1st next index of same tracks
					else if (pSubQ->prev.byIndex == 0 && pSubQ->prev.nRelativeTime == 0) {
						// Psychic Detective Series Vol. 5 - Nightmare (Japan)
						// LBA[080999, 0x13C67], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[00], RMSF[00:00:00], AMSF[18:01:74], RtoW[0, 0, 0, 0]
						// LBA[081000, 0x13C68], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [3010911111863], AMSF[     :00], RtoW[0, 0, 0, 0]
						// LBA[081001, 0x13C69], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[01], RMSF[00:00:01], AMSF[18:02:01], RtoW[0, 0, 0, 0]
						pSubQ->present.nRelativeTime = 0;
						// pattern 2-1-1-2: not change track.
						pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
						// pattern 2-1-1-2-1: change index.
						pSubQ->present.byIndex = (BYTE)(pSubQ->prev.byIndex + 1);
					}
					// 1st index of same tracks
					else if (pSubQ->prev.byIndex > 1 && pSubQ->prev.byIndex != pSubQ->next.byIndex) {
						if (pSubQ->prev.byIndex + 1 == pSubQ->next.byIndex) {
							// Space Jam (Japan)
							// LBA[056262, 0x0dbc6], Audio, 2ch, Copy NG, Pre-emphasis No, Track[04], Idx[53], RMSF[01:38:65], AMSF[12:32:12], RtoW[0, 0, 0, 0]
							// LBA[056263, 0x0dbc7], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :13], RtoW[0, 0, 0, 0]
							// LBA[056264, 0x0dbc8], Audio, 2ch, Copy NG, Pre-emphasis No, Track[04], Idx[54], RMSF[01:38:67], AMSF[12:32:14], RtoW[0, 0, 0, 0]
							// Space Jam (Japan)
							// LBA[086838, 0x15336], Audio, 2ch, Copy NG, Pre-emphasis No, Track[06], Idx[82], RMSF[02:31:05], AMSF[19:19:63], RtoW[0, 0, 0, 0]
							// LBA[086839, 0x15337], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :64], RtoW[0, 0, 0, 0]
							// LBA[086840, 0x15338], Audio, 2ch, Copy NG, Pre-emphasis No, Track[06], Idx[83], RMSF[02:31:07], AMSF[19:19:65], RtoW[0, 0, 0, 0]
							pSubQ->present.nRelativeTime = pSubQ->prev.nRelativeTime + 1;
							// pattern 2-1-1-2: not change track.
							pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
							// pattern 2-1-1-2-1: change index.
							pSubQ->present.byIndex = (BYTE)(pSubQ->prev.byIndex + 1);
						}
					}
					// same index of same tracks
					else {
						if (pSubQ->prev.byIndex == 0) {
							// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
							// LBA[003413, 0x00D55], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[00], RMSF[00:02:26], AMSF[00:47:38], RtoW[0, 0, 0, 0]
							// LBA[003414, 0x00D56], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :39], RtoW[0, 0, 0, 0]
							// LBA[003415, 0x00D57], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[00], RMSF[00:02:24], AMSF[00:47:40], RtoW[0, 0, 0, 0]
							pSubQ->present.nRelativeTime = pSubQ->prev.nRelativeTime - 1;
						}
						else if (pSubQ->prev.byIndex > 0) {
							// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
							// LBA[081541, 0x13E85], Audio, 2ch, Copy NG, Pre-emphasis No, Track[18], Idx[01], RMSF[00:12:57], AMSF[18:09:16], RtoW[0, 0, 0, 0]
							// LBA[081542, 0x13E86], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :17], RtoW[0, 0, 0, 0]
							// LBA[081543, 0x13E87], Audio, 2ch, Copy NG, Pre-emphasis No, Track[18], Idx[01], RMSF[00:12:59], AMSF[18:09:18], RtoW[0, 0, 0, 0]
							pSubQ->present.nRelativeTime = pSubQ->prev.nRelativeTime + 1;
						}
						// pattern 2-1-1-2: not change track.
						pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
						// pattern 2-1-1-2-2: not change index.
						pSubQ->present.byIndex = pSubQ->prev.byIndex;
					}
				}
				// pattern 2-1-2: present sector is data.
				else if ((pSubQ->present.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					// 1st sector of tracks
					if (pSubQ->prev.byTrackNum > 0 &&
						nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum]) {
						pSubQ->present.nRelativeTime = 0;
						// pattern 2-1-2-1: change track.
						pSubQ->present.byTrackNum = (BYTE)(pSubQ->prev.byTrackNum + 1);
						// pattern 2-1-2-1-2: not change index.
						pSubQ->present.byIndex = pSubQ->prev.byIndex;
						OutputMainInfoLogA(
							"LBA[%06d, %#07x], Track[%02u]: The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
							nLBA, nLBA, byCurrentTrackNum + 1, pSubQ->present.byTrackNum, pSubQ->prev.byTrackNum);
					}
					// 1st next index of same tracks
					else if (pSubQ->prev.byIndex == 0 && pSubQ->prev.nRelativeTime == 0) {
						pSubQ->present.nRelativeTime = 0;
						// pattern 2-1-2-2: not change track.
						pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
						// pattern 2-1-2-2-1: change index.
						pSubQ->present.byIndex = (BYTE)(pSubQ->prev.byIndex + 1);
					}
					// same index of same tracks
					else {
						if (pSubQ->prev.byIndex == 0) {
							pSubQ->present.nRelativeTime = pSubQ->prev.nRelativeTime - 1;
						}
						else if (pSubQ->prev.byIndex > 0) {
							pSubQ->present.nRelativeTime = pSubQ->prev.nRelativeTime + 1;
						}
						// pattern 2-1-2-2: not change track.
						pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
						// pattern 2-1-2-2-2: not change index.
						pSubQ->present.byIndex = pSubQ->prev.byIndex;
					}
				}
			}
			// pattern 2-2: prev sector is data.
			else if ((pSubQ->prev.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				// pattern 2-2-1: present sector is audio.
				if ((pSubQ->present.byCtl & AUDIO_DATA_TRACK) == 0) {
					// 1st sector of tracks
					if (pSubQ->prev.byTrackNum > 0 &&
						nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum]) {
						pSubQ->present.nRelativeTime = 0;
						// pattern 2-2-1-1: change track.
						pSubQ->present.byTrackNum = (BYTE)(pSubQ->prev.byTrackNum + 1);
						// pattern 2-2-1-1-2: not change index.
						pSubQ->present.byIndex = pSubQ->prev.byIndex;
						OutputMainInfoLogA(
							"LBA[%06d, %#07x], Track[%02u]: The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
							nLBA, nLBA, byCurrentTrackNum + 1, pSubQ->present.byTrackNum, pSubQ->prev.byTrackNum);
					}
					// 1st next index of same tracks
					else if (pSubQ->prev.byIndex == 0 && pSubQ->prev.nRelativeTime == 0) {
						pSubQ->present.nRelativeTime = 0;
						// pattern 2-2-1-2: not change track.
						pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
						// pattern 2-2-1-2-1: change index.
						pSubQ->present.byIndex = (BYTE)(pSubQ->prev.byIndex + 1);
					}
					// same index of same tracks
					else {
						if (pSubQ->prev.byIndex == 0) {
							pSubQ->present.nRelativeTime = pSubQ->prev.nRelativeTime - 1;
						}
						else if (pSubQ->prev.byIndex > 0) {
							// EVE - burst error (Disc 3) (Terror Disc)
							// LBA[188021, 0x2DE75],  Data,      Copy NG,                  Track[01], Idx[01], RMSF[41:46:71], AMSF[41:48:71], RtoW[0, 0, 0, 0]
							// LBA[188022, 0x2DE76], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :72], RtoW[0, 0, 0, 0]
							// LBA[188023, 0x2DE77],  Data,      Copy NG,                  Track[01], Idx[01], RMSF[41:46:73], AMSF[41:48:73], RtoW[0, 0, 0, 0]
							pSubQ->present.nRelativeTime = pSubQ->prev.nRelativeTime + 1;
						}
						// pattern 2-2-1-2: not change track.
						pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
						// pattern 2-2-1-2-2: not change index.
						pSubQ->present.byIndex = pSubQ->prev.byIndex;
					}
				}
				// pattern 2-2-2: present sector is data.
				else if ((pSubQ->present.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					// 1st sector of tracks
					if (pSubQ->prev.byTrackNum > 0 &&
						nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum]) {
						pSubQ->present.nRelativeTime = 0;
						// pattern 2-2-2-1: change track.
						pSubQ->present.byTrackNum = (BYTE)(pSubQ->prev.byTrackNum + 1);
						// pattern 2-2-2-1-2: not change index.
						pSubQ->present.byIndex = pSubQ->prev.byIndex;
						OutputMainInfoLogA(
							"LBA[%06d, %#07x], Track[%02u]: The first sector of Track %d is replaced with EAN sector (in this case it's NOT the last sector of Track %d)\n",
							nLBA, nLBA, byCurrentTrackNum + 1, pSubQ->present.byTrackNum, pSubQ->prev.byTrackNum);
					}
					// 1st next index of same tracks
					else if (pSubQ->prev.byIndex == 0 && pSubQ->prev.nRelativeTime == 0) {
						// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
						// LBA[142873, 0x22E19], Data, Copy NG, Track[37], Idx[00], RMSF[00:00:00], AMSF[31:46:73], RtoW[0, 0, 0, 0]
						// LBA[142874, 0x22E1A], Data, Copy NG, MediaCatalogNumber [0000000000000], AMSF[     :74], RtoW[0, 0, 0, 0]
						// LBA[142875, 0x22E1B], Data, Copy NG, Track[37], Idx[01], RMSF[00:00:01], AMSF[31:47:00], RtoW[0, 0, 0, 0]
						pSubQ->present.nRelativeTime = 0;
						// pattern 2-2-2-2: not change track.
						pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
						// pattern 2-2-2-2-1: change index.
						pSubQ->present.byIndex = (BYTE)(pSubQ->prev.byIndex + 1);
					}
					// same index of same tracks
					else {
						if (pSubQ->prev.byIndex == 0) {
							pSubQ->present.nRelativeTime = pSubQ->prev.nRelativeTime - 1;
						}
						else if (pSubQ->prev.byIndex > 0) {
							// Cosmic Fantasy 3 - Bouken Shounen Rei (Japan)
							// LBA[174261, 0x2A8B5], Data, Copy NG, Track[60], Idx[01], RMSF[00:06:19], AMSF[38:45:36], RtoW[0, 0, 0, 0]
							// LBA[174262, 0x2A8B6], Data, Copy NG, MediaCatalogNumber [0000000000000], AMSF[     :37], RtoW[0, 0, 0, 0]
							// LBA[174263, 0x2A8B7], Data, Copy NG, Track[60], Idx[01], RMSF[00:06:21], AMSF[38:45:38], RtoW[0, 0, 0, 0]
							pSubQ->present.nRelativeTime = pSubQ->prev.nRelativeTime + 1;
						}
						// pattern 2-2-2-2: not change track.
						pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
						// pattern 2-2-2-2-2: not change index.
						pSubQ->present.byIndex = pSubQ->prev.byIndex;
					}
				}
			}
		}
	}
	return bRet;
}

BOOL IsValidSubQAdrIsrc(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpSubcode,
	PSUB_Q pSubQ,
	BYTE byCurrentTrackNum,
	INT nLBA
	)
{
	if (!pDisc->SUB.byISRC) {
		return FALSE;
	}
	INT session = pDisc->SCSI.lpSessionNumList[byCurrentTrackNum - 1];
	INT nRangeLBA = pDisc->SUB.nRangeLBAForISRC[0][session - 1];
	if (nRangeLBA == -1) {
		return FALSE;
	}
	INT nFirstLBA = pDisc->SUB.nFirstLBAForISRC[0][session - 1];
	INT nPrevISRCSector = pDisc->SUB.nPrevISRCSector;
	BOOL bRet = IsValidSubQAdrSector(pExtArg->dwSubAddionalNum, pSubQ->prev.byAdr,
		pSubQ->next.byAdr, nRangeLBA, nFirstLBA, nPrevISRCSector, nLBA);
	if (!bRet) {
		nRangeLBA = pDisc->SUB.nRangeLBAForISRC[1][session - 1];
		if (nRangeLBA != -1) {
			nFirstLBA = pDisc->SUB.nFirstLBAForISRC[1][session - 1];
			bRet = IsValidSubQAdrSector(pExtArg->dwSubAddionalNum, pSubQ->prev.byAdr,
				pSubQ->next.byAdr, nRangeLBA, nFirstLBA, nPrevISRCSector, nLBA);
		}
	}
	if (bRet) {
		if (pSubQ->present.byAdr != ADR_ENCODES_ISRC &&
			pSubQ->prev.byAdr != ADR_ENCODES_ISRC) {
			OutputSubErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: SubQ[12]:Adr[%d] -> [0x03]\n",
				nLBA, nLBA, byCurrentTrackNum, pSubQ->present.byAdr);
			pSubQ->present.byAdr = ADR_ENCODES_ISRC;
			lpSubcode[12] = (BYTE)(pSubQ->present.byCtl << 4 | pSubQ->present.byAdr);
		}
		if (pSubQ->present.byAdr == ADR_ENCODES_ISRC) {
			CHAR szISRC[META_ISRC_SIZE] = { 0 };
			BOOL bISRC = IsValidSubQISRC(lpSubcode);
			if (!bISRC && pExtArg->byISRC) {
				// force a invalid ISRC to valid ISRC
				bISRC = pExtArg->byISRC;
			}
			SetISRCToString(pDisc, lpSubcode, szISRC, byCurrentTrackNum, bISRC);
			pDisc->SUB.lpISRCList[byCurrentTrackNum - 1] = bISRC;

			if (strncmp(pDisc->SUB.pszISRC[byCurrentTrackNum - 1], szISRC, META_ISRC_SIZE) || !bISRC) {
				OutputSubErrorLogA(
					"LBA[%06d, %#07x], Track[%02u]: SubQ[13-20]:ISRC[%12s], SubQ[20]Lo:[%x] -> [%12s], SubQ[20]Lo:[0]\n",
					nLBA, nLBA, byCurrentTrackNum, szISRC, lpSubcode[20] & 0x0f, pDisc->SUB.pszISRC[byCurrentTrackNum - 1]);
					
				CHAR tmpISRC[META_ISRC_SIZE] = { 0 };
				strncpy(tmpISRC, pDisc->SUB.pszISRC[byCurrentTrackNum - 1], META_ISRC_SIZE);
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
			}
			bRet = TRUE;
		}
	}
	else {
		if (pSubQ->present.byAdr == ADR_ENCODES_ISRC) {
			CHAR szISRC[META_ISRC_SIZE] = { 0 };
			BOOL bISRC = IsValidSubQISRC(lpSubcode);
			if (!bISRC && pExtArg->byISRC) {
				// force a invalid ISRC to valid ISRC
				bISRC = pExtArg->byISRC;
			}
			SetISRCToString(pDisc, lpSubcode, szISRC, byCurrentTrackNum, bISRC);
			pDisc->SUB.lpISRCList[byCurrentTrackNum - 1] = bISRC;

			if (strncmp(pDisc->SUB.pszISRC[byCurrentTrackNum - 1], szISRC, META_ISRC_SIZE) || !bISRC) {
				OutputSubErrorLogA(
					"LBA[%06d, %#07x], Track[%02u]: SubQ[12]:Adr[%d] -> No ISRC frame\n",
					nLBA, nLBA, byCurrentTrackNum, pSubQ->present.byAdr);
				return FALSE;
			}
			INT nTmpFirstLBA = nLBA % pDisc->SUB.nRangeLBAForISRC[0][session - 1];
			OutputMainInfoLogA("LBA[%06d, %#07x], Track[%02u]: Range of ISRC is different [%d]\n",
				nLBA, nLBA, byCurrentTrackNum, nLBA - pDisc->SUB.nPrevISRCSector);
			pDisc->SUB.nFirstLBAForISRC[0][session - 1] = nTmpFirstLBA;
			bRet = TRUE;
		}
	}

	if (bRet) {
		pDisc->SUB.nPrevISRCSector = nLBA;
		// because tracknum, index... doesn't exist
		if (pSubQ->present.byAdr != ADR_ENCODES_ISRC) {
			pSubQ->present.byAdr = ADR_ENCODES_ISRC;
			lpSubcode[12] = (BYTE)(pSubQ->present.byCtl << 4 | pSubQ->present.byAdr);
		}
		pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
		pSubQ->present.byIndex = pSubQ->prev.byIndex;
		if (pSubQ->present.byIndex == 0) {
			pSubQ->present.nRelativeTime = pSubQ->prev.nRelativeTime - 1;
		}
		else if (pSubQ->present.byIndex > 0) {
			pSubQ->present.nRelativeTime = pSubQ->prev.nRelativeTime + 1;
		}
		if (!pDisc->SUB.lpISRCList[byCurrentTrackNum - 1]) {
			pDisc->SUB.lpISRCList[byCurrentTrackNum - 1] = TRUE;
		}
		if ((lpSubcode[16] & 0x03) != 0) {
			OutputSubErrorLogA("LBA[%06d, %#07x], Track[%02u]: SubQ[16]:[%x] -> [%x]\n",
				nLBA, nLBA, byCurrentTrackNum, lpSubcode[16], lpSubcode[16] & 0xfc);
			lpSubcode[16] &= 0xfc;
		}
		if ((lpSubcode[20] & 0x0f) != 0) {
			OutputSubErrorLogA("LBA[%06d, %#07x], Track[%02u]: SubQ[20]:[%x] -> [%x]\n",
				nLBA, nLBA, byCurrentTrackNum, lpSubcode[20], lpSubcode[20] & 0xf0);
			lpSubcode[20] &= 0xf0;
		}
	}
	return bRet;
}

VOID CheckAndFixSubQ(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpSubcode,
	PSUB_Q pSubQ,
	BYTE byCurrentTrackNum,
	INT nLBA,
	BOOL bLibCrypt
	)
{
	if (pSubQ->prevPrev.byTrackNum == 110 ||
		pSubQ->prev.byTrackNum == 110 ||
		pSubQ->present.byTrackNum == 110) {
		// skip lead-out
		if (nLBA > pDisc->SCSI.nAllLength - 10) {
			return;
		}
		else if (pDisc->SCSI.lpSessionNumList[byCurrentTrackNum] >= 2) {
			// Wild Romance [Kyosuke Himuro]
			// LBA[043934, 0x0ab9e], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[01], RMSF[04:19:39], AMSF[09:47:59], RtoW[0, 0, 0, 0]
			// LBA[055335, 0x0d827], Audio, 2ch, Copy NG, Pre-emphasis No, Track[110], Idx[01], RMSF[00:00:01], AMSF[09:47:61], RtoW[0, 0, 0, 0]
			// LBA[055336, 0x0d828],  Data,      Copy NG,                  Track[03], Idx[01], RMSF[00:00:01], AMSF[12:19:61], RtoW[0, 0, 0, 0]
			pSubQ->present.byCtl = AUDIO_DATA_TRACK;
			pSubQ->present.byTrackNum = (BYTE)(byCurrentTrackNum + 1);
			pSubQ->present.nRelativeTime = 0;
			return;
		}
	}

	BOOL bAdrCurrent = FALSE;
	if (1 <= pExtArg->dwSubAddionalNum) {
		// first check adr:2
		if (!IsValidSubQAdrMCN(pExtArg, pDisc, lpSubcode, pSubQ,
			byCurrentTrackNum, nLBA)) {
			// Next check adr:3
			if (!IsValidSubQAdrIsrc(pExtArg, pDisc, lpSubcode,
				pSubQ, byCurrentTrackNum, nLBA)) {
				bAdrCurrent = TRUE;
			}
		}
	}
	else {
		// If it doesn't read the next sector, adr premises that it isn't ADR_ENCODES_CURRENT_POSITION
		if (pSubQ->present.byAdr == ADR_ENCODES_ISRC) {
			// first check adr:3
			if (!IsValidSubQAdrIsrc(pExtArg, pDisc, lpSubcode,
				pSubQ, byCurrentTrackNum, nLBA)) {
				// Next check adr:2
				if (!IsValidSubQAdrMCN(pExtArg, pDisc, lpSubcode, pSubQ,
					byCurrentTrackNum, nLBA)) {
					bAdrCurrent = TRUE;
				}
			}
		}
		else if (pSubQ->present.byAdr != ADR_ENCODES_CURRENT_POSITION) {
			// first check adr:2
			if (!IsValidSubQAdrMCN(pExtArg, pDisc, lpSubcode, pSubQ,
				byCurrentTrackNum, nLBA)) {
				// Next check adr:3
				if (!IsValidSubQAdrIsrc(pExtArg, pDisc, lpSubcode,
					pSubQ, byCurrentTrackNum, nLBA)) {
					bAdrCurrent = TRUE;
				}
			}
		}
	}
	if (bAdrCurrent && pSubQ->present.byAdr != ADR_ENCODES_CURRENT_POSITION) {
		pSubQ->present.byAdr = ADR_ENCODES_CURRENT_POSITION;
		lpSubcode[12] = (BYTE)(pSubQ->present.byCtl << 4 | pSubQ->present.byAdr);
	}

	BYTE SubQcodeForLibCrypt[12] = { 0 };
	if (bLibCrypt) {
		OutputSubInfoLogA(
			"LBA[%06d, %#07x], Track[%02u]: Sub[12-23]: %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02x %02x\n",
			nLBA, nLBA, byCurrentTrackNum,
			BcdToDec(lpSubcode[12]), BcdToDec(lpSubcode[13]), BcdToDec(lpSubcode[14]), BcdToDec(lpSubcode[15]),
			BcdToDec(lpSubcode[16]), BcdToDec(lpSubcode[17]), BcdToDec(lpSubcode[18]), BcdToDec(lpSubcode[19]),
			BcdToDec(lpSubcode[20]), BcdToDec(lpSubcode[21]), BcdToDec(lpSubcode[22]), BcdToDec(lpSubcode[23]));
		memcpy(SubQcodeForLibCrypt, &lpSubcode[12], sizeof(SubQcodeForLibCrypt));
	}
	if (pSubQ->present.byAdr == ADR_ENCODES_CURRENT_POSITION) {
		BOOL bPrevTrackNum = TRUE;
		if (!IsValidSubQTrack(pExecType, pDisc, pSubQ, nLBA, byCurrentTrackNum, &bPrevTrackNum)) {
			OutputSubErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: SubQ[13]:TrackNum[%02u] -> [%02u], L:[%d]\n",
				nLBA, nLBA, byCurrentTrackNum, pSubQ->present.byTrackNum, pSubQ->prev.byTrackNum, s_lineNum);
			if (byCurrentTrackNum == pDisc->SCSI.toc.LastTrack) {
				pSubQ->present.byTrackNum = pDisc->SCSI.toc.LastTrack;
			}
			else if (pDisc->SCSI.lpFirstLBAListOnToc[byCurrentTrackNum] < nLBA) {
				pSubQ->present.byTrackNum = (BYTE)(pSubQ->prev.byTrackNum + 1);
			}
			else if (pSubQ->present.byIndex == 1 && pSubQ->present.nRelativeTime == 0) {
				// Bikkuriman Daijikai (Japan)
				// LBA[106402, 0x19FA2], Audio, 2ch, Copy NG, Pre-emphasis No, Track[70], Idx[01], RMSF[00:16:39], AMSF[23:40:52], RtoW[0, 0, 0, 0]
				// LBA[106403, 0x19FA3], Audio, 2ch, Copy NG, Pre-emphasis No, Track[79], Idx[01], RMSF[00:00:00], AMSF[21:40:53], RtoW[0, 0, 0, 0]
				// LBA[106404, 0x19FA4], Audio, 2ch, Copy NG, Pre-emphasis No, Track[71], Idx[01], RMSF[00:00:01], AMSF[23:40:54], RtoW[0, 0, 0, 0]
				pSubQ->present.byTrackNum = (BYTE)(pSubQ->prev.byTrackNum + 1);
			}
			else if (pSubQ->present.byIndex == 0 && IsValidPregapSector(pDisc, pSubQ, nLBA)) {
				// Network Q RAC Rally Championship (Netherlands)
				// LBA[202407, 0x316a7], Audio, 2ch, Copy NG, Pre-emphasis No, Track[13], Idx[01], RMSF[05:21:29], AMSF[45:00:57], RtoW[0, 0, 0, 0]
				// LBA[202408, 0x316a8], Audio, 2ch, Copy NG, Pre-emphasis No, Track[16], Idx[00], RMSF[00:01:74], AMSF[45:00:58], RtoW[0, 0, 0, 0]
				// LBA[202409, 0x316a9], Audio, 2ch, Copy NG, Pre-emphasis No, Track[14], Idx[00], RMSF[00:01:73], AMSF[45:00:59], RtoW[0, 0, 0, 0]
				pSubQ->present.byTrackNum = (BYTE)(pSubQ->prev.byTrackNum + 1);
			}
			else {
				pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
			}
			lpSubcode[13] = DecToBcd(pSubQ->present.byTrackNum);
		}
		else if (!bPrevTrackNum) {
			if (pSubQ->prev.byTrackNum > 0) {
				OutputSubErrorLogA(
					"LBA[%06d, %#07x], Track[%02u]: SubQ[13]:PrevTrackNum[%02u] -> [%02u]\n",
					nLBA, nLBA, byCurrentTrackNum, pSubQ->prev.byTrackNum, pSubQ->prevPrev.byTrackNum);
				pDisc->SUB.lpFirstLBAListOnSub[pSubQ->prev.byTrackNum - 1][pSubQ->prev.byIndex] = -1;
				pDisc->SUB.lpFirstLBAListOnSubSync[pSubQ->prev.byTrackNum - 1][pSubQ->prev.byIndex] = -1;
				pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[pSubQ->prev.byTrackNum - 1] = -1;
			}
			else {
				OutputSubErrorLogA("Internal error: PrevTrackNum is 0!\n");
			}
			pSubQ->prev.byTrackNum = pSubQ->prevPrev.byTrackNum;
			pSubQ->prev.byIndex = pSubQ->prevPrev.byIndex;
			pSubQ->prev.nRelativeTime = pSubQ->prevPrev.nRelativeTime + 1;
		}
		BOOL bPrevIndex = TRUE;
		BOOL bPrevPrevIndex = TRUE;
		if (!IsValidSubQIdx(pDisc, pSubQ, nLBA, byCurrentTrackNum, &bPrevIndex, &bPrevPrevIndex)) {
			OutputSubErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: SubQ[14]:Idx[%02u] -> [%02u], L:[%d]\n",
				nLBA, nLBA, byCurrentTrackNum, pSubQ->present.byIndex, pSubQ->prev.byIndex, s_lineNum);
			pSubQ->present.byIndex = pSubQ->prev.byIndex;
			lpSubcode[14] = DecToBcd(pSubQ->present.byIndex);
		}
		else if (!bPrevIndex) {
			if (pSubQ->prev.byTrackNum > 0) {
				OutputSubErrorLogA(
					"LBA[%06d, %#07x], Track[%02u]: SubQ[14]:PrevIdx[%02u] -> [%02u]\n",
					nLBA - 1, nLBA - 1, byCurrentTrackNum, pSubQ->prev.byIndex, pSubQ->prevPrev.byIndex);
				pDisc->SUB.lpFirstLBAListOnSub[pSubQ->prev.byTrackNum - 1][pSubQ->prev.byIndex] = -1;
				pDisc->SUB.lpFirstLBAListOnSubSync[pSubQ->prev.byTrackNum - 1][pSubQ->prev.byIndex] = -1;
			}
			else {
				OutputSubErrorLogA("Internal error: PrevTrackNum is 0!\n");
			}
			pSubQ->prev.byTrackNum = pSubQ->prevPrev.byTrackNum;
			pSubQ->prev.byIndex = pSubQ->prevPrev.byIndex;
		}
		else if (!bPrevPrevIndex) {
			if (pSubQ->prev.byTrackNum > 0) {
				OutputSubErrorLogA(
					"LBA[%06d, %#07x], Track[%02u]: SubQ[14]:PrevPrevIdx[%02u] -> [%02u]\n",
					nLBA - 1, nLBA - 1, byCurrentTrackNum, pSubQ->prevPrev.byIndex, pSubQ->prev.byIndex);
				pDisc->SUB.lpFirstLBAListOnSub[pSubQ->prevPrev.byTrackNum - 1][pSubQ->prevPrev.byIndex] = -1;
				pDisc->SUB.lpFirstLBAListOnSubSync[pSubQ->prevPrev.byTrackNum - 1][pSubQ->prevPrev.byIndex] = -1;
			}
			else {
				OutputSubErrorLogA("Internal error: PrevPrevTrackNum is 0!\n");
			}
			pSubQ->prevPrev.byTrackNum = pSubQ->prev.byTrackNum;
			pSubQ->prevPrev.byIndex = pSubQ->prev.byIndex;
		}

		if (!bLibCrypt && !IsValidSubQRMSF(pExecType, pSubQ, lpSubcode, nLBA)) {
			if (!(pSubQ->prev.byIndex == 0 && pSubQ->present.byIndex == 1) &&
				!(pSubQ->prev.byIndex >= 1 && pSubQ->present.byIndex == 0)) {
				BYTE byFrame = 0;
				BYTE bySecond = 0;
				BYTE byMinute = 0;
				INT tmpRel = 0;
				if (pSubQ->present.byIndex > 0) {
					if (pDisc->SCSI.lpFirstLBAListOnToc[byCurrentTrackNum] != nLBA) {
						tmpRel = pSubQ->prev.nRelativeTime + 1;
					}
				}
				else if (pSubQ->present.byIndex == 0) {
					tmpRel = pSubQ->prev.nRelativeTime - 1;
				}
				LBAtoMSF(tmpRel, &byMinute, &bySecond, &byFrame);
				BYTE byPrevFrame = 0;
				BYTE byPrevSecond = 0;
				BYTE byPrevMinute = 0;
				LBAtoMSF(pSubQ->prev.nRelativeTime, &byPrevMinute, &byPrevSecond, &byPrevFrame);
				OutputSubErrorLogA(
					"LBA[%06d, %#07x], Track[%02u]: SubQ[15-17]:PrevRel[%d, %02u:%02u:%02u], Rel[%d, %02u:%02u:%02u] -> [%d, %02u:%02u:%02u], L:[%d]\n",
					nLBA, nLBA, byCurrentTrackNum, pSubQ->prev.nRelativeTime,
					byPrevMinute, byPrevSecond, byPrevFrame, pSubQ->present.nRelativeTime,
					BcdToDec(lpSubcode[15]), BcdToDec(lpSubcode[16]), BcdToDec(lpSubcode[17]),
					tmpRel, byMinute, bySecond, byFrame, s_lineNum);
				BOOL b1 = FALSE;
#if 0
				BOOL b2 = FALSE;
#endif
				// A Bigger Bang (TOCP-66400)
				// LBA[310026, 0x4bb0a], Track[17], PrevRel[  8898], Rel[  8898], RMSF[01:58:48] -> [01:58:49]
				if (pSubQ->prev.nRelativeTime == pSubQ->present.nRelativeTime) {
					b1 = TRUE;
				}
				// A Bigger Bang (TOCP-66400)
				// LBA[332700, 0x5139c], Track[17], PrevRel[31571], Rel[149], RMSF[00:01:74] -> [07:00:72]
#if 0
				else if (pSubQ->present.nRelativeTime <= 149 &&
					pSubQ->present.nRelativeTime < pSubQ->prev.nRelativeTime) {
					b2 = TRUE;
					LBAtoMSF(pSubQ->present.nRelativeTime, &byMinute, &bySecond, &byFrame);
					//					pSubQ->present.byTrackNum = 18;
				}
				if (!b2) {
					pSubQ->present.nRelativeTime = tmpRel;
				}
#endif
				if (b1) {
					pSubQ->present.nRelativeTime--;
				}
				pSubQ->present.nRelativeTime = tmpRel;
				lpSubcode[15] = DecToBcd(byMinute);
				lpSubcode[16] = DecToBcd(bySecond);
				lpSubcode[17] = DecToBcd(byFrame);
			}
		}

		if (lpSubcode[18] != 0) {
			OutputSubErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: SubQ[18]:[%#04x] -> [0x00]\n",
				nLBA, nLBA, byCurrentTrackNum, lpSubcode[18]);
			lpSubcode[18] = 0;
		}

		if (!bLibCrypt && !IsValidSubQAMSF(pExecType,
			pDisc->SUB.byIndex0InTrack1, pSubQ, lpSubcode, nLBA)) {
			BYTE byFrame = 0;
			BYTE bySecond = 0;
			BYTE byMinute = 0;
			INT tmpAbs = nLBA + 150;
			LBAtoMSF(tmpAbs, &byMinute, &bySecond, &byFrame);
			BYTE byPrevFrame = 0;
			BYTE byPrevSecond = 0;
			BYTE byPrevMinute = 0;
			LBAtoMSF(pSubQ->prev.nAbsoluteTime, &byPrevMinute, &byPrevSecond, &byPrevFrame);
			OutputSubErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: SubQ[19-21]:PrevAbs[%d, %02u:%02u:%02u], Abs[%d, %02u:%02u:%02u] -> [%d, %02u:%02u:%02u]\n",
				nLBA, nLBA, byCurrentTrackNum, pSubQ->prev.nAbsoluteTime,
				byPrevMinute, byPrevSecond, byPrevFrame, pSubQ->present.nAbsoluteTime,
				BcdToDec(lpSubcode[19]), BcdToDec(lpSubcode[20]), BcdToDec(lpSubcode[21]),
				tmpAbs, byMinute, bySecond, byFrame);
			pSubQ->present.nAbsoluteTime = MSFtoLBA(byMinute, bySecond, byFrame) + 150;
			lpSubcode[19] = DecToBcd(byMinute);
			lpSubcode[20] = DecToBcd(bySecond);
			lpSubcode[21] = DecToBcd(byFrame);
		}
	}
	else if (nLBA >= 0 &&
		(pSubQ->present.byAdr == ADR_ENCODES_MEDIA_CATALOG ||
		pSubQ->present.byAdr == ADR_ENCODES_ISRC)) {
		if (!IsValidSubQAFrame(lpSubcode, nLBA)) {
			BYTE byFrame = 0;
			BYTE bySecond = 0;
			BYTE byMinute = 0;
			LBAtoMSF(nLBA + 150, &byMinute, &bySecond, &byFrame);
			BYTE byPrevFrame = 0;
			BYTE byPrevSecond = 0;
			BYTE byPrevMinute = 0;
			LBAtoMSF(pSubQ->prev.nAbsoluteTime, &byPrevMinute, &byPrevSecond, &byPrevFrame);
			OutputSubErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: SubQ[21]:PrevAbsFrame[%02u], AbsFrame[%02u] -> [%02u]\n",
				nLBA, nLBA, byCurrentTrackNum, byPrevFrame, BcdToDec(lpSubcode[21]), byFrame);
			pSubQ->present.nAbsoluteTime = MSFtoLBA(byMinute, bySecond, byFrame);
			lpSubcode[21] = DecToBcd(byFrame);
		}
	}
	if (!IsValidSubQCtl(pSubQ, 
		pDisc->SUB.lpEndCtlList[pSubQ->present.byTrackNum - 1])) {
		OutputSubErrorLogA(
			"LBA[%06d, %#07x], Track[%02u]: SubQ[12]:Ctl[%u] -> [%u], L:[%d]\n",
			nLBA, nLBA, byCurrentTrackNum, pSubQ->present.byCtl, pSubQ->prev.byCtl, s_lineNum);
		pSubQ->present.byCtl = pSubQ->prev.byCtl;
		lpSubcode[12] = (BYTE)(pSubQ->present.byCtl << 4 | pSubQ->present.byAdr);
	}
	WORD crc16 = (WORD)GetCrc16CCITT(10, &lpSubcode[12]);
	if (bLibCrypt) {
		// lpSubcode is fixed (= original crc)
		WORD xorCrc16 = (WORD)(crc16 ^ 0x8001);
		BYTE tmp3 = HIBYTE(xorCrc16);
		BYTE tmp4 = LOBYTE(xorCrc16);
		if (SubQcodeForLibCrypt[10] == tmp3 &&
			SubQcodeForLibCrypt[11] == tmp4) {
			OutputSubInfoLogA(
				"LBA[%06d, %#07x], Track[%02u]: LibCrypt exists, CRC-16 is original:[%04x] and XORed with 0x8001:[%04x]\n",
				nLBA, nLBA, byCurrentTrackNum, crc16, xorCrc16);
		}
		else {
			// lpSubcode isn't fixed (= recalc crc)
			WORD reCalcCrc16 = (WORD)GetCrc16CCITT(10, &SubQcodeForLibCrypt[0]);
			WORD reCalcXorCrc16 = (WORD)(reCalcCrc16 ^ 0x0080);
			BYTE tmp5 = HIBYTE(reCalcXorCrc16);
			BYTE tmp6 = LOBYTE(reCalcXorCrc16);
			if (SubQcodeForLibCrypt[10] == tmp5 &&
				SubQcodeForLibCrypt[11] == tmp6) {
				OutputSubInfoLogA(
					"LBA[%06d, %#07x], Track[%02u]: LibCrypt exists, CRC-16 is recalculated:[%04x] and XORed with 0x0080:[%04x]\n",
					nLBA, nLBA, byCurrentTrackNum, reCalcCrc16, reCalcXorCrc16);
			}
			else {
				OutputSubInfoLogA(
					"LBA[%06d, %#07x], Track[%02u]: LibCrypt doesn't exist\n",
					nLBA, nLBA, byCurrentTrackNum);
			}
		}
		lpSubcode[22] = SubQcodeForLibCrypt[10];
		lpSubcode[23] = SubQcodeForLibCrypt[11];
	}
	else {
		BYTE tmp1 = HIBYTE(crc16);
		BYTE tmp2 = LOBYTE(crc16);
		if (lpSubcode[22] != tmp1) {
			OutputSubErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: SubQ[22]:CrcHigh[%#04x] -> [%#04x]\n",
				nLBA, nLBA, byCurrentTrackNum, lpSubcode[22], tmp1);
			lpSubcode[22] = tmp1;
		}
		if (lpSubcode[23] != tmp2) {
			OutputSubErrorLogA(
				"LBA[%06d, %#07x], Track[%02u]: SubQ[23]:CrcLow[%#04x] -> [%#04x]\n",
				nLBA, nLBA, byCurrentTrackNum, lpSubcode[23], tmp2);
			lpSubcode[23] = tmp2;
		}
	}
	return;
}

VOID CheckAndFixSubRtoW(
	PDISC pDisc,
	LPBYTE lpSubcode,
	BYTE byCurrentTrackNum,
	INT nLBA
	)
{
	if (pDisc->SUB.lpRtoWList[byCurrentTrackNum - 1] != SUB_RTOW_TYPE::CDG &&
		pDisc->SUB.lpRtoWList[byCurrentTrackNum - 1] != SUB_RTOW_TYPE::Full) {
		for (INT j = 24; j < CD_RAW_READ_SUBCODE_SIZE; j++) {
			if (lpSubcode[j] != 0) {
				if (24 <= j && j < 36 && 
					(pDisc->SUB.lpRtoWList[byCurrentTrackNum - 1] & SUB_RTOW_TYPE::RFull) != SUB_RTOW_TYPE::RFull) {
					OutputSubErrorLogA(
						"LBA[%06d, %#07x], Track[%02u]: SubR[%02d]:[%#04x] -> [0x00]\n",
						nLBA, nLBA, byCurrentTrackNum, j, lpSubcode[j]);
					lpSubcode[j] = 0;
				}
				else if (36 <= j && j < 48 &&
					(pDisc->SUB.lpRtoWList[byCurrentTrackNum - 1] & SUB_RTOW_TYPE::SFull) != SUB_RTOW_TYPE::SFull) {
					OutputSubErrorLogA(
						"LBA[%06d, %#07x], Track[%02u]: SubS[%02d]:[%#04x] -> [0x00]\n",
						nLBA, nLBA, byCurrentTrackNum, j, lpSubcode[j]);
					lpSubcode[j] = 0;
				}
				else if (48 <= j && j < 60 &&
					(pDisc->SUB.lpRtoWList[byCurrentTrackNum - 1] & SUB_RTOW_TYPE::TFull) != SUB_RTOW_TYPE::TFull) {
					OutputSubErrorLogA(
						"LBA[%06d, %#07x], Track[%02u]: SubT[%02d]:[%#04x] -> [0x00]\n",
						nLBA, nLBA, byCurrentTrackNum, j, lpSubcode[j]);
					lpSubcode[j] = 0;
				}
				else if (60 <= j && j < 72 &&
					(pDisc->SUB.lpRtoWList[byCurrentTrackNum - 1] & SUB_RTOW_TYPE::UFull) != SUB_RTOW_TYPE::UFull) {
					OutputSubErrorLogA(
						"LBA[%06d, %#07x], Track[%02u]: SubU[%02d]:[%#04x] -> [0x00]\n",
						nLBA, nLBA, byCurrentTrackNum, j, lpSubcode[j]);
					lpSubcode[j] = 0;
				}
				else if (72 <= j && j < 84 &&
					(pDisc->SUB.lpRtoWList[byCurrentTrackNum - 1] & SUB_RTOW_TYPE::VFull) != SUB_RTOW_TYPE::VFull) {
					OutputSubErrorLogA(
						"LBA[%06d, %#07x], Track[%02u]: SubV[%02d]:[%#04x] -> [0x00]\n",
						nLBA, nLBA, byCurrentTrackNum, j, lpSubcode[j]);
					lpSubcode[j] = 0;
				}
				else if (84 <= j && j < CD_RAW_READ_SUBCODE_SIZE &&
					(pDisc->SUB.lpRtoWList[byCurrentTrackNum - 1] & SUB_RTOW_TYPE::WFull) != SUB_RTOW_TYPE::WFull) {
					OutputSubErrorLogA(
						"LBA[%06d, %#07x], Track[%02u]: SubW[%02d]:[%#04x] -> [0x00]\n",
						nLBA, nLBA, byCurrentTrackNum, j, lpSubcode[j]);
					lpSubcode[j] = 0;
				}
			}
		}
	}
	return;
}

VOID CheckAndFixSubChannel(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpSubcode,
	PSUB_Q pSubQ,
	BYTE byCurrentTrackNum,
	INT nLBA,
	BOOL bLibCrypt
	)
{
	CheckAndFixSubP(lpSubcode, byCurrentTrackNum, nLBA);
	CheckAndFixSubQ(pExecType, pExtArg, pDisc, lpSubcode, pSubQ,
		byCurrentTrackNum, nLBA, bLibCrypt);
	CheckAndFixSubRtoW(pDisc, lpSubcode, byCurrentTrackNum, nLBA);
	return;
}

BOOL ContainsDiffByte(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	LPBYTE lpBuf,
	UINT i
	)
{
	BOOL bByteError = FALSE;
	for (INT j = 0; j < CD_RAW_SECTOR_SIZE; j++) {
		if (pC2ErrorPerSector[i].lpBufNoC2SectorBackup[j] != lpBuf[j]) {
			bByteError = TRUE;
			break;
		}
	}
	return bByteError;
}

BOOL ContainsC2Error(
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE lpBuf,
	UINT uiC2ErrorLBACnt
	)
{
	BOOL bRet = RETURNED_NO_C2_ERROR_1ST;
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
					(SHORT)(wC2ErrorPos * CHAR_BIT + n + pDisc->MAIN.nCombinedOffset);
				pC2ErrorPerSector[uiC2ErrorLBACnt].uiErrorBytePosCnt++;
				if (pC2ErrorPerSector[uiC2ErrorLBACnt].uiErrorBytePosCnt == 1) {
					bRet = RETURNED_EXIST_C2_ERROR;
				}
			}
//			nBit <<= 1;
			nBit >>= 1;
		}
	}
	return bRet;
}

BOOL SupportIndex0InTrack1(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	)
{
	BOOL bRet = TRUE;
	if (pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PX760A ||
		pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PX755A ||
		pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PX716AL ||
		pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PX716A ||
		pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PX714A ||
		pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PX712A ||
		pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PX708A2 ||
		pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PX708A ||
		pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PX704A ||
		pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PREMIUM2 ||
		pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PREMIUM ||
		pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PXW5224A) {
		pDisc->SUB.byIndex0InTrack1 = TRUE;
	}
	else {
		OutputString(
			_T("This drive doesn't support to rip from 00:00:00 to 00:01:74 AMSF. /p option is ignored\n"));
		pExtArg->byPre = FALSE;
	}
	return bRet;
}
