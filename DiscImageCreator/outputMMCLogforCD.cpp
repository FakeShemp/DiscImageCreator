/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"
#include "struct.h"
#include "convert.h"
#include "output.h"
#include "outputMMCLogforCD.h"

VOID OutputFsBootRecord(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	CHAR str[2][32] = { "0" };
	strncpy(str[0], (PCHAR)&lpBuf[nIdx + 7], sizeof(str[0]));
	strncpy(str[1], (PCHAR)&lpBuf[nIdx + 39], sizeof(str[1]));
	OutputDiscLogA(
		"\t                       Boot System Identifier: %s\n"
		"\t                              Boot Identifier: %s\n"
		"\t                              Boot System Use: ",
		str[0],
		str[1]);
	for (INT i = 71; i <= 2047; i++) {
		OutputDiscLogA("%x", lpBuf[nIdx + i]);
	}
	OutputDiscLogA("\n");
}

VOID OutputFsPrimaryVolumeDescriptorForTime(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	CHAR year[4][4] = { "0" };
	CHAR month[4][2] = { "0" };
	CHAR day[4][2] = { "0" };
	CHAR hour[4][2] = { "0" };
	CHAR time[4][2] = { "0" };
	CHAR second[4][2] = { "0" };
	CHAR milisecond[4][2] = { "0" };
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
	OutputDiscLogA(
		"\t                Volume Creation Date and Time: %.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.2s +%u\n"
		"\t            Volume Modification Date and Time: %.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.2s +%u\n"
		"\t              Volume Expiration Date and Time: %.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.2s +%u\n"
		"\t               Volume Effective Date and Time: %.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.2s +%u\n"
		"\t                       File Structure Version: %u\n"
		"\t                              Application Use: ",
		year[0], month[0], day[0], hour[0], time[0], second[0], milisecond[0], lpBuf[nIdx + 829],
		year[1], month[1], day[1], hour[1], time[1], second[1], milisecond[1], lpBuf[nIdx + 846],
		year[2], month[2], day[2], hour[2], time[2], second[2], milisecond[2], lpBuf[nIdx + 863],
		year[3], month[3], day[3], hour[3], time[3], second[3], milisecond[3], lpBuf[nIdx + 880],
		lpBuf[nIdx + 881]);
	for (INT i = 883; i <= 1394; i++) {
		OutputDiscLogA("%x", lpBuf[nIdx + i]);
	}
	OutputDiscLogA("\n");
}

VOID OutputFsPrimaryVolumeDescriptorFor1(
	LPBYTE lpBuf,
	CHAR str32[][32],
	INT nIdx
	)
{
	OutputDiscLogA(
		"\t                            System Identifier: %.32s\n"
		"\t                            Volume Identifier: %.32s\n"
		"\t                            Volume Space Size: %u\n",
		str32[0],
		str32[1],
		MAKELONG(MAKEWORD(lpBuf[nIdx + 80], lpBuf[nIdx + 81]),
		MAKEWORD(lpBuf[nIdx + 82], lpBuf[nIdx + 83])));
}

VOID OutputFsPrimaryVolumeDescriptorFor2(
	LPBYTE lpBuf,
	CHAR str128[][128],
	CHAR str37[][37],
	INT nIdx
	)
{
	OutputDiscLogA(
		"\t                              Volume Set Size: %u\n"
		"\t                       Volume Sequence Number: %u\n"
		"\t                           Logical Block Size: %u\n"
		"\t                              Path Table Size: %u\n"
		"\t         Location of Occurrence of Path Table: %u\n"
		"\tLocation of Optional Occurrence of Path Table: %u\n"
		"\tDirectory Record\n"
		"\t\t      Length of Directory Record: %u\n"
		"\t\tExtended Attribute Record Length: %u\n"
		"\t\t              Location of Extent: %u\n"
		"\t\t                     Data Length: %u\n"
		"\t\t         Recording Date and Time: %u-%02u-%02u %02u:%02u:%02u +%02u\n"
		"\t\t                      File Flags: %u\n"
		"\t\t                  File Unit Size: %u\n"
		"\t\t             Interleave Gap Size: %u\n"
		"\t\t          Volume Sequence Number: %u\n"
		"\t\t       Length of File Identifier: %u\n"
		"\t\t                 File Identifier: %u\n"
		"\t                        Volume Set Identifier: %.128s\n"
		"\t                         Publisher Identifier: %.128s\n"
		"\t                     Data Preparer Identifier: %.128s\n"
		"\t                       Application Identifier: %.128s\n"
		"\t                    Copyright File Identifier: %.37s\n"
		"\t                     Abstract File Identifier: %.37s\n"
		"\t                Bibliographic File Identifier: %.37s\n",
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
	CHAR str32[3][32] = { 0 };
	CHAR str128[4][128] = { 0 };
	CHAR str37[3][37] = { 0 };
	strncpy(str32[0], (PCHAR)&lpBuf[nIdx + 8], sizeof(str32[0]));
	strncpy(str32[1], (PCHAR)&lpBuf[nIdx + 40], sizeof(str32[1]));
	strncpy(str128[0], (PCHAR)&lpBuf[nIdx + 190], sizeof(str128[0]));
	strncpy(str128[1], (PCHAR)&lpBuf[nIdx + 318], sizeof(str128[1]));
	strncpy(str128[2], (PCHAR)&lpBuf[nIdx + 446], sizeof(str128[2]));
	strncpy(str128[3], (PCHAR)&lpBuf[nIdx + 574], sizeof(str128[3]));
	strncpy(str37[0], (PCHAR)&lpBuf[nIdx + 702], sizeof(str37[0]));
	strncpy(str37[1], (PCHAR)&lpBuf[nIdx + 739], sizeof(str37[1]));
	strncpy(str37[2], (PCHAR)&lpBuf[nIdx + 776], sizeof(str37[2]));
	OutputFsPrimaryVolumeDescriptorFor1(lpBuf, str32, nIdx);
	if (lpBuf[nIdx + 0] == 2) {
		strncpy(str32[2], (PCHAR)&lpBuf[nIdx + 88], 32);
		OutputDiscLogA(
			"\t                             Escape Sequences: %.32s\n", str32[2]);
	}

	OutputFsPrimaryVolumeDescriptorFor2(lpBuf, str128, str37, nIdx);
	OutputFsPrimaryVolumeDescriptorForTime(lpBuf, nIdx);
}

VOID OutputFsPrimaryVolumeDescriptorForJoliet(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	CHAR str32[3][32] = { 0 };
	CHAR str128[4][128] = { 0 };
	CHAR str37[3][37] = { 0 };
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
	OutputFsPrimaryVolumeDescriptorFor1(lpBuf, str32, nIdx);

	LittleToBig(tmp32[2], (_TCHAR*)&lpBuf[nIdx + 88], 32);
	if (!WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)&tmp32[2], 16, str32[2], sizeof(str32[2]), NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	OutputDiscLogA(
		"\t                             Escape Sequences: %.32s\n", str32[2]);
	OutputFsPrimaryVolumeDescriptorFor2(lpBuf, str128, str37, nIdx);
	OutputFsPrimaryVolumeDescriptorForTime(lpBuf, nIdx);
}

VOID OutputFsVolumePartitionDescriptor(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	CHAR str[2][32] = { 0 };
	strncpy(str[0], (PCHAR)&lpBuf[nIdx + 8], 32);
	strncpy(str[1], (PCHAR)&lpBuf[nIdx + 40], 32);
	OutputDiscLogA(
		"\t          System Identifier: %.32s\n"
		"\tVolume Partition Identifier: %.32s\n"
		"\t  Volume Partition Location: %u\n"
		"\t      Volume Partition Size: %u\n"
		"\t                 System Use: ",
		str[0],
		str[1],
		MAKELONG(MAKEWORD(lpBuf[nIdx + 76], lpBuf[nIdx + 77]),
		MAKEWORD(lpBuf[nIdx + 78], lpBuf[nIdx + 79])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 84], lpBuf[nIdx + 85]),
		MAKEWORD(lpBuf[nIdx + 86], lpBuf[nIdx + 87])));
	for (INT i = 88; i <= 2047; i++) {
		OutputDiscLogA("%x", lpBuf[nIdx + i]);
	}
	OutputDiscLogA("\n");
}

VOID OutputFsVolumeDescriptor(
	LPBYTE lpBuf,
	INT nIdx,
	INT nLBA
	)
{
	// 0 is Boot Record. 
	// 1 is Primary Volume Descriptor. 
	// 2 is Supplementary Volume Descriptor. 
	// 3 is Volume Partition Descriptor. 
	// 4-254 is reserved. 
	// 255 is Volume Descriptor Set Terminator.
	CHAR str[5] = { 0 };
	strncpy(str, (PCHAR)&lpBuf[nIdx + 1], sizeof(str));
	OutputDiscLogA(
		"Volume Descriptor, LBA[%06d, %#07x]\n"
		"\t                       Volume Descriptor Type: %u\n"
		"\t                          Standard Identifier: %.5s\n"
		"\t                    Volume Descriptor Version: %u\n",
		nLBA, nLBA,
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

VOID OutputFsMasterDirectoryBlocks(
	LPBYTE lpBuf,
	INT nIdx,
	INT nLBA
	)
{
	CHAR str[27] = { 0 };
	strncpy(str, (PCHAR)&lpBuf[nIdx + 36], sizeof(str));
	OutputDiscLogA(
		"MasterDirectoryBlocks, LBA[%06d, %#07x]\n"
		"\t                       volume signature: %04x\n"
		"\t       date and time of volume creation: %08x\n"
		"\t     date and time of last modification: %08x\n"
		"\t                      volume attributes: %04x\n"
		"\t      number of files in root directory: %04x\n"
		"\t           first block of volume bitmap: %04x\n"
		"\t        start of next allocation search: %04x\n"
		"\t  number of allocation blocks in volume: %04x\n"
		"\t   size (in bytes) of allocation blocks: %08x\n"
		"\t                     default clump size: %08x\n"
		"\t       first allocation block in volume: %04x\n"
		"\t            next unused catalog node ID: %08x\n"
		"\t     number of unused allocation blocks: %04x\n"
		"\t                            volume name: %.27s\n"
		"\t           date and time of last backup: %08x\n"
		"\t          volume backup sequence number: %04x\n"
		"\t                     volume write count: %08x\n"
		"\t   clump size for extents overflow file: %08x\n"
		"\t            clump size for catalog file: %08x\n"
		"\tnumber of directories in root directory: %04x\n"
		"\t              number of files in volume: %08x\n"
		"\t        number of directories in volume: %08x\n"
		"\t         information used by the Finder: %08x\n"
		"\t         information used by the Finder: %08x\n"
		"\t         information used by the Finder: %08x\n"
		"\t         information used by the Finder: %08x\n"
		"\t         information used by the Finder: %08x\n"
		"\t         information used by the Finder: %08x\n"
		"\t         information used by the Finder: %08x\n"
		"\t         information used by the Finder: %08x\n"
		"\t       size (in blocks) of volume cache: %04x\n"
		"\tsize (in blocks) of volume bitmap cache: %04x\n"
		"\tsize (in blocks) of common volume cache: %04x\n"
		"\t          size of extents overflow file: %08x\n"
		"\textent record for extents overflow file: %08x%08x%08x\n"
		"\t                   size of catalog file: %08x\n"
		"\t         extent record for catalog file: %08x%08x%08x\n",
		nLBA, nLBA,
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

VOID OutputFs3doStructure(
	LPBYTE lpBuf,
	INT nLBA
	)
{
	OutputDiscLogA(
		"3DO Header, LBA[%06d, %#07x]\n"
		"\t              Unknown Value: %08x\n"
		"\t              Unknown Value: %08x\n"
		"\t              Unknown Value: %08x\n"
		"\t              Unknown Value: %08x\n"
		"\t              Unknown Value: %08x\n"
		"\t              Unknown Value: %08x\n"
		"\t              Unknown Value: %08x\n"
		"\tNumber of Directory Entries: %08x\n"
		"\t Position of Root Directory: %08x\n",
		nLBA, nLBA,
		MAKELONG(MAKEWORD(lpBuf[91], lpBuf[90]), MAKEWORD(lpBuf[89], lpBuf[88])),
		MAKELONG(MAKEWORD(lpBuf[95], lpBuf[94]), MAKEWORD(lpBuf[93], lpBuf[92])),
		MAKELONG(MAKEWORD(lpBuf[99], lpBuf[98]), MAKEWORD(lpBuf[97], lpBuf[96])),
		MAKELONG(MAKEWORD(lpBuf[103], lpBuf[102]), MAKEWORD(lpBuf[101], lpBuf[100])),
		MAKELONG(MAKEWORD(lpBuf[107], lpBuf[106]), MAKEWORD(lpBuf[105], lpBuf[104])),
		MAKELONG(MAKEWORD(lpBuf[111], lpBuf[110]), MAKEWORD(lpBuf[109], lpBuf[108])),
		MAKELONG(MAKEWORD(lpBuf[115], lpBuf[114]), MAKEWORD(lpBuf[113], lpBuf[112])),
		MAKELONG(MAKEWORD(lpBuf[119], lpBuf[118]), MAKEWORD(lpBuf[117], lpBuf[116])),
		MAKELONG(MAKEWORD(lpBuf[123], lpBuf[122]), MAKEWORD(lpBuf[121], lpBuf[120]))
		);
}

VOID OutputFsPceStuff(
	LPBYTE lpBuf,
	INT nLBA
	)
{
	OutputDiscLogA(
		"PCE Warning msg, all stuff, LBA[%06d, %#07x]\n",
		nLBA, nLBA);
	CHAR str[38] = { 0 };
	size_t len = 0;
	for (size_t idx = 0; idx < 805; idx += len) {
		PCHAR ptr = (PCHAR)&lpBuf[16 + idx];
		if (ptr[0] == -1) {
			len = 1;
			continue;
		}
		len = strlen(ptr) + 1;
		strncpy(str, ptr, len);
		OutputDiscLogA("\t%s\n", str);
	}
}

VOID OutputFsPceBootSector(
	LPBYTE lpBuf,
	INT nLBA
	)
{
	CHAR str[24] = { 0 };
	strncpy(str, (PCHAR)&lpBuf[48], sizeof(str));
	CHAR str2[50] = { 0 };
	strncpy(str2, (PCHAR)&lpBuf[72], sizeof(str2));
	CHAR str3[17] = { 0 };
	strncpy(str3, (PCHAR)&lpBuf[122], sizeof(str3) - 1);
	CHAR str4[7] = { 0 };
	strncpy(str4, (PCHAR)&lpBuf[138], sizeof(str4) - 1);
	OutputDiscLogA(
		"PCE Boot Sector, LBA[%06d, %#07x]\n"
		"\t       load start record no.of CD: %02x:%02x:%02x\n"
		"\t          load block length of CD: %02x\n"
		"\t             program load address: %04x\n"
		"\t          program execute address: %04x\n"
		"\t     ipl set mpr2 (+ max mapping): %02x\n"
		"\t     ipl set mpr3 (+ max mapping): %02x\n"
		"\t     ipl set mpr4 (+ max mapping): %02x\n"
		"\t     ipl set mpr5 (+ max mapping): %02x\n"
		"\t     ipl set mpr6 (+ max mapping): %02x\n"
		"\t                     opening mode: %02x\n"
		"\t  opening graphic data record no.: %02x:%02x:%02x\n"
		"\t      opening graphic data length: %02x\n"
		"\topening graphic data read address: %04x\n"
		"\t    opening ADPCM data record no.: %02x:%02x:%02x\n"
		"\t        opening ADPCM data length: %02x\n"
		"\t      opening ADPCM sampling rate: %02x\n"
		"\t                         reserved: %02x, %02x, %02x, %02x, %02x, %02x, %02x\n"
		"\t                           system: %s\n"
		"\t                        copyright: %s\n"
		"\t                     program name: %s\n"
		"\t                         reserved: %s\n",
		nLBA, nLBA,
		lpBuf[16], lpBuf[17], lpBuf[18],
		lpBuf[19],
		MAKEWORD(lpBuf[21], lpBuf[20]),
		MAKEWORD(lpBuf[23], lpBuf[22]),
		lpBuf[24], lpBuf[25], lpBuf[26], lpBuf[27], lpBuf[28],
		lpBuf[29],
		lpBuf[30], lpBuf[31], lpBuf[32],
		lpBuf[33],
		MAKEWORD(lpBuf[35], lpBuf[34]),
		lpBuf[36], lpBuf[37], lpBuf[38],
		lpBuf[39],
		lpBuf[40],
		lpBuf[41], lpBuf[42], lpBuf[43], lpBuf[44], lpBuf[45], lpBuf[46], lpBuf[47],
		str, str2, str3, str4
		);
}

VOID OutputMmcTocWithPregap(
	PDISC_DATA pDiscData
	)
{
	OutputDiscLogA("TOC with pregap\n");
	for (UINT r = 0; r < pDiscData->SCSI.toc.LastTrack; r++) {
		OutputDiscLogA(
			"\tTrack %2u, Ctl %u, Mode %u", r + 1,
			pDiscData->SUB_CHANNEL.lpCtlList[r], pDiscData->SUB_CHANNEL.lpModeList[r]);
		for (UINT k = 0; k < MAXIMUM_NUMBER_INDEXES; k++) {
			if (pDiscData->SUB_CHANNEL.lpFirstLBAListOnSub[r][k] != -1) {
				OutputDiscLogA(", Index%u %6d", k,
					pDiscData->SUB_CHANNEL.lpFirstLBAListOnSub[r][k]);
			}
			else if (k == 0) {
				OutputDiscLogA(",              ");
			}
		}
		OutputDiscLogA("\n");
	}
}

VOID OutputMmcCDOffset(
	PEXT_ARG pExtArg,
	PDISC_DATA pDiscData,
	BOOL bGetDrive,
	INT nDriveSampleOffset,
	INT nDriveOffset
	)
{
	OutputDiscLogA("Offset");
	if (bGetDrive) {
		OutputDiscLogA(
			"(Drive offset data referes to http://www.accuraterip.com)");
	}
	if (pExtArg->bAdd && pDiscData->SCSI.bAudioOnly) {
		pDiscData->MAIN_CHANNEL.nCombinedOffset += pExtArg->nAudioCDOffsetNum * 4;
		OutputDiscLogA(
			"\n"
			"\t       Combined Offset(Byte) %6d, (Samples) %5d\n"
			"\t-         Drive Offset(Byte) %6d, (Samples) %5d\n"
			"\t----------------------------------------------------\n"
			"\t User Specified Offset(Byte) %6d, (Samples) %5d\n",
			pDiscData->MAIN_CHANNEL.nCombinedOffset, pDiscData->MAIN_CHANNEL.nCombinedOffset / 4,
			nDriveOffset, nDriveSampleOffset,
			pDiscData->MAIN_CHANNEL.nCombinedOffset - nDriveOffset,
			(pDiscData->MAIN_CHANNEL.nCombinedOffset - nDriveOffset) / 4);
	}
	else {
		OutputDiscLogA(
			"\n"
			"\t Combined Offset(Byte) %6d, (Samples) %5d\n"
			"\t-   Drive Offset(Byte) %6d, (Samples) %5d\n"
			"\t----------------------------------------------\n"
			"\t       CD Offset(Byte) %6d, (Samples) %5d\n",
			pDiscData->MAIN_CHANNEL.nCombinedOffset, pDiscData->MAIN_CHANNEL.nCombinedOffset / 4,
			nDriveOffset, nDriveSampleOffset,
			pDiscData->MAIN_CHANNEL.nCombinedOffset - nDriveOffset,
			(pDiscData->MAIN_CHANNEL.nCombinedOffset - nDriveOffset) / 4);
	}
}

VOID OutputMmcCDC2Error296(
	LOG_TYPE type,
	LPBYTE lpBuf,
	INT nLBA
	)
{
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(type);
#endif
	OutputLogA(type,
		"c2 error LBA %d\n"
		"\t         +0 +1 +2 +3 +4 +5 +6 +7\n",
		nLBA);

	for (INT i = 0; i < CD_RAW_READ_C2_SIZE; i += 8) {
		OutputLogA(type,
			"\t%3x(%3d) %02x %02x %02x %02x %02x %02x %02x %02x\n",
			i, i, lpBuf[i], lpBuf[i + 1], lpBuf[i + 2], lpBuf[i + 3],
			lpBuf[i + 4], lpBuf[i + 5], lpBuf[i + 6], lpBuf[i + 7]);
	}
}

VOID OutputMmcCDMain2352(
	LPBYTE lpBuf,
	INT nLBA
	)
{
	OutputDiscLogA(
		"Main Channel LBA %d\n"
		"\t          +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n", nLBA);

	for (INT i = 0; i < CD_RAW_SECTOR_SIZE; i += 16) {
		OutputDiscLogA(
			"\t%3x(%4d) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
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
	OutputDiscLogA(
		"Sub Channel LBA %d\n"
		"\t  +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B\n", nLBA);

	for (INT i = 0, ch = 0x50; i < CD_RAW_READ_SUBCODE_SIZE; i += 12, ch++) {
		OutputDiscLogA(
			"\t%c %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			ch, lpBuf[i], lpBuf[i + 1], lpBuf[i + 2], lpBuf[i + 3], lpBuf[i + 4], lpBuf[i + 5],
			lpBuf[i + 6], lpBuf[i + 7], lpBuf[i + 8], lpBuf[i + 9], lpBuf[i + 10], lpBuf[i + 11]);
	}
}

VOID OutputMmcCDSub96Raw(
	LOG_TYPE type,
	LPBYTE lpBuf,
	INT nLBA
	)
{
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(type);
#endif
	OutputLogA(type,
		"Sub Channel(Raw) LBA %d\n"
		"\t    +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n", nLBA);

	for (INT i = 0; i < CD_RAW_READ_SUBCODE_SIZE; i += 16) {
		OutputLogA(type,
			"\t%3X %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
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
	INT byTrackNum,
	FILE* fpParse
	)
{
	CONST INT BufSize = 256;
	_TCHAR str[BufSize] = { 0 };
	// Ctl
	switch ((lpSubcode[12] >> 4) & 0x0F) {
	case 0:
		_sntprintf(str, BufSize,
			_T("LBA[%06d, %#07x], Audio, 2ch, Copy NG, Pre-emphasis No, "), nLBA, nLBA);
		break;
	case AUDIO_WITH_PREEMPHASIS:
		_sntprintf(str, BufSize,
			_T("LBA[%06d, %#07x], Audio, 2ch, Copy NG, Pre-emphasis Yes, "), nLBA, nLBA);
		break;
	case DIGITAL_COPY_PERMITTED:
		_sntprintf(str, BufSize,
			_T("LBA[%06d, %#07x], Audio, 2ch, Copy OK, Pre-emphasis No, "), nLBA, nLBA);
		break;
	case DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
		_sntprintf(str, BufSize,
			_T("LBA[%06d, %#07x], Audio, 2ch, Copy OK, Pre-emphasis Yes, "), nLBA, nLBA);
		break;
	case AUDIO_DATA_TRACK:
		_sntprintf(str, BufSize,
			_T("LBA[%06d, %#07x],  Data,      Copy NG,                  "), nLBA, nLBA);
		break;
	case AUDIO_DATA_TRACK | DIGITAL_COPY_PERMITTED:
		_sntprintf(str, BufSize,
			_T("LBA[%06d, %#07x],  Data,      Copy OK,                  "), nLBA, nLBA);
		break;
	case TWO_FOUR_CHANNEL_AUDIO:
		_sntprintf(str, BufSize,
			_T("LBA[%06d, %#07x], Audio, 4ch, Copy NG, Pre-emphasis No, "), nLBA, nLBA);
		break;
	case TWO_FOUR_CHANNEL_AUDIO | AUDIO_WITH_PREEMPHASIS:
		_sntprintf(str, BufSize,
			_T("LBA[%06d, %#07x], Audio, 4ch, Copy NG, Pre-emphasis Yes, "), nLBA, nLBA);
		break;
	case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED:
		_sntprintf(str, BufSize,
			_T("LBA[%06d, %#07x], Audio, 4ch, Copy OK, Pre-emphasis No, "), nLBA, nLBA);
		break;
	case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
		_sntprintf(str, BufSize,
			_T("LBA[%06d, %#07x], Audio, 4ch, Copy OK, Pre-emphasis Yes, "), nLBA, nLBA);
		break;
	default:
		_sntprintf(str, BufSize,
			_T("LBA[%06d, %#07x], Unknown,                               "), nLBA, nLBA);
		break;
	}

	// ADR
	_TCHAR str2[BufSize] = { 0 };
	switch (lpSubcode[12] & 0x0F) {
	case ADR_ENCODES_CURRENT_POSITION:
		if (nLBA < -150 && lpSubcode[13] == 0) {
			// lead-in area
			if (lpSubcode[14] == 0xA0) {
				_sntprintf(str2, BufSize,
					_T("Point[%02x], RunningTime[%02x:%02x:%02x], TrackNumOf1stTrack[%02x], ProgramAreaFormat[%02x], "),
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17], lpSubcode[19], lpSubcode[20]);
			}
			else if (lpSubcode[14] == 0xA1) {
				_sntprintf(str2, BufSize,
					_T("Point[%02x], RunningTime[%02x:%02x:%02x], TrackNumOfLastTrack[%02x], "),
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17], lpSubcode[19]);
			}
			else if (lpSubcode[14] == 0xA2) {
				_sntprintf(str2, BufSize,
					_T("Point[%02x], RunningTime[%02x:%02x:%02x], StartTimeOfLead-out[%02x:%02x:%02x], "),
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17],
					lpSubcode[19], lpSubcode[20], lpSubcode[21]);
			}
			else {
				_sntprintf(str2, BufSize,
					_T("Point[%02x], RunningTime[%02x:%02x:%02x], TrackStartTime[%02x:%02x:%02x], "),
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17],
					lpSubcode[19], lpSubcode[20], lpSubcode[21]);
			}
		}
		else if (lpSubcode[13] == 0xAA) {
			// lead-out area
			_sntprintf(str2, BufSize,
				_T("LeadOut  , Idx[%02x], RelTime[%02x:%02x:%02x], AbsTime[%02x:%02x:%02x], "),
				lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17],
				lpSubcode[19], lpSubcode[20], lpSubcode[21]);
		}
		else {
			_sntprintf(str2, BufSize,
				_T("Track[%02x], Idx[%02x], RelTime[%02x:%02x:%02x], AbsTime[%02x:%02x:%02x], "),
				lpSubcode[13], lpSubcode[14], lpSubcode[15], lpSubcode[16],
				lpSubcode[17], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
		}
		break;
	case ADR_ENCODES_MEDIA_CATALOG:
		_sntprintf(str2, BufSize,
			_T("Media Catalog Number  [%13s], AbsTime[     :%02x], "),
			pDiscData->SUB_CHANNEL.szCatalog, lpSubcode[21]);
		break;
	case ADR_ENCODES_ISRC:
		_sntprintf(str2, BufSize,
			_T("Itn Std Recording Code [%12s], AbsTime[     :%02x], "),
			pDiscData->SUB_CHANNEL.pszISRC[byTrackNum - 1], lpSubcode[21]);
		break;
	case 5:
		if (lpSubcode[14] == 0xB0) {
			_sntprintf(str2, BufSize,
				_T("Track[%02x], Point[%02x], StartTimeForTheNexSession[%02x:%02x:%02x], NumberOfDifferentMode-5-%02x, OutermostLead-out[%02x:%02x:%02x], "),
				lpSubcode[13], lpSubcode[14], lpSubcode[15], lpSubcode[16],
				lpSubcode[17], lpSubcode[18], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
		}
		else if (lpSubcode[14] == 0xB1) {
			_sntprintf(str2, BufSize,
				_T("Track[%02x], Point[%02x], NumberOfSkipIntervalPointers[%02x], NumberOfSkipTrackAssignmentsInPoint[%02x], "),
				lpSubcode[13], lpSubcode[14], lpSubcode[19], lpSubcode[20]);
		}
		else if (lpSubcode[14] == 0xB2 || lpSubcode[14] == 0xB3 || lpSubcode[14] == 0xB4) {
			_sntprintf(str2, BufSize,
				_T("Track[%02x], Point[%02x], TrackNumberToSkipUponPlayback[%02x %02x %02x %02x %02x %02x], "),
				lpSubcode[13], lpSubcode[14], lpSubcode[15], lpSubcode[16],
				lpSubcode[17], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
		}
		else if (lpSubcode[14] == 0xC0) {
			_sntprintf(str2, BufSize,
				_T("Track[%02x], Point[%02x], ATIPValues[%02x %02x %02x], StartTimeOfTheFirstLead-in[%02x:%02x:%02x], "),
				lpSubcode[13], lpSubcode[14], lpSubcode[15], lpSubcode[16],
				lpSubcode[17], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
		}
		else {
			_sntprintf(str2, BufSize,
				_T("Track[%02x], Point[%02x], SkipIntervalStopTime[%02x:%02x:%02x], SkipIntervalStartTime[%02x:%02x:%02x], "),
				lpSubcode[13], lpSubcode[14], lpSubcode[15], lpSubcode[16],
				lpSubcode[17], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
		}
		break;
	default:
		_sntprintf(str2, BufSize,
			_T("Track[%02x], Idx[%02x], RelTime[%02x:%02x:%02x], AbsTime[%02x:%02x:%02x], "),
			lpSubcode[13], lpSubcode[14], lpSubcode[15], lpSubcode[16],
			lpSubcode[17], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
		break;
	}

	SUB_R_TO_W_DATA scRW[4] = { 0 };
	BYTE tmpCode[24] = { 0 };
	_TCHAR str3[128] = { 0 };
	_tcsncat(str3, _T("RtoW["), 5);
	for (INT i = 0; i < 4; i++) {
		for (INT j = 0; j < 24; j++) {
			tmpCode[j] = (BYTE)(*(lpSubcodeOrg + (i * 24 + j)) & 0x3F);
		}
		memcpy(&scRW[i], tmpCode, sizeof(scRW[i]));
		switch (scRW[i].command) {
		case 0: // MODE 0, ITEM 0
			_tcsncat(str3, _T("Zero"), 4);
			break;
		case 8: // MODE 1, ITEM 0
			_tcsncat(str3, _T("Line-Graphics"), 13);
			break;
		case 9: // MODE 1, ITEM 1
			_tcsncat(str3, _T("TV-Graphics"), 11);
			break;
		case 10: // MODE 1, ITEM 2
			_tcsncat(str3, _T("Extended-TV-Graphics"), 20);
			break;
		case 20: // MODE 2, ITEM 4
			_tcsncat(str3, _T("CDText"), 6);
			break;
		case 24: // MODE 3, ITEM 0
			_tcsncat(str3, _T("Midi"), 4);
			break;
		case 56: // MODE 7, ITEM 0
			_tcsncat(str3, _T("User"), 4);
			break;
		default:
			_tcsncat(str3, _T("Reserved"), 8);
			break;
		}
		if (i < 3) {
			_tcsncat(str3, _T(", "), 2);
		}
		else {
			_tcsncat(str3, _T("]\n"), 2);
		}
	}
	_ftprintf(fpParse, _T("%s%s%s"), str, str2, str3);
}
