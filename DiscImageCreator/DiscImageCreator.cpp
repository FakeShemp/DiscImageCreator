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
	f,
	c,
	s,
	dec,
	split,
	sub,
} ExecType;

int exec(_TCHAR* argv[], ExecType execType, ExtArg* extArg)
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
		WriteParsingSubfile(argv[2]);
		return TRUE;
	}

	TCHAR szBuf[7] = {0};
	_sntprintf(szBuf, 7, _T("\\\\.\\%c:"), argv[2][0]);
	DEVICE_DATA devData = {0};
	devData.hDevice = CreateFile(szBuf, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(devData.hDevice == INVALID_HANDLE_VALUE) {
		OutputErrorString(_T("Device Open fail\n"));
		return FALSE;
	}

	BOOL bRet = TRUE;
	if(execType == c) {
		StartStop(&devData, START_UNIT_CODE, START_UNIT_CODE);
	}
	else if(execType == s) {
		StartStop(&devData, STOP_UNIT_CODE, STOP_UNIT_CODE);
	}
	else {
		LogFile logFile = {0};
		DISC_DATA discData = {0};
		FILE* fpCcd = NULL;
		try {
			bRet = ReadTestUnitReady(&devData);
			if(!bRet) {
				throw FALSE;
			}

			BOOL bDC = FALSE;
			if(execType == ra && !_tcscmp(argv[5], _T("44990")) && !_tcscmp(argv[6], _T("549150"))) {
				bDC = TRUE;
			}
#ifndef _DEBUG
			_TCHAR szDriveLogtxt[32] = {0};
			_TCHAR szDiscLogtxt[32] = {0};
			_TCHAR szErrorLogtxt[32] = {0};
			if(bDC) {
				_tcscpy(szDiscLogtxt, _T("_dc_disclog.txt"));
				_tcscpy(szDriveLogtxt, _T("_dc_drivelog.txt"));
				_tcscpy(szErrorLogtxt, _T("_dc_errorlog.txt"));
			}
			else if(execType == f) {
				_tcscpy(szDiscLogtxt, _T("_fd_disclog.txt"));
			}
			else {
				_tcscpy(szDiscLogtxt, _T("_disclog.txt"));
				_tcscpy(szDriveLogtxt, _T("_drivelog.txt"));
				_tcscpy(szErrorLogtxt, _T("_errorlog.txt"));
			}
			logFile.fpDiscLog = CreateOrOpenFileW(argv[4], NULL, NULL, NULL, szDiscLogtxt, _T(WFLAG), 0, 0);
			if(!logFile.fpDiscLog) {
				OutputErrorString(_T("Failed to open file: %s\n"), szDiscLogtxt);
				throw FALSE;
			}
			if(execType != f) {
				logFile.fpDriveLog = CreateOrOpenFileW(argv[4], NULL, NULL, NULL, szDriveLogtxt, _T(WFLAG), 0, 0);
				if(!logFile.fpDriveLog) {
					OutputErrorString(_T("Failed to open file: %s\n"), szDriveLogtxt);
					throw FALSE;
				}
				logFile.fpErrorLog = CreateOrOpenFileW(argv[4], NULL, NULL, NULL, szErrorLogtxt, _T(WFLAG), 0, 0);
				if(!logFile.fpErrorLog) {
					OutputErrorString(_T("Failed to open file: %s\n"), szErrorLogtxt);
					throw FALSE;
				}
			}
#endif
			if(execType == f) {
				ReadFloppy(&devData, argv[4], logFile.fpDiscLog);
			}
			else {
				ReadScsiGetAddress(&devData, logFile.fpDriveLog);
				ReadStorageQueryProperty(&devData, logFile.fpDriveLog);
				bRet = ReadInquiryData(&devData, logFile.fpDriveLog);
				if(!bRet) {
					throw FALSE;
				}
				devData.pszVendorId[8] = '\0';
				devData.pszProductId[16] = '\0';

				IsPlextorDrive(&devData);
				ReadBufferCapacity(&devData, logFile.fpDriveLog);
				SetCDSpeed(&devData, _ttoi(argv[3]), logFile.fpDriveLog);
				bRet = ReadConfiguration(&devData, &discData, logFile.fpDriveLog);
				if(!bRet) {
					throw FALSE;
				}

				bRet = ReadDiscInformation(&devData, logFile.fpDiscLog);
				if(!bRet) {
					throw FALSE;
				}
				bRet = ReadTOC(&devData, &discData, logFile.fpDiscLog);
				if(!bRet) {
					throw FALSE;
				}
				if(discData.usCurrentMedia == ProfileCdrom || 
					discData.usCurrentMedia == ProfileCdRecordable ||
					discData.usCurrentMedia == ProfileCdRewritable ||
					(discData.usCurrentMedia == ProfileInvalid && (execType == ra))) {
					TCHAR out[_MAX_PATH] = {0};
					if(execType == rall) {
						fpCcd = CreateOrOpenFileW(argv[4], out, NULL, NULL, _T(".ccd"), _T(WFLAG), 0, 0);
						if(!fpCcd) {
							OutputErrorString(_T("Failed to open file .ccd\n"));
							throw FALSE;
						}
					}
					if(&devData.bCanCDText) {
						size_t dwTrackAllocSize = (size_t)discData.toc.LastTrack + 1;
						if(NULL == (discData.aSessionNum = (PUINT)calloc(dwTrackAllocSize, sizeof(_INT)))) {
							throw _T("Failed to alloc memory discData.aSessionNum\n");
						}
						if(NULL == (discData.szISRC = (_TCHAR**)calloc(dwTrackAllocSize, sizeof(_INT)))) {
							throw _T("Failed to alloc memory discData.szISRC\n");
						}
						if(NULL == (discData.szTitle = (_TCHAR**)calloc(dwTrackAllocSize, sizeof(_INT)))) {
							throw _T("Failed to alloc memory discData.szTitle\n");
						}
						if(NULL == (discData.szPerformer = (_TCHAR**)calloc(dwTrackAllocSize, sizeof(_INT)))) {
							throw _T("Failed to alloc memory discData.szPerformer\n");
						}
						if(NULL == (discData.szSongWriter = (_TCHAR**)calloc(dwTrackAllocSize, sizeof(_INT)))) {
							throw _T("Failed to alloc memory discData.szTitle\n");
						}

						size_t isrcSize = META_ISRC_SIZE + 1;
						size_t textSize = META_CDTEXT_SIZE + 1;
						for(INT h = 0; h < discData.toc.LastTrack + 1; h++) {
							if(NULL == (discData.szISRC[h] = (_TCHAR*)calloc(isrcSize, sizeof(_TCHAR)))) {
								throw _T("Failed to alloc memory discData.szISRC[h]\n");
							}
							if(NULL == (discData.szTitle[h] = (_TCHAR*)calloc(textSize, sizeof(_TCHAR)))) {
								throw _T("Failed to alloc memory discData.szTitle[h]\n");
							}
							if(NULL == (discData.szPerformer[h] = (_TCHAR*)calloc(textSize, sizeof(_TCHAR)))) {
								throw _T("Failed to alloc memory discData.szPerformer[h]\n");
							}
							if(NULL == (discData.szSongWriter[h] = (_TCHAR*)calloc(textSize, sizeof(_TCHAR)))) {
								throw _T("Failed to alloc memory discData.szSongWriter[h]\n");
							}
						}
					}
					bRet = ReadTOCFull(&devData, &discData, logFile.fpDiscLog, fpCcd);
					if(!bRet) {
						throw FALSE;
					}
					bRet = ReadCDForSearchingOffset(&devData, &discData, logFile.fpDiscLog);

					if(execType == rd) {
						bRet = ReadCDPartial(&devData, &discData, argv[4],
							_ttoi(argv[5]), _ttoi(argv[6]), READ_CD_FLAG::All, bDC, FALSE);
					}
					else if(execType == ra) {
						bRet = ReadCDPartial(&devData, &discData, argv[4],
							_ttoi(argv[5]), _ttoi(argv[6]), READ_CD_FLAG::CDDA, bDC, FALSE);
					}
					else if(bRet == TRUE && execType == rall) {
						bRet = ReadCDAll(&devData, &discData, argv[4], extArg, &logFile, fpCcd);
					}
				}
				else if(discData.usCurrentMedia == ProfileDvdRom || 
					discData.usCurrentMedia == ProfileDvdRecordable ||
					discData.usCurrentMedia == ProfileDvdRam || 
					discData.usCurrentMedia == ProfileDvdRewritable || 
					discData.usCurrentMedia == ProfileDvdRWSequential || 
					discData.usCurrentMedia == ProfileDvdDashRDualLayer || 
					discData.usCurrentMedia == ProfileDvdDashRLayerJump || 
					discData.usCurrentMedia == ProfileDvdPlusRW || 
//					discData.usCurrentMedia == ProfileInvalid ||
					discData.usCurrentMedia == ProfileDvdPlusR) {
					bRet = ReadDVDStructure(&devData, &discData, logFile.fpDiscLog);
					if(bRet) {
						if(argv[5] && !_tcscmp(argv[5], _T("raw"))) {
#if 0
							bRet = ReadDVDRaw(&devData, &discData, pszVendorId, argv[4]);
#endif
						}
						else {
							bRet = ReadDVD(&devData, &discData, argv[4], extArg, logFile.fpDiscLog);
						}
					}
				}
			}
		}
		catch(BOOL bErr) {
			bRet = bErr;
		}
		FreeAndNull(discData.aSessionNum);
		if(&devData.bCanCDText) {
			for(INT i = 0; i < discData.toc.LastTrack + 1; i++) {
				if(discData.szISRC) {
					FreeAndNull(discData.szISRC[i]);
				}
				if(discData.szTitle) {
					FreeAndNull(discData.szTitle[i]);
				}
				if(discData.szPerformer) {
					FreeAndNull(discData.szPerformer[i]);
				}
				if(discData.szSongWriter) {
					FreeAndNull(discData.szSongWriter[i]);
				}
			}
			FreeAndNull(discData.szISRC);
			FreeAndNull(discData.szTitle);
			FreeAndNull(discData.szPerformer);
			FreeAndNull(discData.szSongWriter);
		}
		FcloseAndNull(fpCcd);
#ifndef _DEBUG
		FcloseAndNull(logFile.fpDiscLog);
		if(execType != f) {
			FcloseAndNull(logFile.fpDriveLog);
			FcloseAndNull(logFile.fpErrorLog);
		}
#endif
	}
	CloseHandle(devData.hDevice);
	return bRet;
}

int printArg(_TCHAR* argv)
{
	TCHAR drive[_MAX_DRIVE] = {0};
	TCHAR dir[_MAX_DIR] = {0};
	TCHAR fname[_MAX_FNAME] = {0};
	TCHAR ext[_MAX_EXT] = {0};
	_tsplitpath(argv, drive, dir, fname, ext);
	OutputString(
		_T("Input File Name\n")
		_T("\t path: %s\n")
		_T("\tdrive: %s\n")
		_T("\t  dir: %s\n")
		_T("\tfname: %s\n")
		_T("\t  ext: %s\n"),
		argv, drive, dir, fname, ext);
	return 1;
}

int checkArg(int argc, _TCHAR* argv[], ExecType* execType, ExtArg* extArg)
{
	if(argc == 1 ||
		(1 < argc && (_tcscmp(argv[1], _T("-rall")) &&
					_tcscmp(argv[1], _T("-rd")) &&
					_tcscmp(argv[1], _T("-ra")) &&
					_tcscmp(argv[1], _T("-f")) &&
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
	else if(!_tcscmp(argv[1], _T("-f")) && argc < 5) {
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
	if(argc >= 5 && !_tcscmp(argv[1], _T("-rall"))) {
		ul = _tcstoul(argv[3], &endptr, 10);
		if(*endptr) {
			return FALSE;
		}
		*execType = rall;
		printArg(argv[4]);

		for(INT i = 6; i <= argc; i++) {
			if(!_tcscmp(argv[i-1], _T("pre"))) {
				extArg->bPre = TRUE;
			}
			else if(!_tcscmp(argv[i-1], _T("c2"))) {
				extArg->bC2 = TRUE;
			}
			else if(!_tcscmp(argv[i-1], _T("isrc"))) {
				extArg->bIsrc = TRUE;
			}
			else if(!_tcscmp(argv[i-1], _T("cmi"))) {
				extArg->bCmi = TRUE;
			}
		}
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
			printArg(argv[4]);
		}
		else if(!_tcscmp(argv[1], _T("-ra"))) {
			*execType = ra;
			printArg(argv[4]);
		}
	}
	else if(argc == 5 && !_tcscmp(argv[1], _T("-f"))) {
		*execType = f;
		printArg(argv[4]);
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
		printArg(argv[2]);
	}
	else if(argc == 3 && !_tcscmp(argv[1], _T("-split"))) {
		*execType = split;
		printArg(argv[2]);
	}
	else if(argc == 3 && !_tcscmp(argv[1], _T("-sub"))) {
		*execType = sub;
		printArg(argv[2]);
	}
	else {
		return FALSE;
	}
	return TRUE;
}

int _tmain(int argc, _TCHAR* argv[])
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	OSVERSIONINFO OSver;
	OSver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&OSver);
	OutputString(
		_T("OS\n")
		_T("\tMajorVersion: %d, MinorVersion: %d, BuildNumber: %d\n"),
		OSver.dwMajorVersion, OSver.dwMinorVersion, OSver.dwBuildNumber);

	OutputString(_T("AppVersion\n"));
#ifdef WIN64
	OutputString(_T("\tx64, "));
#else
	OutputString(_T("\tx86, "));
#endif
#ifdef UNICODE
	OutputString(_T("Unicode build\n"));
#else
	OutputString(_T("Ansi build\n"));
#endif
	OutputString(
		_T("BuildDate\n")
		_T("\t%s %s\n"), _T(__DATE__), _T(__TIME__));
	ExecType execType;
	ExtArg extArg = {0};
	if(!checkArg(argc, argv, &execType, &extArg)) {
		OutputString(
			_T("Usage\n")
			_T("\t-rall [DriveLetter] [DriveSpeed(0-72)] [filename] <pre/c2/isrc/cmi>\n")
			_T("\t\tRipping CD or DVD from a to z\n")
			_T("\t\tpre: If index0 exists in track1, ripping from LBA-150 to LBA-1\n")
			_T("\t\t\tfor SagaFrontier Original Sound Track (Disc 3)\n")
			_T("\t\tc2: Check C2 error (Only CD)\n")
			_T("\t\t\tTake twice as long\n")
			_T("\t\tisrc: Ignore invalid ISRC (Only CD)\n")
			_T("\t\t\tfor Valis II[PCE]\n")
			_T("\t\tcmi: Log Copyright Management Information (Only DVD)\n")
			_T("\t\t\tVery slow\n")
//			_T("\t\traw:Ripping Raw mode (Only DVD)\n")
			_T("\t-rd [DriveLetter] [DriveSpeed(0-72)] [filename] [StartLBA] [EndLBA]\n")
			_T("\t\tRipping CD from start to end (data) (Only CD)\n")
			_T("\t-ra [DriveLetter] [DriveSpeed(0-72)] [filename] [StartLBA] [EndLBA]\n")
			_T("\t\tRipping CD from start to end (audio) (Only CD)\n")
			_T("\t-f [DriveLetter] [DriveSpeed(0-72)] [filename]\n")
			_T("\t\tRipping floppy\n")
			_T("\t-c [DriveLetter]\n")
			_T("\t\tClose tray\n")
			_T("\t-s [DriveLetter]\n")
			_T("\t\tStop spin disc\n")
			_T("\t-dec [filename] [LBA]\n")
			_T("\t\tDescramble data sector (for GD-ROM Image)\n")
			_T("\t-split [filename]\n")
			_T("\t\tSplit descrambled File (for GD-ROM Image)\n")
			_T("\t-sub [subfile]\n")
			_T("\t\tParse CloneCD sub file\n"));
	}
	else {
		time_t now;
		struct tm* ts;
		_TCHAR buf[128] = {0};

		now = time(NULL);
		ts = localtime(&now);
		_tcsftime(buf, sizeof(buf) / sizeof(buf[0]), _T("%Y-%m-%d(%a) %H:%M:%S"), ts);
		OutputString(_T("Start -> %s\n"), buf);
		BOOL bRet = exec(argv, execType, &extArg);
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
		_tcsftime(buf, sizeof(buf) / sizeof(buf[0]), _T("%Y-%m-%d(%a) %H:%M:%S"), ts);
		OutputString(_T("End -> %s\n"), buf);
	}
	return 0;
}

