/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"

BOOL IsValidAbsoluteTime(
	CONST SUB_Q_DATA* prevSubQ,
	CONST SUB_Q_DATA* subQ,
	CONST PUCHAR Subcode,
	INT nLBA
	)
{
	BOOL bRet = TRUE;
	INT tmpLBA = MSFtoLBA(BcdToDec(Subcode[21]), 
		BcdToDec(Subcode[20]), BcdToDec(Subcode[19])) - 150;
	if(nLBA != tmpLBA ||
		prevSubQ->nAbsoluteTime + 1 != subQ->nAbsoluteTime) {
		bRet = FALSE;
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