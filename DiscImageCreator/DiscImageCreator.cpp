// DiscImageCreator.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//
/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"

typedef enum _ExecType {
	rall,
	rd,
	ra,
	c,
	s,
	dec,
	split,
	sub,
} ExecType;

int exec(_TCHAR* argv[], ExecType execType)
{
	if(execType == dec) {
		DescrambleDataSector(argv[2], _ttoi(argv[3]));
		return TRUE;
	}
	else if(execType == split) {
		SplitDescrambledFile(argv[2]);
		return TRUE;
	}
	else if(execType == sub) {
		OutputParsingSubfile(argv[2]);
		return TRUE;
	}

	TCHAR szBuf[7] = {0};
	_sntprintf(szBuf, 7, _T("\\\\.\\%c:"), argv[2][0]);
	HANDLE hDevice = CreateFile(szBuf, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(hDevice == INVALID_HANDLE_VALUE) {
		OutputErrorString(_T("Device Open fail\n"));
		return FALSE;
	}
	FILE* fpLog = NULL;
	BOOL bDC = FALSE;
	if(execType == ra && !_tcscmp(argv[5], _T("44990")) && !_tcscmp(argv[6], _T("549150"))) {
		bDC = TRUE;
	}
#ifndef _DEBUG
	_TCHAR szLogtxt[12] = {0};
	if(bDC) {
		_tcscpy(szLogtxt, _T("_dc.log.txt"));
	}
	else {
		_tcscpy(szLogtxt, _T(".log.txt"));
	}
	fpLog = CreateOrOpenFileW(argv[4], NULL, NULL, NULL, szLogtxt, _T(WFLAG), 0, 0);

	if(!fpLog) {
		OutputErrorString(_T("Failed to open file %s\n"), szLogtxt);
		return FALSE;
	}
#endif
	BOOL bRet = 0;
	ULONG ulReturned = 0;
	SCSI_ADDRESS adr = {0};
	bRet = DeviceIoControl(hDevice, IOCTL_SCSI_GET_ADDRESS, &adr, 
		sizeof(SCSI_ADDRESS), &adr, sizeof(SCSI_ADDRESS), &ulReturned, NULL);
	OutputLogString(fpLog, _T("IOCTL_SCSI_GET_ADDRESS\n"));
	OutputLogString(fpLog, _T("\tLength %d\n"), adr.Length);
	OutputLogString(fpLog, _T("\tPortNumber %d\n"), adr.PortNumber);
	OutputLogString(fpLog, _T("\tPathId %d\n"), adr.PathId);
	OutputLogString(fpLog, _T("\tTargetId %d\n"), adr.TargetId);
	OutputLogString(fpLog, _T("\tLun %d\n"), adr.Lun);

	if(execType == c) {
		return StartStop(hDevice, &adr, START_UNIT_CODE, START_UNIT_CODE);
	}
	else if(execType == s) {
		return StartStop(hDevice, &adr, STOP_UNIT_CODE, STOP_UNIT_CODE);
	}

	bRet = ReadTestUnitReady(hDevice, &adr);
	if(!bRet) {
		return FALSE;
	}
	CHAR pszVendorId[8+1] = {0};
	CHAR pszProductId[16+1] = {0};

	bRet = ReadDeviceInfo(hDevice, &adr, pszVendorId, pszProductId, fpLog);
	if(!bRet) {
		return FALSE;
	}
	pszVendorId[8] = '\0';
	pszProductId[16] = '\0';

	USHORT usFeatureProfileType = ProfileInvalid;
	BOOL bCanCDText = FALSE;
	BOOL bC2ErrorData = FALSE;

	SetCDSpeed(hDevice, &adr, _ttoi(argv[3]), fpLog);
	bRet = ReadConfiguration(hDevice, &adr, &usFeatureProfileType, &bCanCDText, &bC2ErrorData, fpLog);
	if(!bRet) {
		return FALSE;
	}
	if(usFeatureProfileType == ProfileCdrom || 
		usFeatureProfileType == ProfileCdRecordable ||
		usFeatureProfileType == ProfileCdRewritable ||
		(usFeatureProfileType == ProfileInvalid && (execType == ra))) {
		INT nLength = 0;
		TCHAR out[_MAX_PATH] = {0};
		FILE* fpCcd = CreateOrOpenFileW(argv[4], out, NULL, NULL, _T(".ccd"), _T(WFLAG), 0, 0);
		if(!fpCcd) {
			OutputErrorString(_T("Failed to open file .ccd\n"));
			return FALSE;
		}
		bRet = ReadTOC(hDevice, &adr, &nLength, fpLog);
		if(!bRet) {
			return FALSE;
		}
		bRet = ReadTOCFull(hDevice, &adr, pszVendorId, pszProductId, bCanCDText, fpLog, fpCcd);
		if(!bRet) {
			return FALSE;
		}
		if(execType != rall) {
			fclose(fpCcd);
			_tremove(out);
		}

		INT nCombinedOffset = 0;
		BOOL bAudioOnly = TRUE;
		bRet = ReadCDForSearchingOffset(hDevice, &adr, pszVendorId, 
			pszProductId, &nCombinedOffset, &bAudioOnly, fpLog);

		if(execType == rd) {
			bRet = ReadCDPartial(hDevice, &adr, argv[4], pszVendorId,
				_ttoi(argv[5]), _ttoi(argv[6]), READ_CD_FLAG::All, bDC);
		}
		else if(execType == ra) {
			bRet = ReadCDPartial(hDevice, &adr, argv[4], pszVendorId, 
				_ttoi(argv[5]), _ttoi(argv[6]), READ_CD_FLAG::CDDA, bDC);
		}
		else if(bRet == TRUE && execType == rall) {
			bRet = ReadCDAll(hDevice, &adr, argv[4], pszVendorId, 
				nCombinedOffset, nLength, bC2ErrorData, bAudioOnly, fpLog, fpCcd);
			fclose(fpCcd);
		}
	}
	else if(usFeatureProfileType == ProfileDvdRom || 
		usFeatureProfileType == ProfileDvdRecordable ||
		usFeatureProfileType == ProfileDvdRam || 
		usFeatureProfileType == ProfileDvdRewritable || 
		usFeatureProfileType == ProfileDvdRWSequential || 
		usFeatureProfileType == ProfileDvdDashRDualLayer || 
		usFeatureProfileType == ProfileDvdDashRLayerJump || 
		usFeatureProfileType == ProfileDvdPlusRW || 
//		usFeatureProfileType == ProfileInvalid ||
		usFeatureProfileType == ProfileDvdPlusR) {
		INT nDVDSectorSize = 0;
		bRet = ReadDVDStructure(hDevice, &adr, &nDVDSectorSize, fpLog);
		if(bRet) {
			if(argv[5] && !_tcscmp(argv[5], _T("raw"))) {
#if 0
				bRet = ReadDVDRaw(hDevice, &adr, pszVendorId, argv[4], argv[5], nDVDSectorSize, fpLog);
#endif
			}
			else {
				bRet = ReadDVD(hDevice, &adr, argv[4], argv[5], nDVDSectorSize, fpLog);
			}
		}
	}
#ifndef _DEBUG
	fclose(fpLog);
#endif
	CloseHandle(hDevice);
	return bRet;
}

int checkArg(int argc, _TCHAR* argv[], ExecType* execType)
{
	if(argc == 1 ||
		(1 < argc && (_tcscmp(argv[1], _T("-rall")) &&
					_tcscmp(argv[1], _T("-rd")) &&
					_tcscmp(argv[1], _T("-ra")) &&
					_tcscmp(argv[1], _T("-c")) &&
					_tcscmp(argv[1], _T("-s")) &&
					_tcscmp(argv[1], _T("-dec")) &&
					_tcscmp(argv[1], _T("-split")) &&
					_tcscmp(argv[1], _T("-sub"))
					))) {
		return FALSE;
	}
	else if(!_tcscmp(argv[1], _T("-rall")) && argc < 5) {
		return FALSE;
	}
	else if(!_tcscmp(argv[1], _T("-rd")) && argc < 7) {
		return FALSE;
	}
	else if(!_tcscmp(argv[1], _T("-ra")) && argc < 7) {
		return FALSE;
	}
	else if(!_tcscmp(argv[1], _T("-c")) && argc < 3) {
		return FALSE;
	}
	else if(!_tcscmp(argv[1], _T("-s")) && argc < 3) {
		return FALSE;
	}
	else if(!_tcscmp(argv[1], _T("-dec")) && argc < 4) {
		return FALSE;
	}
	else if(!_tcscmp(argv[1], _T("-split")) && argc < 3) {
		return FALSE;
	}
	else if(!_tcscmp(argv[1], _T("-sub")) && argc < 3) {
		return FALSE;
	}
	TCHAR* endptr = NULL;
	ULONG ul = 0;
	LONG l = 0;
	if((argc == 5 || argc == 6) && !_tcscmp(argv[1], _T("-rall"))) {
		ul = _tcstoul(argv[3], &endptr, 10);
		if(*endptr) {
			return FALSE;
		}
		*execType = rall;
	}
	else if(argc == 7 && (!_tcscmp(argv[1], _T("-rd")) || !_tcscmp(argv[1], _T("-ra")))) {
		ul = _tcstoul(argv[3], &endptr, 10);
		if(*endptr) {
			return FALSE;
		}
		l = _tcstol(argv[5], &endptr, 10);
		if(*endptr) {
			return FALSE;
		}
		l = _tcstol(argv[6], &endptr, 10);
		if(*endptr) {
			return FALSE;
		}
		if(!_tcscmp(argv[1], _T("-rd"))) {
			*execType = rd;
		}
		else if(!_tcscmp(argv[1], _T("-ra"))) {
			*execType = ra;
		}
	}
	else if(argc == 3 && !_tcscmp(argv[1], _T("-c"))) {
		*execType = c;
	}
	else if(argc == 3 && !_tcscmp(argv[1], _T("-s"))) {
		*execType = s;
	}
	else if(argc == 4 && !_tcscmp(argv[1], _T("-dec"))) {
		l = _tcstol(argv[3], &endptr, 10);
		if(*endptr) {
			return FALSE;
		}
		*execType = dec;
	}
	else if(argc == 3 && !_tcscmp(argv[1], _T("-split"))) {
		*execType = split;
	}
	else if(argc == 3 && !_tcscmp(argv[1], _T("-sub"))) {
		*execType = sub;
	}
	else {
		return FALSE;
	}
	return TRUE;
}

int _tmain(int argc, _TCHAR* argv[])
{
	OutputString(_T("DiscImageCreator BuildDate:[%s %s]\n"), _T(__DATE__), _T(__TIME__));
	ExecType execType;
	if(!checkArg(argc, argv, &execType)) {
		OutputString(_T("Usage\n"));
#if 0
		OutputString(_T("\t-rall [DriveLetter] [DriveSpeed(0-72)] [filename] <cmi/raw>\n"));
#endif
		OutputString(_T("\t-rall [DriveLetter] [DriveSpeed(0-72)] [filename] <cmi>\n"));
		OutputString(_T("\t\tRipping CD or DVD from a to z\n"));
		OutputString(_T("\t\tcmi:Log Copyright Management Information (Only DVD)(Very slow)\n"));
#if 0
		OutputString(_T("\t\traw:Ripping Raw mode (Only DVD)\n"));
#endif
		OutputString(_T("\t-rd [DriveLetter] [DriveSpeed(0-72)] [filename] [StartLBA] [EndLBA]\n"));
		OutputString(_T("\t\tRipping CD from start to end (data) (Only CD)\n"));
		OutputString(_T("\t-ra [DriveLetter] [DriveSpeed(0-72)] [filename] [StartLBA] [EndLBA]\n"));
		OutputString(_T("\t\tRipping CD from start to end (audio) (Only CD)\n"));
		OutputString(_T("\t-c [DriveLetter]\n"));
		OutputString(_T("\t\tClose tray\n"));
		OutputString(_T("\t-s [DriveLetter]\n"));
		OutputString(_T("\t\tStop spin disc\n"));
		OutputString(_T("\t-dec [filename] [LBA]\n"));
		OutputString(_T("\t\tDescramble data sector (for GD-ROM Image)\n"));
		OutputString(_T("\t-split [filename]\n"));
		OutputString(_T("\t\tSplit descrambled File (for GD-ROM Image)\n"));
		OutputString(_T("\t-sub [subfile]\n"));
		OutputString(_T("\t\tParse CloneCD sub file\n"));
	}
	else {
		time_t now;
		struct tm* ts;
		_TCHAR buf[128] = {0};

		now = time(NULL);
		ts = localtime(&now);
		_tcsftime(buf, sizeof(buf), _T("%Y-%m-%d(%a) %H:%M:%S"), ts);
		OutputString(_T("Start->%s\n"), buf);
		BOOL bRet = exec(argv, execType);
		if(bRet) {
			Beep(440, 200);   // do
			Beep(494, 200);   // re
			Beep(554, 200);   // mi
			Beep(587, 200);   // fa
			Beep(659, 200);   // so
			Beep(740, 200);   // la
			Beep(830, 200);   // ti
			Beep(880, 200);   // do
		}
		else {
			Beep(880, 200);   // do
			Beep(830, 200);   // ti
			Beep(740, 200);   // la
			Beep(659, 200);   // so
			Beep(587, 200);   // fa
			Beep(554, 200);   // mi
			Beep(494, 200);   // re
			Beep(440, 200);   // do
		}
		now = time(NULL);
		ts = localtime(&now);
		_tcsftime(buf, sizeof(buf), _T("%Y-%m-%d(%a) %H:%M:%S"), ts);
		OutputString(_T("End->%s\n"), buf);
	}
	return 0;
}

