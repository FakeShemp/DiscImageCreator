// DiscImageCreator.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//
/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"
#include "struct.h"
#include "calcHash.h"
#include "check.h"
#include "execIoctl.h"
#include "execMMC.h"
#include "execMMCforCD.h"
#include "execMMCforDVD.h"
#include "get.h"
#include "init.h"
#include "output.h"
#if 0
#include "outputGD.h"
#endif

#define DEFAULT_REREAD_VAL			(1024)
#define DEFAULT_MAX_C2_ERROR_VAL	(32768)
#define DEFAULT_REREAD_SPEED_VAL	(4)

// These global variable is set at pringAndSetArg().
_TCHAR g_szCurrentdir[_MAX_PATH];
_TCHAR g_drive[_MAX_DRIVE];
_TCHAR g_dir[_MAX_DIR];
_TCHAR g_fname[_MAX_FNAME];
_TCHAR g_ext[_MAX_EXT];

int processOutputHash(_TCHAR* szFullPath)
{
	HANDLE h = NULL;
	WIN32_FIND_DATA lp = { 0 };
	_TCHAR szPathWithoutFileName[_MAX_PATH] = { 0 };
	BOOL bRet = GetCreatedFileList(&h, &lp, szPathWithoutFileName);
	if (!bRet) {
		return FALSE;
	}
	/* Table of CRCs of all 8-bit messages. */
	UINT crc_table[256] = { 0 };
	make_crc_table(crc_table);
	FILE* fpHash = CreateOrOpenFileW(szFullPath, NULL, NULL, NULL, _T(".dat"), _T(WFLAG), 0, 0);
	if (!fpHash) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		FindClose(h);
		return FALSE;
	}

	_TCHAR szDstPath[_MAX_PATH] = { 0 };
	_TCHAR ext[_MAX_EXT] = { 0 };
	do {
		if ((lp.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY) {
			_tsplitpath(lp.cFileName, NULL, NULL, NULL, ext);
			if (!_tcsncmp(g_fname, lp.cFileName, _tcslen(g_fname)) &&
				(!_tcscmp(ext, _T(".bin")) || !_tcscmp(ext, _T(".iso")))) {
				_stprintf(szDstPath, _T("%s%s"), szPathWithoutFileName, lp.cFileName);
				FILE* fp = CreateOrOpenFileW(szDstPath, NULL, NULL, NULL, ext, _T("rb"), 0, 0);
				if (!fp) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					break;
				}
				UINT64 ui64Filesize = GetFileSize64(0, fp);
				DWORD dwSectorSizeOne = (DWORD)(
					!_tcscmp(ext, _T(".bin")) ? CD_RAW_SECTOR_SIZE : DISC_RAW_READ);
				UINT64 ui64SectorSizeAll = ui64Filesize / (UINT64)dwSectorSizeOne;
					
				if (ui64Filesize > dwSectorSizeOne * 10) {
					MD5_CTX context = { 0 };
					SHA1Context sha = { 0 };
					CalcInit(&context, &sha);
						
					BYTE data[CD_RAW_SECTOR_SIZE] = { 0 };
					DWORD crc32 = 0;
					OutputString(_T("\rCalculating hash. %s"), szDstPath);
					// TODO: This code can more speed up! if reduce calling fread()
					for (DWORD i = 0; i < ui64SectorSizeAll; i++) {
						fread(data, sizeof(BYTE), dwSectorSizeOne, fp);
						bRet = CalcHash(crc_table, &crc32, &context, &sha, data, dwSectorSizeOne);
						if (!bRet) {
							break;
						}
					}
					if (!bRet) {
						break;
					}
					BYTE digest[16] = { 0 };
					BYTE Message_Digest[20] = { 0 };
					CalcEnd(&context, &sha, digest, Message_Digest);
					OutputHashData(fpHash, lp.cFileName, ui64Filesize, crc32, digest, Message_Digest);
				}
			}
		}
	} while(FindNextFile(h, &lp));
	OutputString(_T("\n"));
	FcloseAndNull(fpHash);
	FindClose(h);
	return bRet;
}

int exec(_TCHAR* argv[], PEXEC_TYPE pExecType, PEXT_ARG pExtArg)
{
	BOOL bRet = TRUE;
	SetLastError(NO_ERROR);
	_TCHAR* endptr = NULL;
	if (*pExecType == sub) {
		bRet = WriteParsingSubfile(argv[2]);
	}
	else {
		_TCHAR szBuf[8] = { 0 };
		_sntprintf(szBuf, 7, _T("\\\\.\\%c:"), argv[2][0]);
		szBuf[7] = 0;
		DEVICE_DATA devData = { 0 };
		devData.hDevice = CreateFile(szBuf, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (devData.hDevice == INVALID_HANDLE_VALUE) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
#if 0
		DWORD dwSize = 0;
		GET_LENGTH_INFORMATION tLenInf;
		DeviceIoControl(devData.hDevice, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &tLenInf, sizeof(tLenInf), &dwSize, NULL );
		OutputString(_T("%llx\n"), tLenInf.Length.QuadPart);
#define LODWORD(l)           ((DWORD)(((UINT64)(l)) & 0xffffffff))
#define HIDWORD(l)           ((DWORD)((((UINT64)(l)) >> 32) & 0xffffffff))
		OutputString(_T("%x\n"), LODWORD(tLenInf.Length.QuadPart));
		OutputString(_T("%x\n"), HIDWORD(tLenInf.Length.QuadPart));
		if (!LockFile(devData.hDevice, 0, 0, 0, 0)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
#endif
		if (*pExecType == c) {
			bRet = StartStopUnit(&devData, START_UNIT_CODE, START_UNIT_CODE);
		}
		else if (*pExecType == s) {
			bRet = StartStopUnit(&devData, STOP_UNIT_CODE, STOP_UNIT_CODE);
		}
		else {
			DISC_DATA discData = { 0 };
			PDISC_DATA pDiscData = &discData;
			FILE* fpCcd = NULL;
			try {
				if (!TestUnitReady(&devData)) {
					throw FALSE;
				}
#ifndef _DEBUG
				if (!InitLogFile(pExecType, argv[4])) {
					throw FALSE;
				}
#endif
				if (*pExecType == f) {
					if (!DiskGetMediaTypes(&devData, argv[4])) {
						throw FALSE;
					}
				}
				else {
					if (!ScsiGetAddress(&devData)) {
						throw FALSE;
					}
					if (!StorageQueryProperty(&devData)) {
						throw FALSE;
					}
					if (!Inquiry(&devData)) {
						throw FALSE;
					}

					IsPlextorDrive(&devData);
					if (!ReadBufferCapacity(&devData)) {
						throw FALSE;
					}
					DWORD dwSpeed = _tcstoul(argv[3], &endptr, 10);
					if (*endptr) {
						OutputErrorString(_T("Bad arg: %s. Please integer\n"), endptr);
						throw FALSE;
					}
					if (!SetCDSpeed(&devData, dwSpeed)) {
						throw FALSE;
					}
					if (!GetConfiguration(&devData, &discData)) {
						throw FALSE;
					}
					if (!ReadDiscInformation(&devData)) {
						throw FALSE;
					}
					if (!ReadTOC(pExecType, &devData, &discData)) {
						throw FALSE;
					}
					if (discData.wCurrentMedia == ProfileCdrom || 
						discData.wCurrentMedia == ProfileCdRecordable ||
						discData.wCurrentMedia == ProfileCdRewritable ||
						(discData.wCurrentMedia == ProfileInvalid && (*pExecType == rgd))) {
						_TCHAR out[_MAX_PATH] = { 0 };
						if (*pExecType == rall) {
							fpCcd = CreateOrOpenFileW(argv[4], out, NULL, NULL, _T(".ccd"), _T(WFLAG), 0, 0);
							if (!fpCcd) {
								OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
								throw FALSE;
							}
						}
						if (!InitTocFullData(pExecType, &devData, &pDiscData)) {
							throw FALSE;
						}
						if (!ReadTOCFull(&devData, &discData, fpCcd)) {
							throw FALSE;
						}
						bRet = ReadCDForSearchingOffset(pExecType, pExtArg, &devData, &discData);
#ifndef _DEBUG
						FlushLog();
#endif
						if (bRet) {
							if (*pExecType == rd || *pExecType == ra) {
								INT nStartLBA = _tcstol(argv[5], &endptr, 10);
								if (*endptr) {
									OutputErrorString(_T("Bad arg: %s. Please integer\n"), endptr);
									throw FALSE;
								}
								INT nEndLBA = _tcstol(argv[6], &endptr, 10);
								if (*endptr) {
									OutputErrorString(_T("Bad arg: %s. Please integer\n"), endptr);
									throw FALSE;
								}
								if (*pExecType == rd) {
									bRet = ReadCDPartial(pExecType, pExtArg, &devData, &discData, argv[4],
										nStartLBA, nEndLBA, READ_CD_FLAG::All, FALSE);
								}
								else if (*pExecType == ra) {
									bRet = ReadCDPartial(pExecType, pExtArg, &devData, &discData, argv[4],
										nStartLBA, nEndLBA, READ_CD_FLAG::CDDA, FALSE);
								}
							}
							else if (*pExecType == rall) {
								bRet = ReadCDAll(pExecType, pExtArg, &devData, &discData, argv[4], fpCcd);
							}
							else if (*pExecType == rgd) {
								bRet = ReadCDPartial(pExecType, pExtArg, &devData, &discData, argv[4],
									45000, 549149 + 1, READ_CD_FLAG::CDDA, FALSE);
							}
						}
					}
					else if (discData.wCurrentMedia == ProfileDvdRom || 
						discData.wCurrentMedia == ProfileDvdRecordable ||
						discData.wCurrentMedia == ProfileDvdRam || 
						discData.wCurrentMedia == ProfileDvdRewritable || 
						discData.wCurrentMedia == ProfileDvdRWSequential || 
						discData.wCurrentMedia == ProfileDvdDashRDualLayer || 
						discData.wCurrentMedia == ProfileDvdDashRLayerJump || 
						discData.wCurrentMedia == ProfileDvdPlusRW || 
//						discData.wCurrentMedia == ProfileInvalid ||
						discData.wCurrentMedia == ProfileDvdPlusR) {
						bRet = ReadDVDStructure(&devData, &discData);
						if (bRet) {
							if (argv[5] && !_tcscmp(argv[5], _T("raw"))) {
#if 0
								bRet = ReadDVDRaw(&devData, &discData, szVendorId, argv[4]);
#endif
							}
							else {
								bRet = ReadDVD(&devData, &discData, pExtArg, argv[4]);
							}
						}
					}
				}
			}
			catch(BOOL bErr) {
				bRet = bErr;
			}
			FcloseAndNull(fpCcd);
			TerminateTocData(&pDiscData);
			TerminateTocFullData(pExecType, &devData, &pDiscData);
#ifndef _DEBUG
			TerminateLogFile(pExecType);
#endif
		}
#if 0
		if (!UnlockFile(devData.hDevice, 0, 0, 0, 0)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
#endif
		if (devData.hDevice && !CloseHandle(devData.hDevice)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}
	if (bRet && (*pExecType == rgd || *pExecType == rall)) {
		bRet = processOutputHash(argv[4]);
	}
	return bRet;
}

int printSeveralInfo()
{
	OSVERSIONINFO OSver;
	OSver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (!GetVersionEx(&OSver)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	OutputString(
		_T("OS\n")
		_T("\tMajorVersion: %d, MinorVersion: %d, BuildNumber: %d\n")
		_T("\tPlatformId: %d, CSDVersion: %s\n"),
		OSver.dwMajorVersion, OSver.dwMinorVersion, OSver.dwBuildNumber, 
		OSver.dwPlatformId, OSver.szCSDVersion);

	OutputString(_T("AppVersion\n"));
#ifdef _WIN64
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
	return TRUE;
}

int printAndSetArg(_TCHAR* szFullPath)
{
	if (!GetCurrentDirectory(_MAX_PATH, g_szCurrentdir)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	_tsplitpath(szFullPath, g_drive, g_dir, g_fname, g_ext);
	OutputString(
		_T("Current dir\n")
		_T("\t%s\n")
		_T("Input File Name\n")
		_T("\t path: %s\n")
		_T("\tdrive: %s\n")
		_T("\t  dir: %s\n")
		_T("\tfname: %s\n")
		_T("\t  ext: %s\n"),
		g_szCurrentdir, szFullPath, g_drive, g_dir, g_fname, g_ext);
	return TRUE;
}

int checkArg(int argc, _TCHAR* argv[], PEXEC_TYPE pExecType, PEXT_ARG pExtArg)
{
	if (argc == 1 ||
		(1 < argc && (_tcscmp(argv[1], _T("-rall")) &&
					_tcscmp(argv[1], _T("-rgd")) &&
					_tcscmp(argv[1], _T("-rd")) &&
					_tcscmp(argv[1], _T("-ra")) &&
					_tcscmp(argv[1], _T("-f")) &&
					_tcscmp(argv[1], _T("-c")) &&
					_tcscmp(argv[1], _T("-s")) &&
#if 0
					_tcscmp(argv[1], _T("-dec")) &&
					_tcscmp(argv[1], _T("-split")) &&
#endif
					_tcscmp(argv[1], _T("-sub"))
					))) {
		return FALSE;
	}
	else if (!_tcscmp(argv[1], _T("-rall")) && argc < 5) {
		return FALSE;
	}
	else if (!_tcscmp(argv[1], _T("-rgd")) && argc < 5) {
		return FALSE;
	}
	else if (!_tcscmp(argv[1], _T("-rd")) && argc < 7) {
		return FALSE;
	}
	else if (!_tcscmp(argv[1], _T("-ra")) && argc < 7) {
		return FALSE;
	}
	else if (!_tcscmp(argv[1], _T("-f")) && argc < 5) {
		return FALSE;
	}
	else if (!_tcscmp(argv[1], _T("-c")) && argc < 3) {
		return FALSE;
	}
	else if (!_tcscmp(argv[1], _T("-s")) && argc < 3) {
		return FALSE;
	}
#if 0
	else if (!_tcscmp(argv[1], _T("-dec")) && argc < 4) {
		return FALSE;
	}
	else if (!_tcscmp(argv[1], _T("-split")) && argc < 3) {
		return FALSE;
	}
#endif
	else if (!_tcscmp(argv[1], _T("-sub")) && argc < 3) {
		return FALSE;
	}
	_TCHAR* endptr = NULL;
	DWORD dwNum = 0;
	LONG lNum = 0;
	if (argc >= 5 && !_tcscmp(argv[1], _T("-rall"))) {
		dwNum = _tcstoul(argv[3], &endptr, 10);
		if (*endptr) {
			return FALSE;
		}
		*pExecType = rall;
		printAndSetArg(argv[4]);

		for (INT i = 6; i <= argc; i++) {
			if (!_tcscmp(argv[i - 1], _T("add"))) {
				pExtArg->bAdd = TRUE;
				if (argc > 6) {
					lNum = _tcstol(argv[i], &endptr, 10);
					if (*endptr) {
						return FALSE;
					}
					pExtArg->nAudioCDOffsetNum = lNum;
				}
			}
			else if (!_tcscmp(argv[i - 1], _T("c2"))) {
				pExtArg->bC2 = TRUE;
				if (argc > 6) {
					pExtArg->uiMaxRereadNum = _tcstoul(argv[i], &endptr, 10);
					if (*endptr) {
						pExtArg->uiMaxRereadNum = DEFAULT_REREAD_VAL;
					}
				}
				else {
					pExtArg->uiMaxRereadNum = DEFAULT_REREAD_VAL;
				}
				if (argc > 7) {
					pExtArg->uiMaxC2ErrorNum = _tcstoul(argv[i + 1], &endptr, 10);
					if (*endptr) {
						pExtArg->uiMaxC2ErrorNum = DEFAULT_MAX_C2_ERROR_VAL;
					}
				}
				else {
					pExtArg->uiMaxC2ErrorNum = DEFAULT_MAX_C2_ERROR_VAL;
				}
				if (argc > 8) {
					pExtArg->uiRereadSpeedNum = _tcstoul(argv[i + 2], &endptr, 10);
					if (*endptr) {
						pExtArg->uiRereadSpeedNum = DEFAULT_REREAD_SPEED_VAL;
					}
				}
				else {
					pExtArg->uiRereadSpeedNum = DEFAULT_REREAD_SPEED_VAL;
				}
			}
			else if (!_tcscmp(argv[i - 1], _T("cmi"))) {
				pExtArg->bCmi = TRUE;
			}
			else if (!_tcscmp(argv[i - 1], _T("fua"))) {
				pExtArg->bFua = TRUE;
			}
			else if (!_tcscmp(argv[i - 1], _T("isrc"))) {
				pExtArg->bIsrc = TRUE;
			}
			else if (!_tcscmp(argv[i - 1], _T("pre"))) {
				pExtArg->bPre = TRUE;
			}
		}
	}
	else if (argc >= 5 && !_tcscmp(argv[1], _T("-rgd"))) {
		*pExecType = rgd;
		printAndSetArg(argv[4]);
		for (INT i = 6; i <= argc; i++) {
			if (!_tcscmp(argv[i - 1], _T("c2"))) {
				pExtArg->bC2 = TRUE;
				if (argc > 6) {
					pExtArg->uiMaxRereadNum = _tcstoul(argv[i], &endptr, 10);
					if (*endptr) {
						pExtArg->uiMaxRereadNum = DEFAULT_REREAD_VAL;
					}
				}
				else {
					pExtArg->uiMaxRereadNum = DEFAULT_REREAD_VAL;
				}
				if (argc > 7) {
					pExtArg->uiMaxC2ErrorNum = _tcstoul(argv[i + 1], &endptr, 10);
					if (*endptr) {
						pExtArg->uiMaxC2ErrorNum = DEFAULT_MAX_C2_ERROR_VAL;
					}
				}
				else {
					pExtArg->uiMaxC2ErrorNum = DEFAULT_MAX_C2_ERROR_VAL;
				}
				if (argc > 8) {
					pExtArg->uiRereadSpeedNum = _tcstoul(argv[i + 2], &endptr, 10);
					if (*endptr) {
						pExtArg->uiRereadSpeedNum = DEFAULT_REREAD_SPEED_VAL;
					}
				}
				else {
					pExtArg->uiRereadSpeedNum = DEFAULT_REREAD_SPEED_VAL;
				}
			}
		}
	}
	else if (argc >= 7 && (!_tcscmp(argv[1], _T("-rd")) || !_tcscmp(argv[1], _T("-ra")))) {
		dwNum = _tcstoul(argv[3], &endptr, 10);
		if (*endptr) {
			return FALSE;
		}
		lNum = _tcstol(argv[5], &endptr, 10);
		if (*endptr) {
			return FALSE;
		}
		lNum = _tcstol(argv[6], &endptr, 10);
		if (*endptr) {
			return FALSE;
		}
		printAndSetArg(argv[4]);
		if (!_tcscmp(argv[1], _T("-rd"))) {
			*pExecType = rd;
		}
		else if (!_tcscmp(argv[1], _T("-ra"))) {
			*pExecType = ra;
			for (INT i = 8; i <= argc; i++) {
				if (!_tcscmp(argv[i - 1], _T("add"))) {
					pExtArg->bAdd = TRUE;
					if (argc > 8) {
						lNum = _tcstol(argv[i], &endptr, 10);
						if (*endptr) {
							return FALSE;
						}
						pExtArg->nAudioCDOffsetNum = lNum;
					}
				}
			}
		}
	}
	else if (argc == 5 && !_tcscmp(argv[1], _T("-f"))) {
		*pExecType = f;
		printAndSetArg(argv[4]);
	}
	else if (argc == 3 && !_tcscmp(argv[1], _T("-c"))) {
		*pExecType = c;
	}
	else if (argc == 3 && !_tcscmp(argv[1], _T("-s"))) {
		*pExecType = s;
	}
#if 0
	else if (argc == 4 && !_tcscmp(argv[1], _T("-dec"))) {
		lNum = _tcstol(argv[3], &endptr, 10);
		if (*endptr) {
			return FALSE;
		}
		*pExecType = dec;
		printAndSetArg(argv[2]);
	}
	else if (argc == 3 && !_tcscmp(argv[1], _T("-split"))) {
		*pExecType = split;
		printAndSetArg(argv[2]);
	}
#endif
	else if (argc == 3 && !_tcscmp(argv[1], _T("-sub"))) {
		*pExecType = sub;
		printAndSetArg(argv[2]);
	}
	else {
		return FALSE;
	}
	return TRUE;
}

void printUsage(void)
{
	OutputString(
		_T("Usage\n")
		_T("\t-rall [DriveLetter] [DriveSpeed(0-72)] [filename] <add (val)/c2 (val1) (val2) (val3)/cmi/fua/isrc/pre>\n")
		_T("\t\tRipping CD or DVD from a to z\n")
		_T("\t-rgd [DriveLetter] [DriveSpeed(0-72)] [filename] <c2 (val1) (val2) (val3)>\n")
		_T("\t\tRipping HD area of GD\n")
		_T("\t-rd [DriveLetter] [DriveSpeed(0-72)] [filename] [StartLBA] [EndLBA+1]\n")
		_T("\t\tRipping CD from start to end (data) (Only CD)\n")
		_T("\t-ra [DriveLetter] [DriveSpeed(0-72)] [filename] [StartLBA] [EndLBA+1] <add (val)>\n")
		_T("\t\tRipping CD from start to end (audio) (Only CD)\n")
		_T("\t-f [DriveLetter] [DriveSpeed(0-72)] [filename]\n")
		_T("\t\tRipping floppy\n")
		_T("\t-c [DriveLetter]\n")
		_T("\t\tClose tray\n")
		_T("\t-s [DriveLetter]\n")
		_T("\t\tStop spin disc\n")
//		_T("\t-dec [filename] [LBA]\n")
//		_T("\t\tDescramble data sector (for GD-ROM Image)\n")
//		_T("\t-split [filename]\n")
//		_T("\t\tSplit descrambled File (for GD-ROM Image)\n")
		_T("\t-sub [subfile]\n")
		_T("\t\tParse CloneCD sub file\n")
		_T("Info\n")
		_T("\tDriveSpeed 0: max, 1: 176kb/s, 2: 353kb/s, 3: 529kb/s 4: 706kb/s ...\n")
		_T("\tadd: Add CD offset manually (Only Audio CD)\n")
		_T("\t\tval: samples value\n")
		_T("\tc2: Fix C2 error existing sector (Only CD)\n")
		_T("\t\tval1: value to reread (default: 1024)\n")
		_T("\t\tval2: value to fix a C2 error (default: 32768)\n")
		_T("\t\tval3: value to reread speed (default: 4)\n")
		_T("\tcmi: Log Copyright Management Information (Only DVD)\n")
		_T("\t\tVery slow\n")
		_T("\tfua: Ripping to Force Unit Access (Only DVD)\n")
		_T("\t\tTo defeat the cache, a little slow\n")
		_T("\tisrc: Ignore invalid ISRC (Only CD)\n")
		_T("\t\tFor Valis II[PCE]\n")
		_T("\tpre: If index0 exists in track1, ripping from LBA: -150 to LBA: -1\n")
		_T("\t\tFor SagaFrontier Original Sound Track (Disc 3)\n"));
//		_T("\traw:Ripping Raw Mode (Only DVD)\n")
}

void soundBeep(BOOL bRet)
{
	if (bRet) {
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
}

int _tmain(int argc, _TCHAR* argv[])
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#if 0
	HANDLE hMutex = CreateMutex(NULL , FALSE , _T("DiscImageCreator"));
	if (!hMutex || 
		GetLastError() == ERROR_INVALID_HANDLE || 
		GetLastError() == ERROR_ALREADY_EXISTS) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return -1;
	}
	HANDLE hSnapshot;
	if ((hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(PROCESSENTRY32);
    
		if (Process32First(hSnapshot,&pe32)) {
			do {
				if (!_tcscmp(pe32.szExeFile, _T("IsoBuster.exe"))) {
					OutputErrorString(_T("Please close %s\n"), pe32.szExeFile);
					CloseHandle(hSnapshot);
					soundBeep(FALSE);
					return FALSE;
				}
			} while(Process32Next(hSnapshot,&pe32));
		}
		CloseHandle(hSnapshot);
	}
#endif
	printSeveralInfo();

	EXEC_TYPE execType;
	EXT_ARG pExtArg = { 0 };
	if (!checkArg(argc, argv, &execType, &pExtArg)) {
		printUsage();
	}
	else {
		time_t now;
		struct tm* ts;
		_TCHAR szBuf[128] = { 0 };

		now = time(NULL);
		ts = localtime(&now);
		_tcsftime(szBuf, sizeof(szBuf) / sizeof(szBuf[0]), _T("%Y-%m-%d(%a) %H:%M:%S"), ts);
		OutputString(_T("Start -> %s\n"), szBuf);
		
		BOOL bRet = exec(argv, &execType, &pExtArg);
		soundBeep(bRet);

		now = time(NULL);
		ts = localtime(&now);
		_tcsftime(szBuf, sizeof(szBuf) / sizeof(szBuf[0]), _T("%Y-%m-%d(%a) %H:%M:%S"), ts);
		OutputString(_T("End -> %s\n"), szBuf);
	}
#if 0
	if (!CloseHandle(hMutex)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
#endif
	return 0;
}

