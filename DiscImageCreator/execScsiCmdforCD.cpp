/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "check.h"
#include "convert.h"
#include "execScsiCmd.h"
#include "execScsiCmdforCD.h"
#include "execIoctl.h"
#include "get.h"
#include "init.h"
#include "output.h"
#include "outputScsiCmdLog.h"
#include "outputScsiCmdLogforCD.h"
#include "set.h"
#include "_external/crc16ccitt.h"

// These global variable is set at DiscImageCreator.cpp
extern BYTE g_aSyncHeader[SYNC_SIZE];
// These global variable is set at prngcd.cpp
extern unsigned char scrambled_table[2352];

VOID ExecEccEdc(
	BYTE byReadContinue,
	LPCTSTR pszImgPath,
	_DISC::_PROTECT::_ERROR_SECTOR errorSector
	)
{
	CONST INT nCmdSize = 6;
	CONST INT nStrSize = _MAX_PATH * 2 + nCmdSize;
	_TCHAR str[nStrSize] = { 0 };
	_TCHAR cmd[nCmdSize] = { _T("check") };
	INT nStartLBA = errorSector.nExtentPos;
	INT nEndLBA = errorSector.nExtentPos + errorSector.nSectorSize;
	if (byReadContinue) {
		ZeroMemory(cmd, sizeof(cmd));
		_tcsncpy(cmd, _T("fix"), 3);
	}
	if (GetEccEdcCmd(str, nStrSize, cmd, pszImgPath, nStartLBA, nEndLBA)) {
		OutputString(_T("Exec %s\n"), str);
		_tsystem(str);
	}
}

BOOL ExecReadCD(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	LPBYTE lpCmd,
	INT nLBA,
	LPBYTE lpBuf,
	DWORD dwBufSize,
	LPCTSTR pszFuncName,
	LONG lLineNum
	)
{
	lpCmd[2] = HIBYTE(HIWORD(nLBA));
	lpCmd[3] = LOBYTE(HIWORD(nLBA));
	lpCmd[4] = HIBYTE(LOWORD(nLBA));
	lpCmd[5] = LOBYTE(LOWORD(nLBA));
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, lpCmd, CDB12GENERIC_LENGTH, lpBuf,
		dwBufSize, &byScsiStatus, pszFuncName, lLineNum)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	return TRUE;
}

BOOL ExecReadCDForC2(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	LPBYTE lpCmd,
	INT nLBA,
	LPBYTE lpBuf,
	BOOL bCheckReading,
	LPCTSTR pszFuncName,
	LONG lLineNum
	)
{
	lpCmd[2] = HIBYTE(HIWORD(nLBA));
	lpCmd[3] = LOBYTE(HIWORD(nLBA));
	lpCmd[4] = HIBYTE(LOWORD(nLBA));
	lpCmd[5] = LOBYTE(LOWORD(nLBA));
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, lpCmd, CDB12GENERIC_LENGTH, lpBuf,
		pDevice->TRANSFER.dwBufLen + pDevice->TRANSFER.dwAdditionalBufLen,
		&byScsiStatus, pszFuncName, lLineNum)) {
		if (pExtArg->byReadContinue) {
			return RETURNED_CONTINUE;
		}
		else {
			return RETURNED_FALSE;
		}
	}

	if (byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		if (*pExecType != gd && !bCheckReading) {
			return RETURNED_CONTINUE;
		}
		else {
			return RETURNED_FALSE;
		}
	}
	return RETURNED_NO_C2_ERROR_1ST;
}

// http://tmkk.undo.jp/xld/secure_ripping.html
// https://forum.dbpoweramp.com/showthread.php?33676
BOOL FlushDriveCache(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	INT nLBA
	)
{
	INT NextLBAAddress = nLBA - 1;
//	INT NextLBAAddress = nLBA + 1;
#if 1
	CDB::_READ12 cdb = { 0 };
	cdb.OperationCode = SCSIOP_READ12;
	cdb.ForceUnitAccess = CDB_FORCE_MEDIA_ACCESS;
	cdb.LogicalUnitNumber = pDevice->address.Lun;
	cdb.LogicalBlock[0] = HIBYTE(HIWORD(NextLBAAddress));
	cdb.LogicalBlock[1] = LOBYTE(HIWORD(NextLBAAddress));
	cdb.LogicalBlock[2] = HIBYTE(LOWORD(NextLBAAddress));
	cdb.LogicalBlock[3] = LOBYTE(LOWORD(NextLBAAddress));
#else
	// Doesn't support FORCE_MEDIA_ACCESS in 0xd8 command
	CDB::_PLXTR_READ_CDDA cdb = { 0 };
	cdb.OperationCode = SCSIOP_PLXTR_READ_CDDA;
	cdb.Reserved0 = CDB_FORCE_MEDIA_ACCESS;
	cdb.LogicalUnitNumber = pDevice->address.Lun;
	cdb.LogicalBlockByte0 = HIBYTE(HIWORD(NextLBAAddress));
	cdb.LogicalBlockByte1 = LOBYTE(HIWORD(NextLBAAddress));
	cdb.LogicalBlockByte2 = HIBYTE(LOWORD(NextLBAAddress));
	cdb.LogicalBlockByte3 = LOBYTE(LOWORD(NextLBAAddress));
	cdb.TransferBlockByte3 = 1;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, (LPBYTE)&cdb, CDB12GENERIC_LENGTH,
		NULL, 0, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	return TRUE;
}

BOOL ProcessReadCD(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	UINT uiC2ErrorLBACnt,
	LPBYTE lpCmd,
	INT nLBA,
	PDISC_PER_SECTOR pDiscPerSector,
	BOOL bCheckReading
	)
{
	BOOL bRet = RETURNED_NO_C2_ERROR_1ST;
	if (pDevice->bySuccessReadTocFull && *pExecType != gd) {
		if (pDisc->SCSI.nFirstLBAof2ndSession != -1) {
			if (pExtArg->byReverse) {
				if (pDisc->SCSI.nFirstLBAof2ndSession == nLBA + 1) {
					OutputMainInfoLogA(
						"Skip from Leadout of Session 1 [%d, %#x] to Leadin of Session 2 [%d, %#x]\n",
						pDisc->SCSI.nFirstLBAofLeadout, pDisc->SCSI.nFirstLBAofLeadout, 
						pDisc->SCSI.nFirstLBAof2ndSession - 1, pDisc->SCSI.nFirstLBAof2ndSession - 1);
					pDiscPerSector->subQ.prev.nAbsoluteTime = nLBA - SESSION_TO_SESSION_SKIP_LBA - 150;
					return RETURNED_SKIP_LBA;
				}
			}
			else {
				if (pDisc->MAIN.nFixFirstLBAofLeadout == nLBA) {
					OutputMainInfoLogA(
						"Skip from Leadout of Session 1 [%d, %#x] to Leadin of Session 2 [%d, %#x]\n",
						pDisc->SCSI.nFirstLBAofLeadout, pDisc->SCSI.nFirstLBAofLeadout, 
						pDisc->SCSI.nFirstLBAof2ndSession - 1, pDisc->SCSI.nFirstLBAof2ndSession - 1);
					if (pDisc->MAIN.nCombinedOffset > 0) {
						pDiscPerSector->subQ.prev.nAbsoluteTime = 
							nLBA + SESSION_TO_SESSION_SKIP_LBA + 150 - pDisc->MAIN.nAdjustSectorNum - 1;
					}
					else if (pDisc->MAIN.nCombinedOffset < 0) {
						pDiscPerSector->subQ.prev.nAbsoluteTime = 
							nLBA + SESSION_TO_SESSION_SKIP_LBA + 150 + pDisc->MAIN.nAdjustSectorNum;
					}
					return RETURNED_SKIP_LBA;
				}
			}
		}
	}
	if (pExtArg->byFua) {
		FlushDriveCache(pExtArg, pDevice, nLBA);
	}
	bRet = ExecReadCDForC2(pExecType, pExtArg, pDevice, lpCmd, nLBA,
		pDiscPerSector->data.present, bCheckReading, _T(__FUNCTION__), __LINE__);
#if 0
	if (0 <= nLBA && nLBA <= 10) {
		OutputCDMain(fileMainInfo, pDiscPerSector->data.present, nLBA, CD_RAW_SECTOR_SIZE);
	}
#endif
	if (bRet == RETURNED_NO_C2_ERROR_1ST) {
		if (!bCheckReading) {
			AlignRowSubcode(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufSubOffset, pDiscPerSector->subcode.present);
			if (pC2ErrorPerSector && pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
				if (pExtArg->byReadContinue && pDisc->PROTECT.byExist &&
					(pDisc->PROTECT.ERROR_SECTOR.nExtentPos <= nLBA &&
						nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize)) {
					// skip check c2 error
					ZeroMemory(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufC2Offset, CD_RAW_READ_C2_294_SIZE);
				}
				else {
					bRet = ContainsC2Error(
						pC2ErrorPerSector, pDevice, pDisc, pDiscPerSector->data.present, uiC2ErrorLBACnt);
				}
			}
			if (!(pExtArg->byReadContinue && pDisc->PROTECT.byExist &&
				(pDisc->PROTECT.ERROR_SECTOR.nExtentPos <= nLBA &&
				nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize))) {
				if (pDiscPerSector->data.next != NULL && 1 <= pExtArg->dwSubAddionalNum) {
					ExecReadCDForC2(pExecType, pExtArg, pDevice, lpCmd,
						nLBA + 1, pDiscPerSector->data.next, bCheckReading, _T(__FUNCTION__), __LINE__);
					AlignRowSubcode(pDiscPerSector->data.next + pDevice->TRANSFER.dwBufSubOffset, pDiscPerSector->subcode.next);
					if (pDiscPerSector->data.nextNext != NULL && 2 <= pExtArg->dwSubAddionalNum) {
						ExecReadCDForC2(pExecType, pExtArg, pDevice, lpCmd,
							nLBA + 2, pDiscPerSector->data.nextNext, bCheckReading, _T(__FUNCTION__), __LINE__);
						AlignRowSubcode(pDiscPerSector->data.nextNext + pDevice->TRANSFER.dwBufSubOffset, pDiscPerSector->subcode.nextNext);
					}
				}
			}
		}
	}
	if (!bCheckReading) {
		if (pExtArg->byReadContinue && pDisc->PROTECT.byExist &&
			(pDisc->PROTECT.ERROR_SECTOR.nExtentPos <= nLBA &&
				nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize)) {
			if (bRet == RETURNED_CONTINUE) {
				if (pC2ErrorPerSector && pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
					// skip check c2 error
					ZeroMemory(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufC2Offset, CD_RAW_READ_C2_294_SIZE);
				}
			}
			// replace sub to sub of prev
			ZeroMemory(pDiscPerSector->data.present + pDevice->TRANSFER.dwBufSubOffset, CD_RAW_READ_SUBCODE_SIZE);
			ZeroMemory(pDiscPerSector->subcode.present, CD_RAW_READ_SUBCODE_SIZE);
			SetBufferFromSubQData(pDiscPerSector->subQ.prev, pDiscPerSector->subcode.present, 0);
		}
	}
	return bRet;
}

BOOL ExecSearchingOffset(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE lpCmd,
	LPBYTE lpBuf,
	DWORD dwBufSize,
	BOOL bGetDrive,
	INT nDriveSampleOffset,
	INT nDriveOffset
	)
{
	if (!ExecReadCD(pExtArg, pDevice, lpCmd, pDisc->SCSI.nFirstLBAofDataTrack, lpBuf,
		dwBufSize, _T(__FUNCTION__), __LINE__)) {
		if (*pExecType == gd) {
			OutputErrorString(
				_T("Couldn't read a data sector at scrambled mode\n")
				_T("Please start it again from 1st step or after waiting a little\n"));
		}
		else {
			if (pExtArg->byD8) {
				OutputLogA(standardErr | fileDrive,
					"This drive doesn't support [command: %#02x, subch: %#02x]\n"
					, lpCmd[0], lpCmd[10]);
			}
			else {
				OutputErrorString(
					_T("This drive can't read a data sector at scrambled mode\n")
					_T("Please start it again at plextor drive (PX-708, 712, 716, 755, 760 etc)\n"));
			}
		}
		return FALSE;
	}
	else {
		if (pExtArg->byD8) {
			OutputLogA(standardErr | fileDrive,
				"This drive support [command: %#02x, subch: %#02x]\n"
				, lpCmd[0], lpCmd[10]);
		}
	}
	if (pExtArg->byD8) {
		if (lpCmd[10] == 0x01 || lpCmd[10] == 0x03) {
			// check only
			return TRUE;
		}
		OutputDiscLogA(
			"======================= Check Drive+CD offset (Sub:%02x) ========================\n"
			, lpCmd[10]);
	}
	else {
		OutputDiscLogA(
			"============================ Check Drive+CD offset ============================\n");
	}
	OutputCDMain(fileDisc, lpBuf, pDisc->SCSI.nFirstLBAofDataTrack, CD_RAW_SECTOR_SIZE);
	if (dwBufSize != CD_RAW_SECTOR_SIZE) {
		BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		if (dwBufSize == CD_RAW_SECTOR_WITH_SUBCODE_SIZE) {
			AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
		}
		else if (dwBufSize == CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE) {
			AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE + CD_RAW_READ_C2_294_SIZE, lpSubcode);
		}
		OutputCDSub96Align(lpSubcode, pDisc->SCSI.nFirstLBAofDataTrack);
	}

	BYTE aBuf2[CD_RAW_SECTOR_SIZE * 2] = { 0 };
	memcpy(aBuf2, lpBuf, CD_RAW_SECTOR_SIZE);

	if (!ExecReadCD(pExtArg, pDevice, lpCmd, pDisc->SCSI.nFirstLBAofDataTrack + 1, lpBuf,
		dwBufSize, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	memcpy(aBuf2 + CD_RAW_SECTOR_SIZE, lpBuf, CD_RAW_SECTOR_SIZE);
	if (!GetWriteOffset(pDisc, aBuf2)) {
		OutputErrorString(_T("Failed to get write-offset\n"));
		return FALSE;
	}
	OutputCDOffset(pExtArg, pDisc, bGetDrive, nDriveSampleOffset, nDriveOffset);
	return TRUE;
}

BOOL ReadCDForSearchingOffset(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	)
{
	BOOL bRet = TRUE;
	INT nDriveSampleOffset = 0;
	BOOL bGetDrive = GetDriveOffset(pDevice->szProductId, &nDriveSampleOffset);
	if (!bGetDrive) {
		_TCHAR buf[6] = { 0 };
		OutputString(
			_T("This drive doesn't define in driveOffset.txt\n")
			_T("Please input Drive offset(Samples): "));
		INT b = _tscanf(_T("%6[^\n]%*[^\n]"), buf);
		b = _gettchar();
		nDriveSampleOffset = _ttoi(buf);
	}

	INT nDriveOffset = nDriveSampleOffset * 4;
	if (pDisc->SCSI.byAudioOnly) {
		pDisc->MAIN.nCombinedOffset = nDriveOffset;
		OutputCDOffset(pExtArg, pDisc, bGetDrive, nDriveSampleOffset, nDriveOffset);
	}
	else {
		LPBYTE pBuf = NULL;
		LPBYTE lpBuf = NULL;
		if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
			CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, &lpBuf, _T(__FUNCTION__), __LINE__)) {
			return FALSE;
		}
		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		if (pExtArg->byD8) {
			CDB::_PLXTR_READ_CDDA cdb = { 0 };
			SetReadD8Command(pDevice, &cdb, 1, PLXTR_READ_CDDA_FLAG::NoSub);
			memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
		}
		else {
			CDB::_READ_CD cdb = { 0 };
			SetReadCDCommand(NULL, pDevice, &cdb,
				READ_CD_FLAG::CDDA, 1, READ_CD_FLAG::NoSub, TRUE);
			memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
		}
		if (*pExecType == gd) {
			pDisc->SCSI.nFirstLBAofDataTrack = FIRST_LBA_FOR_GD;
		}
		if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd,
			lpBuf, CD_RAW_SECTOR_SIZE, bGetDrive, nDriveSampleOffset, nDriveOffset)) {
			bRet = FALSE;
		}
		if (pExtArg->byD8) {
			lpCmd[10] = (BYTE)PLXTR_READ_CDDA_FLAG::MainQ;
			if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd,
				lpBuf, CD_RAW_SECTOR_SIZE + 16, bGetDrive, nDriveSampleOffset, nDriveOffset)) {
			}
			lpCmd[10] = (BYTE)PLXTR_READ_CDDA_FLAG::MainPack;
			if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd,
				lpBuf, CD_RAW_SECTOR_WITH_SUBCODE_SIZE, bGetDrive, nDriveSampleOffset, nDriveOffset)) {
				bRet = FALSE;
			}
			lpCmd[10] = (BYTE)PLXTR_READ_CDDA_FLAG::Raw;
			if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd,
				lpBuf, CD_RAW_READ_SUBCODE_SIZE, bGetDrive, nDriveSampleOffset, nDriveOffset)) {
			}
			lpCmd[10] = (BYTE)PLXTR_READ_CDDA_FLAG::MainC2Raw;
			if (!ExecSearchingOffset(pExecType, pExtArg, pDevice, pDisc, lpCmd,
				lpBuf, CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, bGetDrive, nDriveSampleOffset, nDriveOffset)) {
				pExtArg->byC2 = FALSE;
				pDevice->FEATURE.byC2ErrorData = FALSE;
				// not return FALSE
			}
		}
		FreeAndNull(pBuf);
	}
	return bRet;
}

BOOL ReadCDForCheckingAdrFirst(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE* aBuf2,
	LPBYTE* lpBuf,
	LPBYTE lpCmd,
	LPINT nOfs
	)
{
	if (!GetAlignedCallocatedBuffer(pDevice, aBuf2,
		CD_RAW_SECTOR_WITH_SUBCODE_SIZE, lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	if (pExtArg->byD8) {
		CDB::_PLXTR_READ_CDDA cdb = { 0 };
		SetReadD8Command(pDevice, &cdb, 1, PLXTR_READ_CDDA_FLAG::MainPack);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
		*nOfs = pDisc->MAIN.nCombinedOffset % CD_RAW_SECTOR_SIZE;
		if (pDisc->MAIN.nCombinedOffset < 0) {
			*nOfs = CD_RAW_SECTOR_SIZE + *nOfs;
		}
	}
	else {
		CDB::_READ_CD cdb = { 0 };
		SetReadCDCommand(NULL, pDevice, &cdb,
			READ_CD_FLAG::All, 1, READ_CD_FLAG::Raw, TRUE);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
	}

	OutputDiscLogA(
		"=============================== Check MCN, ISRC ===============================\n");
	return TRUE;
}

BOOL ReadCDForCheckingAdr(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE lpCmd,
	LPBYTE lpBuf,
	INT aLBA[],
	INT nOfs,
	LPBYTE byMode,
	BYTE bySessionIdx,
	FILE* fpCcd
	)
{
	// check MCN, ISRC frame 2 times
	BOOL bCheckMCN = FALSE;
	BOOL bCheckISRC = FALSE;
	INT nMCNIdx = 0;
	INT nISRCIdx = 0;
	INT nTmpMCNLBAList[8] = { -1 };
	INT nTmpISRCLBAList[8] = { -1 };
	CHAR szTmpCatalog[META_CATALOG_SIZE] = { 0 };
	BYTE byScsiStatus = 0;
	for (INT nLBA = aLBA[bySessionIdx]; nLBA < aLBA[bySessionIdx] + 400; nLBA++) {
		lpCmd[2] = HIBYTE(HIWORD(nLBA));
		lpCmd[3] = LOBYTE(HIWORD(nLBA));
		lpCmd[4] = HIBYTE(LOWORD(nLBA));
		lpCmd[5] = LOBYTE(LOWORD(nLBA));
		if (!ScsiPassThroughDirect(pExtArg, pDevice, lpCmd, CDB12GENERIC_LENGTH, lpBuf,
			CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			return FALSE;
		}
		LPBYTE lpBuf2 = lpBuf;
		if (pExtArg->byD8) {
			lpBuf2 = lpBuf + nOfs;
		}
		BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
#if 0
		OutputCDMain(lpBuf2, nLBA, CD_RAW_SECTOR_SIZE);
		OutputCDSub96Align(lpSubcode, nLBA);
#endif
		if (nLBA == aLBA[bySessionIdx]) {
			BYTE byCtl = (BYTE)((lpSubcode[12] >> 4) & 0x0f);
			*byMode = GetMode(lpBuf2, 0, byCtl, UNSCRAMBLED);
		}
		BYTE byAdr = (BYTE)(lpSubcode[12] & 0x0f);
		if (byAdr == ADR_ENCODES_MEDIA_CATALOG) {
			BOOL bMCN = IsValidSubQMCN(lpSubcode);
			if (bMCN) {
				nTmpMCNLBAList[nMCNIdx++] = nLBA;
				CHAR szCatalog[META_CATALOG_SIZE] = { 0 };
				if (!bCheckMCN) {
					SetMCNToString(pDisc, lpSubcode, szCatalog, FALSE);
					strncpy(szTmpCatalog, szCatalog, META_CATALOG_SIZE);
					szTmpCatalog[META_CATALOG_SIZE - 1] = 0;
					bCheckMCN = bMCN;
				}
				else if (!pDisc->SUB.byCatalog) {
					SetMCNToString(pDisc, lpSubcode, szCatalog, FALSE);
					if (!strncmp(szTmpCatalog, szCatalog, META_CATALOG_SIZE)) {
						strncpy(pDisc->SUB.szCatalog, szCatalog, META_CATALOG_SIZE);
						WriteCcdFileForDiscCatalog(pDisc, fpCcd);
						pDisc->SUB.byCatalog = (BYTE)bMCN;
					}
				}
			}
		}
		else if (byAdr == ADR_ENCODES_ISRC) {
			BOOL bISRC = IsValidSubQISRC(lpSubcode);
			if (bISRC) {
				bCheckISRC = bISRC;
				pDisc->SUB.byISRC = (BYTE)bISRC;
				nTmpISRCLBAList[nISRCIdx++] = nLBA;
			}
		}
	}
	if (bCheckMCN) {
		SetFirstAdrSector(pDisc->SUB.nFirstLBAForMCN, pDisc->SUB.nRangeLBAForMCN,
			"MCN", nTmpMCNLBAList, bySessionIdx, pDevice->byPlxtrType);
	}
	if (bCheckISRC) {
		SetFirstAdrSector(pDisc->SUB.nFirstLBAForISRC, pDisc->SUB.nRangeLBAForISRC,
			"ISRC", nTmpISRCLBAList, bySessionIdx, pDevice->byPlxtrType);
	}
	return TRUE;
}

BOOL ReadCDForCheckingReadInOut(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PMAIN_HEADER pMain,
	LPCTSTR pszPath,
	READ_CD_FLAG::EXPECTED_SECTOR_TYPE flg
	)
{
	BOOL bRet = TRUE;
	if (pDisc->MAIN.nCombinedOffset < 0) {
		OutputString(_T("Checking reading lead-in\n"));
		bRet = ReadCDPartial(pExecType, pExtArg,
			pDevice, pDisc, pMain, pszPath, 0, 0, flg, TRUE);
		if (bRet) {
			OutputLogA(standardOut | fileDrive, "Reading lead-in: OK\n");
		}
		else {
			OutputLogA(standardOut | fileDrive, "Reading lead-in: NG\n");
			bRet = FALSE;
		}
	}
	else if (0 < pDisc->MAIN.nCombinedOffset) {
		OutputString(_T("Checking reading lead-out\n"));
		bRet = ReadCDPartial(pExecType, pExtArg, pDevice, pDisc, pMain,
			pszPath, pDisc->SCSI.nAllLength + 1, pDisc->SCSI.nAllLength, flg, TRUE);
		if (bRet) {
			OutputLogA(standardOut | fileDrive, "Reading lead-out: OK\n");
		}
		else {
			OutputLogA(standardOut | fileDrive, "Reading lead-out: NG\n");
			bRet = FALSE;
		}
	}
	return bRet;
}
#if 0
BOOL ReadCDForCheckingCommand(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	)
{
	BOOL bRet = TRUE;
	LPBYTE pBuf = NULL;
	LPBYTE lpBuf = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
		CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
	try {
#if 0
		CDB::_READ_CD cdb = { 0 };
		SetReadCDCommand(NULL, pDevice, &cdb, READ_CD_FLAG::All, 1, READ_CD_FLAG::Raw, TRUE);
		if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, 0, lpBuf,
			CD_RAW_SECTOR_WITH_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)) {
			throw FALSE;
		}
		AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
		OutputDiscLogA(
			"============ Check SubP to W, OperationCode: 0xbe, Subcode: 1(=Raw) ===========\n");
		OutputCDSub96Align(lpSubcode, 0);

		SetReadCDCommand(NULL, pDevice, &cdb, READ_CD_FLAG::All, 1, READ_CD_FLAG::Pack, TRUE);
		if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, 0, lpBuf,
			CD_RAW_SECTOR_WITH_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)) {
			throw FALSE;
		}
		AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
		OutputDiscLogA(
			"=========== Check SubP to W, OperationCode: 0xbe, Subcode: 4(=Pack) ===========\n");
		OutputCDSub96Align(lpSubcode, 0);
#endif
		if (pDevice->byPlxtrType) {
			CDB::_PLXTR_READ_CDDA cdb2 = { 0 };
			SetReadD8Command(pDevice, &cdb2, 1, PLXTR_READ_CDDA_FLAG::MainPack);
			if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb2, pDisc->SCSI.nFirstLBAofDataTrack, lpBuf,
				CD_RAW_SECTOR_WITH_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
			OutputDiscLogA(
				"============ Check Main, OperationCode: 0xd8, Subcode: 0x02(=Pack) ============\n");
			OutputCDMain(lpBuf, pDisc->SCSI.nFirstLBAofDataTrack, CD_RAW_SECTOR_SIZE);
			OutputDiscLogA(
				"========== Check SubP to W, OperationCode: 0xd8, Subcode: 0x02(=Pack) =========\n");
			OutputCDSub96Align(lpSubcode, pDisc->SCSI.nFirstLBAofDataTrack);

			cdb2.SubCode = PLXTR_READ_CDDA_FLAG::Raw;
			if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb2, pDisc->SCSI.nFirstLBAofDataTrack, lpBuf,
				CD_RAW_READ_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(lpBuf, lpSubcode);
			OutputDiscLogA(
				"========== Check SubP to W, OperationCode: 0xd8, Subcode: 0x03(=Pack) =========\n");
			OutputCDSub96Align(lpSubcode, pDisc->SCSI.nFirstLBAofDataTrack);

			if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
				cdb2.SubCode = PLXTR_READ_CDDA_FLAG::MainC2Raw;
				if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb2, pDisc->SCSI.nFirstLBAofDataTrack, lpBuf,
					CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
				AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE + CD_RAW_READ_C2_294_SIZE, lpSubcode);
				OutputDiscLogA(
					"============ Check Main, OperationCode: 0xd8, Subcode: 0x08(=Raw) ============\n");
				OutputCDMain(lpBuf, pDisc->SCSI.nFirstLBAofDataTrack, CD_RAW_SECTOR_SIZE);
				OutputDiscLogA(
					"========== Check SubP to W, OperationCode: 0xd8, Subcode: 0x08(=Raw) ==========\n");
				OutputCDSub96Align(lpSubcode, pDisc->SCSI.nFirstLBAofDataTrack);
			}
		}
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(pBuf);
	return bRet;
}
#endif
BOOL ReadCDForCheckingCDG(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	)
{
	BOOL bRet = TRUE;
	LPBYTE pBuf = NULL;
	LPBYTE lpBuf = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
		CD_RAW_SECTOR_WITH_SUBCODE_SIZE, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
	if (pExtArg->byD8) {
		CDB::_PLXTR_READ_CDDA cdb = { 0 };
		SetReadD8Command(pDevice, &cdb, 1, PLXTR_READ_CDDA_FLAG::MainPack);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
	}
	else {
		CDB::_READ_CD cdb = { 0 };
		SetReadCDCommand(NULL, pDevice, &cdb, READ_CD_FLAG::All, 1, READ_CD_FLAG::Raw, TRUE);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
	}

	for (BYTE i = 0; i < pDisc->SCSI.toc.LastTrack; i++) {
		if ((pDisc->SCSI.toc.TrackData[i].Control & AUDIO_DATA_TRACK) == 0) {
			try {
				INT nTmpLBA = pDisc->SCSI.lpFirstLBAListOnToc[i] + 100;
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, nTmpLBA, lpBuf,
					CD_RAW_SECTOR_WITH_SUBCODE_SIZE, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
				BYTE lpSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
				BYTE lpSubcodeOrg[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
				AlignRowSubcode(lpBuf + CD_RAW_SECTOR_SIZE, lpSubcode);
				memcpy(lpSubcodeOrg, lpBuf + CD_RAW_SECTOR_SIZE, CD_RAW_READ_SUBCODE_SIZE);
				OutputDiscLogA(
					"========================== Check SubRtoW for track[%02d] ========================\n"
					, i + 1);
				OutputCDSub96Align(lpSubcode, nTmpLBA);

				SUB_R_TO_W scRW[4] = { 0 };
				BYTE tmpCode[24] = { 0 };
				INT nRtoW = 0;
				BOOL bCDG = FALSE;
				BOOL bCDEG = FALSE;
				for (INT k = 0; k < 4; k++) {
					for (INT j = 0; j < 24; j++) {
						tmpCode[j] = (BYTE)(*(lpSubcodeOrg + (k * 24 + j)) & 0x3f);
					}
					memcpy(&scRW[k], tmpCode, sizeof(scRW[k]));
					switch (scRW[k].command) {
					case 0: // MODE 0, ITEM 0
						break;
					case 8: // MODE 1, ITEM 0
						break;
					case 9: // MODE 1, ITEM 1
						bCDG = TRUE;
						break;
					case 10: // MODE 1, ITEM 2
						bCDEG = TRUE;
						break;
					case 20: // MODE 2, ITEM 4
						break;
					case 24: // MODE 3, ITEM 0
						break;
					case 56: // MODE 7, ITEM 0
						break;
					default:
						break;
					}
				}
				INT nR = 0;
				INT nS = 0;
				INT nT = 0;
				INT nU = 0;
				INT nV = 0;
				INT nW = 0;
				for (INT j = 24; j < CD_RAW_READ_SUBCODE_SIZE; j++) {
					if (24 <= j && j < 36) {
						nR += lpSubcode[j];
					}
					else if (36 <= j && j < 48) {
						nS += lpSubcode[j];
					}
					else if (48 <= j && j < 60) {
						nT += lpSubcode[j];
					}
					else if (60 <= j && j < 72) {
						nU += lpSubcode[j];
					}
					else if (72 <= j && j < 84) {
						nV += lpSubcode[j];
					}
					else if (84 <= j && j < CD_RAW_READ_SUBCODE_SIZE) {
						nW += lpSubcode[j];
					}
					nRtoW += lpSubcode[j];
				}
				// 0xff * 72 = 0x47b8
				if (nRtoW == 0x47b8) {
					// Why R-W bit is full? Basically, a R-W bit should be off except CD+G or CD-MIDI
					//  Alanis Morissette - Jagged Little Pill (UK)
					//  WipEout 2097: The Soundtrack
					//  and more..
					// Sub Channel LBA 75
					// 	  +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B
					// 	P 00 00 00 00 00 00 00 00 00 00 00 00
					// 	Q 01 01 01 00 01 00 00 00 03 00 2c b9
					// 	R ff ff ff ff ff ff ff ff ff ff ff ff
					// 	S ff ff ff ff ff ff ff ff ff ff ff ff
					// 	T ff ff ff ff ff ff ff ff ff ff ff ff
					// 	U ff ff ff ff ff ff ff ff ff ff ff ff
					// 	V ff ff ff ff ff ff ff ff ff ff ff ff
					// 	W ff ff ff ff ff ff ff ff ff ff ff ff
					pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::Full;
					OutputDiscLogA("Track[%02u]: RtoW is 0xff\n", i + 1);
				}
				else {
					BOOL bFull = FALSE;
					// 0xff * 12 = 0xbf4
					if (nR == 0xbf4) {
						pDisc->SUB.lpRtoWList[i] |= SUB_RTOW_TYPE::RFull;
						OutputDiscLogA("Track[%02u]: All R is 0xff\n", i + 1);
						bFull = TRUE;
					}
					if (nS == 0xbf4) {
						pDisc->SUB.lpRtoWList[i] |= SUB_RTOW_TYPE::SFull;
						OutputDiscLogA("Track[%02u]: All S is 0xff\n", i + 1);
						bFull = TRUE;
					}
					if (nT == 0xbf4) {
						pDisc->SUB.lpRtoWList[i] |= SUB_RTOW_TYPE::TFull;
						OutputDiscLogA("Track[%02u]: All T is 0xff\n", i + 1);
						bFull = TRUE;
					}
					if (nU == 0xbf4) {
						pDisc->SUB.lpRtoWList[i] |= SUB_RTOW_TYPE::UFull;
						OutputDiscLogA("Track[%02u]: All U is 0xff\n", i + 1);
						bFull = TRUE;
					}
					if (nV == 0xbf4) {
						pDisc->SUB.lpRtoWList[i] |= SUB_RTOW_TYPE::VFull;
						OutputDiscLogA("Track[%02u]: All V is 0xff\n", i + 1);
						bFull = TRUE;
					}
					if (nW == 0xbf4) {
						pDisc->SUB.lpRtoWList[i] |= SUB_RTOW_TYPE::WFull;
						OutputDiscLogA("Track[%02u]: All W is 0xff\n", i + 1);
						bFull = TRUE;
					}
					if (!bFull) {
						if (bCDG && nRtoW > 0 && nRtoW != 0x200) {
							pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::CDG;
							OutputDiscLogA("Track[%02u]: CD+G\n", i + 1);
						}
						else if (bCDEG && nRtoW > 0 && nRtoW != 0x200) {
							pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::CDG;
							OutputDiscLogA("Track[%02u]: CD+EG\n", i + 1);
						}
						else {
							pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::Zero;
							OutputDiscLogA("Track[%02u]: Nothing\n", i + 1);
						}
					}
				}
			}
			catch (BOOL bErr) {
				bRet = bErr;
			}
		}
		else {
			pDisc->SUB.lpRtoWList[i] = SUB_RTOW_TYPE::Zero;
		}
	}
	FreeAndNull(pBuf);
	return bRet;
}

BOOL ReadCDForCheckingExe(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	CDB::_READ12* pCdb
	)
{
	BYTE pBuf[DISC_RAW_READ_SIZE] = { 0 };
	for (INT n = 0; pDisc->PROTECT.nExtentPosForExe[n] != 0; n++) {
		if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)pCdb, pDisc->PROTECT.nExtentPosForExe[n],
			pBuf, DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
			return FALSE;
		}
		OutputVolDescLogA(
			"========================= %s, LBA[%06d, %#07x] =========================\n"
			, pDisc->PROTECT.nameForExe[n], pDisc->PROTECT.nExtentPosForExe[n], pDisc->PROTECT.nExtentPosForExe[n]);
		PIMAGE_DOS_HEADER pIDh = (PIMAGE_DOS_HEADER)&pBuf[0];
		OutputFsImageDosHeader(pIDh);
		OutputFsImageNtHeader((PIMAGE_NT_HEADERS32)&pBuf[pIDh->e_lfanew]);
		ULONG nOfs = pIDh->e_lfanew + sizeof(IMAGE_NT_HEADERS32);
		while (pBuf[nOfs] != 0) {
			OutputFsImageSectionHeader(pDisc, (PIMAGE_SECTION_HEADER)&pBuf[nOfs], n);
			nOfs += sizeof(IMAGE_SECTION_HEADER);
		}
	}
	return TRUE;
}

#define MAX_FNAME_FOR_VOLUME (64)

BOOL ReadCDForDirectoryRecord(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	CDB::_READ12* pCdb,
	LPBYTE lpBuf,
	BYTE byTransferLen,
	INT nDirPosNum,
	INT nDirTblPos,
	LPUINT pDirTblSizeList,
	LPSTR* pDirTblNameList
	)
{
	if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)pCdb, nDirTblPos, lpBuf,
		(DWORD)(DISC_RAW_READ_SIZE * byTransferLen), _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
#if 1
	OutputLogA(fileMainInfo, "byTransferLen: %d\n", byTransferLen);
	for (BYTE i = 0; i < byTransferLen; i++) {
		OutputCDMain(fileMainInfo, lpBuf + DISC_RAW_READ_SIZE * i, nDirTblPos + i, DISC_RAW_READ_SIZE);
	}
#endif
	UINT nOfs = 0;
	for (INT nSectorNum = 0; nSectorNum < byTransferLen;) {
		if (*(lpBuf + nOfs) == 0) {
			break;
		}
		OutputVolDescLogA(
			"==================== Directory Record, LBA[%06d, %#07x] ===================\n"
			, nDirTblPos + nSectorNum, nDirTblPos + nSectorNum);
		for (;;) {
			CHAR szCurDirName[MAX_FNAME_FOR_VOLUME] = { 0 };
			LPBYTE lpDirRec = lpBuf + nOfs;
			if (lpDirRec[0] >= 0x22) {
				DWORD dwDataLen = MAKEDWORD(MAKEWORD(lpDirRec[10], lpDirRec[11]),
					MAKEWORD(lpDirRec[12], lpDirRec[13]));
				if (dwDataLen == 0) {
					dwDataLen = MAKEDWORD(MAKEWORD(lpDirRec[17], lpDirRec[16]),
						MAKEWORD(lpDirRec[15], lpDirRec[14]));
				}
				OutputFsDirectoryRecord(pExtArg, pDisc, lpDirRec, dwDataLen, szCurDirName);
				OutputVolDescLogA("\n");
				nOfs += lpDirRec[0];

				for (INT b = 1; b < nDirPosNum; b++) {
					if (pDirTblSizeList[b] == 0 && 
//						!strncmp(szCurDirName, pDirTblNameList[b], MAX_FNAME_FOR_VOLUME)) {
						!_strnicmp(szCurDirName, pDirTblNameList[b], MAX_FNAME_FOR_VOLUME)) {
						INT nPadding = DISC_RAW_READ_SIZE - (INT)dwDataLen;
						if (nPadding > 0) {
							OutputMainInfoLogA(
								"pDirTblSizeList[%d]: DataLength is %d, added %d size for padding\n"
								, b, dwDataLen, nPadding);
							pDirTblSizeList[b] = dwDataLen + nPadding;
						}
						else {
							nPadding = (INT)dwDataLen % DISC_RAW_READ_SIZE;
							if (nPadding != 0) {
								nPadding = DISC_RAW_READ_SIZE - nPadding;
								OutputMainInfoLogA(
									"pDirTblSizeList[%d]: DataLength is %d, added %d size for padding\n"
									, b, dwDataLen, nPadding);
								pDirTblSizeList[b] = dwDataLen + nPadding;
							}
							else {
								pDirTblSizeList[b] = dwDataLen;
							}
						}
						break;
					}
				}
				if (nOfs == (UINT)(DISC_RAW_READ_SIZE * (nSectorNum + 1))) {
					nSectorNum++;
					break;
				}
			}
			else {
				UINT zeroPaddingNum = DISC_RAW_READ_SIZE * (nSectorNum + 1) - nOfs;
				if (nSectorNum < byTransferLen) {
					UINT j = 0;
					for (; j < zeroPaddingNum; j++) {
						if (lpDirRec[j] != 0) {
							break;
						}
					}
					if (j == zeroPaddingNum) {
						nOfs += zeroPaddingNum;
						nSectorNum++;
						break;
					}
				}
				else {
					break;
				}
			}
		}
	}
	return TRUE;
}

BOOL ReadCDForVolumeDescriptor(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	BYTE byIdx,
	CDB::_READ12* pCdb,
	LPBYTE* ppBuf,
	LPBYTE lpBuf,
	LPBOOL pPVD
	)
{
	BOOL bRet = TRUE;
	INT nPVD = pDisc->SCSI.lpFirstLBAListOnToc[byIdx] + 16;
	INT nTmpLBA = nPVD;
	DWORD dwPathTblSize = 0;
	DWORD dwPathTblPos = 0;
	DWORD dwRootDataLen = 0;
	for (;;) {
		if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)pCdb, nTmpLBA, lpBuf,
			DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
			return FALSE;
		}
		if (!strncmp((PCHAR)&lpBuf[1], "CD001", 5) ||
			(pDisc->SCSI.byCdi && !strncmp((PCHAR)&lpBuf[1], "CD-I ", 5))) {
			if (nTmpLBA == nPVD) {
				dwPathTblSize = MAKEDWORD(MAKEWORD(lpBuf[132], lpBuf[133]),
					MAKEWORD(lpBuf[134], lpBuf[135]));
				if (dwPathTblSize == 0) {
					dwPathTblSize = MAKEDWORD(MAKEWORD(lpBuf[139], lpBuf[138]),
						MAKEWORD(lpBuf[137], lpBuf[136]));
				}
				dwPathTblPos = MAKEDWORD(MAKEWORD(lpBuf[140], lpBuf[141]),
					MAKEWORD(lpBuf[142], lpBuf[143]));
				if (dwPathTblPos == 0) {
					dwPathTblPos = MAKEDWORD(MAKEWORD(lpBuf[151], lpBuf[150]),
						MAKEWORD(lpBuf[149], lpBuf[148]));
				}
				dwRootDataLen = MAKEDWORD(MAKEWORD(lpBuf[166], lpBuf[167]),
					MAKEWORD(lpBuf[168], lpBuf[169]));
				if (dwRootDataLen == 0) {
					dwRootDataLen = MAKEDWORD(MAKEWORD(lpBuf[173], lpBuf[172]),
						MAKEWORD(lpBuf[171], lpBuf[170]));
				}
				if (dwRootDataLen > 0) {
					INT nPadding = DISC_RAW_READ_SIZE - (INT)dwRootDataLen;
					if (nPadding != 0) {
						if (nPadding > 0){
							// Generally, rootDataLen is per 2048 byte
							// Exception: Commandos - Behind Enemy Lines (Europe)
							OutputMainInfoLogA(
								"dwRootDataLen: DataLength is %u, added %d size for padding\n"
								, dwRootDataLen, nPadding);
							dwRootDataLen += nPadding;
						}
						else {
							nPadding = (INT)dwRootDataLen % DISC_RAW_READ_SIZE;
							if (nPadding != 0) {
								nPadding = DISC_RAW_READ_SIZE - nPadding;
								OutputMainInfoLogA(
									"dwRootDataLen: DataLength is %u, added %d size for padding\n"
									, dwRootDataLen, nPadding);
								dwRootDataLen += nPadding;
							}
						}
					}
				}
				*pPVD = TRUE;
			}
			OutputFsVolumeDescriptor(pExtArg, pDisc, lpBuf, nTmpLBA++);
		}
		else {
			break;
		}
	}
	if (*pPVD) {
		BYTE byTransferLen = 1;
		if (dwPathTblSize > DISC_RAW_READ_SIZE) {
			SetTransferLength(pCdb, dwPathTblSize, &byTransferLen);
			GetAlignedReallocatedBuffer(pDevice, ppBuf,
				dwPathTblSize, byTransferLen, &lpBuf, _T(__FUNCTION__), __LINE__);
		}
		if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)pCdb, (INT)dwPathTblPos, lpBuf,
			(DWORD)(DISC_RAW_READ_SIZE * byTransferLen), _T(__FUNCTION__), __LINE__)) {
			return FALSE;
		}
		// TODO: buf size
		CONST UINT nMaxDirNum = 4096;
		LPUINT pDirTblPosList = (LPUINT)calloc(nMaxDirNum, sizeof(UINT));
		if (!pDirTblPosList) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		LPUINT pDirTblSizeList = NULL;
		LPSTR* pDirTblNameList = NULL;
		try {
			pDirTblSizeList = (LPUINT)calloc(nMaxDirNum, sizeof(UINT));
			if (!pDirTblSizeList) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			pDirTblNameList = (LPSTR*)calloc(nMaxDirNum, sizeof(UINT_PTR));
			if (!pDirTblNameList) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			for (UINT p = 0; p < nMaxDirNum; p++) {
				pDirTblNameList[p] = (LPSTR)calloc(MAX_FNAME_FOR_VOLUME, sizeof(CHAR));
				if (!(pDirTblNameList[p])) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					throw FALSE;
				}
			}
			INT nDirPosNum = 0;
			OutputFsPathTableRecord(pDisc, lpBuf, dwPathTblPos,
				dwPathTblSize, pDirTblPosList, pDirTblNameList, &nDirPosNum);
			OutputVolDescLogA("Dir Num: %u\n", nDirPosNum);

			// for CD-I
			if (dwRootDataLen == 0) {
				if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)pCdb, (INT)pDirTblPosList[0], lpBuf,
					(DWORD)(DISC_RAW_READ_SIZE * byTransferLen), _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
				dwRootDataLen = MAKEDWORD(MAKEWORD(lpBuf[10], lpBuf[11]),
					MAKEWORD(lpBuf[12], lpBuf[13]));
				if (dwRootDataLen == 0) {
					dwRootDataLen = MAKEDWORD(MAKEWORD(lpBuf[17], lpBuf[16]),
						MAKEWORD(lpBuf[15], lpBuf[14]));
				}
				if (DISC_RAW_READ_SIZE > dwRootDataLen) {
					dwRootDataLen += DISC_RAW_READ_SIZE - dwRootDataLen;
				}
				else {
					dwRootDataLen += DISC_RAW_READ_SIZE - (dwRootDataLen % DISC_RAW_READ_SIZE);
				}
			}

			if (dwRootDataLen > 0 && dwRootDataLen != (DWORD)(DISC_RAW_READ_SIZE * byTransferLen)) {
				SetTransferLength(pCdb, dwRootDataLen, &byTransferLen);
				GetAlignedReallocatedBuffer(pDevice, ppBuf,
					dwRootDataLen, byTransferLen, &lpBuf, _T(__FUNCTION__), __LINE__);
			}
			pDirTblSizeList[0] = dwRootDataLen;

			INT i = 0;
			BYTE byMaxTransferLen = 0;
			DWORD dwAdditionalTransferLen = 0;
			DWORD dwLastTblSize = 0;
			DWORD dwMostLargeSize = dwRootDataLen;
			do {
				if (!ReadCDForDirectoryRecord(pExtArg, pDevice, pDisc, pCdb, lpBuf, byTransferLen,
					nDirPosNum, (INT)pDirTblPosList[i], pDirTblSizeList, pDirTblNameList)) {
					throw FALSE;
				}
				for (DWORD u = 1; u <= dwAdditionalTransferLen; u++) {
					if (u == dwAdditionalTransferLen) {
						SetTransferLength(pCdb, dwLastTblSize, &byTransferLen);
					}
					else {
						byTransferLen = byMaxTransferLen;
						pCdb->TransferLength[3] = byTransferLen;
					}
					if (!ReadCDForDirectoryRecord(pExtArg, pDevice, pDisc, pCdb, lpBuf, byTransferLen,
						nDirPosNum, (INT)pDirTblPosList[i], pDirTblSizeList, pDirTblNameList)) {
						throw FALSE;
					}
				}
				dwAdditionalTransferLen = 0;
				i++;
				if (pDirTblSizeList[i] > dwMostLargeSize) {
					dwMostLargeSize = pDirTblSizeList[i];
					GetAlignedReallocatedBuffer(pDevice, ppBuf, 
						pDirTblSizeList[i], byTransferLen, &lpBuf, _T(__FUNCTION__), __LINE__);
				}
				if (pDirTblSizeList[i] > pDevice->uiMaxTransferLength) {
					SetTransferLength(pCdb, pDevice->uiMaxTransferLength, &byMaxTransferLen);
					dwAdditionalTransferLen = pDirTblSizeList[i] / pDevice->uiMaxTransferLength;
					dwLastTblSize = pDirTblSizeList[i] % pDevice->uiMaxTransferLength;
				}
				else {
					if (pDirTblSizeList[i] != 0) {
						SetTransferLength(pCdb, pDirTblSizeList[i], &byTransferLen);
					}
				}
				OutputString(_T("\rReading DirectoryRecord %4d/%4d"), i, nDirPosNum);
			} while (i < nDirPosNum);
			OutputString(_T("\n"));

			pCdb->TransferLength[3] = 1;
			if (!ReadCDForCheckingExe(pExtArg, pDevice, pDisc, pCdb)) {
				throw FALSE;
			}
			if (pDisc->PROTECT.byExist) {
				OutputLogA(standardOut | fileDisc, "Detected [%s], Skip error from %d to %d\n"
					, pDisc->PROTECT.name, pDisc->PROTECT.ERROR_SECTOR.nExtentPos,
					pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize);
			}
		}
		catch (BOOL bErr) {
			bRet = bErr;
		}
		for (INT p = 0; p < nMaxDirNum; p++) {
			FreeAndNull(pDirTblNameList[p]);
		}
		FreeAndNull(pDirTblPosList);
		FreeAndNull(pDirTblNameList);
		FreeAndNull(pDirTblSizeList);
	}
	return bRet;
}

BOOL ReadCDFor3DODirectory(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	CDB::_READ12* pCdb,
	PCHAR pPath,
	INT nLBA
	)
{
	BOOL bRet = TRUE;
	LPBYTE pBuf = NULL;
	LPBYTE lpBuf = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
		DISC_RAW_READ_SIZE, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	try {
		if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)pCdb, nLBA, lpBuf,
			DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
			throw FALSE;
		}
		LONG lOfs = THREEDO_DIR_HEADER_SIZE;
		LONG lDirSize =
			MAKELONG(MAKEWORD(lpBuf[15], lpBuf[14]), MAKEWORD(lpBuf[13], lpBuf[12]));
		OutputFs3doDirectoryRecord(lpBuf, nLBA, pPath, lDirSize);

		// next dir
		CHAR szNewPath[_MAX_PATH] = { 0 };
		CHAR fname[32] = { 0 };
		while (lOfs < lDirSize) {
			LPBYTE lpDirEnt = lpBuf + lOfs;
			LONG lFlags = MAKELONG(
				MAKEWORD(lpDirEnt[3], lpDirEnt[2]), MAKEWORD(lpDirEnt[1], lpDirEnt[0]));
			strncpy(fname, (PCHAR)&lpDirEnt[32], sizeof(fname));
			LONG lastCopy = MAKELONG(
				MAKEWORD(lpDirEnt[67], lpDirEnt[66]), MAKEWORD(lpDirEnt[65], lpDirEnt[64]));
			lOfs += THREEDO_DIR_ENTRY_SIZE;

			if ((lFlags & 0xff) == 7) {
				sprintf(szNewPath, "%s%s/", pPath, fname);
				if (!ReadCDFor3DODirectory(pExtArg, pDevice, pDisc, pCdb, szNewPath,
					MAKELONG(MAKEWORD(lpDirEnt[71], lpDirEnt[70]), MAKEWORD(lpDirEnt[69], lpDirEnt[68])))) {
					throw FALSE;
				}
			}
			for (LONG i = 0; i < lastCopy; i++) {
				lOfs += sizeof(LONG);
			}
		}
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(pBuf);
	return bRet;
}

BOOL ReadCDForFileSystem(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	)
{
	BOOL bRet = TRUE;
	for (BYTE i = 0; i < pDisc->SCSI.toc.LastTrack; i++) {
		if ((pDisc->SCSI.toc.TrackData[i].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			// for Label Gate CD, XCP
			if (i > 1 && pDisc->SCSI.lpLastLBAListOnToc[i] - pDisc->SCSI.lpFirstLBAListOnToc[i] + 1 <= 750) {
				return TRUE;
			}
			LPBYTE pBuf = NULL;
			LPBYTE lpBuf = NULL;
			if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
				DISC_RAW_READ_SIZE, &lpBuf, _T(__FUNCTION__), __LINE__)) {
				return FALSE;
			}
			CDB::_READ12 cdb = { 0 };
			cdb.OperationCode = SCSIOP_READ12;
			cdb.LogicalUnitNumber = pDevice->address.Lun;
			cdb.TransferLength[3] = 1;
			BOOL bPVD = FALSE;
			try {
				// general data track disc
				if (!ReadCDForVolumeDescriptor(pExtArg,
					pDevice, pDisc, i, &cdb, &pBuf, lpBuf, &bPVD)) {
					throw FALSE;
				}
				if (!bPVD) {
					BOOL bOtherHeader = FALSE;
					// other (pce, pc-fx)
					INT nLBA = pDisc->SCSI.nFirstLBAofDataTrack;
					if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
						DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
						throw FALSE;
					}
					if (IsValidPceSector(lpBuf)) {
						OutputFsPceStuff(lpBuf, nLBA);
						nLBA = pDisc->SCSI.nFirstLBAofDataTrack + 1;
						if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
							DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
							throw FALSE;
						}
						OutputFsPceBootSector(lpBuf, nLBA);
						bOtherHeader = TRUE;
					}
					else if (IsValidPcfxSector(lpBuf)) {
						OutputFsPcfxHeader(lpBuf, nLBA);
						nLBA = pDisc->SCSI.nFirstLBAofDataTrack + 1;
						if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
							DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
							throw FALSE;
						}
						OutputFsPcfxSector(lpBuf, nLBA);
						bOtherHeader = TRUE;
					}

					if (!bOtherHeader) {
						// for 3DO
						nLBA = 0;
						if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
							DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
							throw FALSE;
						}
						if (IsValid3doDataHeader(lpBuf)) {
							OutputFs3doHeader(lpBuf, nLBA);
							if (!ReadCDFor3DODirectory(pExtArg, pDevice, pDisc, &cdb, "/",
								MAKELONG(MAKEWORD(lpBuf[103], lpBuf[102]),
								MAKEWORD(lpBuf[101], lpBuf[100])))) {
								throw FALSE;
							}
							bOtherHeader = TRUE;
						}
					}
					if (!bOtherHeader) {
						// for MAC pattern 1
						nLBA = 1;
						if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
							DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
							throw FALSE;
						}
						if (IsValidMacDataHeader(lpBuf + 1024)) {
							OutputFsMasterDirectoryBlocks(lpBuf + 1024, nLBA);
							bOtherHeader = TRUE;
						}
						else if (IsValidMacDataHeader(lpBuf + 512)) {
							OutputFsMasterDirectoryBlocks(lpBuf + 512, nLBA);
							bOtherHeader = TRUE;
						}
						// for MAC pattern 2
						nLBA = 16;
						if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
							DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
							throw FALSE;
						}
						if (IsValidMacDataHeader(lpBuf + 1024)) {
							OutputFsMasterDirectoryBlocks(lpBuf + 1024, nLBA);
							bOtherHeader = TRUE;
						}
					}
					if (bOtherHeader) {
						FreeAndNull(pBuf);
						break;
					}
				}
			}
			catch (BOOL bErr) {
				bRet = bErr;
			}
			FreeAndNull(pBuf);
		}
	}
	return bRet;
}

BOOL ReadCDForCheckingByteOrder(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDRIVE_DATA_ORDER pOrder
	)
{
	DWORD dwBufLen =
		CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE + pDevice->TRANSFER.dwAdditionalBufLen;
	LPBYTE pBuf = NULL;
	LPBYTE lpBuf = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
		dwBufLen, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
	if (pExtArg->byD8) {
		CDB::_PLXTR_READ_CDDA cdb = { 0 };
		SetReadD8Command(pDevice, &cdb, 1, PLXTR_READ_CDDA_FLAG::MainC2Raw);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
	}
	else {
		// non plextor && support scrambled ripping
		CDB::_READ_CD cdb = { 0 };
		SetReadCDCommand(pExtArg, pDevice, &cdb, READ_CD_FLAG::CDDA, 1, READ_CD_FLAG::Raw, FALSE);
		memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
	}
	BOOL bRet = TRUE;
	if (!ExecReadCD(pExtArg, pDevice, lpCmd, 0, lpBuf, dwBufLen, _T(__FUNCTION__), __LINE__)) {
		OutputLogA(standardErr | fileDrive,
			"This drive doesn't support [command: %#02x, subch: %#02x]\n", lpCmd[0], lpCmd[10]);
		*pOrder = DRIVE_DATA_ORDER::NoC2;
		bRet = FALSE;
	}
	else {
		OutputDriveLogA(
			"============================== Check main+c2+sub ==============================\n");
		OutputCDC2Error296(fileDrive, lpBuf + CD_RAW_SECTOR_SIZE, 0);
		OutputCDSub96Raw(fileDrive, lpBuf + CD_RAW_SECTOR_WITH_C2_294_SIZE, 0);

		OutputDriveLogA(
			"============================== Check main+sub+c2 ==============================\n");
		OutputCDSub96Raw(fileDrive, lpBuf + CD_RAW_SECTOR_SIZE, 0);
		OutputCDC2Error296(fileDrive, lpBuf + CD_RAW_SECTOR_WITH_SUBCODE_SIZE, 0);

		BYTE subcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
		memcpy(subcode, lpBuf + CD_RAW_SECTOR_WITH_C2_294_SIZE, CD_RAW_READ_SUBCODE_SIZE);
		// check main + c2 + sub order
		BOOL bMainSubC2 = TRUE;
		for (INT i = 0; i < CD_RAW_READ_SUBCODE_SIZE; i++) {
			if (subcode[i]) {
				bMainSubC2 = FALSE;
				break;
			}
		}
		if (bMainSubC2) {
			*pOrder = DRIVE_DATA_ORDER::MainSubC2;
		}
	}
	FreeAndNull(pBuf);
	return bRet;
}

BOOL ReadCDForRereadingSector(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector,
	UINT uiC2ErrorLBACnt,
	LPBYTE lpCmd,
	PDISC_PER_SECTOR pDiscPerSector,
	FILE* fpImg
	)
{
	BOOL bProcessRet = RETURNED_NO_C2_ERROR_1ST;
	UINT uiCnt = 0;
	UINT uiContinueCnt = 0;
	if (uiC2ErrorLBACnt > 0 && pDevice->FEATURE.bySetCDSpeed) {
		OutputString(
			_T("\nChange reading speed: %ux\n"), pExtArg->dwRereadSpeedNum);
		SetCDSpeed(pExtArg, pDevice, pExtArg->dwRereadSpeedNum);
	}

	while (uiC2ErrorLBACnt > 0) {
		// forced fua ripping
		if (!pExtArg->byFua) {
			OutputString(_T("force unit access flag set on\n"));
			pExtArg->byFua = TRUE;
		}
		if (uiCnt == pExtArg->dwMaxRereadNum) {
			OutputString(_T("Reread reached max: %u\n"), uiCnt);
			bProcessRet = RETURNED_FALSE;
			break;
		}
		UINT uiC2ErrorLBACntBackup = uiC2ErrorLBACnt;
		uiC2ErrorLBACnt = 0;
		SetC2ErrorBackup(pC2ErrorPerSector, 
			uiC2ErrorLBACntBackup, pDevice->TRANSFER.dwAllBufLen);

		UINT i = 0;
		for (INT nLBA = pC2ErrorPerSector[0].nErrorLBANumBackup; i < uiC2ErrorLBACntBackup; i++) {
			OutputString(
				_T("\rReread times %4u, ErrSectorNum %4u/%4u"), 
				uiCnt + 1, i + 1, uiC2ErrorLBACntBackup);
			nLBA = pC2ErrorPerSector[i].nErrorLBANumBackup;
			bProcessRet = ProcessReadCD(pExecType, pExtArg, pDevice, pDisc, 
				pC2ErrorPerSector, uiC2ErrorLBACnt, lpCmd, nLBA, pDiscPerSector, FALSE);

//#define C2TEST
#if defined C2TEST
			if (nLBA == 100 && uiCnt == 1) {
				memset(pDiscPerSector->data.present, 0xff, 2352);
				bProcessRet = RETURNED_EXIST_C2_ERROR;
			}
#endif
			if (bProcessRet == RETURNED_EXIST_C2_ERROR) {
				SetC2ErrorData(pC2ErrorPerSector, nLBA, &uiC2ErrorLBACnt, FALSE);
			}
			else if (bProcessRet == RETURNED_NO_C2_ERROR_1ST) {
				if (pC2ErrorPerSector[i].byErrorFlagBackup == RETURNED_NO_C2_ERROR_1ST) {
					if (ContainsDiffByte(pC2ErrorPerSector, pDiscPerSector->data.present, i)) {
						SetNoC2ErrorExistsByteErrorData(pC2ErrorPerSector, nLBA, &uiC2ErrorLBACnt);
					}
					else {
						LONG lPos = LONG(CD_RAW_SECTOR_SIZE * nLBA - pDisc->MAIN.nCombinedOffset);
						LONG lEndPos = lPos + CD_RAW_SECTOR_SIZE - 1;
						OutputC2ErrorLogA(
							"LBA[%06d, %#07x], Reread data matched: Fixed from [%d, %#x] to [%d, %#x]\n",
							nLBA, nLBA, lPos, lPos, lEndPos, lEndPos);
						fseek(fpImg, lPos, SEEK_SET);
						// Write track to scrambled again
						WriteMainChannel(pExtArg, pDisc, pDiscPerSector->data.present, nLBA, fpImg);
					}
				}
				else {
					SetNoC2ErrorData(pC2ErrorPerSector, pDiscPerSector->data.present,
						nLBA, pDevice->TRANSFER.dwAllBufLen, &uiC2ErrorLBACnt);
				}
			}
			else if (bProcessRet == RETURNED_CONTINUE) {
				uiContinueCnt++;
			}
			else if (bProcessRet == RETURNED_FALSE) {
				break;
			}
		}
		OutputString(_T("\n"));
		uiCnt++;
	}
	if (uiCnt == 0 && uiC2ErrorLBACnt == 0) {
		OutputLogA(standardOut | fileC2Error, "No C2 errors\n");
	}
	else if (uiCnt > 0 && uiC2ErrorLBACnt == 0 && uiContinueCnt == 0) {
		OutputLogA(standardOut | fileC2Error, 
			"C2 error was fixed at all\n"
			"But please dump at least twice (if possible, using different drives)\n");
	}
	else if (uiC2ErrorLBACnt > 0 || uiContinueCnt > 0) {
		OutputLogA(standardOut | fileC2Error, 
			"There are unrecoverable errors: %d\n", uiC2ErrorLBACnt);
	}
	return bProcessRet;
}

BOOL ReadCDAll(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PMAIN_HEADER pMain,
	LPCTSTR pszPath,
	FILE* fpCcd
	)
{
	if (pDisc->SCSI.toc.FirstTrack < 1 || 99 < pDisc->SCSI.toc.FirstTrack ||
		pDisc->SCSI.toc.LastTrack < 1 || 99 < pDisc->SCSI.toc.LastTrack) {
		return FALSE;
	}
	FILE* fpImg = NULL;
	_TCHAR pszOutReverseScmFile[_MAX_PATH] = { 0 };
	_TCHAR pszOutScmFile[_MAX_PATH] = { 0 };
	if (pExtArg->byReverse) {
		if (NULL == (fpImg = CreateOrOpenFileW(pszPath, _T("_reverse"),
			pszOutReverseScmFile, NULL, NULL, _T(".scm"), _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}
	else {
		if (NULL == (fpImg = CreateOrOpenFileW(pszPath, NULL,
			pszOutScmFile, NULL, NULL, _T(".scm"), _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}
	BOOL bRet = TRUE;
	FILE* fpC2 = NULL;
	FILE* fpCue = NULL;
	FILE* fpCueForImg = NULL;
	FILE* fpParse = NULL;
	FILE* fpSub = NULL;
	LPBYTE pBuf = NULL;
	LPBYTE pNextBuf = NULL;
	LPBYTE pNextNextBuf = NULL;
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector = NULL;
#if 0
	BYTE aScrambledBuf[CD_RAW_SECTOR_SIZE] = { 0 };
#endif
	DISC_PER_SECTOR discPerSector = { 0 };
	INT nType = SCRAMBLED;
	if (pExtArg->byBe) {
		nType = UNSCRAMBLED;
	}
	memcpy(&discPerSector.mainHeader, pMain, sizeof(MAIN_HEADER));

	try {
		// init start
#if 0
		FILE* fpTbl = NULL;
		if (NULL == (fpTbl = OpenProgrammabledFile(_T("scramble.bin"), _T("rb")))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		fread(aScrambledBuf, sizeof(BYTE), sizeof(aScrambledBuf), fpTbl);
		FcloseAndNull(fpTbl);
#endif
		if (NULL == (fpCue = CreateOrOpenFileW(
			pszPath, NULL, NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpCueForImg = CreateOrOpenFileW(
			pszPath, _T("_img"), NULL, NULL, NULL, _T(".cue"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpParse = CreateOrOpenFileW(
			pszPath, _T("_sub"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		if (NULL == (fpSub = CreateOrOpenFileW(
			pszPath, NULL, NULL, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}

		SetCDTransfer(pDevice, DRIVE_DATA_ORDER::NoC2);
		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			if (NULL == (fpC2 = CreateOrOpenFileW(
				pszPath, NULL, NULL, NULL, NULL, _T(".c2"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			SetCDTransfer(pDevice, DRIVE_DATA_ORDER::MainC2Sub);
			DRIVE_DATA_ORDER dataOrder = DRIVE_DATA_ORDER::MainC2Sub;

			if (!InitC2ErrorData(pExtArg, 
				&pC2ErrorPerSector, pDevice->TRANSFER.dwAllBufLen)) {
				throw FALSE;
			}
			if (!ReadCDForCheckingByteOrder(pExtArg, pDevice, &dataOrder)) {
				TerminateC2ErrorData(pExtArg, pDevice, &pC2ErrorPerSector);
				pDevice->FEATURE.byC2ErrorData = FALSE;
				SetCDTransfer(pDevice, DRIVE_DATA_ORDER::NoC2);
			}
			if (dataOrder == DRIVE_DATA_ORDER::MainSubC2) {
				OutputDriveLogA(
					"=> Byte order of this drive is main + sub + c2\n");
				SetCDTransfer(pDevice, DRIVE_DATA_ORDER::MainSubC2);
			}
			else if (dataOrder == DRIVE_DATA_ORDER::MainC2Sub) {
				OutputDriveLogA(
					"=> Byte order of this drive is main + c2 + sub\n");
			}
		}
#ifdef _DEBUG
		OutputString(
			_T("TransferLen %u, BufLen %ubyte, AdditionalBufLen %ubyte, AllBufLen %ubyte, BufC2Offset %ubyte, BufSubOffset %ubyte\n"),
			pDevice->TRANSFER.uiTransferLen, pDevice->TRANSFER.dwBufLen,
			pDevice->TRANSFER.dwAdditionalBufLen, pDevice->TRANSFER.dwAllBufLen, 
			pDevice->TRANSFER.dwBufC2Offset, pDevice->TRANSFER.dwBufSubOffset);
#endif
		// store main + (c2) + sub data all
		if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
			pDevice->TRANSFER.dwAllBufLen, &discPerSector.data.present, _T(__FUNCTION__), __LINE__)) {
			throw FALSE;
		}
		if (1 <= pExtArg->dwSubAddionalNum) {
			if (!GetAlignedCallocatedBuffer(pDevice, &pNextBuf,
				pDevice->TRANSFER.dwAllBufLen, &discPerSector.data.next, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			if (2 <= pExtArg->dwSubAddionalNum) {
				if (!GetAlignedCallocatedBuffer(pDevice, &pNextNextBuf,
					pDevice->TRANSFER.dwAllBufLen, &discPerSector.data.nextNext, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
			}
		}
		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		_TCHAR szSubCode[5] = { 0 };
		if (pExtArg->byD8 && !pDisc->SCSI.byAudioOnly && !pExtArg->byBe) {
			CDB::_PLXTR_READ_CDDA cdb = { 0 };
			if (pDevice->FEATURE.byC2ErrorData && pExtArg->byC2) {
				SetReadD8Command(pDevice, &cdb, 1, PLXTR_READ_CDDA_FLAG::MainC2Raw);
				_tcsncpy(szSubCode, _T("Raw"), 3);
			}
			else {
				SetReadD8Command(pDevice, &cdb, 1, PLXTR_READ_CDDA_FLAG::MainPack);
				_tcsncpy(szSubCode, _T("Pack"), 4);
			}
			memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
		}
		else {
			// non plextor && support scrambled ripping
			CDB::_READ_CD cdb = { 0 };
			READ_CD_FLAG::EXPECTED_SECTOR_TYPE type = READ_CD_FLAG::CDDA;
			if (pExtArg->byBe) {
				type = READ_CD_FLAG::All;
			}
			SetReadCDCommand(pExtArg, pDevice, &cdb,
				type, 1, READ_CD_FLAG::Raw, FALSE);
			_tcsncpy(szSubCode, _T("Raw"), 3);
			memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
		}
		OutputString(
			_T("Read command: %#x, Subcode reading mode: %s\n"), lpCmd[0], szSubCode);

		BYTE lpPrevSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 }; // only use PX-S88T
		// to get prevSubQ
		if (pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PXS88T) {
			// Somehow PX-S88T is sliding subchannel +1;
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, -2, discPerSector.data.present,
				pDevice->TRANSFER.dwAllBufLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(discPerSector.data.present + pDevice->TRANSFER.dwBufSubOffset, discPerSector.subcode.present);
			SetSubQDataFromBuffer(&discPerSector.subQ.prev, discPerSector.subcode.present);

			if (!ExecReadCD(pExtArg, pDevice, lpCmd, -1, discPerSector.data.present,
				pDevice->TRANSFER.dwAllBufLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(discPerSector.data.present + pDevice->TRANSFER.dwBufSubOffset, discPerSector.subcode.present);
			SetSubQDataFromBuffer(&discPerSector.subQ.present, discPerSector.subcode.present);
			memcpy(lpPrevSubcode, discPerSector.subcode.present, CD_RAW_READ_SUBCODE_SIZE);
		}
		else {
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, -1, discPerSector.data.present,
				pDevice->TRANSFER.dwAllBufLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(discPerSector.data.present + pDevice->TRANSFER.dwBufSubOffset, discPerSector.subcode.present);
			SetSubQDataFromBuffer(&discPerSector.subQ.prev, discPerSector.subcode.present);
		}
		if (discPerSector.subQ.prev.byAdr != ADR_ENCODES_CURRENT_POSITION) {
			// 1552 Tenka Tairan
			discPerSector.subQ.prev.byAdr = ADR_ENCODES_CURRENT_POSITION;
			discPerSector.subQ.prev.byTrackNum = 1;
			discPerSector.subQ.prev.nAbsoluteTime = 149;
		}

		for (UINT p = 0; p < pDisc->SCSI.toc.LastTrack; p++) {
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, pDisc->SCSI.lpFirstLBAListOnToc[p], discPerSector.data.present,
				pDevice->TRANSFER.dwAllBufLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			AlignRowSubcode(discPerSector.data.present + pDevice->TRANSFER.dwBufSubOffset, discPerSector.subcode.present);
			pDisc->SUB.lpEndCtlList[p] = (BYTE)((discPerSector.subcode.present[12] >> 4) & 0x0f);
		}
		SetCDOffset(pExtArg->byBe, pDisc, 0, pDisc->SCSI.nAllLength);
		BYTE byCurrentTrackNum = pDisc->SCSI.toc.FirstTrack;
		INT nFirstLBAForSub = 0;
		INT nFirstLBA = pDisc->MAIN.nOffsetStart;
		INT nLastLBA = pDisc->SCSI.nAllLength + pDisc->MAIN.nOffsetEnd;
		INT nLBA = nFirstLBA;	// This value switches by /r option.

		if (pExtArg->byReverse) {
			SetCDOffset(pExtArg->byBe, pDisc, nLastLBA, pDisc->SCSI.nFirstLBAofDataTrack);
			byCurrentTrackNum = pDisc->SCSI.byLastDataTrack;
			nFirstLBA = pDisc->SCSI.nFirstLBAofDataTrack;
			nLastLBA = pDisc->SCSI.nLastLBAofDataTrack + 1;
			nLBA = nLastLBA;
			if (pDisc->MAIN.nCombinedOffset > 0) {
				discPerSector.subQ.prev.nAbsoluteTime = 149 + nLastLBA;
			}
			else if (pDisc->MAIN.nCombinedOffset < 0) {
				discPerSector.subQ.prev.nAbsoluteTime = 150 + nLastLBA + pDisc->MAIN.nAdjustSectorNum - 1;
			}
			else {
				discPerSector.subQ.prev.nAbsoluteTime = 149 + nLastLBA;
			}
			discPerSector.subQ.prev.byCtl = pDisc->SUB.lpEndCtlList[pDisc->SCSI.byLastDataTrack - 1];
			discPerSector.subQ.prev.byAdr = ADR_ENCODES_CURRENT_POSITION;
			discPerSector.subQ.prev.byTrackNum = pDisc->SCSI.byLastDataTrack;
			discPerSector.subQ.prev.byIndex = pDisc->MAIN.nOffsetStart < 0 ? (BYTE)0 : (BYTE)1;
		}
		else if (pDisc->SUB.byIndex0InTrack1) {
			nFirstLBAForSub = -5000;
			nFirstLBA = -5000;
			nLBA = nFirstLBA;
			pDisc->MAIN.nOffsetStart = -5000;
		}
		// init end
#ifndef _DEBUG
		FlushLog();
#endif
		UINT uiC2ErrorLBACnt = 0;
		BOOL bReadOK = pDisc->SUB.byIndex0InTrack1 ? FALSE : TRUE;

		while (nFirstLBA <= nLastLBA) {
			BOOL bProcessRet = ProcessReadCD(pExecType, pExtArg, pDevice, pDisc,
				pC2ErrorPerSector, uiC2ErrorLBACnt, lpCmd, nLBA, &discPerSector, FALSE);
//#define C2TEST
#if defined C2TEST
			if (nLBA == 100) {
				memset(discPerSector.data.present, 0xff, 2352);
				bProcessRet = RETURNED_EXIST_C2_ERROR;
			}
#endif
			if (pC2ErrorPerSector && bProcessRet == RETURNED_EXIST_C2_ERROR) {
				OutputErrorString(
					_T(" Detected C2 error. LBA[%06d, %#07x]\n"), nLBA, nLBA);
				SetC2ErrorData(pC2ErrorPerSector, nLBA, &uiC2ErrorLBACnt, TRUE);
				if (uiC2ErrorLBACnt == pExtArg->dwMaxC2ErrorNum) {
					OutputErrorString(_T("C2 error Max: %u\n"), uiC2ErrorLBACnt);
					throw FALSE;
				}
			}
			else if (bProcessRet == RETURNED_SKIP_LBA) {
				if (pExtArg->byReverse) {
					nLBA = pDisc->SCSI.nFirstLBAof2ndSession - SESSION_TO_SESSION_SKIP_LBA;
				}
				else {
					nLBA = pDisc->MAIN.nFixFirstLBAof2ndSession - 1;
					nFirstLBA = nLBA;
				}
			}
			else if (bProcessRet == RETURNED_CONTINUE) {
#if 1
				OutputCDMain(fileMainError,
					discPerSector.mainHeader.present, nLBA, MAINHEADER_MODE1_SIZE);
#endif
				UpdateTmpMainHeader(&discPerSector.mainHeader,
					(LPBYTE)&discPerSector.mainHeader.prev, discPerSector.subQ.prev.byCtl, nType);
				WriteErrorBuffer(pExtArg, pDevice, pDisc, &discPerSector,
					scrambled_table, nLBA, fpImg, fpSub, fpC2);
//					aScrambledBuf, nLBA, fpImg, fpSub, fpC2);
#if 1
				if (pExtArg->byBe) {
					OutputCDMain(fileMainError,
						discPerSector.mainHeader.present, nLBA, MAINHEADER_MODE1_SIZE);
				}
				else {
					OutputCDMain(fileMainError,
						discPerSector.data.present + pDisc->MAIN.uiMainDataSlideSize, nLBA, MAINHEADER_MODE1_SIZE);
				}
#endif
				if (discPerSector.subQ.prev.byIndex == 0) {
					discPerSector.subQ.prev.nRelativeTime--;
				}
				else {
					discPerSector.subQ.prev.nRelativeTime++;
				}
				discPerSector.subQ.prev.nAbsoluteTime++;
			}
			else if (bProcessRet == RETURNED_FALSE) {
				throw FALSE;
			}
			if (bProcessRet != RETURNED_CONTINUE &&
				bProcessRet != RETURNED_SKIP_LBA) {
				if (pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PXS88T) {
					memcpy(discPerSector.subcode.nextNext, discPerSector.subcode.next, CD_RAW_READ_SUBCODE_SIZE);
					memcpy(discPerSector.subcode.next, discPerSector.subcode.present, CD_RAW_READ_SUBCODE_SIZE);
					memcpy(discPerSector.subcode.present, lpPrevSubcode, CD_RAW_READ_SUBCODE_SIZE);
				}
				SetSubQDataFromBuffer(&discPerSector.subQ.present, discPerSector.subcode.present);

				if (pDisc->SUB.byIndex0InTrack1 && -5000 <= nLBA && nLBA <= -76) {
					if (discPerSector.subQ.present.byTrackNum == 1 && discPerSector.subQ.present.nAbsoluteTime == 0) {
						discPerSector.subQ.prev.nRelativeTime = discPerSector.subQ.present.nRelativeTime + 1;
						discPerSector.subQ.prev.nAbsoluteTime = -1;
						pDisc->MAIN.nFixStartLBA = nLBA;
						bReadOK = TRUE;
						if (pDisc->MAIN.nAdjustSectorNum < 0 ||
							1 < pDisc->MAIN.nAdjustSectorNum) {
							for (INT i = 0; i < abs(pDisc->MAIN.nAdjustSectorNum) * CD_RAW_SECTOR_SIZE; i++) {
								fputc(0, fpImg);
							}
						}
					}
					if (bReadOK) {
						if (discPerSector.subQ.present.byTrackNum == 1 && discPerSector.subQ.present.nAbsoluteTime == 74) {
							nFirstLBA = -76;
						}
					}
				}
				if (bReadOK) {
					if (nFirstLBAForSub <= nLBA && nLBA < pDisc->SCSI.nAllLength) {
						if (nLBA < pDisc->SCSI.nFirstLBAofLeadout ||
							pDisc->SCSI.nFirstLBAof2ndSession <= nLBA) {
							BOOL bLibCrypt = IsValidLibCryptSector(pExtArg->byLibCrypt, nLBA);
							if (!pExtArg->byReverse) {
								if (pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PXS88T) {
									SetSubQDataFromBuffer(&discPerSector.subQ.next, discPerSector.subcode.next);
									if (1 <= pExtArg->dwSubAddionalNum) {
										SetSubQDataFromBuffer(&discPerSector.subQ.nextNext, discPerSector.subcode.nextNext);
									}
								}
								else {
									if (1 <= pExtArg->dwSubAddionalNum) {
										SetSubQDataFromBuffer(&discPerSector.subQ.next, discPerSector.subcode.next);
										if (2 <= pExtArg->dwSubAddionalNum) {
											SetSubQDataFromBuffer(&discPerSector.subQ.nextNext, discPerSector.subcode.nextNext);
										}
									}
								}
								CheckAndFixSubChannel(pExecType, pExtArg, pDisc,
									discPerSector.subcode.present, &discPerSector.subQ, byCurrentTrackNum, nLBA, bLibCrypt);
								BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
								// fix raw subchannel
								AlignColumnSubcode(discPerSector.subcode.present, lpSubcodeRaw);
#if 0
								OutputCDSub96Align(discPerSector.subcode.present, nLBA);
#endif
								WriteSubChannel(pDisc, lpSubcodeRaw,
									discPerSector.subcode.present, nLBA, byCurrentTrackNum, fpSub, fpParse);
							}
							BOOL bHeader = IsValidMainDataHeader(discPerSector.data.present + pDisc->MAIN.uiMainDataSlideSize);
							if (bHeader) {
								if (pDisc->PROTECT.byExist == smartE &&
									(pDisc->PROTECT.ERROR_SECTOR.nExtentPos <= nLBA &&
									nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize)) {
									BYTE m, s, f;
									if (!pExtArg->byBe) {
										m = BcdToDec(BYTE(discPerSector.data.present[12 + pDisc->MAIN.uiMainDataSlideSize] ^ 0x01));
										s = BcdToDec(BYTE(discPerSector.data.present[13 + pDisc->MAIN.uiMainDataSlideSize] ^ 0x80));
										f = BcdToDec(discPerSector.data.present[14 + pDisc->MAIN.uiMainDataSlideSize]);
									}
									else {
										m = BcdToDec(discPerSector.data.present[12]);
										s = BcdToDec(discPerSector.data.present[13]);
										f = BcdToDec(discPerSector.data.present[14]);
									}
									INT tmpLBA = MSFtoLBA(m, s, f) - 150;
									if (tmpLBA < nLBA) {
										BYTE rm, rs, rf, mb, sb;
										LBAtoMSF(nLBA + 150, &rm, &rs, &rf);
										mb = rm;
										sb = rs;
										if (!pExtArg->byBe) {
											rm ^= 0x01;
											rs ^= 0x80;
										}
										discPerSector.data.present[12 + pDisc->MAIN.uiMainDataSlideSize] = rm;
										discPerSector.data.present[13 + pDisc->MAIN.uiMainDataSlideSize] = rs;
										discPerSector.data.present[14 + pDisc->MAIN.uiMainDataSlideSize] = rf;
										OutputMainErrorLogA(
											"LBA[%06d, %#07x], Track[%02u]: m[%02u], s[%02u], f[%02u] -> m[%02u], s[%02u], f[%02u]\n"
											, nLBA, nLBA, byCurrentTrackNum, m, s, f, mb, sb, rf);
									}
								}
							}
							else {
								if (pExtArg->byReadContinue && pDisc->PROTECT.byExist &&
									(pDisc->PROTECT.ERROR_SECTOR.nExtentPos <= nLBA &&
									nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize)) {
									discPerSector.data.present[pDisc->MAIN.uiMainDataSlideSize + 15] = discPerSector.mainHeader.present[15];
								}
							}
							UpdateTmpMainHeader(&discPerSector.mainHeader,
								discPerSector.data.present + pDisc->MAIN.uiMainDataSlideSize, discPerSector.subQ.present.byCtl, nType);
#if 0
							if (nLBA >= 979 && 989 >= nLBA) {
								OutputCDMain(fileMainError, discPerSector.mainHeader.present, nLBA, CD_RAW_SECTOR_SIZE);
							}
#endif
							if (!bHeader) {
								if (pExtArg->byReadContinue && pDisc->PROTECT.byExist &&
									(pDisc->PROTECT.ERROR_SECTOR.nExtentPos <= nLBA &&
									nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize)) {
									OutputMainErrorLogA(
										"LBA[%06d, %#07x], Track[%02u]: This sector is data, but a header doesn't exist. Header is generated\n",
										nLBA, nLBA, byCurrentTrackNum);
									memcpy(discPerSector.data.present + pDisc->MAIN.uiMainDataSlideSize,
										discPerSector.mainHeader.present, MAINHEADER_MODE1_SIZE);
								}
							}
							PreserveTrackAttribution(pExtArg, pDisc, nLBA,
								&byCurrentTrackNum, &discPerSector.mainHeader, &discPerSector.subQ);
							UpdateSubQData(&discPerSector.subQ, bLibCrypt);
						}
					}
					CheckMainChannel(pDisc, discPerSector.data.present,
						&discPerSector.subQ, byCurrentTrackNum, nLBA);
					// Write track to scrambled
					WriteMainChannel(pExtArg, pDisc, discPerSector.data.present, nLBA, fpImg);
					if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
						WriteC2(pDisc, discPerSector.data.present + pDevice->TRANSFER.dwBufC2Offset, nLBA, fpC2);
					}
				}
				if (pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PXS88T) {
					memcpy(lpPrevSubcode, discPerSector.subcode.next, CD_RAW_READ_SUBCODE_SIZE);
				}
			}

			if (pExtArg->byReverse) {
				OutputString(_T("\rCreated img (LBA) %6d/%6d"),
					nLBA, pDisc->SCSI.nFirstLBAofDataTrack);
				nLBA--;
			}
			else {
				OutputString(_T("\rCreated img (LBA) %6d/%6d"),
					nLBA, pDisc->SCSI.nAllLength - 1);
//					nFirstLBA - 1, pDisc->SCSI.nAllLength - 1);
				if (nFirstLBA == -76) {
					nLBA = nFirstLBA;
				}
				nLBA++;
			}
			nFirstLBA++;
		}
		OutputString(_T("\n"));
		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			FcloseAndNull(fpC2);
		}
		FcloseAndNull(fpSub);
		FcloseAndNull(fpParse);
#ifndef _DEBUG
		FlushLog();
#endif
		if (!pExtArg->byReverse) {
			if (pDisc->SCSI.toc.FirstTrack == pDisc->SCSI.toc.LastTrack) {
				pDisc->SUB.lpFirstLBAListOnSub[0][1] = pDisc->SCSI.lpLastLBAListOnToc[0];
			}
			for (INT i = 0; i < pDisc->SCSI.toc.LastTrack; i++) {
				BOOL bErr = FALSE;
				LONG lLine = 0;
				if (pDisc->SUB.lpFirstLBAListOnSub[i][1] == -1) {
					bErr = TRUE;
					lLine = __LINE__;
				}
				else if ((pDisc->SCSI.toc.TrackData[i].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
					if (pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[i] == -1) {
						bErr = TRUE;
						lLine = __LINE__;
					}
					else if (pDisc->SUB.lpLastLBAListOfDataTrackOnSub[i] == -1) {
						bErr = TRUE;
						lLine = __LINE__;
					}
				}
				if (bErr) {
					OutputErrorString(
						_T("[L:%d] Internal error. Failed to analyze the subchannel. Track[%02u]/[%02u]\n"),
						lLine, i + 1, pDisc->SCSI.toc.LastTrack);
					throw FALSE;
				}
			}
		}
		if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			if (!ReadCDForRereadingSector(pExecType, pExtArg, pDevice, pDisc,
				pC2ErrorPerSector, uiC2ErrorLBACnt, lpCmd, &discPerSector, fpImg)) {
				throw FALSE;
			}
		}
		FcloseAndNull(fpImg);

		if (!pExtArg->byReverse) {
			OutputTocWithPregap(pDisc);
		}
		else {
			FILE* fpImg_r = NULL;
			if (NULL == (fpImg_r = CreateOrOpenFileW(
				pszPath, _T("_reverse"), NULL, NULL, NULL, _T(".scm"), _T("rb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == (fpImg = CreateOrOpenFileW(
				pszPath, NULL, pszOutScmFile, NULL, NULL, _T(".scm"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			OutputString(_T("Reversing _reverse.scm to .scm\n"));
			BYTE rBuf[CD_RAW_SECTOR_SIZE] = { 0 };
			DWORD dwRoop = GetFileSize(0, fpImg_r) - CD_RAW_SECTOR_SIZE * 2;
			LONG lSeek = CD_RAW_SECTOR_SIZE - (LONG)pDisc->MAIN.uiMainDataSlideSize;
			fseek(fpImg_r, -lSeek, SEEK_END);
			fread(rBuf, sizeof(BYTE), (size_t)lSeek, fpImg_r);
			fwrite(rBuf, sizeof(BYTE), (size_t)lSeek, fpImg);
			fseek(fpImg_r, -lSeek, SEEK_CUR);
			for (DWORD i = 0; i < dwRoop; i += CD_RAW_SECTOR_SIZE) {
				fseek(fpImg_r, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
				fread(rBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpImg_r);
				fwrite(rBuf, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpImg);
				fseek(fpImg_r, -CD_RAW_SECTOR_SIZE, SEEK_CUR);
			}
			rewind(fpImg_r);
			fread(rBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpImg_r);
			fwrite(rBuf, sizeof(BYTE), pDisc->MAIN.uiMainDataSlideSize, fpImg);
			FcloseAndNull(fpImg);
			FcloseAndNull(fpImg_r);
		}

		_TCHAR pszNewPath[_MAX_PATH] = { 0 };
		_tcsncpy(pszNewPath, pszPath, _MAX_PATH);
		pszNewPath[_MAX_PATH - 1] = 0;
		if (!PathRenameExtension(pszNewPath, _T(".img"))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		// audio only -> from .scm to .img. other descramble img.
		if (pDisc->SCSI.byAudioOnly || pExtArg->byBe) {
			OutputString(_T("Moving .scm to .img\n"));
			if (!MoveFileEx(pszOutScmFile, pszNewPath, MOVEFILE_REPLACE_EXISTING)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (pExtArg->byBe) {
				ExecEccEdc(pExtArg->byReadContinue, pszNewPath, pDisc->PROTECT.ERROR_SECTOR);
			}
		}
		else {
			OutputString(_T("Copying .scm to .img\n"));
			if (!CopyFile(pszOutScmFile, pszNewPath, FALSE)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			_TCHAR pszImgPath[_MAX_PATH] = { 0 };
			if (NULL == (fpImg = CreateOrOpenFileW(
				pszPath, NULL, pszImgPath, NULL, NULL, _T(".img"), _T("rb+"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			DescrambleMainChannel(pExtArg, pDisc, scrambled_table, fpImg);
//			DescrambleMainChannel(pExtArg, pDisc, aScrambledBuf, fpImg);
			FcloseAndNull(fpImg);
			ExecEccEdc(pExtArg->byReadContinue, pszImgPath, pDisc->PROTECT.ERROR_SECTOR);
		}

		if (pExtArg->byReverse) {
			_TCHAR pszNewPath2[_MAX_PATH] = { 0 };
			FILE* fpBin = CreateOrOpenFileW(pszPath, NULL, pszNewPath2, NULL, NULL, _T(".bin"),
				_T("wb"), pDisc->SCSI.byFirstDataTrack, pDisc->SCSI.byFirstDataTrack);
			if (!fpBin) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			OutputString(_T("Copying .img to %s\n"), pszNewPath2);
			if (!CopyFile(pszNewPath, pszNewPath2, FALSE)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			FcloseAndNull(fpBin);
		}
		else {
			_TCHAR pszImgName[_MAX_FNAME] = { 0 };
			if (NULL == (fpImg = CreateOrOpenFileW(
				pszPath, NULL, NULL, pszImgName, NULL, _T(".img"), _T("rb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (!CreateBinCueCcd(pDisc, pszPath, pszImgName,
				pDevice->FEATURE.byCanCDText, fpImg, fpCue, fpCueForImg, fpCcd)) {
				throw FALSE;
			}
		}
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpImg);
	if (pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
		FcloseAndNull(fpC2);
	}
	FcloseAndNull(fpCueForImg);
	FcloseAndNull(fpCue);
	FcloseAndNull(fpParse);
	FcloseAndNull(fpSub);
	FreeAndNull(pBuf);
	if (1 <= pExtArg->dwSubAddionalNum) {
		FreeAndNull(pNextBuf);
		if (2 <= pExtArg->dwSubAddionalNum) {
			FreeAndNull(pNextNextBuf);
		}
	}
	TerminateC2ErrorData(pExtArg, pDevice, &pC2ErrorPerSector);

	return bRet;
}

BOOL ReadCDPartial(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PMAIN_HEADER pMain,
	LPCTSTR pszPath,
	INT nStart,
	INT nEnd,
	READ_CD_FLAG::EXPECTED_SECTOR_TYPE flg,
	BOOL bCheckReading
	)
{
	CONST INT size = 8;
	_TCHAR szPlusFnameSub[size] = { 0 };
	_TCHAR szPlusFnameTxt[size] = { 0 };
	_TCHAR szExt[size] = { 0 };

	if (*pExecType == gd) {
		_tcsncpy(szPlusFnameSub, _T("_dc"), size);
		_tcsncpy(szPlusFnameTxt, _T("_dc_sub"), size);
		_tcsncpy(szExt, _T(".scm2"), size);
	}
	else {
		_tcsncpy(szPlusFnameTxt, _T("_sub"), size);
		_tcsncpy(szExt, _T(".bin"), size);
	}
	FILE* fpBin = 
		CreateOrOpenFileW(pszPath, NULL, NULL, NULL, NULL, szExt, _T("wb"), 0, 0);
	if (!fpBin) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	FILE* fpC2 = NULL;
	FILE* fpParse = NULL;
	FILE* fpSub = NULL;
	BOOL bRet = TRUE;
	LPBYTE pBuf = NULL;
	LPBYTE pNextBuf = NULL;
	LPBYTE pNextNextBuf = NULL;
	PC2_ERROR_PER_SECTOR pC2ErrorPerSector = NULL;
#if 0
	BYTE aScrambledBuf[CD_RAW_SECTOR_SIZE] = { 0 };
#endif
	DISC_PER_SECTOR discPerSector = { 0 };
	memcpy(&discPerSector.mainHeader, pMain, sizeof(MAIN_HEADER));
	try {
#if 0
		FILE* fpTbl = NULL;
		if (NULL == (fpTbl = OpenProgrammabledFile(_T("scramble.bin"), _T("rb")))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		fread(aScrambledBuf, sizeof(BYTE), sizeof(aScrambledBuf), fpTbl);
		FcloseAndNull(fpTbl);
#endif
		if (!pExtArg->byReverse) {
			if (NULL == (fpParse = CreateOrOpenFileW(
				pszPath, szPlusFnameTxt, NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (NULL == (fpSub = CreateOrOpenFileW(
				pszPath, szPlusFnameSub, NULL, NULL, NULL, _T(".sub"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
		}

		SetCDTransfer(pDevice, DRIVE_DATA_ORDER::NoC2);
		if (!bCheckReading && pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			if (NULL == (fpC2 = CreateOrOpenFileW(
				pszPath, NULL, NULL, NULL, NULL, _T(".c2"), _T("wb"), 0, 0))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			SetCDTransfer(pDevice, DRIVE_DATA_ORDER::MainC2Sub);
			DRIVE_DATA_ORDER dataOrder = DRIVE_DATA_ORDER::MainC2Sub;

			if (!InitC2ErrorData(pExtArg,
				&pC2ErrorPerSector, pDevice->TRANSFER.dwAllBufLen)) {
				throw FALSE;
			}
			if (!ReadCDForCheckingByteOrder(pExtArg, pDevice, &dataOrder)) {
				TerminateC2ErrorData(pExtArg, pDevice, &pC2ErrorPerSector);
				pDevice->FEATURE.byC2ErrorData = FALSE;
				SetCDTransfer(pDevice, DRIVE_DATA_ORDER::NoC2);
			}
			if (dataOrder == DRIVE_DATA_ORDER::MainSubC2) {
				OutputDriveLogA(
					"=> Byte order of this drive is main + sub + c2\n");
				SetCDTransfer(pDevice, DRIVE_DATA_ORDER::MainSubC2);
			}
			else if (dataOrder == DRIVE_DATA_ORDER::MainC2Sub) {
				OutputDriveLogA(
					"=> Byte order of this drive is main + c2 + sub\n");
			}
		}
		// store main+(c2)+sub data
		if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
			pDevice->TRANSFER.dwAllBufLen, &discPerSector.data.present, _T(__FUNCTION__), __LINE__)) {
			throw FALSE;
		}
		if (1 <= pExtArg->dwSubAddionalNum) {
			if (!GetAlignedCallocatedBuffer(pDevice, &pNextBuf,
				pDevice->TRANSFER.dwAllBufLen, &discPerSector.data.next, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			if (2 <= pExtArg->dwSubAddionalNum) {
				if (!GetAlignedCallocatedBuffer(pDevice, &pNextNextBuf,
					pDevice->TRANSFER.dwAllBufLen, &discPerSector.data.nextNext, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
			}
		}
		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		_TCHAR szSubCode[5] = { 0 };
		if (pExtArg->byD8 && !pDisc->SCSI.byAudioOnly) {
			CDB::_PLXTR_READ_CDDA cdb = { 0 };
			if (!bCheckReading && pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
				SetReadD8Command(pDevice, &cdb, 1, PLXTR_READ_CDDA_FLAG::MainC2Raw);
				_tcsncpy(szSubCode, _T("Raw"), 3);
			}
			else {
				SetReadD8Command(pDevice, &cdb, 1, PLXTR_READ_CDDA_FLAG::MainPack);
				_tcsncpy(szSubCode, _T("Pack"), 4);
			}
			memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
		}
		else {
			// non plextor && support scrambled ripping
			CDB::_READ_CD cdb = { 0 };
			SetReadCDCommand(pExtArg, pDevice, &cdb,
				flg, 1, READ_CD_FLAG::Raw, bCheckReading);
			_tcsncpy(szSubCode, _T("Raw"), 3);
			memcpy(lpCmd, &cdb, CDB12GENERIC_LENGTH);
		}
		OutputString(
			_T("Read command: %#x, Subcode reading mode: %s\n"), lpCmd[0], szSubCode);

		BYTE lpPrevSubcode[CD_RAW_READ_SUBCODE_SIZE] = { 0 }; // only use PX-S88T
		if (*pExecType != audio && *pExecType != data) {
			if (!ExecReadCD(pExtArg, pDevice, lpCmd, nStart - 1, discPerSector.data.present,
				pDevice->TRANSFER.dwAllBufLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
		}
		AlignRowSubcode(discPerSector.data.present + pDevice->TRANSFER.dwBufSubOffset, discPerSector.subcode.present);
		SetSubQDataFromBuffer(&discPerSector.subQ.prev, discPerSector.subcode.present);

		if (*pExecType == gd) {
			for (INT p = pDisc->GDROM_TOC.FirstTrack - 1; p < pDisc->GDROM_TOC.LastTrack; p++) {
				pDisc->SUB.lpEndCtlList[p] = pDisc->GDROM_TOC.TrackData[p].Control;
			}
		}
		else {
			for (UINT p = 0; p < pDisc->SCSI.toc.LastTrack; p++) {
				if (!ExecReadCD(pExtArg, pDevice, lpCmd, pDisc->SCSI.lpFirstLBAListOnToc[p],
					discPerSector.data.present, pDevice->TRANSFER.dwAllBufLen, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
				AlignRowSubcode(discPerSector.data.present + pDevice->TRANSFER.dwBufSubOffset, discPerSector.subcode.present);
				pDisc->SUB.lpEndCtlList[p] = (BYTE)((discPerSector.subcode.present[12] >> 4) & 0x0f);
			}
		}
		SetCDOffset(pExtArg->byBe, pDisc, nStart, nEnd);
		BYTE byCurrentTrackNum = discPerSector.subQ.prev.byTrackNum;
		if (*pExecType == gd) {
			byCurrentTrackNum = pDisc->GDROM_TOC.FirstTrack;
			// because address out of range
			pDisc->MAIN.nOffsetEnd = 0;
		}
		INT nLBA = nStart + pDisc->MAIN.nOffsetStart;
		INT nLastLBA = nEnd + pDisc->MAIN.nOffsetEnd;
#ifndef _DEBUG
		FlushLog();
#endif
		UINT uiC2ErrorLBACnt = 0;

		while (nLBA <= nLastLBA) {
			BOOL bProcessRet = ProcessReadCD(pExecType, pExtArg, pDevice, pDisc, 
				pC2ErrorPerSector, uiC2ErrorLBACnt, lpCmd, nLBA, &discPerSector, bCheckReading);
			if (pC2ErrorPerSector && bProcessRet == RETURNED_EXIST_C2_ERROR) {
				OutputErrorString(
					_T(" Detected C2 error. LBA[%06d, %#07x]\n"), nLBA, nLBA);
				SetC2ErrorData(pC2ErrorPerSector, nLBA, &uiC2ErrorLBACnt, TRUE);
				if (uiC2ErrorLBACnt == pExtArg->dwMaxC2ErrorNum) {
					OutputErrorString(_T("C2 error Max: %u\n"), uiC2ErrorLBACnt);
					throw FALSE;
				}
#ifdef _DEBUG
				OutputCDMain(fileMainError,
					discPerSector.data.present, nLBA, CD_RAW_SECTOR_SIZE);
#endif
			}
			else if (bProcessRet == RETURNED_SKIP_LBA) {
				nLBA = pDisc->MAIN.nFixFirstLBAof2ndSession - 1;
			}
			else if (bProcessRet == RETURNED_FALSE) {
				throw FALSE;
			}

			if (bProcessRet != RETURNED_CONTINUE &&
				bProcessRet != RETURNED_SKIP_LBA) {
				if (pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PXS88T) {
					memcpy(discPerSector.subcode.nextNext, discPerSector.subcode.next, CD_RAW_READ_SUBCODE_SIZE);
					memcpy(discPerSector.subcode.next, discPerSector.subcode.present, CD_RAW_READ_SUBCODE_SIZE);
					memcpy(discPerSector.subcode.present, lpPrevSubcode, CD_RAW_READ_SUBCODE_SIZE);
				}
				SetSubQDataFromBuffer(&discPerSector.subQ.present, discPerSector.subcode.present);

				if (nStart <= nLBA && nLBA < nEnd) {
					if (nLBA < pDisc->SCSI.nFirstLBAofLeadout ||
						pDisc->SCSI.nFirstLBAof2ndSession <= nLBA) {
						if (pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PXS88T) {
							SetSubQDataFromBuffer(&discPerSector.subQ.next, discPerSector.subcode.next);
							if (1 <= pExtArg->dwSubAddionalNum) {
								SetSubQDataFromBuffer(&discPerSector.subQ.nextNext, discPerSector.subcode.nextNext);
							}
						}
						else {
							if (1 <= pExtArg->dwSubAddionalNum) {
								SetSubQDataFromBuffer(&discPerSector.subQ.next, discPerSector.subcode.next);
								if (2 <= pExtArg->dwSubAddionalNum) {
									SetSubQDataFromBuffer(&discPerSector.subQ.nextNext, discPerSector.subcode.nextNext);
								}
							}
						}
						if (*pExecType != audio && *pExecType != data) {
							CheckAndFixSubChannel(pExecType, pExtArg, pDisc, discPerSector.subcode.present,
								&discPerSector.subQ, byCurrentTrackNum, nLBA, FALSE);
						}
						BYTE lpSubcodeRaw[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
						// fix raw subchannel
						AlignColumnSubcode(discPerSector.subcode.present, lpSubcodeRaw);
#if 0
						OutputCDSub96Align(discPerSector.subcode.present, nLBA);
#endif
						if (!pExtArg->byReverse) {
							WriteSubChannel(pDisc, lpSubcodeRaw,
								discPerSector.subcode.present, nLBA, byCurrentTrackNum, fpSub, fpParse);
						}
						if (IsValidMainDataHeader(discPerSector.data.present + pDisc->MAIN.uiMainDataSlideSize)) {
							if (discPerSector.data.present[pDisc->MAIN.uiMainDataSlideSize + 12] == discPerSector.mainHeader.present[12] &&
								discPerSector.data.present[pDisc->MAIN.uiMainDataSlideSize + 13] == discPerSector.mainHeader.present[13] &&
								discPerSector.data.present[pDisc->MAIN.uiMainDataSlideSize + 14] == discPerSector.mainHeader.present[14]) {
								memcpy(discPerSector.mainHeader.present,
									discPerSector.data.present + pDisc->MAIN.uiMainDataSlideSize, MAINHEADER_MODE1_SIZE);
							}
						}
						UpdateTmpMainHeader(&discPerSector.mainHeader,
							discPerSector.data.present + pDisc->MAIN.uiMainDataSlideSize, discPerSector.subQ.present.byCtl, SCRAMBLED);
#if 0
						OutputCDMain(discPerSector.mainHeader.present, nLBA, MAINHEADER_MODE1_SIZE);
#endif
						if (*pExecType == gd) {
							byCurrentTrackNum = discPerSector.subQ.present.byTrackNum;
						}
						else {
							PreserveTrackAttribution(pExtArg, pDisc, nLBA,
								&byCurrentTrackNum, &discPerSector.mainHeader, &discPerSector.subQ);
						}
						UpdateSubQData(&discPerSector.subQ, FALSE);
					}
				}
				if (!bCheckReading) {
					CheckMainChannel(pDisc, discPerSector.data.present,
						&discPerSector.subQ, byCurrentTrackNum, nLBA);
				}
				WriteMainChannel(pExtArg, pDisc, discPerSector.data.present, nLBA, fpBin);
				if (!bCheckReading && pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
					WriteC2(pDisc, discPerSector.data.present + pDevice->TRANSFER.dwBufC2Offset, nLBA, fpC2);
				}
				if (pDevice->byPlxtrType == PLXTR_DRIVE_TYPE::PXS88T) {
					memcpy(lpPrevSubcode, discPerSector.subcode.next, CD_RAW_READ_SUBCODE_SIZE);
				}
			}
			OutputString(_T("\rCreating bin from %d to %d (LBA) %6d"), 
				nStart + pDisc->MAIN.nOffsetStart, nEnd + pDisc->MAIN.nOffsetEnd, nLBA);
			nLBA++;
		}
		OutputString(_T("\n"));
		if (!bCheckReading && pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			FcloseAndNull(fpC2);
		}
		FcloseAndNull(fpParse);
		FcloseAndNull(fpSub);
#ifndef _DEBUG
		FlushLog();
#endif
		if (!bCheckReading && pExtArg->byC2 && pDevice->FEATURE.byC2ErrorData) {
			if (!ReadCDForRereadingSector(pExecType, pExtArg, pDevice, pDisc,
				pC2ErrorPerSector, uiC2ErrorLBACnt, lpCmd, &discPerSector, fpBin)) {
				throw FALSE;
			}
		}
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FcloseAndNull(fpBin);
	FcloseAndNull(fpC2);
	FcloseAndNull(fpParse);
	FcloseAndNull(fpSub);
	FreeAndNull(pBuf);
	if (1 <= pExtArg->dwSubAddionalNum) {
		FreeAndNull(pNextBuf);
		if (2 <= pExtArg->dwSubAddionalNum) {
			FreeAndNull(pNextNextBuf);
		}
	}
	TerminateC2ErrorData(pExtArg, pDevice, &pC2ErrorPerSector);

	if (bRet && *pExecType == gd) {
		if (!DescrambleMainChannelForGD(pszPath)) {
			return FALSE;
		}
		if (!SplitFileForGD(pszPath)) {
			return FALSE;
		}
	}
	return bRet;
}

BOOL ReadCDForGDTOC(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
	)
{
	CDB::_READ_CD cdb = { 0 };
	SetReadCDCommand(NULL, pDevice, &cdb,
		READ_CD_FLAG::All, 1, READ_CD_FLAG::NoSub, TRUE);
	BYTE aToc[CD_RAW_SECTOR_SIZE] = { 0 };
#if 0
	FILE* fpTbl = NULL;
	if (NULL == (fpTbl = OpenProgrammabledFile(_T("scramble.bin"), _T("rb")))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BYTE bufTbl[CD_RAW_SECTOR_SIZE] = { 0 };
	fread(bufTbl, sizeof(BYTE), CD_RAW_SECTOR_SIZE, fpTbl);
	FcloseAndNull(fpTbl);
#endif
	if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, FIRST_LBA_FOR_GD, aToc,
		CD_RAW_SECTOR_SIZE, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}

	BYTE bufDec[CD_RAW_SECTOR_SIZE] = { 0 };
	INT idx = pDisc->MAIN.nCombinedOffset;
	for (INT j = 0; j < CD_RAW_SECTOR_SIZE; j++) {
#if 0
		bufDec[j] = (BYTE)(aToc[idx + j] ^ bufTbl[j]);
#else
		bufDec[j] = (BYTE)(aToc[idx + j] ^ scrambled_table[j]);
#endif
	}
	/*
	0x110 - 0x113: TOC1
	0x114 - 0x116: LBA(little) |
	0x117        : Ctl/Adr     |
	  :                        |-> 100 track
	  :                        |
	0x294 - 0x296: LBA(little) |
	0x297        : Ctl/Adr     |
	0x298 - 0x299: Zero
	0x29a        : First track |-> alway "3"
	0x29b        : Ctl/Adr     |-> alway "41"
	0x29c - 0x29d: Zero
	0x29e        : Last track
	0x29f        : Ctl/Adr
	0x2a0 - 0x2a2: Max LBA     |-> alway "b4 61 08" (549300)
	0x2a3        : Ctl/Adr     |-> alway "41"
	*/
	if (bufDec[0x110] != 'T' || bufDec[0x111] != 'O' ||
		bufDec[0x112] != 'C' || bufDec[0x113] != '1') {
		OutputErrorString(_T("No GD-ROM data\n"));
		return FALSE;
	}
	pDisc->GDROM_TOC.FirstTrack = bufDec[0x29a];
	pDisc->GDROM_TOC.LastTrack = bufDec[0x29e];
	pDisc->GDROM_TOC.Length = MAKELONG(
		MAKEWORD(bufDec[0x2a0], bufDec[0x2a1]), MAKEWORD(bufDec[0x2a2], 0));

	for (INT i = pDisc->GDROM_TOC.FirstTrack - 1, j = 0; i < pDisc->GDROM_TOC.LastTrack; i++, j += 4) {
		pDisc->GDROM_TOC.TrackData[i].Address = MAKELONG(
			MAKEWORD(bufDec[0x114 + j], bufDec[0x115 + j]), MAKEWORD(bufDec[0x116 + j], 0));
		pDisc->GDROM_TOC.TrackData[i].Control = BYTE((bufDec[0x117 + j]) >> 4 & 0x0f);
		pDisc->GDROM_TOC.TrackData[i].Adr = BYTE((bufDec[0x117 + j]) & 0x0f);
		pDisc->GDROM_TOC.TrackData[i].TrackNumber = (BYTE)(i + 1);
	}
	OutputTocForGD(pDisc);
	return TRUE;
}
