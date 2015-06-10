/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"
#include "struct.h"
#include "convert.h"
#include "output.h"
#include "outputMMCLog.h"

VOID OutputFsBootRecord(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	_TCHAR str[2][32] = {_T("0")};
#ifdef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 7], 32, str[0], sizeof(str) / sizeof(str[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 39], 32, str[1], sizeof(str) / sizeof(str[1]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
#else
	strncpy(str[0], (PCHAR)&lpBuf[nIdx + 7], sizeof(str[0]));
	strncpy(str[1], (PCHAR)&lpBuf[nIdx + 39], sizeof(str[1]));
#endif
	OutputDiscLog(
		_T("\t                       Boot System Identifier: %s\n")
		_T("\t                              Boot Identifier: %s\n")
		_T("\t                              Boot System Use: "),
		str[0],
		str[1]);
	for (INT i = 71; i <= 2047; i++) {
		OutputDiscLog(_T("%x"), lpBuf[nIdx + i]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputFsPrimaryVolumeDescriptorForTime(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	_TCHAR year[4][4] = {_T("0")};
	_TCHAR month[4][2] = {_T("0")};
	_TCHAR day[4][2] = {_T("0")};
	_TCHAR hour[4][2] = {_T("0")};
	_TCHAR time[4][2] = {_T("0")};
	_TCHAR second[4][2] = {_T("0")};
	_TCHAR milisecond[4][2] = {_T("0")};
#ifdef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 813], 4, year[0], sizeof(year[0]) / sizeof(year[0][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 817], 2, month[0], sizeof(month[0]) / sizeof(month[0][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 819], 2, day[0], sizeof(day[0]) / sizeof(day[0][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 821], 2, hour[0], sizeof(hour[0]) / sizeof(hour[0][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 823], 2, time[0], sizeof(time[0]) / sizeof(time[0][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 825], 2, second[0], sizeof(second[0]) / sizeof(second[0][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 827], 2, milisecond[0], sizeof(milisecond[0]) / sizeof(milisecond[0][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 830], 4, year[1], sizeof(year[1]) / sizeof(year[1][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 834], 2, month[1], sizeof(month[1]) / sizeof(month[1][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 836], 2, day[1], sizeof(day[1]) / sizeof(day[1][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 838], 2, hour[1], sizeof(hour[1]) / sizeof(hour[1][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 840], 2, time[1], sizeof(time[1]) / sizeof(time[1][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 842], 2, second[1], sizeof(second[1]) / sizeof(second[1][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 844], 2, milisecond[1], sizeof(milisecond[1]) / sizeof(milisecond[1][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 847], 4, year[2], sizeof(year[2]) / sizeof(year[2][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 851], 2, month[2], sizeof(month[2]) / sizeof(month[2][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 853], 2, day[2], sizeof(day[2]) / sizeof(day[2][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 855], 2, hour[2], sizeof(hour[2]) / sizeof(hour[2][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 857], 2, time[2], sizeof(time[2]) / sizeof(time[2][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 859], 2, second[2], sizeof(second[2]) / sizeof(second[2][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 861], 2, milisecond[2], sizeof(milisecond[2]) / sizeof(milisecond[2][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 864], 4, year[3], sizeof(year[3]) / sizeof(year[3][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 868], 2, month[3], sizeof(month[3]) / sizeof(month[3][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 870], 2, day[3], sizeof(day[3]) / sizeof(day[3][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 872], 2, hour[3], sizeof(hour[3]) / sizeof(hour[3][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 874], 2, time[3], sizeof(time[3]) / sizeof(time[3][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 876], 2, second[3], sizeof(second[3]) / sizeof(second[3][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 878], 2, milisecond[3], sizeof(milisecond[3]) / sizeof(milisecond[3][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
#else
	strncpy(year[0], (PCHAR)&lpBuf[nIdx + 813], sizeof(year[0]));
	strncpy(month[0], (PCHAR)&lpBuf[nIdx + 817], sizeof(month[0]));
	strncpy(day[0], (PCHAR)&lpBuf[nIdx + 819], sizeof(day[0]));
	strncpy(hour[0], (PCHAR)&lpBuf[nIdx + 821], sizeof(hour[0]));
	strncpy(time[0], (PCHAR)&lpBuf[nIdx + 823], sizeof(time[0]));
	strncpy(second[0], (PCHAR)&lpBuf[nIdx + 825], sizeof(second[0]));
	strncpy(milisecond[0], (PCHAR)&lpBuf[nIdx + 827], sizeof(milisecond[0]));
	strncpy(year[1], (PCHAR)&lpBuf[nIdx + 830], sizeof(year[1]));
	strncpy(month[1], (PCHAR)&lpBuf[nIdx + 834], sizeof(month[1]));
	strncpy(day[1], (PCHAR)&lpBuf[nIdx + 836], sizeof(day[1]));
	strncpy(hour[1], (PCHAR)&lpBuf[nIdx + 838], sizeof(hour[1]));
	strncpy(time[1], (PCHAR)&lpBuf[nIdx + 840], sizeof(time[1]));
	strncpy(second[1], (PCHAR)&lpBuf[nIdx + 842], sizeof(second[1]));
	strncpy(milisecond[1], (PCHAR)&lpBuf[nIdx + 844], sizeof(milisecond[1]));
	strncpy(year[2], (PCHAR)&lpBuf[nIdx + 847], sizeof(year[2]));
	strncpy(month[2], (PCHAR)&lpBuf[nIdx + 851], sizeof(month[2]));
	strncpy(day[2], (PCHAR)&lpBuf[nIdx + 853], sizeof(day[2]));
	strncpy(hour[2], (PCHAR)&lpBuf[nIdx + 855], sizeof(hour[2]));
	strncpy(time[2], (PCHAR)&lpBuf[nIdx + 857], sizeof(time[2]));
	strncpy(second[2], (PCHAR)&lpBuf[nIdx + 859], sizeof(second[2]));
	strncpy(milisecond[2], (PCHAR)&lpBuf[nIdx + 861], sizeof(milisecond[2]));
	strncpy(year[3], (PCHAR)&lpBuf[nIdx + 864], sizeof(year[3]));
	strncpy(month[3], (PCHAR)&lpBuf[nIdx + 868], sizeof(month[3]));
	strncpy(day[3], (PCHAR)&lpBuf[nIdx + 870], sizeof(day[3]));
	strncpy(hour[3], (PCHAR)&lpBuf[nIdx + 872], sizeof(hour[3]));
	strncpy(time[3], (PCHAR)&lpBuf[nIdx + 874], sizeof(time[3]));
	strncpy(second[3], (PCHAR)&lpBuf[nIdx + 876], sizeof(second[3]));
	strncpy(milisecond[3], (PCHAR)&lpBuf[nIdx + 878], sizeof(milisecond[3]));
#endif
	OutputDiscLog(
		_T("\t                Volume Creation Date and Time: %.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.2s +%d\n")
		_T("\t            Volume Modification Date and Time: %.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.2s +%d\n")
		_T("\t              Volume Expiration Date and Time: %.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.2s +%d\n")
		_T("\t               Volume Effective Date and Time: %.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.2s +%d\n")
		_T("\t                       File Structure Version: %d\n")
		_T("\t                              Application Use: "), 
		year[0], month[0], day[0], hour[0], time[0], second[0], milisecond[0], lpBuf[nIdx + 829],
		year[1], month[1], day[1], hour[1], time[1], second[1], milisecond[1], lpBuf[nIdx + 846],
		year[2], month[2], day[2], hour[2], time[2], second[2], milisecond[2], lpBuf[nIdx + 863],
		year[3], month[3], day[3], hour[3], time[3], second[3], milisecond[3], lpBuf[nIdx + 880],
		lpBuf[nIdx + 881]);
	for (INT i = 883; i <= 1394; i++) {
		OutputDiscLog(_T("%x"), lpBuf[nIdx + i]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputFsPrimaryVolumeDescriptorFor1(
	LPBYTE lpBuf,
	_TCHAR str32[][32],
	INT nIdx
	)
{
	OutputDiscLog(
		_T("\t                            System Identifier: %.32s\n")
		_T("\t                            Volume Identifier: %.32s\n")
		_T("\t                            Volume Space Size: %d\n"),
		str32[0],
		str32[1],
		MAKELONG(MAKEWORD(lpBuf[nIdx + 80], lpBuf[nIdx + 81]),
		MAKEWORD(lpBuf[nIdx + 82], lpBuf[nIdx + 83])));
}

VOID OutputFsPrimaryVolumeDescriptorFor2(
	LPBYTE lpBuf,
	_TCHAR str128[][128],
	_TCHAR str37[][37],
	INT nIdx
	)
{
	OutputDiscLog(
		_T("\t                              Volume Set Size: %d\n")
		_T("\t                       Volume Sequence Number: %d\n")
		_T("\t                           Logical Block Size: %d\n")
		_T("\t                              Path Table Size: %d\n")
		_T("\t         Location of Occurrence of Path Table: %d\n")
		_T("\tLocation of Optional Occurrence of Path Table: %d\n")
		_T("\tDirectory Record\n")
		_T("\t\t      Length of Directory Record: %d\n")
		_T("\t\tExtended Attribute Record Length: %d\n")
		_T("\t\t              Location of Extent: %d\n")
		_T("\t\t                     Data Length: %d\n")
		_T("\t\t         Recording Date and Time: %d-%02d-%02d %02d:%02d:%02d +%02d\n")
		_T("\t\t                      File Flags: %d\n")
		_T("\t\t                  File Unit Size: %d\n")
		_T("\t\t             Interleave Gap Size: %d\n")
		_T("\t\t          Volume Sequence Number: %d\n")
		_T("\t\t       Length of File Identifier: %d\n")
		_T("\t\t                 File Identifier: %d\n")
		_T("\t                        Volume Set Identifier: %.128s\n")
		_T("\t                         Publisher Identifier: %.128s\n")
		_T("\t                     Data Preparer Identifier: %.128s\n")
		_T("\t                       Application Identifier: %.128s\n")
		_T("\t                    Copyright File Identifier: %.37s\n")
		_T("\t                     Abstract File Identifier: %.37s\n")
		_T("\t                Bibliographic File Identifier: %.37s\n"), 
		MAKEWORD(lpBuf[nIdx + 120], lpBuf[nIdx + 121]),
		MAKEWORD(lpBuf[nIdx + 124], lpBuf[nIdx + 125]),
		MAKEWORD(lpBuf[nIdx + 128], lpBuf[nIdx + 129]),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 132], lpBuf[nIdx + 133]),
		MAKEWORD(lpBuf[nIdx + 134], lpBuf[nIdx + 135])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 140], lpBuf[nIdx + 141]),
		MAKEWORD(lpBuf[nIdx + 142], lpBuf[nIdx + 143])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 144], lpBuf[nIdx + 145]),
		MAKEWORD(lpBuf[nIdx + 146], lpBuf[nIdx + 147])),
		lpBuf[nIdx + 156],
		lpBuf[nIdx + 157],
		MAKELONG(MAKEWORD(lpBuf[nIdx + 158], lpBuf[nIdx + 159]),
		MAKEWORD(lpBuf[nIdx + 160], lpBuf[nIdx + 161])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 166], lpBuf[nIdx + 167]),
		MAKEWORD(lpBuf[nIdx + 168], lpBuf[nIdx + 169])),
		lpBuf[nIdx + 174] + 1900, lpBuf[nIdx + 175], lpBuf[nIdx + 176], lpBuf[nIdx + 177], lpBuf[nIdx + 178], lpBuf[nIdx + 179], lpBuf[nIdx + 180],
		lpBuf[nIdx + 181],
		lpBuf[nIdx + 182],
		lpBuf[nIdx + 183],
		MAKEWORD(lpBuf[nIdx + 184], lpBuf[nIdx + 185]),
		lpBuf[nIdx + 188],
		lpBuf[nIdx + 189],
		str128[0],
		str128[1],
		str128[2],
		str128[3],
		str37[0],
		str37[1],
		str37[2]);
}

VOID OutputFsPrimaryVolumeDescriptorForISO9660(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	_TCHAR str32[3][32] = {_T("0")};
	_TCHAR str128[4][128] = {_T("0")};
	_TCHAR str37[3][37] = {_T("0")};
#ifdef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 8], 32, str32[0], sizeof(str32[0]) / sizeof(str32[0][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 40], 32, str32[1], sizeof(str32[1]) / sizeof(str32[1][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 190], 128, str128[0], sizeof(str128[0]) / sizeof(str128[0][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 318], 128, str128[1], sizeof(str128[1]) / sizeof(str128[1][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 446], 128, str128[2], sizeof(str128[2]) / sizeof(str128[2][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 574], 128, str128[3], sizeof(str128[3]) / sizeof(str128[3][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 702], 37, str37[0], sizeof(str37[0]) / sizeof(str37[0][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 739], 37, str37[1], sizeof(str37[1]) / sizeof(str37[1][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 776], 37, str37[2], sizeof(str37[2]) / sizeof(str37[2][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
#else
	strncpy(str32[0], (PCHAR)&lpBuf[nIdx + 8], sizeof(str32[0]));
	strncpy(str32[1], (PCHAR)&lpBuf[nIdx + 40], sizeof(str32[1]));
	strncpy(str128[0], (PCHAR)&lpBuf[nIdx + 190], sizeof(str128[0]));
	strncpy(str128[1], (PCHAR)&lpBuf[nIdx + 318], sizeof(str128[1]));
	strncpy(str128[2], (PCHAR)&lpBuf[nIdx + 446], sizeof(str128[2]));
	strncpy(str128[3], (PCHAR)&lpBuf[nIdx + 574], sizeof(str128[3]));
	strncpy(str37[0], (PCHAR)&lpBuf[nIdx + 702], sizeof(str37[0]));
	strncpy(str37[1], (PCHAR)&lpBuf[nIdx + 739], sizeof(str37[1]));
	strncpy(str37[2], (PCHAR)&lpBuf[nIdx + 776], sizeof(str37[2]));
#endif
	OutputFsPrimaryVolumeDescriptorFor1(lpBuf, str32, nIdx);
	if (lpBuf[nIdx + 0] == 2) {
#ifdef UNICODE
		if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 88], 32, str32[2], sizeof(str32[2]) / sizeof(str32[2][0]))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		}
#else
		strncpy(str32[2], (PCHAR)&lpBuf[nIdx + 88], 32);
#endif
		OutputDiscLog(_T(
			"\t                             Escape Sequences: %.32s\n"), str32[2]);
	}

	OutputFsPrimaryVolumeDescriptorFor2(lpBuf, str128, str37, nIdx);
	OutputFsPrimaryVolumeDescriptorForTime(lpBuf, nIdx);
}

VOID OutputFsPrimaryVolumeDescriptorForJoliet(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	_TCHAR str32[3][32] = { 0 };
	_TCHAR str128[4][128] = { 0 };
	_TCHAR str37[3][37] = { 0 };
#ifdef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 8], 16, str32[0], sizeof(str32[0]) / sizeof(str32[0][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	LittleToBig(str32[0], (_TCHAR*)&lpBuf[nIdx + 8], 16);
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 40], 16, str32[1], sizeof(str32[1]) / sizeof(str32[1][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	LittleToBig(str32[1], (_TCHAR*)&lpBuf[nIdx + 40], 16);
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 190], 16, str128[0], sizeof(str128[0]) / sizeof(str128[0][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	LittleToBig(str128[0], (_TCHAR*)&lpBuf[nIdx + 190], 64);
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 318], 16, str128[1], sizeof(str128[1]) / sizeof(str128[1][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	LittleToBig(str128[1], (_TCHAR*)&lpBuf[nIdx + 318], 64);
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 446], 16, str128[2], sizeof(str128[2]) / sizeof(str128[2][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	LittleToBig(str128[2], (_TCHAR*)&lpBuf[nIdx + 446], 64);
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 574], 16, str128[3], sizeof(str128[3]) / sizeof(str128[3][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	LittleToBig(str128[3], (_TCHAR*)&lpBuf[nIdx + 574], 64);
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 702], 16, str37[0], sizeof(str37[0]) / sizeof(str37[0][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	LittleToBig(str37[0], (_TCHAR*)&lpBuf[nIdx + 702], 18);
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 739], 16, str37[1], sizeof(str37[1]) / sizeof(str37[1][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	LittleToBig(str37[1], (_TCHAR*)&lpBuf[nIdx + 739], 18);
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 776], 16, str37[2], sizeof(str37[2]) / sizeof(str37[2][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	LittleToBig(str37[2], (_TCHAR*)&lpBuf[nIdx + 776], 18);
#else
	_TCHAR tmp32[3][32 + 1] = { 0 };
	_TCHAR tmp128[4][128 + 1] = { 0 };
	_TCHAR tmp37[3][37 + 1] = { 0 };
	LittleToBig(tmp32[0], (_TCHAR*)&lpBuf[nIdx + 8], 32);
	if (!WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)&tmp32[0], 16, str32[0], sizeof(str32[0]), NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	LittleToBig(tmp32[1], (_TCHAR*)&lpBuf[nIdx + 40], 32);
	if (!WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)&tmp32[1], 16, str32[1], sizeof(str32[1]), NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	LittleToBig(tmp128[0], (_TCHAR*)&lpBuf[nIdx + 190], 128);
	if (!WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)&tmp128[0], 64, str128[0], sizeof(str128[0]), NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	LittleToBig(tmp128[1], (_TCHAR*)&lpBuf[nIdx + 318], 128);
	if (!WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)&tmp128[1], 64, str128[1], sizeof(str128[1]), NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	LittleToBig(tmp128[2], (_TCHAR*)&lpBuf[nIdx + 446], 128);
	if (!WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)&tmp128[2], 64, str128[2], sizeof(str128[2]), NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	LittleToBig(tmp128[3], (_TCHAR*)&lpBuf[nIdx + 574], 128);
	if (!WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)&tmp128[3], 64, str128[3], sizeof(str128[3]), NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	LittleToBig(tmp37[0], (_TCHAR*)&lpBuf[nIdx + 702], 36);
	if (!WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)&tmp37[0], 18, str37[0], sizeof(str37[0]), NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	LittleToBig(tmp37[1], (_TCHAR*)&lpBuf[nIdx + 739], 36);
	if (!WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)&tmp37[1], 18, str37[1], sizeof(str37[1]), NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	LittleToBig(tmp37[2], (_TCHAR*)&lpBuf[nIdx + 776], 36);
	if (!WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)&tmp37[2], 18, str37[2], sizeof(str37[2]), NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
#endif
	OutputFsPrimaryVolumeDescriptorFor1(lpBuf, str32, nIdx);

#ifdef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 88], 16, str32[2], sizeof(str32[2]) / sizeof(str32[2][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	LittleToBig(str32[2], (_TCHAR*)&lpBuf[nIdx + 88], 16);
#else
	LittleToBig(tmp32[2], (_TCHAR*)&lpBuf[nIdx + 88], 32);
	if (!WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)&tmp32[2], 16, str32[2], sizeof(str32[2]), NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
#endif
	OutputDiscLog(_T(
		"\t                             Escape Sequences: %.32s\n"), str32[2]);
	OutputFsPrimaryVolumeDescriptorFor2(lpBuf, str128, str37, nIdx);
	OutputFsPrimaryVolumeDescriptorForTime(lpBuf, nIdx);
}

VOID OutputFsVolumePartitionDescriptor(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	_TCHAR str[2][32] = {_T("0")};
#ifdef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 8], 32, str[0], sizeof(str[0]) / sizeof(str[0][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 40], 32, str[1], sizeof(str[1]) / sizeof(str[1][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
#else
	strncpy(str[0], (PCHAR)&lpBuf[nIdx + 8], 32);
	strncpy(str[1], (PCHAR)&lpBuf[nIdx + 40], 32);
#endif
	OutputDiscLog(
		_T("\t          System Identifier: %.32s\n")
		_T("\tVolume Partition Identifier: %.32s\n")
		_T("\t  Volume Partition Location: %d\n")
		_T("\t      Volume Partition Size: %d\n")
		_T("\t                 System Use: "),
		str[0],
		str[1], 
		MAKELONG(MAKEWORD(lpBuf[nIdx + 76], lpBuf[nIdx + 77]),
		MAKEWORD(lpBuf[nIdx + 78], lpBuf[nIdx + 79])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 84], lpBuf[nIdx + 85]),
		MAKEWORD(lpBuf[nIdx + 86], lpBuf[nIdx + 87])));
	for (INT i = 88; i <= 2047; i++) {
		OutputDiscLog(_T("%x"), lpBuf[nIdx + i]);
	}
	OutputDiscLog(_T("\n"));
}
VOID OutputFsVolumeDescriptor(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	// 0 is Boot Record. 
	// 1 is Primary Volume Descriptor. 
	// 2 is Supplementary Volume Descriptor. 
	// 3 is Volume Partition Descriptor. 
	// 4-254 is reserved. 
	// 255 is Volume Descriptor Set Terminator.
	_TCHAR str[5] = { 0 };
#ifdef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 1], 5, str, sizeof(str) / sizeof(str[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
#else
	strncpy(str, (PCHAR)&lpBuf[nIdx + 1], sizeof(str));
#endif
	OutputDiscLog(
		_T("Volume Descriptor\n")
		_T("\t                       Volume Descriptor Type: %d\n")
		_T("\t                          Standard Identifier: %.5s\n")
		_T("\t                    Volume Descriptor Version: %d\n"),
		lpBuf[nIdx],
		str,
		lpBuf[nIdx + 6]);

	if (lpBuf[nIdx] == 0) {
		OutputFsBootRecord(lpBuf, nIdx);
	}
	else if (lpBuf[nIdx] == 1) {
		OutputFsPrimaryVolumeDescriptorForISO9660(lpBuf, nIdx);
	}
	else if (lpBuf[nIdx] == 2) {
		OutputFsPrimaryVolumeDescriptorForJoliet(lpBuf, nIdx);
	}
	else if (lpBuf[nIdx] == 3) {
		OutputFsVolumePartitionDescriptor(lpBuf, nIdx);
	}
	else if (lpBuf[nIdx] == 255) {
	}
}

VOID OutputFsVolumeStructureDescriptorFormat(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	_TCHAR str[5] = { 0 };
#ifdef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 1], 5, str, sizeof(str) / sizeof(str[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
#else
	strncpy(str, (PCHAR)&lpBuf[nIdx + 1], sizeof(str));
#endif
	OutputDiscLog(
		_T("Volume Recognition Sequence\n")
		_T("\t                               Structure Type: %d\n")
		_T("\t                          Standard Identifier: %.5s\n")
		_T("\t                            Structure Version: %d\n"),
		lpBuf[nIdx],
		str,
		lpBuf[nIdx + 6]);
}

VOID OutputFsRecordingDateAndTime(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	OutputDiscLog(_T(
		"\tRecording Date and Time: %x %d-%02d-%02d %02d:%02d:%02d.%d.%d.%d\n"),
		MAKEWORD(lpBuf[nIdx], lpBuf[nIdx + 1]), MAKEWORD(lpBuf[nIdx + 2], lpBuf[nIdx + 3]),
		lpBuf[nIdx + 4], lpBuf[nIdx + 5], lpBuf[nIdx + 6], lpBuf[nIdx + 7], lpBuf[nIdx + 8],
		lpBuf[nIdx + 9], lpBuf[nIdx + 10], lpBuf[nIdx + 11]);
}

VOID OutputFsBootDescriptor(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	_TCHAR str8[2][8] = {_T("0")};
	_TCHAR str23[2][23] = {_T("0")};
#ifdef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 9], 23, str23[0], sizeof(str23[0]) / sizeof(str23[0][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 24], 8, str8[0], sizeof(str8[0]) / sizeof(str8[0][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 41], 23, str23[1], sizeof(str23[1]) / sizeof(str23[1][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 64], 8, str8[1], sizeof(str8[1]) / sizeof(str8[1][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
#else
	strncpy(str23[0], (PCHAR)&lpBuf[nIdx + 9], sizeof(str23[0]));
	strncpy(str8[0], (PCHAR)&lpBuf[nIdx + 24], sizeof(str8[0]));
	strncpy(str23[1], (PCHAR)&lpBuf[nIdx + 41], sizeof(str23[1]));
	strncpy(str8[1], (PCHAR)&lpBuf[nIdx + 64], sizeof(str8[1]));
#endif
	OutputDiscLog(
		_T("\tArchitecture Type\n")
		_T("\t\t            Flags: %d\n")
		_T("\t\t       Identifier: %.23s\n")
		_T("\t\tIdentifier Suffix: %.8s\n")
		_T("\tBoot Identifier\n")
		_T("\t\t            Flags: %d\n")
		_T("\t\t       Identifier: %.23s\n")
		_T("\t\tIdentifier Suffix: %.8s\n")
		_T("\tBoot Extent Location: %d\n")
		_T("\t  Boot Extent Length: %d\n")
		_T("\t        Load Address: %d%d\n")
		_T("\t       Start Address: %d%d\n"),
		lpBuf[nIdx + 8],
		str23[0],
		str8[0],
		lpBuf[nIdx + 40],
		str23[1],
		str8[1],
		MAKELONG(MAKEWORD(lpBuf[nIdx + 72], lpBuf[nIdx + 73]),
		MAKEWORD(lpBuf[nIdx + 74], lpBuf[nIdx + 75])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 76], lpBuf[nIdx + 77]),
		MAKEWORD(lpBuf[nIdx + 78], lpBuf[nIdx + 79])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 80], lpBuf[nIdx + 81]),
		MAKEWORD(lpBuf[nIdx + 82], lpBuf[nIdx + 83])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 84], lpBuf[nIdx + 85]),
		MAKEWORD(lpBuf[nIdx + 86], lpBuf[nIdx + 87])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 88], lpBuf[nIdx + 89]),
		MAKEWORD(lpBuf[nIdx + 90], lpBuf[nIdx + 91])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 92], lpBuf[nIdx + 93]),
		MAKEWORD(lpBuf[nIdx + 94], lpBuf[nIdx + 95])));

	OutputFsRecordingDateAndTime(lpBuf, nIdx + 96);
	OutputDiscLog(
		_T("\t               Flags: %d\n")
		_T("\t            Boot Use: "),
		MAKEWORD(lpBuf[nIdx + 108], lpBuf[nIdx + 109]));
	for (INT i = 142; i <= 2047; i++) {
		OutputDiscLog(_T("%x"), lpBuf[nIdx + i]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputFsVolumeRecognitionSequence(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	_TCHAR str[5] = { 0 };
#ifdef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 1], 5, str, sizeof(str) / sizeof(str[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
#else
	strncpy(str, (PCHAR)&lpBuf[nIdx + 1], sizeof(str));
#endif
	if (lpBuf[nIdx] == 1 && !_tcsncmp(str, _T("CD001"), 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nIdx);
		OutputFsPrimaryVolumeDescriptorForISO9660(lpBuf, nIdx);
	}
	else if (lpBuf[nIdx] == 2 && !_tcsncmp(str, _T("CD001"), 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nIdx);
		OutputDiscLog(_T("\t                                 Volume Flags: %d\n"), lpBuf[nIdx + 7]);
		OutputFsPrimaryVolumeDescriptorForJoliet(lpBuf, nIdx);
	}
	else if (lpBuf[nIdx] == 255 && !_tcsncmp(str, _T("CD001"), 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nIdx);
	}
	else if (lpBuf[nIdx] == 0 && !_tcsncmp(str, _T("BOOT2"), 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nIdx);
		OutputFsBootDescriptor(lpBuf, nIdx);
	}
	else if (lpBuf[nIdx] == 0 && !_tcsncmp(str, _T("BEA01"), 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nIdx);
	}
	else if (lpBuf[nIdx] == 0 && !_tcsncmp(str, _T("NSR02"), 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nIdx);
	}
	else if (lpBuf[nIdx] == 0 && !_tcsncmp(str, _T("NSR03"), 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nIdx);
	}
	else if (lpBuf[nIdx] == 0 && !_tcsncmp(str, _T("TEA01"), 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nIdx);
	}
}

VOID OutputFsCharspec(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	_TCHAR str[23] = { 0 };
#ifdef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 1], 23, str, sizeof(str) / sizeof(str[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
#else
	strncpy(str, (PCHAR)&lpBuf[nIdx + 1], sizeof(str));
#endif
	OutputDiscLog(
		_T("\t\t       Character Set Type: %d\n")
		_T("\t\tCharacter Set Information: %.23s\n"),
		lpBuf[nIdx], str);
}

VOID OutputFsExtentDescriptor(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	OutputDiscLog(
		_T("\t\t  Extent Length: %d\n")
		_T("\t\tExtent Location: %d\n"),
		MAKELONG(MAKEWORD(lpBuf[nIdx], lpBuf[nIdx + 1]),
		MAKEWORD(lpBuf[nIdx + 2], lpBuf[nIdx + 3])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 4], lpBuf[nIdx + 5]),
		MAKEWORD(lpBuf[nIdx + 6], lpBuf[nIdx + 7])));
}

VOID OutputFsRegid(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	_TCHAR str[23] = { 0 };
#ifdef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 1], 23, str, sizeof(str) / sizeof(str[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
#else
	strncpy(str, (PCHAR)&lpBuf[nIdx + 1], sizeof(str));
#endif
	OutputDiscLog(
		 _T("\t\t            Flags: %d\n")
		 _T("\t\t       Identifier: %.23s\n")
		 _T("\t\tIdentifier Suffix: "),
		lpBuf[nIdx],
		str);
	for (INT i = 24; i <= 31; i++) {
		OutputDiscLog(_T("%x"), lpBuf[nIdx + i]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputFsPrimaryVolumeDescriptorForUDF(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	_TCHAR str32[32] = { 0 };
	_TCHAR str128[128] = { 0 };
#ifdef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 24], 32, str32, sizeof(str32) / sizeof(str32[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 72], 128, str128, sizeof(str128) / sizeof(str128[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
#else
	strncpy(str32, (PCHAR)&lpBuf[nIdx + 24], sizeof(str32));
	strncpy(str128, (PCHAR)&lpBuf[nIdx + 72], sizeof(str128));
#endif
	OutputDiscLog(
		_T("\tVolume Descriptor Sequence Number: %d\n")
		_T("\t Primary Volume Descriptor Number: %d\n")
		_T("\t                Volume Identifier: %.32s\n")
		_T("\t           Volume Sequence Number: %d\n")
		_T("\t   Maximum Volume Sequence Number: %d\n")
		_T("\t                Interchange Level: %d\n")
		_T("\t        Maximum Interchange Level: %d\n")
		_T("\t               Character Set List: %d\n")
		_T("\t       Maximum Character Set List: %d\n")
		_T("\t            Volume Set Identifier: %.128s\n")
		_T("\tDescriptor Character Set\n"), 
		MAKELONG(MAKEWORD(lpBuf[nIdx + 16], lpBuf[nIdx + 17]),
		MAKEWORD(lpBuf[nIdx + 18], lpBuf[nIdx + 19])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 20], lpBuf[nIdx + 21]),
		MAKEWORD(lpBuf[nIdx + 22], lpBuf[nIdx + 23])),
		str32,
		MAKEWORD(lpBuf[nIdx + 56], lpBuf[nIdx + 57]),
		MAKEWORD(lpBuf[nIdx + 58], lpBuf[nIdx + 59]),
		MAKEWORD(lpBuf[nIdx + 60], lpBuf[nIdx + 61]),
		MAKEWORD(lpBuf[nIdx + 62], lpBuf[nIdx + 63]),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 64], lpBuf[nIdx + 65]),
		MAKEWORD(lpBuf[nIdx + 66], lpBuf[nIdx + 67])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 68], lpBuf[nIdx + 69]),
		MAKEWORD(lpBuf[nIdx + 70], lpBuf[nIdx + 71])),
		str128);

	OutputFsCharspec(lpBuf, nIdx + 200);

	OutputDiscLog(_T("\tExplanatory Character Set\n"));
	OutputFsCharspec(lpBuf, nIdx + 264);

	OutputDiscLog(_T("\tVolume Abstract\n"));
	OutputFsExtentDescriptor(lpBuf, nIdx + 328);

	OutputDiscLog(_T("\tVolume Copyright Notice\n"));
	OutputFsExtentDescriptor(lpBuf, nIdx + 336);

	OutputDiscLog(_T("\tApplication Identifier\n"));
	OutputFsRegid(lpBuf, nIdx + 344);

	OutputFsRecordingDateAndTime(lpBuf, nIdx + 376);

	OutputDiscLog(_T("\tImplementation Identifier\n"));
	OutputFsRegid(lpBuf, nIdx + 388);
	OutputDiscLog(_T("\tImplementation Use: "));
	for (INT i = 420; i <= 483; i++) {
		OutputDiscLog(_T("%x"), lpBuf[nIdx + i]);
	}
	OutputDiscLog(_T("\n"));

	OutputDiscLog(_T(
		"\tPredecessor Volume Descriptor Sequence Location: %d\n"), 
		MAKELONG(MAKEWORD(lpBuf[nIdx + 484], lpBuf[nIdx + 485]),
		MAKEWORD(lpBuf[nIdx + 486], lpBuf[nIdx + 487])));
	OutputDiscLog(_T(
		"\t                                          Flags: %d\n"), 
		MAKEWORD(lpBuf[nIdx + 488], lpBuf[nIdx + 489]));
}

VOID OutputFsAnchorVolumeDescriptorPointer(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	OutputDiscLog(_T("\tMain Volume Descriptor Sequence Extent\n"));
	OutputFsExtentDescriptor(lpBuf, nIdx + 16);
	OutputDiscLog(_T("\tReserve Volume Descriptor Sequence Extent\n"));
	OutputFsExtentDescriptor(lpBuf, nIdx + 24);
}

VOID OutputFsVolumeDescriptorPointer(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	OutputDiscLog(
		_T("\t     Volume Descriptor Sequence Number: %d\n")
		_T("\tNext Volume Descriptor Sequence Extent\n"), 
		MAKELONG(MAKEWORD(lpBuf[nIdx + 16], lpBuf[nIdx + 17]),
		MAKEWORD(lpBuf[nIdx + 18], lpBuf[nIdx + 19])));
	OutputFsExtentDescriptor(lpBuf, nIdx + 20);
}

VOID OutputFsImplementationUseVolumeDescriptor(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	OutputDiscLog(
		_T("\tVolume Descriptor Sequence Number: %d\n")
		_T("\tImplementation Identifier\n"), 
		MAKELONG(MAKEWORD(lpBuf[nIdx + 16], lpBuf[nIdx + 17]),
		MAKEWORD(lpBuf[nIdx + 18], lpBuf[nIdx + 19])));

	OutputFsRegid(lpBuf, nIdx + 20);

	OutputDiscLog(_T("\tLVI Charset\n"));
	OutputFsCharspec(lpBuf, nIdx + 52);

	_TCHAR str128[128] = {_T("0")};
	_TCHAR str36[3][128] = {_T("0")};
#ifdef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 116], 128, str128, sizeof(str128) / sizeof(str128[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 244], 36, str36[0], sizeof(str36[0]) / sizeof(str36[0][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 280], 36, str36[1], sizeof(str36[1]) / sizeof(str36[1][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 316], 36, str36[2], sizeof(str36[2]) / sizeof(str36[2][0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
#else
	strncpy(str128, (PCHAR)&lpBuf[nIdx + 116], sizeof(str128));
	strncpy(str36[0], (PCHAR)&lpBuf[nIdx + 244], sizeof(str36[0]));
	strncpy(str36[1], (PCHAR)&lpBuf[nIdx + 280], sizeof(str36[1]));
	strncpy(str36[2], (PCHAR)&lpBuf[nIdx + 316], sizeof(str36[2]));
#endif
	OutputDiscLog(_T("\tLogical Volume Identifier: %.128s\n"), str128);
	for (INT i = 1; i < 4; i++) {
		OutputDiscLog(_T("\t               LV Info %d: %.36s\n"), i, str36[i-1]);
	}
	OutputDiscLog(_T("\tImplemention ID\n"));
	OutputFsRegid(lpBuf, nIdx + 352);
	OutputDiscLog(_T("\tImplementation Use: "));
	for (INT i = 384; i <= 511; i++) {
		OutputDiscLog(_T("%x"), lpBuf[nIdx + i]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputFsPartitionDescriptor(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	OutputDiscLog(
		_T("\tVolume Descriptor Sequence Number: %d\n")
		_T("\t                  Partition Flags: %d\n")
		_T("\t                 Partition Number: %d\n")
		_T("\tPartition Contents\n"), 
		MAKELONG(MAKEWORD(lpBuf[nIdx + 16], lpBuf[nIdx + 17]),
		MAKEWORD(lpBuf[nIdx + 18], lpBuf[nIdx + 19])),
		MAKEWORD(lpBuf[nIdx + 20], lpBuf[nIdx + 21]),
		MAKEWORD(lpBuf[nIdx + 22], lpBuf[nIdx + 23]));

	OutputFsRegid(lpBuf, nIdx + 24);

	OutputDiscLog(_T("\tPartition Contents Use: "));
	for (INT i = 56; i <= 183; i++) {
		OutputDiscLog(_T("%x"), lpBuf[nIdx + i]);
	}
	OutputDiscLog(_T("\n"));
	OutputDiscLog(
		_T("\t                Access Type: %d\n")
		_T("\tPartition Starting Location: %d\n")
		_T("\t           Partition Length: %d\n")
		_T("\tImplementation Identifier\n"), 
		MAKELONG(MAKEWORD(lpBuf[nIdx + 184], lpBuf[nIdx + 185]),
		MAKEWORD(lpBuf[nIdx + 186], lpBuf[nIdx + 187])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 188], lpBuf[nIdx + 189]),
		MAKEWORD(lpBuf[nIdx + 190], lpBuf[nIdx + 191])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 192], lpBuf[nIdx + 193]),
		MAKEWORD(lpBuf[nIdx + 194], lpBuf[nIdx + 195])));

	OutputFsRegid(lpBuf, nIdx + 196);
	OutputDiscLog(_T("\tImplementation Use: "));
	for (INT i = 228; i <= 355; i++) {
		OutputDiscLog(_T("%x"), lpBuf[nIdx + i]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputFsLongAllocationDescriptor(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	OutputDiscLog(
		_T("\tLongAllocationDescriptor\n")
		_T("\t\t             Extent Length: %d\n")
		_T("\t\t      Logical Block Number: %d\n")
		_T("\t\tPartition Reference Number: %d\n"), 
		MAKELONG(MAKEWORD(lpBuf[nIdx], lpBuf[nIdx + 1]),
		MAKEWORD(lpBuf[nIdx + 2], lpBuf[nIdx + 3])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 4], lpBuf[nIdx + 5]),
		MAKEWORD(lpBuf[nIdx + 6], lpBuf[nIdx + 7])),
		MAKEWORD(lpBuf[nIdx + 8], lpBuf[nIdx + 9]));
}

VOID OutputFsLogicalVolumeDescriptor(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	OutputDiscLog(
		_T("\tVolume Descriptor Sequence Number: %d\n")
		_T("\tDescriptor Character Set\n"), 
		MAKELONG(MAKEWORD(lpBuf[nIdx + 16], lpBuf[nIdx + 17]),
		MAKEWORD(lpBuf[nIdx + 18], lpBuf[nIdx + 19])));

	OutputFsCharspec(lpBuf, nIdx + 20);

	_TCHAR str[128] = { 0 };
#ifdef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 84], 128, str, sizeof(str) / sizeof(str[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
#else
	strncpy(str, (PCHAR)&lpBuf[nIdx + 84], 128);
#endif
	OutputDiscLog(
		_T("\tLogical Volume Identifier: %.128s\n")
		_T("\t      Logical Block Size : %d\n")
		_T("\tDomain Identifier\n"),
		str, 
		MAKELONG(MAKEWORD(lpBuf[nIdx + 212], lpBuf[nIdx + 213]),
		MAKEWORD(lpBuf[nIdx + 214], lpBuf[nIdx + 215])));

	OutputFsCharspec(lpBuf, nIdx + 216);
	OutputFsLongAllocationDescriptor(lpBuf, nIdx + 248);

	LONG MT_L = MAKELONG(MAKEWORD(lpBuf[nIdx + 264], lpBuf[nIdx + 265]),
		MAKEWORD(lpBuf[nIdx + 266], lpBuf[nIdx + 267]));
	OutputDiscLog(
		_T("\t        Map Table Length: %d\n")
		_T("\tNumber of Partition Maps: %d\n")
		_T("\tImplementation Identifier\n"),
		MT_L, 
		MAKELONG(MAKEWORD(lpBuf[nIdx + 268], lpBuf[nIdx + 269]),
		MAKEWORD(lpBuf[nIdx + 270], lpBuf[nIdx + 271])));

	OutputFsRegid(lpBuf, nIdx + 272);

	OutputDiscLog(_T("\tImplementation Use: "));
	for (INT i = 304; i <= 431; i++) {
		OutputDiscLog(_T("%x"), lpBuf[nIdx + i]);
	}
	OutputDiscLog(_T("\n"));
	OutputDiscLog(_T("\tIntegrity Sequence Extent\n"));
	OutputFsExtentDescriptor(lpBuf, nIdx + 432);

	OutputDiscLog(_T("\tPartition Maps: "));
	for (INT i = 0; i < MT_L; i++) {
		OutputDiscLog(_T("%x"), lpBuf[nIdx + 440 + i]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputFsUnallocatedSpaceDescriptor(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	LONG N_AD = MAKELONG(MAKEWORD(lpBuf[nIdx + 20], lpBuf[nIdx + 21]),
		MAKEWORD(lpBuf[nIdx + 22], lpBuf[nIdx + 23]));
	OutputDiscLog(
		_T("\tVolume Descriptor Sequence Number: %d\n")
		_T("\t Number of Allocation Descriptors: %d\n")
		_T("\tAllocation Descriptors\n"), 
		MAKELONG(MAKEWORD(lpBuf[nIdx + 16], lpBuf[nIdx + 17]),
		MAKEWORD(lpBuf[nIdx + 18], lpBuf[nIdx + 19])),
		N_AD);
	for (INT i = 0; i < N_AD * 8; i+=8) {
		OutputFsExtentDescriptor(lpBuf, nIdx + 24 + i);
	}
}

VOID OutputFsVolumeDescriptorSequence(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	WORD wTagId = MAKEWORD(lpBuf[nIdx], lpBuf[nIdx + 1]);
	if (wTagId == 0 || (10 <= wTagId && wTagId <= 255) || 267 <= wTagId) {
		return;
	}
	switch (wTagId) {
	case 1:
		OutputDiscLog(_T("Primary Volume Descriptor\n"));
		break;
	case 2:
		OutputDiscLog(_T("Anchor Volume Descriptor Pointer\n"));
		break;
	case 3:
		OutputDiscLog(_T("Volume Descriptor Pointer\n"));
		break;
	case 4:
		OutputDiscLog(_T("Implementation Use Volume Descriptor\n"));
		break;
	case 5:
		OutputDiscLog(_T("Partition Descriptor\n"));
		break;
	case 6:
		OutputDiscLog(_T("Logical Volume Descriptor\n"));
		break;
	case 7:
		OutputDiscLog(_T("Unallocated Space Descriptor\n"));
		break;
	case 8:
		OutputDiscLog(_T("Terminating Descriptor\n"));
		break;
	case 9:
		OutputDiscLog(_T("Logical Volume Integrity Descriptor\n"));
		break;
	case 256:
		OutputDiscLog(_T("File Set Descriptor\n"));
		break;
	case 257:
		OutputDiscLog(_T("File Identifier Descriptor\n"));
		break;
	case 258:
		OutputDiscLog(_T("Allocation Extent Descriptor\n"));
		break;
	case 259:
		OutputDiscLog(_T("Indirect Entry\n"));
		break;
	case 260:
		OutputDiscLog(_T("Terminal Entry\n"));
		break;
	case 261:
		OutputDiscLog(_T("File Entry\n"));
		break;
	case 262:
		OutputDiscLog(_T("Extended Attribute Header Descriptor\n"));
		break;
	case 263:
		OutputDiscLog(_T("Unallocated Space Entry\n"));
		break;
	case 264:
		OutputDiscLog(_T("Space Bitmap Descriptor\n"));
		break;
	case 265:
		OutputDiscLog(_T("Partition Integrity Entry\n"));
		break;
	case 266:
		OutputDiscLog(_T("Extended File Entry\n"));
		break;
	}

	OutputDiscLog(
		_T("\t\t           Descriptor Version: %d\n")
		_T("\t\t                 Tag Checksum: %d\n")
		_T("\t\t            Tag Serial Number: %d\n")
		_T("\t\t               Descriptor CRC: %x\n")
		_T("\t\t        Descriptor CRC Length: %d\n")
		_T("\t\t                 Tag Location: %d\n"), 
		MAKEWORD(lpBuf[nIdx + 2], lpBuf[nIdx + 3]),
		lpBuf[nIdx + 4],
		MAKEWORD(lpBuf[nIdx + 6], lpBuf[nIdx + 7]),
		MAKEWORD(lpBuf[nIdx + 8], lpBuf[nIdx + 9]),
		MAKEWORD(lpBuf[nIdx + 10], lpBuf[nIdx + 11]),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 12], lpBuf[nIdx + 13]),
		MAKEWORD(lpBuf[nIdx + 14], lpBuf[nIdx + 15])));

	switch (wTagId) {
	case 1:
		OutputFsPrimaryVolumeDescriptorForUDF(lpBuf, nIdx);
		break;
	case 2:
		OutputFsAnchorVolumeDescriptorPointer(lpBuf, nIdx);
		break;
	case 3:
		OutputFsVolumeDescriptorPointer(lpBuf, nIdx);
		break;
	case 4:
		OutputFsImplementationUseVolumeDescriptor(lpBuf, nIdx);
		break;
	case 5:
		OutputFsPartitionDescriptor(lpBuf, nIdx);
		break;
	case 6:
		OutputFsLogicalVolumeDescriptor(lpBuf, nIdx);
		break;
	case 7:
		OutputFsUnallocatedSpaceDescriptor(lpBuf, nIdx);
		break;
	}
	return;
}
// end for DVD

VOID OutputFsMasterDirectoryBlocks(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	_TCHAR str[27] = { 0 };
#ifdef UNICODE
	str[0] = lpBuf[nIdx+36];
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)&lpBuf[nIdx + 37], 26, str + 1, sizeof(str) / sizeof(str[0]) - 1)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
#else
	strncpy(str, (PCHAR)&lpBuf[nIdx + 36], sizeof(str));
#endif
	OutputDiscLog(
		_T("MasterDirectoryBlocks\n")
		_T("\t                       volume signature: %04x\n")
		_T("\t       date and time of volume creation: %08x\n")
		_T("\t     date and time of last modification: %08x\n")
		_T("\t                      volume attributes: %04x\n")
		_T("\t      number of files in root directory: %04x\n")
		_T("\t           first block of volume bitmap: %04x\n")
		_T("\t        start of next allocation search: %04x\n")
		_T("\t  number of allocation blocks in volume: %04x\n")
		_T("\t   size (in bytes) of allocation blocks: %08x\n")
		_T("\t                     default clump size: %08x\n")
		_T("\t       first allocation block in volume: %04x\n")
		_T("\t            next unused catalog node ID: %08x\n")
		_T("\t     number of unused allocation blocks: %04x\n")
		_T("\t                            volume name: %.27s\n")
		_T("\t           date and time of last backup: %08x\n")
		_T("\t          volume backup sequence number: %04x\n")
		_T("\t                     volume write count: %08x\n")
		_T("\t   clump size for extents overflow file: %08x\n")
		_T("\t            clump size for catalog file: %08x\n")
		_T("\tnumber of directories in root directory: %04x\n")
		_T("\t              number of files in volume: %08x\n")
		_T("\t        number of directories in volume: %08x\n")
		_T("\t         information used by the Finder: %08x\n")
		_T("\t         information used by the Finder: %08x\n")
		_T("\t         information used by the Finder: %08x\n")
		_T("\t         information used by the Finder: %08x\n")
		_T("\t         information used by the Finder: %08x\n")
		_T("\t         information used by the Finder: %08x\n")
		_T("\t         information used by the Finder: %08x\n")
		_T("\t         information used by the Finder: %08x\n")
		_T("\t       size (in blocks) of volume cache: %04x\n")
		_T("\tsize (in blocks) of volume bitmap cache: %04x\n")
		_T("\tsize (in blocks) of common volume cache: %04x\n")
		_T("\t          size of extents overflow file: %08x\n")
		_T("\textent record for extents overflow file: %08x%08x%08x\n")
		_T("\t                   size of catalog file: %08x\n")
		_T("\t         extent record for catalog file: %08x%08x%08x\n"),
		MAKEWORD(lpBuf[nIdx + 1], lpBuf[nIdx]),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 5], lpBuf[nIdx + 4]),
		MAKEWORD(lpBuf[nIdx + 3], lpBuf[nIdx + 2])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 9], lpBuf[nIdx + 8]),
		MAKEWORD(lpBuf[nIdx + 7], lpBuf[nIdx + 6])),
		MAKEWORD(lpBuf[nIdx + 11], lpBuf[nIdx + 10]),
		MAKEWORD(lpBuf[nIdx + 13], lpBuf[nIdx + 12]),
		MAKEWORD(lpBuf[nIdx + 15], lpBuf[nIdx + 14]),
		MAKEWORD(lpBuf[nIdx + 17], lpBuf[nIdx + 16]),
		MAKEWORD(lpBuf[nIdx + 19], lpBuf[nIdx + 18]),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 23], lpBuf[nIdx + 22]),
		MAKEWORD(lpBuf[nIdx + 21], lpBuf[nIdx + 20])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 27], lpBuf[nIdx + 26]),
		MAKEWORD(lpBuf[nIdx + 25], lpBuf[nIdx + 24])),
		MAKEWORD(lpBuf[nIdx + 29], lpBuf[nIdx + 28]),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 33], lpBuf[nIdx + 32]),
		MAKEWORD(lpBuf[nIdx + 31], lpBuf[nIdx + 30])),
		MAKEWORD(lpBuf[nIdx + 35], lpBuf[nIdx + 34]),
		str,
		MAKELONG(MAKEWORD(lpBuf[nIdx + 66], lpBuf[nIdx + 65]),
		MAKEWORD(lpBuf[nIdx + 64], lpBuf[nIdx + 63])),
		MAKEWORD(lpBuf[nIdx + 68], lpBuf[nIdx + 67]),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 72], lpBuf[nIdx + 71]),
		MAKEWORD(lpBuf[nIdx + 70], lpBuf[nIdx + 69])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 76], lpBuf[nIdx + 75]),
		MAKEWORD(lpBuf[nIdx + 74], lpBuf[nIdx + 73])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 80], lpBuf[nIdx + 79]),
		MAKEWORD(lpBuf[nIdx + 78], lpBuf[nIdx + 77])),
		MAKEWORD(lpBuf[nIdx + 82], lpBuf[nIdx + 81]),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 86], lpBuf[nIdx + 85]),
		MAKEWORD(lpBuf[nIdx + 84], lpBuf[nIdx + 83])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 90], lpBuf[nIdx + 89]),
		MAKEWORD(lpBuf[nIdx + 88], lpBuf[nIdx + 87])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 94], lpBuf[nIdx + 93]),
		MAKEWORD(lpBuf[nIdx + 92], lpBuf[nIdx + 91])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 98], lpBuf[nIdx + 97]),
		MAKEWORD(lpBuf[nIdx + 96], lpBuf[nIdx + 95])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 102], lpBuf[nIdx + 101]),
		MAKEWORD(lpBuf[nIdx + 100], lpBuf[nIdx + 99])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 106], lpBuf[nIdx + 105]),
		MAKEWORD(lpBuf[nIdx + 104], lpBuf[nIdx + 103])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 110], lpBuf[nIdx + 109]),
		MAKEWORD(lpBuf[nIdx + 108], lpBuf[nIdx + 107])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 114], lpBuf[nIdx + 113]),
		MAKEWORD(lpBuf[nIdx + 112], lpBuf[nIdx + 111])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 118], lpBuf[nIdx + 117]),
		MAKEWORD(lpBuf[nIdx + 116], lpBuf[nIdx + 115])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 122], lpBuf[nIdx + 121]),
		MAKEWORD(lpBuf[nIdx + 120], lpBuf[nIdx + 119])),
		MAKEWORD(lpBuf[nIdx + 124], lpBuf[nIdx + 123]),
		MAKEWORD(lpBuf[nIdx + 126], lpBuf[nIdx + 125]),
		MAKEWORD(lpBuf[nIdx + 128], lpBuf[nIdx + 127]),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 132], lpBuf[nIdx + 131]),
		MAKEWORD(lpBuf[nIdx + 130], lpBuf[nIdx + 129])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 136], lpBuf[nIdx + 135]),
		MAKEWORD(lpBuf[nIdx + 134], lpBuf[nIdx + 133])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 140], lpBuf[nIdx + 139]),
		MAKEWORD(lpBuf[nIdx + 138], lpBuf[nIdx + 137])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 144], lpBuf[nIdx + 143]),
		MAKEWORD(lpBuf[nIdx + 142], lpBuf[nIdx + 141])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 148], lpBuf[nIdx + 147]),
		MAKEWORD(lpBuf[nIdx + 146], lpBuf[nIdx + 145])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 152], lpBuf[nIdx + 151]),
		MAKEWORD(lpBuf[nIdx + 150], lpBuf[nIdx + 149])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 156], lpBuf[nIdx + 155]),
		MAKEWORD(lpBuf[nIdx + 154], lpBuf[nIdx + 153])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 160], lpBuf[nIdx + 159]),
		MAKEWORD(lpBuf[nIdx + 158], lpBuf[nIdx + 157])));
}

VOID OutputMmcBufferCapacity(
	PREAD_BUFFER_CAPACITY_DATA pReadBufCapaData
	)
{
	OutputDriveLog(
		_T("Read Buffer Capacity\n")
		_T("\t  Length of the Buffer: %dKByte\n")
		_T("\tBlank Length of Buffer: %dKByte\n"),
		MAKELONG(MAKEWORD(pReadBufCapaData->TotalBufferSize[3], 
			pReadBufCapaData->TotalBufferSize[2]), 
			MAKEWORD(pReadBufCapaData->TotalBufferSize[1],
			pReadBufCapaData->TotalBufferSize[0])) / 1024,
		MAKELONG(MAKEWORD(pReadBufCapaData->AvailableBufferSize[3], 
			pReadBufCapaData->AvailableBufferSize[2]), 
			MAKEWORD(pReadBufCapaData->AvailableBufferSize[1],
			pReadBufCapaData->AvailableBufferSize[0])) / 1024);
}

VOID OutputMmcDiscInformation(
	PDISC_INFORMATION pDiscInformation
	)
{
	LPCTSTR lpDiscStatus[] = {
		_T("Empty Disc"), _T("Incomplete Disc"),
		_T("Finalized Disc"), _T("Others")
	};
	LPCTSTR lpStateOfLastSession[] = {
		_T("Empty Session"), _T("Incomplete Session"),
		_T("Reserved / Damaged Session"), _T("Complete Session")
	};
	LPCTSTR lpBGFormatStatus[] = {
		_T("The disc is neither CD-RW nor DVD+RW"),
		_T("A background format was started"),
		_T("A background format is in progress"),
		_T("Background formatting has completed")
	};
	OutputDiscLog(
		_T("DiscInformation\n")
		_T("\t                      Disc Status: %s\n")
		_T("\t              Last Session Status: %s\n")
		_T("\t                         Erasable: %s\n")
		_T("\t               First Track Number: %d\n")
		_T("\t               Number of Sessions: %d\n")
		_T("\t         Last Session First Track: %d\n")
		_T("\t          Last Session Last Track: %d\n")
		_T("\t         Background Format Status: %s\n")
		_T("\t                        Dirty Bit: %s\n")
		_T("\t            Unrestricted Use Disc: %s\n")
		_T("\t              Disc Bar Code Valid: %s\n")
		_T("\t                    Disc ID Valid: %s\n")
		_T("\t                        Disc Type: "),
		lpDiscStatus[pDiscInformation->DiscStatus],
		lpStateOfLastSession[pDiscInformation->LastSessionStatus],
		BOOLEAN_TO_STRING_YES_NO(pDiscInformation->Erasable),
		pDiscInformation->FirstTrackNumber,
		pDiscInformation->NumberOfSessionsLsb,
		pDiscInformation->LastSessionFirstTrackLsb,
		pDiscInformation->LastSessionLastTrackLsb,
		lpBGFormatStatus[pDiscInformation->MrwStatus],
		BOOLEAN_TO_STRING_YES_NO(pDiscInformation->MrwDirtyBit),
		BOOLEAN_TO_STRING_YES_NO(pDiscInformation->URU),
		BOOLEAN_TO_STRING_YES_NO(pDiscInformation->DBC_V),
		BOOLEAN_TO_STRING_YES_NO(pDiscInformation->DID_V));
	switch (pDiscInformation->DiscType) {
	case 0:
		OutputDiscLog(_T("CD-DA or CD-ROM Disc\n"));
		break;
	case 0x10:
		OutputDiscLog(_T("CD-I Disc\n"));
		break;
	case 0x20:
		OutputDiscLog(_T("CD-ROM XA Disc\n"));
		break;
	case 0xff:
		OutputDiscLog(_T("Undefined\n"));
		break;
	default:
		OutputDiscLog(_T("Reserved\n"));
		break;
	}
	if (pDiscInformation->DID_V) {
		OutputDiscLog(
			_T("\t              Disc Identification: %d%d%d%d\n"),
			pDiscInformation->DiskIdentification[0],
			pDiscInformation->DiskIdentification[1],
			pDiscInformation->DiskIdentification[2],
			pDiscInformation->DiskIdentification[3]);
	}
	OutputDiscLog(
		_T("\t  Last Session Lead-in Start Time: %02x:%02x:%02x:%02x\n")
		_T("\tLast Possible Lead-out Start Time: %02x:%02x:%02x:%02x\n"),
		pDiscInformation->LastSessionLeadIn[0],
		pDiscInformation->LastSessionLeadIn[1],
		pDiscInformation->LastSessionLeadIn[2],
		pDiscInformation->LastSessionLeadIn[3],
		pDiscInformation->LastPossibleLeadOutStartTime[0],
		pDiscInformation->LastPossibleLeadOutStartTime[1],
		pDiscInformation->LastPossibleLeadOutStartTime[2],
		pDiscInformation->LastPossibleLeadOutStartTime[3]);
	if (pDiscInformation->DBC_V) {
		OutputDiscLog(
			_T("\t                    Disc Bar Code: %d%d%d%d%d%d%d%d\n"),
			pDiscInformation->DiskBarCode[0],
			pDiscInformation->DiskBarCode[1],
			pDiscInformation->DiskBarCode[2],
			pDiscInformation->DiskBarCode[3],
			pDiscInformation->DiskBarCode[4],
			pDiscInformation->DiskBarCode[5],
			pDiscInformation->DiskBarCode[6],
			pDiscInformation->DiskBarCode[7]);
	}
	OutputDiscLog(
		_T("\t             Number of OPC Tables: %d\n"),
		pDiscInformation->NumberOPCEntries);
	if (pDiscInformation->NumberOPCEntries) {
		OutputDiscLog(
			_T("\t                        OPC Table\n"));
	}
	for (INT i = 0; i < pDiscInformation->NumberOPCEntries; i++) {
		OutputDiscLog(
			_T("\t\t                          Speed: %d%d\n")
			_T("\t\t                     OPC Values: %d%d%d%d%d%d\n"),
			pDiscInformation->OPCTable[0].Speed[0],
			pDiscInformation->OPCTable[0].Speed[1],
			pDiscInformation->OPCTable[0].OPCValue[0],
			pDiscInformation->OPCTable[0].OPCValue[1],
			pDiscInformation->OPCTable[0].OPCValue[2],
			pDiscInformation->OPCTable[0].OPCValue[3],
			pDiscInformation->OPCTable[0].OPCValue[4],
			pDiscInformation->OPCTable[0].OPCValue[5]);
	}
}

VOID OutputMmcDriveSpeed(
	PCDROM_SET_SPEED pSetspeed
	)
{
	OutputDriveLog(
		_T("Drive speed\n")
		_T("\t    RequestType: %s\n")
		_T("\t      ReadSpeed: %uKB/sec\n")
		_T("\t     WriteSpeed: %uKB/sec\n")
		_T("\tRotationControl: %s\n"),
		pSetspeed->RequestType == 0 ? 
			_T("CdromSetSpeed") : _T("CdromSetStreaming"),
		pSetspeed->ReadSpeed,
		pSetspeed->WriteSpeed,
		pSetspeed->RotationControl == 0 ? 
			_T("CdromDefaultRotation") : _T("CdromCAVRotation"));
}

VOID OutputMmcFeatureCore(
	PFEATURE_DATA_CORE pCore
	)
{
	OutputDriveLog(
		_T("\tFeatureCore\n")
		_T("\t\tPhysicalInterface: "));
	LONG lVal = MAKELONG(
		MAKEWORD(pCore->PhysicalInterface[3], pCore->PhysicalInterface[2]),
		MAKEWORD(pCore->PhysicalInterface[1], pCore->PhysicalInterface[0]));
	switch (lVal) {
	case 0:
		OutputDriveLog(_T("Unspecified\n"));
		break;
	case 1:
		OutputDriveLog(_T("SCSI Family\n"));
		break;
	case 2:
		OutputDriveLog(_T("ATAPI\n"));
		break;
	case 3:
		OutputDriveLog(_T("IEEE 1394 - 1995\n"));
		break;
	case 4:
		OutputDriveLog(_T("IEEE 1394A\n"));
		break;
	case 5:
		OutputDriveLog(_T("Fibre Channel\n"));
		break;
	case 6:
		OutputDriveLog(_T("IEEE 1394B\n"));
		break;
	case 7:
		OutputDriveLog(_T("Serial ATAPI\n"));
		break;
	case 8:
		OutputDriveLog(_T("USB (both 1.1 and 2.0)\n"));
		break;
	case 0xFFFF:
		OutputDriveLog(_T("Vendor Unique\n"));
		break;
	default:
		OutputDriveLog(_T("Reserved: %08d\n"), lVal);
		break;
	}
	OutputDriveLog(
		_T("\t\t  DeviceBusyEvent: %s\n")
		_T("\t\t         INQUIRY2: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(pCore->DeviceBusyEvent),
		BOOLEAN_TO_STRING_YES_NO(pCore->INQUIRY2));
}

VOID OutputMmcFeatureMorphing(
	PFEATURE_DATA_MORPHING pMorphing
	)
{
	OutputDriveLog(
		_T("\tFeatureMorphing\n")
		_T("\t\tAsynchronous: %s\n")
		_T("\t\t     OCEvent: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(pMorphing->Asynchronous),
		BOOLEAN_TO_STRING_YES_NO(pMorphing->OCEvent));
}

VOID OutputMmcFeatureRemovableMedium(
	PFEATURE_DATA_REMOVABLE_MEDIUM pRemovableMedium
	)
{
	OutputDriveLog(
		_T("\tFeatureRemovableMedium\n")
		_T("\t\t        Lockable: %s\n")
		_T("\t\tDefaultToPrevent: %s\n")
		_T("\t\t           Eject: %s\n")
		_T("\t\tLoadingMechanism: "),
		BOOLEAN_TO_STRING_YES_NO(pRemovableMedium->Lockable),
		BOOLEAN_TO_STRING_YES_NO(pRemovableMedium->DefaultToPrevent),
		BOOLEAN_TO_STRING_YES_NO(pRemovableMedium->Eject));
	switch (pRemovableMedium->LoadingMechanism) {
	case 0:
		OutputDriveLog(_T("Caddy/Slot type loading mechanism\n"));
		break;
	case 1:
		OutputDriveLog(_T("Tray type loading mechanism\n"));
		break;
	case 2:
		OutputDriveLog(_T("Pop-up type loading mechanism\n"));
		break;
	case 4:
		OutputDriveLog(
			_T("Embedded changer with individually changeable discs\n"));
		break;
	case 5:
		OutputDriveLog(
			_T("Embedded changer using a magazine mechanism\n"));
		break;
	default:
		OutputDriveLog(
			_T("Reserved: %08d\n"), pRemovableMedium->LoadingMechanism);
		break;
	}
}

VOID OutputMmcFeatureWriteProtect(
	PFEATURE_DATA_WRITE_PROTECT pWriteProtect
	)
{
	OutputDriveLog(
		_T("\tFeatureWriteProtect\n")
		_T("\t\t               SupportsSWPPBit: %s\n")
		_T("\t\tSupportsPersistentWriteProtect: %s\n")
		_T("\t\t               WriteInhibitDCB: %s\n")
		_T("\t\t           DiscWriteProtectPAC: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(pWriteProtect->SupportsSWPPBit),
		BOOLEAN_TO_STRING_YES_NO(pWriteProtect->SupportsPersistentWriteProtect),
		BOOLEAN_TO_STRING_YES_NO(pWriteProtect->WriteInhibitDCB),
		BOOLEAN_TO_STRING_YES_NO(pWriteProtect->DiscWriteProtectPAC));
}

VOID OutputMmcFeatureRandomReadable(
	PFEATURE_DATA_RANDOM_READABLE pRandomReadable
	)
{
	OutputDriveLog(
		_T("\tFeatureRandomReadable\n")
		_T("\t\t        LogicalBlockSize: %d\n")
		_T("\t\t                Blocking: %d\n")
		_T("\t\tErrorRecoveryPagePresent: %s\n"),
		MAKELONG(MAKEWORD(pRandomReadable->LogicalBlockSize[3], pRandomReadable->LogicalBlockSize[2]),
			MAKEWORD(pRandomReadable->LogicalBlockSize[1], pRandomReadable->LogicalBlockSize[0])),
		MAKEWORD(pRandomReadable->Blocking[1], pRandomReadable->Blocking[0]),
		BOOLEAN_TO_STRING_YES_NO(pRandomReadable->ErrorRecoveryPagePresent));
}

VOID OutputMmcFeatureMultiRead(
	PFEATURE_DATA_MULTI_READ pMultiRead
	)
{
	OutputDriveLog(
		_T("\tFeatureMultiRead\n")
		_T("\t\t   Current: %d\n")
		_T("\t\tPersistent: %d\n")
		_T("\t\t   Version: %d\n"),
		pMultiRead->Header.Current,
		pMultiRead->Header.Persistent,
		pMultiRead->Header.Version);
}

VOID SetAndOutputMmcFeatureCdRead(
	PFEATURE_DATA_CD_READ pCDRead,
	PDEVICE_DATA pDevData
	)
{
	OutputDriveLog(
		_T("\tFeatureCdRead\n")
		_T("\t\t          CDText: %s\n")
		_T("\t\t     C2ErrorData: %s\n")
		_T("\t\tDigitalAudioPlay: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(pCDRead->CDText),
		BOOLEAN_TO_STRING_YES_NO(pCDRead->C2ErrorData),
		BOOLEAN_TO_STRING_YES_NO(pCDRead->DigitalAudioPlay));
	pDevData->bCanCDText = (BOOL)(pCDRead->CDText);
	pDevData->bC2ErrorData = (BOOL)(pCDRead->C2ErrorData);
}

VOID OutputMmcFeatureDvdRead(
	PFEATURE_DATA_DVD_READ pDVDRead
	)
{
	OutputDriveLog(
		_T("\tFeatureDvdRead\n")
		_T("\t\t  Multi110: %s\n")
		_T("\t\t DualDashR: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(pDVDRead->Multi110),
		BOOLEAN_TO_STRING_YES_NO(pDVDRead->DualDashR));
}

VOID OutputMmcFeatureRandomWritable(
	PFEATURE_DATA_RANDOM_WRITABLE pRandomWritable
	)
{
	OutputDriveLog(
		_T("\tFeatureRandomWritable\n")
		_T("\t\t                 LastLBA: %d\n")
		_T("\t\t        LogicalBlockSize: %d\n")
		_T("\t\t                Blocking: %d\n")
		_T("\t\tErrorRecoveryPagePresent: %s\n"),
		MAKELONG(MAKEWORD(pRandomWritable->LastLBA[3], pRandomWritable->LastLBA[2]),
			MAKEWORD(pRandomWritable->LastLBA[1], pRandomWritable->LastLBA[0])),
		MAKELONG(MAKEWORD(pRandomWritable->LogicalBlockSize[3], pRandomWritable->LogicalBlockSize[2]),
			MAKEWORD(pRandomWritable->LogicalBlockSize[1], pRandomWritable->LogicalBlockSize[0])),
		MAKEWORD(pRandomWritable->Blocking[1], pRandomWritable->Blocking[0]),
		BOOLEAN_TO_STRING_YES_NO(pRandomWritable->ErrorRecoveryPagePresent));
}

VOID OutputMmcFeatureIncrementalStreamingWritable(
	PFEATURE_DATA_INCREMENTAL_STREAMING_WRITABLE pIncremental
	)
{
	OutputDriveLog(
		_T("\tFeatureIncrementalStreamingWritable\n")
		_T("\t\t        DataTypeSupported: %d\n")
		_T("\t\t       BufferUnderrunFree: %s\n")
		_T("\t\t   AddressModeReservation: %s\n")
		_T("\t\tTrackRessourceInformation: %s\n")
		_T("\t\t        NumberOfLinkSizes: %d\n"),
		MAKEWORD(pIncremental->DataTypeSupported[1], pIncremental->DataTypeSupported[0]),
		BOOLEAN_TO_STRING_YES_NO(pIncremental->BufferUnderrunFree),
		BOOLEAN_TO_STRING_YES_NO(pIncremental->AddressModeReservation),
		BOOLEAN_TO_STRING_YES_NO(pIncremental->TrackRessourceInformation),
		pIncremental->NumberOfLinkSizes);
	for (INT i = 0; i < pIncremental->NumberOfLinkSizes; i++) {
		OutputDriveLog(
			_T("\t\tLinkSize%d: %d\n"), i, pIncremental->LinkSize[i]);
	}
}

VOID OutputMmcFeatureSectorErasable(
	PFEATURE_DATA_SECTOR_ERASABLE pSectorErasable
	)
{
	OutputDriveLog(
		_T("\tFeatureSectorErasable\n")
		_T("\t\t   Current: %d\n")
		_T("\t\tPersistent: %d\n")
		_T("\t\t   Version: %d\n"),
		pSectorErasable->Header.Current,
		pSectorErasable->Header.Persistent,
		pSectorErasable->Header.Version);
}

VOID OutputMmcFeatureFormattable(
	PFEATURE_DATA_FORMATTABLE pFormattable
	)
{
	OutputDriveLog(
		_T("\tFeatureFormattable\n")
		_T("\t\t FullCertification: %s\n")
		_T("\t\tQuickCertification: %s\n")
		_T("\t\tSpareAreaExpansion: %s\n")
		_T("\t\tRENoSpareAllocated: %s\n")
		_T("\t\t   RRandomWritable: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(pFormattable->FullCertification),
		BOOLEAN_TO_STRING_YES_NO(pFormattable->QuickCertification),
		BOOLEAN_TO_STRING_YES_NO(pFormattable->SpareAreaExpansion),
		BOOLEAN_TO_STRING_YES_NO(pFormattable->RENoSpareAllocated),
		BOOLEAN_TO_STRING_YES_NO(pFormattable->RRandomWritable));
}

VOID OutputMmcFeatureDefectManagement(
	PFEATURE_DATA_DEFECT_MANAGEMENT pDefect
	)
{
	OutputDriveLog(
		_T("\tFeatureDefectManagement\n")
		_T("\t\tSupplimentalSpareArea: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(pDefect->SupplimentalSpareArea));
}

VOID OutputMmcFeatureWriteOnce(
	PFEATURE_DATA_WRITE_ONCE pWriteOnce
	)
{
	OutputDriveLog(
		_T("\tFeatureWriteOnce\n")
		_T("\t\t        LogicalBlockSize: %d\n")
		_T("\t\t                Blocking: %d\n")
		_T("\t\tErrorRecoveryPagePresent: %s\n"),
		MAKELONG(MAKEWORD(pWriteOnce->LogicalBlockSize[3], pWriteOnce->LogicalBlockSize[2]),
			MAKEWORD(pWriteOnce->LogicalBlockSize[1], pWriteOnce->LogicalBlockSize[0])),
		MAKEWORD(pWriteOnce->Blocking[1], pWriteOnce->Blocking[0]),
		BOOLEAN_TO_STRING_YES_NO(pWriteOnce->ErrorRecoveryPagePresent));
}

VOID OutputMmcFeatureRestrictedOverwrite(
	PFEATURE_DATA_RESTRICTED_OVERWRITE pRestricted
	)
{
	OutputDriveLog(
		_T("\tFeatureRestrictedOverwrite\n")
		_T("\t\t   Current: %d\n")
		_T("\t\tPersistent: %d\n")
		_T("\t\t   Version: %d\n"),
		pRestricted->Header.Current,
		pRestricted->Header.Persistent,
		pRestricted->Header.Version);
}

VOID OutputMmcFeatureCdrwCAVWrite(
	PFEATURE_DATA_CDRW_CAV_WRITE pCDRW
	)
{
	OutputDriveLog(
		_T("\tFeatureCdrwCAVWrite\n")
		_T("\t\t   Current: %d\n")
		_T("\t\tPersistent: %d\n")
		_T("\t\t   Version: %d\n"),
		pCDRW->Header.Current,
		pCDRW->Header.Persistent,
		pCDRW->Header.Version);
}

VOID OutputMmcFeatureMrw(
	PFEATURE_DATA_MRW pMrw
	)
{
	OutputDriveLog(
		_T("\tFeatureMrw\n")
		_T("\t\t       Write: %s\n")
		_T("\t\t DvdPlusRead: %s\n")
		_T("\t\tDvdPlusWrite: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(pMrw->Write),
		BOOLEAN_TO_STRING_YES_NO(pMrw->DvdPlusRead),
		BOOLEAN_TO_STRING_YES_NO(pMrw->DvdPlusWrite));
}

VOID OutputMmcFeatureEnhancedDefectReporting(
	PFEATURE_ENHANCED_DEFECT_REPORTING pEnhanced
	)
{
	OutputDriveLog(
		_T("\tFeatureEnhancedDefectReporting\n")
		_T("\t\t       DRTDMSupported: %s\n")
		_T("\t\tNumberOfDBICacheZones: %d\n")
		_T("\t\t      NumberOfEntries: %d\n"),
		BOOLEAN_TO_STRING_YES_NO(pEnhanced->DRTDMSupported),
		pEnhanced->NumberOfDBICacheZones,
		MAKEWORD(pEnhanced->NumberOfEntries[1], pEnhanced->NumberOfEntries[0]));
}

VOID OutputMmcFeatureDvdPlusRW(
	PFEATURE_DATA_DVD_PLUS_RW pDVDPLUSRW
	)
{
	OutputDriveLog(
		_T("\tFeatureDvdPlusRW\n")
		_T("\t\t     Write: %s\n")
		_T("\t\t CloseOnly: %s\n")
		_T("\t\tQuickStart: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(pDVDPLUSRW->Write),
		BOOLEAN_TO_STRING_YES_NO(pDVDPLUSRW->CloseOnly),
		BOOLEAN_TO_STRING_YES_NO(pDVDPLUSRW->QuickStart));
}

VOID OutputMmcFeatureDvdPlusR(
	PFEATURE_DATA_DVD_PLUS_R pDVDPLUSR
	)
{
	OutputDriveLog(
		_T("\tFeatureDvdPlusR\n")
		_T("\t\tWrite: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(pDVDPLUSR->Write));
}

VOID OutputMmcFeatureRigidRestrictedOverwrite(
	PFEATURE_DATA_DVD_RW_RESTRICTED_OVERWRITE pDVDRWRestricted
	)
{
	OutputDriveLog(
		_T("\tFeatureRigidRestrictedOverwrite\n")
		_T("\t\t                   Blank: %s\n")
		_T("\t\t            Intermediate: %s\n")
		_T("\t\t    DefectStatusDataRead: %s\n")
		_T("\t\tDefectStatusDataGenerate: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(pDVDRWRestricted->Blank),
		BOOLEAN_TO_STRING_YES_NO(pDVDRWRestricted->Intermediate),
		BOOLEAN_TO_STRING_YES_NO(pDVDRWRestricted->DefectStatusDataRead),
		BOOLEAN_TO_STRING_YES_NO(pDVDRWRestricted->DefectStatusDataGenerate));
}

VOID OutputMmcFeatureCdTrackAtOnce(
	PFEATURE_DATA_CD_TRACK_AT_ONCE pCDTrackAtOnce
	)
{
	OutputDriveLog(
		_T("\tFeatureCdTrackAtOnce\n")
		_T("\t\tRWSubchannelsRecordable: %s\n")
		_T("\t\t           CdRewritable: %s\n")
		_T("\t\t            TestWriteOk: %s\n")
		_T("\t\t   RWSubchannelPackedOk: %s\n")
		_T("\t\t      RWSubchannelRawOk: %s\n")
		_T("\t\t     BufferUnderrunFree: %s\n")
		_T("\t\t      DataTypeSupported: %d\n"),
		BOOLEAN_TO_STRING_YES_NO(pCDTrackAtOnce->RWSubchannelsRecordable),
		BOOLEAN_TO_STRING_YES_NO(pCDTrackAtOnce->CdRewritable),
		BOOLEAN_TO_STRING_YES_NO(pCDTrackAtOnce->TestWriteOk),
		BOOLEAN_TO_STRING_YES_NO(pCDTrackAtOnce->RWSubchannelPackedOk),
		BOOLEAN_TO_STRING_YES_NO(pCDTrackAtOnce->RWSubchannelRawOk),
		BOOLEAN_TO_STRING_YES_NO(pCDTrackAtOnce->BufferUnderrunFree),
		MAKEWORD(pCDTrackAtOnce->DataTypeSupported[1], pCDTrackAtOnce->DataTypeSupported[0]));
}

VOID OutputMmcFeatureCdMastering(
	PFEATURE_DATA_CD_MASTERING pCDMastering
	)
{
	OutputDriveLog(
		_T("\tFeatureCdMastering\n")
		_T("\t\tRWSubchannelsRecordable: %s\n")
		_T("\t\t           CdRewritable: %s\n")
		_T("\t\t            TestWriteOk: %s\n")
		_T("\t\t        RRawRecordingOk: %s\n")
		_T("\t\t      RawMultiSessionOk: %s\n")
		_T("\t\t        SessionAtOnceOk: %s\n")
		_T("\t\t     BufferUnderrunFree: %s\n")
		_T("\t\t  MaximumCueSheetLength: %d\n"),
		BOOLEAN_TO_STRING_YES_NO(pCDMastering->RWSubchannelsRecordable),
		BOOLEAN_TO_STRING_YES_NO(pCDMastering->CdRewritable),
		BOOLEAN_TO_STRING_YES_NO(pCDMastering->TestWriteOk),
		BOOLEAN_TO_STRING_YES_NO(pCDMastering->RawRecordingOk),
		BOOLEAN_TO_STRING_YES_NO(pCDMastering->RawMultiSessionOk),
		BOOLEAN_TO_STRING_YES_NO(pCDMastering->SessionAtOnceOk),
		BOOLEAN_TO_STRING_YES_NO(pCDMastering->BufferUnderrunFree),
		MAKELONG(MAKEWORD(0, pCDMastering->MaximumCueSheetLength[2]),
			MAKEWORD(pCDMastering->MaximumCueSheetLength[1], pCDMastering->MaximumCueSheetLength[0])));
}

VOID OutputMmcFeatureDvdRecordableWrite(
	PFEATURE_DATA_DVD_RECORDABLE_WRITE pDVDRecordable
	)
{
	OutputDriveLog(
		_T("\tFeatureDvdRecordableWrite\n")
		_T("\t\t            DVD_RW: %s\n")
		_T("\t\t         TestWrite: %s\n")
		_T("\t\t        RDualLayer: %s\n")
		_T("\t\tBufferUnderrunFree: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(pDVDRecordable->DVD_RW),
		BOOLEAN_TO_STRING_YES_NO(pDVDRecordable->TestWrite),
		BOOLEAN_TO_STRING_YES_NO(pDVDRecordable->RDualLayer),
		BOOLEAN_TO_STRING_YES_NO(pDVDRecordable->BufferUnderrunFree));
}

VOID OutputMmcFeatureLayerJumpRecording(
	PFEATURE_DATA_LAYER_JUMP_RECORDING pLayerJumpRec
	)
{
	OutputDriveLog(
		_T("\tFeatureLayerJumpRecording\n")
		_T("\t\tNumberOfLinkSizes: %d\n"),
		pLayerJumpRec->NumberOfLinkSizes);
	for (INT i = 0; i < pLayerJumpRec->NumberOfLinkSizes; i++) {
		OutputDriveLog(
			_T("\t\tLinkSize %d: %d\n"), i, pLayerJumpRec->LinkSizes[i]);
	}
}

VOID OutputMmcFeatureCDRWMediaWriteSupport(
	PFEATURE_CD_RW_MEDIA_WRITE_SUPPORT pCDRWMediaWrite
	)
{
	OutputDriveLog(
		_T("\tFeatureCDRWMediaWriteSupport\n")
		_T("\t\tSubtype 0: %s\n")
		_T("\t\tSubtype 1: %s\n")
		_T("\t\tSubtype 2: %s\n")
		_T("\t\tSubtype 3: %s\n")
		_T("\t\tSubtype 4: %s\n")
		_T("\t\tSubtype 5: %s\n")
		_T("\t\tSubtype 6: %s\n")
		_T("\t\tSubtype 7: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype0),
		BOOLEAN_TO_STRING_YES_NO(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype1),
		BOOLEAN_TO_STRING_YES_NO(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype2),
		BOOLEAN_TO_STRING_YES_NO(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype3),
		BOOLEAN_TO_STRING_YES_NO(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype4),
		BOOLEAN_TO_STRING_YES_NO(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype5),
		BOOLEAN_TO_STRING_YES_NO(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype6),
		BOOLEAN_TO_STRING_YES_NO(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype7));
}

VOID OutputMmcFeatureDvdPlusRWDualLayer(
	PFEATURE_DATA_DVD_PLUS_RW_DUAL_LAYER pDVDPlusRWDL
	)
{
	OutputDriveLog(
		_T("\tFeatureDvdPlusRWDualLayer\n")
		_T("\t\t     Write: %s\n")
		_T("\t\t CloseOnly: %s\n")
		_T("\t\tQuickStart: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(pDVDPlusRWDL->Write),
		BOOLEAN_TO_STRING_YES_NO(pDVDPlusRWDL->CloseOnly),
		BOOLEAN_TO_STRING_YES_NO(pDVDPlusRWDL->QuickStart));
}

VOID OutputMmcFeatureDvdPlusRDualLayer(
	PFEATURE_DATA_DVD_PLUS_R_DUAL_LAYER pDVDPlusRDL
	)
{
	OutputDriveLog(
		_T("\tFeatureDvdPlusRDualLayer\n")
		_T("\t\tWrite: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(pDVDPlusRDL->Write));
}

VOID OutputMmcFeatureHybridDisc(
	PFEATURE_HYBRID_DISC pHybridDisc
	)
{
	OutputDriveLog(
		_T("\tFeatureHybridDisc\n")
		_T("\t\tResetImmunity: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(pHybridDisc->ResetImmunity));
}

VOID OutputMmcFeaturePowerManagement(
	PFEATURE_DATA_POWER_MANAGEMENT pPower
	)
{
	OutputDriveLog(
		_T("\tFeaturePowerManagement\n")
		_T("\t\t   Current: %d\n")
		_T("\t\tPersistent: %d\n")
		_T("\t\t   Version: %d\n"),
		pPower->Header.Current,
		pPower->Header.Persistent,
		pPower->Header.Version);
}

VOID OutputMmcFeatureSMART(
	PFEATURE_DATA_SMART pSmart
	)
{
	OutputDriveLog(
		_T("\tFeatureSMART\n")
		_T("\t\tFaultFailureReportingPagePresent: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(pSmart->FaultFailureReportingPagePresent));
}

VOID OutputMmcFeatureEmbeddedChanger(
	PFEATURE_DATA_EMBEDDED_CHANGER pEmbedded
	)
{
	OutputDriveLog(
		_T("\tFeatureEmbeddedChanger\n")
		_T("\t\tSupportsDiscPresent: %s\n")
		_T("\t\t  SideChangeCapable: %s\n")
		_T("\t\t  HighestSlotNumber: %d\n"),
		BOOLEAN_TO_STRING_YES_NO(pEmbedded->SupportsDiscPresent),
		BOOLEAN_TO_STRING_YES_NO(pEmbedded->SideChangeCapable),
		pEmbedded->HighestSlotNumber);
}

VOID OutputMmcFeatureCDAudioAnalogPlay(
	PFEATURE_DATA_CD_AUDIO_ANALOG_PLAY pCDAudio
	)
{
	OutputDriveLog(
		_T("\tFeatureCDAudioAnalogPlay\n")
		_T("\t\t     SeperateVolume: %s\n")
		_T("\t\tSeperateChannelMute: %s\n")
		_T("\t\t      ScanSupported: %s\n")
		_T("\t\tNumerOfVolumeLevels: %d\n"),
		BOOLEAN_TO_STRING_YES_NO(pCDAudio->SeperateVolume),
		BOOLEAN_TO_STRING_YES_NO(pCDAudio->SeperateChannelMute),
		BOOLEAN_TO_STRING_YES_NO(pCDAudio->ScanSupported),
		MAKEWORD(pCDAudio->NumerOfVolumeLevels[1], pCDAudio->NumerOfVolumeLevels[0]));
}

VOID OutputMmcFeatureMicrocodeUpgrade(
	PFEATURE_DATA_MICROCODE_UPDATE pMicrocode
	)
{
	OutputDriveLog(
		_T("\tFeatureMicrocodeUpgrade\n")
		_T("\t\tM5: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(pMicrocode->M5));
}

VOID OutputMmcFeatureTimeout(
	PFEATURE_DATA_TIMEOUT pTimeOut
	)
{
	OutputDriveLog(
		_T("\tFeatureTimeout\n")
		_T("\t\t    Group3: %s\n")
		_T("\t\tUnitLength: %d\n"),
		BOOLEAN_TO_STRING_YES_NO(pTimeOut->Group3),
		MAKEWORD(pTimeOut->UnitLength[1], pTimeOut->UnitLength[0]));
}

VOID OutputMmcFeatureDvdCSS(
	PFEATURE_DATA_DVD_CSS pDVDCss
	)
{
	OutputDriveLog(
		_T("\tFeatureDvdCSS\n")
		_T("\t\tCssVersion: %d\n"),
		pDVDCss->CssVersion);
}

VOID OutputMmcFeatureRealTimeStreaming(
	PFEATURE_DATA_REAL_TIME_STREAMING pRealTimeStreaming
	)
{
	OutputDriveLog(
		_T("\tFeatureRealTimeStreaming\n")
		_T("\t\t        StreamRecording: %s\n")
		_T("\t\t    WriteSpeedInGetPerf: %s\n")
		_T("\t\t       WriteSpeedInMP2A: %s\n")
		_T("\t\t             SetCDSpeed: %s\n")
		_T("\t\tReadBufferCapacityBlock: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(pRealTimeStreaming->StreamRecording),
		BOOLEAN_TO_STRING_YES_NO(pRealTimeStreaming->WriteSpeedInGetPerf),
		BOOLEAN_TO_STRING_YES_NO(pRealTimeStreaming->WriteSpeedInMP2A),
		BOOLEAN_TO_STRING_YES_NO(pRealTimeStreaming->SetCDSpeed),
		BOOLEAN_TO_STRING_YES_NO(pRealTimeStreaming->ReadBufferCapacityBlock));
}

VOID OutputMmcFeatureLogicalUnitSerialNumber(
	PFEATURE_DATA_LOGICAL_UNIT_SERIAL_NUMBER pLogical
	)
{
	OutputDriveLog(
		_T("\tFeatureLogicalUnitSerialNumber\n")
		_T("\t\tSerialNumber: "));
	for (INT i = 0; i < pLogical->Header.AdditionalLength; i++) {
		OutputDriveLog(_T("%c"), pLogical->SerialNumber[i]);
	}
	OutputDriveLog(_T("\n"));
}

VOID OutputMmcFeatureMediaSerialNumber(
	PFEATURE_MEDIA_SERIAL_NUMBER pMediaSerialNumber
	)
{
	OutputDriveLog(
		_T("\tFeatureMediaSerialNumber\n")
		_T("\t\t   Current: %d\n")
		_T("\t\tPersistent: %d\n")
		_T("\t\t   Version: %d\n"),
		pMediaSerialNumber->Header.Current,
		pMediaSerialNumber->Header.Persistent,
		pMediaSerialNumber->Header.Version);
}

VOID OutputMmcFeatureDiscControlBlocks(
	PFEATURE_DATA_DISC_CONTROL_BLOCKS pDiscCtrlBlk
	)
{
	OutputDriveLog(_T("\tFeatureDiscControlBlocks\n"));
	for (INT i = 0; i < pDiscCtrlBlk->Header.AdditionalLength; i++) {
		OutputDriveLog(
			_T("\t\tContentDescriptor %02d: %08d\n"), i,
			MAKELONG(
				MAKEWORD(pDiscCtrlBlk->Data[i].ContentDescriptor[3], pDiscCtrlBlk->Data[i].ContentDescriptor[2]),
				MAKEWORD(pDiscCtrlBlk->Data[i].ContentDescriptor[1], pDiscCtrlBlk->Data[i].ContentDescriptor[0])));
	}
}

VOID OutputMmcFeatureDvdCPRM(
	PFEATURE_DATA_DVD_CPRM pDVDCprm
	)
{
	OutputDriveLog(
		_T("\tFeatureDvdCPRM\n")
		_T("\t\tCPRMVersion: %d\n"),
		pDVDCprm->CPRMVersion);
}

VOID OutputMmcFeatureFirmwareDate(
	PFEATURE_DATA_FIRMWARE_DATE pFirmwareDate
	)
{
	OutputDriveLog(
		_T("\tFeatureFirmwareDate: %04d-%02d-%02d %02d:%02d:%02d\n"),
		MAKELONG(MAKEWORD(pFirmwareDate->Year[3], pFirmwareDate->Year[2]),
			MAKEWORD(pFirmwareDate->Year[1], pFirmwareDate->Year[0])),
		MAKEWORD(pFirmwareDate->Month[1], pFirmwareDate->Month[0]),
		MAKEWORD(pFirmwareDate->Day[1], pFirmwareDate->Day[0]),
		MAKEWORD(pFirmwareDate->Hour[1], pFirmwareDate->Hour[0]),
		MAKEWORD(pFirmwareDate->Minute[1], pFirmwareDate->Minute[0]),
		MAKEWORD(pFirmwareDate->Seconds[1], pFirmwareDate->Seconds[0]));
}

VOID OutputMmcFeatureAACS(
	PFEATURE_DATA_AACS pAacs
	)
{
	OutputDriveLog(
		_T("\tFeatureAACS\n")
		_T("\t\tBindingNonceGeneration: %s\n")
		_T("\t\tBindingNonceBlockCount: %d\n")
		_T("\t\t         NumberOfAGIDs: %d\n")
		_T("\t\t           AACSVersion: %d\n"),
		BOOLEAN_TO_STRING_YES_NO(pAacs->BindingNonceGeneration),
		pAacs->BindingNonceBlockCount,
		pAacs->NumberOfAGIDs,
		pAacs->AACSVersion);
}

VOID OutputMmcFeatureVCPS(
	PFEATURE_VCPS pVcps
	)
{
	OutputDriveLog(
		_T("\tFeatureVCPS\n")
		_T("\t\t   Current: %d\n")
		_T("\t\tPersistent: %d\n")
		_T("\t\t   Version: %d\n"),
		pVcps->Header.Current,
		pVcps->Header.Persistent,
		pVcps->Header.Version);
}

VOID OutputMmcFeatureProfileList(
	PFEATURE_DATA_PROFILE_LIST pList
	)
{
	OutputDriveLog(_T("\tFeatureProfileList\n"));
	for (UINT i = 0; i < pList->Header.AdditionalLength / sizeof(FEATURE_DATA_PROFILE_LIST_EX); i++) {
		OutputDriveLog(_T("\t\t"));
		OutputMmcFeatureProfileType(
			MAKEWORD(pList->Profiles[i].ProfileNumber[1], pList->Profiles[i].ProfileNumber[0]));
		OutputDriveLog(_T("\n"));
	}
}

VOID OutputMmcFeatureVendorSpecific(
	PFEATURE_DATA_VENDOR_SPECIFIC pVendorSpecific
	)
{
	OutputDriveLog(
		_T("\tVendorSpecific. FeatureCode[0x%04x]\n")
		_T("\t\tVendorSpecificData: "), 
		MAKEWORD(pVendorSpecific->Header.FeatureCode[1], pVendorSpecific->Header.FeatureCode[0]));
	for (INT i = 0; i < pVendorSpecific->Header.AdditionalLength; i++) {
		OutputDriveLog(_T("%02x"), pVendorSpecific->VendorSpecificData[i]);
	}
	OutputDriveLog(_T("\n"));
}

VOID OutputMmcFeatureReserved(
	PFEATURE_DATA_RESERVED pReserved
	)
{
	OutputDriveLog(
		_T("\tReserved. FeatureCode[0x%04x]\n")
		_T("\t\tData: "), MAKEWORD(pReserved->Header.FeatureCode[1], pReserved->Header.FeatureCode[0]));
	for (INT i = 0; i < pReserved->Header.AdditionalLength; i++) {
		OutputDriveLog(_T("%02x"), pReserved->Data[i]);
	}
	OutputDriveLog(_T("\n"));
}

VOID OutputMmcFeatureNumber(
	PDEVICE_DATA pDevData,
	LPBYTE lpConf,
	DWORD dwAllLen,
	DWORD dwSize
	)
{
	DWORD n = 0;
	while (n < dwAllLen - dwSize) {
		WORD wCode = MAKEWORD(lpConf[dwSize + 1 + n], lpConf[dwSize + 0 + n]);
		switch (wCode) {
		case FeatureProfileList:
			OutputMmcFeatureProfileList((PFEATURE_DATA_PROFILE_LIST)&lpConf[dwSize + n]);
			break;
		case FeatureCore:
			OutputMmcFeatureCore((PFEATURE_DATA_CORE)&lpConf[dwSize + n]);
			break;
		case FeatureMorphing:
			OutputMmcFeatureMorphing((PFEATURE_DATA_MORPHING)&lpConf[dwSize + n]);
			break;
		case FeatureRemovableMedium:
			OutputMmcFeatureRemovableMedium((PFEATURE_DATA_REMOVABLE_MEDIUM)&lpConf[dwSize + n]);
			break;
		case FeatureWriteProtect:
			OutputMmcFeatureWriteProtect((PFEATURE_DATA_WRITE_PROTECT)&lpConf[dwSize + n]);
			break;
		case FeatureRandomReadable:
			OutputMmcFeatureRandomReadable((PFEATURE_DATA_RANDOM_READABLE)&lpConf[dwSize + n]);
			break;
		case FeatureMultiRead:
			OutputMmcFeatureMultiRead((PFEATURE_DATA_MULTI_READ)&lpConf[dwSize + n]);
			break;
		case FeatureCdRead:
			SetAndOutputMmcFeatureCdRead((PFEATURE_DATA_CD_READ)&lpConf[dwSize + n], pDevData);
			break;
		case FeatureDvdRead:
			OutputMmcFeatureDvdRead((PFEATURE_DATA_DVD_READ)&lpConf[dwSize + n]);
			break;
		case FeatureRandomWritable:
			OutputMmcFeatureRandomWritable((PFEATURE_DATA_RANDOM_WRITABLE)&lpConf[dwSize + n]);
			break;
		case FeatureIncrementalStreamingWritable:
			OutputMmcFeatureIncrementalStreamingWritable((PFEATURE_DATA_INCREMENTAL_STREAMING_WRITABLE)&lpConf[dwSize + n]);
			break;
		case FeatureSectorErasable:
			OutputMmcFeatureSectorErasable((PFEATURE_DATA_SECTOR_ERASABLE)&lpConf[dwSize + n]);
			break;
		case FeatureFormattable:
			OutputMmcFeatureFormattable((PFEATURE_DATA_FORMATTABLE)&lpConf[dwSize + n]);
			break;
		case FeatureDefectManagement:
			OutputMmcFeatureDefectManagement((PFEATURE_DATA_DEFECT_MANAGEMENT)&lpConf[dwSize + n]);
			break;
		case FeatureWriteOnce:
			OutputMmcFeatureWriteOnce((PFEATURE_DATA_WRITE_ONCE)&lpConf[dwSize + n]);
			break;
		case FeatureRestrictedOverwrite:
			OutputMmcFeatureRestrictedOverwrite((PFEATURE_DATA_RESTRICTED_OVERWRITE)&lpConf[dwSize + n]);
			break;
		case FeatureCdrwCAVWrite:
			OutputMmcFeatureCdrwCAVWrite((PFEATURE_DATA_CDRW_CAV_WRITE)&lpConf[dwSize + n]);
			break;
		case FeatureMrw:
			OutputMmcFeatureMrw((PFEATURE_DATA_MRW)&lpConf[dwSize + n]);
			break;
		case FeatureEnhancedDefectReporting:
			OutputMmcFeatureEnhancedDefectReporting((PFEATURE_ENHANCED_DEFECT_REPORTING)&lpConf[dwSize + n]);
			break;
		case FeatureDvdPlusRW:
			OutputMmcFeatureDvdPlusRW((PFEATURE_DATA_DVD_PLUS_RW)&lpConf[dwSize + n]);
			break;
		case FeatureDvdPlusR:
			OutputMmcFeatureDvdPlusR((PFEATURE_DATA_DVD_PLUS_R)&lpConf[dwSize + n]);
			break;
		case FeatureRigidRestrictedOverwrite:
			OutputMmcFeatureRigidRestrictedOverwrite((PFEATURE_DATA_DVD_RW_RESTRICTED_OVERWRITE)&lpConf[dwSize + n]);
			break;
		case FeatureCdTrackAtOnce:
			OutputMmcFeatureCdTrackAtOnce((PFEATURE_DATA_CD_TRACK_AT_ONCE)&lpConf[dwSize + n]);
			break;
		case FeatureCdMastering:
			OutputMmcFeatureCdMastering((PFEATURE_DATA_CD_MASTERING)&lpConf[dwSize + n]);
			break;
		case FeatureDvdRecordableWrite:
			OutputMmcFeatureDvdRecordableWrite((PFEATURE_DATA_DVD_RECORDABLE_WRITE)&lpConf[dwSize + n]);
			break;
		case FeatureLayerJumpRecording:
			OutputMmcFeatureLayerJumpRecording((PFEATURE_DATA_LAYER_JUMP_RECORDING)&lpConf[dwSize + n]);
			break;
		case FeatureCDRWMediaWriteSupport:
			OutputMmcFeatureCDRWMediaWriteSupport((PFEATURE_CD_RW_MEDIA_WRITE_SUPPORT)&lpConf[dwSize + n]);
			break;
		case FeatureDvdPlusRWDualLayer:
			OutputMmcFeatureDvdPlusRWDualLayer((PFEATURE_DATA_DVD_PLUS_RW_DUAL_LAYER)&lpConf[dwSize + n]);
			break;
		case FeatureDvdPlusRDualLayer:
			OutputMmcFeatureDvdPlusRDualLayer((PFEATURE_DATA_DVD_PLUS_R_DUAL_LAYER)&lpConf[dwSize + n]);
			break;
		case FeatureHybridDisc:
			OutputMmcFeatureHybridDisc((PFEATURE_HYBRID_DISC)&lpConf[dwSize + n]);
			break;
		case FeaturePowerManagement:
			OutputMmcFeaturePowerManagement((PFEATURE_DATA_POWER_MANAGEMENT)&lpConf[dwSize + n]);
			break;
		case FeatureSMART:
			OutputMmcFeatureSMART((PFEATURE_DATA_SMART)&lpConf[dwSize + n]);
			break;
		case FeatureEmbeddedChanger:
			OutputMmcFeatureEmbeddedChanger((PFEATURE_DATA_EMBEDDED_CHANGER)&lpConf[dwSize + n]);
			break;
		case FeatureCDAudioAnalogPlay:
			OutputMmcFeatureCDAudioAnalogPlay((PFEATURE_DATA_CD_AUDIO_ANALOG_PLAY)&lpConf[dwSize + n]);
			break;
		case FeatureMicrocodeUpgrade:
			OutputMmcFeatureMicrocodeUpgrade((PFEATURE_DATA_MICROCODE_UPDATE)&lpConf[dwSize + n]);
			break;
		case FeatureTimeout:
			OutputMmcFeatureTimeout((PFEATURE_DATA_TIMEOUT)&lpConf[dwSize + n]);
			break;
		case FeatureDvdCSS:
			OutputMmcFeatureDvdCSS((PFEATURE_DATA_DVD_CSS)&lpConf[dwSize + n]);
			break;
		case FeatureRealTimeStreaming:
			OutputMmcFeatureRealTimeStreaming((PFEATURE_DATA_REAL_TIME_STREAMING)&lpConf[dwSize + n]);
			break;
		case FeatureLogicalUnitSerialNumber:
			OutputMmcFeatureLogicalUnitSerialNumber((PFEATURE_DATA_LOGICAL_UNIT_SERIAL_NUMBER)&lpConf[dwSize + n]);
			break;
		case FeatureMediaSerialNumber:
			OutputMmcFeatureMediaSerialNumber((PFEATURE_MEDIA_SERIAL_NUMBER)&lpConf[dwSize + n]);
			break;
		case FeatureDiscControlBlocks:
			OutputMmcFeatureDiscControlBlocks((PFEATURE_DATA_DISC_CONTROL_BLOCKS)&lpConf[dwSize + n]);
			break;
		case FeatureDvdCPRM:
			OutputMmcFeatureDvdCPRM((PFEATURE_DATA_DVD_CPRM)&lpConf[dwSize + n]);
			break;
		case FeatureFirmwareDate:
			OutputMmcFeatureFirmwareDate((PFEATURE_DATA_FIRMWARE_DATE)&lpConf[dwSize + n]);
			break;
		case FeatureAACS:
			OutputMmcFeatureAACS((PFEATURE_DATA_AACS)&lpConf[dwSize + n]);
			break;
		case FeatureVCPS:
			OutputMmcFeatureVCPS((PFEATURE_VCPS)&lpConf[dwSize + n]);
			break;
		default:
			if (0x0111 <= wCode && wCode <= 0xFEFF) {
				OutputMmcFeatureReserved((PFEATURE_DATA_RESERVED)&lpConf[dwSize + n]);
			}
			else if (0xFF00 <= wCode && wCode <= 0xFFFF) {
				OutputMmcFeatureVendorSpecific((PFEATURE_DATA_VENDOR_SPECIFIC)&lpConf[dwSize + n]);
			}
			break;
		}
		n += lpConf[dwSize + 3 + n] + sizeof(FEATURE_HEADER);
	}
}

VOID OutputMmcFeatureProfileType(
	WORD wFeatureProfileType
	)
{
	switch (wFeatureProfileType) {
		case ProfileInvalid:
			OutputDriveLog(_T("Invalid"));
			break;
		case ProfileNonRemovableDisk:
			OutputDriveLog(_T("NonRemovableDisk"));
			break;
		case ProfileRemovableDisk:
			OutputDriveLog(_T("RemovableDisk"));
			break;
		case ProfileMOErasable:
			OutputDriveLog(_T("MOErasable"));
			break;
		case ProfileMOWriteOnce:
			OutputDriveLog(_T("MOWriteOnce"));
			break;
		case ProfileAS_MO:
			OutputDriveLog(_T("AS_MO"));
			break;
		case ProfileCdrom:
			OutputDriveLog(_T("CD-ROM"));
			break;
		case ProfileCdRecordable:
			OutputDriveLog(_T("CD-R"));
			break;
		case ProfileCdRewritable:
			OutputDriveLog(_T("CD-RW"));
			break;
		case ProfileDvdRom:
			OutputDriveLog(_T("DVD-ROM"));
			break;
		case ProfileDvdRecordable:
			OutputDriveLog(_T("DVD-R"));
			break;
		case ProfileDvdRam:
			OutputDriveLog(_T("DVD-RAM"));
			break;
		case ProfileDvdRewritable:
			OutputDriveLog(_T("DVD-RW"));
			break;
		case ProfileDvdRWSequential:
			OutputDriveLog(_T("DVD-RW Sequential"));
			break;
		case ProfileDvdDashRDualLayer:
			OutputDriveLog(_T("DVD-R DL"));
			break;
		case ProfileDvdDashRLayerJump:
			OutputDriveLog(_T("DVD-R LayerJump"));
			break;
		case ProfileDvdPlusRW:
			OutputDriveLog(_T("DVD+RW"));
			break;
		case ProfileDvdPlusR:
			OutputDriveLog(_T("DVD+R"));
			break;
		case ProfileDvdPlusRWDualLayer:
			OutputDriveLog(_T("DVD+RW DL"));
			break;
		case ProfileDvdPlusRDualLayer:
			OutputDriveLog(_T("DVD+R DL"));
			break;
		case ProfileNonStandard:
			OutputDriveLog(_T("NonStandard"));
			break;
		default:
			OutputDriveLog(_T("Reserved [%x]"), wFeatureProfileType);
			break;
	}
}

VOID OutputMmcInquiryData(
	PDEVICE_DATA pDevData,
	PINQUIRYDATA pInquiry
	)
{
	OutputDriveLog(
		_T("Inquiry Data\n")
		_T("\t          DeviceType: "));
	switch (pInquiry->DeviceType) {
		case READ_ONLY_DIRECT_ACCESS_DEVICE:
			OutputDriveLog(_T("CD/DVD device\n"));
			break;
		default:
			OutputDriveLog(_T("Other device\n"));
			break;
	}
	OutputDriveLog(
		_T("\t DeviceTypeQualifier: "));
	switch (pInquiry->DeviceTypeQualifier) {
		case DEVICE_QUALIFIER_ACTIVE:
			OutputDriveLog(_T("Active\n"));
			break;
		case DEVICE_QUALIFIER_NOT_ACTIVE:
			OutputDriveLog(_T("Not Active\n"));
			break;
		case DEVICE_QUALIFIER_NOT_SUPPORTED:
			OutputDriveLog(_T("Not Supported\n"));
			break;
		default:
			OutputDriveLog(_T("\n"));
			break;
	}

	OutputDriveLog(
		_T("\t  DeviceTypeModifier: %d\n")
		_T("\t      RemovableMedia: %s\n")
		_T("\t            Versions: %d\n")
		_T("\t  ResponseDataFormat: %d\n")
		_T("\t           HiSupport: %s\n")
		_T("\t             NormACA: %s\n")
		_T("\t       TerminateTask: %s\n")
		_T("\t                AERC: %s\n")
		_T("\t    AdditionalLength: %d\n")
		_T("\t       MediumChanger: %s\n")
		_T("\t           MultiPort: %s\n")
		_T("\t   EnclosureServices: %s\n")
		_T("\t           SoftReset: %s\n")
		_T("\t        CommandQueue: %s\n")
		_T("\t      LinkedCommands: %s\n")
		_T("\t  RelativeAddressing: %s\n"),
		pInquiry->DeviceTypeModifier,
		BOOLEAN_TO_STRING_YES_NO(pInquiry->RemovableMedia),
		pInquiry->Versions,
		pInquiry->ResponseDataFormat,
		BOOLEAN_TO_STRING_YES_NO(pInquiry->HiSupport),
		BOOLEAN_TO_STRING_YES_NO(pInquiry->NormACA),
		BOOLEAN_TO_STRING_YES_NO(pInquiry->TerminateTask),
		BOOLEAN_TO_STRING_YES_NO(pInquiry->AERC),
		pInquiry->AdditionalLength,
		BOOLEAN_TO_STRING_YES_NO(pInquiry->MediumChanger),
		BOOLEAN_TO_STRING_YES_NO(pInquiry->MultiPort),
		BOOLEAN_TO_STRING_YES_NO(pInquiry->EnclosureServices),
		BOOLEAN_TO_STRING_YES_NO(pInquiry->SoftReset),
		BOOLEAN_TO_STRING_YES_NO(pInquiry->CommandQueue),
		BOOLEAN_TO_STRING_YES_NO(pInquiry->LinkedCommands),
		BOOLEAN_TO_STRING_YES_NO(pInquiry->RelativeAddressing));

	strncpy(pDevData->szVendorId, 
		(PCHAR)pInquiry->VendorId, sizeof(pInquiry->VendorId));
	strncpy(pDevData->szProductId, 
		(PCHAR)pInquiry->ProductId, sizeof(pInquiry->ProductId));
#ifdef UNICODE
	_TCHAR buf1[8] = { 0 };
	_TCHAR buf2[16] = { 0 };
	_TCHAR buf3[4] = { 0 };
	_TCHAR buf4[20] = { 0 };
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)pInquiry->VendorId,
		sizeof(pInquiry->VendorId) / sizeof(pInquiry->VendorId[0]), 
		buf1, sizeof(buf1) / sizeof(buf1[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)pInquiry->ProductId,
		sizeof(pInquiry->ProductId) / sizeof(pInquiry->ProductId[0]), 
		buf2, sizeof(buf2) / sizeof(buf2[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)pInquiry->ProductRevisionLevel,
		sizeof(pInquiry->ProductRevisionLevel) / sizeof(pInquiry->ProductRevisionLevel[0]), 
		buf3, sizeof(buf3) / sizeof(buf3[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	if (!MultiByteToWideChar(CP_ACP, 0, (PCHAR)pInquiry->VendorSpecific,
		sizeof(pInquiry->VendorSpecific) / sizeof(pInquiry->VendorSpecific[0]), 
		buf4, sizeof(buf4) / sizeof(buf4[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	OutputDriveLog(
		_T("\t            VendorId: %.8s\n")
		_T("\t           ProductId: %.16s\n")
		_T("\tProductRevisionLevel: %.4s\n")
		_T("\t      VendorSpecific: %.20s\n"),
		buf1,
		buf2,
		buf3,
		buf4);
#else
	OutputDriveLog(
		_T("\t            VendorId: %.8s\n")
		_T("\t           ProductId: %.16s\n")
		_T("\tProductRevisionLevel: %.4s\n")
		_T("\t      VendorSpecific: %.20s\n"),
		pInquiry->VendorId,
		pInquiry->ProductId,
		pInquiry->ProductRevisionLevel,
		pInquiry->VendorSpecific);
#endif
}

VOID OutputMmcTocWithPregap(
	PDISC_DATA pDiscData,
	LPBYTE lpCtlList,
	LPBYTE lpModeList,
	LPINT* lpLBAStartList
	)
{
	OutputDiscLog(_T("TOC with pregap\n"));
	for (INT r = 0; r < pDiscData->toc.LastTrack; r++) {
		OutputDiscLog(
			_T("\tTrack %2d, Ctl %d, Mode %d"), r + 1, lpCtlList[r], lpModeList[r]);
		for (INT k = 0; k < MAXIMUM_NUMBER_INDEXES; k++) {
			if (lpLBAStartList[r][k] != -1) {
				OutputDiscLog(_T(", Index%d %6d"), k, lpLBAStartList[r][k]);
			}
			else if (k == 0) {
				OutputDiscLog(_T(",              "));
			}
		}
		OutputDiscLog(_T("\n"));
	}
}

VOID OutputMmcCDC2Error296(
	LPBYTE lpBuf,
	INT nLBA
	)
{
	OutputDiscLog(
		_T("c2 error LBA %d\n")
		_T("\t         +0 +1 +2 +3 +4 +5 +6 +7\n"),
		nLBA);

	for (INT i = 0; i < CD_RAW_READ_C2_SIZE; i += 8) {
		OutputDiscLog(
			_T("\t%3x(%3d) %02x %02x %02x %02x %02x %02x %02x %02x\n"), 
			i, i, lpBuf[i], lpBuf[i + 1], lpBuf[i + 2], lpBuf[i + 3],
			lpBuf[i + 4], lpBuf[i + 5], lpBuf[i + 6], lpBuf[i + 7]);
	}
}

VOID OutputMmcCDMain2352(
	LPBYTE lpBuf,
	INT nLBA
	)
{
	OutputDiscLog(
		_T("Main Channel LBA %d\n")
		_T("\t          +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n"), nLBA);

	for (INT i = 0; i < CD_RAW_SECTOR_SIZE; i += 16) {
		OutputDiscLog(
			_T("\t%3x(%4d) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"), 
			i, i, lpBuf[i], lpBuf[i + 1], lpBuf[i + 2], lpBuf[i + 3], lpBuf[i + 4], lpBuf[i + 5],
			lpBuf[i + 6], lpBuf[i + 7], lpBuf[i + 8], lpBuf[i + 9], lpBuf[i + 10], lpBuf[i + 11],
			lpBuf[i + 12], lpBuf[i + 13], lpBuf[i + 14], lpBuf[i + 15]);
	}
}

VOID OutputMmcCDSub96Align(
	LPBYTE lpBuf,
	INT nLBA
	)
{
	OutputDiscLog(
		_T("Sub Channel LBA %d\n")
		_T("\t  +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B\n"), nLBA);

	for (INT i = 0, ch = 0x50; i < CD_RAW_READ_SUBCODE_SIZE; i += 12, ch++) {
		OutputDiscLog(
			_T("\t%c %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"), 
			ch, lpBuf[i], lpBuf[i + 1], lpBuf[i + 2], lpBuf[i + 3], lpBuf[i + 4], lpBuf[i + 5],
			lpBuf[i + 6], lpBuf[i + 7], lpBuf[i + 8], lpBuf[i + 9], lpBuf[i + 10], lpBuf[i + 11]);
	}
}

VOID OutputMmcCDSub96Raw(
	LPBYTE lpBuf,
	INT nLBA
	)
{
	OutputDiscLog(
		_T("Sub Channel(Raw) LBA %d\n")
		_T("\t    +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n"), nLBA);

	for (INT i = 0; i < CD_RAW_READ_SUBCODE_SIZE; i += 16) {
		OutputDiscLog(
			_T("\t%3X %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"), 
			i, lpBuf[i], lpBuf[i + 1], lpBuf[i + 2], lpBuf[i + 3], lpBuf[i + 4], lpBuf[i + 5],
			lpBuf[i + 6], lpBuf[i + 7], lpBuf[i + 8], lpBuf[i + 9], lpBuf[i + 10], lpBuf[i + 11],
			lpBuf[i + 12], lpBuf[i + 13], lpBuf[i + 14], lpBuf[i + 15]);
	}
}

VOID OutputMmcCDSubToLog(
	PDISC_DATA pDiscData,
	LPBYTE lpSubcode,
	LPBYTE lpSubcodeOrg,
	INT nLBA,
	INT nTrackNum,
	FILE* fpParse
	)
{
	_TCHAR str[256] = { 0 };
	_stprintf(str, _T("LBA[%06d, 0x%05X], "), nLBA, nLBA);
	// Ctl
	switch ((lpSubcode[12] >> 4) & 0x0F) {
	case 0:
		_tcscat(str, _T("Audio, 2ch, Copy NG, Pre-emphasis No, "));
		break;
	case AUDIO_WITH_PREEMPHASIS:
		_tcscat(str, _T("Audio, 2ch, Copy NG, Pre-emphasis 50/15, "));
		break;
	case DIGITAL_COPY_PERMITTED:
		_tcscat(str, _T("Audio, 2ch, Copy OK, Pre-emphasis No, "));
		break;
	case DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
		_tcscat(str, _T("Audio, 2ch, Copy OK, Pre-emphasis 50/15, "));
		break;
	case AUDIO_DATA_TRACK:
		_tcscat(str, _T("Data, Copy NG, "));
		break;
	case AUDIO_DATA_TRACK | DIGITAL_COPY_PERMITTED:
		_tcscat(str, _T("Data, Copy OK, "));
		break;
	case TWO_FOUR_CHANNEL_AUDIO:
		_tcscat(str, _T("Audio, 4ch, Copy NG, Pre-emphasis No, "));
		break;
	case TWO_FOUR_CHANNEL_AUDIO | AUDIO_WITH_PREEMPHASIS:
		_tcscat(str, _T("Audio, 4ch, Copy NG, Pre-emphasis 50/15, "));
		break;
	case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED:
		_tcscat(str, _T("Audio, 4ch, Copy OK, Pre-emphasis No, "));
		break;
	case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
		_tcscat(str, _T("Audio, 4ch, Copy OK, Pre-emphasis 50/15, "));
		break;
	default:
		_tcscat(str, _T("Unknown, "));
		break;
	}

	// ADR
	_TCHAR str2[256] = { 0 };
	switch (lpSubcode[12] & 0x0F) {
	case ADR_ENCODES_CURRENT_POSITION:
		if (nLBA < -150 && lpSubcode[13] == 0) {
			// lead-in area
			if (lpSubcode[14] == 0xA0) {
				_stprintf(str2, 
					_T("TOC[Point-%02x, RunningTime-%02x:%02x:%02x, TrackNumOf1stTrack-%02x, ProgramAreaFormat-%02x] "), 
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17],	lpSubcode[19], lpSubcode[20]);
			}
			else if (lpSubcode[14] == 0xA1) {
				_stprintf(str2, 
					_T("TOC[Point-%02x, RunningTime-%02x:%02x:%02x, TrackNumOfLastTrack-%02x] "), 
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17],	lpSubcode[19]);
			}
			else if (lpSubcode[14] == 0xA2) {
				_stprintf(str2, 
					_T("TOC[Point-%02x, RunningTime-%02x:%02x:%02x, StartTimeOfLead-out-%02x:%02x:%02x] "), 
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17],
					lpSubcode[19], lpSubcode[20], lpSubcode[21]);
			}
			else {
				_stprintf(str2, 
					_T("TOC[Point-%02x, RunningTime-%02x:%02x:%02x, TrackStartTime-%02x:%02x:%02x] "), 
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17],
					lpSubcode[19], lpSubcode[20], lpSubcode[21]);
			}
		}
		else if (lpSubcode[13] == 0xAA) {
			// lead-out area
			_stprintf(str2, 
				_T("TOC[LeadOut    , Index-%02x, RelativeTime-%02x:%02x:%02x, AbsoluteTime-%02x:%02x:%02x] "), 
				lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17], 
				lpSubcode[19], lpSubcode[20], lpSubcode[21]);
		}
		else {
			_stprintf(str2, 
				_T("TOC[TrackNum-%02x, Index-%02x, RelativeTime-%02x:%02x:%02x, AbsoluteTime-%02x:%02x:%02x] "), 
				lpSubcode[13], lpSubcode[14], lpSubcode[15], lpSubcode[16], 
				lpSubcode[17], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
		}
		break;
	case ADR_ENCODES_MEDIA_CATALOG:
		_stprintf(str2, 
			_T("Media Catalog Number (MCN)[%s        , AbsoluteTime-     :%02x] "), 
			pDiscData->szCatalog, lpSubcode[21]);
		break;
	case ADR_ENCODES_ISRC:
		_stprintf(str2, 
			_T("International Standard... (ISRC)[%s   , AbsoluteTime-     :%02x] "), 
			pDiscData->pszISRC[nTrackNum-1], lpSubcode[21]);
		break;
	case 5:
		if (lpSubcode[14] == 0xB0) {
			_stprintf(str2, 
				_T("TOC[TrackNum-%02x, Point-%02x, StartTimeForTheNexSession-%02x:%02x:%02x, NumberOfDifferentMode-5-%02x, OutermostLead-out-%02x:%02x:%02x] "), 
				lpSubcode[13], lpSubcode[14], lpSubcode[15], lpSubcode[16], 
				lpSubcode[17], lpSubcode[18], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
		}
		else if (lpSubcode[14] == 0xB1) {
			_stprintf(str2, 
				_T("TOC[TrackNum-%02x, Point-%02x, NumberOfSkipIntervalPointers-%02x, NumberOfSkipTrackAssignmentsInPOINT-%02x] "), 
				lpSubcode[13], lpSubcode[14], lpSubcode[19], lpSubcode[20]);
		}
		else if (lpSubcode[14] == 0xB2 || lpSubcode[14] == 0xB3 || lpSubcode[14] == 0xB4) {
			_stprintf(str2, 
				_T("TOC[TrackNum-%02x, Point-%02x, TrackNumberToSkipUponPlayback-%02x, %02x, %02x, %02x, %02x, %02x] "), 
				lpSubcode[13], lpSubcode[14], lpSubcode[15], lpSubcode[16], 
				lpSubcode[17], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
		}
		else if (lpSubcode[14] == 0xC0) {
			_stprintf(str2, 
				_T("TOC[TrackNum-%02x, Point-%02x, ATIPValues-%02x, %02x, %02x, StartTimeOfTheFirstLead-in-%02x:%02x:%02x] "), 
				lpSubcode[13], lpSubcode[14], lpSubcode[15], lpSubcode[16], 
				lpSubcode[17], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
		}
		else {
			_stprintf(str2, 
				_T("TOC[TrackNum-%02x, Point-%02x, SkipIntervalStopTime-%02x:%02x:%02x, SkipIntervalStartTime-%02x:%02x:%02x] "), 
				lpSubcode[13], lpSubcode[14], lpSubcode[15], lpSubcode[16], 
				lpSubcode[17], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
		}
		break;
	default:
		_stprintf(str2, 
			_T("TOC[TrackNum-%02x, Index-%02x, RelativeTime-%02x:%02x:%02x, AbsoluteTime-%02x:%02x:%02x] "), 
			lpSubcode[13], lpSubcode[14], lpSubcode[15], lpSubcode[16], 
			lpSubcode[17], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
		break;
	}
	_tcscat(str, str2);

	// EN 60908:1999 Page25-
	typedef struct _SubcodeRtoW {
        CHAR command;
        CHAR instruction;
        CHAR parityQ[2];
        CHAR data[16];
        CHAR parityP[4];
	} SubcodeRtoW;

	SubcodeRtoW scRW[4] = { 0 };
	BYTE tmpCode[24] = { 0 };
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 24; j++) {
			tmpCode[j] = (BYTE)(*(lpSubcodeOrg + (i * 24 + j)) & 0x3F);
		}
		memcpy(&scRW[i], tmpCode, sizeof(scRW[i]));

		switch (scRW[i].command) {
		case 0: // MODE 0, ITEM 0
			_tcscat(str, _T("RtoW:ZERO"));
			break;
		case 8: // MODE 1, ITEM 0
			_tcscat(str, _T("RtoW:LINE-GRAPHICS"));
			break;
		case 9: // MODE 1, ITEM 1
			_tcscat(str, _T("RtoW:TV-GRAPHICS"));
			break;
		case 10: // MODE 1, ITEM 2
			_tcscat(str, _T("RtoW:EXTENDED-TV-GRAPHICS"));
			break;
		case 20: // MODE 2, ITEM 4
			_tcscat(str, _T("RtoW:CD TEXT"));
			break;
		case 24: // MODE 3, ITEM 0
			_tcscat(str, _T("RtoW:MIDI"));
			break;
		case 56: // MODE 7, ITEM 0
			_tcscat(str, _T("RtoW:USER"));
			break;
		default:
			_tcscat(str, _T("RtoW:Reserved"));
			break;
		}
		if (i < 3) {
			_tcscat(str, _T(", "));
		}
		else {
			_tcscat(str, _T("\n"));
		}
	}
	fwrite(str, sizeof(_TCHAR), _tcslen(str), fpParse);
}

VOID OutputMmcDvdLayerDescriptor(
	PDISC_DATA pDiscData,
	PDVD_FULL_LAYER_DESCRIPTOR dvdLayer,
	LPBYTE lpLayerNum,
	INT nNum
	)
{
	LPCTSTR lpBookType[] = {
		_T("DVD-ROM"), _T("DVD-RAM"), _T("DVD-R"), _T("DVD-RW"),
		_T("HD DVD-ROM"), _T("HD DVD-RAM"), _T("HD DVD-R"), _T("Reserved"),
		_T("Reserved"), _T("DVD+RW"), _T("DVD+R"), _T("Reserved"),
		_T("Reserved"), _T("DVD+RW DL"), _T("DVD+R DL"), _T("Reserved")
	};

	LPCTSTR lpMaximumRate[] = {
		_T("2.52 Mbps"), _T("5.04 Mbps"), _T("10.08 Mbps"), _T("20.16 Mbps"),
		_T("30.24 Mbps"), _T("Reserved"), _T("Reserved"), _T("Reserved"),
		_T("Reserved"), _T("Reserved"), _T("Reserved"), _T("Reserved"),
		_T("Reserved"), _T("Reserved"), _T("Reserved"), _T("Not Specified")
	};

	LPCTSTR lpLayerType[] = {
		_T("Unknown"), _T("Layer contains embossed data"), _T("Layer contains recordable data"), _T("Unknown"),
		_T("Layer contains rewritable data"), _T("Unknown"), _T("Unknown"), _T("Unknown"),
		_T("Reserved"), _T("Unknown"), _T("Unknown"), _T("Unknown"),
		_T("Unknown"), _T("Unknown"), _T("Unknown"), _T("Unknown")
	};

	LPCTSTR lpTrackDensity[] = {
		_T("0.74ƒÊm/track"), _T("0.80ƒÊm/track"), _T("0.615ƒÊm/track"), _T("0.40ƒÊm/track"),
		_T("0.34ƒÊm/track"), _T("Reserved"), _T("Reserved"), _T("Reserved"),
		_T("Reserved"), _T("Reserved"), _T("Reserved"), _T("Reserved"),
		_T("Reserved"), _T("Reserved"), _T("Reserved"), _T("Reserved")
	};

	LPCTSTR lpLinearDensity[] = {
		_T("0.267ƒÊm/bit"), _T("0.293ƒÊm/bit"), _T("0.409 to 0.435ƒÊm/bit"), _T("Reserved"),
		_T("0.280 to 0.291ƒÊm/bit"), _T("0.153ƒÊm/bit"), _T("0.130 to 0.140ƒÊm/bit"), _T("Reserved"),
		_T("0.353ƒÊm/bit"), _T("Reserved"), _T("Reserved"), _T("Reserved"),
		_T("Reserved"), _T("Reserved"), _T("Reserved"), _T("Reserved")
	};

	*lpLayerNum = dvdLayer->commonHeader.NumberOfLayers;
	DWORD dwStartSector = dvdLayer->commonHeader.StartingDataSector;
	DWORD dwEndSector = dvdLayer->commonHeader.EndDataSector;
	DWORD dwEndSectorLayer0 = dvdLayer->commonHeader.EndLayerZeroSector;
	REVERSE_LONG(&dwStartSector);
	REVERSE_LONG(&dwEndSector);
	REVERSE_LONG(&dwEndSectorLayer0);

	OutputDiscLog(
		_T("\tPhysicalFormatInformation\n")
		_T("\t\t       BookVersion: %d\n")
		_T("\t\t          BookType: %s\n")
		_T("\t\t       MinimumRate: %s\n")
		_T("\t\t          DiskSize: %s\n")
		_T("\t\t         LayerType: %s\n")
		_T("\t\t         TrackPath: %s\n")
		_T("\t\t    NumberOfLayers: %s\n")
		_T("\t\t      TrackDensity: %s\n")
		_T("\t\t     LinearDensity: %s\n")
		_T("\t\t   StartDataSector: %8d (0x%x)\n")
		_T("\t\t     EndDataSector: %8d (0x%x)\n")
		_T("\t\tEndLayerZeroSector: %8d (0x%x)\n")
		_T("\t\t           BCAFlag: %s\n")
		_T("\t\t     MediaSpecific: "),
		dvdLayer->commonHeader.BookVersion,
		lpBookType[dvdLayer->commonHeader.BookType],
		lpMaximumRate[dvdLayer->commonHeader.MinimumRate],
		dvdLayer->commonHeader.DiskSize == 0 ? _T("120mm") : _T("80mm"),
		lpLayerType[dvdLayer->commonHeader.LayerType],
		dvdLayer->commonHeader.TrackPath == 0 ? _T("Parallel Track Path") : _T("Opposite Track Path"),
		dvdLayer->commonHeader.NumberOfLayers == 0 ? _T("Single Layer") : _T("Double Layer"),
		lpTrackDensity[dvdLayer->commonHeader.TrackDensity],
		lpLinearDensity[dvdLayer->commonHeader.LinearDensity],
		dwStartSector, dwStartSector,
		dwEndSector, dwEndSector,
		dwEndSectorLayer0, dwEndSectorLayer0,
		dvdLayer->commonHeader.BCAFlag == 0 ? _T("None") : _T("Exist"));

	for (DWORD k = 0; k < sizeof(dvdLayer->MediaSpecific); k++) {
		OutputDiscLog(_T("%02x"), dvdLayer->MediaSpecific[k]);
	}

	DWORD dwSector = 0;
	if (dvdLayer->commonHeader.TrackPath) {
		dwSector = dwEndSectorLayer0 - dwStartSector + 1;
	}
	else {
		dwSector = dwEndSector - dwStartSector + 1;
	}
	if (!pDiscData->bSuccessReadToc) {
		pDiscData->nAllLength += dwSector;
	}
	OutputDiscLog(_T("\n"));
	OutputDiscLog(
		_T("\t\t         L%d Sector: %8d (0x%x)\n"), nNum, dwSector, dwSector);
}

VOID OutputMmcDvdCopyrightDescriptor(
	PDVD_COPYRIGHT_DESCRIPTOR dvdCopyright
	)
{
	OutputDiscLog(_T("\tCopyrightProtectionType: "));
	switch (dvdCopyright->CopyrightProtectionType) {
	case 0:
		OutputDiscLog(_T("No\n"));
		break;
	case 1:
		OutputDiscLog(_T("CSS/CPPM\n"));
		break;
	case 2:
		OutputDiscLog(_T("CPRM\n"));
		break;
	case 3:
		OutputDiscLog(_T("AACS with HD DVD content\n"));
		break;
	case 10:
		OutputDiscLog(_T("AACS with BD content\n"));
		break;
	default:
		OutputDiscLog(_T("Unknown: %02x\n"), dvdCopyright->CopyrightProtectionType);
		break;
	}
	OutputDiscLog(
		_T("\tRegionManagementInformation: %02x\n"), dvdCopyright->RegionManagementInformation);
}

VOID OutputMmcDvdDiskKeyDescriptor(
	PDVD_DISK_KEY_DESCRIPTOR dvdDiskKey
	)
{
	OutputDiscLog(_T("\tDiskKeyData: "));
	for (DWORD k = 0; k < sizeof(dvdDiskKey->DiskKeyData); k++) {
		OutputDiscLog(_T("%02x"), dvdDiskKey->DiskKeyData[k]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputMmcDvdBCADescriptor(
	PDVD_BCA_DESCRIPTOR dvdBca,
	LPWORD lpStructureLength,
	size_t uiFormatCode
	)
{
	OutputDiscLog(_T("\tBCAInformation: "));
	for (DWORD k = 0;
		k < lpStructureLength[uiFormatCode] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
		OutputDiscLog(_T("%02x"), dvdBca->BCAInformation[k]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputMmcDvdManufacturerDescriptor(
	PDVD_MANUFACTURER_DESCRIPTOR dvdManufacturer
	)
{
	OutputDiscLog(_T("\tManufacturingInformation: "));
	for (DWORD k = 0; k < sizeof(dvdManufacturer->ManufacturingInformation); k++) {
		OutputDiscLog(_T("%02x"), dvdManufacturer->ManufacturingInformation[k]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputMmcDvdMediaId(
	LPBYTE lpStructure,
	LPWORD lpStructureLength,
	size_t uiFormatCode
	)
{
	OutputDiscLog(_T("\tmedia ID: "));
	for (DWORD k = 0;
		k < lpStructureLength[uiFormatCode] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
		OutputDiscLog(_T("%02x"), lpStructure[k]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputMmcDvdMediaKeyBlock(
	LPBYTE lpStructure,
	LPWORD lpStructureLength,
	size_t uiFormatCode
	)
{
	OutputDiscLog(
		_T("\tMedia Key Block Total Packs: %d")
		_T("\tmedia key block: "),
		lpStructure[3]);
	for (DWORD k = 0;
		k < lpStructureLength[uiFormatCode] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
		OutputDiscLog(_T("%02x"), lpStructure[k]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputMmcDvdRamDds(
	LPBYTE lpStructure,
	LPWORD lpStructureLength,
	size_t uiFormatCode
	)
{
	OutputDiscLog(_T("\tDVD-RAM DDS: "));
	for (DWORD k = 0;
		k < lpStructureLength[uiFormatCode] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
		OutputDiscLog(_T("%02x"), lpStructure[k]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputMmcDvdRamMediumStatus(
	PDVD_RAM_MEDIUM_STATUS dvdRamMeium
	)
{
	OutputDiscLog(
		_T("\tDvdRamMediumStatus\n")
		_T("\t\t              PersistentWriteProtect: %s\n")
		_T("\t\t               CartridgeWriteProtect: %s\n")
		_T("\t\t           MediaSpecificWriteInhibit: %s\n")
		_T("\t\t                  CartridgeNotSealed: %s\n")
		_T("\t\t                    MediaInCartridge: %s\n")
		_T("\t\t              DiscTypeIdentification: %x\n")
		_T("\t\tMediaSpecificWriteInhibitInformation: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(dvdRamMeium->PersistentWriteProtect),
		BOOLEAN_TO_STRING_YES_NO(dvdRamMeium->CartridgeWriteProtect),
		BOOLEAN_TO_STRING_YES_NO(dvdRamMeium->MediaSpecificWriteInhibit),
		BOOLEAN_TO_STRING_YES_NO(dvdRamMeium->CartridgeNotSealed),
		BOOLEAN_TO_STRING_YES_NO(dvdRamMeium->MediaInCartridge),
		dvdRamMeium->DiscTypeIdentification,
		BOOLEAN_TO_STRING_YES_NO(dvdRamMeium->MediaSpecificWriteInhibitInformation));
}

VOID OutputMmcDvdRamSpareArea(
	PDVD_RAM_SPARE_AREA_INFORMATION dvdRamSpare
	)
{
	OutputDiscLog(
		_T("\tDvdRamSpareAreaInformation\n")
		_T("\t\t          FreePrimarySpareSectors: %d\n")
		_T("\t\t     FreeSupplementalSpareSectors: %d\n")
		_T("\t\tAllocatedSupplementalSpareSectors: %d\n"),
		MAKELONG(
		MAKEWORD(dvdRamSpare->FreePrimarySpareSectors[3], dvdRamSpare->FreePrimarySpareSectors[2]),
		MAKEWORD(dvdRamSpare->FreePrimarySpareSectors[1], dvdRamSpare->FreePrimarySpareSectors[0])),
		MAKELONG(
		MAKEWORD(dvdRamSpare->FreeSupplementalSpareSectors[3], dvdRamSpare->FreeSupplementalSpareSectors[2]),
		MAKEWORD(dvdRamSpare->FreeSupplementalSpareSectors[1], dvdRamSpare->FreeSupplementalSpareSectors[0])),
		MAKELONG(
		MAKEWORD(dvdRamSpare->AllocatedSupplementalSpareSectors[3], dvdRamSpare->AllocatedSupplementalSpareSectors[2]),
		MAKEWORD(dvdRamSpare->AllocatedSupplementalSpareSectors[1], dvdRamSpare->AllocatedSupplementalSpareSectors[0])));
}

VOID OutputMmcDvdRamRecordingType(
	PDVD_RAM_RECORDING_TYPE dvdRamRecording
	)
{
	OutputDiscLog(
		_T("\tDvdRamRecordingType\n")
		_T("\t\tRealTimeData: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(dvdRamRecording->RealTimeData));
}

VOID OutputMmcDvdRmdLastBorderOut(
	LPBYTE lpStructure,
	LPWORD lpStructureLength,
	size_t uiFormatCode
	)
{
	OutputDiscLog(
		_T("\tRMD in last border-out: "));
	for (DWORD k = 0;
		k < lpStructureLength[uiFormatCode] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
		OutputDiscLog(_T("%02x"), lpStructure[k]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputMmcDvdRecordingManagementAreaData(
	PDVD_RECORDING_MANAGEMENT_AREA_DATA dvdRecordingMan,
	LPWORD lpStructureLength,
	size_t uiFormatCode
	)
{
	OutputDiscLog(
		_T("\tDVD_RECORDING_MANAGEMENT_AREA_DATA\n")
		_T("\t\tLastRecordedRMASectorNumber: %d\n")
		_T("\t\t                   RMDBytes: "),
		MAKELONG(MAKEWORD(dvdRecordingMan->LastRecordedRMASectorNumber[3], 
		dvdRecordingMan->LastRecordedRMASectorNumber[2]),
		MAKEWORD(dvdRecordingMan->LastRecordedRMASectorNumber[1], 
		dvdRecordingMan->LastRecordedRMASectorNumber[0])));
	for (DWORD k = 0;
		k < lpStructureLength[uiFormatCode] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
		OutputDiscLog(_T("%02x"), dvdRecordingMan->RMDBytes[k]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputMmcDvdPreRecordedInformation(
	PDVD_PRERECORDED_INFORMATION dvdPreRecorded
	)
{
	OutputDiscLog(
		_T("\tDVD_PRERECORDED_INFORMATION\n")
		_T("\t\t                      FieldID_1: %d\n")
		_T("\t\t            DiscApplicatiowCode: %d\n")
		_T("\t\t               DiscPhysicalCode: %d\n")
		_T("\t\tLastAddressOfDataRecordableArea: %d\n")
		_T("\t\t                  ExtensiowCode: %d\n")
		_T("\t\t                    PartVers1on: %d\n")
		_T("\t\t                      FieldID_2: %d\n")
		_T("\t\t               OpcSuggestedCode: %d\n")
		_T("\t\t                 WavelengthCode: %d\n")
		_T("\t\t              WriteStrategyCode: %d\n")
		_T("\t\t                      FieldID_3: %d\n")
		_T("\t\t               ManufacturerId_3: %c%c%c%c%c%c\n")
		_T("\t\t                      FieldID_4: %d\n")
		_T("\t\t               ManufacturerId_4: %c%c%c%c%c%c\n")
		_T("\t\t                      FieldID_5: %d\n")
		_T("\t\t               ManufacturerId_5: %c%c%c%c%c%c\n"),
		dvdPreRecorded->FieldID_1,
		dvdPreRecorded->DiscApplicationCode,
		dvdPreRecorded->DiscPhysicalCode,
		MAKELONG(MAKEWORD(0, dvdPreRecorded->LastAddressOfDataRecordableArea[2]),
		MAKEWORD(dvdPreRecorded->LastAddressOfDataRecordableArea[1],
		dvdPreRecorded->LastAddressOfDataRecordableArea[0])),
		dvdPreRecorded->ExtensionCode,
		dvdPreRecorded->PartVers1on,
		dvdPreRecorded->FieldID_2,
		dvdPreRecorded->OpcSuggestedCode,
		dvdPreRecorded->WavelengthCode,
		MAKELONG(MAKEWORD(dvdPreRecorded->WriteStrategyCode[3], dvdPreRecorded->WriteStrategyCode[2]),
		MAKEWORD(dvdPreRecorded->WriteStrategyCode[1], dvdPreRecorded->WriteStrategyCode[0])),
		dvdPreRecorded->FieldID_3,
		dvdPreRecorded->ManufacturerId_3[5], dvdPreRecorded->ManufacturerId_3[4],
		dvdPreRecorded->ManufacturerId_3[3], dvdPreRecorded->ManufacturerId_3[2],
		dvdPreRecorded->ManufacturerId_3[1], dvdPreRecorded->ManufacturerId_3[0],
		dvdPreRecorded->FieldID_4,
		dvdPreRecorded->ManufacturerId_4[5], dvdPreRecorded->ManufacturerId_4[4],
		dvdPreRecorded->ManufacturerId_4[3], dvdPreRecorded->ManufacturerId_4[2],
		dvdPreRecorded->ManufacturerId_4[1], dvdPreRecorded->ManufacturerId_4[0],
		dvdPreRecorded->FieldID_5,
		dvdPreRecorded->ManufacturerId_5[5], dvdPreRecorded->ManufacturerId_5[4],
		dvdPreRecorded->ManufacturerId_5[3], dvdPreRecorded->ManufacturerId_5[2],
		dvdPreRecorded->ManufacturerId_5[1], dvdPreRecorded->ManufacturerId_5[0]);
}

VOID OutputMmcDvdUniqueDiscIdentifer(
	PDVD_UNIQUE_DISC_IDENTIFIER dvdUnique
	)
{
	OutputDiscLog(
		_T("\tDVD_UNIQUE_DISC_IDENTIFIER\n")
		_T("\t\tRandomNumber: %d\n")
		_T("\t\t     YMD HMS: %04d-%02d-%02d %02d:%02d:%02d\n"),
		MAKEWORD(dvdUnique->RandomNumber[1], dvdUnique->RandomNumber[0]),
		MAKELONG(MAKEWORD(dvdUnique->Year[3], dvdUnique->Year[2]),
		MAKEWORD(dvdUnique->Year[1], dvdUnique->Year[0])),
		MAKEWORD(dvdUnique->Month[1], dvdUnique->Month[0]),
		MAKEWORD(dvdUnique->Day[1], dvdUnique->Day[0]),
		MAKEWORD(dvdUnique->Hour[1], dvdUnique->Hour[0]),
		MAKEWORD(dvdUnique->Minute[1], dvdUnique->Minute[0]),
		MAKEWORD(dvdUnique->Second[1], dvdUnique->Second[0]));
}

VOID OutputMmcAdipInformation(
	LPBYTE lpStructure,
	LPWORD lpStructureLength,
	size_t uiFormatCode
	)
{
	OutputDiscLog(
		_T("\tADIP information: "));
	for (DWORD k = 0;
		k < lpStructureLength[uiFormatCode] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
		OutputDiscLog(_T("%02x"), lpStructure[k]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputMmcDvdDualLayerRecordingInformation(
	PDVD_DUAL_LAYER_RECORDING_INFORMATION dvdDualLayer
	)
{
	OutputDiscLog(
		_T("\tDvdDualLayerRecordingInformation\n")
		_T("\t\tLayer0SectorsImmutable: %s\n")
		_T("\t\t         Layer0Sectors: %d\n"),
		BOOLEAN_TO_STRING_YES_NO(dvdDualLayer->Layer0SectorsImmutable),
		MAKELONG(MAKEWORD(dvdDualLayer->Layer0Sectors[3], dvdDualLayer->Layer0Sectors[2]),
		MAKEWORD(dvdDualLayer->Layer0Sectors[1], dvdDualLayer->Layer0Sectors[0])));
}
VOID OutputMmcDvdDualLayerMiddleZone(
	PDVD_DUAL_LAYER_MIDDLE_ZONE_START_ADDRESS dvdDualLayerMiddle
	)
{
	OutputDiscLog(
		_T("\tDvdDualLayerMiddleZoneStartAddress\n")
		_T("\t\t                   InitStatus: %s\n")
		_T("\t\tShiftedMiddleAreaStartAddress: %d\n"),
		BOOLEAN_TO_STRING_YES_NO(dvdDualLayerMiddle->InitStatus),
		MAKELONG(MAKEWORD(dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[3], 
			dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[2]),
			MAKEWORD(dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[1], 
			dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[0])));
}

VOID OutputMmcDvdDualLayerJumpInterval(
	PDVD_DUAL_LAYER_JUMP_INTERVAL_SIZE dvdDualLayerJump
	)
{
	OutputDiscLog(
		_T("\tDvdDualLayerJumpIntervalSize\n")
		_T("\t\tJumpIntervalSize: %d\n"),
		MAKELONG(MAKEWORD(dvdDualLayerJump->JumpIntervalSize[3],
		dvdDualLayerJump->JumpIntervalSize[2]),
		MAKEWORD(dvdDualLayerJump->JumpIntervalSize[1],
		dvdDualLayerJump->JumpIntervalSize[0])));
}

VOID OutputMmcDvdDualLayerManualLayerJump(
	PDVD_DUAL_LAYER_MANUAL_LAYER_JUMP dvdDualLayerMan
	)
{
	OutputDiscLog(
		_T("\tDvdDualLayerManualLayerJump\n")
		_T("\t\tManualJumpLayerAddress: %d\n"),
		MAKELONG(MAKEWORD(dvdDualLayerMan->ManualJumpLayerAddress[3],
		dvdDualLayerMan->ManualJumpLayerAddress[2]),
		MAKEWORD(dvdDualLayerMan->ManualJumpLayerAddress[1],
		dvdDualLayerMan->ManualJumpLayerAddress[0])));
}

VOID OutputMmcDvdDualLayerRemapping(
	PDVD_DUAL_LAYER_REMAPPING_INFORMATION dvdDualLayerRemapping
	)
{
	OutputDiscLog(
		_T("\tDvdDualLayerRemappingInformation\n")
		_T("\t\tManualJumpLayerAddress: %d\n"),
		MAKELONG(MAKEWORD(dvdDualLayerRemapping->RemappingAddress[3],
		dvdDualLayerRemapping->RemappingAddress[2]),
		MAKEWORD(dvdDualLayerRemapping->RemappingAddress[1],
		dvdDualLayerRemapping->RemappingAddress[0])));
}

VOID OutputMmcDvdDiscControlBlockHeader(
	PDVD_DISC_CONTROL_BLOCK_HEADER dvdDiscCtrlBlk
	)
{
	OutputDiscLog(
		_T("\tDVD_DISC_CONTROL_BLOCK_HEADER\n")
		_T("\t\tContentDescriptor: %d\n")
		_T("\t\t           AsByte: %d\n")
		_T("\t\t         VendorId: "),
		MAKELONG(MAKEWORD(dvdDiscCtrlBlk->ContentDescriptor[3],
		dvdDiscCtrlBlk->ContentDescriptor[2]),
		MAKEWORD(dvdDiscCtrlBlk->ContentDescriptor[1],
		dvdDiscCtrlBlk->ContentDescriptor[0])),
		MAKELONG(MAKEWORD(dvdDiscCtrlBlk->ProhibitedActions.AsByte[3],
		dvdDiscCtrlBlk->ProhibitedActions.AsByte[2]),
		MAKEWORD(dvdDiscCtrlBlk->ProhibitedActions.AsByte[1],
		dvdDiscCtrlBlk->ProhibitedActions.AsByte[0])));
	for (DWORD k = 0; k < sizeof(dvdDiscCtrlBlk->VendorId); k++) {
		OutputDiscLog(_T("%c"), dvdDiscCtrlBlk->VendorId[k]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputMmcDvdDiscControlBlockWriteInhibit(
	PDVD_DISC_CONTROL_BLOCK_WRITE_INHIBIT dvdDiscCtrlBlkWrite
	)
{
	OutputDiscLog(
		_T("\tDVD_DISC_CONTROL_BLOCK_WRITE_INHIBIT\n")
		_T("\t\t      UpdateCount: %d\n")
		_T("\t\t           AsByte: %d\n")
		_T("\t\t   UpdatePassword: "),
		MAKELONG(MAKEWORD(dvdDiscCtrlBlkWrite->UpdateCount[3], dvdDiscCtrlBlkWrite->UpdateCount[2]),
		MAKEWORD(dvdDiscCtrlBlkWrite->UpdateCount[1], dvdDiscCtrlBlkWrite->UpdateCount[0])),
		MAKELONG(MAKEWORD(dvdDiscCtrlBlkWrite->WriteProtectActions.AsByte[3], 
		dvdDiscCtrlBlkWrite->WriteProtectActions.AsByte[2]),
		MAKEWORD(dvdDiscCtrlBlkWrite->WriteProtectActions.AsByte[1], 
		dvdDiscCtrlBlkWrite->WriteProtectActions.AsByte[0])));
	for (DWORD k = 0; k < sizeof(dvdDiscCtrlBlkWrite->UpdatePassword); k++) {
		OutputDiscLog(_T("%c"), dvdDiscCtrlBlkWrite->UpdatePassword[k]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputMmcDvdDiscControlBlockSession(
	PDVD_DISC_CONTROL_BLOCK_SESSION dvdDiscCtrlBlkSession
	)
{
	OutputDiscLog(
		_T("\tDVD_DISC_CONTROL_BLOCK_SESSION\n")
		_T("\t\tSessionNumber: %d\n")
		_T("\t\t       DiscID: \n"),
		MAKEWORD(dvdDiscCtrlBlkSession->SessionNumber[1], dvdDiscCtrlBlkSession->SessionNumber[0]));
	for (DWORD k = 0; k < sizeof(dvdDiscCtrlBlkSession->DiscID); k++) {
		OutputDiscLog(_T("%c"), dvdDiscCtrlBlkSession->DiscID[k]);
	}
	OutputDiscLog(_T("\n"));

	for (DWORD j = 0; j < sizeof(dvdDiscCtrlBlkSession->SessionItem); j++) {
		OutputDiscLog(
			_T("\t\t  SessionItem: %d\n")
			_T("\t\t\t     AsByte: "), j);
		for (DWORD k = 0; k < sizeof(dvdDiscCtrlBlkSession->SessionItem[j].AsByte); k++) {
			OutputDiscLog(_T("%c"), dvdDiscCtrlBlkSession->SessionItem[j].AsByte[k]);
		}
		OutputDiscLog(_T("\n"));
	}
}

VOID OutputMmcDvdDiscControlBlockList(
	PDVD_DISC_CONTROL_BLOCK_LIST dvdDiscCtrlBlkList,
	LPWORD lpStructureLength,
	size_t uiFormatCode
	)
{
	OutputDiscLog(
		_T("\tDVD_DISC_CONTROL_BLOCK_LIST\n")
		_T("\t\tReadabldDCBs: %s\n")
		_T("\t\tWritableDCBs: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(dvdDiscCtrlBlkList->ReadabldDCBs),
		BOOLEAN_TO_STRING_YES_NO(dvdDiscCtrlBlkList->WritableDCBs));
	OutputDiscLog(
		_T("\t\tDVD_DISC_CONTROL_BLOCK_LIST_DCB: "));
	for (DWORD k = 0;
		k < lpStructureLength[uiFormatCode] - sizeof(DVD_DISC_CONTROL_BLOCK_LIST); k++) {
		OutputDiscLog(_T("%d"),
			MAKELONG(MAKEWORD(dvdDiscCtrlBlkList->Dcbs[k].DcbIdentifier[3], dvdDiscCtrlBlkList->Dcbs[k].DcbIdentifier[2]),
			MAKEWORD(dvdDiscCtrlBlkList->Dcbs[k].DcbIdentifier[1], dvdDiscCtrlBlkList->Dcbs[k].DcbIdentifier[0])));
	}
	OutputDiscLog(_T("\n"));

}

VOID OutputMmcDvdMtaEccBlock(
	LPBYTE lpStructure,
	LPWORD lpStructureLength,
	size_t uiFormatCode
	)
{
	OutputDiscLog(
		_T("\tMTA ECC Block: "));
	for (DWORD k = 0;
		k < lpStructureLength[uiFormatCode] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
		OutputDiscLog(_T("%02x"), lpStructure[k]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputMmcDvdWriteProtectionStatus(
	PDVD_WRITE_PROTECTION_STATUS dvdWrite
	)
{
	OutputDiscLog(
		_T("\tDVD_WRITE_PROTECTION_STATUS\n")
		_T("\t\tSoftwareWriteProtectUntilPowerdown: %s\n")
		_T("\t\t       MediaPersistentWriteProtect: %s\n")
		_T("\t\t             CartridgeWriteProtect: %s\n")
		_T("\t\t         MediaSpecificWriteProtect: %s\n"),
		BOOLEAN_TO_STRING_YES_NO(dvdWrite->SoftwareWriteProtectUntilPowerdown),
		BOOLEAN_TO_STRING_YES_NO(dvdWrite->MediaPersistentWriteProtect),
		BOOLEAN_TO_STRING_YES_NO(dvdWrite->CartridgeWriteProtect),
		BOOLEAN_TO_STRING_YES_NO(dvdWrite->MediaSpecificWriteProtect));
}

VOID OutputMmcDvdAacsVolumeIdentifier(
	LPBYTE lpStructure,
	LPWORD lpStructureLength,
	size_t uiFormatCode
	)
{
	OutputDiscLog(
		_T("\tAacs Volume Identifier: "));
	for (DWORD k = 0;
		k < lpStructureLength[uiFormatCode] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
		OutputDiscLog(_T("%02x"), lpStructure[k]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputMmcDvdPreRecordedAacsMediaSerialNumber(
	LPBYTE lpStructure,
	LPWORD lpStructureLength,
	size_t uiFormatCode
	)
{
	OutputDiscLog(
		_T("\tPreRecorded Aacs Media Serial Number: "));
	for (DWORD k = 0;
		k < lpStructureLength[uiFormatCode] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
		OutputDiscLog(_T("%02x"), lpStructure[k]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputMmcDvdAacsMediaIdentifier(
	LPBYTE lpStructure,
	LPWORD lpStructureLength,
	size_t uiFormatCode
	)
{
	OutputDiscLog(
		_T("\tAacs Media Identifier: "));
	for (DWORD k = 0;
		k < lpStructureLength[uiFormatCode] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
		OutputDiscLog(_T("%02x"), lpStructure[k]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputMmcDvdAacsMediaKeyBlock(
	LPBYTE lpStructure,
	LPWORD lpStructureLength,
	size_t uiFormatCode
	)
{
	OutputDiscLog(
		_T("\tAacs Media Key Block: "));
	for (DWORD k = 0;
		k < lpStructureLength[uiFormatCode] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
		OutputDiscLog(_T("%02x"), lpStructure[k]);
	}
	OutputDiscLog(_T("\n"));
}

VOID OutputMmcDvdListOfRecognizedFormatLayers(
	PDVD_LIST_OF_RECOGNIZED_FORMAT_LAYERS_TYPE_CODE dvdListOf
	)
{
	OutputDiscLog(
		_T("\t\tNumberOfRecognizedFormatLayers: %d\n")
		_T("\t\t             OnlineFormatlayer: %d\n")
		_T("\t\t            DefaultFormatLayer: %d\n"),
		dvdListOf->NumberOfRecognizedFormatLayers,
		dvdListOf->OnlineFormatlayer,
		dvdListOf->DefaultFormatLayer);
}

VOID OutputMmcDVDStructureFormat(
	PDISC_DATA pDiscData,
	LPBYTE lpStructure,
	LPWORD lpStructureLength,
	LPBYTE lpLayerNum,
	LPBYTE lpFormat, 
	size_t uiFormatCode,
	INT nNum
	)
{
	switch (lpFormat[uiFormatCode]) {
	case DvdPhysicalDescriptor:
	case 0x10:
		OutputMmcDvdLayerDescriptor(pDiscData, (PDVD_FULL_LAYER_DESCRIPTOR)(lpStructure + 4), lpLayerNum, nNum);
		break;
	case DvdCopyrightDescriptor:
		OutputMmcDvdCopyrightDescriptor((PDVD_COPYRIGHT_DESCRIPTOR)(lpStructure + 4));
		break;
	case DvdDiskKeyDescriptor:
		OutputMmcDvdDiskKeyDescriptor((PDVD_DISK_KEY_DESCRIPTOR)(lpStructure + 4));
		break;
	case DvdBCADescriptor:
		OutputMmcDvdBCADescriptor((PDVD_BCA_DESCRIPTOR)(lpStructure + 4), lpStructureLength, uiFormatCode);
		break;
	case DvdManufacturerDescriptor:
		OutputMmcDvdManufacturerDescriptor((PDVD_MANUFACTURER_DESCRIPTOR)(lpStructure + 4));
		break;
	case 0x06:
		OutputMmcDvdMediaId(lpStructure + 4, lpStructureLength, uiFormatCode);
		break;
	case 0x07:
		OutputMmcDvdMediaKeyBlock(lpStructure + 4, lpStructureLength, uiFormatCode);
		break;
	case 0x08:
		OutputMmcDvdRamDds(lpStructure + 4, lpStructureLength, uiFormatCode);
		break;
	case 0x09:
		OutputMmcDvdRamMediumStatus((PDVD_RAM_MEDIUM_STATUS)(lpStructure + 4));
		break;
	case 0x0a:
		OutputMmcDvdRamSpareArea((PDVD_RAM_SPARE_AREA_INFORMATION)(lpStructure + 4));
		break;
	case 0x0b:
		OutputMmcDvdRamRecordingType((PDVD_RAM_RECORDING_TYPE)(lpStructure + 4));
		break;
	case 0x0c:
		OutputMmcDvdRmdLastBorderOut(lpStructure + 4, lpStructureLength, uiFormatCode);
		break;
	case 0x0d:
		OutputMmcDvdRecordingManagementAreaData((PDVD_RECORDING_MANAGEMENT_AREA_DATA)(lpStructure + 4), lpStructureLength, uiFormatCode);
		break;
	case 0x0e:
		OutputMmcDvdPreRecordedInformation((PDVD_PRERECORDED_INFORMATION)(lpStructure + 4));
		break;
	case 0x0f:
		OutputMmcDvdUniqueDiscIdentifer((PDVD_UNIQUE_DISC_IDENTIFIER)(lpStructure + 4));
		break;
	case 0x11:
		OutputMmcAdipInformation(lpStructure + 4, lpStructureLength, uiFormatCode);
		break;
	// formats 0x12, 0x15 are is unstructured in public spec
	// formats 0x13, 0x14, 0x16 through 0x18 are not yet defined
	// formats 0x19, 0x1A are HD DVD-R
	// formats 0x1B through 0x1F are not yet defined
	case 0x20:
		OutputMmcDvdDualLayerRecordingInformation((PDVD_DUAL_LAYER_RECORDING_INFORMATION)(lpStructure + 4));
		break;
	case 0x21:
		OutputMmcDvdDualLayerMiddleZone((PDVD_DUAL_LAYER_MIDDLE_ZONE_START_ADDRESS)(lpStructure + 4));
		break;
	case 0x22:
		OutputMmcDvdDualLayerJumpInterval((PDVD_DUAL_LAYER_JUMP_INTERVAL_SIZE)(lpStructure + 4));
		break;
	case 0x23:
		OutputMmcDvdDualLayerManualLayerJump((PDVD_DUAL_LAYER_MANUAL_LAYER_JUMP)(lpStructure + 4));
		break;
	case 0x24:
		OutputMmcDvdDualLayerRemapping((PDVD_DUAL_LAYER_REMAPPING_INFORMATION)(lpStructure + 4));
		break;
	// formats 0x25 through 0x2F are not yet defined
	case 0x30: 
	{
		OutputMmcDvdDiscControlBlockHeader((PDVD_DISC_CONTROL_BLOCK_HEADER)(lpStructure + 4));
		DWORD len = lpStructureLength[uiFormatCode] - sizeof(DVD_DESCRIPTOR_HEADER);
		if (len == sizeof(DVD_DISC_CONTROL_BLOCK_WRITE_INHIBIT)) {
			OutputMmcDvdDiscControlBlockWriteInhibit((PDVD_DISC_CONTROL_BLOCK_WRITE_INHIBIT)(lpStructure + 4));
		}
		else if (len == sizeof(DVD_DISC_CONTROL_BLOCK_SESSION)) {
			OutputMmcDvdDiscControlBlockSession((PDVD_DISC_CONTROL_BLOCK_SESSION)(lpStructure + 4));
		}
		else if (len == sizeof(DVD_DISC_CONTROL_BLOCK_LIST)) {
			OutputMmcDvdDiscControlBlockList((PDVD_DISC_CONTROL_BLOCK_LIST)(lpStructure + 4), lpStructureLength, uiFormatCode);
		}
		break;
	}
	case 0x31:
		OutputMmcDvdMtaEccBlock(lpStructure + 4, lpStructureLength, uiFormatCode);
		break;
	// formats 0x32 through 0xBF are not yet defined
	case 0xc0:
		OutputMmcDvdWriteProtectionStatus((PDVD_WRITE_PROTECTION_STATUS)(lpStructure + 4));
		break;
	// formats 0xC1 through 0x7F are not yet defined
	case 0x80:
		OutputMmcDvdAacsVolumeIdentifier(lpStructure + 4, lpStructureLength, uiFormatCode);
		break;
	case 0x81:
		OutputMmcDvdPreRecordedAacsMediaSerialNumber(lpStructure + 4, lpStructureLength, uiFormatCode);
		break;
	case 0x82:
		OutputMmcDvdAacsMediaIdentifier(lpStructure + 4, lpStructureLength, uiFormatCode);
		break;
	case 0x83:
		OutputMmcDvdAacsMediaKeyBlock(lpStructure + 4, lpStructureLength, uiFormatCode);
		break;
	// formats 0x84 through 0x8F are not yet defined
	case 0x90:
		OutputMmcDvdListOfRecognizedFormatLayers((PDVD_LIST_OF_RECOGNIZED_FORMAT_LAYERS_TYPE_CODE)(lpStructure + 4));
		break;
	// formats 0x91 through 0xFE are not yet defined
	default:
		OutputDiscLog(_T("\tUnknown: %02x\n"), lpFormat[uiFormatCode]);
		break;
	}
}

VOID OutputMmcDVDCopyrightManagementInformation(
	PDVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR dvdCopyright,
	INT nLBA,
	INT nIdx	
	)
{
	OutputDiscLog(_T("\t\tLBA %7u, "), nLBA + nIdx);
	if ((dvdCopyright->CPR_MAI & 0x80) == 0x80) {
		OutputDiscLog(_T("CPM exists"));
		if ((dvdCopyright->CPR_MAI & 0x40) == 0x40) {
			switch (dvdCopyright->CPR_MAI & 0x0f) {
			case 0:
				OutputDiscLog(_T(", the sector is scrambled by CSS"));
				break;
			case 0x01:
				OutputDiscLog(_T(", the sector is encrypted by CPPM"));
				break;
			default:
				OutputDiscLog(_T(", reserved"));
			}
		}
		else {
			OutputDiscLog(_T(", CSS or CPPM don't exists in this sector"));
		}
		switch (dvdCopyright->CPR_MAI & 0x30) {
		case 0:
			OutputDiscLog(_T(", copying is permitted without restriction\n"));
			break;
		case 0x10:
			OutputDiscLog(_T(", reserved\n"));
			break;
		case 0x20:
			OutputDiscLog(_T(", one generation of copies may be made\n"));
			break;
		case 0x30:
			OutputDiscLog(_T(", no copying is permitted\n"));
			break;
		default:
			OutputDiscLog(_T("\n"));
		}
	}
	else {
		OutputDiscLog(_T("CPM don't exists\n"));
	}
}
