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
	DEVICE_DATA devData = {0};
	devData.hDevice = CreateFile(szBuf, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(devData.hDevice == INVALID_HANDLE_VALUE) {
		OutputErrorString(_T("Device Open fail\n"));
		return FALSE;
	}
	FILE* fpLog = NULL;
	BOOL bRet = TRUE;
	try {
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
			throw;
		}
#endif
		if(execType == c) {
			StartStop(&devData, START_UNIT_CODE, START_UNIT_CODE);
		}
		else if(execType == s) {
			StartStop(&devData, STOP_UNIT_CODE, STOP_UNIT_CODE);
		}
		else {
			bRet = ReadTestUnitReady(&devData);
			if(!bRet) {
				throw FALSE;
			}
			ULONG ulReturned = 0;
			bRet = DeviceIoControl(devData.hDevice, IOCTL_SCSI_GET_ADDRESS, &devData.adress, 
				sizeof(SCSI_ADDRESS), &devData.adress, sizeof(SCSI_ADDRESS), &ulReturned, NULL);
			OutputScsiAdress(&devData, fpLog);

			STORAGE_DESCRIPTOR_HEADER header = {0};
			STORAGE_PROPERTY_QUERY query;
			query.QueryType = PropertyStandardQuery;
			query.PropertyId = StorageAdapterProperty;

			bRet = DeviceIoControl(devData.hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &query, 
				sizeof(STORAGE_PROPERTY_QUERY), &header, sizeof(STORAGE_DESCRIPTOR_HEADER), &ulReturned, FALSE);

			devData.adapterDescriptor = (PSTORAGE_ADAPTER_DESCRIPTOR)malloc(header.Size);
			if (devData.adapterDescriptor == NULL) {
				throw FALSE;
			}
			ZeroMemory(devData.adapterDescriptor, header.Size);
			bRet = DeviceIoControl(devData.hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &query, 
				sizeof(STORAGE_PROPERTY_QUERY), devData.adapterDescriptor, header.Size, &ulReturned, FALSE);
			OutputStorageAdaptorDescriptor(&devData, fpLog);
#ifdef WIN64
			devData.AlignmentMask64 = (ULONG64)(devData.adapterDescriptor->AlignmentMask) & 0x00000000FFFFFFFF;
#endif
			bRet = ReadDeviceInfo(&devData, fpLog);
			if(!bRet) {
				throw FALSE;
			}
			devData.pszVendorId[8] = '\0';
			devData.pszProductId[16] = '\0';
			if(!strncmp(devData.pszVendorId, "PLEXTOR", 7)) {
				devData.bPlextor = TRUE;
			}
			SetCDSpeed(&devData, _ttoi(argv[3]), fpLog);

			DISC_DATA discData = {0};
			discData.nLastLBAof1stSession = -1;
			discData.nStartLBAof2ndSession = -1;
			bRet = ReadConfiguration(&devData, &discData, fpLog);
			if(!bRet) {
				throw FALSE;
			}
			if(discData.pusCurrentMedia == ProfileCdrom || 
				discData.pusCurrentMedia == ProfileCdRecordable ||
				discData.pusCurrentMedia == ProfileCdRewritable ||
				(discData.pusCurrentMedia == ProfileInvalid && (execType == ra))) {
				TCHAR out[_MAX_PATH] = {0};
				FILE* fpCcd = CreateOrOpenFileW(argv[4], out, NULL, NULL, _T(".ccd"), _T(WFLAG), 0, 0);
				if(!fpCcd) {
					OutputErrorString(_T("Failed to open file .ccd\n"));
					throw FALSE;
				}
				bRet = ReadTOC(&devData, &discData, fpLog);
				if(!bRet) {
					throw FALSE;
				}
				bRet = ReadTOCFull(&devData, &discData, fpLog, fpCcd);
				if(!bRet) {
					throw FALSE;
				}
				if(execType != rall) {
					fclose(fpCcd);
					_tremove(out);
				}

				bRet = ReadCDForSearchingOffset(&devData, &discData, fpLog);

				if(execType == rd) {
					bRet = ReadCDPartial(&devData, argv[4],
						_ttoi(argv[5]), _ttoi(argv[6]), READ_CD_FLAG::All, bDC);
				}
				else if(execType == ra) {
					bRet = ReadCDPartial(&devData, argv[4],
						_ttoi(argv[5]), _ttoi(argv[6]), READ_CD_FLAG::CDDA, bDC);
				}
				else if(bRet == TRUE && execType == rall) {
					bRet = ReadCDAll(&devData, &discData, argv[4], fpLog, fpCcd);
					fclose(fpCcd);
				}
			}
			else if(discData.pusCurrentMedia == ProfileDvdRom || 
				discData.pusCurrentMedia == ProfileDvdRecordable ||
				discData.pusCurrentMedia == ProfileDvdRam || 
				discData.pusCurrentMedia == ProfileDvdRewritable || 
				discData.pusCurrentMedia == ProfileDvdRWSequential || 
				discData.pusCurrentMedia == ProfileDvdDashRDualLayer || 
				discData.pusCurrentMedia == ProfileDvdDashRLayerJump || 
				discData.pusCurrentMedia == ProfileDvdPlusRW || 
//				discData.pusCurrentMedia == ProfileInvalid ||
				discData.pusCurrentMedia == ProfileDvdPlusR) {
				INT nDVDSectorSize = 0;
				bRet = ReadDVDStructure(&devData, &nDVDSectorSize, fpLog);
				if(bRet) {
					if(argv[5] && !_tcscmp(argv[5], _T("raw"))) {
#if 0
						bRet = ReadDVDRaw(&devData, pszVendorId, argv[4], nDVDSectorSize);
#endif
					}
					else {
						bRet = ReadDVD(&devData, argv[4], argv[5], nDVDSectorSize, fpLog);
					}
				}
			}
		}
	}
	catch(BOOL bErr) {
		bRet = bErr;
	}
#ifndef _DEBUG
	FcloseAndNull(fpLog);
#endif
	FreeAndNull(devData.adapterDescriptor);
	CloseHandle(devData.hDevice);
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
		OutputString(
			_T("Usage\n")
//			_T("\t-rall [DriveLetter] [DriveSpeed(0-72)] [filename] <cmi/raw>\n")
			_T("\t-rall [DriveLetter] [DriveSpeed(0-72)] [filename] <cmi>\n")
			_T("\t\tRipping CD or DVD from a to z\n")
			_T("\t\tcmi:Log Copyright Management Information (Only DVD)(Very slow)\n")
//			_T("\t\traw:Ripping Raw mode (Only DVD)\n")
			_T("\t-rd [DriveLetter] [DriveSpeed(0-72)] [filename] [StartLBA] [EndLBA]\n")
			_T("\t\tRipping CD from start to end (data) (Only CD)\n")
			_T("\t-ra [DriveLetter] [DriveSpeed(0-72)] [filename] [StartLBA] [EndLBA]\n")
			_T("\t\tRipping CD from start to end (audio) (Only CD)\n")
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
		_tcsftime(buf, sizeof(buf), _T("%Y-%m-%d(%a) %H:%M:%S"), ts);
		OutputString(_T("Start -> %s\n"), buf);
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
		OutputString(_T("End -> %s\n"), buf);
	}
	return 0;
}

