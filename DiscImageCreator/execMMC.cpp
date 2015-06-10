/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"

CDROM_TOC toc = {0};
// store start & end LBA
INT aTocLBA[MAXIMUM_NUMBER_TRACKS][2];
INT aSessionNum[MAXIMUM_NUMBER_TRACKS];
INT nFirstDataLBA = 0;
INT nLastLBAof1stSession = -1;
INT nStartLBAof2ndSession = -1;
INT nAdjustSectorNum = 0;

inline PVOID ConvParagraphBoundary(
	PVOID pv
	)
{
	return ((PVOID)(((ULONG_PTR)pv + 0x10) & 0xFFFFFFF0));
}

BOOL ExecCommand(
	HANDLE hDevice,
	PUCHAR pbyCdbCmd,
	UCHAR byCdbCmdLength,
	PVOID pvBuffer,
	ULONG dwBufferLength,
	PUCHAR byScsiStatus,
	LPSTR pszFuncname,
	INT nLineNum
	)
{
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER swb;
	ZeroMemory(&swb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));
	swb.ScsiPassThroughDirect.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
	swb.ScsiPassThroughDirect.CdbLength = byCdbCmdLength;
	swb.ScsiPassThroughDirect.SenseInfoLength = SPTWB_SENSE_LENGTH;
	swb.ScsiPassThroughDirect.DataIn = SCSI_IOCTL_DATA_IN;
	swb.ScsiPassThroughDirect.DataTransferLength = dwBufferLength;
	swb.ScsiPassThroughDirect.TimeOutValue = 10;
	swb.ScsiPassThroughDirect.DataBuffer = pvBuffer;
	swb.ScsiPassThroughDirect.SenseInfoOffset = 
		offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, SenseInfoBuffer);
	memcpy(swb.ScsiPassThroughDirect.Cdb, pbyCdbCmd, byCdbCmdLength);
	ULONG ulLength = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);

	ULONG ulReturned = 0;
	BOOL bRet = DeviceIoControl(hDevice, IOCTL_SCSI_PASS_THROUGH_DIRECT, &swb, 
		ulLength, &swb, ulLength, &ulReturned, NULL);
	*byScsiStatus = swb.ScsiPassThroughDirect.ScsiStatus;
	if(bRet) {
		OutputScsiStatus(&swb, byScsiStatus, pszFuncname, nLineNum);
	}
	else {
		OutputErrorStringA(
			"\nDeviceIoControl with SCSI_PASS_THROUGH_DIRECT command failed [F:%s][L:%d]\n", 
			pszFuncname, nLineNum);
	}
	return bRet;
}

void Init(
	void
	)
{
	ZeroMemory(aTocLBA, sizeof(aTocLBA));
	ZeroMemory(aSessionNum, sizeof(aSessionNum));
	InitForOutput();
}

BOOL ReadCDAll(
	HANDLE hDevice,
	LPCTSTR pszOutFile,
	LPCSTR pszVendorId,
	INT nWriteOffset,
	INT nLength,
	BOOL bC2ErrorData,
	FILE* fpLog,
	FILE* fpCcd
	)
{
	if(toc.FirstTrack < 1 || 99 < toc.FirstTrack ||
		toc.LastTrack < 1 || 99 < toc.LastTrack) {
		return FALSE;
	}
	FILE* fpImg = NULL;
	if(NULL == (fpImg = CreateOrOpenFileW(pszOutFile, NULL, NULL, _T(".img"), _T("wb"), 0, 0))) {
		return FALSE;
	}

	FILE* fpImg2 = NULL;
	FILE* fpBin = NULL;
	FILE* fpSub = NULL;
	FILE* fpCue = NULL;
	FILE* fpParse = NULL;
	FILE* fpC2 = NULL;
	FILE* fpTbl = NULL;
	PINT* aLBAStart = NULL;
	PINT* aLBAOfDataTrack = NULL;
	PUCHAR aCtl = NULL;
	PUCHAR aEndCtl = NULL;
	PUCHAR aMode = NULL;
	PBOOL aISRC = NULL;
	PUCHAR aBuf = NULL;
	PUCHAR pBuf2 = NULL;
	BOOL bRet = TRUE;
	try {
		if(NULL == (fpSub = CreateOrOpenFileW(pszOutFile, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
			throw "Failed to open .sub\n";
		}
		if(NULL == (fpCue = CreateOrOpenFileW(pszOutFile, NULL, NULL, _T(".cue"), _T("w"), 0, 0))) {
			throw "Failed to open .cue\n";
		}
		if(NULL == (fpParse = CreateOrOpenFileW(pszOutFile, NULL, NULL, _T(".sub.txt"), _T("w"), 0, 0))) {
			throw "Failed to open .sub.txt\n";
		}

		ULONG ulBufLen = CD_RAW_SECTOR_WITH_SUBCODE_SIZE;
		if(bC2ErrorData && strncmp(pszVendorId, "PLEXTOR", 7)) {
			if(NULL == (fpC2 = CreateOrOpenFileW(pszOutFile, NULL, NULL, _T(".c2.err"), _T("wb"), 0, 0))) {
				throw "Failed to open .c2.err\n";
			}
			ulBufLen = CD_RAW_SECTOR_WITH_C2_AND_SUBCODE_SIZE;
		}

		if(NULL == (fpTbl = OpenProgrammabledFile(_T("scramble.bin"), _T("rb")))) {
			throw "Failed to open scramble.bin\n";
		}
		size_t dwTrackAllocSize = (size_t)toc.LastTrack + 1;
#ifdef WIN64
		size_t dwTrackPointerAllocSize = dwTrackAllocSize * sizeof(INT64);
#else
		size_t dwTrackPointerAllocSize = dwTrackAllocSize * sizeof(INT);
#endif
		if(NULL == (aLBAStart = (INT**)malloc(dwTrackPointerAllocSize))) {
			throw "Failed to alloc memory aLBAStart\n";
		}
		if(NULL == (aLBAOfDataTrack = (INT**)malloc(dwTrackPointerAllocSize))) {
			throw "Failed to alloc memory aLBAOfDataTrack\n";
		}
		if(NULL == (aCtl = (PUCHAR)malloc(dwTrackAllocSize))) {
			throw "Failed to alloc memory aCtl\n";
		}
		if(NULL == (aMode = (PUCHAR)malloc(dwTrackAllocSize))) {
			throw "Failed to alloc memory aMode\n";
		}
		if(NULL == (aISRC = (BOOL*)malloc(dwTrackPointerAllocSize))) {
			throw "Failed to alloc memory aISRC\n";
		}
#ifdef WIN64
		size_t dwIndexAllocSize = (size_t)MAXIMUM_NUMBER_INDEXES * sizeof(INT64);
		size_t dwRangeAllocSize = (size_t)2 * sizeof(INT64);
#else
		size_t dwIndexAllocSize = (size_t)MAXIMUM_NUMBER_INDEXES * sizeof(INT);
		size_t dwRangeAllocSize = (size_t)2 * sizeof(INT);
#endif
		for(INT h = 0; h < toc.LastTrack + 1; h++) {
			if(NULL == (aLBAStart[h] = (INT*)malloc(dwIndexAllocSize))) {
				throw "Failed to alloc memory aLBAStart[h]\n";
			}
			if(NULL == (aLBAOfDataTrack[h] = (INT*)malloc(dwRangeAllocSize))) {
				throw "Failed to alloc memory aLBAOfDataTrack[h]\n";
			}
			FillMemory(aLBAStart[h], dwIndexAllocSize, -1);
			FillMemory(aLBAOfDataTrack[h], dwRangeAllocSize, -1);
		}
		ZeroMemory(aCtl, dwTrackAllocSize);
		ZeroMemory(aMode, dwTrackAllocSize);
		ZeroMemory(aISRC, dwTrackPointerAllocSize);

		if(NULL == (aBuf = (PUCHAR)malloc(ulBufLen + 0x10))) {
			throw "Failed to alloc memory aBuf[h]\n";
		}
		ZeroMemory(aBuf, ulBufLen + 0x10);
		PUCHAR pBuf = (PUCHAR)ConvParagraphBoundary(aBuf);
		UCHAR aCmd[CDB12GENERIC_LENGTH];
		ZeroMemory(aCmd, sizeof(aCmd));

		UCHAR byTransferLen = 1;
		aCmd[0] = SCSIOP_READ_CD;
		aCmd[8] = byTransferLen;
		aCmd[9] = READ_CD_FLAG::SyncData | READ_CD_FLAG::SubHeader | 
			READ_CD_FLAG::UserData | READ_CD_FLAG::MainHeader | READ_CD_FLAG::Edc;
		if(strncmp(pszVendorId, "PLEXTOR", 7)) {
			aCmd[9] |= READ_CD_FLAG::C2ErrorBlockData;
		}
		aCmd[10] = READ_CD_FLAG::PtoW;

		UCHAR byScsiStatus = 0;
		for(INT n = 16; ; n++) {
			if((toc.TrackData[0].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				aCmd[1] = READ_CD_FLAG::All;
				aCmd[5] = LOBYTE(LOWORD(n));
				if(!ExecCommand(hDevice, aCmd, CDB12GENERIC_LENGTH, pBuf, 
					ulBufLen, &byScsiStatus, __FUNCTION__, __LINE__)
					|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
					throw "";
				}
				if(pBuf[15] == DATA_BLOCK_MODE1) {
					OutputVolumeDescriptor(SYNC_SIZE + HEADER_SIZE, pBuf, fpLog);
					if(pBuf[SYNC_SIZE+HEADER_SIZE] == 0xff) {
						break;
					}
				}
				else if(pBuf[15] == DATA_BLOCK_MODE2) {
					OutputVolumeDescriptor(
						SYNC_SIZE + HEADER_SIZE + SUBHEADER_SIZE, pBuf, fpLog);
					if(pBuf[SYNC_SIZE+HEADER_SIZE+SUBHEADER_SIZE] == 0xff) {
						break;
					}
				}
			}
			else {
				break;
			}
		}
		UCHAR Subcode[CD_RAW_READ_SUBCODE_SIZE];
		ZeroMemory(Subcode, sizeof(Subcode));
		UCHAR SubcodeRtoW[CD_RAW_READ_SUBCODE_SIZE];
		ZeroMemory(SubcodeRtoW, sizeof(SubcodeRtoW));
		if(NULL == (aEndCtl = (PUCHAR)malloc(dwTrackAllocSize))) {
			throw "Failed to alloc memory aEndCtl\n";
		}
		ZeroMemory(aEndCtl, dwTrackAllocSize);
		if(!strncmp(pszVendorId, "PLEXTOR", 7)) {
			// PX-504A don't support...
			// Sense data, Key:Asc:Ascq: 05:20:00(ILLEGAL_REQUEST. INVALID COMMAND OPERATION CODE)
			aCmd[0] = 0xD8;
			aCmd[1] = 0x00;
			aCmd[8] = 0x00;
			aCmd[9] = byTransferLen;
			aCmd[10] = 0x02; // 0=none, 1=Q, 2=P-W, 3=P-W only
		}
		else {
			aCmd[1] = READ_CD_FLAG::CDDA;
		}
		for(INT p = 0; p < toc.LastTrack; p++) {
			aCmd[2] = HIBYTE(HIWORD(aTocLBA[p][0]));
			aCmd[3] = LOBYTE(HIWORD(aTocLBA[p][0]));
			aCmd[4] = HIBYTE(LOWORD(aTocLBA[p][0]));
			aCmd[5] = LOBYTE(LOWORD(aTocLBA[p][0]));
			if(!ExecCommand(hDevice, aCmd, CDB12GENERIC_LENGTH, 
				pBuf, ulBufLen, &byScsiStatus, __FUNCTION__, __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw "";
			}
			AlignSubcode(pBuf + CD_RAW_SECTOR_SIZE, Subcode, SubcodeRtoW);
			aEndCtl[p] = (UCHAR)((Subcode[12] >> 4) & 0x0F);
		}

		size_t uiShift = 0;
		INT nOffsetStart = 0;
		INT nOffsetEnd = 0;
		INT nFixStartLBA = 0;
		INT nFixEndLBA = 0;
		SUB_Q_DATA prevPrevSubQ = {0};
		SUB_Q_DATA prevSubQ = {0};
		if(nWriteOffset > 0) {
			uiShift = (size_t)nWriteOffset % CD_RAW_SECTOR_SIZE;
			nOffsetStart = 0;
			nOffsetEnd = nAdjustSectorNum;
			nFixStartLBA = nAdjustSectorNum - 1;
			nFixEndLBA = nAdjustSectorNum;
			prevSubQ.nRelativeTime = 0;
			prevSubQ.nAbsoluteTime = 149;
		}
		else if(nWriteOffset < 0) {
			uiShift = (size_t)CD_RAW_SECTOR_SIZE + (nWriteOffset % CD_RAW_SECTOR_SIZE);
			nOffsetStart = nAdjustSectorNum;
			nOffsetEnd = 0;
			nFixStartLBA = nAdjustSectorNum;
			nFixEndLBA = nAdjustSectorNum + 1;
			prevSubQ.nRelativeTime = 150 + nAdjustSectorNum - 1;
			prevSubQ.nAbsoluteTime = 150 + nAdjustSectorNum - 1;
		}
		prevSubQ.byCtl = aEndCtl[0];
		prevSubQ.byAdr = ADR_ENCODES_CURRENT_POSITION;
		prevSubQ.byTrackNum = toc.FirstTrack;
		prevSubQ.byIndex = nOffsetStart < 0 ? (UCHAR)0 : (UCHAR)1;
		prevSubQ.byMode = DATA_BLOCK_MODE0;
		BOOL bCatalog = FALSE;
		UCHAR byCurrentTrackNum = toc.FirstTrack;
		CHAR szCatalog[META_CATALOG_SIZE+1];
		CHAR szISRC[META_ISRC_SIZE+1];
		ZeroMemory(szCatalog, META_CATALOG_SIZE + 1);
		ZeroMemory(szISRC, META_ISRC_SIZE + 1);
		OutputLogStringA(fpLog, 
			"\n--------------------------------error log begin--------------------------------\n");

		for(INT nLBA = nOffsetStart; nLBA < nLength + nOffsetEnd; nLBA++) {
			if(nStartLBAof2ndSession != -1 && nLastLBAof1stSession == nLBA) {
				INT nStartSession2 = nStartLBAof2ndSession - 1;
				OutputLogStringA(fpLog, 
					"Skip from Leadout of Session 1 [%d]", nLastLBAof1stSession);
				OutputLogStringA(fpLog, 
					" to Leadin of Session 2 [%d]\n", nStartSession2);
				nLBA = nStartSession2;
				prevSubQ.nAbsoluteTime = nLBA + 150;
				continue;
			}

			// 1st:Read main & sub channel
			aCmd[2] = HIBYTE(HIWORD(nLBA));
			aCmd[3] = LOBYTE(HIWORD(nLBA));
			aCmd[4] = HIBYTE(LOWORD(nLBA));
			aCmd[5] = LOBYTE(LOWORD(nLBA));
			if(!ExecCommand(hDevice, aCmd, CDB12GENERIC_LENGTH, 
				pBuf, ulBufLen, &byScsiStatus, __FUNCTION__, __LINE__)) {
				throw "";
			}

			if(byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				size_t uiSize = 0;
				ZeroMemory(pBuf, ulBufLen);
				if(nLBA == nFixStartLBA) {
					uiSize = CD_RAW_SECTOR_SIZE - uiShift;
					fwrite(pBuf + uiShift, sizeof(UCHAR), uiSize, fpImg);
				}
				else if(nLBA == nLength + nFixEndLBA - 1) {
					uiSize = uiShift;
					fwrite(pBuf, sizeof(UCHAR), uiSize, fpImg);
				}
				else {
					uiSize = CD_RAW_SECTOR_SIZE;
					fwrite(pBuf, sizeof(UCHAR), uiSize, fpImg);
				}
				OutputErrorStringA(
					"LBA %d Read error [L:%d]. Zero padding [%dbyte]\n", 
					nLBA, __LINE__, uiSize);
				prevSubQ.nAbsoluteTime++;
				continue;
			}
			AlignSubcode(pBuf + CD_RAW_SECTOR_SIZE, Subcode, SubcodeRtoW);
			SUB_Q_DATA subQ = {0};
			subQ.byCtl = (UCHAR)((Subcode[12] >> 4) & 0x0F);
			subQ.byAdr = (UCHAR)(Subcode[12] & 0x0F);
			subQ.byTrackNum = BcdToDec(Subcode[13]);
			subQ.byIndex = BcdToDec(Subcode[14]);
			subQ.nRelativeTime = MSFtoLBA(BcdToDec(Subcode[17]), 
				BcdToDec(Subcode[16]), BcdToDec(Subcode[15]));
			subQ.nAbsoluteTime = MSFtoLBA(BcdToDec(Subcode[21]), 
				BcdToDec(Subcode[20]), BcdToDec(Subcode[19]));
			subQ.byMode = GetMode(pBuf + uiShift);

			if(0 <= nLBA && nLBA < nLength) {
				// 2nd:Verification subchannel
				if(subQ.byAdr == ADR_ENCODES_MEDIA_CATALOG) {
					BOOL bMCN = IsValidMCN(Subcode);
					SetMCNToString(Subcode, szCatalog, bMCN);
					szCatalog[13] = '\0';
					// only once
					if(prevSubQ.byTrackNum == toc.FirstTrack) {
						bCatalog = bMCN;
					}
					if(!bMCN) {
						OutputLogStringA(fpLog, "LBA %6d, MCN[%s]\n", nLBA, szCatalog);
						subQ.byAdr = prevSubQ.byAdr;
					}
					// because don't exist tracknum, index...
					subQ.byTrackNum = prevSubQ.byTrackNum;
					// Psychic Detective Series Vol. 5 - Nightmare (Japan)
					// LBA[080999, 0x13C67], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-02, Index-00, RelativeTime-00:00:00, AbsoluteTime-18:01:74] RtoW:ZERO mode
					// LBA[081000, 0x13C68], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number (MCN)[3010911111863        , AbsoluteTime-     :00] RtoW:ZERO mode
					// LBA[081001, 0x13C69], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-02, Index-01, RelativeTime-00:00:01, AbsoluteTime-18:02:01] RtoW:ZERO mode
					if(prevSubQ.byIndex == 0 && prevSubQ.nRelativeTime == 0) {
						subQ.byIndex = 1;
					}
					// Super Schwarzschild 2 (Japan)
					// LBA[234845, 0x3955D], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-19, Index-01, RelativeTime-01:50:09, AbsoluteTime-52:13:20] RtoW:ZERO mode
					// LBA[234846, 0x3955E], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-19, Index-02, RelativeTime-01:50:10, AbsoluteTime-52:13:21] RtoW:ZERO mode
					// LBA[234847, 0x3955F], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number (MCN)[0000000000000        , AbsoluteTime-     :22] RtoW:ZERO mode
					// LBA[234848, 0x39560], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-19, Index-01, RelativeTime-01:50:12, AbsoluteTime-52:13:23] RtoW:ZERO mode
					else if(prevSubQ.byIndex > 1) {
						subQ.byIndex = prevPrevSubQ.byIndex;
						prevSubQ.byIndex = prevPrevSubQ.byIndex;
					}
					else {
						subQ.byIndex = prevSubQ.byIndex;
					}
				}
				else if(subQ.byAdr == ADR_ENCODES_ISRC) {
					BOOL bISRC = IsValidISRC(Subcode);
					SetISRCToString(Subcode, byCurrentTrackNum, szISRC, bISRC);
					szISRC[12] = '\0';
					aISRC[byCurrentTrackNum-1] = bISRC;
					if(!bISRC) {
						OutputLogStringA(fpLog, "LBA %6d, ISRC[%s]\n", nLBA, szISRC);
						subQ.byAdr = prevSubQ.byAdr;
					}
					// because don't exist tracknum, index...
					subQ.byTrackNum = prevSubQ.byTrackNum;
					subQ.byIndex = prevSubQ.byIndex;
				}
				else {
					if(subQ.byAdr == ADR_NO_MODE_INFORMATION || 
						subQ.byAdr > ADR_ENCODES_ISRC) {
						OutputLogStringA(fpLog, "LBA %6d, Adr[%d], correct[%d]\n", 
							nLBA, subQ.byAdr, prevSubQ.byAdr);
						subQ.byAdr = prevSubQ.byAdr;
					}
					BOOL bPrevTrackNum = TRUE;
					if(!IsValidTrackNumber(&prevPrevSubQ, &prevSubQ,
						&subQ, toc.FirstTrack, toc.LastTrack, &bPrevTrackNum)) {
						OutputLogStringA(fpLog, "LBA %6d, TrackNum[%02d], correct[%02d]\n", 
							nLBA, subQ.byTrackNum, prevSubQ.byTrackNum);
						// Bikkuriman Daijikai (Japan)
						// LBA[106402, 0x19FA2], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-70, Index-01, RelativeTime-00:16:39, AbsoluteTime-23:40:52] RtoW:ZERO mode
						// LBA[106403, 0x19FA3], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-79, Index-01, RelativeTime-00:00:00, AbsoluteTime-21:40:53] RtoW:ZERO mode
						// LBA[106404, 0x19FA4], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-71, Index-01, RelativeTime-00:00:01, AbsoluteTime-23:40:54] RtoW:ZERO mode
						// Kagami no Kuni no Legend (Japan)
						// LBA[004373, 0x01115], Data, Copy NG, TOC[TrackNum-02, Index-00, RelativeTime-00:00:01, AbsoluteTime-01:00:23] RtoW:ZERO mode
						// LBA[004374, 0x01116], Data, Copy NG, TOC[TrackNum-0a, Index-00, RelativeTime-00:00:00, AbsoluteTime-01:00:24] RtoW:ZERO mode
						// LBA[004375, 0x01117], Data, Copy NG, TOC[TrackNum-02, Index-01, RelativeTime-00:00:00, AbsoluteTime-01:00:25] RtoW:ZERO mode
						subQ.byTrackNum = prevSubQ.byTrackNum;
						if(prevSubQ.byTrackNum < toc.LastTrack &&
							subQ.byIndex == 1 &&
							subQ.nRelativeTime == 0) {
							subQ.byTrackNum += 1;
						}
					}
					else if(!bPrevTrackNum) {
						OutputLogStringA(fpLog, "LBA %6d, PrevTrackNum[%02d], correct[%02d]\n", 
							nLBA, prevSubQ.byTrackNum, prevPrevSubQ.byTrackNum);
						aLBAStart[prevSubQ.byTrackNum-1][prevSubQ.byIndex] = -1;
						aLBAOfDataTrack[prevSubQ.byTrackNum-1][0] = -1;
						prevSubQ.byTrackNum = prevPrevSubQ.byTrackNum;
					}
					BOOL bPrevIndex = TRUE;
					BOOL bPrevPrevIndex = TRUE;
					if(!IsValidIndex(&prevPrevSubQ, &prevSubQ, &subQ, &bPrevIndex, &bPrevPrevIndex)) {
						OutputLogStringA(fpLog, "LBA %6d, Index[%02d], correct[%02d]\n", 
							nLBA, subQ.byIndex, prevSubQ.byIndex);
						subQ.byTrackNum = prevSubQ.byTrackNum;
						subQ.byIndex = prevSubQ.byIndex;
					}
					else if(!bPrevIndex) {
						OutputLogStringA(fpLog, "LBA %6d, PrevIndex[%02d], correct[%02d]\n", 
							nLBA - 1 , prevSubQ.byIndex, prevPrevSubQ.byIndex);
						aLBAStart[prevSubQ.byTrackNum-1][prevSubQ.byIndex] = -1;
						prevSubQ.byTrackNum = prevPrevSubQ.byTrackNum;
						prevSubQ.byIndex = prevPrevSubQ.byIndex;
					}
					else if(!bPrevPrevIndex) {
						OutputLogStringA(fpLog, "LBA %6d, PrevPrevIndex[%02d], correct[%02d]\n", 
							nLBA - 1 , prevPrevSubQ.byIndex, prevSubQ.byIndex);
						aLBAStart[prevPrevSubQ.byTrackNum-1][prevPrevSubQ.byIndex] = -1;
						prevPrevSubQ.byTrackNum = prevSubQ.byTrackNum;
						prevPrevSubQ.byIndex = prevSubQ.byIndex;
					}
					if(Subcode[18] != 0) {
						OutputLogStringA(fpLog, 
							"LBA %6d, Zero[%02d], correct[0]\n", nLBA, Subcode[18]);
					}
					if(!IsValidAbsoluteTime(&prevSubQ, &subQ, Subcode, nLBA)) {
						UCHAR byFrame, bySecond, byMinute;
						LBAtoMSF(nLBA + 150, &byFrame, &bySecond, &byMinute);
						OutputLogStringA(fpLog, 
							"LBA %6d, AbsoluteTime[%02d:%02d:%02d], correct[%02d:%02d:%02d]\n", 
							nLBA, BcdToDec(Subcode[19]), BcdToDec(Subcode[20]), 
							BcdToDec(Subcode[21]), byMinute, bySecond, byFrame);
					}
				}
				if(!IsValidControl(&prevPrevSubQ, &prevSubQ, 
					&subQ, aEndCtl[subQ.byTrackNum-1])) {
					OutputLogStringA(fpLog, "LBA %6d, Ctl[%d], correct[%d]\n",
						nLBA, subQ.byCtl, prevSubQ.byCtl);
					subQ.byCtl = prevSubQ.byCtl;
				}

				// 3rd:Write subchannel
				fwrite(Subcode, sizeof(UCHAR), CD_RAW_READ_SUBCODE_SIZE, fpSub);
				OutputSubcode(nLBA, byCurrentTrackNum, 
					Subcode, pBuf + CD_RAW_SECTOR_SIZE, fpParse);

				// preserve mode, ctl
				if(nLBA == aTocLBA[subQ.byTrackNum-1][0]) {
					aCtl[subQ.byTrackNum-1] = subQ.byCtl;
					aMode[subQ.byTrackNum-1] = subQ.byMode;
				}
				// preserve nLBA
				if(prevSubQ.byTrackNum + 1 == subQ.byTrackNum) {
					aLBAStart[subQ.byTrackNum-1][subQ.byIndex] = nLBA;
					if(subQ.nRelativeTime == 1) {
						// Madou Monogatari I - Honoo no Sotsuenji (Japan)
						// LBA[183031, 0x2CAF7], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-21, Index-01, RelativeTime-00:31:70, AbsoluteTime-40:42:31] RtoW:ZERO mode
						// LBA[183032, 0x2CAF8], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number (MCN)[0000000000000        , AbsoluteTime-     :32] RtoW:ZERO mode
						// LBA[183033, 0x2CAF9], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-22, Index-01, RelativeTime-00:00:01, AbsoluteTime-40:42:33] RtoW:ZERO mode
						aLBAStart[subQ.byTrackNum-1][subQ.byIndex] -= 1;
					}
					// preserve end lba of data track
					if(aLBAOfDataTrack[prevSubQ.byTrackNum-1][0] != -1 &&
						aLBAOfDataTrack[prevSubQ.byTrackNum-1][1] == -1 &&
						(prevSubQ.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
							aLBAOfDataTrack[prevSubQ.byTrackNum-1][1] = nLBA - 1;
					}
					// preserve first lba of data track
					if(aLBAOfDataTrack[subQ.byTrackNum-1][0] == -1 &&
						(subQ.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
							aLBAOfDataTrack[subQ.byTrackNum-1][0] = nLBA;
					}
					byCurrentTrackNum = subQ.byTrackNum;
				}
				// Hatsukoi Monotagari (Japan)
				if(prevSubQ.byIndex + 1 == subQ.byIndex && 
					(prevSubQ.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK &&
					(subQ.byCtl & AUDIO_DATA_TRACK) == 0) {
					aLBAOfDataTrack[subQ.byTrackNum-1][subQ.byIndex] = nLBA - 1;
				}
				// preserve index
				if(prevSubQ.byIndex + 1 == subQ.byIndex && 
					byCurrentTrackNum != toc.FirstTrack) {
					aLBAStart[subQ.byTrackNum-1][subQ.byIndex] = nLBA;
				}
				// preserve first lba of data track offset
				if(aLBAOfDataTrack[subQ.byTrackNum-1][0] == -1 &&
					(subQ.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
						aLBAOfDataTrack[subQ.byTrackNum-1][0] = nLBA;
				}
				// preserve end data track offset
				else if(nLBA == nLength - 1) {
					// preserve end data track offset
					if(aLBAOfDataTrack[subQ.byTrackNum-1][0] != -1 &&
						aLBAOfDataTrack[subQ.byTrackNum-1][1] == -1 &&
						(subQ.byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
							aLBAOfDataTrack[subQ.byTrackNum-1][1] = nLength - 1;
					}
				}
			}

			// 4th:Write track to scrambled
			if(nFixStartLBA <= nLBA && nLBA < nLength + nFixEndLBA) {
				// first sector
				if(nLBA == nFixStartLBA) {
					fwrite(pBuf + uiShift, sizeof(UCHAR), 
						CD_RAW_SECTOR_SIZE - uiShift, fpImg);
					aLBAStart[0][0] = -150;
					aLBAStart[0][1] = nLBA - nFixStartLBA;
				}
				// last sector
				else if(nLBA == nLength + nFixEndLBA - 1) {
					fwrite(pBuf, sizeof(UCHAR), uiShift, fpImg);
				}
				else {
					if(nStartLBAof2ndSession != -1 && nLBA == nStartLBAof2ndSession) {
						if(nWriteOffset > 0) {
							ZeroMemory(pBuf, (size_t)nWriteOffset);
						}
						else if(nWriteOffset < 0) {
							// todo
						}
					}
					fwrite(pBuf, sizeof(UCHAR), CD_RAW_SECTOR_SIZE, fpImg);
				}
			}
			if(strncmp(pszVendorId, "PLEXTOR", 7)) {
				fwrite(pBuf + CD_RAW_SECTOR_WITH_SUBCODE_SIZE, 
					sizeof(UCHAR), CD_RAW_READ_C2_SIZE, fpC2);
			}
			if(prevSubQ.byAdr != ADR_ENCODES_MEDIA_CATALOG && 
				prevSubQ.byAdr != ADR_ENCODES_ISRC) {
				prevPrevSubQ.byAdr = prevSubQ.byAdr;
				prevPrevSubQ.byTrackNum = prevSubQ.byTrackNum;
				prevPrevSubQ.byIndex = prevSubQ.byIndex;
				prevPrevSubQ.nRelativeTime = prevSubQ.nRelativeTime;
				prevPrevSubQ.byMode = prevSubQ.byMode;
			}
			if(subQ.byAdr != ADR_ENCODES_MEDIA_CATALOG && 
				subQ.byAdr != ADR_ENCODES_ISRC) {
				prevSubQ.byAdr = subQ.byAdr;
				prevSubQ.byTrackNum = subQ.byTrackNum;
				prevSubQ.byIndex = subQ.byIndex;
				prevSubQ.nRelativeTime = subQ.nRelativeTime;
				prevSubQ.byMode = subQ.byMode;
			}
			else if(prevSubQ.byIndex == 0 && prevSubQ.nRelativeTime == 0) {
				prevSubQ.byIndex = 1;
			}
			prevPrevSubQ.byCtl = prevSubQ.byCtl;
			prevPrevSubQ.nAbsoluteTime = prevSubQ.nAbsoluteTime;
			prevSubQ.byCtl = subQ.byCtl;
			prevSubQ.nAbsoluteTime++;
			printf("\rCreating img(LBA) %6d/%6d", nLBA - nOffsetEnd, nLength - 1);
		}
		printf("\n");
		FcloseAndNull(fpImg);
		FcloseAndNull(fpSub);
		if(strncmp(pszVendorId, "PLEXTOR", 7)) {
			FcloseAndNull(fpC2);
		}
		FcloseAndNull(fpParse);
		FreeAndNull(aBuf);

		OutputLogStringA(fpLog, 
			"---------------------------------error log end---------------------------------\n\n");
		OutputLogStringA(fpLog, "TOC on Subchannel\n");
		for(INT r = 0; r < toc.LastTrack; r++) {
			OutputLogStringA(fpLog, 
				"\tTrack %2d, Ctl %d, Mode %d", r + 1, aCtl[r], aMode[r]);
			for(INT k = 0; k < MAXIMUM_NUMBER_INDEXES; k++) {
				if(aLBAStart[r][k] != -1) {
					OutputLogStringA(fpLog, ", Index%d %6d", k, aLBAStart[r][k]);
				}
				else if(k == 0) {
					OutputLogStringA(fpLog, ",              ");
				}
			}
			OutputLogStringA(fpLog, "\n");
		}

		// 5th:Descramble img
		UCHAR aScrambledBuf[CD_RAW_SECTOR_SIZE];
		fread(aScrambledBuf, sizeof(UCHAR), sizeof(aScrambledBuf), fpTbl);
		FcloseAndNull(fpTbl);
		UCHAR aSrcBuf[CD_RAW_SECTOR_SIZE];

		if(NULL == (fpImg2 = CreateOrOpenFileW(pszOutFile, NULL, NULL, _T(".img"), _T("rb+"), 0, 0))) {
			throw "Failed to open file .img\n";
		}

		for(INT k = 0; k < toc.LastTrack; k++) {
			if(aLBAOfDataTrack[k][0] != -1) {
				OutputLogStringA(fpLog, "\tData Sector, LBA %6d-%6d\n", 
					aLBAOfDataTrack[k][0], aLBAOfDataTrack[k][1]);
				INT nStartLBA = aSessionNum[k] >= 2 
					? aLBAOfDataTrack[k][0] - (11400 * (aSessionNum[k] - 1)) 
					: aLBAOfDataTrack[k][0];
				INT nEndLBA = aSessionNum[k] >= 2 
					? aLBAOfDataTrack[k][1] - (11400 * (aSessionNum[k] - 1)) 
					: aLBAOfDataTrack[k][1];
				for(; nStartLBA <= nEndLBA; nStartLBA++) {
					// ファイルを読み書き両用モードで開いている時は 注意が必要です。
					// 読み込みを行った後に書き込みを行う場合やその逆を行う場合は、 
					// 必ずfseekを呼ばなければなりません。もしこれを忘れると、
					// 場合によってはバッファー内と 実際にディスクに描き込まれた
					// データに矛盾が生じ、正確に書き込まれない場合や、
					// 嘘の データを読み込む場合があります。
					fseek(fpImg2, nStartLBA * CD_RAW_SECTOR_SIZE, SEEK_SET);
					fread(aSrcBuf, sizeof(UCHAR), sizeof(aSrcBuf), fpImg2);
					if(IsValidDataHeader(aSrcBuf)) {
						fseek(fpImg2, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
						for(INT n = 0; n < CD_RAW_SECTOR_SIZE; n++) {
							aSrcBuf[n] ^= aScrambledBuf[n];
						}
						fwrite(aSrcBuf, sizeof(UCHAR), sizeof(aSrcBuf), fpImg2);
					}
					printf("\rDescrambling data sector of img(LBA) %6d/%6d", nStartLBA, nEndLBA);
				}
				printf("\n");
			}
		}
		// 6th:Creating bin, cue, ccd
		TCHAR pszFileNameWithoutPath[_MAX_FNAME];
		for(INT i = toc.FirstTrack; i <= toc.LastTrack; i++) {
			if(NULL == (fpBin = CreateOrOpenFileW(pszOutFile, NULL, 
				pszFileNameWithoutPath, _T(".bin"), _T("wb"), i, toc.LastTrack))) {
				throw "Failed to open .bin\n";
			}
			printf("\rCreating bin, cue, ccd(Track) %2d/%2d", i, toc.LastTrack);
			CHAR fname[_MAX_FNAME*sizeof(TCHAR)];
			ZeroMemory(fname, sizeof(fname));
			WideCharToMultiByte(CP_ACP, 0, pszFileNameWithoutPath,
				-1, fname, sizeof(fname), NULL, NULL);
			WriteCueFile(bCatalog, fname, 
				i, aMode[i-1], aISRC[i-1], aCtl[i-1], fpCue);
			bCatalog = FALSE;
			WriteCcdFileForTrack(i, aMode[i-1], aISRC[i-1], fpCcd);
			if(i == toc.FirstTrack) {
				WriteCueFileForIndex(1, 0, 0, 0, fpCue);
				WriteCcdFileForTrackIndex(1, 0, fpCcd);
			}
			// only 1 index
			else if(aLBAStart[i-1][0] == -1 && aLBAStart[i-1][2] == -1) {
				WriteCueFileForIndex(1, 0, 0, 0, fpCue);
				WriteCcdFileForTrackIndex(1, aLBAStart[i-1][1], fpCcd);
			}
			else if(aLBAStart[i-1][0] != -1) {
				for(UCHAR index = 0; index < MAXIMUM_NUMBER_INDEXES; index++) {
					BYTE byFrame = 0, bySecond = 0, byMinute = 0;
					if(aLBAStart[i-1][index] != -1) {
						LBAtoMSF(aLBAStart[i-1][index] - aLBAStart[i-1][0], 
							&byFrame, &bySecond, &byMinute);
						WriteCueFileForIndex(index, byFrame, bySecond, byMinute, fpCue);
						WriteCcdFileForTrackIndex(index, aLBAStart[i-1][index], fpCcd);
					}
				}
			}
			else {
				for(UCHAR index = 1; index < MAXIMUM_NUMBER_INDEXES; index++) {
					BYTE byFrame = 0, bySecond = 0, byMinute = 0;
					if(aLBAStart[i-1][index] != -1) {
						LBAtoMSF(aLBAStart[i-1][index] - aLBAStart[i-1][1], 
							&byFrame, &bySecond, &byMinute);
						WriteCueFileForIndex(index, byFrame, bySecond, byMinute, fpCue);
						WriteCcdFileForTrackIndex(index, aLBAStart[i-1][index], fpCcd);
					}
				}
			}
			// write each track
			size_t uiBufsize = 0;
			INT nLBA = aLBAStart[i][0] == -1 ? aLBAStart[i][1] : aLBAStart[i][0];
			INT nPrevLba = aLBAStart[i-1][0] == -1 ? aLBAStart[i-1][1] : aLBAStart[i-1][0];
			if(toc.LastTrack == toc.FirstTrack) {
				uiBufsize = (size_t)nLength * CD_RAW_SECTOR_SIZE;
				nPrevLba = 0;
			}
			else if(i == toc.FirstTrack) {
				uiBufsize = (size_t)nLBA * CD_RAW_SECTOR_SIZE;
				nPrevLba = 0;
			}
			else if(i == toc.LastTrack) {
				INT nTmpLength = nLength;
				if(aSessionNum[i-1] > 1) {
					INT nLeadinoutSize = 11400 * (aSessionNum[i-1] - 1);
					nPrevLba -= nLeadinoutSize;
					nTmpLength -= nLeadinoutSize;
				}
				uiBufsize = (size_t)(nTmpLength - nPrevLba) * CD_RAW_SECTOR_SIZE;
			}
			else {
				if(i == toc.LastTrack - 1 && aSessionNum[i] > 1) {
					nLBA -= 11400 * (aSessionNum[i] - 1);
				}
				uiBufsize = (size_t)(nLBA - nPrevLba) * CD_RAW_SECTOR_SIZE;
			}
			fseek(fpImg2, nPrevLba * CD_RAW_SECTOR_SIZE, SEEK_SET);
			if(NULL == (pBuf2 = (PUCHAR)malloc(uiBufsize))) {
				throw "Failed to alloc memory pBuf2\n";
			}
			fread(pBuf2, sizeof(UCHAR), uiBufsize, fpImg2);
			fwrite(pBuf2, sizeof(UCHAR), uiBufsize, fpBin);
			FreeAndNull(pBuf2);
			FcloseAndNull(fpBin);
		}
		printf("\n");
	}
	catch(PCHAR str) {
		OutputErrorStringA(str);
		bRet = FALSE;
	}
#ifndef _DEBUG
	fflush(fpLog);
#endif
	FcloseAndNull(fpImg);
	FcloseAndNull(fpImg2);
	FcloseAndNull(fpBin);
	FcloseAndNull(fpSub);
	FcloseAndNull(fpCue);
	FcloseAndNull(fpParse);
	FcloseAndNull(fpC2);
	FcloseAndNull(fpTbl);
	for(INT h = 0; h < toc.LastTrack + 1; h++) {
		FreeAndNull(aLBAStart[h]);
		FreeAndNull(aLBAOfDataTrack[h]);
	}
	FreeAndNull(aLBAStart);
	FreeAndNull(aLBAOfDataTrack);
	FreeAndNull(aEndCtl);
	FreeAndNull(aCtl);
	FreeAndNull(aMode);
	FreeAndNull(aISRC);
	FreeAndNull(aBuf);
	FreeAndNull(pBuf2);
	return bRet;
}

BOOL ReadCDForSearchingOffset(
	HANDLE hDevice,
	LPCSTR pszVendorId,
	LPCSTR pszProductId,
	PINT nCombinedOffset, 
	FILE* fpLog
	)
{
	BOOL bAudioOnly = TRUE;
	if((toc.TrackData[0].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
		bAudioOnly = FALSE;
	}
	else {
		for(INT i = toc.FirstTrack + 1; i <= toc.LastTrack; i++) {
			if(toc.TrackData[0].Control != toc.TrackData[i-1].Control) {
				bAudioOnly = FALSE;
				break;
			}
		}
	}
	BOOL bRet = FALSE;
	INT nDriveSampleOffset = 0;
	BOOL bGetDrive = GetDriveOffset(pszProductId, &nDriveSampleOffset);
	if(!bGetDrive) {
		CHAR a[6];
		ZeroMemory(a, sizeof(a));
		printf("This drive isn't defined in driveOffset.txt.\n");
		printf("Please input Drive offset(Samples) -> ");
		INT b = scanf("%6[^\n]%*[^\n]", a);
		b = getchar();
		nDriveSampleOffset = atoi(a) * 4;
	}
	if(bAudioOnly) {
		*nCombinedOffset = nDriveSampleOffset * 4;
		OutputLogStringA(fpLog, "Offset");
		if(bGetDrive) {
			OutputLogStringA(fpLog, "(Drive offset data referes to www.accuraterip.com)");
		}
		OutputLogStringA(fpLog, "\n");
		OutputLogStringA(fpLog, "\tDrive Offset(Byte) %d, (Samples) %d\n", 
			*nCombinedOffset, nDriveSampleOffset);
		bRet = TRUE;
	}
	else {
		UCHAR byScsiStatus = 0;
		UCHAR aBuf[CD_RAW_SECTOR_WITH_SUBCODE_SIZE+0x10];
		UCHAR aCmd[CDB12GENERIC_LENGTH];

		ZeroMemory(aBuf, sizeof(aBuf));
		PUCHAR pBuf = (PUCHAR)ConvParagraphBoundary(aBuf);
		ZeroMemory(aCmd, sizeof(aCmd));

		INT nDriveOffset = nDriveSampleOffset * 4;
		UCHAR byTransferLen = 1;
		if(!strncmp(pszVendorId, "PLEXTOR", 7)) {
			// PX-504A don't support...
			// Sense data, Key:Asc:Ascq: 05:20:00(ILLEGAL_REQUEST. INVALID COMMAND OPERATION CODE)
			aCmd[0] = 0xD8;
			aCmd[9] = byTransferLen;
			aCmd[10] = 0x02; // 0=none, 1=Q, 2=P-W, 3=P-W only
		}
		else {
			aCmd[0] = SCSIOP_READ_CD;
			aCmd[1] = READ_CD_FLAG::CDDA;
			aCmd[8] = byTransferLen;
			aCmd[9] = READ_CD_FLAG::SyncData | READ_CD_FLAG::SubHeader | 
				READ_CD_FLAG::UserData | READ_CD_FLAG::MainHeader | 
				READ_CD_FLAG::Edc;
			aCmd[10] = READ_CD_FLAG::PtoW;
		}
		aCmd[2] = HIBYTE(HIWORD(nFirstDataLBA));
		aCmd[3] = LOBYTE(HIWORD(nFirstDataLBA));
		aCmd[4] = HIBYTE(LOWORD(nFirstDataLBA));
		aCmd[5] = LOBYTE(LOWORD(nFirstDataLBA));
		if(!ExecCommand(hDevice, aCmd, CDB12GENERIC_LENGTH, pBuf, 
			CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &byScsiStatus, __FUNCTION__, __LINE__)) {
			return FALSE;
		}
		if(byScsiStatus == 0) {
			OutputMain2352(pBuf, nFirstDataLBA, fpLog);
			UCHAR Subcode[CD_RAW_READ_SUBCODE_SIZE];
			UCHAR SubcodeRtoW[CD_RAW_READ_SUBCODE_SIZE];

			AlignSubcode(pBuf + CD_RAW_SECTOR_SIZE, Subcode, SubcodeRtoW);
			OutputSub96(Subcode, nFirstDataLBA, fpLog);
			bRet = GetWriteOffset(pBuf, nFirstDataLBA, nCombinedOffset);
		}
		else {
			printf("This drive can't read data sector in scrambled mode.\n");
			return FALSE;
		}

		if(bRet) {
			OutputLogStringA(fpLog, "Offset");
			if(bGetDrive) {
				OutputLogStringA(fpLog, "(Drive offset data referes to www.accuraterip.com)");
			}
			OutputLogStringA(fpLog, "\n");
			OutputLogStringA(fpLog, 
				"\t Combined Offset(Byte) %6d, (Samples) %5d\n", 
				*nCombinedOffset, *nCombinedOffset / 4);
			OutputLogStringA(fpLog, 
				"\t-   Drive Offset(Byte) %6d, (Samples) %5d\n", 
				nDriveOffset, nDriveSampleOffset);
			OutputLogStringA(fpLog, 
				"\t----------------------------------------------\n");
			OutputLogStringA(fpLog, 
				"\t       CD Offset(Byte) %6d, (Samples) %5d\n", 
				*nCombinedOffset - nDriveOffset, 
				(*nCombinedOffset - nDriveOffset) / 4);
		}
		else {
			OutputErrorStringA("Failed to get write-offset[L:%d]\n", __LINE__);
		}
	}

	if(0 < *nCombinedOffset) {
		nAdjustSectorNum = *nCombinedOffset / CD_RAW_SECTOR_SIZE + 1;
	}
	else if(*nCombinedOffset < 0) {
		nAdjustSectorNum = *nCombinedOffset / CD_RAW_SECTOR_SIZE - 1;
	}
	OutputLogStringA(fpLog, "\tNeed overread sector:%d\n", nAdjustSectorNum);
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#else
	fflush(fpLog);
#endif
	return bRet;
}

BOOL ReadCDPartial(
	HANDLE hDevice,
	LPCTSTR pszOutFile,
	LPCSTR pszVendorId,
	INT nStart,
	INT nEnd,
	_READ_CD_FLAG::_SectorType flg
	)
{
	FILE* fp = CreateOrOpenFileW(pszOutFile, NULL, NULL, _T(".bin"), _T("wb"), 0, 0);
	if(!fp) {
		OutputErrorStringA(
			"Failed to open file .bin [L:%d]\n", __LINE__);
		return FALSE;
	}
	FILE* fpSub = NULL;
	FILE* fpParse = NULL;
	BOOL bRet = TRUE;
	try {
		if(NULL == (fpSub = CreateOrOpenFileW(pszOutFile, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
			throw "Failed to open .sub\n";
		}
		if(NULL == (fpParse = CreateOrOpenFileW(pszOutFile, NULL, NULL, _T(".sub.txt"), _T("w"), 0, 0))) {
			throw "Failed to open .sub.txt\n";
		}
		UCHAR byScsiStatus = 0;
		UCHAR aBuf[CD_RAW_SECTOR_WITH_SUBCODE_SIZE+0x10];
		UCHAR aCmd[CDB12GENERIC_LENGTH];
		ZeroMemory(aBuf, sizeof(aBuf));
		PUCHAR pBuf = (PUCHAR)ConvParagraphBoundary(aBuf);
		ZeroMemory(aCmd, sizeof(aCmd));

		UCHAR Subcode[CD_RAW_READ_SUBCODE_SIZE];
		UCHAR SubcodeRtoW[CD_RAW_READ_SUBCODE_SIZE];
		UCHAR byTransferLen = 1;
		if(!strncmp(pszVendorId, "PLEXTOR", 7)) {
			// PX-504A don't support...
			// Sense data, Key:Asc:Ascq: 05:20:00(ILLEGAL_REQUEST. INVALID COMMAND OPERATION CODE)
			aCmd[0] = 0xD8;
			aCmd[9] = byTransferLen;
			aCmd[10] = 0x02; // 0=none, 1=Q, 2=P-W, 3=P-W only
		}
		else {
			aCmd[0] = SCSIOP_READ_CD;
			aCmd[1] = (UCHAR)flg;
			aCmd[8] = byTransferLen;
			aCmd[9] = READ_CD_FLAG::SyncData | READ_CD_FLAG::SubHeader | 
				READ_CD_FLAG::UserData | READ_CD_FLAG::MainHeader | READ_CD_FLAG::Edc;
			aCmd[10] = READ_CD_FLAG::PtoW;
		}
		for(INT nLBA = nStart; nLBA <= nEnd; nLBA++) {
			aCmd[2] = HIBYTE(HIWORD(nLBA));
			aCmd[3] = LOBYTE(HIWORD(nLBA));
			aCmd[4] = HIBYTE(LOWORD(nLBA));
			aCmd[5] = LOBYTE(LOWORD(nLBA));
			if(!ExecCommand(hDevice, aCmd, CDB12GENERIC_LENGTH, pBuf, 
				CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &byScsiStatus, 
				__FUNCTION__, __LINE__)) {
				throw "";
			}
			if(byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				OutputErrorStringA("LBA %d Read error [L:%d]\n", nLBA, __LINE__);
				if(nEnd == 549150) {
					throw "";
				}
			}
			else {
				fwrite(pBuf, sizeof(UCHAR), CD_RAW_SECTOR_SIZE, fp);
				AlignSubcode(pBuf + CD_RAW_SECTOR_SIZE, Subcode, SubcodeRtoW);
				fwrite(Subcode, sizeof(UCHAR), CD_RAW_READ_SUBCODE_SIZE, fpSub);
				OutputSubcode(nLBA, BcdToDec(Subcode[13]) - 1, 
					Subcode, pBuf + CD_RAW_SECTOR_SIZE, fpParse);
			}
			printf("\rCreating bin from %d to %d (LBA) %6d", nStart, nEnd, nLBA);
		}
		printf("\n");
	}
	catch(PCHAR str) {
		OutputErrorStringA(str);
		bRet = FALSE;
	}
	FcloseAndNull(fp);
	FcloseAndNull(fpSub);
	FcloseAndNull(fpParse);
	return bRet;
}

BOOL ReadConfiguration(
	HANDLE hDevice,
	PUSHORT pusCurrentMedia,
	PBOOL bCanCDText,
	PBOOL bC2ErrorData,
	FILE* fpLog
	)
{
	UCHAR byScsiStatus = 0;
	size_t uiSize = sizeof(GET_CONFIGURATION_HEADER);
	UCHAR aCmd[CDB10GENERIC_LENGTH];
	UCHAR pHeader[sizeof(GET_CONFIGURATION_HEADER)+0x10];

	ZeroMemory(pHeader, uiSize);
	ZeroMemory(aCmd, sizeof(aCmd));
	aCmd[0] = SCSIOP_GET_CONFIGURATION;
	aCmd[1] = SCSI_GET_CONFIGURATION_REQUEST_TYPE_CURRENT;
	aCmd[3] = FeatureProfileList;
	aCmd[7] = HIBYTE(uiSize);
	aCmd[8] = LOBYTE(uiSize);
	if(!ExecCommand(hDevice, aCmd, CDB10GENERIC_LENGTH, pHeader, 
		(ULONG)uiSize, &byScsiStatus, __FUNCTION__, __LINE__) || 
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		*pusCurrentMedia = ProfileCdrom;
		return TRUE;
	}
	OutputLogStringA(fpLog, "Configuration\n");
	OutputLogStringA(fpLog, "\tCurrentMedia:");
	*pusCurrentMedia = MAKEWORD(pHeader[7], pHeader[6]);
	OutputFeatureProfileType(fpLog, *pusCurrentMedia);
	OutputLogStringA(fpLog, "\n");
	// 4 is DataLength size of GET_CONFIGURATION_HEADER
	LONG lAllLen = MAKELONG(MAKEWORD(pHeader[3], pHeader[2]), 
		MAKEWORD(pHeader[1], pHeader[0])) + 4;

	PUCHAR pConf = (PUCHAR)malloc((size_t)lAllLen);
	if(!pConf) {
		printf("Can't alloc memory [F:%s][L:%d]\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	aCmd[7] = HIBYTE(lAllLen);
	aCmd[8] = LOBYTE(lAllLen);
	if(!ExecCommand(hDevice, aCmd, CDB10GENERIC_LENGTH, pConf, 
		(size_t)lAllLen, &byScsiStatus, __FUNCTION__, __LINE__) || 
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		FreeAndNull(pConf);
		// because undefined mmc1..
		return TRUE;
	}
	OutputFeatureNumber(pConf, lAllLen, uiSize, bCanCDText, bC2ErrorData, fpLog);
	return TRUE;
}

BOOL ReadDeviceInfo(
	HANDLE hDevice,
	LPSTR pszVendorId,
	LPSTR pszProductId,
	FILE* fpLog
	)
{
	UCHAR byScsiStatus = 0;
	UCHAR byInquirySize = sizeof(INQUIRYDATA);
	UCHAR aInquiryBuf[sizeof(INQUIRYDATA)+0x10];
	UCHAR aCmd[CDB6GENERIC_LENGTH];
	ZeroMemory(aInquiryBuf, sizeof(aInquiryBuf));
	ZeroMemory(aCmd, sizeof(aCmd));
	CHAR* pInquiry = (CHAR*)ConvParagraphBoundary(aInquiryBuf);

	aCmd[0] = SCSIOP_INQUIRY;
	aCmd[4] = byInquirySize;
	if(!ExecCommand(hDevice, aCmd, CDB6GENERIC_LENGTH, pInquiry, 
		byInquirySize, &byScsiStatus, __FUNCTION__, __LINE__) || 
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	OutputInquiryData(pInquiry, pszVendorId, pszProductId, fpLog);
	return TRUE;
}

BOOL ReadDVD(
	HANDLE hDevice,
	LPCTSTR pszOutFile,
	LPCTSTR pszOption,
	INT nDVDSectorSize,
	FILE* fpLog
	)
{
	FILE* fp = CreateOrOpenFileW(pszOutFile, NULL, NULL, _T(".iso"), _T("wb"), 0, 0);
	if(!fp) {
		OutputErrorStringA("Failed to open file .iso [L:%d]\n", __LINE__);
		return FALSE;
	}
	UCHAR byScsiStatus = 0;
	UCHAR byTransferLen = 32;
	PUCHAR aBuf = NULL;
	BOOL bRet = TRUE;
	try {
		if(NULL == (aBuf = (PUCHAR)malloc(CD_RAW_READ * (size_t)byTransferLen + 0x10))) {
			throw "Can't alloc memory aBuf\n";
		}
		UCHAR aCmd[CDB12GENERIC_LENGTH];
		ZeroMemory(aBuf, CD_RAW_READ * (size_t)byTransferLen + 0x10);
		PUCHAR pBuf = (PUCHAR)ConvParagraphBoundary(aBuf);
		ZeroMemory(aCmd, sizeof(aCmd));
		aCmd[0] = SCSIOP_READ12;
		aCmd[5] = 0;
		aCmd[9] = byTransferLen;
		aCmd[10] = 0x80; // Streaming
		if(!ExecCommand(hDevice, aCmd, CDB12GENERIC_LENGTH, pBuf, 
			(ULONG)CD_RAW_READ * byTransferLen, &byScsiStatus, __FUNCTION__, __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			OutputErrorStringA("Not supported Streaming. Changed ForceMediaAccess.\n");
			aCmd[1] = CDB_FORCE_MEDIA_ACCESS;
			aCmd[10] = 0;
		}

		UCHAR aBuf2[sizeof(DVD_DESCRIPTOR_HEADER)+sizeof(DVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR)+0x10];
		UCHAR aCmd2[CDB10GENERIC_LENGTH];
		PUCHAR pBuf2 = NULL;
		size_t uiSize = 0;
		if(pszOption && !_tcscmp(pszOption, _T("cmi"))) {
			ZeroMemory(aBuf2, sizeof(aBuf2));
			pBuf2 = (PUCHAR)ConvParagraphBoundary(aBuf2);
			ZeroMemory(aCmd2, sizeof(aCmd2));
			uiSize = sizeof(DVD_DESCRIPTOR_HEADER) + 
				sizeof(DVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR);
			aCmd2[0] = SCSIOP_READ_DVD_STRUCTURE;
			aCmd2[7] = DvdMaxDescriptor;
			aCmd2[8] = HIBYTE(LOWORD(uiSize));
			aCmd2[9] = LOBYTE(LOWORD(uiSize));
			OutputLogStringA(fpLog, "\tCopyrightManagementInformation\n");
		}
		aCmd[5] = LOBYTE(0);
		if(!ExecCommand(hDevice, aCmd, CDB12GENERIC_LENGTH, pBuf, 
			(ULONG)CD_RAW_READ * byTransferLen, &byScsiStatus, __FUNCTION__, __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw "";
		}
		for(INT i = CD_RAW_READ * 16; i <= CD_RAW_READ * 21; i += CD_RAW_READ) {
			OutputVolumeRecognitionSequence(i, pBuf, fpLog);
		}
		aCmd[5] = LOBYTE(32);
		if(!ExecCommand(hDevice, aCmd, CDB12GENERIC_LENGTH, pBuf, 
			(ULONG)CD_RAW_READ * byTransferLen, &byScsiStatus, __FUNCTION__, __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw "";
		}
		if(pBuf[20] == 0 && pBuf[21] == 0 && pBuf[22] == 0 && pBuf[23] == 0) {
			for(INT i = 0; i <= CD_RAW_READ * 5; i += CD_RAW_READ) {
				OutputVolumeDescriptorSequence(i, pBuf, fpLog);
			}
		}
		aCmd[4] = HIBYTE(LOWORD(256));
		aCmd[5] = LOBYTE(LOWORD(256));
		if(!ExecCommand(hDevice, aCmd, CDB12GENERIC_LENGTH, pBuf, 
			(ULONG)CD_RAW_READ * byTransferLen, &byScsiStatus, __FUNCTION__, __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw "";
		}
		OutputVolumeDescriptorSequence(0, pBuf, fpLog);
		fflush(fpLog);

		for(INT nLBA = 0; nLBA < nDVDSectorSize; nLBA += byTransferLen) {
			if(nDVDSectorSize - nLBA < byTransferLen) {
				byTransferLen = (UCHAR)(nDVDSectorSize - nLBA);
				aCmd[9] = byTransferLen;
			}
			aCmd[2] = HIBYTE(HIWORD(nLBA));
			aCmd[3] = LOBYTE(HIWORD(nLBA));
			aCmd[4] = HIBYTE(LOWORD(nLBA));
			aCmd[5] = LOBYTE(LOWORD(nLBA));
			if(!ExecCommand(hDevice, aCmd, CDB12GENERIC_LENGTH, pBuf, 
				(ULONG)CD_RAW_READ * byTransferLen, &byScsiStatus, __FUNCTION__, __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw "";
			}

			fwrite(pBuf, sizeof(UCHAR), (size_t)CD_RAW_READ * byTransferLen, fp);
			if(pszOption && !_tcscmp(pszOption, _T("cmi"))) {
				for(INT i = 0; i < byTransferLen; i++) {
					aCmd2[2] = HIBYTE(HIWORD(nLBA + i));
					aCmd2[3] = LOBYTE(HIWORD(nLBA + i));
					aCmd2[4] = HIBYTE(LOWORD(nLBA + i));
					aCmd2[5] = LOBYTE(LOWORD(nLBA + i));
					if(!ExecCommand(hDevice, aCmd2, CDB10GENERIC_LENGTH, 
						pBuf2, (ULONG)uiSize, &byScsiStatus, __FUNCTION__, __LINE__)) {
						throw "";
					}
					if(byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
						OutputErrorStringA(
							"LBA %7d, Read Disc Structure Cmd error\n", nLBA);
					}
					OutputCopyrightManagementInformation(nLBA, i, pBuf2, fpLog);
				}
			}
			printf("\rCreating iso(LBA):%7d/%7d", 
				nLBA + byTransferLen - 1, nDVDSectorSize - 1);
		}
		printf("\n");
	}
	catch(PCHAR str) {
		OutputErrorStringA(str);
		bRet = FALSE;
	}
	FreeAndNull(aBuf);
	FcloseAndNull(fp);
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
	return bRet;
}
#if 0
BOOL ReadDVDRaw(
	HANDLE hDevice,
	LPCSTR pszVendorId,
	LPCTSTR pszOutFile,
	LPCTSTR pszOption,
	INT nDVDSectorSize,
	FILE* fpLog
	)
{
	FILE* fp = CreateOrOpenFileW(pszOutFile, NULL, NULL, _T(".raw"), _T("wb"), 0, 0);
	if(!fp) {
		OutputErrorStringA("Failed to open file .raw [L:%d]\n", __LINE__);
		return FALSE;
	}
	UCHAR byScsiStatus = 0;
	UCHAR byTransferLen = 31;
	PUCHAR aBuf = NULL;
	BOOL bRet = TRUE;
	try {
		if(NULL == (aBuf = (PUCHAR)malloc(DVD_RAW_READ * (size_t)byTransferLen + 0x10))) {
			throw "Can't alloc memory aBuf\n";
		}
		UCHAR aCmd[CDB12GENERIC_LENGTH];
		ZeroMemory(aBuf, DVD_RAW_READ * (size_t)byTransferLen + 0x10);
		PUCHAR pBuf = (PUCHAR)ConvParagraphBoundary(aBuf);
		ZeroMemory(aCmd, sizeof(aCmd));
		UCHAR cdblen = CDB12GENERIC_LENGTH;
		if(pszVendorId && !strncmp(pszVendorId, "PLEXTER", 7)) {
			aCmd[0] = SCSIOP_READ_DATA_BUFF;
			aCmd[1] = 0x02;
			aCmd[2] = 0x00;
			aCmd[6] = LOBYTE(HIWORD(DVD_RAW_READ * (size_t)byTransferLen));
			aCmd[7] = HIBYTE(LOWORD(DVD_RAW_READ * (size_t)byTransferLen));
			aCmd[8] = LOBYTE(LOWORD(DVD_RAW_READ * (size_t)byTransferLen));
			cdblen = CDB10GENERIC_LENGTH;
		}
		else if(pszVendorId && !strncmp(pszVendorId, "HL-DT-ST", 8)) {
			aCmd[0] = 0xE7; // vendor specific command (discovered by DaveX)
			aCmd[1] = 0x48; // H
			aCmd[2] = 0x49; // I
			aCmd[3] = 0x54; // T
			aCmd[4] = 0x01; // read MCU memory sub-command
			aCmd[10] = HIBYTE(LOWORD(DVD_RAW_READ * (size_t)byTransferLen));
			aCmd[11] = LOBYTE(LOWORD(DVD_RAW_READ * (size_t)byTransferLen));
		}

		for(INT nLBA = 0; nLBA < nDVDSectorSize; nLBA += byTransferLen) {
			if(nDVDSectorSize - nLBA < byTransferLen) {
				byTransferLen = (UCHAR)(nDVDSectorSize - nLBA);
				if(pszVendorId && !strncmp(pszVendorId, "PLEXTER", 7)) {
					aCmd[6] = LOBYTE(HIWORD(DVD_RAW_READ * (size_t)byTransferLen));
					aCmd[7] = HIBYTE(LOWORD(DVD_RAW_READ * (size_t)byTransferLen));
					aCmd[8] = LOBYTE(LOWORD(DVD_RAW_READ * (size_t)byTransferLen));
				}
				else if(pszVendorId && !strncmp(pszVendorId, "HL-DT-ST", 8)) {
					aCmd[10] = HIBYTE(LOWORD(DVD_RAW_READ * (size_t)byTransferLen));
					aCmd[11] = LOBYTE(LOWORD(DVD_RAW_READ * (size_t)byTransferLen));
				}
			}
			if(pszVendorId && !strncmp(pszVendorId, "PLEXTER", 7)) {
				aCmd[3] = LOBYTE(HIWORD(nLBA));
				aCmd[4] = HIBYTE(LOWORD(nLBA));
				aCmd[5] = LOBYTE(LOWORD(nLBA));
			}
			else if(pszVendorId && !strncmp(pszVendorId, "HL-DT-ST", 8)) {
				aCmd[6] = HIBYTE(HIWORD(nLBA));
				aCmd[7] = LOBYTE(HIWORD(nLBA));
				aCmd[8] = HIBYTE(LOWORD(nLBA));
				aCmd[9] = LOBYTE(LOWORD(nLBA));
			}
			if(!ExecCommand(hDevice, aCmd, cdblen, pBuf, 
				(ULONG)DVD_RAW_READ * byTransferLen, &byScsiStatus, __FUNCTION__, __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw "";
			}

			fwrite(pBuf, sizeof(UCHAR), (size_t)DVD_RAW_READ * byTransferLen, fp);
			printf("\rCreating raw(LBA):%7d/%7d", 
				nLBA + byTransferLen - 1, nDVDSectorSize - 1);
		}
		printf("\n");
	}
	catch(PCHAR str) {
		OutputErrorStringA(str);
		bRet = FALSE;
	}
	FreeAndNull(aBuf);
	FcloseAndNull(fp);

	unscrambler *u = unscrambler_new ();
	CHAR pszOutFileA[_MAX_PATH];
	ZeroMemory(pszOutFileA, sizeof(pszOutFileA));
	WideCharToMultiByte(CP_THREAD_ACP, 0, pszOutFile, -1, pszOutFileA, _MAX_PATH, NULL, NULL);

	CHAR pszInPath[_MAX_PATH];
	ZeroMemory(pszInPath, sizeof(pszInPath));
	FILE* fpIn = CreateOrOpenFileA(pszOutFileA, pszInPath, NULL, ".raw", "rb", 0, 0);
	if(!fpIn) {
		OutputErrorStringA("Failed to open file .raw [L:%d]\n", __LINE__);
		return FALSE;
	}
	CHAR pszOutPath[_MAX_PATH];
	ZeroMemory(pszOutPath, sizeof(pszOutPath));
	FILE* fpOut = CreateOrOpenFileA(pszOutFileA, pszOutPath, NULL, ".iso", "wb", 0, 0);
	if(!fpOut) {
		OutputErrorStringA("Failed to open file .iso [L:%d]\n", __LINE__);
		return FALSE;
	}
	UINT current_sector = 0;
	unscrambler_set_disctype(3);
	unscrambler_unscramble_file(u, pszInPath, pszOutPath, &current_sector);
	u = (unscrambler *)unscrambler_destroy (u);

#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
	return bRet;
}
#endif
BOOL ReadDVDStructure(
	HANDLE hDevice,
	PINT nDVDSectorSize,
	FILE* fpLog
	)
{
	UCHAR byScsiStatus = 0;
	size_t uiMaxDVDStructureSize = sizeof(DVD_STRUCTURE_LIST_ENTRY) * 0xFF;
	UCHAR pDiscStructure[sizeof(DVD_STRUCTURE_LIST_ENTRY)*0xFF+0x10];
	UCHAR aCmd[CDB12GENERIC_LENGTH];

	ZeroMemory(pDiscStructure, uiMaxDVDStructureSize);
	ZeroMemory(aCmd, sizeof(aCmd));
	aCmd[0] = SCSIOP_READ_DVD_STRUCTURE;
	aCmd[7] = 0xFF;
	aCmd[8] = HIBYTE(uiMaxDVDStructureSize);
	aCmd[9] = LOBYTE(uiMaxDVDStructureSize);

	if(!ExecCommand(hDevice, aCmd, CDB10GENERIC_LENGTH, pDiscStructure, 
		(ULONG)uiMaxDVDStructureSize, &byScsiStatus, __FUNCTION__, __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}

	USHORT usDataLength = MAKEWORD(pDiscStructure[1], pDiscStructure[0]);
	INT nAllStructureSize = usDataLength - 2;
	size_t uiFormatSize = (size_t)(nAllStructureSize / 4);
	if(uiFormatSize == 0) {
		OutputLogStringA(fpLog, "DVD Structure failed. formatSize is 0\n");
		return FALSE;
	}
	PUCHAR pFormat = (PUCHAR)malloc(uiFormatSize);
	if(!pFormat) {
		OutputErrorStringA("Can't alloc memory L:%d", __LINE__);
		return FALSE;
	}
	ZeroMemory(pFormat, uiFormatSize);
	PUSHORT pStructureLength = NULL;
	PUCHAR pStructure = NULL;
	BOOL bRet = TRUE;
	try {
		if(NULL == (pStructureLength =(PUSHORT)malloc(uiFormatSize * sizeof(USHORT)))) {
			throw "Can't alloc memory pStructureLength\n";
		}
		ZeroMemory(pStructureLength, uiFormatSize * sizeof(USHORT));
		OutputLogStringA(fpLog, "DVD Structure\n");
		for(INT i = 0, j = 0; i < nAllStructureSize; i+=4, j++) {
#ifdef _DEBUG
			if(i == 0) {
				OutputLogStringA(fpLog, "\tDisc Structure List\n");
			}
			OutputLogStringA(fpLog, 
				"\t\tFormatCode:%02X, SDR:%s, RDS:%s, Structure Length:%d\n", 
				pDiscStructure[4+i], 
				(pDiscStructure[5+i] & 0x80) == 0x80 ? "Yes" : "No ", 
				(pDiscStructure[5+i] & 0x40) == 0x40 ? "Yes" : "No ", 
				(pDiscStructure[6+i] << 8) | pDiscStructure[7+i]);
#endif
			pFormat[j] = pDiscStructure[4+i];
			pStructureLength[j] = 
				MAKEWORD(pDiscStructure[7+i], pDiscStructure[6+i]);
		}

		for(size_t i = 0; i < uiFormatSize; i++) {
			if(pStructureLength[i] == 0 || pFormat[i] == 0x05 || 
				(0x08 <= pFormat[i] && pFormat[i] <= 0xC0)) {
				continue;
			}
			if(NULL == (pStructure = (PUCHAR)malloc(pStructureLength[i]))) {
				throw "Can't alloc memory pStructure\n";
			}
			ZeroMemory(pStructure, pStructureLength[i]);
			aCmd[7] = pFormat[i];
			aCmd[8] = HIBYTE(pStructureLength[i]);
			aCmd[9] = LOBYTE(pStructureLength[i]);

			if(!ExecCommand(hDevice, aCmd, CDB12GENERIC_LENGTH, pStructure, 
				pStructureLength[i], &byScsiStatus, __FUNCTION__, __LINE__)) {
				throw "";
			}
			if(byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				OutputErrorStringA("Failure - Format %d\n", pFormat[i]);
				continue;
			}
			OutputDVDStructureFormat(pFormat, i, pStructure, pStructureLength, nDVDSectorSize, fpLog);
			FreeAndNull(pStructure);
		}
	}
	catch(PCHAR str) {
		OutputErrorStringA(str);
		bRet = FALSE;
	}
	FreeAndNull(pFormat);
	FreeAndNull(pStructureLength);
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
	return bRet;
}

BOOL ReadTOC(
	HANDLE hDevice,
	PINT nLength,
	FILE* fpLog
	)
{
	UCHAR byScsiStatus = 0;
	UCHAR aCmd[CDB10GENERIC_LENGTH];
	ZeroMemory(aCmd, sizeof(aCmd));
	aCmd[0] = SCSIOP_READ_TOC;
	aCmd[7] = HIBYTE(CDROM_TOC_SIZE);
	aCmd[8] = LOBYTE(CDROM_TOC_SIZE);

	if(!ExecCommand(hDevice, aCmd, CDB10GENERIC_LENGTH, &toc, 
		CDROM_TOC_SIZE, &byScsiStatus, __FUNCTION__, __LINE__) || 
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		OutputErrorStringA("Can't get toc\n");
		return FALSE;
	}

	OutputLogStringA(fpLog, "TOC on SCSIOP_READ_TOC\n");
	CHAR strType[6];
	BOOL bFirstData = TRUE;
	for(INT i = toc.FirstTrack; i <= toc.LastTrack; i++) {
		for(INT j = 0, k = 24; j < 4; j++, k -= 8) {
			aTocLBA[i-1][0] |= toc.TrackData[i-1].Address[j] << k;
			aTocLBA[i-1][1] |= toc.TrackData[i].Address[j] << k;
		}
		aTocLBA[i-1][1] -= 1;
		*nLength += aTocLBA[i-1][1] - aTocLBA[i-1][0] + 1;

		if((toc.TrackData[i-1].Control & AUDIO_DATA_TRACK) == 0) {
			strcpy(strType, "Audio");
		}
		else if((toc.TrackData[i-1].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			if(bFirstData) {
				nFirstDataLBA = aTocLBA[i-1][0];
				bFirstData = FALSE;
			}
			strcpy(strType, " Data");
		}
		OutputLogStringA(fpLog, "\t%s Track %2u, LBA %6u-%6u, Length %6u\n", 
			strType, i, aTocLBA[i-1][0], aTocLBA[i-1][1], 
			aTocLBA[i-1][1] - aTocLBA[i-1][0] + 1);
	}
	OutputLogStringA(fpLog, 
		"\t                                   Total  %6u\n", *nLength);
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
	return TRUE;
}

BOOL ReadTOCFull(
	HANDLE hDevice,
	BOOL bCanCDText,
	FILE* fpLog,
	FILE* fpCcd
	)
{
	UCHAR aCmd[CDB10GENERIC_LENGTH];
	ZeroMemory(aCmd, sizeof(aCmd));
	aCmd[0] = SCSIOP_READ_TOC;
	aCmd[2] = CDROM_READ_TOC_EX_FORMAT_FULL_TOC;
	size_t uiFullTocDataSize = sizeof(CDROM_TOC_FULL_TOC_DATA);
	aCmd[7] = HIBYTE(uiFullTocDataSize);
	aCmd[8] = LOBYTE(uiFullTocDataSize);

	UCHAR byScsiStatus = 0;
	CDROM_TOC_FULL_TOC_DATA fullToc = {0};
	if(!ExecCommand(hDevice, aCmd, CDB10GENERIC_LENGTH, &fullToc,
		(ULONG)uiFullTocDataSize, &byScsiStatus, __FUNCTION__, __LINE__) 
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		OutputErrorStringA("Can't get CDROM_TOC_FULL_TOC_DATA 1\n");
		return FALSE;
	}

	size_t uiFulltocsize = 
		MAKEWORD(fullToc.Length[1], fullToc.Length[0]) - sizeof(fullToc.Length);
	size_t uiTocEntries = uiFulltocsize / sizeof(CDROM_TOC_FULL_TOC_DATA_BLOCK);
	WriteCcdFileForDisc(uiTocEntries, fullToc.LastCompleteSession, fpCcd);
	if(bCanCDText) {
		ReadTOCText(hDevice, fpLog, fpCcd);
	}
	size_t uiFullTocDataMaxSize = uiFulltocsize + uiFullTocDataSize;
	PBYTE pFullToc = (PBYTE)malloc(uiFullTocDataMaxSize + 0x10);
	if(!pFullToc) {
		printf("Can't alloc memory [L:%d]\n", __LINE__);
		return FALSE;
	}
	aCmd[7] = HIBYTE(uiFullTocDataMaxSize);
	aCmd[8] = LOBYTE(uiFullTocDataMaxSize);
	BOOL bRet = TRUE;
	try {
		if(!ExecCommand(hDevice, aCmd, CDB10GENERIC_LENGTH, pFullToc, 
			(ULONG)uiFullTocDataMaxSize, &byScsiStatus, __FUNCTION__, __LINE__) 
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw "Can't get CDROM_TOC_FULL_TOC_DATA 2\n";
		}
		CDROM_TOC_FULL_TOC_DATA_BLOCK* pTocData = 
			((CDROM_TOC_FULL_TOC_DATA*)pFullToc)->Descriptors;
		size_t uiIdx = 0;
		INT aLBA[] = {0, nFirstDataLBA};
		for(size_t b = 0; b < uiTocEntries; b++) {
			if(pTocData[b].Point < 100 && uiIdx < pTocData[b].SessionNumber) {
				UCHAR aBuf2[CD_RAW_SECTOR_WITH_SUBCODE_SIZE+0x10];
				UCHAR aCmd2[CDB12GENERIC_LENGTH];
				ZeroMemory(aBuf2, sizeof(aBuf2));
				PUCHAR pBuf = (PUCHAR)ConvParagraphBoundary(aBuf2);
				ZeroMemory(aCmd2, sizeof(aCmd2));

				aCmd2[0] = SCSIOP_READ_CD;
				aCmd2[1] = READ_CD_FLAG::All;
				aCmd2[8] = 0x01;	// Transfer Length
				aCmd2[9] = READ_CD_FLAG::SyncData | 
					READ_CD_FLAG::SubHeader | READ_CD_FLAG::UserData | 
					READ_CD_FLAG::MainHeader | READ_CD_FLAG::Edc;
				aCmd2[10] = READ_CD_FLAG::PtoW;

				UCHAR ucMode = 0;
				for(INT nLBA =  aLBA[uiIdx]; nLBA < aLBA[uiIdx] + 100; nLBA++) {
					aCmd2[2] = HIBYTE(HIWORD(nLBA));
					aCmd2[3] = LOBYTE(HIWORD(nLBA));
					aCmd2[4] = HIBYTE(LOWORD(nLBA));
					aCmd2[5] = LOBYTE(LOWORD(nLBA));
					if(!ExecCommand(hDevice, aCmd2, CDB12GENERIC_LENGTH, pBuf,
						CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &byScsiStatus, __FUNCTION__, __LINE__)
						|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
						OutputErrorStringA("Can't read CD for MCN\n");
						break;
					}
					if(nLBA ==  aLBA[uiIdx]) {
						ucMode = GetMode(pBuf);
					}
					UCHAR Subcode[CD_RAW_READ_SUBCODE_SIZE];
					UCHAR SubcodeRtoW[CD_RAW_READ_SUBCODE_SIZE];
					AlignSubcode(pBuf + CD_RAW_SECTOR_SIZE, Subcode, SubcodeRtoW);
					UCHAR byAdr = (UCHAR)(Subcode[12] & 0x0F);
					if(byAdr == ADR_ENCODES_MEDIA_CATALOG) {
						BOOL bMCN = IsValidMCN(Subcode);
						CHAR szCatalog[META_CATALOG_SIZE+1];
						SetMCNToString(Subcode, szCatalog, bMCN);
						WriteCcdFileForDiscCatalog(fpCcd);
						break;
					}
				}
				WriteCcdFileForSession(pTocData[b].SessionNumber, fpCcd);
				WriteCcdFileForSessionPregap(ucMode, fpCcd);
				uiIdx++;
			}
		}
		OutputLogStringA(fpLog, "FULL TOC on SCSIOP_READ_TOC\n");
		OutputLogStringA(fpLog, "\tFirstCompleteSession %d\n", 
			fullToc.FirstCompleteSession);
		OutputLogStringA(fpLog, "\tLastCompleteSession %d\n", 
			fullToc.LastCompleteSession);

		for(size_t a = 0; a < uiTocEntries; a++) {
			WriteCcdFileForEntry(a, pTocData, fpCcd);
			switch(pTocData[a].Point) {
			case 0xA0:
				OutputLogStringA(fpLog, "\tSession %d, FirstTrack %d\n", 
					pTocData[a].SessionNumber, pTocData[a].Msf[0]);
				break;
			case 0xA1:
				OutputLogStringA(fpLog, "\tSession %d, LastTrack %d\n", 
					pTocData[a].SessionNumber, pTocData[a].Msf[0]);
				break;
			case 0xA2:
				OutputLogStringA(fpLog, 
					"\tSession %d, Leadout MSF %02d:%02d:%02d\n", 
					pTocData[a].SessionNumber, pTocData[a].Msf[0], 
					pTocData[a].Msf[1], pTocData[a].Msf[2]);
				if(pTocData[a].SessionNumber == 1) {
					nLastLBAof1stSession = 
						MSFtoLBA(pTocData[a].Msf[2], pTocData[a].Msf[1], pTocData[a].Msf[0]) - 150;
				}
				break;
			case 0xB0:
				OutputLogStringA(fpLog, 
					"\tSession %d, NextSession MSF %02d:%02d:%02d, LastWritable MSF %02d:%02d:%02d\n", 
					pTocData[a].SessionNumber, pTocData[a].MsfExtra[0], 
					pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2], 
					pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
				break;
			case 0xC0:
				OutputLogStringA(fpLog, 
					"\tSession %d, WriteLaserOutput %02d, FirstLeadin MSF %02d:%02d:%02d\n", 
					pTocData[a].SessionNumber, pTocData[a].MsfExtra[0],	
					pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
				break;
			default:
				OutputLogStringA(fpLog, 
					"\tSession %d, Track %2d, MSF %02d:%02d:%02d\n", 
					pTocData[a].SessionNumber, pTocData[a].Point, pTocData[a].Msf[0], 
					pTocData[a].Msf[1], pTocData[a].Msf[2]);
				if(pTocData[a].SessionNumber == 2) {
					nStartLBAof2ndSession = MSFtoLBA(pTocData[a].Msf[2], 
						pTocData[a].Msf[1], pTocData[a].Msf[0]) - 150;
				}
				aSessionNum[pTocData[a].Point-1] = pTocData[a].SessionNumber;
				break;
			}
		}
	}
	catch(PCHAR str) {
		OutputErrorStringA(str);
		bRet = FALSE;
	}
	FreeAndNull(pFullToc);
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
	return bRet;
}

BOOL ReadTOCText(
	HANDLE hDevice,
	FILE* fpLog,
	FILE* fpCcd
	)
{
	UCHAR aCmd[CDB10GENERIC_LENGTH];
	ZeroMemory(aCmd, sizeof(aCmd));
	aCmd[0] = SCSIOP_READ_TOC;
	aCmd[2] = CDROM_READ_TOC_EX_FORMAT_CDTEXT;
	size_t uiCDTextDataSize = sizeof(CDROM_TOC_CD_TEXT_DATA);
	aCmd[7] = HIBYTE(uiCDTextDataSize);
	aCmd[8] = LOBYTE(uiCDTextDataSize);
	UCHAR byScsiStatus = 0;
	CDROM_TOC_CD_TEXT_DATA tocText = {0};
	if(!ExecCommand(hDevice, aCmd, CDB10GENERIC_LENGTH, &tocText,
		(ULONG)uiCDTextDataSize, &byScsiStatus, __FUNCTION__, __LINE__) 
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		OutputErrorStringA("Can't get CDROM_TOC_CD_TEXT_DATA\n");
		return FALSE;
	}

	OutputLogStringA(fpLog, "CDTEXT on SCSIOP_READ_TOC\n");
	size_t uiTocTextsize = 
		MAKEWORD(tocText.Length[1], tocText.Length[0]) - sizeof(tocText.Length);
	size_t uiTocTextEntries = uiTocTextsize / sizeof(CDROM_TOC_CD_TEXT_DATA_BLOCK);
	WriteCcdFileForDiscCDTextLength(uiTocTextsize, fpCcd);
	if(!uiTocTextEntries) {
		OutputLogStringA(fpLog, "\tNothing\n");
		// almost CD is nothing text
		return TRUE;
	}
	size_t uiCDTextDataMaxSize = uiTocTextsize + uiCDTextDataSize;
	PBYTE pTocText = (PBYTE)malloc(uiCDTextDataMaxSize + 0x10);
	if(!pTocText) {
		OutputErrorStringA("Can't alloc memory [L:%d]\n", __LINE__);
		return FALSE;
	}
	aCmd[7] = HIBYTE(uiCDTextDataMaxSize);
	aCmd[8] = LOBYTE(uiCDTextDataMaxSize);
	BOOL bRet = TRUE;
	PCD_TEXT_INFO pInfo = NULL;
	CHAR* pTmpText = NULL;
	try {
		if(!ExecCommand(hDevice, aCmd, CDB10GENERIC_LENGTH, pTocText, 
			(ULONG)uiCDTextDataMaxSize, &byScsiStatus, __FUNCTION__, __LINE__) 
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw "Can't get CDROM_TOC_CD_TEXT_DATA_BLOCK\n";
		}
		WriteCcdFileForCDText(uiTocTextEntries, fpCcd);
		if(NULL == (pInfo = (PCD_TEXT_INFO)malloc(uiTocTextEntries * META_STRING_SIZE))) {
			throw "Can't alloc memory PCD_TEXT_INFO\n";
		}
		CDROM_TOC_CD_TEXT_DATA_BLOCK* pDesc = 
			((CDROM_TOC_CD_TEXT_DATA*)pTocText)->Descriptors;
		size_t allTextSize = uiTocTextEntries * sizeof(pDesc->Text);
		if(NULL == (pTmpText = (CHAR*)malloc(allTextSize))) {
			throw "Can't alloc memory CHAR*\n";
		}

		UCHAR byAlbumTitleCnt = 0;
		UCHAR byAlbumCnt = 0;
		UCHAR byPerformerCnt = 0;
		UCHAR bySongwriterCnt = 0;
		UCHAR byComposerCnt = 0;
		UCHAR byArrangerCnt = 0;
		UCHAR byMessagesCnt = 0;
		UCHAR byDiscIdCnt = 0;
		UCHAR byGenreCnt = 0;
		UCHAR byTocInfoCnt = 0;
		UCHAR byTocInfo2Cnt = 0;
		UCHAR byUpcEanCnt = 0;
		UCHAR bySizeInfoCnt = 0;

		UCHAR byAlbumTitleIdx = 0;
		UCHAR byAlbumIdx = 0;
		UCHAR byPerformerIdx = 0;
		UCHAR bySongwriterIdx = 0;
		UCHAR byComposerIdx = 0;
		UCHAR byArrangerIdx = 0;
		UCHAR byMessagesIdx = 0;
		UCHAR byDiscIdIdx = 0;
		UCHAR byGenreIdx = 0;
		UCHAR byTocInfoIdx = 0;
		UCHAR byTocInfo2Idx = 0;
		UCHAR byUpcEanIdx = 0;
		UCHAR bySizeInfoIdx = 0;
		for(size_t t = 0; t < uiTocTextEntries; t++) {
			WriteCcdFileForCDTextEntry(t, pDesc, fpCcd);
			if(pDesc[t].PackType == CDROM_CD_TEXT_PACK_ALBUM_NAME) {
				byAlbumCnt = pDesc[t].TrackNumber;
				byAlbumIdx = pDesc[t].TrackNumber;
			}
			else if(pDesc[t].PackType == CDROM_CD_TEXT_PACK_PERFORMER) {
				if( pDesc[t].Text[0] != 0) {
					byPerformerCnt = pDesc[t].TrackNumber;
					byPerformerIdx = (UCHAR)(pDesc[t].TrackNumber + byAlbumCnt);
					if(byPerformerCnt == 0) {
						byPerformerCnt++;
						byPerformerIdx++;
					}
				}
			}
			else if(pDesc[t].PackType == CDROM_CD_TEXT_PACK_SONGWRITER) {
				bySongwriterCnt = pDesc[t].TrackNumber;
				bySongwriterIdx = 
					(UCHAR)(pDesc[t].TrackNumber + byAlbumCnt + byPerformerCnt);
			}
			else if(pDesc[t].PackType == CDROM_CD_TEXT_PACK_COMPOSER) {
				byComposerCnt = pDesc[t].TrackNumber;
				byComposerIdx = (UCHAR)(pDesc[t].TrackNumber + 
					byAlbumCnt + byPerformerCnt + bySongwriterCnt);
			}
			else if(pDesc[t].PackType == CDROM_CD_TEXT_PACK_ARRANGER) {
				byArrangerCnt = pDesc[t].TrackNumber;
				byArrangerIdx = (UCHAR)(pDesc[t].TrackNumber + 
					byAlbumCnt + byPerformerCnt + bySongwriterCnt + byComposerCnt);
			}
			else if(pDesc[t].PackType == CDROM_CD_TEXT_PACK_MESSAGES) {
				byMessagesCnt = pDesc[t].TrackNumber;
				byMessagesIdx = (UCHAR)(pDesc[t].TrackNumber + byAlbumCnt + 
					byPerformerCnt + bySongwriterCnt + byComposerCnt + byArrangerCnt);
			}
			else if(pDesc[t].PackType == CDROM_CD_TEXT_PACK_DISC_ID) {
				byDiscIdCnt = pDesc[t].TrackNumber;
				byDiscIdIdx = (UCHAR)(pDesc[t].TrackNumber + byAlbumCnt +
					byPerformerCnt + bySongwriterCnt + byComposerCnt + 
					byArrangerCnt + byMessagesCnt);
				if(byDiscIdCnt == 0) {
					byDiscIdCnt++;
					byDiscIdIdx++;
				}
			}
			else if(pDesc[t].PackType == CDROM_CD_TEXT_PACK_GENRE) {
				byGenreCnt = pDesc[t].TrackNumber;
				byGenreIdx = (UCHAR)(pDesc[t].TrackNumber + byAlbumCnt + 
					byPerformerCnt + bySongwriterCnt + byComposerCnt + 
					byArrangerCnt + byMessagesCnt + byDiscIdCnt);
			}
			else if(pDesc[t].PackType == CDROM_CD_TEXT_PACK_TOC_INFO) {
				byTocInfoCnt = pDesc[t].TrackNumber;
				byTocInfoIdx = (UCHAR)(pDesc[t].TrackNumber + byAlbumCnt +
					byPerformerCnt + bySongwriterCnt + byComposerCnt +
					byArrangerCnt + byMessagesCnt + byDiscIdCnt + byGenreCnt);
			}
			else if(pDesc[t].PackType == CDROM_CD_TEXT_PACK_TOC_INFO2) {
				byTocInfo2Cnt = pDesc[t].TrackNumber;
				byTocInfo2Idx = (UCHAR)(pDesc[t].TrackNumber + byAlbumCnt +
					byPerformerCnt + bySongwriterCnt + byComposerCnt + 
					byArrangerCnt + byMessagesCnt + byDiscIdCnt + byGenreCnt +
					byTocInfoCnt);
			}
			else if(pDesc[t].PackType == CDROM_CD_TEXT_PACK_UPC_EAN) {
				byUpcEanCnt = pDesc[t].TrackNumber;
				byUpcEanIdx = (UCHAR)(pDesc[t].TrackNumber + byAlbumCnt +
					byPerformerCnt + bySongwriterCnt + byComposerCnt +
					byArrangerCnt + byMessagesCnt + byDiscIdCnt + byGenreCnt +
					byTocInfoCnt + byTocInfo2Cnt);
			}
			else if(pDesc[t].PackType == CDROM_CD_TEXT_PACK_SIZE_INFO) {
				bySizeInfoCnt = pDesc[t].TrackNumber;
				bySizeInfoIdx = (UCHAR)(pDesc[t].TrackNumber + byAlbumCnt +
					byPerformerCnt + bySongwriterCnt + byComposerCnt +
					byArrangerCnt + byMessagesCnt + byDiscIdCnt + byGenreCnt +
					byTocInfoCnt + byTocInfo2Cnt + byUpcEanCnt);
			}
			memcpy(pTmpText + 12 * t, (CHAR*)(pDesc[t].Text), 12);
		}
		if(byAlbumCnt != 0) {
			byAlbumTitleCnt++;
			byAlbumTitleIdx++;
		}
		size_t uiIdx = 0;
		size_t uiAllSize = (size_t)(byAlbumTitleCnt + byAlbumCnt + byPerformerCnt +
			bySongwriterCnt + byComposerCnt + byArrangerCnt + byMessagesCnt +
			byDiscIdCnt + byGenreCnt + byTocInfoCnt + byTocInfo2Cnt + byUpcEanCnt +
			bySizeInfoCnt);
		INT nTitleCnt = 0;
		for(size_t z = 0; z < uiAllSize; z++) {
			size_t len = strlen(pTmpText + uiIdx);
			if(len == 0) {
				z--;
			}
			else if(len != 0) {
				ZeroMemory(pInfo[z].Text, sizeof(pInfo[z].Text));
				strncpy(pInfo[z].Text, pTmpText + uiIdx, len);
				if(byAlbumTitleIdx != 0 && z < byAlbumTitleIdx) {
					OutputLogStringA(fpLog, "\tAlbumTitle : %s\n", pInfo[z].Text);
					SetAlbumTitle(pInfo[z].Text);
				}
				else if(byAlbumTitleIdx != 0 && byAlbumTitleIdx != byAlbumIdx && 
					byAlbumTitleIdx <= z && z <= byAlbumIdx) {
					OutputLogStringA(fpLog, "\tTitle : %s\n", pInfo[z].Text);
					SetTitle(pInfo[z].Text, nTitleCnt);
					nTitleCnt++;
				}
				else if(byPerformerIdx != 0 && byAlbumIdx != byPerformerIdx && 
					byAlbumIdx <= z && z <= byPerformerIdx) {
					OutputLogStringA(fpLog, "\tPerformer : %s\n", pInfo[z].Text);
					SetPerformer(pInfo[z].Text);
				}
				else if(bySongwriterIdx != 0 && byPerformerIdx != bySongwriterIdx && 
					byPerformerIdx <= z && z <= bySongwriterIdx) {
					OutputLogStringA(fpLog, "\tSongwriter : %s\n", pInfo[z].Text);
					SetSongWriter(pInfo[z].Text);
				}
				else if(byComposerIdx != 0 && bySongwriterIdx != byComposerIdx && 
					bySongwriterIdx <= z && z <= byComposerIdx) {
					OutputLogStringA(fpLog, "\tComposer : %s\n", pInfo[z].Text);
				}
				else if(byArrangerIdx && byComposerIdx != byArrangerIdx && 
					byComposerIdx <= z && z <= byArrangerIdx) {
					OutputLogStringA(fpLog, "\tArranger : %s\n", pInfo[z].Text);
				}
				else if(byMessagesIdx != 0 && byArrangerIdx != byMessagesIdx && 
					byArrangerIdx <= z && z <= byMessagesIdx) {
					OutputLogStringA(fpLog, "\tMessages : %s\n", pInfo[z].Text);
				}
				else if(byDiscIdIdx != 0 && byMessagesIdx != byDiscIdIdx && 
					byMessagesIdx <= z && z <= byDiscIdIdx) {
					OutputLogStringA(fpLog, "\tDiscId : %s\n", pInfo[z].Text);
				}
				else if(byGenreIdx != 0 && byDiscIdIdx != byGenreIdx && 
					byDiscIdIdx <= z && z <= byGenreIdx) {
					OutputLogStringA(fpLog, "\tGenre : %s\n", pInfo[z].Text);
				}
#if 0
				// detail in Page 54-55 of EN 60908:1999
				else if(byTocInfoIdx != 0 && byGenreIdx != byTocInfoIdx && 
					byGenreIdx <= z && z <= byTocInfoIdx) {
					OutputLogStringA(fpLog, "\tTocInfo : %s\n", pInfo[z].Text);
				}
				else if(byTocInfo2Idx != 0 && byTocInfoIdx != byTocInfo2Idx && 
					byTocInfoIdx <= z && z <= byTocInfo2Idx) {
					OutputLogStringA(fpLog, "\tTocInfo2 : %s\n", pInfo[z].Text);
				}
#endif
				else if(byUpcEanIdx != 0 && byTocInfo2Idx != byUpcEanIdx && 
					byTocInfo2Idx <= z && z <= byUpcEanIdx) {
					OutputLogStringA(fpLog, "\tUpcEan : %s\n", pInfo[z].Text);
				}
#if 0
				// detail in Page 56 of EN 60908:1999
				else if(bySizeInfoIdx != 0 && byUpcEanIdx != bySizeInfoIdx && 
					byUpcEanIdx <= z && z <= bySizeInfoIdx) {
					OutputLogStringA(fpLog, "\tSizeInfo : %s\n", pInfo[z].Text);
				}
#endif
			}
			uiIdx += len + 1;
		}
	}
	catch(PCHAR str) {
		OutputErrorStringA(str);
		bRet = FALSE;
	}
	FreeAndNull(pTocText);
	FreeAndNull(pInfo);
	FreeAndNull(pTmpText);
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
	return bRet;
}

BOOL SetCDSpeed(
	HANDLE hDevice,
	INT nCDSpeedNum,
	FILE* fpLog
	)
{
	UCHAR byScsiStatus = 0;
	UCHAR aCDCmd[CDB12GENERIC_LENGTH];
	CDROM_SET_SPEED setspeed;
	USHORT wCDSpeedList[] = {
		CD_RAW_SECTOR_SIZE * 75 * 1 / 1000,		// 176.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 2 / 1000, 	// 352.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 3 / 1000, 	// 529.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 4 / 1000, 	// 705.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 5 / 1000, 	// 882.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 6 / 1000, 	// 1058.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 7 / 1000, 	// 1234.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 8 / 1000, 	// 1411.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 9 / 1000, 	// 1587.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 10 / 1000, 	// 1764.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 11 / 1000, 	// 1940.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 12 / 1000, 	// 2116.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 13 / 1000, 	// 2293.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 14 / 1000, 	// 2469.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 15 / 1000, 	// 2646.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 16 / 1000, 	// 2822.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 17 / 1000, 	// 2998.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 18 / 1000, 	// 3175.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 19 / 1000, 	// 3351.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 20 / 1000, 	// 3528.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 21 / 1000, 	// 3704.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 22 / 1000, 	// 3880.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 23 / 1000, 	// 4057.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 24 / 1000, 	// 4233.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 25 / 1000, 	// 4410.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 26 / 1000, 	// 4586.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 27 / 1000, 	// 4762.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 28 / 1000, 	// 4939.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 29 / 1000, 	// 5115.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 30 / 1000, 	// 5292.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 31 / 1000, 	// 5468.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 32 / 1000, 	// 5644.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 33 / 1000, 	// 5821.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 34 / 1000, 	// 5997.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 35 / 1000, 	// 6174.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 36 / 1000, 	// 6350.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 37 / 1000, 	// 6526.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 38 / 1000, 	// 6703.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 39 / 1000, 	// 6879.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 40 / 1000, 	// 7056.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 41 / 1000, 	// 7232.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 42 / 1000, 	// 7408.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 43 / 1000, 	// 7585.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 44 / 1000, 	// 7761.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 45 / 1000, 	// 7938.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 46 / 1000, 	// 8114.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 47 / 1000, 	// 8290.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 48 / 1000, 	// 8467.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 49 / 1000, 	// 8643.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 50 / 1000, 	// 8820.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 51 / 1000, 	// 8996.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 52 / 1000, 	// 9172.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 53 / 1000, 	// 9349.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 54 / 1000, 	// 9525.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 55 / 1000, 	// 9702.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 56 / 1000, 	// 9878.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 57 / 1000, 	// 10054.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 58 / 1000, 	// 10231.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 59 / 1000, 	// 10407.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 60 / 1000, 	// 10584.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 61 / 1000, 	// 10760.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 62 / 1000, 	// 10936.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 63 / 1000, 	// 11113.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 64 / 1000, 	// 11289.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 65 / 1000, 	// 11466.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 66 / 1000, 	// 11642.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 67 / 1000, 	// 11818.800 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 68 / 1000, 	// 11995.200 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 69 / 1000, 	// 12171.600 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 70 / 1000, 	// 12348.000 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 71 / 1000, 	// 12524.400 Kbytes/sec
		CD_RAW_SECTOR_SIZE * 75 * 72 / 1000, 	// 12700.800 Kbytes/sec
		0xFFFF,									// MAX
	};
	if(nCDSpeedNum < 0) {
		nCDSpeedNum = 0;
	}
	else if(nCDSpeedNum > DRIVE_MAX_SPEED) {
		nCDSpeedNum = DRIVE_MAX_SPEED;
	}
	ZeroMemory(aCDCmd, sizeof(aCDCmd));
	aCDCmd[0] = SCSIOP_SET_CD_SPEED;
	aCDCmd[2] = HIBYTE(wCDSpeedList[nCDSpeedNum]);
	aCDCmd[3] = LOBYTE(wCDSpeedList[nCDSpeedNum]);

	if(!ExecCommand(hDevice, aCDCmd, CDB12GENERIC_LENGTH, &setspeed, 
		sizeof(CDROM_SET_SPEED), &byScsiStatus, __FUNCTION__, __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	OutputLogStringA(fpLog, "Drive speed\n");
	OutputLogStringA(fpLog, "\tRequestType:%s\n",
		setspeed.RequestType == 0 ? "CdromSetSpeed" : "CdromSetStreaming");
	OutputLogStringA(fpLog, "\tReadSpeed:%uKB/sec\n", setspeed.ReadSpeed);
	OutputLogStringA(fpLog, "\tWriteSpeed:%uKB/sec\n", setspeed.WriteSpeed);
	OutputLogStringA(fpLog, "\tRotationControl:%s\n",
		setspeed.RotationControl == 0 ? "CdromDefaultRotation" : "CdromCAVRotation");
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
	return TRUE;
}

BOOL StartStop(
	HANDLE hDevice,
    UCHAR Start,
    UCHAR LoadEject
	)
{
	UCHAR byScsiStatus = 0;
	UCHAR aCmd[CDB6GENERIC_LENGTH];
	ZeroMemory(aCmd, sizeof(aCmd));
	aCmd[0] = SCSIOP_START_STOP_UNIT;
	aCmd[4] = UCHAR(LoadEject << 1 | Start);

	if(!ExecCommand(hDevice, aCmd, CDB6GENERIC_LENGTH, NULL, 0, 
		&byScsiStatus, __FUNCTION__, __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	return TRUE;
}
