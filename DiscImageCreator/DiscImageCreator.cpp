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
	TCHAR szBuf[7];
	ZeroMemory(szBuf, sizeof(szBuf));
	_sntprintf(szBuf, 7, _T("\\\\.\\%c:"), argv[2][0]);
	HANDLE hDevice = CreateFile(szBuf, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(hDevice == INVALID_HANDLE_VALUE) {
		OutputErrorStringA("Device Open fail\n");
		return FALSE;
	}
	BOOL bRet = FALSE;
	if(execType == c) {
		return StartStop(hDevice, START_UNIT_CODE, START_UNIT_CODE);
	}
	else if(execType == s) {
		return StartStop(hDevice, STOP_UNIT_CODE, STOP_UNIT_CODE);
	}
	CHAR pszVendorId[8+1];
	CHAR pszProductId[16+1];
	ZeroMemory(pszVendorId, sizeof(pszVendorId));
	ZeroMemory(pszProductId, sizeof(pszProductId));
	FILE* fpLog = NULL;
#ifndef _DEBUG
	fpLog = CreateOrOpenFileW(argv[4], NULL, NULL, _T(".log.txt"), _T("w"), 0, 0);
	if(!fpLog) {
		OutputErrorStringA("Failed to open file .log.txt\n");
		return FALSE;
	}
#endif
	bRet = ReadDeviceInfo(hDevice, pszVendorId, pszProductId, fpLog);
	if(!bRet) {
		return FALSE;
	}
	pszVendorId[8] = '\0';
	pszProductId[16] = '\0';
	Init();
	USHORT usFeatureProfileType = ProfileInvalid;
	BOOL bCanCDText = FALSE;
	BOOL bC2ErrorData = FALSE;
	SetCDSpeed(hDevice, _ttoi(argv[3]), fpLog);
	ReadConfiguration(hDevice, &usFeatureProfileType, &bCanCDText, &bC2ErrorData, fpLog);
	if(usFeatureProfileType == ProfileCdrom || 
		usFeatureProfileType == ProfileCdRecordable ||
		usFeatureProfileType == ProfileCdRewritable ||
		(usFeatureProfileType == ProfileInvalid && (execType == ra))) {
		INT nLength = 0;
		TCHAR out[_MAX_PATH];
		ZeroMemory(out, sizeof(out));
		FILE* fpCcd = CreateOrOpenFileW(argv[4], out, NULL, _T(".ccd"), _T("w"), 0, 0);
		if(!fpCcd) {
			OutputErrorStringA("Failed to open file .ccd\n");
			return FALSE;
		}
		bRet = ReadTOC(hDevice, &nLength, fpLog);
		if(!bRet) {
			return FALSE;
		}
		bRet = ReadTOCFull(hDevice, bCanCDText, fpLog, fpCcd);
		if(!bRet) {
			return FALSE;
		}
		if(execType != rall) {
			fclose(fpCcd);
			_tremove(out);
		}

		INT nCombinedOffset = 0;
		bRet = ReadCDForSearchingOffset(hDevice, pszVendorId, 
			pszProductId, &nCombinedOffset, fpLog);

		if(execType == rd) {
			bRet = ReadCDPartial(hDevice, argv[4], pszVendorId,
				_ttoi(argv[5]), _ttoi(argv[6]), READ_CD_FLAG::All);
		}
		else if(execType == ra) {
			bRet = ReadCDPartial(hDevice, argv[4], pszVendorId, 
				_ttoi(argv[5]), _ttoi(argv[6]), READ_CD_FLAG::CDDA);
		}
		else if(bRet == TRUE && execType == rall) {
			bRet = ReadCDAll(hDevice, argv[4], pszVendorId, 
				nCombinedOffset, nLength, bC2ErrorData, fpLog, fpCcd);
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
		usFeatureProfileType == ProfileInvalid ||
		usFeatureProfileType == ProfileDvdPlusR) {
		INT nDVDSectorSize = 0;
		bRet = ReadDVDStructure(hDevice, &nDVDSectorSize, fpLog);
		if(bRet) {
			if(argv[5] && !_tcscmp(argv[5], _T("raw"))) {
#if 0
				bRet = ReadDVDRaw(hDevice, pszVendorId, argv[4], argv[5], nDVDSectorSize, fpLog);
#endif
			}
			else {
				bRet = ReadDVD(hDevice, argv[4], argv[5], nDVDSectorSize, fpLog);
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
	OutputStringA("DiscImageCreator BuildDate:[%s %s]\n", __DATE__, __TIME__);
	ExecType execType;
	if(!checkArg(argc, argv, &execType)) {
		OutputStringA("Usage\n");
#if 0
		OutputStringA("\t-rall [DriveLetter] [DriveSpeed(0-72)] [filename] <cmi/raw>\n");
#endif
		OutputStringA("\t-rall [DriveLetter] [DriveSpeed(0-72)] [filename] <cmi>\n");
		OutputStringA("\t\tRipping CD or DVD from a to z\n");
		OutputStringA("\t\tcmi:Log Copyright Management Information (Only DVD)(Very slow)\n");
#if 0
		OutputStringA("\t\traw:Ripping Raw mode (Only DVD)\n");
#endif
		OutputStringA("\t-rd [DriveLetter] [DriveSpeed(0-72)] [filename] [StartLBA] [EndLBA]\n");
		OutputStringA("\t\tRipping CD from start to end (data) (Only CD)\n");
		OutputStringA("\t-ra [DriveLetter] [DriveSpeed(0-72)] [filename] [StartLBA] [EndLBA]\n");
		OutputStringA("\t\tRipping CD from start to end (audio) (Only CD)\n");
		OutputStringA("\t-c [DriveLetter]\n");
		OutputStringA("\t\tClose tray\n");
		OutputStringA("\t-s [DriveLetter]\n");
		OutputStringA("\t\tStop spin disc\n");
		OutputStringA("\t-dec [filename] [LBA]\n");
		OutputStringA("\t\tDescramble data sector (for GD-ROM Image)\n");
		OutputStringA("\t-split [filename]\n");
		OutputStringA("\t\tSplit descrambled File (for GD-ROM Image)\n");
		OutputStringA("\t-sub [subfile]\n");
		OutputStringA("\t\tParse CloneCD sub file\n");
	}
	else {
		time_t now;
		struct tm* ts;
		char buf[80];
		memset(buf, 0, sizeof(buf));
		now = time(NULL);
		ts = localtime(&now);
		strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);
		printf("Start->%s\n", buf);
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
		strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);
		printf("End->%s\n", buf);
	}
	return 0;
}

