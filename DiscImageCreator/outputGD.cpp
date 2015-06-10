/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"

BOOL DescrambleDataSector(
	LPCTSTR pszInFilename,
	INT nLBA
	)
{
	FILE* fpScr = CreateOrOpenFile(pszInFilename, NULL, NULL, _T(".bin"), _T("rb"), 0, 0);
	if (!fpScr) {
		OutputErrorString("Failed to open file .bin [F:%s][L:%d]\n", 
			__FUNCTION__, __LINE__);
		return FALSE;
	}
	FILE* fpDescr = NULL;
	FILE* fptxt = NULL;
	FILE* fpTbl = NULL;
	BOOL bRet = TRUE;
	try {
		if(NULL == (fpDescr = CreateOrOpenFile(pszInFilename, NULL, NULL, _T(".dec"), _T("wb"), 0, 0))) {
			throw "Failed to open .dec\n";
		}
		if(NULL == (fptxt = CreateOrOpenFile(pszInFilename, NULL, NULL, _T(".offset.txt"), _T("w"), 0, 0))) {
			throw "Failed to open .offset.txt\n";
		}
		if(NULL == (fpTbl = OpenProgrammabledFile(_T("scramble.bin"), _T("rb")))) {
			throw "Failed to open scramble.bin\n";
		}
		ULONG ulFilesize = GetFilesize(fpScr, 0);
		UCHAR bufScr[CD_RAW_SECTOR_SIZE];
		ZeroMemory(bufScr, sizeof(bufScr));
		INT nOffset = 0;
		BOOL bRet2 = FALSE;
		for(ULONG i = 0; i < CD_RAW_SECTOR_SIZE; i++) {
			fread(bufScr, sizeof(UCHAR), SYNC_SIZE, fpScr);
			if(IsValidDataHeader(bufScr)) {
				bRet2 = TRUE;
				break;
			}
			else {
				nOffset++;
				fseek(fpScr, -11, SEEK_CUR);
			}
		}
		if(!bRet2) {
			throw "Not GD-ROM File\n";
		}
		// skip to 10:02:00(11:82:00)
		fread(bufScr + SYNC_SIZE, sizeof(UCHAR), 3, fpScr);
		LONG lba1 = MSFtoLBA(00, 02, 10);
		LONG lba2 = MSFtoLBA(BcdToDec(bufScr[0x0E]), 
			BcdToDec((UCHAR)(bufScr[0x0D] ^ 0x80)), 
			BcdToDec((UCHAR)(bufScr[0x0C] ^ 0x01)));
		INT nVal = lba2 - 150 - nLBA;
		INT nCombinedOffset = 
			nVal > 0 ? -CD_RAW_SECTOR_SIZE * nVal + nOffset : nOffset;
		OutputLogString(fptxt, "Combined offset(byte):%d, (sample):%d\n",
			nCombinedOffset, nCombinedOffset / 4);
		FcloseAndNull(fptxt);

		UCHAR bufTbl[CD_RAW_SECTOR_SIZE];
		fread(bufTbl, sizeof(UCHAR), CD_RAW_SECTOR_SIZE, fpTbl);
		FcloseAndNull(fpTbl);

		LONG lSeekSize = CD_RAW_SECTOR_SIZE * (lba1 - lba2) + nOffset;
		if(ulFilesize < (ULONG)lSeekSize) {
			OutputErrorString("Out of range! SeekSize:%d, FileSize:%d\n", lSeekSize, ulFilesize);
			return FALSE;
		}
		fseek(fpScr, lSeekSize, SEEK_SET);

		UCHAR bufDescr[CD_RAW_SECTOR_SIZE];
		ZeroMemory(bufDescr, sizeof(bufDescr));
		ULONG filesize = GetFilesize(fpScr, lSeekSize);
		ULONG ulDecFilesize = filesize - lSeekSize;
		ULONG ulRoop = ulDecFilesize / CD_RAW_SECTOR_SIZE;
		for(ULONG i = 1; i <= ulRoop; i++) {
			fread(bufScr, sizeof(UCHAR), CD_RAW_SECTOR_SIZE, fpScr);
			if(IsValidDataHeader(bufScr)) {
				if(bufScr[0x0C] == 0xC3 && bufScr[0x0D] == 0x84 && bufScr[0x0E] >= 0x00) {
					break;
				}
				for(INT j = 0; j < CD_RAW_SECTOR_SIZE; j++) {
					bufDescr[j] = (UCHAR)(bufScr[j] ^ bufTbl[j]);
				}
				fwrite(bufDescr, sizeof(UCHAR), CD_RAW_SECTOR_SIZE, fpDescr);
			}
			else {
				// copy audio data
				fwrite(bufScr, sizeof(UCHAR), CD_RAW_SECTOR_SIZE, fpDescr);
			}
			printf("\rDescrambling File(size) %10d/%10d", CD_RAW_SECTOR_SIZE * i, ulDecFilesize);
		}
		printf("\n");
	}
	catch(PCHAR str) {
		OutputErrorString(str);
	}
	FcloseAndNull(fpScr);
	FcloseAndNull(fpDescr);
	FcloseAndNull(fptxt);
	FcloseAndNull(fpTbl);
	return bRet;
}

BOOL SplitDescrambledFile(
	LPCTSTR pszInFilename
	)
{
	FILE* fpDescr = CreateOrOpenFile(pszInFilename, NULL, NULL, _T(".dec"), _T("rb"), 0, 0);
	if(!fpDescr) {
	OutputErrorString("Failed to open file .dec [F:%s][L:%d]\n", 
			__FUNCTION__, __LINE__);
		return FALSE;
	}
	FILE* fptxt = NULL;
#if 0
	FILE* fpCcd = NULL;
#endif
	PLONG pToc = NULL;
	FILE* fpBin = NULL;
	PUCHAR pBuf = NULL;
	BOOL bRet = TRUE;
	try {
		if(NULL == (fptxt = CreateOrOpenFile(pszInFilename, NULL, NULL, _T(".toc.txt"), _T("w"), 0, 0))) {
			throw "Failed to open .toc.txt\n";
		}
#if 0
		if(NULL == (fpCcd = CreateOrOpenFile(pszInFilename, NULL, NULL, _T(".ccd"), _T("w"), 0, 0))) {
			throw "Failed to open .ccd\n";
		}
#endif

		ULONG ulFilesize = GetFilesize(fpDescr, 0);
		if(ulFilesize < 0x110 + 512) {
			throw "Not GD-ROM data\n";
		}
		fseek(fpDescr, 0x110, SEEK_SET);
		// 0x110 - 0x31F is toc data
		UCHAR byToc[512];
		fread(byToc, sizeof(UCHAR), sizeof(byToc), fpDescr);
		if(byToc[0] != 'T' || byToc[1] != 'O' || byToc[2] != 'C' || byToc[3] != '1') {
			throw "Not GD-ROM data\n";
		}
		INT nTrackNum = 3;
#if 0
		fprintf(fpCcd, "[CloneCD]\n");
		fprintf(fpCcd, "Version=3\n");
		fprintf(fpCcd, "[Disc]\n");
		fprintf(fpCcd, "TocEntries=%d\n", nTrackNum + 6);
		fprintf(fpCcd, "Sessions=2\n");
		fprintf(fpCcd, "DataTracksScrambled=0\n");
		fprintf(fpCcd, "CDTextLength=0\n");
		fprintf(fpCcd, "[Session 1]\n");
		fprintf(fpCcd, "PreGapMode=0\n");
		fprintf(fpCcd, "PreGapSubC=0\n");
		fprintf(fpCcd, "[Session 2]\n");
		fprintf(fpCcd, "PreGapMode=0\n");
		fprintf(fpCcd, "PreGapSubC=0\n");
		//Entry 0 = SessionNum 1, FirstTrackNum
		//Entry 1 = SessionNum 1, LastTrackNum
		//Entry 2 = SessionNum 1, Leadout MSF
		//Entry 3 = SessionNum 1, TrackNum 1
		//Entry 4 = SessionNum 1, TrackNum 2
		//Entry 5 = SessionNum 2, FirstTrackNum
		//Entry 6 = SessionNum 2, LastTrackNum
		//Entry 7 = SessionNum 2, Leadout MSF
		//Entry 8 = SessionNum 2, TrackNum 1
		//					:
		//					:
		//					:
		INT aSession[] = {1, 1, 1, 1, 1, 2, 2, 2};
		INT aPoint[] = {0xa0, 0xa1, 0xa2, 0x01, 0x02, 0xa0, 0xa1, 0xa2};
		INT aControl[] = {0x04, 0x00, 0x00, 0x04, 0x00, 0x04, 0x04, 0x04};
		INT aPMin[] = {1, 2, 0, 0, 0, 3, nTrackNum, 122};
		INT aPSec[] = {0, 0, 0xffffffff, 2, 0xffffffff, 0, 0, 4};
		INT aPFrame[] = {0, 0, 0xffffffff, 0, 0xffffffff, 0, 0, 0};
		INT aPLBA[] = {4350, 8850, 0xffffffff, 0, 0xffffffff, 62850, 62850, 1650};
		for(INT c = 0; c < 8; c++) {
			fprintf(fpCcd, "[Entry %d]\n", c);
			fprintf(fpCcd, "Session=%d\n", aSession[c]);
			fprintf(fpCcd, "Point=0x%02x\n", aPoint[c]);
			fprintf(fpCcd, "ADR=0x01\n");
			fprintf(fpCcd, "Control=0x%02x\n", aControl[c]);
			fprintf(fpCcd, "TrackNo=0\n");
			fprintf(fpCcd, "AMin=0\n");
			fprintf(fpCcd, "ASec=0\n");
			fprintf(fpCcd, "AFrame=0\n");
			fprintf(fpCcd, "ALBA=-150\n"); 
			fprintf(fpCcd, "Zero=0\n");
			fprintf(fpCcd, "PMin=%d\n", aPMin[c]);
			fprintf(fpCcd, "PSec=%d\n", aPSec[c]);
			fprintf(fpCcd, "PFrame=%d\n", aPFrame[c]);
			fprintf(fpCcd, "PLBA=%d\n", aPLBA[c]);
		}
#endif
		INT nMaxToc = 98 * 4;
		INT nMaxTrackNum = byToc[nMaxToc+4*1+2];
		pToc = (PLONG)malloc(nMaxTrackNum * sizeof(LONG));
		if(!pToc) {
			throw "Failed to alloc memory to toc";
		}

		for(INT i = 0; i < nMaxToc; i += 4, nTrackNum++) {
			if(byToc[7+i] == 0xFF) {
				break;
			}
			UCHAR byCtl = (UCHAR)((byToc[7+i] >> 4) & 0x0F);
			LONG lToc = 
				MAKELONG(MAKEWORD(byToc[4+i], byToc[5+i]), MAKEWORD(byToc[6+i], 0));
#if 0
			UCHAR byAdr = (UCHAR)(byToc[7+i] & 0x0F);
			fprintf(fpCcd, "[Entry %d]\n", nTrackNum + 5);
			fprintf(fpCcd, "Session=2\n");
			fprintf(fpCcd, "Point=0x%02x\n", nTrackNum);
			fprintf(fpCcd, "ADR=0x%02x\n", byAdr);
			fprintf(fpCcd, "Control=0x%02x\n", byCtl);
			fprintf(fpCcd, "TrackNo=0\n");
			fprintf(fpCcd, "AMin=0\n");
			fprintf(fpCcd, "ASec=0\n");
			fprintf(fpCcd, "AFrame=0\n");
			fprintf(fpCcd, "ALBA=-150\n"); 
			fprintf(fpCcd, "Zero=0\n");
			UCHAR byFrame = 0;
			UCHAR bySecond = 0;
			UCHAR byMinute = 0;
			LBAtoMSF(lToc, &byFrame, &bySecond, &byMinute);
			fprintf(fpCcd, "PMin=%d\n", byMinute);
			fprintf(fpCcd, "PSec=%d\n", bySecond);
			fprintf(fpCcd, "PFrame=%d\n", byFrame);
			fprintf(fpCcd, "PLBA=%d\n", lToc);
#endif
			pToc[nTrackNum-3] = lToc - 300;
			if(nTrackNum == 3) {
				pToc[nTrackNum-3] += 150;
				OutputLogString(fptxt, 
					"Track %2d, Ctl %d,              , Index1 %6d\n", 
					nTrackNum, byCtl, pToc[nTrackNum-3]);
			}
			else {
				if((byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					pToc[nTrackNum-3] -= 75;
					OutputLogString(fptxt, 
						"Track %2d, Ctl %d, Index0 %6d, Index1 %6d\n", 
						nTrackNum, byCtl, pToc[nTrackNum-3], pToc[nTrackNum-3] + 225);
				}
				else {
					OutputLogString(fptxt, 
						"Track %2d, Ctl %d, Index0 %6d, Index1 %6d\n", 
						nTrackNum, byCtl, pToc[nTrackNum-3], pToc[nTrackNum-3] + 150);
				}
			}
		}
		OutputLogString(fptxt, "MaxTrackNum %2d\n", nMaxTrackNum);
		LONG lToc = 
			MAKELONG(MAKEWORD(byToc[nMaxToc+4*2], byToc[nMaxToc+4*2+1]), 
			MAKEWORD(byToc[nMaxToc+4*2+2], 0)) - 150;
		OutputLogString(fptxt, "Leadout, LBA %6d\n", lToc);
		pToc[nTrackNum-3] = lToc;

		if(nMaxTrackNum > 3) {
			rewind(fpDescr);
			for(INT i = 3; i <= nMaxTrackNum; i++) {
				printf("\rSplit File(num) %2d/%2d", i, nMaxTrackNum);
				if(NULL == (fpBin = CreateOrOpenFile(pszInFilename, NULL,
					NULL, _T(".bin"), _T("wb"), i, nMaxTrackNum))) {
					throw "Failed to open .bin";
				}
				size_t size = 
					(size_t)(pToc[i-2] - pToc[i-3]) * CD_RAW_SECTOR_SIZE;
				if(NULL == (pBuf = (PUCHAR)malloc(size))) {
					throw "Failed alloc memory .bin";
				}
				fread(pBuf, sizeof(UCHAR), size, fpDescr);
				fwrite(pBuf, sizeof(UCHAR), size, fpBin);
				FcloseAndNull(fpBin);
				FreeAndNull(pBuf);
			}
		}
	}
	catch(PCHAR str) {
		OutputErrorString(str);
		bRet = FALSE;
	}
	FcloseAndNull(fpDescr);
	FcloseAndNull(fptxt);
#if 0
	FcloseAndNull(fpCcd);
#endif
	FreeAndNull(pToc);
	FcloseAndNull(fpBin);
	return bRet;
}
