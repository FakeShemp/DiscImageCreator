// DiscImageCreator.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//
/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "calcHash.h"
#include "check.h"
#include "execIoctl.h"
#include "execScsiCmd.h"
#include "execScsiCmdforCD.h"
#include "execScsiCmdforDVD.h"
#include "get.h"
#include "init.h"
#include "output.h"
#include "_external\prngcd.h"

#define DEFAULT_REREAD_VAL			(1024)
#define DEFAULT_MAX_C2_ERROR_VAL	(4096)
#define DEFAULT_REREAD_SPEED_VAL	(4)
#define DEFAULT_SPTD_TIMEOUT_VAL	(60)

// These global variable is set at printAndSetPath().
_TCHAR g_szCurrentdir[_MAX_PATH];
_TCHAR g_drive[_MAX_DRIVE];
_TCHAR g_dir[_MAX_DIR];
_TCHAR g_fname[_MAX_FNAME];
_TCHAR g_ext[_MAX_EXT];

BYTE g_aSyncHeader[SYNC_SIZE] = {
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0x00
};

// These static variable is set at checkArg().
static DWORD s_dwSpeed = 0;
static INT s_nStartLBA = 0;
static INT s_nEndLBA = 0;

#define playtime (200)
#define c4 (262)
#define d4 (294)
#define e4 (330)
#define f4 (349)
#define g4 (392)
#define a4 (440)
#define b4 (494)
#define c5 (523)
#define d5 (587)
#define e5 (659)
#define f5 (698)
#define g5 (784)
#define a5 (880)
#define b5 (988)
#define c6 (1047)

int soundBeep(int nRet)
{
	if (nRet) {
		if (!Beep(c4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(d4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(e4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(f4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(g4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(a4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(b4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(c5, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
	}
	else {
		if (!Beep(c5, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(b4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(a4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(g4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(f4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(e4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(d4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(c4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
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
	make_crc_table();
	FILE* fpHash = CreateOrOpenFileW(
		szFullPath, NULL, NULL, NULL, NULL, _T(".dat"), _T(WFLAG), 0, 0);
	if (!fpHash) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		FindClose(h);
		return FALSE;
	}

	_TCHAR szDstPath[_MAX_PATH] = { 0 };
	_TCHAR ext[_MAX_EXT] = { 0 };
	// http://msdn.microsoft.com/en-us/library/aa364428%28v=vs.85%29.aspx
	//  The order in which the search returns the files, such as alphabetical order, is not guaranteed, and is dependent on the file system. 
	//  If the data must be sorted, the application must do the ordering after obtaining all the results.
	//  The order in which this function returns the file names is dependent on the file system type. 
	//  With the NTFS file system and CDFS file systems, the names are usually returned in alphabetical order. 
	//  With FAT file systems, the names are usually returned in the order the files were written to the disk, which may or may not be in alphabetical order.
	//  However, as stated previously, these behaviors are not guaranteed.
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
					dwSectorSizeOne = DISC_RAW_READ_SIZE;
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
						bRet = CalcHash(&crc32, &context,
							&sha, data, dwSectorSizeOne);
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
						OutputDiscLogA(
							"============================= Hash (entire image) =============================\n");
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
		DEVICE devData = { 0 };
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
		DWORD dwReturned = 0;
		CDROM_EXCLUSIVE_ACCESS exclusive;
		exclusive.RequestType = ExclusiveAccessQueryState;
		exclusive.Flags = 0;

		CDROM_EXCLUSIVE_LOCK_STATE lockstate = { 0 };
		bRet = DeviceIoControl(devData.hDevice, IOCTL_CDROM_EXCLUSIVE_ACCESS,
			&exclusive, sizeof(CDROM_EXCLUSIVE_ACCESS), &lockstate,
			sizeof(CDROM_EXCLUSIVE_LOCK_STATE), &dwReturned, NULL);
		printf("RequestType %u, Flags %u\n", exclusive.RequestType, exclusive.Flags);
		printf("LockState %u, CallerName %s\n", lockstate.LockState, lockstate.CallerName);

		CDROM_EXCLUSIVE_LOCK lock;
		exclusive.RequestType = ExclusiveAccessLockDevice;
		lock.Access = exclusive;
		bRet = DeviceIoControl(devData.hDevice, IOCTL_CDROM_EXCLUSIVE_ACCESS,
			&lock, sizeof(CDROM_EXCLUSIVE_LOCK), &lock,
			sizeof(CDROM_EXCLUSIVE_LOCK), &dwReturned, NULL);
		printf("RequestType %u, CallerName %s\n", lock.Access.RequestType, lock.CallerName);
#endif
		// 1st: set TimeOutValue here (because use ScsiPassThroughDirect)
		if (pExtArg->byReadContinue) {
			devData.dwTimeOutValue = pExtArg->dwTimeoutNum;
		}
		else {
			devData.dwTimeOutValue = DEFAULT_SPTD_TIMEOUT_VAL;
		}
		if (*pExecType == stop) {
			bRet = StartStopUnit(pExtArg, &devData, STOP_UNIT_CODE, STOP_UNIT_CODE);
		}
		else if (*pExecType == start) {
			bRet = StartStopUnit(pExtArg, &devData, START_UNIT_CODE, STOP_UNIT_CODE);
		}
		else if (*pExecType == eject) {
			bRet = StartStopUnit(pExtArg, &devData, STOP_UNIT_CODE, START_UNIT_CODE);
		}
		else if (*pExecType == closetray) {
			bRet = StartStopUnit(pExtArg, &devData, START_UNIT_CODE, START_UNIT_CODE);
		}
		else if (*pExecType == reset) {
			bRet = Reset(pExtArg, &devData);
		}
		else {
			DISC discData = { '\0' };
			PDISC pDisc = &discData;
			MAIN_HEADER mainHeader = { 0 };
			FILE* fpCcd = NULL;
			try {
				if (!TestUnitReady(pExtArg, &devData)) {
					throw FALSE;
				}
#ifndef _DEBUG
				// 2nd: create logfile here (because logging all working)
				if (!InitLogFile(pExecType, pExtArg, argv[3])) {
					throw FALSE;
				}
#endif
				if (*pExecType == fd) {
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
					// 3rd: get drive vender, product id here (because use IsValidPlextorDrive)
					if (!Inquiry(pExtArg, &devData)) {
						throw FALSE;
					}
					// 4th: check PLEXTOR or not here (because use modesense and from there)
					IsValidPlextorDrive(pExtArg, &devData);
					if ((PLXTR_DRIVE_TYPE)devData.byPlxtrType != PLXTR_DRIVE_TYPE::No) {
						if (pExtArg->byPre) {
							SupportIndex0InTrack1(pExtArg, &devData, pDisc);
						}
						ReadEeprom(pExtArg, &devData);
						SetSpeedRead(pExtArg, &devData, TRUE);
					}
					// 5th: get currentMedia, if use CD-Text, C2 error, modesense, readbuffercapacity, setcdspeed or not here.
					if (!GetConfiguration(pExtArg, &devData, &discData)) {
						throw FALSE;
					}
					ModeSense10(pExtArg, &devData);
					ReadBufferCapacity(pExtArg, &devData);
					SetCDSpeed(pExtArg, &devData, s_dwSpeed);

					ReadDiscInformation(pExtArg, &devData);
					if (!ReadTOC(pExtArg, pExecType, &devData, &discData)) {
						throw FALSE;
					}
					if (discData.SCSI.wCurrentMedia == ProfileCdrom || 
						discData.SCSI.wCurrentMedia == ProfileCdRecordable ||
						discData.SCSI.wCurrentMedia == ProfileCdRewritable ||
						(discData.SCSI.wCurrentMedia == ProfileInvalid && (*pExecType == gd))) {
						_TCHAR out[_MAX_PATH] = { 0 };
						// 6th: open ccd here (because use ReadTOCFull and from there)
						if (*pExecType == cd && !pExtArg->byReverse) {
							fpCcd = CreateOrOpenFileW(argv[3], NULL, 
								out, NULL, NULL, _T(".ccd"), _T(WFLAG), 0, 0);
							if (!fpCcd) {
								OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
								throw FALSE;
							}
						}
						InitMainDataHeader(pExtArg, &mainHeader);
						if (!InitSubData(pExecType, &pDisc)) {
							throw FALSE;
						}
						if (!InitTocFullData(pExecType, &pDisc)) {
							throw FALSE;
						}
						if (!InitTocTextData(pExecType, &devData, &pDisc)) {
							throw FALSE;
						}
						if (!ReadCDForSearchingOffset(pExecType, pExtArg, &devData, &discData)) {
							// check only for drive info
							OutputString(_T("read in/out test\n"));
							OutputString(_T("======== start ========\n"));
							READ_CD_FLAG::_EXPECTED_SECTOR_TYPE flgFirst = READ_CD_FLAG::All;
							if ((pDisc->SCSI.toc.TrackData[pDisc->SCSI.toc.FirstTrack - 1].Control & AUDIO_DATA_TRACK) == 0) {
								flgFirst = READ_CD_FLAG::CDDA;
								OutputString(_T("First track: audio\n"));
							}
							else {
								OutputString(_T("First track: data\n"));
							}
							pDisc->MAIN.nCombinedOffset = -1;
							ReadCDForCheckingReadInOut(pExecType,
								pExtArg, &devData, pDisc, &mainHeader, argv[3], flgFirst);

							READ_CD_FLAG::_EXPECTED_SECTOR_TYPE flgLast = READ_CD_FLAG::All;
							if ((pDisc->SCSI.toc.TrackData[pDisc->SCSI.toc.LastTrack - 1].Control & AUDIO_DATA_TRACK) == 0) {
								flgLast = READ_CD_FLAG::CDDA;
								OutputString(_T("Last track: audio\n"));
							}
							else {
								OutputString(_T("Last track: data\n"));
							}
							pDisc->MAIN.nCombinedOffset = 1;
							ReadCDForCheckingReadInOut(pExecType,
								pExtArg, &devData, pDisc, &mainHeader, argv[3], flgLast);
							OutputString(_T("========= end =========\n"));
							throw FALSE;
						}
						if (!ReadTOCFull(pExtArg, &devData, &discData, fpCcd)) {
							throw FALSE;
						}
						make_scrambled_table();
						if (*pExecType == cd) {
							if (!ReadCDForCheckingReadInOut(pExecType, pExtArg,
								&devData, pDisc, &mainHeader, argv[3], READ_CD_FLAG::CDDA)) {
								throw FALSE;
							}
							if (!pExtArg->byReverse) {
#if 0
								if (!ReadCDForCheckingCommand(pExtArg, &devData, pDisc)) {
									throw FALSE;
								}
#endif
								// Basically, CD+G data is included in audio only disc
								// But exceptionally, WonderMega Collection (SCD)(mixed disc) exists CD+G data.
								if (!ReadCDForCheckingCDG(pExtArg, &devData, pDisc)) {
									throw FALSE;
								}
								if (!pDisc->SCSI.byAudioOnly) {
									if (!ReadCDForFileSystem(pExtArg, &devData, pDisc)) {
										throw FALSE;
									}
								}
							}
							bRet = ReadCDAll(pExecType, pExtArg,
								&devData, &discData, &mainHeader, argv[3], fpCcd);
						}
						else if (*pExecType == gd) {
							if (!ReadCDForGDTOC(pExtArg, &devData, &discData)) {
								throw FALSE;
							}
							bRet = ReadCDPartial(pExecType, pExtArg, &devData,
								&discData, &mainHeader, argv[3], FIRST_LBA_FOR_GD,
								549149 + 1, READ_CD_FLAG::CDDA, FALSE);
						}
						else if (*pExecType == data) {
							bRet = ReadCDPartial(pExecType, pExtArg, &devData,
								&discData, &mainHeader, argv[3], s_nStartLBA,
								s_nEndLBA, READ_CD_FLAG::All, FALSE);
						}
						else if (*pExecType == audio) {
							bRet = ReadCDPartial(pExecType, pExtArg, &devData,
								&discData, &mainHeader, argv[3], s_nStartLBA,
								s_nEndLBA, READ_CD_FLAG::CDDA, FALSE);
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
						bRet = ReadDVDStructure(pExtArg, &devData, &discData);
#ifndef _DEBUG
						FlushLog();
#endif
						if (pExtArg->byCmi) {
							bRet = ReadDVDForCMI(pExtArg, &devData, &discData);
						}
						if (bRet) {
							if (argv[5] && !_tcscmp(argv[5], _T("raw"))) {
#if 0
								bRet = ReadDVDRaw(&devData, &discData, szVendorId, argv[3]);
#endif
							}
							else {
								bRet = ReadDVD(pExtArg, &devData, &discData, argv[3]);
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
			TerminateLBAPerTrack(&pDisc);
			TerminateSubData(pExecType, &pDisc);
			TerminateTocFullData(&pDisc);
			if (devData.bySuccessReadToc) {
				TerminateTocTextData(pExecType, &devData, &pDisc);
			}
			FcloseAndNull(fpCcd);
#ifndef _DEBUG
			TerminateLogFile(pExecType, pExtArg);
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
			OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
			return FALSE;
		}
		pExtArg->dwSubAddionalNum = 1;
		for (INT i = 6; i <= argc; i++) {
			if (!_tcscmp(argv[i - 1], _T("/a"))) {
				pExtArg->byAdd = TRUE;
				if (argc > i && _tcsncmp(argv[i], _T("/"), 1)) {
					pExtArg->nAudioCDOffsetNum = _tcstol(argv[i++], &endptr, 10);
					if (*endptr) {
						OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
						return FALSE;
					}
				}
				else {
					pExtArg->nAudioCDOffsetNum = 0;
					OutputString(_T("/a val is omitted. set [%d]\n"), 0);
				}
			}
			else if (!_tcscmp(argv[i - 1], _T("/be"))) {
				pExtArg->byBe = TRUE;
				pExtArg->byD8 = FALSE;
			}
			else if (!_tcscmp(argv[i - 1], _T("/d8"))) {
				pExtArg->byBe = FALSE;
				pExtArg->byD8 = TRUE;
			}
			else if (!_tcscmp(argv[i - 1], _T("/c2"))) {
				pExtArg->byC2 = TRUE;
				if (argc > i && _tcsncmp(argv[i], _T("/"), 1)) {
					pExtArg->dwMaxRereadNum = _tcstoul(argv[i++], &endptr, 10);
					if (*endptr) {
						OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
						return FALSE;
					}
					if (argc > i && _tcsncmp(argv[i], _T("/"), 1)) {
						pExtArg->dwMaxC2ErrorNum = _tcstoul(argv[i++], &endptr, 10);
						if (*endptr) {
							OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
							return FALSE;
						}
						if (argc > i && _tcsncmp(argv[i], _T("/"), 1)) {
							pExtArg->dwRereadSpeedNum = _tcstoul(argv[i++], &endptr, 10);
							if (*endptr) {
								OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
								return FALSE;
							}
						}
						else {
							pExtArg->dwRereadSpeedNum = DEFAULT_REREAD_SPEED_VAL;
							OutputString(
								_T("/c2 val3 is omitted. set [%d]\n")
								, DEFAULT_REREAD_SPEED_VAL);
						}
					}
					else {
						pExtArg->dwMaxC2ErrorNum = DEFAULT_MAX_C2_ERROR_VAL;
						pExtArg->dwRereadSpeedNum = DEFAULT_REREAD_SPEED_VAL;
						OutputString(
							_T("/c2 val2 is omitted. set [%d]\n")
							_T("/c2 val3 is omitted. set [%d]\n")
							, DEFAULT_MAX_C2_ERROR_VAL
							, DEFAULT_REREAD_SPEED_VAL);
					}
				}
				else {
					pExtArg->dwMaxRereadNum = DEFAULT_REREAD_VAL;
					pExtArg->dwMaxC2ErrorNum = DEFAULT_MAX_C2_ERROR_VAL;
					pExtArg->dwRereadSpeedNum = DEFAULT_REREAD_SPEED_VAL;
					OutputString(
						_T("/c2 val1 is omitted. set [%d]\n")
						_T("/c2 val2 is omitted. set [%d]\n")
						_T("/c2 val3 is omitted. set [%d]\n")
						, DEFAULT_REREAD_VAL
						, DEFAULT_MAX_C2_ERROR_VAL
						, DEFAULT_REREAD_SPEED_VAL);
				}
			}
			else if (!_tcscmp(argv[i - 1], _T("/f"))) {
				pExtArg->byFua = TRUE;
			}
			else if (!_tcscmp(argv[i - 1], _T("/i"))) {
				pExtArg->byISRC = TRUE;
			}
			else if (!_tcscmp(argv[i - 1], _T("/l"))) {
				pExtArg->byLibCrypt = TRUE;
			}
			else if (!_tcscmp(argv[i - 1], _T("/m"))) {
				pExtArg->byMCN = TRUE;
			}
			else if (!_tcscmp(argv[i - 1], _T("/p"))) {
				pExtArg->byPre = TRUE;
			}
			else if (!_tcscmp(argv[i - 1], _T("/r"))) {
				pExtArg->byReverse = TRUE;
			}
			else if (!_tcscmp(argv[i - 1], _T("/rc"))) {
				pExtArg->byReadContinue = TRUE;
				if (argc > i && _tcsncmp(argv[i], _T("/"), 1)) {
					pExtArg->dwTimeoutNum = _tcstoul(argv[i++], &endptr, 10);
					if (*endptr) {
						OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
						return FALSE;
					}
				}
				else {
					pExtArg->dwTimeoutNum = DEFAULT_SPTD_TIMEOUT_VAL;
					OutputString(
						_T("/rc val is omitted. set [%d]\n"), DEFAULT_SPTD_TIMEOUT_VAL);
				}
			}
			else if (!_tcscmp(argv[i - 1], _T("/s"))) {
				if (argc > i && _tcsncmp(argv[i], _T("/"), 1)) {
					pExtArg->dwSubAddionalNum = _tcstoul(argv[i++], &endptr, 10);
					if (*endptr) {
						OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
						return FALSE;
					}
				}
				else {
					pExtArg->dwSubAddionalNum = 1;
					OutputString(_T("/s val is omitted. set [%d]\n"), 1);
				}
			}
			else {
				OutputErrorString(_T("Unknown option: [%s]\n"), argv[i - 1]);
				return FALSE;
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
			OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
			return FALSE;
		}
		for (INT i = 6; i <= argc; i++) {
			if (!_tcscmp(argv[i - 1], _T("/c"))) {
				pExtArg->byCmi = TRUE;
			}
			else if (!_tcscmp(argv[i - 1], _T("/f"))) {
				pExtArg->byFua = TRUE;
			}
			else {
				OutputErrorString(_T("Unknown option: [%s]\n"), argv[i - 1]);
				return FALSE;
			}
		}
		*pExecType = dvd;
		printAndSetPath(argv[3]);
	}
	else if (argc >= 7 && (!_tcscmp(argv[1], _T("data")) || !_tcscmp(argv[1], _T("audio")))) {
		s_dwSpeed = _tcstoul(argv[4], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
			return FALSE;
		}
		s_nStartLBA = _tcstol(argv[5], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
			return FALSE;
		}
		s_nEndLBA = _tcstol(argv[6], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
			return FALSE;
		}

		if (!_tcscmp(argv[1], _T("data"))) {
			*pExecType = data;
		}
		else if (!_tcscmp(argv[1], _T("audio"))) {
			*pExecType = audio;
			for (INT i = 8; i <= argc; i++) {
				if (!_tcscmp(argv[i - 1], _T("/a"))) {
					pExtArg->byAdd = TRUE;
					if (argc > i && _tcsncmp(argv[i], _T("/"), 1)) {
						pExtArg->nAudioCDOffsetNum = _tcstol(argv[i++], &endptr, 10);
						if (*endptr) {
							OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
							return FALSE;
						}
					}
				}
				else {
					OutputErrorString(_T("Unknown option: [%s]\n"), argv[i - 1]);
					return FALSE;
				}
			}
		}
		printAndSetPath(argv[3]);
	}
	else if (argc == 4 && !_tcscmp(argv[1], _T("fd"))) {
		*pExecType = fd;
		printAndSetPath(argv[3]);
	}
	else if (argc == 3 && !_tcscmp(argv[1], _T("stop"))) {
		*pExecType = stop;
	}
	else if (argc == 3 && !_tcscmp(argv[1], _T("start"))) {
		*pExecType = start;
	}
	else if (argc == 3 && !_tcscmp(argv[1], _T("eject"))) {
		*pExecType = eject;
	}
	else if (argc == 3 && !_tcscmp(argv[1], _T("close"))) {
		*pExecType = closetray;
	}
	else if (argc == 3 && !_tcscmp(argv[1], _T("reset"))) {
		*pExecType = reset;
	}
	else if (argc == 3 && !_tcscmp(argv[1], _T("sub"))) {
		*pExecType = sub;
		printAndSetPath(argv[2]);
	}
	else {
		if (argc > 1) {
			OutputErrorString(_T("Unknown command: [%s]\n"), argv[1]);
		}
		return FALSE;
	}
	return TRUE;
}

int createCmdFile(int argc, _TCHAR* argv[])
{
	if (argc >= 4) {
		FILE* fpCmd = CreateOrOpenFileW(
			argv[3], _T("_cmd"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0);
		if (!fpCmd) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		fwrite(_T(__DATE__), _tcslen(_T(__DATE__)), sizeof(_TCHAR), fpCmd);
		_fputts(_T(" "), fpCmd);
		fwrite(_T(__TIME__), _tcslen(_T(__TIME__)), sizeof(_TCHAR), fpCmd);
		_fputts(_T("\n"), fpCmd);
		for (int i = 0; i < argc; i++) {
			fwrite(argv[i], _tcslen(argv[i]), sizeof(_TCHAR), fpCmd);
			_fputts(_T(" "), fpCmd);
		}
		FcloseAndNull(fpCmd);
	}
	return TRUE;
}

void printUsage(void)
{
	OutputString(
		_T("Usage\n")
		_T("\tcd <DriveLetter> <Filename> <DriveSpeed(0-72)> [/a (val)] [/be or /d8]\n")
		_T("\t   [/c2 (val1) (val2) (val3)] [/f] [/i] [/l] [/m] [/p] [/r]\n")
		_T("\t   [/rc (val)] [/s (val)]\n")
		_T("\t\tRipping a CD from a to z\n")
		_T("\tdata <DriveLetter> <Filename> <DriveSpeed(0-72)> <StartLBA> <EndLBA+1>\n")
		_T("\t\tRipping a CD from start to end (using 'all' flag)\n")
		_T("\taudio <DriveLetter> <Filename> <DriveSpeed(0-72)> <StartLBA> <EndLBA+1>\n")
		_T("\t      [/a (val)]\n")
		_T("\t\tRipping a CD from start to end (using 'cdda' flag)\n")
		_T("\t\tUse this for dumping a lead-in, lead-out mainly\n")
		_T("\tgd <DriveLetter> <Filename> <DriveSpeed(0-72)>\n")
		_T("\t   [/c2 (val1) (val2) (val3)]\n")
		_T("\t\tRipping a HD area of GD from a to z\n")
		_T("\tdvd <DriveLetter> <Filename> <DriveSpeed(0-72)> [/c] [/f]\n")
		_T("\t\tRipping a DVD from a to z\n")
		_T("\tfd <DriveLetter> <Filename>\n")
		_T("\t\tRipping a floppy disk\n")
		);
	_tsystem(_T("pause"));
	OutputString(
		_T("\tstop <DriveLetter>\n")
		_T("\t\tSpin off the disc\n")
		_T("\tstart <DriveLetter>\n")
		_T("\t\tSpin up the disc\n")
		_T("\teject <DriveLetter>\n")
		_T("\t\tEject the tray\n")
		_T("\tclose <DriveLetter>\n")
		_T("\t\tClose the tray\n")
		_T("\treset <DriveLetter>\n")
		_T("\t\tReset the drive (Only PLEXTOR)\n")
		_T("\tsub <Subfile>\n")
		_T("\t\tParse CloneCD sub file\n")
		_T("Option Info\n")
		_T("\t/a\tAdd CD offset manually (Only Audio CD)\n")
		_T("\t\t\tval\tsamples value\n")
		_T("\t/be\tUse 0xbe as ReadCD command forcibly (for data disc)\n")
		_T("\t/d8\tUse 0xd8 as ReadCD command forcibly (for data disc)\n")
		_T("\t/c2\tContinue to read cd to recover C2 error existing sector\n")
		_T("\t\t\tval1\tvalue to reread (default: 1024)\n")
		_T("\t\t\tval2\tvalue to fix a C2 error (default: 4096)\n")
		_T("\t\t\tval3\tvalue to reread speed (default: 4)\n")
		_T("\t/f\tUse 'Force Unit Access' flag to defeat the cache (very slow)\n")
		_T("\t/i\tIgnore invalid ISRC\n")
		_T("\t\t\tFor Valis II [PC-Engine][TTFTTJ10529-3    Q6X3]\n")
		);
		_tsystem(_T("pause"));
	OutputString(
		_T("\t/l\tNot fix SubQ (RMSF, AMSF, CRC) (RMSFs 03:xx:xx and 09:xx:xx)\n")
		_T("\t\t\tFor PlayStation LibCrypt discs\n")
		_T("\t/m\tIf MCN exists in the first pregap sector of the track, use this\n")
		_T("\t\t\tFor some PC-Engine discs\n")
		_T("\t/p\tRipping AMSF from 00:00:00 to 00:01:74\n")
		_T("\t\t\tFor SagaFrontier Original Sound Track (Disc 3) etc.\n")
		_T("\t\t\tSupport drive: PLEXTOR PX-W5224, PREMIUM, PREMIUM2\n")
		_T("\t\t\t               PX-704, 708, 712, 714, 716, 755, 760\n")
		_T("\t/r\tReverse reading CD (including data track)\n")
		_T("\t\t\tFor Alpha-Disc, very slow\n")
		_T("\t/rc\tIf read error, continue reading and ignore c2 error on specific\n")
		_T("\t   \tsector (Only CD)\n")
		_T("\t\t\tFor LaserLock, RingPROTECH, safedisc, smartE\n")
		_T("\t\t\tval\ttimeout value (default: 60)\n")
		_T("\t/s\tIf it reads subchannel precisely, use this\n")
		_T("\t\t\tval\t0: no read next sub (fast, but lack precision)\n")
		_T("\t\t\t   \t1: read next sub (normal, this val is default)\n")
		_T("\t\t\t   \t2: read next & next next sub (slow, precision)\n")
		_T("\t/c\tLog Copyright Management Information\n"));
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
#ifdef UNICODE
	if (_setmode(_fileno(stdin), _O_U8TEXT) == -1) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return EXIT_FAILURE;
	}
	if (_setmode(_fileno(stdout), _O_U8TEXT) == -1) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return EXIT_FAILURE;
	}
	if (_setmode(_fileno(stderr), _O_U8TEXT) == -1) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return EXIT_FAILURE;
	}
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
	EXT_ARG extArg = { 0 };
	if (!checkArg(argc, argv, &execType, &extArg)) {
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

		createCmdFile(argc, argv);
		nRet = exec(argv, &execType, &extArg);
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
#ifdef _DEBUG
	_tsystem(_T("pause"));
#endif
	return nRet = nRet == TRUE ? EXIT_SUCCESS : EXIT_FAILURE;
}

