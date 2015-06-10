/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"

#define PARAGRAPH_BOUNDARY_NUM 0x0F

inline PVOID ConvParagraphBoundary(
	PVOID pv
	)
{
#ifdef WIN64
	return ((PVOID)(((ULONG_PTR)pv + PARAGRAPH_BOUNDARY_NUM) & 0xFFFFFFFFFFFFFFF0));
#else
	return ((PVOID)(((DWORD)pv + PARAGRAPH_BOUNDARY_NUM) & 0xFFFFFFF0));
#endif
}

BOOL ExecCommand(
	PDEVICE_DATA pDevData,
	PUCHAR pbyCdbCmd,
	UCHAR byCdbCmdLength,
	PVOID pvBuffer,
	ULONG dwBufferLength,
	PUCHAR byScsiStatus,
	LPTSTR pszFuncname,
	INT nLineNum
	)
{
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER swb = {0};

	swb.ScsiPassThroughDirect.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
	swb.ScsiPassThroughDirect.PathId = pDevData->adress.PathId;
	swb.ScsiPassThroughDirect.TargetId = pDevData->adress.TargetId;
	swb.ScsiPassThroughDirect.Lun = pDevData->adress.Lun;
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

	SetLastError(NO_ERROR);

	ULONG ulReturned = 0;
	BOOL bRet = DeviceIoControl(pDevData->hDevice, IOCTL_SCSI_PASS_THROUGH_DIRECT, &swb, 
		ulLength, &swb, ulLength, &ulReturned, NULL);
	*byScsiStatus = swb.ScsiPassThroughDirect.ScsiStatus;

	if(bRet) {
		OutputScsiStatus(&swb, byScsiStatus, pszFuncname, nLineNum);
	}
	else {
		OutputErrorString(
			_T("\nDeviceIoControl with SCSI_PASS_THROUGH_DIRECT command failed [F:%s][L:%d]\n"), 
			pszFuncname, nLineNum);
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

		OutputErrorString(_T("%d, %s\n"), GetLastError(), lpMsgBuf);
		LocalFree(lpMsgBuf);
	}
	return bRet;
}

BOOL ReadVolumeDescriptor(
	PDEVICE_DATA pDevData,
	PCDROM_TOC pToc,
	PUCHAR aCmd,
	ULONG ulBufLen,
	PUCHAR pBuf,
	FILE* fpLog
	)
{
	BOOL bRet = TRUE;
	UCHAR byScsiStatus = 0;
	for(INT n = 0; ; n++) {
		// check VolumeDescriptor
		if((pToc->TrackData[0].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			aCmd[5] = LOBYTE(LOWORD(n));
			if(!ExecCommand(pDevData, aCmd, CDB12GENERIC_LENGTH, pBuf, 
				ulBufLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				bRet = FALSE;
			}
			if(n == 0) {
				if(Is3DOData(pBuf)) {
					break;
				}
				else {
					n += 15;
				}					
			}
			else if(pBuf[15] == DATA_BLOCK_MODE1) {
				OutputVolumeDescriptor(SYNC_SIZE + HEADER_SIZE, pBuf, fpLog);
				if(pBuf[SYNC_SIZE+HEADER_SIZE] == 0xFF) {
					break;
				}
			}
			else if(pBuf[15] == DATA_BLOCK_MODE2) {
				OutputVolumeDescriptor(
					SYNC_SIZE + HEADER_SIZE + SUBHEADER_SIZE, pBuf, fpLog);
				if(pBuf[SYNC_SIZE+HEADER_SIZE+SUBHEADER_SIZE] == 0xFF) {
					break;
				}
			}
		}
		else {
			break;
		}
	}
	return bRet;
}

BOOL CheckCDG(
	PDEVICE_DATA pDevData,
	PUCHAR aCmd,
	ULONG ulBufLen,
	PUCHAR pBuf,
	PBOOL bCDG
	)
{
	BOOL bRet = TRUE;
	UCHAR byScsiStatus = 0;
	aCmd[5] = 75;
	if(!ExecCommand(pDevData, aCmd, CDB12GENERIC_LENGTH, pBuf, 
		ulBufLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		bRet = FALSE;
	}
	else {
		INT nCDG = 0;
		for(INT i = 0; i < 4; i++) {
			nCDG += *(pBuf + CD_RAW_SECTOR_SIZE + i * 24);
		}
		if(nCDG > 0) {
			*bCDG = TRUE;
		}
	}
	return bRet;
}

BOOL UpdateSubchannelQData(
	PSUB_Q_DATA subQ,
	PSUB_Q_DATA prevSubQ,
	PSUB_Q_DATA prevPrevSubQ
	)
{
	if(prevSubQ->byAdr != ADR_ENCODES_MEDIA_CATALOG && 
		prevSubQ->byAdr != ADR_ENCODES_ISRC) {
		prevPrevSubQ->byAdr = prevSubQ->byAdr;
//		prevPrevSubQ->byTrackNum = prevSubQ->byTrackNum;
//		prevPrevSubQ->byIndex = prevSubQ->byIndex;
		prevPrevSubQ->nRelativeTime = prevSubQ->nRelativeTime;
		prevPrevSubQ->byMode = prevSubQ->byMode;
	}
	else if((prevSubQ->byAdr == ADR_ENCODES_MEDIA_CATALOG || 
		prevSubQ->byAdr == ADR_ENCODES_ISRC) && prevSubQ->byIndex == 0) {
			subQ->nRelativeTime = prevSubQ->nRelativeTime + 1;
	}

	if(subQ->byAdr != ADR_ENCODES_MEDIA_CATALOG && 
		subQ->byAdr != ADR_ENCODES_ISRC) {
		prevSubQ->byAdr = subQ->byAdr;
//		prevSubQ->byTrackNum = subQ->byTrackNum;
//		prevSubQ->byIndex = subQ->byIndex;
//		prevSubQ->nRelativeTime = subQ->nRelativeTime;
		prevSubQ->byMode = subQ->byMode;
	}
	else if(prevSubQ->byIndex == 0 && prevSubQ->nRelativeTime == 0) {
		prevSubQ->byIndex = 1;
	}

	prevPrevSubQ->byCtl = prevSubQ->byCtl;
	prevPrevSubQ->nAbsoluteTime = prevSubQ->nAbsoluteTime;
	prevSubQ->byCtl = subQ->byCtl;
	prevSubQ->nAbsoluteTime++;
	prevPrevSubQ->byTrackNum = prevSubQ->byTrackNum;
	prevPrevSubQ->byIndex = prevSubQ->byIndex;
	prevSubQ->byTrackNum = subQ->byTrackNum;
	prevSubQ->byIndex = subQ->byIndex;
	prevSubQ->nRelativeTime = subQ->nRelativeTime;

	return TRUE;
}

BOOL PreserveTrackAttribution(
	PDISC_DATA pDiscData,
	INT nLBA,
	PUCHAR byCurrentTrackNum,
	PSUB_Q_DATA subQ,
	PSUB_Q_DATA prevSubQ,
	PSUB_Q_DATA prevPrevSubQ,
	PUCHAR pCtlList,
	PUCHAR pModeList,
	PINT* pLBAStartList,
	PINT* pLBAOfDataTrackList
	)
{
	// preserve mode, ctl
	if(nLBA == pDiscData->aTocLBA[subQ->byTrackNum-1][0]) {
		pCtlList[subQ->byTrackNum-1] = subQ->byCtl;
		pModeList[subQ->byTrackNum-1] = subQ->byMode;
	}
	// preserve nLBA
	if(prevSubQ->byTrackNum + 1 == subQ->byTrackNum) {
		pLBAStartList[subQ->byTrackNum-1][subQ->byIndex] = nLBA;
		if(subQ->nRelativeTime == 1) {
			// Madou Monogatari I - Honoo no Sotsuenji (Japan)
			// LBA[183031, 0x2CAF7], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-21, Index-01, RelativeTime-00:31:70, AbsoluteTime-40:42:31] RtoW:ZERO mode
			// LBA[183032, 0x2CAF8], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number (MCN)[0000000000000        , AbsoluteTime-     :32] RtoW:ZERO mode
			// LBA[183033, 0x2CAF9], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-22, Index-01, RelativeTime-00:00:01, AbsoluteTime-40:42:33] RtoW:ZERO mode
			pLBAStartList[subQ->byTrackNum-1][subQ->byIndex] -= 1;
		}
		// preserve end lba of data track
		if(pLBAOfDataTrackList[prevSubQ->byTrackNum-1][0] != -1 &&
			pLBAOfDataTrackList[prevSubQ->byTrackNum-1][1] == -1 &&
			(prevSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				pLBAOfDataTrackList[prevSubQ->byTrackNum-1][1] = nLBA - 1;
		}
		// preserve first lba of data track
		if(pLBAOfDataTrackList[subQ->byTrackNum-1][0] == -1 &&
			(subQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				pLBAOfDataTrackList[subQ->byTrackNum-1][0] = nLBA;
		}
		*byCurrentTrackNum = subQ->byTrackNum;
	}
	// Hatsukoi Monotagari (Japan)
	if(prevSubQ->byIndex + 1 == subQ->byIndex && 
		(prevSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK &&
		(subQ->byCtl & AUDIO_DATA_TRACK) == 0) {
		pLBAOfDataTrackList[subQ->byTrackNum-1][subQ->byIndex] = nLBA - 1;
	}
	// Hyper Wars (Japan)
	// LBA[098484, 0x180B4], Data, Copy NG, TOC[TrackNum-03, Index-00, RelativeTime-00:02:01, AbsoluteTime-21:55:09] RtoW:ZERO mode
	// LBA[098485, 0x180B5], Data, Copy NG, TOC[TrackNum-03, Index-00, RelativeTime-00:02:00, AbsoluteTime-21:55:10] RtoW:ZERO mode
	// LBA[098486, 0x180B6], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-03, Index-00, RelativeTime-00:01:74, AbsoluteTime-21:55:11] RtoW:ZERO mode
	// LBA[098487, 0x180B7], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-03, Index-00, RelativeTime-00:01:73, AbsoluteTime-21:55:12] RtoW:ZERO mode
	if((prevPrevSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK &&
		(prevSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK &&
		(subQ->byCtl & AUDIO_DATA_TRACK) == 0 &&
			subQ->byIndex == 0 && prevSubQ->byIndex == 0 && prevPrevSubQ->byIndex == 0 
		) {
		pLBAOfDataTrackList[subQ->byTrackNum-1][subQ->byIndex+1] = nLBA - 1;
	}
	// preserve index
	if(prevSubQ->byIndex + 1 == subQ->byIndex && 
		*byCurrentTrackNum != pDiscData->toc.FirstTrack) {
		pLBAStartList[subQ->byTrackNum-1][subQ->byIndex] = nLBA;
	}
	// preserve first lba of data track offset
	if(pLBAOfDataTrackList[subQ->byTrackNum-1][0] == -1 &&
		(subQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			pLBAOfDataTrackList[subQ->byTrackNum-1][0] = nLBA;
	}
	// preserve end data track offset
	else if(nLBA == pDiscData->nLength - 1) {
		// preserve end data track offset
		if(pLBAOfDataTrackList[subQ->byTrackNum-1][0] != -1 &&
			pLBAOfDataTrackList[subQ->byTrackNum-1][1] == -1 &&
			(subQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				pLBAOfDataTrackList[subQ->byTrackNum-1][1] = pDiscData->nLength - 1;
		}
	}
	return TRUE;
}

BOOL DescrambleMainChannel(
	PDISC_DATA pDiscData,
	PINT* pLBAOfDataTrackList,
	FILE* fpTbl,
	FILE* fpImg,
	FILE* fpLog
	)
{
	BOOL bRet = TRUE;
	UCHAR aScrambledBuf[CD_RAW_SECTOR_SIZE] = {0};
	fread(aScrambledBuf, sizeof(UCHAR), sizeof(aScrambledBuf), fpTbl);
	UCHAR aSrcBuf[CD_RAW_SECTOR_SIZE] = {0};

	for(INT k = 0; k < pDiscData->toc.LastTrack; k++) {
		if(pLBAOfDataTrackList[k][0] != -1) {
			OutputLogString(fpLog, _T("\tData Sector, LBA %6d-%6d\n"), 
				pLBAOfDataTrackList[k][0], pLBAOfDataTrackList[k][1]);
			INT nStartLBA = pDiscData->aSessionNum[k] >= 2 
				? pLBAOfDataTrackList[k][0] - (11400 * (pDiscData->aSessionNum[k] - 1)) 
				: pLBAOfDataTrackList[k][0];
			INT nEndLBA = pDiscData->aSessionNum[k] >= 2 
				? pLBAOfDataTrackList[k][1] - (11400 * (pDiscData->aSessionNum[k] - 1)) 
				: pLBAOfDataTrackList[k][1];
			for(; nStartLBA <= nEndLBA; nStartLBA++) {
				// ファイルを読み書き両用モードで開いている時は 注意が必要です。
				// 読み込みを行った後に書き込みを行う場合やその逆を行う場合は、 
				// 必ずfseekを呼ばなければなりません。もしこれを忘れると、
				// 場合によってはバッファー内と 実際にディスクに描き込まれた
				// データに矛盾が生じ、正確に書き込まれない場合や、
				// 嘘の データを読み込む場合があります。
				fseek(fpImg, nStartLBA * CD_RAW_SECTOR_SIZE, SEEK_SET);
				fread(aSrcBuf, sizeof(UCHAR), sizeof(aSrcBuf), fpImg);
				if(IsValidDataHeader(aSrcBuf)) {
					fseek(fpImg, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
					for(INT n = 0; n < CD_RAW_SECTOR_SIZE; n++) {
						aSrcBuf[n] ^= aScrambledBuf[n];
					}
					fwrite(aSrcBuf, sizeof(UCHAR), sizeof(aSrcBuf), fpImg);
				}
				OutputString(_T("\rDescrambling data sector of img(LBA) %6d/%6d"), nStartLBA, nEndLBA);
			}
			OutputString(_T("\n"));
		}
	}
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
	return bRet;
}

BOOL MergeMainChannelAndCDG(
	LPCTSTR pszOutFile,
	BOOL bCDG,
	BOOL bAudioOnly,
	FILE* fpImg
	)
{
	BOOL bRet = TRUE;
	if(bCDG && bAudioOnly) {
		OutputString(_T("Merging img+cdg->bin\n"));
		FILE* fpCdg = CreateOrOpenFileW(pszOutFile, NULL, NULL, NULL, _T(".cdg"), _T("rb"), 0, 0);
		if(fpCdg == NULL) {
			OutputErrorString(_T("Failed to open file .cdg\n"));
			bRet = FALSE;
		}
		FILE* fpBinAll = CreateOrOpenFileW(pszOutFile, NULL, NULL, NULL, _T(".bin"), _T("wb"), 0, 0);
		if(fpBinAll == NULL) {
			OutputErrorString(_T("Failed to open file .bin\n"));
			bRet = FALSE;
		}
		if(bRet) {
			LONG fsizeImg2 = (LONG)GetFilesize(fpImg, 0);
			UCHAR buf[CD_RAW_SECTOR_WITH_SUBCODE_SIZE] = {0};

			for(int i = 0; i < fsizeImg2 / CD_RAW_SECTOR_SIZE; i++) {
				fread(buf, sizeof(UCHAR), CD_RAW_SECTOR_SIZE, fpImg);
				fread(buf + CD_RAW_SECTOR_SIZE, sizeof(UCHAR), CD_RAW_READ_SUBCODE_SIZE, fpCdg);
				fwrite(buf, sizeof(UCHAR), CD_RAW_SECTOR_WITH_SUBCODE_SIZE, fpBinAll);
			}
			FcloseAndNull(fpBinAll);
			FcloseAndNull(fpCdg);
		}
	}
	return bRet;
}

BOOL CreatingBinCueCcd(
	PDISC_DATA pDiscData,
	LPCTSTR pszOutFile,
	BOOL bCatalog,
	BOOL bCDG,
	PUCHAR pCtlList,
	PUCHAR pModeList,
	PBOOL pbISRCList,
	PINT* pLBAStartList,
	FILE* fpImg,
	FILE* fpCue,
	FILE* fpCcd
	)
{
	BOOL bRet = TRUE;
	TCHAR pszFileNameWithoutPath[_MAX_FNAME] = {0};
	FILE* fpBinWithCDG = NULL;
	if(bCDG && pDiscData->bAudioOnly) {
		fpBinWithCDG = 
			CreateOrOpenFileW(pszOutFile, NULL, NULL, NULL, _T(".bin"), _T("rb"), 0, 0);
		if(fpBinWithCDG == NULL) {
			OutputErrorString(_T("Failed to open file .bin\n"));
			return FALSE;
		}
	}
	for(INT i = pDiscData->toc.FirstTrack; i <= pDiscData->toc.LastTrack; i++) {
		FILE* fpBin = CreateOrOpenFileW(pszOutFile, NULL, 
			pszFileNameWithoutPath, NULL, _T(".bin"), _T("wb"), i, pDiscData->toc.LastTrack);
		if(fpBin == NULL) {
			OutputErrorString(_T("Failed to open .bin\n"));
			bRet = FALSE;
			break;
		}
		else {
			OutputString(_T("\rCreating bin, cue, ccd(Track) %2d/%2d"), i, pDiscData->toc.LastTrack);
			WriteCueFile(bCatalog, bCDG, pszFileNameWithoutPath, 
				i, pModeList[i-1], pbISRCList[i-1], pCtlList[i-1], fpCue);
			bCatalog = FALSE;
			WriteCcdFileForTrack(i, pModeList[i-1], pbISRCList[i-1], fpCcd);
			if(i == pDiscData->toc.FirstTrack) {
				WriteCueFileForIndex(1, 0, 0, 0, fpCue);
				WriteCcdFileForTrackIndex(1, 0, fpCcd);
			}
			// only 1 index
			else if(pLBAStartList[i-1][0] == -1 && pLBAStartList[i-1][2] == -1) {
				WriteCueFileForIndex(1, 0, 0, 0, fpCue);
				WriteCcdFileForTrackIndex(1, pLBAStartList[i-1][1], fpCcd);
			}
			else if(pLBAStartList[i-1][0] != -1) {
				for(UCHAR index = 0; index < MAXIMUM_NUMBER_INDEXES; index++) {
					BYTE byFrame = 0, bySecond = 0, byMinute = 0;
					if(pLBAStartList[i-1][index] != -1) {
						LBAtoMSF(pLBAStartList[i-1][index] - pLBAStartList[i-1][0], 
							&byFrame, &bySecond, &byMinute);
						WriteCueFileForIndex(index, byFrame, bySecond, byMinute, fpCue);
						WriteCcdFileForTrackIndex(index, pLBAStartList[i-1][index], fpCcd);
					}
				}
			}
			else {
				for(UCHAR index = 1; index < MAXIMUM_NUMBER_INDEXES; index++) {
					BYTE byFrame = 0, bySecond = 0, byMinute = 0;
					if(pLBAStartList[i-1][index] != -1) {
						LBAtoMSF(pLBAStartList[i-1][index] - pLBAStartList[i-1][1], 
							&byFrame, &bySecond, &byMinute);
						WriteCueFileForIndex(index, byFrame, bySecond, byMinute, fpCue);
						WriteCcdFileForTrackIndex(index, pLBAStartList[i-1][index], fpCcd);
					}
				}
			}
			// write each track
			size_t uiBufsize = 0;
			INT nLBA = pLBAStartList[i][0] == -1 ? pLBAStartList[i][1] : pLBAStartList[i][0];
			INT nPrevLba = pLBAStartList[i-1][0] == -1 ? pLBAStartList[i-1][1] : pLBAStartList[i-1][0];
			INT nWriteSectorSize = 
				(bCDG && pDiscData->bAudioOnly) ? CD_RAW_SECTOR_WITH_SUBCODE_SIZE : CD_RAW_SECTOR_SIZE;

			if(pDiscData->toc.LastTrack == pDiscData->toc.FirstTrack) {
				uiBufsize = (size_t)pDiscData->nLength * nWriteSectorSize;
				nPrevLba = 0;
			}
			else if(i == pDiscData->toc.FirstTrack) {
				uiBufsize = (size_t)nLBA * nWriteSectorSize;
				nPrevLba = 0;
			}
			else if(i == pDiscData->toc.LastTrack) {
				INT nTmpLength = pDiscData->nLength;
				if(pDiscData->aSessionNum[i-1] > 1) {
					INT nLeadinoutSize = 11400 * (pDiscData->aSessionNum[i-1] - 1);
					nPrevLba -= nLeadinoutSize;
					nTmpLength -= nLeadinoutSize;
				}
				uiBufsize = (size_t)(nTmpLength - nPrevLba) * nWriteSectorSize;
			}
			else {
				if(i == pDiscData->toc.LastTrack - 1 && pDiscData->aSessionNum[i] > 1) {
					nLBA -= 11400 * (pDiscData->aSessionNum[i] - 1);
				}
				uiBufsize = (size_t)(nLBA - nPrevLba) * nWriteSectorSize;
			}
			if(!(bCDG && pDiscData->bAudioOnly)) {
				fseek(fpImg, nPrevLba * nWriteSectorSize, SEEK_SET);
			}
			PUCHAR pBuf = (PUCHAR)malloc(uiBufsize);
			if(pBuf == NULL) {
				OutputErrorString(_T("Failed to alloc memory pBuf2\n"));
				bRet = FALSE;
			}
			else {
				if(bCDG && pDiscData->bAudioOnly) {
					fread(pBuf, sizeof(UCHAR), uiBufsize, fpBinWithCDG);
				}
				else {
					fread(pBuf, sizeof(UCHAR), uiBufsize, fpImg);
				}
				fwrite(pBuf, sizeof(UCHAR), uiBufsize, fpBin);
				FreeAndNull(pBuf);
				FcloseAndNull(fpBin);
			}
		}
	}
	OutputString(_T("\n"));
	return bRet;
}

BOOL ReadCDAll(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	LPCTSTR pszOutFile,
	FILE* fpLog,
	FILE* fpCcd
	)
{
	if(pDiscData->toc.FirstTrack < 1 || 99 < pDiscData->toc.FirstTrack ||
		pDiscData->toc.LastTrack < 1 || 99 < pDiscData->toc.LastTrack) {
		return FALSE;
	}
	FILE* fpImg = NULL;
	if(NULL == (fpImg = CreateOrOpenFileW(pszOutFile, NULL, NULL, NULL, _T(".img"), _T("wb"), 0, 0))) {
		return FALSE;
	}

	FILE* fpC2 = NULL;
	FILE* fpCdg = NULL;
	FILE* fpCue = NULL;
	FILE* fpParse = NULL;
	FILE* fpSub = NULL;
	FILE* fpTbl = NULL;
	PINT* pLBAStartList = NULL;
	PINT* pLBAOfDataTrackList = NULL;
	PUCHAR pCtlList = NULL;
	PUCHAR pEndCtlList = NULL;
	PUCHAR pModeList = NULL;
	PBOOL pbISRCList = NULL;
	PUCHAR aBuf = NULL;
	BOOL bRet = TRUE;
	try {
		// init start
		if(NULL == (fpSub = CreateOrOpenFileW(pszOutFile, NULL, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
			throw _T("Failed to open .sub\n");
		}
		if(NULL == (fpCue = CreateOrOpenFileW(pszOutFile, NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
			throw _T("Failed to open .cue\n");
		}
		if(NULL == (fpParse = CreateOrOpenFileW(pszOutFile, NULL, NULL, NULL, _T(".sub.txt"), _T(WFLAG), 0, 0))) {
			throw _T("Failed to open .sub.txt\n");
		}

		if(NULL == (fpTbl = OpenProgrammabledFile(_T("scramble.bin"), _T("rb")))) {
			throw _T("Failed to open scramble.bin\n");
		}
		size_t dwTrackAllocSize = (size_t)pDiscData->toc.LastTrack + 1;
		size_t dwTrackPointerAllocSize = dwTrackAllocSize * sizeof(_INT);

		if(NULL == (pLBAStartList = (PINT*)malloc(dwTrackPointerAllocSize))) {
			throw _T("Failed to alloc memory pLBAStartList\n");
		}
		if(NULL == (pLBAOfDataTrackList = (PINT*)malloc(dwTrackPointerAllocSize))) {
			throw _T("Failed to alloc memory pLBAOfDataTrackList\n");
		}
		if(NULL == (pCtlList = (PUCHAR)malloc(dwTrackAllocSize))) {
			throw _T("Failed to alloc memory pCtlList\n");
		}
		if(NULL == (pModeList = (PUCHAR)malloc(dwTrackAllocSize))) {
			throw _T("Failed to alloc memory pModeList\n");
		}
		if(NULL == (pbISRCList = (BOOL*)malloc(dwTrackPointerAllocSize))) {
			throw _T("Failed to alloc memory pbISRCList\n");
		}
		size_t dwIndexAllocSize = (size_t)MAXIMUM_NUMBER_INDEXES * sizeof(_INT);
		size_t dwRangeAllocSize = (size_t)2 * sizeof(_INT);

		for(INT h = 0; h < pDiscData->toc.LastTrack + 1; h++) {
			if(NULL == (pLBAStartList[h] = (PINT)malloc(dwIndexAllocSize))) {
				throw _T("Failed to alloc memory pLBAStartList[h]\n");
			}
			if(NULL == (pLBAOfDataTrackList[h] = (PINT)malloc(dwRangeAllocSize))) {
				throw _T("Failed to alloc memory pLBAOfDataTrackList[h]\n");
			}
			FillMemory(pLBAStartList[h], dwIndexAllocSize, -1);
			FillMemory(pLBAOfDataTrackList[h], dwRangeAllocSize, -1);
		}
		ZeroMemory(pCtlList, dwTrackAllocSize);
		ZeroMemory(pModeList, dwTrackAllocSize);
		ZeroMemory(pbISRCList, dwTrackPointerAllocSize);

		ULONG ulBufLen = CD_RAW_SECTOR_WITH_SUBCODE_SIZE;
		if(pDiscData->bC2ErrorData && strncmp(pDiscData->pszVendorId, "PLEXTOR", 7)) {
			if(NULL == (fpC2 = CreateOrOpenFileW(pszOutFile, NULL, NULL, NULL, _T(".c2.err"), _T("wb"), 0, 0))) {
				throw _T("Failed to open .c2.err\n");
			}
			ulBufLen = CD_RAW_SECTOR_WITH_C2_AND_SUBCODE_SIZE;
		}

		if(NULL == (aBuf = (PUCHAR)malloc(ulBufLen + PARAGRAPH_BOUNDARY_NUM))) {
			throw _T("Failed to alloc memory aBuf[h]\n");
		}
		ZeroMemory(aBuf, ulBufLen + PARAGRAPH_BOUNDARY_NUM);
		PUCHAR pBuf = (PUCHAR)ConvParagraphBoundary(aBuf);
		UCHAR aCmd[CDB12GENERIC_LENGTH] = {0};
		UCHAR byTransferLen = 1;
		aCmd[0] = SCSIOP_READ_CD;
		aCmd[1] = READ_CD_FLAG::All;
		aCmd[8] = byTransferLen;
		aCmd[9] = READ_CD_FLAG::SyncData | READ_CD_FLAG::SubHeader | 
			READ_CD_FLAG::UserData | READ_CD_FLAG::MainHeader | READ_CD_FLAG::Edc;
		if(strncmp(pDiscData->pszVendorId, "PLEXTOR", 7)) {
			aCmd[9] |= READ_CD_FLAG::C2ErrorBlockData;
		}
		aCmd[10] = READ_CD_FLAG::PtoW;

		ReadVolumeDescriptor(pDevData, &pDiscData->toc, aCmd, ulBufLen, pBuf, fpLog);
		BOOL bCDG = FALSE;
		CheckCDG(pDevData, aCmd, ulBufLen, pBuf, &bCDG);
		if(bCDG) {
			if(NULL == (fpCdg = CreateOrOpenFileW(pszOutFile, NULL, NULL, NULL, _T(".cdg"), _T("wb"), 0, 0))) {
				throw _T("Failed to open .cdg\n");
			}
		}
		if(NULL == (pEndCtlList = (PUCHAR)malloc(dwTrackAllocSize))) {
			throw _T("Failed to alloc memory pEndCtlList\n");
		}
		ZeroMemory(pEndCtlList, dwTrackAllocSize);
		if(!strncmp(pDiscData->pszVendorId, "PLEXTOR", 7)) {
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

		UCHAR byScsiStatus = 0;
		UCHAR Subcode[CD_RAW_READ_SUBCODE_SIZE] = {0};
		for(INT p = 0; p < pDiscData->toc.LastTrack; p++) {
			aCmd[2] = HIBYTE(HIWORD(pDiscData->aTocLBA[p][0]));
			aCmd[3] = LOBYTE(HIWORD(pDiscData->aTocLBA[p][0]));
			aCmd[4] = HIBYTE(LOWORD(pDiscData->aTocLBA[p][0]));
			aCmd[5] = LOBYTE(LOWORD(pDiscData->aTocLBA[p][0]));
			if(!ExecCommand(pDevData, aCmd, CDB12GENERIC_LENGTH, 
				pBuf, ulBufLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw _T("");
			}
			AlignRowSubcode(pBuf + CD_RAW_SECTOR_SIZE, Subcode);
			pEndCtlList[p] = (UCHAR)((Subcode[12] >> 4) & 0x0F);
		}

		size_t uiShift = 0;
		INT nOffsetStart = 0;
		INT nOffsetEnd = 0;
		INT nFixStartLBA = 0;
		INT nFixEndLBA = 0;
		SUB_Q_DATA prevPrevSubQ = {0};
		SUB_Q_DATA prevSubQ = {0};
		if(pDiscData->nCombinedOffset > 0) {
			uiShift = (size_t)pDiscData->nCombinedOffset % CD_RAW_SECTOR_SIZE;
			nOffsetStart = 0;
			nOffsetEnd = pDiscData->nAdjustSectorNum;
			nFixStartLBA = pDiscData->nAdjustSectorNum - 1;
			nFixEndLBA = pDiscData->nAdjustSectorNum;
			prevSubQ.nRelativeTime = 0;
			prevSubQ.nAbsoluteTime = 149;
		}
		else if(pDiscData->nCombinedOffset < 0) {
			uiShift = (size_t)CD_RAW_SECTOR_SIZE + (pDiscData->nCombinedOffset % CD_RAW_SECTOR_SIZE);
			nOffsetStart = pDiscData->nAdjustSectorNum;
			nOffsetEnd = 0;
			nFixStartLBA = pDiscData->nAdjustSectorNum;
			nFixEndLBA = pDiscData->nAdjustSectorNum + 1;
			prevSubQ.nRelativeTime = 150 + pDiscData->nAdjustSectorNum - 1;
			prevSubQ.nAbsoluteTime = 150 + pDiscData->nAdjustSectorNum - 1;
		}
		prevSubQ.byCtl = pEndCtlList[0];
		prevSubQ.byAdr = ADR_ENCODES_CURRENT_POSITION;
		prevSubQ.byTrackNum = pDiscData->toc.FirstTrack;
		prevSubQ.byIndex = nOffsetStart < 0 ? (UCHAR)0 : (UCHAR)1;
		prevSubQ.byMode = DATA_BLOCK_MODE0;
		// init end

		BOOL bCatalog = FALSE;
		UCHAR byCurrentTrackNum = pDiscData->toc.FirstTrack;
		OutputLogString(fpLog, 
			_T("\n--------------------------------error log begin--------------------------------\n"));

		for(INT nLBA = nOffsetStart; nLBA < pDiscData->nLength + nOffsetEnd; nLBA++) {
			if(pDiscData->nStartLBAof2ndSession != -1 && pDiscData->nLastLBAof1stSession == nLBA) {
				INT nStartSession2 = pDiscData->nStartLBAof2ndSession - 1;
				OutputLogString(fpLog, 
					_T("Skip from Leadout of Session 1 [%d]"), pDiscData->nLastLBAof1stSession);
				OutputLogString(fpLog, 
					_T(" to Leadin of Session 2 [%d]\n"), nStartSession2);
				nLBA = nStartSession2;
				prevSubQ.nAbsoluteTime = nLBA + 150;
				continue;
			}

			// 1st:Read main & sub channel
			aCmd[2] = HIBYTE(HIWORD(nLBA));
			aCmd[3] = LOBYTE(HIWORD(nLBA));
			aCmd[4] = HIBYTE(LOWORD(nLBA));
			aCmd[5] = LOBYTE(LOWORD(nLBA));
			if(!ExecCommand(pDevData, aCmd, CDB12GENERIC_LENGTH, 
				pBuf, ulBufLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)) {
				throw _T("");
			}

			if(byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				size_t uiSize = 0;
				ZeroMemory(pBuf, ulBufLen);
				if(nLBA == nFixStartLBA) {
					uiSize = CD_RAW_SECTOR_SIZE - uiShift;
					fwrite(pBuf + uiShift, sizeof(UCHAR), uiSize, fpImg);
				}
				else if(nLBA == pDiscData->nLength + nFixEndLBA - 1) {
					uiSize = uiShift;
					fwrite(pBuf, sizeof(UCHAR), uiSize, fpImg);
				}
				else {
					uiSize = CD_RAW_SECTOR_SIZE;
					fwrite(pBuf, sizeof(UCHAR), uiSize, fpImg);
				}
				OutputErrorString(
					_T("LBA %d Read error [L:%d]. Zero padding [%dbyte]\n"), 
					nLBA, __LINE__, uiSize);
				prevSubQ.nAbsoluteTime++;
				continue;
			}
			AlignRowSubcode(pBuf + CD_RAW_SECTOR_SIZE, Subcode);

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

			if(0 <= nLBA && nLBA < pDiscData->nLength) {
				// 2nd:Verification subchannel
				CheckAndFixSubchannel(&pDiscData->toc, nLBA, Subcode, &subQ, &prevSubQ, &prevPrevSubQ,
					&byCurrentTrackNum, &bCatalog, pbISRCList, pEndCtlList, pLBAStartList, pLBAOfDataTrackList, fpLog);
				UCHAR SubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = {0};
				// fix raw subchannel
				AlignColumnSubcode(Subcode, SubcodeRaw);
				// 3rd:Write subchannel
				WriteSubChannel(nLBA, byCurrentTrackNum, pBuf, Subcode, SubcodeRaw, fpSub, fpParse, fpCdg);
				PreserveTrackAttribution(pDiscData, nLBA, &byCurrentTrackNum, &subQ,
					&prevSubQ, &prevPrevSubQ, pCtlList, pModeList, pLBAStartList, pLBAOfDataTrackList);
			}

			// 4th:Write track to scrambled
			WriteMainChannel(pDiscData, nLBA, nFixStartLBA, nFixEndLBA, uiShift, pLBAStartList, pBuf, fpImg);
			if(strncmp(pDiscData->pszVendorId, "PLEXTOR", 7)) {
				fwrite(pBuf + CD_RAW_SECTOR_WITH_SUBCODE_SIZE, 
					sizeof(UCHAR), CD_RAW_READ_C2_SIZE, fpC2);
			}
			UpdateSubchannelQData(&subQ, &prevSubQ, &prevPrevSubQ);
			OutputString(_T("\rCreating img(LBA) %6d/%6d"), nLBA - nOffsetEnd, pDiscData->nLength - 1);
		}
		OutputString(_T("\n"));
		FcloseAndNull(fpCdg);
		FcloseAndNull(fpImg);
		FcloseAndNull(fpSub);
		if(strncmp(pDiscData->pszVendorId, "PLEXTOR", 7)) {
			FcloseAndNull(fpC2);
		}
		FcloseAndNull(fpParse);
		FreeAndNull(aBuf);

		OutputLogString(fpLog, 
			_T("---------------------------------error log end---------------------------------\n\n"));
		OutputLogString(fpLog, _T("TOC on Subchannel\n"));
		for(INT r = 0; r < pDiscData->toc.LastTrack; r++) {
			OutputLogString(fpLog, 
				_T("\tTrack %2d, Ctl %d, Mode %d"), r + 1, pCtlList[r], pModeList[r]);
			for(INT k = 0; k < MAXIMUM_NUMBER_INDEXES; k++) {
				if(pLBAStartList[r][k] != -1) {
					OutputLogString(fpLog, _T(", Index%d %6d"), k, pLBAStartList[r][k]);
				}
				else if(k == 0) {
					OutputLogString(fpLog, _T(",              "));
				}
			}
			OutputLogString(fpLog, _T("\n"));
		}
		// 5th:Descramble img
		if(NULL == (fpImg = CreateOrOpenFileW(pszOutFile, NULL, NULL, NULL, _T(".img"), _T("rb+"), 0, 0))) {
			throw _T("Failed to open file .img\n");
		}
		if(!DescrambleMainChannel(pDiscData, pLBAOfDataTrackList, fpTbl, fpImg, fpLog)) {
			throw _T("");
		}
		FcloseAndNull(fpTbl);

		if(!MergeMainChannelAndCDG(pszOutFile, bCDG, pDiscData->bAudioOnly, fpImg)) {
			throw _T("");
		}

		// 6th:Creating bin, cue, ccd
		if(!CreatingBinCueCcd(pDiscData, pszOutFile, bCatalog, bCDG, 
			 pCtlList, pModeList, pbISRCList, pLBAStartList, fpImg, fpCue, fpCcd)) {
			throw _T("");
		}
	}
	catch(LPTSTR str) {
		OutputErrorString(str);
		bRet = FALSE;
	}
#ifndef _DEBUG
	fflush(fpLog);
#endif
	FcloseAndNull(fpImg);
	FcloseAndNull(fpC2);
	FcloseAndNull(fpCdg);
	FcloseAndNull(fpCue);
	FcloseAndNull(fpParse);
	FcloseAndNull(fpSub);
	FcloseAndNull(fpTbl);
	for(INT h = 0; h < pDiscData->toc.LastTrack + 1; h++) {
		FreeAndNull(pLBAStartList[h]);
		FreeAndNull(pLBAOfDataTrackList[h]);
	}
	FreeAndNull(pLBAStartList);
	FreeAndNull(pLBAOfDataTrackList);
	FreeAndNull(pEndCtlList);
	FreeAndNull(pCtlList);
	FreeAndNull(pModeList);
	FreeAndNull(pbISRCList);
	FreeAndNull(aBuf);
	return bRet;
}

BOOL ReadCDForSearchingOffset(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	FILE* fpLog
	)
{
	pDiscData->bAudioOnly = TRUE;
	if((pDiscData->toc.TrackData[0].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
		pDiscData->bAudioOnly = FALSE;
	}
	else {
		for(INT i = pDiscData->toc.FirstTrack + 1; i <= pDiscData->toc.LastTrack; i++) {
			if(pDiscData->toc.TrackData[0].Control != pDiscData->toc.TrackData[i-1].Control) {
				pDiscData->bAudioOnly = FALSE;
				break;
			}
		}
	}
	BOOL bRet = FALSE;
	INT nDriveSampleOffset = 0;
	BOOL bGetDrive = GetDriveOffset(pDiscData->pszProductId, &nDriveSampleOffset);
	if(!bGetDrive) {
		CHAR a[6];
		ZeroMemory(a, sizeof(a));
		OutputString(_T("This drive isn't defined in driveOffset.txt.\n"));
		OutputString(_T("Please input Drive offset(Samples) -> "));
		INT b = scanf("%6[^\n]%*[^\n]", a);
		b = getchar();
		nDriveSampleOffset = atoi(a) * 4;
	}
	if(pDiscData->bAudioOnly) {
		pDiscData->nCombinedOffset = nDriveSampleOffset * 4;
		OutputLogString(fpLog, _T("Offset"));
		if(bGetDrive) {
			OutputLogString(fpLog, _T("(Drive offset data referes to www.accuraterip.com)"));
		}
		OutputLogString(fpLog, _T("\n"));
		OutputLogString(fpLog, _T("\tDrive Offset(Byte) %d, (Samples) %d\n"), 
			pDiscData->nCombinedOffset, nDriveSampleOffset);
		bRet = TRUE;
	}
	else {
		UCHAR byScsiStatus = 0;
		UCHAR aCmd[CDB12GENERIC_LENGTH] = {0};
		UCHAR aBuf[CD_RAW_SECTOR_WITH_SUBCODE_SIZE*2+PARAGRAPH_BOUNDARY_NUM] = {0};
		PUCHAR pBuf = (PUCHAR)ConvParagraphBoundary(aBuf);

		INT nDriveOffset = nDriveSampleOffset * 4;
		UCHAR byTransferLen = 2;
		if(!strncmp(pDiscData->pszVendorId, "PLEXTOR", 7)) {
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
				READ_CD_FLAG::UserData | READ_CD_FLAG::MainHeader | READ_CD_FLAG::Edc;
			aCmd[10] = READ_CD_FLAG::PtoW;
		}
		aCmd[2] = HIBYTE(HIWORD(pDiscData->nFirstDataLBA));
		aCmd[3] = LOBYTE(HIWORD(pDiscData->nFirstDataLBA));
		aCmd[4] = HIBYTE(LOWORD(pDiscData->nFirstDataLBA));
		aCmd[5] = LOBYTE(LOWORD(pDiscData->nFirstDataLBA));
		if(!ExecCommand(pDevData, aCmd, CDB12GENERIC_LENGTH, pBuf, 
			CD_RAW_SECTOR_WITH_SUBCODE_SIZE * 2, &byScsiStatus, _T(__FUNCTION__), __LINE__)) {
			return FALSE;
		}
		if(byScsiStatus == 0) {
			OutputMain2352(pBuf, pDiscData->nFirstDataLBA, fpLog);
			UCHAR Subcode[CD_RAW_READ_SUBCODE_SIZE] = {0};

			AlignRowSubcode(pBuf + CD_RAW_SECTOR_SIZE, Subcode);
			OutputAlignSub96(Subcode, pDiscData->nFirstDataLBA, fpLog);
			UCHAR aBuf2[CD_RAW_SECTOR_SIZE*2] = {0};
			memcpy(aBuf2, pBuf, CD_RAW_SECTOR_SIZE);
			memcpy(aBuf2 + CD_RAW_SECTOR_SIZE, pBuf + CD_RAW_SECTOR_WITH_SUBCODE_SIZE, CD_RAW_SECTOR_SIZE);
			bRet = GetWriteOffset(aBuf2, pDiscData->nFirstDataLBA, &pDiscData->nCombinedOffset);
		}
		else {
			// PLEXTOR(PX-W8432, PX-W1210T, PX-W2410T)
			// if Track1 is DataTrack (mostly game)
			// ==>Sense data, Key:Asc:Ascq: 05:64:00(ILLEGAL_REQUEST. ILLEGAL MODE FOR THIS TRACK)
			// else if Track1 isn't DataTrack (pc engine etc)
			// ==>no error.
			OutputString(_T("This drive can't read data sector in scrambled mode.\n"));
			return FALSE;
		}

		if(bRet) {
			OutputLogString(fpLog, _T("Offset"));
			if(bGetDrive) {
				OutputLogString(fpLog, _T("(Drive offset data referes to www.accuraterip.com)"));
			}
			OutputLogString(fpLog, _T("\n"));
			OutputLogString(fpLog, 
				_T("\t Combined Offset(Byte) %6d, (Samples) %5d\n"), 
				pDiscData->nCombinedOffset, pDiscData->nCombinedOffset / 4);
			OutputLogString(fpLog, 
				_T("\t-   Drive Offset(Byte) %6d, (Samples) %5d\n"), 
				nDriveOffset, nDriveSampleOffset);
			OutputLogString(fpLog, 
				_T("\t----------------------------------------------\n"));
			OutputLogString(fpLog, 
				_T("\t       CD Offset(Byte) %6d, (Samples) %5d\n"), 
				pDiscData->nCombinedOffset - nDriveOffset, 
				(pDiscData->nCombinedOffset - nDriveOffset) / 4);
		}
		else {
			OutputErrorString(_T("Failed to get write-offset[L:%d]\n"), __LINE__);
		}
	}

	if(0 < pDiscData->nCombinedOffset) {
		pDiscData->nAdjustSectorNum = pDiscData->nCombinedOffset / CD_RAW_SECTOR_SIZE + 1;
	}
	else if(pDiscData->nCombinedOffset < 0) {
		pDiscData->nAdjustSectorNum = pDiscData->nCombinedOffset / CD_RAW_SECTOR_SIZE - 1;
	}
	OutputLogString(fpLog, _T("\tNeed overread sector:%d\n"), pDiscData->nAdjustSectorNum);
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#else
	fflush(fpLog);
#endif
	return bRet;
}

BOOL ReadCDPartial(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	LPCTSTR pszOutFile,
	INT nStart,
	INT nEnd,
	_READ_CD_FLAG::_SectorType flg,
	BOOL bDC
	)
{
	_TCHAR szBin[8] = {0};
	_TCHAR szSub[8] = {0};
	_TCHAR szSubtxt[12] = {0};

	if(bDC) {
		_tcscpy(szSub, _T("_dc.sub"));
		_tcscpy(szSubtxt, _T("_dc.sub.txt"));
	}
	else {
		_tcscpy(szSub, _T(".sub"));
		_tcscpy(szSubtxt, _T(".sub.txt"));
	}
	FILE* fp = CreateOrOpenFileW(pszOutFile, NULL, NULL, NULL, _T(".bin"), _T("wb"), 0, 0);
	if(!fp) {
		OutputErrorString(_T("Failed to open file %s [L:%d]\n"), szBin, __LINE__);
		return FALSE;
	}
	FILE* fpSub = NULL;
	FILE* fpParse = NULL;
	BOOL bRet = TRUE;
	try {
		if(NULL == (fpSub = CreateOrOpenFileW(pszOutFile, NULL, NULL, NULL, szSub, _T("wb"), 0, 0))) {
			throw _T("Failed to open .sub\n");
		}
		if(NULL == (fpParse = CreateOrOpenFileW(pszOutFile, NULL, NULL, NULL, szSubtxt, _T(WFLAG), 0, 0))) {
			throw _T("Failed to open .sub.txt\n");
		}
		UCHAR byScsiStatus = 0;
		UCHAR aCmd[CDB12GENERIC_LENGTH] = {0};
		UCHAR aBuf[CD_RAW_SECTOR_WITH_SUBCODE_SIZE+PARAGRAPH_BOUNDARY_NUM] = {0};
		PUCHAR pBuf = (PUCHAR)ConvParagraphBoundary(aBuf);

		UCHAR Subcode[CD_RAW_READ_SUBCODE_SIZE] = {0};
		UCHAR byTransferLen = 1;
		if(!strncmp(pDiscData->pszVendorId, "PLEXTOR", 7)) {
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
			if(!ExecCommand(pDevData, aCmd, CDB12GENERIC_LENGTH, pBuf, 
				CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)) {
				throw _T("");
			}
			if(byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				OutputErrorString(_T("LBA %d Read error [L:%d]\n"), nLBA, __LINE__);
				if(nEnd == 549150) {
					throw _T("");
				}
			}
			else {
				fwrite(pBuf, sizeof(UCHAR), CD_RAW_SECTOR_SIZE, fp);
				AlignRowSubcode(pBuf + CD_RAW_SECTOR_SIZE, Subcode);
				fwrite(Subcode, sizeof(UCHAR), CD_RAW_READ_SUBCODE_SIZE, fpSub);
				OutputSubcode(nLBA, BcdToDec(Subcode[13]) - 1, 
					Subcode, pBuf + CD_RAW_SECTOR_SIZE, fpParse);
			}
			OutputString(_T("\rCreating bin from %d to %d (LBA) %6d"), nStart, nEnd, nLBA);
		}
		OutputString(_T("\n"));
	}
	catch(LPTSTR str) {
		OutputErrorString(str);
		bRet = FALSE;
	}
	FcloseAndNull(fp);
	FcloseAndNull(fpSub);
	FcloseAndNull(fpParse);
	return bRet;
}

BOOL ReadConfiguration(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	FILE* fpLog
	)
{
	UCHAR byScsiStatus = 0;
	UCHAR aCmd[CDB10GENERIC_LENGTH] = {0};
	GET_CONFIGURATION_HEADER pHeader = {0};
	size_t uiSize = sizeof(GET_CONFIGURATION_HEADER);

	aCmd[0] = SCSIOP_GET_CONFIGURATION;
	aCmd[1] = SCSI_GET_CONFIGURATION_REQUEST_TYPE_CURRENT;
	aCmd[3] = FeatureProfileList;
	aCmd[7] = HIBYTE(uiSize);
	aCmd[8] = LOBYTE(uiSize);
	OutputLogString(fpLog, _T("Configuration\n"));
	BOOL bRet = ExecCommand(pDevData, aCmd, CDB10GENERIC_LENGTH, &pHeader, 
		(ULONG)uiSize, &byScsiStatus, _T(__FUNCTION__), __LINE__);
	if(!bRet || byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		pDiscData->pusCurrentMedia = ProfileCdrom;
		// not false. because undefined mmc1..
		OutputLogString(fpLog, _T("\tUndefined this drive\n"));
	}
	else {
		OutputLogString(fpLog, _T("\tCurrentMedia:"));
		pDiscData->pusCurrentMedia = MAKEWORD(pHeader.CurrentProfile[1], pHeader.CurrentProfile[0]);
		OutputFeatureProfileType(fpLog, pDiscData->pusCurrentMedia);
		OutputLogString(fpLog, _T("\n"));
		// 4 is DataLength size of GET_CONFIGURATION_HEADER
		ULONG ulAllLen = MAKELONG(MAKEWORD(pHeader.DataLength[3], pHeader.DataLength[2]), 
			MAKEWORD(pHeader.DataLength[1], pHeader.DataLength[0])) + sizeof(pHeader.DataLength);

		PUCHAR pPConf = (PUCHAR)malloc((size_t)ulAllLen + PARAGRAPH_BOUNDARY_NUM);
		if(!pPConf) {
			OutputErrorString(_T("Can't alloc memory [F:%s][L:%d]\n"), _T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		PUCHAR pConf = (PUCHAR)ConvParagraphBoundary(pPConf);
		aCmd[7] = HIBYTE(ulAllLen);
		aCmd[8] = LOBYTE(ulAllLen);
		bRet = ExecCommand(pDevData, aCmd, CDB10GENERIC_LENGTH, pConf, 
			ulAllLen, &byScsiStatus, _T(__FUNCTION__), __LINE__);
		if(!bRet || byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			// not false. because undefined mmc1..
			OutputLogString(fpLog, _T("\tUndefined this drive\n"));
		}
		else {
			OutputFeatureNumber(pConf, ulAllLen, uiSize, &pDiscData->bCanCDText, &pDiscData->bC2ErrorData, fpLog);
		}
		FreeAndNull(pPConf);
	}
	return TRUE;
}

BOOL ReadDeviceInfo(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	FILE* fpLog
	)
{
	UCHAR byScsiStatus = 0;
	UCHAR aCmd[CDB6GENERIC_LENGTH] = {0};
	INQUIRYDATA inquiryData = {0};

	aCmd[0] = SCSIOP_INQUIRY;
	aCmd[4] = sizeof(INQUIRYDATA);
	if(!ExecCommand(pDevData, aCmd, CDB6GENERIC_LENGTH, &inquiryData, 
		sizeof(INQUIRYDATA), &byScsiStatus, _T(__FUNCTION__), __LINE__) || 
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	OutputInquiryData(&inquiryData, pDiscData->pszVendorId, pDiscData->pszProductId, fpLog);
	return TRUE;
}

BOOL ReadDVD(
	PDEVICE_DATA pDevData,
	LPCTSTR pszOutFile,
	LPCTSTR pszOption,
	INT nDVDSectorSize,
	FILE* fpLog
	)
{
	FILE* fp = CreateOrOpenFileW(pszOutFile, NULL, NULL, NULL, _T(".iso"), _T("wb"), 0, 0);
	if(!fp) {
		OutputErrorString(_T("Failed to open file .iso [L:%d]\n"), __LINE__);
		return FALSE;
	}
	UCHAR byScsiStatus = 0;
	UCHAR byTransferLen = 32;
	PUCHAR pPBuf = NULL;
	BOOL bRet = TRUE;
	try {
		pPBuf = (PUCHAR)malloc(CD_RAW_READ * (size_t)byTransferLen + PARAGRAPH_BOUNDARY_NUM);
		if(pPBuf == NULL) {
			throw _T("Can't alloc memory aBuf\n");
		}
		UCHAR aCmd[CDB12GENERIC_LENGTH] = {0};
		ZeroMemory(pPBuf, CD_RAW_READ * (size_t)byTransferLen + PARAGRAPH_BOUNDARY_NUM);
		PUCHAR pBuf = (PUCHAR)ConvParagraphBoundary(pPBuf);

		aCmd[0] = SCSIOP_READ12;
		aCmd[5] = 0;
		aCmd[9] = byTransferLen;
		aCmd[10] = 0x80; // Streaming
		if(!ExecCommand(pDevData, aCmd, CDB12GENERIC_LENGTH, pBuf, 
			(ULONG)CD_RAW_READ * byTransferLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			OutputErrorString(_T("Not supported Streaming. Changed ForceMediaAccess.\n"));
			aCmd[1] = CDB_FORCE_MEDIA_ACCESS;
			aCmd[10] = 0;
		}

		aCmd[5] = LOBYTE(0);
		if(!ExecCommand(pDevData, aCmd, CDB12GENERIC_LENGTH, pBuf, 
			(ULONG)CD_RAW_READ * byTransferLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw _T("");
		}
		for(INT i = CD_RAW_READ * 16; i <= CD_RAW_READ * 21; i += CD_RAW_READ) {
			OutputVolumeRecognitionSequence(i, pBuf, fpLog);
		}
		aCmd[5] = LOBYTE(32);
		if(!ExecCommand(pDevData, aCmd, CDB12GENERIC_LENGTH, pBuf, 
			(ULONG)CD_RAW_READ * byTransferLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw _T("");
		}
		if(pBuf[20] == 0 && pBuf[21] == 0 && pBuf[22] == 0 && pBuf[23] == 0) {
			for(INT i = 0; i <= CD_RAW_READ * 5; i += CD_RAW_READ) {
				OutputVolumeDescriptorSequence(i, pBuf, fpLog);
			}
		}
		aCmd[4] = HIBYTE(LOWORD(256));
		aCmd[5] = LOBYTE(LOWORD(256));
		if(!ExecCommand(pDevData, aCmd, CDB12GENERIC_LENGTH, pBuf, 
			(ULONG)CD_RAW_READ * byTransferLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw _T("");
		}
		OutputVolumeDescriptorSequence(0, pBuf, fpLog);
		fflush(fpLog);

		UCHAR aBuf2[sizeof(DVD_DESCRIPTOR_HEADER)+sizeof(DVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR)+PARAGRAPH_BOUNDARY_NUM] = {0};
		UCHAR aCmd2[CDB10GENERIC_LENGTH] = {0};
		PUCHAR pBuf2 = NULL;
		size_t uiSize = 0;
		if(pszOption && !_tcscmp(pszOption, _T("cmi"))) {
			pBuf2 = (PUCHAR)ConvParagraphBoundary(aBuf2);
			uiSize = sizeof(DVD_DESCRIPTOR_HEADER) + 
				sizeof(DVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR);
			aCmd2[0] = SCSIOP_READ_DVD_STRUCTURE;
			aCmd2[7] = DvdMaxDescriptor;
			aCmd2[8] = HIBYTE(LOWORD(uiSize));
			aCmd2[9] = LOBYTE(LOWORD(uiSize));
			OutputLogString(fpLog, _T("\tCopyrightManagementInformation\n"));
		}
		for(INT nLBA = 0; nLBA < nDVDSectorSize; nLBA += byTransferLen) {
			if(nDVDSectorSize - nLBA < byTransferLen) {
				byTransferLen = (UCHAR)(nDVDSectorSize - nLBA);
				aCmd[9] = byTransferLen;
			}
			aCmd[2] = HIBYTE(HIWORD(nLBA));
			aCmd[3] = LOBYTE(HIWORD(nLBA));
			aCmd[4] = HIBYTE(LOWORD(nLBA));
			aCmd[5] = LOBYTE(LOWORD(nLBA));
			if(!ExecCommand(pDevData, aCmd, CDB12GENERIC_LENGTH, pBuf, 
				(ULONG)CD_RAW_READ * byTransferLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw _T("");
			}
			fwrite(pBuf, sizeof(UCHAR), (size_t)CD_RAW_READ * byTransferLen, fp);

			if(pszOption && !_tcscmp(pszOption, _T("cmi"))) {
				for(INT i = 0; i < byTransferLen; i++) {
					aCmd2[2] = HIBYTE(HIWORD(nLBA + i));
					aCmd2[3] = LOBYTE(HIWORD(nLBA + i));
					aCmd2[4] = HIBYTE(LOWORD(nLBA + i));
					aCmd2[5] = LOBYTE(LOWORD(nLBA + i));
					if(!ExecCommand(pDevData, aCmd2, CDB10GENERIC_LENGTH, 
						pBuf2, (ULONG)uiSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)) {
						throw _T("");
					}
					if(byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
						OutputErrorString(
							_T("LBA %7d, Read Disc Structure Cmd error\n"), nLBA);
					}
					OutputCopyrightManagementInformation(nLBA, i, pBuf2, fpLog);
				}
			}
			OutputString(_T("\rCreating iso(LBA):%7d/%7d"), 
				nLBA + byTransferLen - 1, nDVDSectorSize - 1);
		}
		OutputString(_T("\n"));
	}
	catch(LPTSTR str) {
		OutputErrorString(str);
		bRet = FALSE;
	}
	FreeAndNull(pPBuf);
	FcloseAndNull(fp);
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
	return bRet;
}

BOOL ReadDVDRaw(
	PDEVICE_DATA pDevData,
	LPCSTR pszVendorId,
	LPCTSTR pszOutFile,
	INT nDVDSectorSize
	)
{
	FILE* fp = CreateOrOpenFileW(pszOutFile, NULL, NULL, NULL, _T(".raw"), _T("wb"), 0, 0);
	if(!fp) {
		OutputErrorString(_T("Failed to open file .raw [L:%d]\n"), __LINE__);
		return FALSE;
	}
	UCHAR byScsiStatus = 0;
	UCHAR byTransferLen = 31;
	PUCHAR aBuf = NULL;
	BOOL bRet = TRUE;
	try {
		if(NULL == (aBuf = (PUCHAR)malloc(DVD_RAW_READ * (size_t)byTransferLen + 0x10))) {
			throw _T("Can't alloc memory aBuf\n");
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
			if(!ExecCommand(pDevData, aCmd, cdblen, pBuf, 
				(ULONG)DVD_RAW_READ * byTransferLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw _T("");
			}

			fwrite(pBuf, sizeof(UCHAR), (size_t)DVD_RAW_READ * byTransferLen, fp);
			OutputString(_T("\rCreating raw(LBA):%7d/%7d"), 
				nLBA + byTransferLen - 1, nDVDSectorSize - 1);
		}
		OutputString(_T("\n"));
	}
	catch(LPTSTR str) {
		OutputErrorString(str);
		bRet = FALSE;
	}
	FreeAndNull(aBuf);
	FcloseAndNull(fp);
#if 0
	unscrambler *u = unscrambler_new ();
	CHAR pszOutFileA[_MAX_PATH];
	ZeroMemory(pszOutFileA, sizeof(pszOutFileA));
	WideCharToMultiByte(CP_THREAD_ACP, 0, pszOutFile, -1, pszOutFileA, _MAX_PATH, NULL, NULL);

	CHAR pszInPath[_MAX_PATH];
	ZeroMemory(pszInPath, sizeof(pszInPath));
	FILE* fpIn = CreateOrOpenFileA(pszOutFileA, pszInPath, NULL, ".raw", "rb", 0, 0);
	if(!fpIn) {
		OutputErrorString(_T("Failed to open file .raw [L:%d]\n"), __LINE__);
		return FALSE;
	}
	CHAR pszOutPath[_MAX_PATH];
	ZeroMemory(pszOutPath, sizeof(pszOutPath));
	FILE* fpOut = CreateOrOpenFileA(pszOutFileA, pszOutPath, NULL, ".iso", "wb", 0, 0);
	if(!fpOut) {
		OutputErrorString(_T("Failed to open file .iso [L:%d]\n"), __LINE__);
		return FALSE;
	}
	UINT current_sector = 0;
	unscrambler_set_disctype(3);
	unscrambler_unscramble_file(u, pszInPath, pszOutPath, &current_sector);
	u = (unscrambler *)unscrambler_destroy (u);
#endif
	return bRet;
}

BOOL ReadDVDStructure(
	PDEVICE_DATA pDevData,
	PINT nDVDSectorSize,
	FILE* fpLog
	)
{
	UCHAR byScsiStatus = 0;
	UCHAR aCmd[CDB12GENERIC_LENGTH] = {0};
	size_t uiMaxDVDStructureSize = sizeof(DVD_STRUCTURE_LIST_ENTRY) * 0xFF;
	UCHAR pDiscStructure[sizeof(DVD_STRUCTURE_LIST_ENTRY)*0xFF+0x10];

	aCmd[0] = SCSIOP_READ_DVD_STRUCTURE;
	aCmd[7] = 0xFF;
	aCmd[8] = HIBYTE(uiMaxDVDStructureSize);
	aCmd[9] = LOBYTE(uiMaxDVDStructureSize);

	if(!ExecCommand(pDevData, aCmd, CDB10GENERIC_LENGTH, pDiscStructure, 
		(ULONG)uiMaxDVDStructureSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}

	USHORT usDataLength = MAKEWORD(pDiscStructure[1], pDiscStructure[0]);
	INT nAllStructureSize = usDataLength - 2;
	size_t uiFormatSize = (size_t)(nAllStructureSize / 4);
	if(uiFormatSize == 0) {
		OutputLogString(fpLog, _T("DVD Structure failed. formatSize is 0\n"));
		return FALSE;
	}
	PUCHAR pFormat = (PUCHAR)malloc(uiFormatSize);
	if(!pFormat) {
		OutputErrorString(_T("Can't alloc memory L:%d"), __LINE__);
		return FALSE;
	}
	ZeroMemory(pFormat, uiFormatSize);
	PUSHORT pStructureLength = NULL;
	PUCHAR pStructure = NULL;
	BOOL bRet = TRUE;
	try {
		if(NULL == (pStructureLength =(PUSHORT)malloc(uiFormatSize * sizeof(USHORT)))) {
			throw _T("Can't alloc memory pStructureLength\n");
		}
		ZeroMemory(pStructureLength, uiFormatSize * sizeof(USHORT));
		OutputLogString(fpLog, _T("DVD Structure\n"));
		for(INT i = 0, j = 0; i < nAllStructureSize; i+=4, j++) {
#ifdef _DEBUG
			if(i == 0) {
				OutputLogString(fpLog, _T("\tDisc Structure List\n"));
			}
			OutputLogString(fpLog, 
				_T("\t\tFormatCode:%02X, SDR:%s, RDS:%s, Structure Length:%d\n"), 
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
				throw _T("Can't alloc memory pStructure\n");
			}
			ZeroMemory(pStructure, pStructureLength[i]);
			aCmd[7] = pFormat[i];
			aCmd[8] = HIBYTE(pStructureLength[i]);
			aCmd[9] = LOBYTE(pStructureLength[i]);

			if(!ExecCommand(pDevData, aCmd, CDB12GENERIC_LENGTH, pStructure, 
				pStructureLength[i], &byScsiStatus, _T(__FUNCTION__), __LINE__)) {
				throw _T("");
			}
			if(byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				OutputErrorString(_T("Failure - Format %d\n"), pFormat[i]);
				continue;
			}
			OutputDVDStructureFormat(pFormat, i, pStructure, pStructureLength, nDVDSectorSize, fpLog);
			FreeAndNull(pStructure);
		}
	}
	catch(LPTSTR str) {
		OutputErrorString(str);
		bRet = FALSE;
	}
	FreeAndNull(pFormat);
	FreeAndNull(pStructureLength);
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
	return bRet;
}

BOOL ReadTestUnitReady(
	PDEVICE_DATA pDevData
	)
{
	UCHAR byScsiStatus = 0;
	UCHAR aCmd[CDB6GENERIC_LENGTH] = {0};
	aCmd[0] = SCSIOP_TEST_UNIT_READY;

	if(!ExecCommand(pDevData, aCmd, CDB6GENERIC_LENGTH, NULL, 
		0, &byScsiStatus, _T(__FUNCTION__), __LINE__) || 
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	return TRUE;
}

BOOL ReadTOC(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	FILE* fpLog
	)
{
	UCHAR byScsiStatus = 0;
	UCHAR aCmd[CDB10GENERIC_LENGTH] = {0};
	aCmd[0] = SCSIOP_READ_TOC;
	aCmd[7] = HIBYTE(CDROM_TOC_SIZE);
	aCmd[8] = LOBYTE(CDROM_TOC_SIZE);

	if(!ExecCommand(pDevData, aCmd, CDB10GENERIC_LENGTH, &pDiscData->toc, 
		CDROM_TOC_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__) || 
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		OutputErrorString(_T("Can't get toc\n"));
		return FALSE;
	}

	OutputLogString(fpLog, _T("TOC on SCSIOP_READ_TOC\n"));
	_TCHAR strType[6] = {0};
	BOOL bFirstData = TRUE;
	for(INT i = pDiscData->toc.FirstTrack; i <= pDiscData->toc.LastTrack; i++) {
		for(INT j = 0, k = 24; j < 4; j++, k -= 8) {
			pDiscData->aTocLBA[i-1][0] |= pDiscData->toc.TrackData[i-1].Address[j] << k;
			pDiscData->aTocLBA[i-1][1] |= pDiscData->toc.TrackData[i].Address[j] << k;
		}
		pDiscData->aTocLBA[i-1][1] -= 1;
		pDiscData->nLength += pDiscData->aTocLBA[i-1][1] - pDiscData->aTocLBA[i-1][0] + 1;

		if((pDiscData->toc.TrackData[i-1].Control & AUDIO_DATA_TRACK) == 0) {
			_tcscpy(strType, _T("Audio"));
		}
		else if((pDiscData->toc.TrackData[i-1].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			if(bFirstData) {
				pDiscData->nFirstDataLBA = pDiscData->aTocLBA[i-1][0];
				bFirstData = FALSE;
			}
			_tcscpy(strType, _T(" Data"));
		}
		OutputLogString(fpLog, _T("\t%s Track %2u, LBA %6u-%6u, Length %6u\n"), 
			strType, i, pDiscData->aTocLBA[i-1][0], pDiscData->aTocLBA[i-1][1], 
			pDiscData->aTocLBA[i-1][1] - pDiscData->aTocLBA[i-1][0] + 1);
	}
	OutputLogString(fpLog, 
		_T("\t                                   Total  %6u\n"), pDiscData->nLength);
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
	return TRUE;
}

BOOL ReadTOCFull(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	FILE* fpLog,
	FILE* fpCcd
	)
{
	UCHAR byScsiStatus = 0;
	UCHAR aCmd[CDB10GENERIC_LENGTH] = {0};
	CDROM_TOC_FULL_TOC_DATA fullToc = {0};
	size_t uiFullTocDataSize = sizeof(CDROM_TOC_FULL_TOC_DATA);
	aCmd[0] = SCSIOP_READ_TOC;
	aCmd[2] = CDROM_READ_TOC_EX_FORMAT_FULL_TOC;
	aCmd[7] = HIBYTE(uiFullTocDataSize);
	aCmd[8] = LOBYTE(uiFullTocDataSize);

	if(!ExecCommand(pDevData, aCmd, CDB10GENERIC_LENGTH, &fullToc,
		(ULONG)uiFullTocDataSize, &byScsiStatus, _T(__FUNCTION__), __LINE__) 
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		OutputErrorString(_T("Can't get CDROM_TOC_FULL_TOC_DATA 1\n"));
		return FALSE;
	}

	size_t uiFulltocsize = 
		MAKEWORD(fullToc.Length[1], fullToc.Length[0]) - sizeof(fullToc.Length);
	size_t uiTocEntries = uiFulltocsize / sizeof(CDROM_TOC_FULL_TOC_DATA_BLOCK);

	WriteCcdFileForDisc(uiTocEntries, fullToc.LastCompleteSession, fpCcd);
	if(pDiscData->bCanCDText) {
		ReadTOCText(pDevData, fpLog, fpCcd);
	}
	size_t uiFullTocDataMaxSize = uiFulltocsize + uiFullTocDataSize;
	if(!strncmp(pDiscData->pszVendorId, "PLEXTOR", 7) &&
		!strncmp(pDiscData->pszProductId, "DVDR   PX-712A", 14) &&
		uiFullTocDataMaxSize > 1028) {
		OutputErrorString(_T("On this drive, can't get CDROM_TOC_FULL_TOC_DATA of this disc\n"));
		return TRUE;
	}
	OutputLogString(fpLog, _T("CDROM_TOC_FULL_TOC_DATA:Length %d\n"), uiFulltocsize);
	PUCHAR pPFullToc = (PUCHAR)malloc(uiFullTocDataMaxSize + PARAGRAPH_BOUNDARY_NUM);
	if(!pPFullToc) {
		OutputString(_T("Can't alloc memory [L:%d]\n"), __LINE__);
		return FALSE;
	}
	PUCHAR pFullToc = (PUCHAR)ConvParagraphBoundary(pPFullToc);
	aCmd[7] = HIBYTE(uiFullTocDataMaxSize);
	aCmd[8] = LOBYTE(uiFullTocDataMaxSize);
	BOOL bRet = TRUE;
	try {
		if(!ExecCommand(pDevData, aCmd, CDB10GENERIC_LENGTH, pFullToc, 
			(ULONG)uiFullTocDataMaxSize, &byScsiStatus, _T(__FUNCTION__), __LINE__) 
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw _T("Can't get CDROM_TOC_FULL_TOC_DATA 2\n");
		}
		CDROM_TOC_FULL_TOC_DATA_BLOCK* pTocData = 
			((CDROM_TOC_FULL_TOC_DATA*)pFullToc)->Descriptors;
		size_t uiIdx = 0;
		INT aLBA[] = {0, pDiscData->nFirstDataLBA};
		for(size_t b = 0; b < uiTocEntries; b++) {
			if(pTocData[b].Point < 100 && uiIdx < pTocData[b].SessionNumber) {
				UCHAR aBuf2[CD_RAW_SECTOR_WITH_SUBCODE_SIZE+PARAGRAPH_BOUNDARY_NUM] = {0};
				UCHAR aCmd2[CDB12GENERIC_LENGTH] = {0};
				PUCHAR pBuf = (PUCHAR)ConvParagraphBoundary(aBuf2);

				if(!strncmp(pDiscData->pszVendorId, "PLEXTOR", 7)) {
					// PX-504A don't support...
					// Sense data, Key:Asc:Ascq: 05:20:00(ILLEGAL_REQUEST. INVALID COMMAND OPERATION CODE)
					aCmd2[0] = 0xD8;
					aCmd2[9] = 0x01;
					aCmd2[10] = 0x02; // 0=none, 1=Q, 2=P-W, 3=P-W only
				}
				else {
					aCmd2[0] = SCSIOP_READ_CD;
					aCmd2[1] = READ_CD_FLAG::All;
					aCmd2[8] = 0x01;
					aCmd2[9] = READ_CD_FLAG::SyncData | READ_CD_FLAG::SubHeader | 
						READ_CD_FLAG::UserData | READ_CD_FLAG::MainHeader | READ_CD_FLAG::Edc;
					aCmd2[10] = READ_CD_FLAG::PtoW;
				}
				UCHAR ucMode = 0;
				for(INT nLBA =  aLBA[uiIdx]; nLBA < aLBA[uiIdx] + 100; nLBA++) {
					aCmd2[2] = HIBYTE(HIWORD(nLBA));
					aCmd2[3] = LOBYTE(HIWORD(nLBA));
					aCmd2[4] = HIBYTE(LOWORD(nLBA));
					aCmd2[5] = LOBYTE(LOWORD(nLBA));

					if(!ExecCommand(pDevData, aCmd2, CDB12GENERIC_LENGTH, pBuf,
						CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
						|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
						OutputErrorString(_T("Can't read CD for MCN\n"));
						break;
					}
					if(nLBA ==  aLBA[uiIdx]) {
						ucMode = GetMode(pBuf);
					}
					UCHAR Subcode[CD_RAW_READ_SUBCODE_SIZE] = {0};
					AlignRowSubcode(pBuf + CD_RAW_SECTOR_SIZE, Subcode);

					UCHAR byAdr = (UCHAR)(Subcode[12] & 0x0F);
					if(byAdr == ADR_ENCODES_MEDIA_CATALOG) {
						BOOL bMCN = IsValidMCN(Subcode);
						_TCHAR szCatalog[META_CATALOG_SIZE+1] = {0};
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
		OutputTocFull(&fullToc, pTocData, uiTocEntries,	&pDiscData->nLastLBAof1stSession,
			&pDiscData->nStartLBAof2ndSession, pDiscData->aSessionNum, fpCcd, fpLog);
	}
	catch(LPTSTR str) {
		OutputErrorString(str);
		bRet = FALSE;
	}
	FreeAndNull(pPFullToc);
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
	return bRet;
}

BOOL ReadTOCText(
	PDEVICE_DATA pDevData,
	FILE* fpLog,
	FILE* fpCcd
	)
{
	UCHAR byScsiStatus = 0;
	UCHAR aCmd[CDB10GENERIC_LENGTH] = {0};
	CDROM_TOC_CD_TEXT_DATA tocText = {0};
	size_t uiCDTextDataSize = sizeof(CDROM_TOC_CD_TEXT_DATA);

	aCmd[0] = SCSIOP_READ_TOC;
	aCmd[2] = CDROM_READ_TOC_EX_FORMAT_CDTEXT;
	aCmd[7] = HIBYTE(uiCDTextDataSize);
	aCmd[8] = LOBYTE(uiCDTextDataSize);
	if(!ExecCommand(pDevData, aCmd, CDB10GENERIC_LENGTH, &tocText,
		(ULONG)uiCDTextDataSize, &byScsiStatus, _T(__FUNCTION__), __LINE__) 
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		OutputErrorString(_T("Can't get CDROM_TOC_CD_TEXT_DATA\n"));
		return FALSE;
	}

	OutputLogString(fpLog, _T("CDTEXT on SCSIOP_READ_TOC\n"));
	size_t uiTocTextsize = 
		MAKEWORD(tocText.Length[1], tocText.Length[0]) - sizeof(tocText.Length);
	size_t uiTocTextEntries = uiTocTextsize / sizeof(CDROM_TOC_CD_TEXT_DATA_BLOCK);

	WriteCcdFileForDiscCDTextLength(uiTocTextsize, fpCcd);
	if(!uiTocTextEntries) {
		OutputLogString(fpLog, _T("\tNothing\n"));
		// almost CD is nothing text
		return TRUE;
	}
	size_t uiCDTextDataMaxSize = uiTocTextsize + uiCDTextDataSize;
	PUCHAR pPTocText = (PUCHAR)malloc(uiCDTextDataMaxSize + PARAGRAPH_BOUNDARY_NUM);
	if(!pPTocText) {
		OutputErrorString(_T("Can't alloc memory [L:%d]\n"), __LINE__);
		return FALSE;
	}
	PUCHAR pTocText = (PUCHAR)ConvParagraphBoundary(pPTocText);
	aCmd[7] = HIBYTE(uiCDTextDataMaxSize);
	aCmd[8] = LOBYTE(uiCDTextDataMaxSize);
	BOOL bRet = TRUE;
	PCD_TEXT_INFO pInfo = NULL;
	CHAR* pTmpText = NULL;
	try {
		if(!ExecCommand(pDevData, aCmd, CDB10GENERIC_LENGTH, pTocText, 
			(ULONG)uiCDTextDataMaxSize, &byScsiStatus, _T(__FUNCTION__), __LINE__) 
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw _T("Can't get CDROM_TOC_CD_TEXT_DATA_BLOCK\n");
		}
		WriteCcdFileForCDText(uiTocTextEntries, fpCcd);
		if(NULL == (pInfo = (PCD_TEXT_INFO)malloc(uiTocTextEntries * META_STRING_SIZE))) {
			throw _T("Can't alloc memory PCD_TEXT_INFO\n");
		}
		CDROM_TOC_CD_TEXT_DATA_BLOCK* pDesc = 
			((CDROM_TOC_CD_TEXT_DATA*)pTocText)->Descriptors;
		size_t allTextSize = uiTocTextEntries * sizeof(pDesc->Text);
		if(NULL == (pTmpText = (CHAR*)malloc(allTextSize))) {
			throw _T("Can't alloc memory CHAR*\n");
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
					OutputLogString(fpLog, _T("\tAlbumTitle : %s\n"), pInfo[z].Text);
					SetAlbumTitle(pInfo[z].Text);
				}
				else if(byAlbumTitleIdx != 0 && byAlbumTitleIdx != byAlbumIdx && 
					byAlbumTitleIdx <= z && z <= byAlbumIdx) {
					OutputLogString(fpLog, _T("\tTitle : %s\n"), pInfo[z].Text);
					SetTitle(pInfo[z].Text, nTitleCnt);
					nTitleCnt++;
				}
				else if(byPerformerIdx != 0 && byAlbumIdx != byPerformerIdx && 
					byAlbumIdx <= z && z <= byPerformerIdx) {
					OutputLogString(fpLog, _T("\tPerformer : %s\n"), pInfo[z].Text);
					SetPerformer(pInfo[z].Text);
				}
				else if(bySongwriterIdx != 0 && byPerformerIdx != bySongwriterIdx && 
					byPerformerIdx <= z && z <= bySongwriterIdx) {
					OutputLogString(fpLog, _T("\tSongwriter : %s\n"), pInfo[z].Text);
					SetSongWriter(pInfo[z].Text);
				}
				else if(byComposerIdx != 0 && bySongwriterIdx != byComposerIdx && 
					bySongwriterIdx <= z && z <= byComposerIdx) {
					OutputLogString(fpLog, _T("\tComposer : %s\n"), pInfo[z].Text);
				}
				else if(byArrangerIdx && byComposerIdx != byArrangerIdx && 
					byComposerIdx <= z && z <= byArrangerIdx) {
					OutputLogString(fpLog, _T("\tArranger : %s\n"), pInfo[z].Text);
				}
				else if(byMessagesIdx != 0 && byArrangerIdx != byMessagesIdx && 
					byArrangerIdx <= z && z <= byMessagesIdx) {
					OutputLogString(fpLog, _T("\tMessages : %s\n"), pInfo[z].Text);
				}
				else if(byDiscIdIdx != 0 && byMessagesIdx != byDiscIdIdx && 
					byMessagesIdx <= z && z <= byDiscIdIdx) {
					OutputLogString(fpLog, _T("\tDiscId : %s\n"), pInfo[z].Text);
				}
				else if(byGenreIdx != 0 && byDiscIdIdx != byGenreIdx && 
					byDiscIdIdx <= z && z <= byGenreIdx) {
					OutputLogString(fpLog, _T("\tGenre : %s\n"), pInfo[z].Text);
				}
#if 0
				// detail in Page 54-55 of EN 60908:1999
				else if(byTocInfoIdx != 0 && byGenreIdx != byTocInfoIdx && 
					byGenreIdx <= z && z <= byTocInfoIdx) {
					OutputLogString(fpLog, _T("\tTocInfo : %s\n"), pInfo[z].Text);
				}
				else if(byTocInfo2Idx != 0 && byTocInfoIdx != byTocInfo2Idx && 
					byTocInfoIdx <= z && z <= byTocInfo2Idx) {
					OutputLogString(fpLog, _T("\tTocInfo2 : %s\n"), pInfo[z].Text);
				}
#endif
				else if(byUpcEanIdx != 0 && byTocInfo2Idx != byUpcEanIdx && 
					byTocInfo2Idx <= z && z <= byUpcEanIdx) {
					OutputLogString(fpLog, _T("\tUpcEan : %s\n"), pInfo[z].Text);
				}
#if 0
				// detail in Page 56 of EN 60908:1999
				else if(bySizeInfoIdx != 0 && byUpcEanIdx != bySizeInfoIdx && 
					byUpcEanIdx <= z && z <= bySizeInfoIdx) {
					OutputLogString(fpLog, _T("\tSizeInfo : %s\n"), pInfo[z].Text);
				}
#endif
			}
			uiIdx += len + 1;
		}
	}
	catch(LPTSTR str) {
		OutputErrorString(str);
		bRet = FALSE;
	}
	FreeAndNull(pPTocText);
	FreeAndNull(pInfo);
	FreeAndNull(pTmpText);
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
	return bRet;
}

BOOL SetCDSpeed(
	PDEVICE_DATA pDevData,
	INT nCDSpeedNum,
	FILE* fpLog
	)
{
	UCHAR byScsiStatus = 0;
	UCHAR aCDCmd[CDB12GENERIC_LENGTH] = {0};
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
	aCDCmd[0] = SCSIOP_SET_CD_SPEED;
	aCDCmd[2] = HIBYTE(wCDSpeedList[nCDSpeedNum]);
	aCDCmd[3] = LOBYTE(wCDSpeedList[nCDSpeedNum]);

	if(!ExecCommand(pDevData, aCDCmd, CDB12GENERIC_LENGTH, &setspeed, 
		sizeof(CDROM_SET_SPEED), &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	OutputLogString(fpLog, _T("Drive speed\n"));
	OutputLogString(fpLog, _T("\tRequestType:%s\n"),
		setspeed.RequestType == 0 ? _T("CdromSetSpeed") : _T("CdromSetStreaming"));
	OutputLogString(fpLog, _T("\tReadSpeed:%uKB/sec\n"), setspeed.ReadSpeed);
	OutputLogString(fpLog, _T("\tWriteSpeed:%uKB/sec\n"), setspeed.WriteSpeed);
	OutputLogString(fpLog, _T("\tRotationControl:%s\n"),
		setspeed.RotationControl == 0 ? _T("CdromDefaultRotation") : _T("CdromCAVRotation"));
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
	return TRUE;
}

BOOL StartStop(
	PDEVICE_DATA pDevData,
   UCHAR Start,
    UCHAR LoadEject
	)
{
	UCHAR byScsiStatus = 0;
	UCHAR aCmd[CDB6GENERIC_LENGTH] = {0};
	aCmd[0] = SCSIOP_START_STOP_UNIT;
	aCmd[4] = UCHAR(LoadEject << 1 | Start);

	if(!ExecCommand(pDevData, aCmd, CDB6GENERIC_LENGTH, NULL, 0, 
		&byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	return TRUE;
}
