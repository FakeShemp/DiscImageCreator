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

#define DEFAULT_REREAD_VAL			(1024)
#define DEFAULT_MAX_C2_ERROR_VAL	(4096)
#define DEFAULT_REREAD_SPEED_VAL	(4)

// These global variable is set at printAndSetPath().
_TCHAR g_szCurrentdir[_MAX_PATH];
_TCHAR g_drive[_MAX_DRIVE];
_TCHAR g_dir[_MAX_DIR];
_TCHAR g_fname[_MAX_FNAME];
_TCHAR g_ext[_MAX_EXT];

// These static variable is set at checkArg().
static DWORD s_dwSpeed = 0;
static INT s_nStartLBA = 0;
static INT s_nEndLBA = 0;

int soundBeep(int nRet)
{
	if (nRet) {
		if (!Beep(440, 200)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};   // do
		if (!Beep(494, 200)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};   // re
		if (!Beep(554, 200)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};   // mi
		if (!Beep(587, 200)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};   // fa
		if (!Beep(659, 200)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};   // so
		if (!Beep(740, 200)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};   // la
		if (!Beep(830, 200)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};   // ti
		if (!Beep(880, 200)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};   // do
	}
	else {
		if (!Beep(880, 200)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};   // do
		if (!Beep(830, 200)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};   // ti
		if (!Beep(740, 200)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};   // la
		if (!Beep(659, 200)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};   // so
		if (!Beep(587, 200)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};   // fa
		if (!Beep(554, 200)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};   // mi
		if (!Beep(494, 200)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};   // re
		if (!Beep(440, 200)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};   // do
	}
	return TRUE;
}

int processOutputHash(_TCHAR* szFullPath)
{
	HANDLE h = NULL;
	WIN32_FIND_DATA lp = { 0 };
	_TCHAR szPathWithoutFileName[_MAX_PATH] = { 0 };
	BOOL bRet = GetCreatedFileList(
		&h, &lp, szPathWithoutFileName, sizeof(szPathWithoutFileName));
	if (!bRet) {
		return FALSE;
	}
	/* Table of CRCs of all 8-bit messages. */
	UINT crc_table[256] = { 0 };
	make_crc_table(crc_table);
	FILE* fpHash = CreateOrOpenFileW(
		szFullPath, NULL, NULL, NULL, NULL, _T(".dat"), _T(WFLAG), 0, 0);
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
				(!_tcscmp(ext, _T(".bin")) || !_tcscmp(ext, _T(".iso")) ||
				!_tcscmp(ext, _T(".img")))) {
				_sntprintf(szDstPath, _MAX_PATH, 
					_T("%s%s"), szPathWithoutFileName, lp.cFileName);
				FILE* fp = CreateOrOpenFileW(
					szDstPath, NULL, NULL, NULL, NULL, ext, _T("rb"), 0, 0);
				if (!fp) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					break;
				}
				UINT64 ui64FileSize = GetFileSize64(0, fp);
				DWORD dwSectorSizeOne = CD_RAW_SECTOR_SIZE;
				if (!_tcscmp(ext, _T(".iso"))) {
					dwSectorSizeOne = DISC_RAW_READ;
				}
				UINT64 ui64SectorSizeAll = ui64FileSize / (UINT64)dwSectorSizeOne;

				if (ui64FileSize > dwSectorSizeOne * 10) {
					MD5_CTX context = { 0 };
					SHA1Context sha = { 0 };
					CalcInit(&context, &sha);

					BYTE data[CD_RAW_SECTOR_SIZE] = { 0 };
					DWORD crc32 = 0;
					OutputString(_T("Calculating hash: %s\n"), lp.cFileName);
					// TODO: This code can more speed up! if reduce calling fread()
					for (UINT64 i = 0; i < ui64SectorSizeAll; i++) {
						fread(data, sizeof(BYTE), dwSectorSizeOne, fp);
						bRet = CalcHash(crc_table, &crc32, 
							&context, &sha, data, dwSectorSizeOne);
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
					if (!_tcscmp(ext, _T(".img"))) {
#ifndef _DEBUG
						OutputDiscLogA("Hash (entire image)\n");
						OutputHashData(g_LogFile.fpDisc, lp.cFileName,
							ui64FileSize, crc32, digest, Message_Digest);
#endif
					}
					else {
						OutputHashData(fpHash, lp.cFileName,
							ui64FileSize, crc32, digest, Message_Digest);
					}
				}
			}
		}
	} while (FindNextFile(h, &lp));
	OutputString(_T("\n"));
	FcloseAndNull(fpHash);
	FindClose(h);
	return bRet;
}

int exec(_TCHAR* argv[], PEXEC_TYPE pExecType, PEXT_ARG pExtArg)
{
	BOOL bRet = TRUE;
	SetLastError(NO_ERROR);

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
		if (*pExecType == close) {
			bRet = StartStopUnit(&devData, START_UNIT_CODE, START_UNIT_CODE);
		}
		else if (*pExecType == stop) {
			bRet = StartStopUnit(&devData, STOP_UNIT_CODE, STOP_UNIT_CODE);
		}
		else if (*pExecType == reset) {
			bRet = Reset(&devData);
		}
		else {
			DISC_DATA discData = { '\0' };
			PDISC_DATA pDiscData = &discData;
			FILE* fpCcd = NULL;
			try {
				if (!TestUnitReady(&devData)) {
					throw FALSE;
				}
#ifndef _DEBUG
				if (!InitLogFile(pExecType, argv[3])) {
					throw FALSE;
				}
#endif
				if (*pExecType == floppy) {
					if (!DiskGetMediaTypes(&devData, argv[3])) {
						throw FALSE;
					}
				}
				else {
					BOOL bBusTypeUSB = FALSE;
					if (!StorageQueryProperty(&devData, &bBusTypeUSB)) {
						throw FALSE;
					}
					if (!bBusTypeUSB) {
						if (!ScsiGetAddress(&devData)) {
							throw FALSE;
						}
					}
					if (!Inquiry(&devData)) {
						throw FALSE;
					}
					
					IsValidPlextorDrive(&devData);
					if (&devData.byPlexType) {
						ReadEeprom(&devData);
						SetSpeedRead(&devData, TRUE);
					}
					if (!ReadBufferCapacity(&devData)) {
						throw FALSE;
					}
					if (!SetCDSpeed(&devData, s_dwSpeed)) {
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
					if (discData.SCSI.wCurrentMedia == ProfileCdrom || 
						discData.SCSI.wCurrentMedia == ProfileCdRecordable ||
						discData.SCSI.wCurrentMedia == ProfileCdRewritable ||
						(discData.SCSI.wCurrentMedia == ProfileInvalid && (*pExecType == gd))) {
						_TCHAR out[_MAX_PATH] = { 0 };
						if (*pExecType == cd && !pExtArg->bReverse) {
							fpCcd = CreateOrOpenFileW(argv[3], NULL, out, NULL, NULL, _T(".ccd"), _T(WFLAG), 0, 0);
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
						if (bRet) {
							if (*pExecType == cd) {
								if (!ReadCDForCheckingReadInOut(pExecType, pExtArg, 
									&devData, pDiscData, argv[3], READ_CD_FLAG::CDDA)) {
									throw FALSE;
								}
								if (!pExtArg->bReverse) {
									if (pExtArg->bPre) {
										if (!ReadCDForCheckingIndex0InTrack1(pExtArg, &devData, pDiscData)) {
											throw FALSE;
										}
									}
									// Basically, CD+G data exists an audio only disc
									// But exceptionally, WonderMega Collection (SCD)(mixed disc) exists CD+G data.
									size_t dwTrackAllocSize = (size_t)pDiscData->SCSI.toc.LastTrack + 1;
									if (NULL == (pDiscData->SUB_CHANNEL.lpRtoWList =
										(LPBYTE)calloc(dwTrackAllocSize, sizeof(BYTE)))) {
										OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
										throw FALSE;
									}
									if (!ReadCDForCheckingCDG(&devData, pDiscData)) {
										throw FALSE;
									}
									else {
										if (!ReadCDForVolumeDescriptor(&devData, 
											&pDiscData->SCSI.toc, pDiscData->SCSI.nFirstLBAofDataTrack)) {
											throw FALSE;
										}
									}
								}
								bRet = ReadCDAll(pExecType, pExtArg, &devData, &discData, argv[3], fpCcd);
							}
							else if (*pExecType == gd) {
								bRet = ReadCDPartial(pExecType, pExtArg, &devData, &discData, argv[3],
									45000, 549149 + 1, READ_CD_FLAG::CDDA, FALSE);
							}
							else if (*pExecType == data) {
								bRet = ReadCDPartial(pExecType, pExtArg, &devData, &discData, argv[3],
									s_nStartLBA, s_nEndLBA, READ_CD_FLAG::All, FALSE);
							}
							else if (*pExecType == audio) {
								bRet = ReadCDPartial(pExecType, pExtArg, &devData, &discData, argv[3],
									s_nStartLBA, s_nEndLBA, READ_CD_FLAG::CDDA, FALSE);
							}
						}
						else {
							// check only for drive info
							OutputString(_T("read in/out test\n"));
							OutputString(_T("======== start ========\n"));
							READ_CD_FLAG::_SECTOR_TYPE flgFirst = READ_CD_FLAG::All;
							if ((pDiscData->SCSI.toc.TrackData[pDiscData->SCSI.toc.FirstTrack - 1].Control & AUDIO_DATA_TRACK) == 0) {
								flgFirst = READ_CD_FLAG::CDDA;
								OutputString(_T("First track: audio\n"));
							}
							else {
								OutputString(_T("First track: data\n"));
							}
							pDiscData->MAIN_CHANNEL.nCombinedOffset = -1;
							ReadCDForCheckingReadInOut(pExecType,
								pExtArg, &devData, pDiscData, argv[3], flgFirst);

							READ_CD_FLAG::_SECTOR_TYPE flgLast = READ_CD_FLAG::All;
							if ((pDiscData->SCSI.toc.TrackData[pDiscData->SCSI.toc.LastTrack - 1].Control & AUDIO_DATA_TRACK) == 0) {
								flgLast = READ_CD_FLAG::CDDA;
								OutputString(_T("Last track: audio\n"));
							}
							else {
								OutputString(_T("Last track: data\n"));
							}
							pDiscData->MAIN_CHANNEL.nCombinedOffset = 1;
							ReadCDForCheckingReadInOut(pExecType,
								pExtArg, &devData, pDiscData, argv[3], flgLast);
							OutputString(_T("========= end =========\n"));
						}
					}
					else if (discData.SCSI.wCurrentMedia == ProfileDvdRom || 
						discData.SCSI.wCurrentMedia == ProfileDvdRecordable ||
						discData.SCSI.wCurrentMedia == ProfileDvdRam || 
						discData.SCSI.wCurrentMedia == ProfileDvdRewritable || 
						discData.SCSI.wCurrentMedia == ProfileDvdRWSequential || 
						discData.SCSI.wCurrentMedia == ProfileDvdDashRDualLayer || 
						discData.SCSI.wCurrentMedia == ProfileDvdDashRLayerJump || 
						discData.SCSI.wCurrentMedia == ProfileDvdPlusRW || 
//						discData.SCSI.wCurrentMedia == ProfileInvalid ||
						discData.SCSI.wCurrentMedia == ProfileDvdPlusR) {
						bRet = ReadDVDStructure(&devData, &discData);
#ifndef _DEBUG
						FlushLog();
#endif
						if (pExtArg->bCmi) {
							bRet = ReadDVDForCMI(&devData, &discData);
						}
						if (bRet) {
							if (argv[5] && !_tcscmp(argv[5], _T("raw"))) {
#if 0
								bRet = ReadDVDRaw(&devData, &discData, szVendorId, argv[3]);
#endif
							}
							else {
								bRet = ReadDVD(&devData, &discData, pExtArg, argv[3]);
							}
						}
					}
					if (bRet && (*pExecType == cd || *pExecType == dvd || *pExecType == gd)) {
						bRet = processOutputHash(argv[3]);
					}
				}
			}
			catch (BOOL bErr) {
				bRet = bErr;
			}
			FreeAndNull(pDiscData->SUB_CHANNEL.lpRtoWList);
			TerminateLBAPerTrack(&pDiscData);
			if (devData.bSuccessReadToc) {
				TerminateTocFullData(pExecType, &devData, &pDiscData);
			}
			FcloseAndNull(fpCcd);
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
	return bRet;
}

int printAndSetPath(_TCHAR* szFullPath)
{
	if (!GetCurrentDirectory(_MAX_PATH, g_szCurrentdir)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	_tsplitpath(szFullPath, g_drive, g_dir, g_fname, g_ext);
	OutputString(
		_T("CurrentDir\n")
		_T("\t%s\n")
		_T("InputPath\n")
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
	_TCHAR* endptr = NULL;
	if (argc >= 5 && (!_tcscmp(argv[1], _T("cd")) || !_tcscmp(argv[1], _T("gd")))) {
		s_dwSpeed = _tcstoul(argv[4], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("Bad arg: %s Please integer\n"), endptr);
			return FALSE;
		}
		for (INT i = 6; i <= argc; i++) {
			if (!_tcscmp(argv[i - 1], _T("/a"))) {
				pExtArg->bAdd = TRUE;
				if (argc > i) {
					pExtArg->nAudioCDOffsetNum = _tcstol(argv[i], &endptr, 10);
					if (*endptr) {
						OutputErrorString(_T("Bad arg: %s Please integer\n"), endptr);
						return FALSE;
					}
				}
			}
			else if (!_tcscmp(argv[i - 1], _T("/c2"))) {
				pExtArg->bC2 = TRUE;
				if (argc > i && _tcsncmp(argv[i], _T("/"), 1)) {
					pExtArg->dwMaxRereadNum = _tcstoul(argv[i], &endptr, 10);
					if (*endptr) {
						OutputErrorString(_T("Bad arg: %s Please integer\n"), endptr);
						return FALSE;
					}
				}
				else {
					pExtArg->dwMaxRereadNum = DEFAULT_REREAD_VAL;
					OutputString(_T("/c2 opt val1 is omitted. set %d\n"), DEFAULT_REREAD_VAL);
				}
				if (argc > i + 1 && _tcsncmp(argv[i + 1], _T("/"), 1)) {
					pExtArg->dwMaxC2ErrorNum = _tcstoul(argv[i + 1], &endptr, 10);
					if (*endptr) {
						OutputErrorString(_T("Bad arg: %s Please integer\n"), endptr);
						return FALSE;
					}
				}
				else {
					pExtArg->dwMaxC2ErrorNum = DEFAULT_MAX_C2_ERROR_VAL;
					OutputString(_T("/c2 opt val2 is omitted. set %d\n"), DEFAULT_MAX_C2_ERROR_VAL);
				}
				if (argc > i + 2 && _tcsncmp(argv[i + 2], _T("/"), 1)) {
					pExtArg->dwRereadSpeedNum = _tcstoul(argv[i + 2], &endptr, 10);
					if (*endptr) {
						OutputErrorString(_T("Bad arg: %s Please integer\n"), endptr);
						return FALSE;
					}
				}
				else {
					pExtArg->dwRereadSpeedNum = DEFAULT_REREAD_SPEED_VAL;
					OutputString(_T("/c2 opt val3 is omitted. set %d\n"), DEFAULT_REREAD_SPEED_VAL);
				}
			}
			else if (!_tcscmp(argv[i - 1], _T("/i"))) {
				pExtArg->bISRC = TRUE;
			}
			else if (!_tcscmp(argv[i - 1], _T("/l"))) {
				pExtArg->bLibCrypt = TRUE;
			}
			else if (!_tcscmp(argv[i - 1], _T("/m"))) {
				pExtArg->bMCN = TRUE;
			}
			else if (!_tcscmp(argv[i - 1], _T("/p"))) {
				pExtArg->bPre = TRUE;
			}
			else if (!_tcscmp(argv[i - 1], _T("/r"))) {
				pExtArg->bReverse = TRUE;
			}
		}
		if (!_tcscmp(argv[1], _T("cd"))) {
			*pExecType = cd;
		}
		else if (!_tcscmp(argv[1], _T("gd"))) {
			*pExecType = gd;
		}
		printAndSetPath(argv[3]);
	}
	else if (argc >= 5 && !_tcscmp(argv[1], _T("dvd"))) {
		s_dwSpeed = _tcstoul(argv[4], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("Bad arg: %s Please integer\n"), endptr);
			return FALSE;
		}
		for (INT i = 6; i <= argc; i++) {
			if (!_tcscmp(argv[i - 1], _T("/c"))) {
				pExtArg->bCmi = TRUE;
			}
			else if (!_tcscmp(argv[i - 1], _T("/f"))) {
				pExtArg->bFua = TRUE;
			}
		}
		*pExecType = dvd;
		printAndSetPath(argv[3]);
	}
	else if (argc >= 7 && (!_tcscmp(argv[1], _T("data")) || !_tcscmp(argv[1], _T("audio")))) {
		s_dwSpeed = _tcstoul(argv[4], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("Bad arg: %s Please integer\n"), endptr);
			return FALSE;
		}
		s_nStartLBA = _tcstol(argv[5], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("Bad arg: %s Please integer\n"), endptr);
			return FALSE;
		}
		s_nEndLBA = _tcstol(argv[6], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("Bad arg: %s Please integer\n"), endptr);
			return FALSE;
		}

		if (!_tcscmp(argv[1], _T("data"))) {
			*pExecType = data;
		}
		else if (!_tcscmp(argv[1], _T("audio"))) {
			*pExecType = audio;
			for (INT i = 8; i <= argc; i++) {
				if (!_tcscmp(argv[i - 1], _T("/a"))) {
					pExtArg->bAdd = TRUE;
					if (argc > i) {
						pExtArg->nAudioCDOffsetNum = _tcstol(argv[i], &endptr, 10);
						if (*endptr) {
							OutputErrorString(_T("Bad arg: %s Please integer\n"), endptr);
							return FALSE;
						}
					}
				}
			}
		}
		printAndSetPath(argv[3]);
	}
	else if (argc == 4 && !_tcscmp(argv[1], _T("floppy"))) {
		*pExecType = floppy;
		printAndSetPath(argv[3]);
	}
	else if (argc == 3 && !_tcscmp(argv[1], _T("close"))) {
		*pExecType = close;
	}
	else if (argc == 3 && !_tcscmp(argv[1], _T("stop"))) {
		*pExecType = stop;
	}
	else if (argc == 3 && !_tcscmp(argv[1], _T("sub"))) {
		*pExecType = sub;
		printAndSetPath(argv[2]);
	}
	else if (argc == 3 && !_tcscmp(argv[1], _T("reset"))) {
		*pExecType = reset;
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
		_T("\tcd <DriveLetter> <Filename> <DriveSpeed(0-72)> [/a (val)]\n")
		_T("\t   [/c2 (val1) (val2) (val3)] [/i] [/l] [/m] [/p] [/r]\n")
		_T("\t\tRipping CD from a to z\n")
		_T("\tdvd <DriveLetter> <Filename> <DriveSpeed(0-72)> [/c] [/f]\n")
		_T("\t\tRipping DVD from a to z\n")
		_T("\tgd <DriveLetter> <Filename> <DriveSpeed(0-72)>\n")
		_T("\t   [/c2 (val1) (val2) (val3)]\n")
		_T("\t\tRipping HD area of GD from a to z\n")
		_T("\tdata <DriveLetter> <Filename> <DriveSpeed(0-72)> <StartLBA> <EndLBA+1>\n")
		_T("\t\tRipping CD from start to end (data) (Only CD)\n")
		_T("\taudio <DriveLetter> <Filename> <DriveSpeed(0-72)> <StartLBA> <EndLBA+1>\n")
		_T("\t      [/a (val)]\n")
		_T("\t\tRipping CD from start to end (audio) (Only CD)\n")
		_T("\tfloppy <DriveLetter> <Filename>\n")
		_T("\t\tRipping floppy\n")
		_T("\tclose <DriveLetter>\n")
		_T("\t\tClose tray\n"));
	_tsystem(_T("pause"));
	OutputString(
		_T("\tstop <DriveLetter>\n")
		_T("\t\tStop spin disc\n")
		_T("\tsub <Subfile>\n")
		_T("\t\tParse CloneCD sub file\n")
		_T("\treset <DriveLetter>\n")
		_T("\t\tReset the drive (Only Plextor)\n")
		_T("Option Info (CD)\n")
		_T("\t/a\tAdd CD offset manually (Only Audio CD)\n")
		_T("\t\t\tval\tsamples value\n")
		_T("\t/c2\tFix C2 error existing sector\n")
		_T("\t\t\tval1\tvalue to reread (default: 1024)\n")
		_T("\t\t\tval2\tvalue to fix a C2 error (default: 4096)\n")
		_T("\t\t\tval3\tvalue to reread speed (default: 4)\n")
		_T("\t/i\tIgnore invalid ISRC\n")
		_T("\t\t\tFor Valis II [PC-Engine]\n")
		_T("\t/l\tNot fix SubQ (RMSF, AMSF, CRC) (RMSFs 03:xx:xx and 09:xx:xx)\n")
		_T("\t\t\tFor PlayStation LibCrypt disc\n")
		_T("\t/m\tIf MCN sector exists in the last sector of the track, use this\n")
		_T("\t\t\tFor WonderMega Collection [Mega-CD]\n")
		_T("\t/p\tRipping AMSF from 00:00:00 to 00:01:74\n")
		_T("\t\t\tFor SagaFrontier Original Sound Track (Disc 3) etc\n")
		_T("\t\t\tSupport drive: PLEXTOR PX-W5224, PREMIUM, PREMIUM2\n")
		_T("\t\t\t               PX-708, PX-712, PX-716, PX-755, PX-760\n"));
	_tsystem(_T("pause"));
	OutputString(
		_T("\t/r\tReverse reading CD (including data track)\n")
		_T("\t\t\tFor Alpha-Disc, very slow\n")
		_T("Option Info (DVD)\n")
		_T("\t/c\tLog Copyright Management Information\n")
		_T("\t/f\tRipping to Force Unit Access\n")
		_T("\t\tTo defeat the cache, a little slow\n"));
}

int printSeveralInfo()
{
	if (!OutputWindowsVer()) {
		return FALSE;
	}
	OutputString(_T("AppVersion\n"));
#ifdef _WIN64
	OutputString(_T("\tx64, "));
#else
	OutputString(_T("\tx86, "));
#endif
#ifdef UNICODE
	OutputString(_T("UnicodeBuild, "));
#else
	OutputString(_T("AnsiBuild, "));
#endif
	OutputString(_T("%s %s\n"), _T(__DATE__), _T(__TIME__));
	return TRUE;
}

int _tmain(int argc, _TCHAR* argv[])
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	int nRet = TRUE;
	HANDLE hMutex = CreateMutex(NULL, FALSE, _T("DiscImageCreator"));
	if (!hMutex || 
		GetLastError() == ERROR_INVALID_HANDLE || 
		GetLastError() == ERROR_ALREADY_EXISTS) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return EXIT_FAILURE;
	}
#if 0
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
		nRet = FALSE;
	}
	else {
		time_t now;
		struct tm* ts;
		_TCHAR szBuf[128] = { 0 };

		now = time(NULL);
		ts = localtime(&now);
		_tcsftime(szBuf, sizeof(szBuf) / sizeof(szBuf[0]), _T("%Y-%m-%d(%a) %H:%M:%S"), ts);
		OutputString(_T("Start: %s\n"), szBuf);
		
		nRet = exec(argv, &execType, &pExtArg);
		nRet = soundBeep(nRet);

		now = time(NULL);
		ts = localtime(&now);
		_tcsftime(szBuf, sizeof(szBuf) / sizeof(szBuf[0]), _T("%Y-%m-%d(%a) %H:%M:%S"), ts);
		OutputString(_T("End: %s\n"), szBuf);
	}
	if (!CloseHandle(hMutex)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		nRet = FALSE;
	}
	return nRet = nRet == TRUE ? EXIT_SUCCESS : EXIT_FAILURE;
}

