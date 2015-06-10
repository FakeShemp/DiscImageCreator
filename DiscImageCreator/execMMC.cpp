/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"

inline PUCHAR ConvParagraphBoundary(
	PUCHAR pv,
	PDEVICE_DATA pDevData
	)
{
	return (PUCHAR)(((UINT_PTR)pv + pDevData->AlignmentMask) & ~pDevData->AlignmentMask);
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
	swb.ScsiPassThroughDirect.PathId = pDevData->address.PathId;
	swb.ScsiPassThroughDirect.TargetId = pDevData->address.TargetId;
	swb.ScsiPassThroughDirect.Lun = pDevData->address.Lun;
	swb.ScsiPassThroughDirect.CdbLength = byCdbCmdLength;
	swb.ScsiPassThroughDirect.SenseInfoLength = SPTWB_SENSE_LENGTH;
	swb.ScsiPassThroughDirect.DataIn = SCSI_IOCTL_DATA_IN;
	swb.ScsiPassThroughDirect.DataTransferLength = dwBufferLength;
	swb.ScsiPassThroughDirect.TimeOutValue = 2;
	swb.ScsiPassThroughDirect.DataBuffer = pvBuffer;
	swb.ScsiPassThroughDirect.SenseInfoOffset = 
		offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, SenseInfoBuffer);
	memcpy(swb.ScsiPassThroughDirect.Cdb, pbyCdbCmd, byCdbCmdLength);
	ULONG ulLength = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);

	SetLastError(NO_ERROR);

	ULONG ulReturned = 0;
	BOOL bRet = DeviceIoControl(pDevData->hDevice, IOCTL_SCSI_PASS_THROUGH_DIRECT,
		&swb, ulLength, &swb, ulLength, &ulReturned, NULL);
	*byScsiStatus = swb.ScsiPassThroughDirect.ScsiStatus;

	if(bRet) {
		OutputIoctlInfoScsiStatus(&swb, byScsiStatus, pszFuncname, nLineNum);
	}
	else {
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

		OutputErrorString(
			_T("\rDeviceIoControl with SCSI_PASS_THROUGH_DIRECT command failed [F:%s][L:%d], ")
			_T("GetLastError: %d, %s"), 
			pszFuncname, nLineNum, GetLastError(), (LPTSTR)lpMsgBuf);
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
	// check VolumeDescriptor
	if((pToc->TrackData[0].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
		aCmd[5] = 0;
		if(!ExecCommand(pDevData, aCmd, CDB12GENERIC_LENGTH, pBuf, 
			ulBufLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			return FALSE;
		}
		if(Is3DOData(pBuf)) {
			return bRet;
		}
		else {
			aCmd[5] = 1;
			if(!ExecCommand(pDevData, aCmd, CDB12GENERIC_LENGTH, pBuf, 
				ulBufLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				return FALSE;
			}
			if(IsMacData(pBuf + 1040)) {
				OutputFsMasterDirectoryBlocks(pBuf, 1040, fpLog);
			}
			else {
				aCmd[5] = 16;
				if(!ExecCommand(pDevData, aCmd, CDB12GENERIC_LENGTH, pBuf, 
					ulBufLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
					|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
					return FALSE;
				}
				if(pBuf[15] == DATA_BLOCK_MODE1) {
					OutputFsVolumeDescriptor(pBuf, SYNC_SIZE + HEADER_SIZE, fpLog);
					if(pBuf[SYNC_SIZE+HEADER_SIZE] == 0xFF) {
						return bRet;
					}
				}
				else if(pBuf[15] == DATA_BLOCK_MODE2) {
					OutputFsVolumeDescriptor(pBuf,
						SYNC_SIZE + HEADER_SIZE + SUBHEADER_SIZE, fpLog);
					if(pBuf[SYNC_SIZE+HEADER_SIZE+SUBHEADER_SIZE] == 0xFF) {
						return bRet;
					}
				}
			}
		}
	}
	return bRet;
}

BOOL CheckCDG(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	CONST PUCHAR aCmd,
	ULONG ulBufLen,
	PUCHAR pBuf,
	PBOOL bCDG
	)
{
	BOOL bRet = TRUE;
	UCHAR byScsiStatus = 0;
	aCmd[5] = 75;
	INT nShift = CD_RAW_SECTOR_SIZE;
	if(pDevData->bC2ErrorData && (pDiscData->bAudioOnly && pDevData->bPlextor)) {
		if(pDevData->bPlextor) {
			nShift += CD_RAW_READ_C2_SIZE_294;
		}
	}

	if(!ExecCommand(pDevData, aCmd, CDB12GENERIC_LENGTH, pBuf, 
		ulBufLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		bRet = FALSE;
	}
	else {
		INT nCDG = 0;
		for(INT i = 0; i < 4; i++) {
			nCDG += *(pBuf + nShift + i * 24);
		}
		if(nCDG == 0xfc) {
			// Why R-W bit is full? If it isn't CD+G or CD-MIDI, R-W bit should be off...
			//  Alanis Morissette - Jagged Little Pill (UK)
			//  WipEout 2097: The Soundtrack
			//  and more..
			*bCDG = FALSE;
		}
		else if(nCDG > 0) {
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
		prevPrevSubQ->byMode = prevSubQ->byMode;
		prevPrevSubQ->byAdr = prevSubQ->byAdr;
		prevPrevSubQ->nRelativeTime = prevSubQ->nRelativeTime;
	}
	else if((prevSubQ->byAdr == ADR_ENCODES_MEDIA_CATALOG || 
		prevSubQ->byAdr == ADR_ENCODES_ISRC) && prevSubQ->byIndex == 0) {
			subQ->nRelativeTime = prevSubQ->nRelativeTime + 1;
	}

	if(subQ->byAdr != ADR_ENCODES_MEDIA_CATALOG && 
		subQ->byAdr != ADR_ENCODES_ISRC) {
		prevSubQ->byMode = subQ->byMode;
		prevSubQ->byAdr = subQ->byAdr;
	}
	else if(prevSubQ->byIndex == 0 && prevSubQ->nRelativeTime == 0) {
		prevSubQ->byIndex = 1;
	}

	prevPrevSubQ->byTrackNum = prevSubQ->byTrackNum;
	prevPrevSubQ->byIndex = prevSubQ->byIndex;
	prevPrevSubQ->byCtl = prevSubQ->byCtl;
	prevPrevSubQ->nAbsoluteTime = prevSubQ->nAbsoluteTime;
	prevSubQ->byTrackNum = subQ->byTrackNum;
	prevSubQ->byIndex = subQ->byIndex;
	prevSubQ->byCtl = subQ->byCtl;
	prevSubQ->nRelativeTime = subQ->nRelativeTime;
	prevSubQ->nAbsoluteTime++;

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
	PINT* pLBAOfDataTrackList,
	FILE* fpLog
	)
{
	if(subQ->byTrackNum > 0) {
		// preserve mode, ctl
		if(nLBA == pDiscData->aTocLBA[subQ->byTrackNum-1][0]) {
			pCtlList[subQ->byTrackNum-1] = subQ->byCtl;
			pModeList[subQ->byTrackNum-1] = subQ->byMode;
		}
		// preserve nLBA
		if(prevSubQ->byTrackNum + 1 == subQ->byTrackNum) {
			if(subQ->byIndex > 0) {
				// index 1 is prior to TOC
				if(subQ->byIndex == 1 && nLBA != pDiscData->aTocLBA[subQ->byTrackNum-1][0]) {
					OutputLogString(fpLog, 
						_T("Subchannel & TOC isn't sync. Track %2d, LBA on subchannel: %6d, index: %2d, LBA on TOC: %6d\n"),
						subQ->byTrackNum, nLBA, subQ->byIndex, pDiscData->aTocLBA[subQ->byTrackNum-1][0]);
					pLBAStartList[subQ->byTrackNum-1][1] = pDiscData->aTocLBA[subQ->byTrackNum-1][0];
					if(pLBAStartList[subQ->byTrackNum-1][0] == pLBAStartList[subQ->byTrackNum-1][1]) {
						pLBAStartList[subQ->byTrackNum-1][0] = -1;
					}
				}
				else {
					pLBAStartList[subQ->byTrackNum-1][subQ->byIndex] = nLBA;
				}
			}
			// preserve end lba of data track
			if(prevSubQ->byTrackNum > 0) {
				if(pLBAOfDataTrackList[prevSubQ->byTrackNum-1][0] != -1 &&
					pLBAOfDataTrackList[prevSubQ->byTrackNum-1][1] == -1 &&
					(prevSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
						pLBAOfDataTrackList[prevSubQ->byTrackNum-1][1] = nLBA - 1;
				}
			}
			*byCurrentTrackNum = subQ->byTrackNum;
		}
		// Hyper Wars (Japan)
		// LBA[098484, 0x180B4], Data, Copy NG, TOC[TrackNum-03, Index-00, RelativeTime-00:02:01, AbsoluteTime-21:55:09] RtoW:ZERO mode
		// LBA[098485, 0x180B5], Data, Copy NG, TOC[TrackNum-03, Index-00, RelativeTime-00:02:00, AbsoluteTime-21:55:10] RtoW:ZERO mode
		// LBA[098486, 0x180B6], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-03, Index-00, RelativeTime-00:01:74, AbsoluteTime-21:55:11] RtoW:ZERO mode
		// LBA[098487, 0x180B7], Audio, 2ch, Copy NG, Pre-emphasis No, TOC[TrackNum-03, Index-00, RelativeTime-00:01:73, AbsoluteTime-21:55:12] RtoW:ZERO mode
		if((prevPrevSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK &&
			(prevSubQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK &&
			(subQ->byCtl & AUDIO_DATA_TRACK) == 0 &&
			subQ->byIndex == 0 && prevSubQ->byIndex == 0 && prevPrevSubQ->byIndex == 0) {
			pLBAOfDataTrackList[subQ->byTrackNum-1][subQ->byIndex+1] = nLBA - 1;
		}
		// preserve index
		if(prevSubQ->byIndex + 1 == subQ->byIndex && *byCurrentTrackNum >= 0) {
			if(subQ->byIndex != 1) {
				pLBAStartList[subQ->byTrackNum-1][subQ->byIndex] = nLBA;
			}
			else {
				// index 1 is prior to TOC
				if(nLBA != pDiscData->aTocLBA[subQ->byTrackNum-1][0]) {
					OutputLogString(fpLog,
						_T("Subchannel & TOC isn't sync. Track %2d, LBA on subchannel: %6d, prevIndex: %2d, LBA on TOC: %6d\n"),
						subQ->byTrackNum, nLBA, prevSubQ->byIndex, pDiscData->aTocLBA[subQ->byTrackNum-1][0]);
				}
				pLBAStartList[subQ->byTrackNum-1][1] = pDiscData->aTocLBA[subQ->byTrackNum-1][0];
				if(pLBAStartList[subQ->byTrackNum-1][0] == pLBAStartList[subQ->byTrackNum-1][1]) {
					// Crow, The - Original Motion Picture Soundtrack (82519-2)
					pLBAStartList[subQ->byTrackNum-1][0] = -1;
				}
			}
		}
		else if(prevSubQ->byIndex >= 1 && subQ->byIndex == 0) {
			pLBAStartList[subQ->byTrackNum-1][subQ->byIndex] = nLBA;
		}

		if((pDiscData->toc.TrackData[subQ->byTrackNum-1].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK &&
			(subQ->byCtl & AUDIO_DATA_TRACK) == 0) {
			OutputLogString(fpLog,
				_T("LBA %6d, Track[%02d] is data track, but this sector is audio\n"),
				nLBA, subQ->byTrackNum);
		}
		else if((pDiscData->toc.TrackData[subQ->byTrackNum-1].Control & AUDIO_DATA_TRACK) == 0 &&
			(subQ->byCtl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			OutputLogString(fpLog,
				_T("LBA %6d, Track[%02d] is audio track, but this sector is data\n"),
				nLBA, subQ->byTrackNum);
		}

		// preserve first lba of data track offset
		if(pLBAOfDataTrackList[subQ->byTrackNum-1][0] == -1 &&
			(pDiscData->toc.TrackData[subQ->byTrackNum-1].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
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
			UINT nStartLBA = pDiscData->aSessionNum[k] >= 2 
				? pLBAOfDataTrackList[k][0] - (11400 * (pDiscData->aSessionNum[k] - 1)) 
				: pLBAOfDataTrackList[k][0];
			UINT nEndLBA = pDiscData->aSessionNum[k] >= 2 
				? pLBAOfDataTrackList[k][1] - (11400 * (pDiscData->aSessionNum[k] - 1)) 
				: pLBAOfDataTrackList[k][1];
			for(; nStartLBA <= nEndLBA; nStartLBA++) {
				// ファイルを読み書き両用モードで開いている時は 注意が必要です。
				// 読み込みを行った後に書き込みを行う場合やその逆を行う場合は、 
				// 必ずfseekを呼ばなければなりません。もしこれを忘れると、
				// 場合によってはバッファー内と 実際にディスクに描き込まれた
				// データに矛盾が生じ、正確に書き込まれない場合や、
				// 嘘の データを読み込む場合があります。
				fseek(fpImg, (long)nStartLBA * CD_RAW_SECTOR_SIZE, SEEK_SET);
				fread(aSrcBuf, sizeof(UCHAR), sizeof(aSrcBuf), fpImg);
				if(IsValidDataHeader(aSrcBuf)) {
					fseek(fpImg, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
					for(INT n = 0; n < CD_RAW_SECTOR_SIZE; n++) {
						aSrcBuf[n] ^= aScrambledBuf[n];
					}
					fwrite(aSrcBuf, sizeof(UCHAR), sizeof(aSrcBuf), fpImg);
				}
				OutputString(
					_T("\rDescrambling data sector of img(LBA) %6d/%6d"), nStartLBA, nEndLBA);
			}
			OutputString(_T("\n"));
		}
	}
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
		FILE* fpCdg =
			CreateOrOpenFileW(pszOutFile, NULL, NULL, NULL, _T(".cdg"), _T("rb"), 0, 0);
		if(fpCdg == NULL) {
			OutputErrorString(_T("Failed to open file .cdg\n"));
			bRet = FALSE;
		}
		FILE* fpBinAll =
			CreateOrOpenFileW(pszOutFile, NULL, NULL, NULL, _T(".bin"), _T("wb"), 0, 0);
		if(fpBinAll == NULL) {
			OutputErrorString(_T("Failed to open file .bin\n"));
			bRet = FALSE;
		}
		if(bRet) {
			LONG fsizeImg2 = (LONG)GetFilesize(0, fpImg);
			UCHAR buf[CD_RAW_SECTOR_WITH_SUBCODE_SIZE] = {0};

			for(int i = 0; i < fsizeImg2 / CD_RAW_SECTOR_SIZE; i++) {
				fread(buf, sizeof(UCHAR), CD_RAW_SECTOR_SIZE, fpImg);
				fread(buf + CD_RAW_SECTOR_SIZE,
					sizeof(UCHAR), CD_RAW_READ_SUBCODE_SIZE, fpCdg);
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
	for(UINT i = pDiscData->toc.FirstTrack; i <= pDiscData->toc.LastTrack; i++) {
		FILE* fpBin = CreateOrOpenFileW(pszOutFile, NULL, pszFileNameWithoutPath,
			NULL, _T(".bin"), _T("wb"), i, pDiscData->toc.LastTrack);
		if(fpBin == NULL) {
			OutputErrorString(_T("Failed to open .bin\n"));
			bRet = FALSE;
			break;
		}
		else {
			OutputString(_T("\rCreating bin, cue, ccd(Track) %2d/%2d"),
				i, pDiscData->toc.LastTrack);
			if(i == pDiscData->toc.FirstTrack) {
				WriteCueFileFirst(pDiscData, bCatalog, fpCue);
			}
			WriteCueFile(pDiscData, pszFileNameWithoutPath, bCDG,
				i, pModeList[i-1], pbISRCList[i-1], pCtlList[i-1], fpCue);
			WriteCcdFileForTrack(pDiscData, i,
				pModeList[i-1], pbISRCList[i-1], pCtlList[i-1], fpCcd);

			BYTE byFrame = 0, bySecond = 0, byMinute = 0;
			if(i == pDiscData->toc.FirstTrack) {
				LBAtoMSF(pLBAStartList[i-1][1] - pLBAStartList[i-1][0], 
					&byFrame, &bySecond, &byMinute);
				if(pLBAStartList[i-1][1] > 0) {
					// Crow, The - Original Motion Picture Soundtrack (82519-2)
					WriteCueFileForIndex(0, 0, 0, 0, fpCue);
					WriteCcdFileForTrackIndex(0, 0, fpCcd);
				}
				WriteCueFileForIndex(1, byFrame, 0, 0, fpCue);
				WriteCcdFileForTrackIndex(1, pLBAStartList[i-1][1], fpCcd);
			}
			else if(pLBAStartList[i-1][0] == -1 && pLBAStartList[i-1][2] == -1) {
				LBAtoMSF(pLBAStartList[i-1][1] - pLBAStartList[i-1][0], 
					&byFrame, &bySecond, &byMinute);
				WriteCueFileForIndex(1, 0, 0, 0, fpCue);
				WriteCcdFileForTrackIndex(1, pLBAStartList[i-1][1], fpCcd);
			}
			else if(pLBAStartList[i-1][0] != -1) {
				for(UCHAR index = 0; index < MAXIMUM_NUMBER_INDEXES; index++) {
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
			INT nLBA =
				pLBAStartList[i][0] == -1 ? pLBAStartList[i][1] : pLBAStartList[i][0];
			INT nPrevLba =
				pLBAStartList[i-1][0] == -1 ? pLBAStartList[i-1][1] : pLBAStartList[i-1][0];
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
					UINT nLeadinoutSize = 11400 * (pDiscData->aSessionNum[i-1] - 1);
					nPrevLba -= nLeadinoutSize;
					nTmpLength -= nLeadinoutSize;
				}
				uiBufsize = (size_t)(nTmpLength - nPrevLba) * nWriteSectorSize;
			}
			else {
				if(i == pDiscData->toc.LastTrack - (UINT)1 && pDiscData->aSessionNum[i] > 1) {
					nLBA -= 11400 * (pDiscData->aSessionNum[i] - 1);
				}
				uiBufsize = (size_t)(nLBA - nPrevLba) * nWriteSectorSize;
			}
			if(!(bCDG && pDiscData->bAudioOnly)) {
				fseek(fpImg, nPrevLba * nWriteSectorSize, SEEK_SET);
			}
			PUCHAR pBuf = (PUCHAR)calloc(uiBufsize, sizeof(UCHAR));
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

	BOOL bRet = TRUE;
	if(0 < pDiscData->nCombinedOffset) {
		// read lead-out
		bRet = ReadCDPartial(pDevData, pDiscData, pszOutFile, pDiscData->nLength,
			pDiscData->nLength + pDiscData->nAdjustSectorNum - 1, READ_CD_FLAG::CDDA, FALSE, TRUE);
		if(bRet) {
			OutputLogString(fpLog, _T("Reading Lead-out: OK\n"));
		}
		else {
			OutputLogString(fpLog, _T("Reading Lead-out: NG\n"));
			return FALSE;
		}
	}
	else if(pDiscData->nCombinedOffset < 0) {
		// read lead-in
		bRet = ReadCDPartial(pDevData, pDiscData, pszOutFile,
			pDiscData->nAdjustSectorNum, -1, READ_CD_FLAG::CDDA, FALSE, TRUE);
		if(bRet) {
			OutputLogString(fpLog, _T("Reading Lead-in: OK\n"));
		}
		else {
			OutputLogString(fpLog, _T("Reading Lead-in: NG\n"));
			return FALSE;
		}
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
		if(NULL == (pLBAStartList = (PINT*)calloc(dwTrackAllocSize, sizeof(_INT)))) {
			throw _T("Failed to alloc memory pLBAStartList\n");
		}
		if(NULL == (pLBAOfDataTrackList = (PINT*)calloc(dwTrackAllocSize, sizeof(_INT)))) {
			throw _T("Failed to alloc memory pLBAOfDataTrackList\n");
		}
		if(NULL == (pbISRCList = (PBOOL)calloc(dwTrackAllocSize, sizeof(_INT)))) {
			throw _T("Failed to alloc memory pbISRCList\n");
		}
		if(NULL == (pCtlList = (PUCHAR)calloc(dwTrackAllocSize, sizeof(UCHAR)))) {
			throw _T("Failed to alloc memory pCtlList\n");
		}
		if(NULL == (pModeList = (PUCHAR)calloc(dwTrackAllocSize, sizeof(UCHAR)))) {
			throw _T("Failed to alloc memory pModeList\n");
		}
		if(NULL == (pEndCtlList = (PUCHAR)calloc(dwTrackAllocSize, sizeof(UCHAR)))) {
			throw _T("Failed to alloc memory pEndCtlList\n");
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

		ULONG ulBufLen = CD_RAW_SECTOR_WITH_SUBCODE_SIZE;
		_TCHAR c2Path[_MAX_PATH] = {0};
		if(pDevData->bC2ErrorData && (pDiscData->bAudioOnly || !pDevData->bPlextor)) {
			if(NULL == (fpC2 = CreateOrOpenFileW(pszOutFile, c2Path, NULL, NULL, _T(".c2.err"), _T("wb"), 0, 0))) {
				throw _T("Failed to open .c2.err\n");
			}
			if(pDevData->bPlextor) {
//				if(pDevData->bPlextor) {
					ulBufLen = CD_RAW_SECTOR_WITH_SUBCODE_SIZE + CD_RAW_READ_C2_SIZE_294;
//				}
			}
			else {
				ulBufLen = CD_RAW_SECTOR_WITH_C2_AND_SUBCODE_SIZE;
			}
		}
		if(NULL == (aBuf = (PUCHAR)calloc(ulBufLen + pDevData->AlignmentMask, sizeof(UCHAR)))) {
			throw _T("Failed to alloc memory aBuf[h]\n");
		}

		PUCHAR pBuf = (PUCHAR)ConvParagraphBoundary(aBuf, pDevData);
		UCHAR aCmd[CDB12GENERIC_LENGTH] = {0};
		UCHAR byTransferLen = 1;
		aCmd[0] = SCSIOP_READ_CD;
		aCmd[1] = READ_CD_FLAG::All;
		aCmd[8] = byTransferLen;
		aCmd[9] = READ_CD_FLAG::SyncData | READ_CD_FLAG::SubHeader | 
			READ_CD_FLAG::UserData | READ_CD_FLAG::MainHeader | READ_CD_FLAG::Edc;
		if(pDevData->bC2ErrorData && (pDiscData->bAudioOnly || !pDevData->bPlextor)) {
			if(pDevData->bPlextor) {
				if(pDevData->bPlextor) {
					aCmd[9] |= READ_CD_FLAG::C2ErrorBlockData;
				}
			}
			else {
				aCmd[9] |= READ_CD_FLAG::C2AndBlockErrorBits;
			}
		}
		// memo CD+G ripping
		// raw mode(001b) don't play CDG
		// plextor:D8                  -> play CDG OK
		// plextor:READ_CD_FLAG::Pack  -> play CDG OK
		// plextor:READ_CD_FLAG::Raw   -> play CDG NG (PQ is OK)
		// slimtype:READ_CD_FLAG::Pack -> play CDG NG (PQ is None)
		// slimtype:READ_CD_FLAG::Raw  -> play CDG NG (PQ is OK)
		aCmd[10] = READ_CD_FLAG::Raw;

		BOOL bCDG = FALSE;
		if(pDiscData->bAudioOnly) {
			if(!CheckCDG(pDevData, pDiscData, aCmd, ulBufLen, pBuf, &bCDG)) {
				throw _T("");
			}
			if(bCDG) {
				aCmd[10] = READ_CD_FLAG::Pack;
				if(NULL == (fpCdg = CreateOrOpenFileW(pszOutFile, NULL, NULL, NULL, _T(".cdg"), _T("wb"), 0, 0))) {
					throw _T("Failed to open .cdg\n");
				}
			}
		}
		else {
			ReadVolumeDescriptor(pDevData, &pDiscData->toc, aCmd, ulBufLen, pBuf, fpLog);
		}
		if(!pDiscData->bAudioOnly && pDevData->bPlextor) {
			// PX-504A don't support...
			// Sense data, Key:Asc:Ascq: 05:20:00(ILLEGAL_REQUEST. INVALID COMMAND OPERATION CODE)
			aCmd[0] = 0xD8;
			aCmd[1] = 0x00;
			aCmd[8] = 0x00;
			aCmd[9] = byTransferLen;
			aCmd[10] = 0x02; // 0=none, 1=Q(sub16[formatted]), 2=P-W(main+sub96[raw]), 3=P-W only(sub96[raw])
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
			if(pDevData->bC2ErrorData && pDiscData->bAudioOnly && pDevData->bPlextor) {
//				if(pDevData->bPlextor) {
					AlignRowSubcode(pBuf + CD_RAW_SECTOR_SIZE + CD_RAW_READ_C2_SIZE_294, Subcode);
//				}
//				else {
//					AlignRowSubcode(pBuf + CD_RAW_SECTOR_SIZE, Subcode);
//				}
			}
			else {
				AlignRowSubcode(pBuf + CD_RAW_SECTOR_SIZE, Subcode);
			}
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
			_T("\n-----------------------------------log begin------------------------------------\n"));

		for(INT nLBA = nOffsetStart; nLBA < pDiscData->nLength + nOffsetEnd; nLBA++) {
			if(pDiscData->nStartLBAof2ndSession != -1 && pDiscData->nLastLBAof1stSession == nLBA) {
				INT nStartSession2 = pDiscData->nStartLBAof2ndSession - 1;
				OutputLogString(fpLog, 
					_T("Skip from Leadout of Session 1 [%d] to Leadin of Session 2 [%d]\n"),
					pDiscData->nLastLBAof1stSession, nStartSession2);
				nLBA = nStartSession2;
				prevSubQ.nAbsoluteTime = nLBA + 150;
				continue;
			}
#if 0
			nLBA=75;
#endif
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
			if(pDevData->bC2ErrorData && pDiscData->bAudioOnly && pDevData->bPlextor) {
				// plextor PX755A:main+c2+sub
				if(pDevData->bPlextor) {
#if 0
					OutputMmcCdSub96Raw(pBuf + CD_RAW_SECTOR_SIZE + CD_RAW_READ_C2_SIZE_294, nLBA, fpLog);
#endif
					AlignRowSubcode(pBuf + CD_RAW_SECTOR_SIZE + CD_RAW_READ_C2_SIZE_294, Subcode);
				}
				else {
#if 0
					OutputMmcCdSub96Raw(pBuf + CD_RAW_SECTOR_SIZE, nLBA, fpLog);
#endif
					AlignRowSubcode(pBuf + CD_RAW_SECTOR_SIZE, Subcode);
				}
			}
			else {
				// slimtype:main+sub+c2
#if 0
				OutputMmcCdSub96Raw(pBuf + CD_RAW_SECTOR_SIZE, nLBA, fpLog);
#endif
				AlignRowSubcode(pBuf + CD_RAW_SECTOR_SIZE, Subcode);
			}
#if 0
			OutputMmcCdSub96Raw(Subcode, nLBA, fpLog);
#endif

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
				CheckAndFixSubchannel(pDiscData, Subcode, &subQ, &prevSubQ, 
					&prevPrevSubQ, &byCurrentTrackNum, &bCatalog, pbISRCList,
					pEndCtlList, pLBAStartList, pLBAOfDataTrackList, nLBA, fpLog);
				UCHAR SubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = {0};
				// fix raw subchannel
				AlignColumnSubcode(Subcode, SubcodeRaw);
				// 3rd:Write subchannel
				WriteSubChannel(pDevData, pDiscData, pBuf, Subcode,
					SubcodeRaw, nLBA, byCurrentTrackNum, fpSub, fpParse, fpCdg);
				PreserveTrackAttribution(pDiscData, nLBA, &byCurrentTrackNum,
					&subQ, &prevSubQ, &prevPrevSubQ, pCtlList, pModeList,
					pLBAStartList, pLBAOfDataTrackList, fpLog);
			}

			// 4th:Write track to scrambled
			WriteMainChannel(pDiscData, pBuf, pLBAStartList, 
				nLBA, nFixStartLBA, nFixEndLBA, uiShift, fpImg);
			if(pDevData->bC2ErrorData && (pDiscData->bAudioOnly || !pDevData->bPlextor)) {
				if(pDevData->bPlextor) {
					// plextor:main+c2+sub
//					if(pDevData->bPlextor) {
						fwrite(pBuf + CD_RAW_SECTOR_SIZE, 
							sizeof(UCHAR), CD_RAW_READ_C2_SIZE_294, fpC2);
//					}
				}
				else {
					// slimtype:main+sub+c2
					fwrite(pBuf + CD_RAW_SECTOR_WITH_SUBCODE_SIZE, 
						sizeof(UCHAR), CD_RAW_READ_C2_SIZE, fpC2);
				}
			}
			UpdateSubchannelQData(&subQ, &prevSubQ, &prevPrevSubQ);
			OutputString(
				_T("\rCreating img(LBA) %6d/%6d"), nLBA - nOffsetEnd, pDiscData->nLength - 1);
		}
		OutputString(_T("\n"));
		FcloseAndNull(fpCdg);
		FcloseAndNull(fpImg);
		FcloseAndNull(fpSub);
		if(pDevData->bC2ErrorData && (pDiscData->bAudioOnly || !pDevData->bPlextor)) {
			FcloseAndNull(fpC2);
//			if(pDevData->bPlextor && !pDevData->bPlextor) {
//				_tremove(c2Path);
//			}
		}
		FcloseAndNull(fpParse);
		FreeAndNull(aBuf);

		OutputLogString(fpLog, 
			_T("------------------------------------log end-------------------------------------\n\n"));
		OutputLogString(fpLog, _T("TOC with pregap\n"));
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
		if(pLBAStartList) {
			FreeAndNull(pLBAStartList[h]);
		}
		if(pLBAOfDataTrackList) {
			FreeAndNull(pLBAOfDataTrackList[h]);
		}
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
	if(pDiscData->toc.FirstTrack < 1 || 99 < pDiscData->toc.FirstTrack ||
		pDiscData->toc.LastTrack < 1 || 99 < pDiscData->toc.LastTrack) {
		return FALSE;
	}
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
	BOOL bGetDrive = GetDriveOffset(pDevData->pszProductId, &nDriveSampleOffset);
	if(!bGetDrive) {
		CHAR a[6];
		ZeroMemory(a, sizeof(a));
		OutputString(
			_T("This drive isn't defined in driveOffset.txt.\n")
			_T("Please input Drive offset(Samples) -> "));
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
		OutputLogString(fpLog, _T("\n\tDrive Offset(Byte) %d, (Samples) %d\n"), 
			pDiscData->nCombinedOffset, nDriveSampleOffset);
		bRet = TRUE;
	}
	else {
		UCHAR byScsiStatus = 0;
		UCHAR aCmd[CDB12GENERIC_LENGTH] = {0};
		PUCHAR aBuf = 
			(PUCHAR)calloc(CD_RAW_SECTOR_WITH_SUBCODE_SIZE * 2 + pDevData->AlignmentMask, sizeof(UCHAR));
		if(!aBuf) {
			return FALSE;
		}
		PUCHAR pBuf = (PUCHAR)ConvParagraphBoundary(aBuf, pDevData);
		INT nDriveOffset = nDriveSampleOffset * 4;
		UCHAR byTransferLen = 2;
		if(pDevData->bPlextor) {
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
			aCmd[10] = READ_CD_FLAG::Raw;
		}
		aCmd[2] = HIBYTE(HIWORD(pDiscData->nFirstDataLBA));
		aCmd[3] = LOBYTE(HIWORD(pDiscData->nFirstDataLBA));
		aCmd[4] = HIBYTE(LOWORD(pDiscData->nFirstDataLBA));
		aCmd[5] = LOBYTE(LOWORD(pDiscData->nFirstDataLBA));
		if(!ExecCommand(pDevData, aCmd, CDB12GENERIC_LENGTH, pBuf, 
			CD_RAW_SECTOR_WITH_SUBCODE_SIZE * 2, &byScsiStatus, _T(__FUNCTION__), __LINE__)) {
			FreeAndNull(aBuf);
			return FALSE;
		}
		if(byScsiStatus == 0) {
			OutputMmcCdMain2352(pBuf, pDiscData->nFirstDataLBA, fpLog);
			UCHAR Subcode[CD_RAW_READ_SUBCODE_SIZE] = {0};

			AlignRowSubcode(pBuf + CD_RAW_SECTOR_SIZE, Subcode);
			OutputMmcCdSub96Align(Subcode, pDiscData->nFirstDataLBA, fpLog);
			UCHAR aBuf2[CD_RAW_SECTOR_SIZE*2] = {0};
			memcpy(aBuf2, pBuf, CD_RAW_SECTOR_SIZE);
			memcpy(aBuf2 + CD_RAW_SECTOR_SIZE,
				pBuf + CD_RAW_SECTOR_WITH_SUBCODE_SIZE, CD_RAW_SECTOR_SIZE);
			bRet = GetWriteOffset(pDiscData, aBuf2);
		}
		else {
			// PLEXTOR(PX-W8432, PX-W1210T, PX-W2410T)
			// if Track1 is DataTrack (mostly game)
			// ==>Sense data, Key:Asc:Ascq: 05:64:00(ILLEGAL_REQUEST. ILLEGAL MODE FOR THIS TRACK)
			// else if Track1 isn't DataTrack (pc engine etc)
			// ==>no error.
			OutputString(
				_T("This drive can't read data sector in scrambled mode.\n"));
			FreeAndNull(aBuf);
			return FALSE;
		}

		if(bRet) {
			OutputLogString(fpLog, _T("Offset"));
			if(bGetDrive) {
				OutputLogString(fpLog,
					_T("(Drive offset data referes to www.accuraterip.com)"));
			}
			OutputLogString(fpLog,
				_T("\n")
				_T("\t Combined Offset(Byte) %6d, (Samples) %5d\n")
				_T("\t-   Drive Offset(Byte) %6d, (Samples) %5d\n")
				_T("\t----------------------------------------------\n")
				_T("\t       CD Offset(Byte) %6d, (Samples) %5d\n"),
				pDiscData->nCombinedOffset, pDiscData->nCombinedOffset / 4,
				nDriveOffset, nDriveSampleOffset,
				pDiscData->nCombinedOffset - nDriveOffset, 
				(pDiscData->nCombinedOffset - nDriveOffset) / 4);
		}
		else {
			OutputErrorString(_T("Failed to get write-offset[L:%d]\n"), __LINE__);
		}
		FreeAndNull(aBuf);
	}

	if(0 < pDiscData->nCombinedOffset) {
		pDiscData->nAdjustSectorNum =
			pDiscData->nCombinedOffset / CD_RAW_SECTOR_SIZE + 1;
	}
	else if(pDiscData->nCombinedOffset < 0) {
		pDiscData->nAdjustSectorNum =
			pDiscData->nCombinedOffset / CD_RAW_SECTOR_SIZE - 1;
	}
	OutputLogString(fpLog,
		_T("\tNeed overread sector: %d\n"), pDiscData->nAdjustSectorNum);
	fflush(fpLog);

	return bRet;
}

BOOL ReadCDPartial(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	LPCTSTR pszOutFile,
	INT nStart,
	INT nEnd,
	_READ_CD_FLAG::_SectorType flg,
	BOOL bDC,
	BOOL bCheckReading
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
	PUCHAR aBuf = NULL;
	try {
		if(NULL == (fpSub = CreateOrOpenFileW(pszOutFile, NULL, NULL, NULL, szSub, _T("wb"), 0, 0))) {
			throw _T("Failed to open .sub\n");
		}
		if(NULL == (fpParse = CreateOrOpenFileW(pszOutFile, NULL, NULL, NULL, szSubtxt, _T(WFLAG), 0, 0))) {
			throw _T("Failed to open .sub.txt\n");
		}
		UCHAR byScsiStatus = 0;
		UCHAR aCmd[CDB12GENERIC_LENGTH] = {0};
		aBuf = (PUCHAR)calloc(CD_RAW_SECTOR_WITH_SUBCODE_SIZE + pDevData->AlignmentMask, sizeof(UCHAR));
		if(!aBuf) {
			return FALSE;
		}
		PUCHAR pBuf = (PUCHAR)ConvParagraphBoundary(aBuf, pDevData);
		UCHAR Subcode[CD_RAW_READ_SUBCODE_SIZE] = {0};
		UCHAR byTransferLen = 1;
		if(pDevData->bPlextor) {
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
			aCmd[10] = READ_CD_FLAG::Raw;
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
				if(bCheckReading) {
					throw _T("");
				}
			}
			else {
				fwrite(pBuf, sizeof(UCHAR), CD_RAW_SECTOR_SIZE, fp);
				AlignRowSubcode(pBuf + CD_RAW_SECTOR_SIZE, Subcode);
				fwrite(Subcode, sizeof(UCHAR), CD_RAW_READ_SUBCODE_SIZE, fpSub);
				OutputMmcCdSubToLog(pDiscData, Subcode, pBuf + CD_RAW_SECTOR_SIZE,
					nLBA, BcdToDec(Subcode[13]) - 1, fpParse);
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
	FreeAndNull(aBuf);
	return bRet;
}

BOOL ReadBufferCapacity(
	PDEVICE_DATA pDevData,
	FILE* fpLog
	)
{
	UCHAR byScsiStatus = 0;
	UCHAR aCmd[CDB10GENERIC_LENGTH] = {0};
	UCHAR buf[12] = {0};

	aCmd[0] = SCSIOP_READ_BUFFER_CAPACITY;
	aCmd[8] = 12;

	if(!ExecCommand(pDevData, aCmd, CDB12GENERIC_LENGTH, buf,
		12, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	OutputMmcBufferCapacity(buf, fpLog);
	return TRUE;
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
		pDiscData->usCurrentMedia = ProfileCdrom;
		// not false. because undefined mmc1..
		OutputLogString(fpLog,
			_T("\tUndefined SCSIOP_GET_CONFIGURATION Command on this drive\n"));
		bRet = TRUE;
	}
	else {
		OutputLogString(fpLog, _T("\tCurrenProfile: "));
		pDiscData->usCurrentMedia =
			MAKEWORD(pHeader.CurrentProfile[1], pHeader.CurrentProfile[0]);
		OutputMmcFeatureProfileType(pDiscData->usCurrentMedia, fpLog);
		OutputLogString(fpLog, _T("\n"));

		ULONG ulAllLen =
			MAKELONG(MAKEWORD(pHeader.DataLength[3], pHeader.DataLength[2]), 
			MAKEWORD(pHeader.DataLength[1], pHeader.DataLength[0])) + sizeof(pHeader.DataLength);
		PUCHAR pPConf = (PUCHAR)calloc((size_t)ulAllLen + pDevData->AlignmentMask, sizeof(UCHAR));
		if(!pPConf) {
			OutputErrorString(_T("Can't alloc memory [F:%s][L:%d]\n"), _T(__FUNCTION__), __LINE__);
			return FALSE;
		}

		PUCHAR pConf = (PUCHAR)ConvParagraphBoundary(pPConf, pDevData);
		aCmd[7] = HIBYTE(ulAllLen);
		aCmd[8] = LOBYTE(ulAllLen);
		bRet = ExecCommand(pDevData, aCmd, CDB10GENERIC_LENGTH, pConf, 
			ulAllLen, &byScsiStatus, _T(__FUNCTION__), __LINE__);
		if(!bRet || byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			// not false. because undefined mmc1..
			OutputLogString(fpLog, _T("\tUndefined SCSIOP_GET_CONFIGURATION Command on this drive\n"));
			bRet = TRUE;
		}
		else {
			OutputMmcFeatureNumber(pDevData, pConf, ulAllLen, uiSize, fpLog);
		}
		FreeAndNull(pPConf);
	}
	return bRet;
}

BOOL ReadDiscInformation(
	PDEVICE_DATA pDevData,
	FILE* fpLog
	)
{
	UCHAR byScsiStatus = 0;
	UCHAR aCmd[CDB10GENERIC_LENGTH] = {0};
	USHORT usDiscInformationLength = 0;

	aCmd[0] = SCSIOP_READ_DISC_INFORMATION;
	aCmd[8] = sizeof(usDiscInformationLength);

	BOOL bRet = ExecCommand(pDevData, aCmd, CDB10GENERIC_LENGTH, &usDiscInformationLength,
		sizeof(usDiscInformationLength), &byScsiStatus, _T(__FUNCTION__), __LINE__);
	if(!bRet || byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		OutputErrorString(_T("error\n"));
	}
	else {
		USHORT ulAllLen = usDiscInformationLength + sizeof(usDiscInformationLength);
		PUCHAR pPInfo = (PUCHAR)calloc((size_t)ulAllLen + pDevData->AlignmentMask, sizeof(UCHAR));
		if(!pPInfo) {
			OutputErrorString(_T("Can't alloc memory [F:%s][L:%d]\n"), _T(__FUNCTION__), __LINE__);
			bRet = FALSE;
		}
		else {
			PUCHAR pInfo = (PUCHAR)ConvParagraphBoundary(pPInfo, pDevData);
			aCmd[7] = HIBYTE(ulAllLen);
			aCmd[8] = HIBYTE(ulAllLen);
			bRet = ExecCommand(pDevData, aCmd, CDB10GENERIC_LENGTH, pInfo,
				ulAllLen, &byScsiStatus, _T(__FUNCTION__), __LINE__);
			if(!bRet || byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				OutputErrorString(_T("error\n"));
			}
			else {
				OutputMmcDiscInformation(pInfo, fpLog);
			}
		}
	}
	return bRet;
}

BOOL ReadDVD(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	LPCTSTR pszOutFile,
	LPCTSTR pszOption,
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
	PUCHAR aBuf2 = NULL;
	BOOL bRet = TRUE;
	try {
		pPBuf = (PUCHAR)calloc(CD_RAW_READ * (size_t)byTransferLen + pDevData->AlignmentMask, sizeof(UCHAR));
		if(pPBuf == NULL) {
			throw _T("Can't alloc memory aBuf\n");
		}
		PUCHAR pBuf = (PUCHAR)ConvParagraphBoundary(pPBuf, pDevData);

		UCHAR aCmd[CDB12GENERIC_LENGTH] = {0};
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
			OutputFsVolumeRecognitionSequence(pBuf, i, fpLog);
		}
		aCmd[5] = LOBYTE(32);
		if(!ExecCommand(pDevData, aCmd, CDB12GENERIC_LENGTH, pBuf, 
			(ULONG)CD_RAW_READ * byTransferLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw _T("");
		}
		if(pBuf[20] == 0 && pBuf[21] == 0 && pBuf[22] == 0 && pBuf[23] == 0) {
			for(INT i = 0; i <= CD_RAW_READ * 5; i += CD_RAW_READ) {
				OutputFsVolumeDescriptorSequence(pBuf, i, fpLog);
			}
		}
		aCmd[4] = HIBYTE(LOWORD(256));
		aCmd[5] = LOBYTE(LOWORD(256));
		if(!ExecCommand(pDevData, aCmd, CDB12GENERIC_LENGTH, pBuf, 
			(ULONG)CD_RAW_READ * byTransferLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw _T("");
		}
		OutputFsVolumeDescriptorSequence(pBuf, 0, fpLog);
		fflush(fpLog);

		aBuf2 = (PUCHAR)calloc(sizeof(DVD_DESCRIPTOR_HEADER) +
			sizeof(DVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR) +
			pDevData->AlignmentMask, sizeof(UCHAR));
		if(!aBuf2) {
			throw _T("");
		}
		UCHAR aCmd2[CDB10GENERIC_LENGTH] = {0};
		PUCHAR pBuf2 = NULL;
		size_t uiSize = 0;
		if(pszOption && !_tcscmp(pszOption, _T("cmi"))) {
			pBuf2 = (PUCHAR)ConvParagraphBoundary(aBuf2, pDevData);
			uiSize = sizeof(DVD_DESCRIPTOR_HEADER) + 
				sizeof(DVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR);
			aCmd2[0] = SCSIOP_READ_DVD_STRUCTURE;
			aCmd2[7] = DvdMaxDescriptor;
			aCmd2[8] = HIBYTE(LOWORD(uiSize));
			aCmd2[9] = LOBYTE(LOWORD(uiSize));
			OutputLogString(fpLog, _T("\tCopyrightManagementInformation\n"));
		}
		for(INT nLBA = 0; nLBA < pDiscData->nLength; nLBA += byTransferLen) {
			if(pDiscData->nLength - nLBA < byTransferLen) {
				byTransferLen = (UCHAR)(pDiscData->nLength - nLBA);
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
					OutputMmcDVDCopyrightManagementInformation(pBuf2, nLBA, i, fpLog);
				}
			}
			OutputString(_T("\rCreating iso(LBA) %8u/%8u"), 
				nLBA + byTransferLen - 1, pDiscData->nLength - 1);
		}
		OutputString(_T("\n"));
	}
	catch(LPTSTR str) {
		OutputErrorString(str);
		bRet = FALSE;
	}
	FreeAndNull(pPBuf);
	FreeAndNull(aBuf2);
	FcloseAndNull(fp);
	return bRet;
}

BOOL ReadDVDRaw(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	LPCSTR pszVendorId,
	LPCTSTR pszOutFile
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
		if(NULL == (aBuf = (PUCHAR)calloc(DVD_RAW_READ *
			(size_t)byTransferLen + pDevData->AlignmentMask, sizeof(UCHAR)))) {
			throw _T("Can't alloc memory aBuf\n");
		}
		PUCHAR pBuf = (PUCHAR)ConvParagraphBoundary(aBuf, pDevData);
		UCHAR cdblen = CDB12GENERIC_LENGTH;
		UCHAR aCmd[CDB12GENERIC_LENGTH] = {0};
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

		for(INT nLBA = 0; nLBA < pDiscData->nLength; nLBA += byTransferLen) {
			if(pDiscData->nLength - nLBA < byTransferLen) {
				byTransferLen = (UCHAR)(pDiscData->nLength - nLBA);
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
			OutputString(_T("\rCreating raw(LBA) %7d/%7d"), 
				nLBA + byTransferLen - 1, pDiscData->nLength - 1);
		}
		OutputString(_T("\n"));
	}
	catch(LPTSTR str) {
		OutputErrorString(str);
		bRet = FALSE;
	}
	FreeAndNull(aBuf);
	FcloseAndNull(fp);
#if 0 // TODO
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
	PDISC_DATA pDiscData,
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
	PUCHAR pFormat = (PUCHAR)calloc(uiFormatSize, sizeof(UCHAR));
	if(!pFormat) {
		OutputErrorString(_T("Can't alloc memory [L:%d]\n"), __LINE__);
		return FALSE;
	}
	PUSHORT pStructureLength = NULL;
	PUCHAR pStructure = NULL;
	BOOL bRet = TRUE;
	try {
		if(NULL == (pStructureLength =(PUSHORT)calloc(uiFormatSize, sizeof(USHORT)))) {
			throw _T("Can't alloc memory pStructureLength\n");
		}
		OutputLogString(fpLog, _T("DVD Structure\n"));
		for(INT i = 0, j = 0; i < nAllStructureSize; i+=4, j++) {
			if(i == 0) {
				OutputLogString(fpLog, _T("\tDisc Structure List\n"));
			}
			OutputLogString(fpLog, 
				_T("\t\tFormatCode: %02x, SDR: %3s, RDS: %3s, Structure Length: %d\n"), 
				pDiscStructure[4+i], 
				BOOLEAN_TO_STRING_YES_NO(pDiscStructure[5+i] & 0x80), 
				BOOLEAN_TO_STRING_YES_NO(pDiscStructure[5+i] & 0x40), 
				(pDiscStructure[6+i] << 8) | pDiscStructure[7+i]);
			pFormat[j] = pDiscStructure[4+i];
			pStructureLength[j] = 
				MAKEWORD(pDiscStructure[7+i], pDiscStructure[6+i]);
		}

		UCHAR nLayerNum = 0;
		for(size_t i = 0; i < uiFormatSize; i++) {
			if(pStructureLength[i] == 0 || pFormat[i] == 0x05 || pFormat[i] == 0xff) {
				continue;
			}
			if(NULL == (pStructure = (PUCHAR)calloc(pStructureLength[i], sizeof(UCHAR)))) {
				throw _T("Can't alloc memory pStructure\n");
			}
			aCmd[6] = 0;
			aCmd[7] = pFormat[i];
			aCmd[8] = HIBYTE(pStructureLength[i]);
			aCmd[9] = LOBYTE(pStructureLength[i]);

			if(!ExecCommand(pDevData, aCmd, CDB12GENERIC_LENGTH, pStructure, 
				pStructureLength[i], &byScsiStatus, _T(__FUNCTION__), __LINE__)) {
				OutputErrorString(_T("Failure - Format %02x\n"), pFormat[i]);
			}
			else {
				if(byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
					OutputErrorString(_T("Failure - Format %02x\n"), pFormat[i]);
				}
				else {
					OutputMmcDVDStructureFormat(pDiscData, 0, pFormat,
						pStructure, pStructureLength, &nLayerNum, nDVDSectorSize, i, fpLog);
					if(nLayerNum == 1 &&
						(pFormat[i] == 0 || pFormat[i] == 0x01 || pFormat[i] == 0x04 ||
						pFormat[i] == 0x10 || pFormat[i] == 0x12 || pFormat[i] == 0x15)) {
						aCmd[6] = nLayerNum;
						if(!ExecCommand(pDevData, aCmd, CDB12GENERIC_LENGTH, pStructure, 
							pStructureLength[i], &byScsiStatus, _T(__FUNCTION__), __LINE__)) {
							OutputErrorString(_T("Failure - Format %02x\n"), pFormat[i]);
						}
						else {
							if(byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
								OutputErrorString(_T("Failure - Format %02x\n"), pFormat[i]);
							}
							else {
								OutputMmcDVDStructureFormat(pDiscData, 1, pFormat,
									pStructure, pStructureLength, &nLayerNum, nDVDSectorSize, i, fpLog);
							}
						}
					}
				}
			}
			FreeAndNull(pStructure);
		}
	}
	catch(LPTSTR str) {
		OutputErrorString(str);
		bRet = FALSE;
	}
	FreeAndNull(pFormat);
	FreeAndNull(pStructure);
	FreeAndNull(pStructureLength);
	return bRet;
}

BOOL ReadInquiryData(
	PDEVICE_DATA pDevData,
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
	OutputMmcInquiryData(pDevData, &inquiryData, fpLog);
	return TRUE;
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
	aCmd[6] = 1;
	aCmd[7] = HIBYTE(CDROM_TOC_SIZE);
	aCmd[8] = LOBYTE(CDROM_TOC_SIZE);

	if(!ExecCommand(pDevData, aCmd, CDB10GENERIC_LENGTH, &pDiscData->toc, 
		CDROM_TOC_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__) || 
		byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		OutputErrorString(_T("Can't get toc\n"));
		return FALSE;
	}
	OutputMmcToc(pDiscData, fpLog);
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
	size_t uiFullTocHeaderSize = sizeof(CDROM_TOC_FULL_TOC_DATA);

	aCmd[0] = SCSIOP_READ_TOC;
	aCmd[2] = CDROM_READ_TOC_EX_FORMAT_FULL_TOC;
	aCmd[6] = 1;
	aCmd[7] = HIBYTE(uiFullTocHeaderSize);
	aCmd[8] = LOBYTE(uiFullTocHeaderSize);

	if(!ExecCommand(pDevData, aCmd, CDB10GENERIC_LENGTH, &fullToc,
		(ULONG)uiFullTocHeaderSize, &byScsiStatus, _T(__FUNCTION__), __LINE__) 
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		OutputErrorString(_T("Can't get CDROM_TOC_FULL_TOC_DATA 1\n"));
		return FALSE;
	}

	size_t uiFullTocSize = 
		MAKEWORD(fullToc.Length[1], fullToc.Length[0]) - sizeof(fullToc.Length);
	size_t uiTocEntries = uiFullTocSize / sizeof(CDROM_TOC_FULL_TOC_DATA_BLOCK);

	WriteCcdFileForDisc(uiTocEntries, fullToc.LastCompleteSession, fpCcd);
	if(pDevData->bCanCDText) {
		ReadTOCText(pDevData, pDiscData, fpLog, fpCcd);
	}
	size_t uiFullTocStructSize = uiFullTocSize + uiFullTocHeaderSize;
	if(pDevData->bPlextor && pDevData->bPlextorPX712A && uiFullTocStructSize > 1028) {
		OutputErrorString(
			_T("On this drive, can't get CDROM_TOC_FULL_TOC_DATA of this disc\n"));
		return TRUE;
	}
	size_t uiFullTocFixStructSize = uiFullTocStructSize;
	// 4 byte padding
	if(uiFullTocFixStructSize % 4) {
		uiFullTocFixStructSize = (uiFullTocFixStructSize / 4 + 1) * 4;
	}
	OutputLogString(fpLog,
		_T("FullTocSize: %d, FullTocStructSize: %d, FullTocStructFixSize: %d\n"),
		uiFullTocSize, uiFullTocStructSize, uiFullTocFixStructSize);

	PUCHAR pPFullToc = 
		(PUCHAR)calloc(uiFullTocFixStructSize + pDevData->AlignmentMask, sizeof(UCHAR));
	if(!pPFullToc) {
		OutputErrorString(_T("Can't alloc memory [L:%d]\n"), __LINE__);
		return FALSE;
	}
	PUCHAR pFullToc = (PUCHAR)ConvParagraphBoundary(pPFullToc, pDevData);

	aCmd[7] = HIBYTE(uiFullTocFixStructSize);
	aCmd[8] = LOBYTE(uiFullTocFixStructSize);

	BOOL bRet = TRUE;
	try {
		if(!ExecCommand(pDevData, aCmd, CDB10GENERIC_LENGTH, pFullToc, 
			(ULONG)uiFullTocFixStructSize, &byScsiStatus, _T(__FUNCTION__), __LINE__) 
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			throw _T("Can't get CDROM_TOC_FULL_TOC_DATA 2\n");
		}
		CDROM_TOC_FULL_TOC_DATA_BLOCK* pTocData = 
			((CDROM_TOC_FULL_TOC_DATA*)pFullToc)->Descriptors;
		size_t uiIdx = 0;
		INT aLBA[] = {0, pDiscData->nFirstDataLBA};
		for(size_t b = 0; b < uiTocEntries; b++) {
			if(pTocData[b].Point < 100 && uiIdx < pTocData[b].SessionNumber) {
				PUCHAR aBuf2 = 
					(PUCHAR)calloc(CD_RAW_SECTOR_WITH_SUBCODE_SIZE + pDevData->AlignmentMask, sizeof(UCHAR));
				if(!aBuf2) {
					return FALSE;
				}
				UCHAR aCmd2[CDB12GENERIC_LENGTH] = {0};
				PUCHAR pBuf = (PUCHAR)ConvParagraphBoundary(aBuf2, pDevData);

				if(pDevData->bPlextor) {
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
					aCmd2[10] = READ_CD_FLAG::Raw;
				}
				UCHAR ucMode = 0;
				for(INT nLBA = aLBA[uiIdx]; nLBA < aLBA[uiIdx] + 100; nLBA++) {
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
					if(nLBA == aLBA[uiIdx]) {
						ucMode = GetMode(pBuf);
					}
					UCHAR Subcode[CD_RAW_READ_SUBCODE_SIZE] = {0};
					AlignRowSubcode(pBuf + CD_RAW_SECTOR_SIZE, Subcode);

					UCHAR byAdr = (UCHAR)(Subcode[12] & 0x0F);
					if(byAdr == ADR_ENCODES_MEDIA_CATALOG) {
						BOOL bMCN = IsValidMCN(Subcode);
						_TCHAR szCatalog[META_CATALOG_SIZE+1] = {0};
						SetMCNToString(pDiscData, Subcode, szCatalog, bMCN);
						WriteCcdFileForDiscCatalog(pDiscData, fpCcd);
						break;
					}
				}
				FreeAndNull(aBuf2);
				WriteCcdFileForSession(pTocData[b].SessionNumber, fpCcd);
				WriteCcdFileForSessionPregap(ucMode, fpCcd);
				uiIdx++;
			}
		}
		OutputMmcTocFull(pDiscData, &fullToc, pTocData, uiTocEntries, fpCcd, fpLog);
	}
	catch(LPTSTR str) {
		OutputErrorString(str);
		bRet = FALSE;
	}
	FreeAndNull(pPFullToc);
	return bRet;
}

BOOL ReadTOCText(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
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

	BOOL bRet = ExecCommand(pDevData, aCmd, CDB10GENERIC_LENGTH, &tocText,
		(ULONG)uiCDTextDataSize, &byScsiStatus, _T(__FUNCTION__), __LINE__);
	OutputLogString(fpLog, _T("CDTEXT on SCSIOP_READ_TOC\n"));
	if(!bRet || byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		// not false. because undefined mmc1..
		OutputLogString(fpLog,
			_T("\tUndefined CDROM_READ_TOC_EX_FORMAT_CDTEXT on this drive\n"));
		bRet = TRUE;
	}
	else {
		size_t uiTocTextsize = 
			MAKEWORD(tocText.Length[1], tocText.Length[0]) - sizeof(tocText.Length);
		WriteCcdFileForDiscCDTextLength(uiTocTextsize, fpCcd);

		size_t uiTocTextEntries = uiTocTextsize / sizeof(CDROM_TOC_CD_TEXT_DATA_BLOCK);
		if(!uiTocTextEntries) {
			OutputLogString(fpLog, _T("\tNothing\n"));
			// almost CD is nothing text
			return TRUE;
		}
		WriteCcdFileForCDText(uiTocTextEntries, fpCcd);

		PUCHAR pPTocText = NULL;
		PCHAR pTmpText = NULL;
		try {
			size_t uiCDTextDataMaxSize = uiTocTextsize + uiCDTextDataSize;
			if(NULL == (pPTocText = (PUCHAR)calloc(uiCDTextDataMaxSize + pDevData->AlignmentMask, sizeof(UCHAR)))) {
				OutputErrorString(_T("Can't alloc memory [L:%d]\n"), __LINE__);
				throw;
			}
			PUCHAR pTocText = (PUCHAR)ConvParagraphBoundary(pPTocText, pDevData);
			aCmd[7] = HIBYTE(uiCDTextDataMaxSize);
			aCmd[8] = LOBYTE(uiCDTextDataMaxSize);
			if(!ExecCommand(pDevData, aCmd, CDB10GENERIC_LENGTH, pTocText, 
				(ULONG)uiCDTextDataMaxSize, &byScsiStatus, _T(__FUNCTION__), __LINE__) 
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw _T("Can't get CDROM_TOC_CD_TEXT_DATA_BLOCK\n");
			}
			PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc = 
				((PCDROM_TOC_CD_TEXT_DATA)pTocText)->Descriptors;
			WriteCcdFileForCDTextEntry(pDesc, uiTocTextEntries, fpCcd);

			size_t allTextSize = uiTocTextEntries * sizeof(pDesc->Text);
			if(NULL == (pTmpText = (PCHAR)calloc(allTextSize, sizeof(_TCHAR)))) {
				OutputErrorString(_T("Can't alloc memory [L:%d]\n"), __LINE__);
				throw;
			}
			size_t entrySize = 0;
			BOOL bUnicode = FALSE;
			while(entrySize < uiTocTextEntries) {
				if(pDesc[entrySize].Unicode == 1) {
					bUnicode = TRUE;
					break;
				}
				entrySize++;
			}
			OutputMmcTocCDText(pDiscData, pDesc, pTmpText, entrySize, allTextSize, fpLog);
			if(bUnicode) {
				PWCHAR pTmpWText = NULL;
				if(NULL == (pTmpWText = (PWCHAR)calloc(allTextSize, sizeof(UCHAR)))) {
					OutputErrorString(_T("Can't alloc memory [L:%d]\n"), __LINE__);
					throw;
				}
				OutputMmcTocCDWText(pDiscData, pDesc, pTmpText,
					entrySize, uiTocTextEntries, allTextSize, fpLog);
				FreeAndNull(pTmpWText);
			}
		}
		catch(LPTSTR str) {
			OutputErrorString(str);
			bRet = FALSE;
		}
		FreeAndNull(pPTocText);
		FreeAndNull(pTmpText);
	}
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
	OutputMmcDriveSpeed(&setspeed, fpLog);
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
