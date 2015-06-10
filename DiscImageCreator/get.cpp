/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"

#define CRCPOLY1  0x1021U  /* x^{16}+x^{12}+x^5+1 */
// init val = 0, output xor = 0xFFFF, shift left
USHORT GetCrc16CCITT(
	UCHAR c[],
	INT n
	)
{
	USHORT r = 0;
	for(INT i = 0; i < n; i++) {
		r ^= (USHORT)c[i] << (16 - CHAR_BIT);
		for(INT j = 0; j < CHAR_BIT; j++) {
			if(r & 0x8000U) {
				r = (r << 1) ^ CRCPOLY1;
			}
			else {
				r <<= 1;
			}
		}
	}
	return ~r & 0xFFFFU;
}

BOOL GetDriveOffset(
	LPCSTR pszProductId,
	PINT nDriveOffset
	)
{
	BOOL bGetOffset = FALSE;
	FILE* fpDrive = OpenProgrammabledFile(_T("driveOffset.txt"), _T("r"));
	if (!fpDrive) {
		OutputErrorString(_T("Failed to open file [F:%s][L:%d]:%s"), 
			_T(__FUNCTION__), __LINE__, _T("driveOffset.txt\n"));
		return FALSE;
	}

	CHAR szProduct[16+1] = {0};
	for(INT src = 0, dst = 0; dst < sizeof(szProduct) - 1; dst++) {
		if(pszProductId[dst] == ' ' && (pszProductId[dst+1] == ' ' || 
			pszProductId[dst+1] == '\0')) {
			continue;
		}
		szProduct[src++] = pszProductId[dst];
	}

	PCHAR trimId[5] = {0};
	PCHAR id = NULL;
	trimId[0] = strtok(szProduct, " ");
	// get model pszString (ex. PX-755A)
	for(INT nRoop = 1; nRoop < 5; nRoop++) {
		trimId[nRoop] = strtok(NULL, " ");
		if(trimId[nRoop] != NULL) {
			id = trimId[nRoop];
		}
		else {
			if(trimId[1] == NULL) {
				id = trimId[0];
			}
			break;
		}
	}
	if(id != NULL) {
		PCHAR tp[10] = {0};
		CHAR buf[1024] = {0};

		while((fgets(buf, sizeof(buf), fpDrive)) != NULL) {
			tp[0] = strtok(buf, " 	"); // space & tab
			for(INT nRoop = 1; nRoop < 10; nRoop++) {
				tp[nRoop] = strtok(NULL, " 	"); // space & tab
			}
			if(*tp[0] == '\n' || (*tp[1] != '-' && *tp[2] != '-')) {
				continue;
			}
			for(INT nRoop = 0; nRoop < 10 && tp[nRoop] != NULL; nRoop++) {
				if(strstr(tp[nRoop], id) != NULL) {
					*nDriveOffset = atoi(tp[nRoop+1]);
					bGetOffset = TRUE;
					break;
				}
			}
			if(bGetOffset) {
				break;
			}
		}
	}
	fclose(fpDrive);
	return bGetOffset;
}

ULONG GetFilesize(
	LONG nOffset,
	FILE *fp
	)
{
	ULONG filesize = 0;
	if (fp != NULL) {
		fseek(fp, 0, SEEK_END);
		filesize = (ULONG)ftell(fp);
		fseek(fp, nOffset, SEEK_SET);
	}

	return filesize;
}

UCHAR GetMode(
	CONST PUCHAR pBuf
	)
{
	UCHAR byMode = DATA_BLOCK_MODE0;
	if(IsValidDataHeader(pBuf)) {
		if((pBuf[15] & 0x60) == 0x60) {
			byMode = BcdToDec((UCHAR)(pBuf[15] ^ 0x60));
		}
		else {
			byMode = BcdToDec(pBuf[15]);
		}
	}
	return byMode;
}

BOOL GetWriteOffset(
	PDISC_DATA pDiscData,
	CONST PUCHAR pBuf
	)
{
	BOOL bRet = FALSE;
	for(INT i = 0; i < CD_RAW_SECTOR_SIZE * 2; i++) {
		if(IsValidDataHeader(pBuf + i)) {
			UCHAR sm = BcdToDec((UCHAR)(pBuf[i+12] ^ 0x01));
			UCHAR ss = BcdToDec((UCHAR)(pBuf[i+13] ^ 0x80));
			UCHAR sf = BcdToDec((UCHAR)(pBuf[i+14]));
			INT tmpLBA = MSFtoLBA(sf, ss, sm) - 150;
			pDiscData->nCombinedOffset = 
				CD_RAW_SECTOR_SIZE * -(tmpLBA - pDiscData->nFirstDataLBA) + i;
			bRet = TRUE;
			break;
		}
	}
	return bRet;
}
