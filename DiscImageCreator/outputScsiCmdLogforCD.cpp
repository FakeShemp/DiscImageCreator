/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "convert.h"
#include "output.h"
#include "outputScsiCmdLogforCD.h"

VOID OutputFsBootRecord(
	LPBYTE lpBuf
	)
{
	CHAR str[2][32] = { "0" };
	strncpy(str[0], (PCHAR)&lpBuf[7], sizeof(str[0]));
	strncpy(str[1], (PCHAR)&lpBuf[39], sizeof(str[1]));
	OutputInfoLogA(
		"\t                       Boot System Identifier: %s\n"
		"\t                              Boot Identifier: %s\n"
		"\t                              Boot System Use: ",
		str[0],
		str[1]);
	for (INT i = 71; i <= 2047; i++) {
		OutputInfoLogA("%x", lpBuf[i]);
	}
	OutputInfoLogA("\n");
}

VOID OutputFsVolumeDescriptorFirst(
	LPBYTE lpBuf,
	CHAR str32[][32]
	)
{
	OutputInfoLogA(
		"\t                            System Identifier: %.32s\n"
		"\t                            Volume Identifier: %.32s\n"
		"\t                            Volume Space Size: %u\n",
		str32[0],
		str32[1],
		MAKELONG(MAKEWORD(lpBuf[80], lpBuf[81]),
		MAKEWORD(lpBuf[82], lpBuf[83])));
}

VOID OutputFsDirectoryRecord(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	INT nDataLen,
	LPSTR fname
	)
{
	INT nExtentPos = MAKELONG(MAKEWORD(lpBuf[2], lpBuf[3]),
		MAKEWORD(lpBuf[4], lpBuf[5]));
	CHAR str[128] { 0 };
	INT nFileFlag = lpBuf[25];
	if (nFileFlag & 0x01) {
		strncat(str, "Invisible, ", 11);
	}
	else {
		strncat(str, "Visible, ", 9);
	}
	if (nFileFlag & 0x02) {
		strncat(str, "Directory, ", 11);
	}
	else {
		strncat(str, "File, ", 6);
	}
	if (nFileFlag & 0x04) {
		strncat(str, "Associated, ", 12);
	}
	else {
		strncat(str, "Disassociated, ", 15);
	}
	if (nFileFlag & 0x08) {
		strncat(str, "File has record format, ", 24);
	}
	else {
		strncat(str, "File has't record format, ", 26);
	}
	if (nFileFlag & 0x10) {
		strncat(str, "Owner/Group ID has, ", 20);
	}
	else {
		strncat(str, "Owner/Group ID has't, ", 22);
	}
	if (nFileFlag & 0x80) {
		strncat(str, "Next Directory Record has", 25);
	}
	else {
		strncat(str, "Final Directory Record", 22);
	}
	OutputInfoLogA(
		"\t\t      Length of Directory Record: %u\n"
		"\t\tExtended Attribute Record Length: %u\n"
		"\t\t              Location of Extent: %u\n"
		"\t\t                     Data Length: %u\n"
		"\t\t         Recording Date and Time: %u-%02u-%02u %02u:%02u:%02u +%02u\n"
		"\t\t                      File Flags: %u (%s)\n"
		"\t\t                  File Unit Size: %u\n"
		"\t\t             Interleave Gap Size: %u\n"
		"\t\t          Volume Sequence Number: %u\n"
		"\t\t       Length of File Identifier: %u\n"
		"\t\t                 File Identifier: "
		, lpBuf[0]
		, lpBuf[1]
		, nExtentPos
		, nDataLen
		, lpBuf[18] + 1900, lpBuf[19], lpBuf[20]
		, lpBuf[21], lpBuf[22], lpBuf[23], lpBuf[24]
		, lpBuf[25], str
		, lpBuf[26]
		, lpBuf[27]
		, MAKEWORD(lpBuf[28], lpBuf[29])
		, lpBuf[32]);
	for (INT n = 0; n < lpBuf[32]; n++) {
		OutputInfoLogA("%c", lpBuf[33 + n]);
		fname[n] = (CHAR)lpBuf[33 + n];
	}
	OutputInfoLogA("\n");
	if (pExtArg->bReadContinue) {
		if ((nFileFlag & 0x02) == 0 && pDisc->PROTECT.bExist) {
			if (pDisc->PROTECT.ERROR_SECTOR.nExtentPos < nExtentPos) {
				if (pDisc->PROTECT.bTmpForSafeDisc) {
					pDisc->PROTECT.ERROR_SECTOR.nNextExtentPos = nExtentPos;
					pDisc->PROTECT.bTmpForSafeDisc = FALSE;
				}
				if (nExtentPos <= pDisc->PROTECT.ERROR_SECTOR.nNextExtentPos) {
					pDisc->PROTECT.ERROR_SECTOR.nSectorSize = nExtentPos - pDisc->PROTECT.ERROR_SECTOR.nExtentPos - 1;
					pDisc->PROTECT.ERROR_SECTOR.nNextExtentPos = nExtentPos;
				}
			}
		}
		if (!strncmp(fname, "PROTECT.PRO", 11) ||
			!strncmp(fname, "00000001.LT1", 12) ||
			!strncmp(fname, "00000001.TMP", 12)
			) {
			pDisc->PROTECT.bExist = TRUE;
			if (!strncmp(fname, "PROTECT.PRO", 11)) {
				strncpy(pDisc->PROTECT.name, fname, 11);
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos = nExtentPos;
				pDisc->PROTECT.ERROR_SECTOR.nSectorSize = nDataLen / DISC_RAW_READ - 1;
			}
			else if (!strncmp(fname, "00000001.LT1", 12) ||
				!strncmp(fname, "00000001.TMP", 12)
				) {
				strncpy(pDisc->PROTECT.name, fname, 12);
				pDisc->PROTECT.bTmpForSafeDisc = TRUE;
				pDisc->PROTECT.ERROR_SECTOR.nExtentPos = nExtentPos + nDataLen / DISC_RAW_READ;
			}
		}
	}
}

VOID OutputFsVolumeDescriptorSecond(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
	CHAR str128[][128],
	CHAR str37[][37],
	BOOL bTCHAR
	)
{
	OutputInfoLogA(
		"\t                              Volume Set Size: %u\n"
		"\t                       Volume Sequence Number: %u\n"
		"\t                           Logical Block Size: %u\n"
		"\t                              Path Table Size: %u\n"
		"\t         Location of Occurrence of Path Table: %u\n"
		"\tLocation of Optional Occurrence of Path Table: %u\n"
		, MAKEWORD(lpBuf[120], lpBuf[121])
		, MAKEWORD(lpBuf[124], lpBuf[125])
		, MAKEWORD(lpBuf[128], lpBuf[129])
		, MAKELONG(MAKEWORD(lpBuf[132], lpBuf[133]), 
		MAKEWORD(lpBuf[134], lpBuf[135]))
		, MAKELONG(MAKEWORD(lpBuf[140], lpBuf[141]),
		MAKEWORD(lpBuf[142], lpBuf[143]))
		, MAKELONG(MAKEWORD(lpBuf[144], lpBuf[145]),
		MAKEWORD(lpBuf[146], lpBuf[147])));
	INT nDataLen = MAKELONG(MAKEWORD(lpBuf[166], lpBuf[167]),
		MAKEWORD(lpBuf[168], lpBuf[169]));
	CHAR fname[64] = { 0 };
	OutputFsDirectoryRecord(pExtArg, pDisc, lpBuf + 156, nDataLen, fname);
	if (bTCHAR) {
		OutputInfoLogA(
			"\t                        Volume Set Identifier: %.64s\n"
			"\t                         Publisher Identifier: %.64s\n"
			"\t                     Data Preparer Identifier: %.64s\n"
			"\t                       Application Identifier: %.64s\n"
			"\t                    Copyright File Identifier: %.18s\n"
			"\t                     Abstract File Identifier: %.18s\n"
			"\t                Bibliographic File Identifier: %.18s\n"
			, str128[0]
			, str128[1]
			, str128[2]
			, str128[3]
			, str37[0]
			, str37[1]
			, str37[2]);
	}
	else {
		OutputInfoLogA(
			"\t                        Volume Set Identifier: %.128s\n"
			"\t                         Publisher Identifier: %.128s\n"
			"\t                     Data Preparer Identifier: %.128s\n"
			"\t                       Application Identifier: %.128s\n"
			"\t                    Copyright File Identifier: %.37s\n"
			"\t                     Abstract File Identifier: %.37s\n"
			"\t                Bibliographic File Identifier: %.37s\n"
			, str128[0]
			, str128[1]
			, str128[2]
			, str128[3]
			, str37[0]
			, str37[1]
			, str37[2]);
	}
}

VOID OutputFsVolumeDescriptorForTime(
	LPBYTE lpBuf
	)
{
	CHAR year[4][4] = { "0" };
	CHAR month[4][2] = { "0" };
	CHAR day[4][2] = { "0" };
	CHAR hour[4][2] = { "0" };
	CHAR time[4][2] = { "0" };
	CHAR second[4][2] = { "0" };
	CHAR milisecond[4][2] = { "0" };
	strncpy(year[0], (PCHAR)&lpBuf[813], sizeof(year[0]));
	strncpy(month[0], (PCHAR)&lpBuf[817], sizeof(month[0]));
	strncpy(day[0], (PCHAR)&lpBuf[819], sizeof(day[0]));
	strncpy(hour[0], (PCHAR)&lpBuf[821], sizeof(hour[0]));
	strncpy(time[0], (PCHAR)&lpBuf[823], sizeof(time[0]));
	strncpy(second[0], (PCHAR)&lpBuf[825], sizeof(second[0]));
	strncpy(milisecond[0], (PCHAR)&lpBuf[827], sizeof(milisecond[0]));
	strncpy(year[1], (PCHAR)&lpBuf[830], sizeof(year[1]));
	strncpy(month[1], (PCHAR)&lpBuf[834], sizeof(month[1]));
	strncpy(day[1], (PCHAR)&lpBuf[836], sizeof(day[1]));
	strncpy(hour[1], (PCHAR)&lpBuf[838], sizeof(hour[1]));
	strncpy(time[1], (PCHAR)&lpBuf[840], sizeof(time[1]));
	strncpy(second[1], (PCHAR)&lpBuf[842], sizeof(second[1]));
	strncpy(milisecond[1], (PCHAR)&lpBuf[844], sizeof(milisecond[1]));
	strncpy(year[2], (PCHAR)&lpBuf[847], sizeof(year[2]));
	strncpy(month[2], (PCHAR)&lpBuf[851], sizeof(month[2]));
	strncpy(day[2], (PCHAR)&lpBuf[853], sizeof(day[2]));
	strncpy(hour[2], (PCHAR)&lpBuf[855], sizeof(hour[2]));
	strncpy(time[2], (PCHAR)&lpBuf[857], sizeof(time[2]));
	strncpy(second[2], (PCHAR)&lpBuf[859], sizeof(second[2]));
	strncpy(milisecond[2], (PCHAR)&lpBuf[861], sizeof(milisecond[2]));
	strncpy(year[3], (PCHAR)&lpBuf[864], sizeof(year[3]));
	strncpy(month[3], (PCHAR)&lpBuf[868], sizeof(month[3]));
	strncpy(day[3], (PCHAR)&lpBuf[870], sizeof(day[3]));
	strncpy(hour[3], (PCHAR)&lpBuf[872], sizeof(hour[3]));
	strncpy(time[3], (PCHAR)&lpBuf[874], sizeof(time[3]));
	strncpy(second[3], (PCHAR)&lpBuf[876], sizeof(second[3]));
	strncpy(milisecond[3], (PCHAR)&lpBuf[878], sizeof(milisecond[3]));
	OutputInfoLogA(
		"\t                Volume Creation Date and Time: %.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.2s +%u\n"
		"\t            Volume Modification Date and Time: %.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.2s +%u\n"
		"\t              Volume Expiration Date and Time: %.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.2s +%u\n"
		"\t               Volume Effective Date and Time: %.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.2s +%u\n"
		"\t                       File Structure Version: %u\n"
		"\t                              Application Use: ",
		year[0], month[0], day[0], hour[0], time[0], second[0], milisecond[0], lpBuf[829],
		year[1], month[1], day[1], hour[1], time[1], second[1], milisecond[1], lpBuf[846],
		year[2], month[2], day[2], hour[2], time[2], second[2], milisecond[2], lpBuf[863],
		year[3], month[3], day[3], hour[3], time[3], second[3], milisecond[3], lpBuf[880],
		lpBuf[881]);
	for (INT i = 883; i <= 1394; i++) {
		OutputInfoLogA("%x", lpBuf[i]);
	}
	OutputInfoLogA("\n");
}

VOID OutputFsVolumeDescriptorForISO9660(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf
	)
{
	CHAR str32[3][32] = { 0 };
	CHAR str128[4][128] = { 0 };
	CHAR str37[3][37] = { 0 };
	strncpy(str32[0], (PCHAR)&lpBuf[8], sizeof(str32[0]));
	strncpy(str32[1], (PCHAR)&lpBuf[40], sizeof(str32[1]));
	strncpy(str128[0], (PCHAR)&lpBuf[190], sizeof(str128[0]));
	strncpy(str128[1], (PCHAR)&lpBuf[318], sizeof(str128[1]));
	strncpy(str128[2], (PCHAR)&lpBuf[446], sizeof(str128[2]));
	strncpy(str128[3], (PCHAR)&lpBuf[574], sizeof(str128[3]));
	strncpy(str37[0], (PCHAR)&lpBuf[702], sizeof(str37[0]));
	strncpy(str37[1], (PCHAR)&lpBuf[739], sizeof(str37[1]));
	strncpy(str37[2], (PCHAR)&lpBuf[776], sizeof(str37[2]));
	OutputFsVolumeDescriptorFirst(lpBuf, str32);
	if (lpBuf[0] == 2) {
		strncpy(str32[2], (PCHAR)&lpBuf[88], 32);
		OutputInfoLogA(
			"\t                             Escape Sequences: %.32s\n", str32[2]);
	}
	OutputFsVolumeDescriptorSecond(pExtArg, pDisc, lpBuf, str128, str37, FALSE);
	OutputFsVolumeDescriptorForTime(lpBuf);
}

VOID OutputFsVolumeDescriptorForJoliet(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf
	)
{
	CHAR str32[3][32] = { 0 };
	CHAR str128[4][128] = { 0 };
	CHAR str37[3][37] = { 0 };
	wchar_t tmp16[3][16] = { 0 };
	wchar_t tmp64[4][64] = { 0 };
	wchar_t tmp18[3][18] = { 0 };
	BOOL bTCHAR = FALSE;
	if (lpBuf[8] == 0 && lpBuf[9] >= 0x20) {
		LittleToBig(tmp16[0], (wchar_t*)&lpBuf[8], 32);
		bTCHAR = TRUE;
	}
	else if (lpBuf[8] >= 0x20 && lpBuf[9] == 0) {
		wcsncpy(tmp16[0], (wchar_t*)&lpBuf[8], 16);
		bTCHAR = TRUE;
	}
	if (bTCHAR) {
		if (!WideCharToMultiByte(CP_ACP, 0,
			(LPCWSTR)&tmp16[0], 16, str32[0], sizeof(str32[0]), NULL, NULL)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		}
	}
	else {
		strncpy(str32[0], (PCHAR)&lpBuf[8], sizeof(str32[0]));
	}

	if (lpBuf[40] == 0 && lpBuf[41] >= 0x20) {
		LittleToBig(tmp16[1], (wchar_t*)&lpBuf[40], 32);
	}
	else if (lpBuf[40] >= 0x20 && lpBuf[41] == 0) {
		wcsncpy(tmp16[1], (wchar_t*)&lpBuf[40], 16);
	}
	if (!WideCharToMultiByte(CP_ACP, 0,
		(LPCWSTR)&tmp16[1], 16, str32[1], sizeof(str32[1]), NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}

	if (lpBuf[190] == 0 && lpBuf[191] >= 0x20) {
		LittleToBig(tmp64[0], (wchar_t*)&lpBuf[190], 128);
	}
	else if (lpBuf[190] >= 0x20 && lpBuf[191] == 0) {
		wcsncpy(tmp64[0], (wchar_t*)&lpBuf[190], 64);
	}
	if (!WideCharToMultiByte(CP_ACP, 0,
		(LPCWSTR)&tmp64[0], 64, str128[0], sizeof(str128[0]), NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}

	if (lpBuf[318] == 0 && lpBuf[319] >= 0x20) {
		LittleToBig(tmp64[1], (wchar_t*)&lpBuf[318], 128);
	}
	else if (lpBuf[318] >= 0x20 && lpBuf[319] == 0) {
		wcsncpy(tmp64[1], (wchar_t*)&lpBuf[318], 64);
	}
	if (!WideCharToMultiByte(CP_ACP, 0,
		(LPCWSTR)&tmp64[1], 64, str128[1], sizeof(str128[1]), NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}

	if (lpBuf[446] == 0 && lpBuf[447] >= 0x20) {
		LittleToBig(tmp64[2], (wchar_t*)&lpBuf[446], 128);
	}
	else if (lpBuf[446] >= 0x20 && lpBuf[447] == 0) {
		wcsncpy(tmp64[2], (wchar_t*)&lpBuf[446], 64);
	}
	if (!WideCharToMultiByte(CP_ACP, 0,
		(LPCWSTR)&tmp64[2], 64, str128[2], sizeof(str128[2]), NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}

	if (lpBuf[574] == 0 && lpBuf[575] >= 0x20) {
		LittleToBig(tmp64[3], (wchar_t*)&lpBuf[574], 128);
	}
	else if (lpBuf[574] >= 0x20 && lpBuf[575] == 0) {
		wcsncpy(tmp64[3], (wchar_t*)&lpBuf[574], 64);
	}
	if (!WideCharToMultiByte(CP_ACP, 0,
		(LPCWSTR)&tmp64[3], 64, str128[3], sizeof(str128[3]), NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}

	if (lpBuf[702] == 0 && lpBuf[703] >= 0x20) {
		LittleToBig(tmp18[0], (wchar_t*)&lpBuf[702], 36);
	}
	else if (lpBuf[702] >= 0x20 && lpBuf[703] == 0) {
		wcsncpy(tmp18[0], (wchar_t*)&lpBuf[702], 18);
	}
	if (!WideCharToMultiByte(CP_ACP, 0,
		(LPCWSTR)&tmp18[0], 18, str37[0], sizeof(str37[0]) - 1, NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}

	if (lpBuf[739] == 0 && lpBuf[740] >= 0x20) {
		LittleToBig(tmp18[1], (wchar_t*)&lpBuf[739], 36);
	}
	else if (lpBuf[739] >= 0x20 && lpBuf[740] == 0) {
		wcsncpy(tmp18[1], (wchar_t*)&lpBuf[739], 18);
	}
	if (!WideCharToMultiByte(CP_ACP, 0,
		(LPCWSTR)&tmp18[1], 18, str37[1], sizeof(str37[1]) - 1, NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}

	if (lpBuf[776] == 0 && lpBuf[777] >= 0x20) {
		LittleToBig(tmp18[2], (wchar_t*)&lpBuf[776], 36);
	}
	else if (lpBuf[776] >= 0x20 && lpBuf[777] == 0) {
		wcsncpy(tmp18[2], (wchar_t*)&lpBuf[776], 18);
	}
	if (!WideCharToMultiByte(CP_ACP, 0,
		(LPCWSTR)&tmp18[2], 18, str37[2], sizeof(str37[2]) - 1, NULL, NULL)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}

	OutputFsVolumeDescriptorFirst(lpBuf, str32);

	strncpy(str32[2], (PCHAR)&lpBuf[88], 32);
	OutputInfoLogA(
		"\t                             Escape Sequences: %.32s\n", str32[2]);
	OutputFsVolumeDescriptorSecond(pExtArg, pDisc, lpBuf, str128, str37, TRUE);
	OutputFsVolumeDescriptorForTime(lpBuf);
}

VOID OutputFsVolumePartitionDescriptor(
	LPBYTE lpBuf
	)
{
	CHAR str[2][32] = { 0 };
	strncpy(str[0], (PCHAR)&lpBuf[8], 32);
	strncpy(str[1], (PCHAR)&lpBuf[40], 32);
	OutputInfoLogA(
		"\t          System Identifier: %.32s\n"
		"\tVolume Partition Identifier: %.32s\n"
		"\t  Volume Partition Location: %u\n"
		"\t      Volume Partition Size: %u\n"
		"\t                 System Use: ",
		str[0],
		str[1],
		MAKELONG(MAKEWORD(lpBuf[76], lpBuf[77]),
		MAKEWORD(lpBuf[78], lpBuf[79])),
		MAKELONG(MAKEWORD(lpBuf[84], lpBuf[85]),
		MAKEWORD(lpBuf[86], lpBuf[87])));
	for (INT i = 88; i <= 2047; i++) {
		OutputInfoLogA("%x", lpBuf[i]);
	}
	OutputInfoLogA("\n");
}

VOID OutputFsVolumeDescriptor(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf,
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
	strncpy(str, (PCHAR)&lpBuf[1], sizeof(str));
	OutputInfoLogA(
		"=================== Volume Descriptor, LBA[%06d, %#07x] ===================\n"
		"\t                       Volume Descriptor Type: %u\n"
		"\t                          Standard Identifier: %.5s\n"
		"\t                    Volume Descriptor Version: %u\n",
		nLBA, nLBA,
		lpBuf[0],
		str,
		lpBuf[6]);

	if (lpBuf[0] == 0) {
		OutputFsBootRecord(lpBuf);
	}
	else if (lpBuf[0] == 1) {
		OutputFsVolumeDescriptorForISO9660(pExtArg, pDisc, lpBuf);
	}
	else if (lpBuf[0] == 2) {
		OutputFsVolumeDescriptorForJoliet(pExtArg, pDisc, lpBuf);
	}
	else if (lpBuf[0] == 3) {
		OutputFsVolumePartitionDescriptor(lpBuf);
	}
	else if (lpBuf[0] == 255) {
		// all zero
	}
}

VOID OutputFsPathTableRecord(
	LPBYTE lpBuf,
	INT nLBA,
	INT nPathTblSize,
	LPINT pDirTblPosList,
	LPSTR* pDirTblNameList,
	LPINT nDirPosNum
	)
{
	OutputInfoLogA(
		"=================== Path Table Record, LBA[%06d, %#07x] ===================\n"
		, nLBA, nLBA);
	for (INT i = 0; i < nPathTblSize;) {
		size_t nLenDI = lpBuf[i];
		INT nPos = MAKELONG(MAKEWORD(lpBuf[2 + i], lpBuf[3 + i]),
			MAKEWORD(lpBuf[4 + i], lpBuf[5 + i]));
		if (nLenDI > 0) {
			OutputInfoLogA(
				"     Length of Directory Identifier: %u\n"
				"Length of Extended Attribute Record: %u\n"
				"                 Position of Extent: %u\n"
				"          Number of Upper Directory: %u\n"
				"               Directory Identifier: "
				, nLenDI
				, lpBuf[1 + i]
				, nPos
				, MAKEWORD(lpBuf[6 + i], lpBuf[7 + i])
				);
			for (size_t n = 0; n < nLenDI; n++) {
				OutputInfoLogA("%c", lpBuf[8 + i + n]);
				pDirTblNameList[*nDirPosNum][n] = (CHAR)lpBuf[8 + i + n];
			}
			OutputInfoLogA("\n\n");
			pDirTblPosList[*nDirPosNum] = nPos;
			*nDirPosNum = *nDirPosNum + 1;
			
			i += 8 + nLenDI;
			if ((i % 2) != 0) {
				i++;
			}
		}
	}
}

/*
http://www.dubeyko.com/development/FileSystems/HFS/hfs_ondisk_layout/Files-102.html
*/
VOID OutputFsMasterDirectoryBlocks(
	LPBYTE lpBuf,
	INT nLBA
	)
{
	CHAR str[27] = { 0 };
	strncpy(str, (PCHAR)&lpBuf[36], sizeof(str));
	OutputInfoLogA(
		"================ Master Directory Blocks, LBA[%06d, %#07x] ================\n"
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
		MAKEWORD(lpBuf[1], lpBuf[0]),
		MAKELONG(MAKEWORD(lpBuf[5], lpBuf[4]), MAKEWORD(lpBuf[3], lpBuf[2])),
		MAKELONG(MAKEWORD(lpBuf[9], lpBuf[8]),
		MAKEWORD(lpBuf[7], lpBuf[6])),
		MAKEWORD(lpBuf[11], lpBuf[10]),
		MAKEWORD(lpBuf[13], lpBuf[12]),
		MAKEWORD(lpBuf[15], lpBuf[14]),
		MAKEWORD(lpBuf[17], lpBuf[16]),
		MAKEWORD(lpBuf[19], lpBuf[18]),
		MAKELONG(MAKEWORD(lpBuf[23], lpBuf[22]), MAKEWORD(lpBuf[21], lpBuf[20])),
		MAKELONG(MAKEWORD(lpBuf[27], lpBuf[26]), MAKEWORD(lpBuf[25], lpBuf[24])),
		MAKEWORD(lpBuf[29], lpBuf[28]),
		MAKELONG(MAKEWORD(lpBuf[33], lpBuf[32]), MAKEWORD(lpBuf[31], lpBuf[30])),
		MAKEWORD(lpBuf[35], lpBuf[34]),
		str,
		MAKELONG(MAKEWORD(lpBuf[66], lpBuf[65]), MAKEWORD(lpBuf[64], lpBuf[63])),
		MAKEWORD(lpBuf[68], lpBuf[67]),
		MAKELONG(MAKEWORD(lpBuf[72], lpBuf[71]), MAKEWORD(lpBuf[70], lpBuf[69])),
		MAKELONG(MAKEWORD(lpBuf[76], lpBuf[75]), MAKEWORD(lpBuf[74], lpBuf[73])),
		MAKELONG(MAKEWORD(lpBuf[80], lpBuf[79]), MAKEWORD(lpBuf[78], lpBuf[77])),
		MAKEWORD(lpBuf[82], lpBuf[81]),
		MAKELONG(MAKEWORD(lpBuf[86], lpBuf[85]), MAKEWORD(lpBuf[84], lpBuf[83])),
		MAKELONG(MAKEWORD(lpBuf[90], lpBuf[89]), MAKEWORD(lpBuf[88], lpBuf[87])),
		MAKELONG(MAKEWORD(lpBuf[94], lpBuf[93]), MAKEWORD(lpBuf[92], lpBuf[91])),
		MAKELONG(MAKEWORD(lpBuf[98], lpBuf[97]), MAKEWORD(lpBuf[96], lpBuf[95])),
		MAKELONG(MAKEWORD(lpBuf[102], lpBuf[101]), MAKEWORD(lpBuf[100], lpBuf[99])),
		MAKELONG(MAKEWORD(lpBuf[106], lpBuf[105]), MAKEWORD(lpBuf[104], lpBuf[103])),
		MAKELONG(MAKEWORD(lpBuf[110], lpBuf[109]), MAKEWORD(lpBuf[108], lpBuf[107])),
		MAKELONG(MAKEWORD(lpBuf[114], lpBuf[113]), MAKEWORD(lpBuf[112], lpBuf[111])),
		MAKELONG(MAKEWORD(lpBuf[118], lpBuf[117]), MAKEWORD(lpBuf[116], lpBuf[115])),
		MAKELONG(MAKEWORD(lpBuf[122], lpBuf[121]), MAKEWORD(lpBuf[120], lpBuf[119])),
		MAKEWORD(lpBuf[124], lpBuf[123]),
		MAKEWORD(lpBuf[126], lpBuf[125]),
		MAKEWORD(lpBuf[128], lpBuf[127]),
		MAKELONG(MAKEWORD(lpBuf[132], lpBuf[131]), MAKEWORD(lpBuf[130], lpBuf[129])),
		MAKELONG(MAKEWORD(lpBuf[136], lpBuf[135]), MAKEWORD(lpBuf[134], lpBuf[133])),
		MAKELONG(MAKEWORD(lpBuf[140], lpBuf[139]), MAKEWORD(lpBuf[138], lpBuf[137])),
		MAKELONG(MAKEWORD(lpBuf[144], lpBuf[143]), MAKEWORD(lpBuf[142], lpBuf[141])),
		MAKELONG(MAKEWORD(lpBuf[148], lpBuf[147]), MAKEWORD(lpBuf[146], lpBuf[145])),
		MAKELONG(MAKEWORD(lpBuf[152], lpBuf[151]), MAKEWORD(lpBuf[150], lpBuf[149])),
		MAKELONG(MAKEWORD(lpBuf[156], lpBuf[155]), MAKEWORD(lpBuf[154], lpBuf[153])),
		MAKELONG(MAKEWORD(lpBuf[160], lpBuf[159]), MAKEWORD(lpBuf[158], lpBuf[157])));
}

VOID OutputFs3doHeader(
	LPBYTE lpBuf,
	INT nLBA
	)
{
	OutputInfoLogA(
		"======================= 3DO Header, LBA[%06d, %#07x] ======================\n"
		"\t                Record Type: %#04x\n"
		"\t                 Sync Bytes: %#04x %#04x %#04x %#04x %#04x\n"
		"\t             Record Version: %#04x\n"
		"\t               Volume Flags: %#04x\n"
		"\t             Volume Comment: "
		, nLBA, nLBA,
		lpBuf[0],
		lpBuf[1], lpBuf[2], lpBuf[3], lpBuf[4], lpBuf[5],
		lpBuf[6],
		lpBuf[7]);
	for (INT i = 0; i < 32; i++) {
		OutputInfoLogA("%c", lpBuf[8 + i]);
	}
	OutputInfoLogA(
		"\n"
		"\t               Volume Label: ");
	for (INT i = 0; i < 32; i++) {
		OutputInfoLogA("%c", lpBuf[40 + i]);
	}
	LONG dwNumOfCopy =
		MAKELONG(MAKEWORD(lpBuf[99], lpBuf[98]), MAKEWORD(lpBuf[97], lpBuf[96]));
	OutputInfoLogA(
		"\n"
		"\t                  Volume ID: %#10x\n"
		"\t         Logical Block Size: %u\n"
		"\t          Volume Space Size: %u + 152\n"
		"\t                Root Dir ID: %#10x\n"
		"\t            Root Dir Blocks: %u\n"
		"\t        Root Dir Block Size: %u\n"
		"\tNum of Pos of Root Dir Copy: %u\n"
		"\t            Pos of Root Dir: %u\n",
		MAKELONG(MAKEWORD(lpBuf[75], lpBuf[74]), MAKEWORD(lpBuf[73], lpBuf[72])),
		MAKELONG(MAKEWORD(lpBuf[79], lpBuf[78]), MAKEWORD(lpBuf[77], lpBuf[76])),
		MAKELONG(MAKEWORD(lpBuf[83], lpBuf[82]), MAKEWORD(lpBuf[81], lpBuf[80])),
		MAKELONG(MAKEWORD(lpBuf[87], lpBuf[86]), MAKEWORD(lpBuf[85], lpBuf[84])),
		MAKELONG(MAKEWORD(lpBuf[91], lpBuf[90]), MAKEWORD(lpBuf[89], lpBuf[88])),
		MAKELONG(MAKEWORD(lpBuf[95], lpBuf[94]), MAKEWORD(lpBuf[93], lpBuf[92])),
		dwNumOfCopy,
		MAKELONG(MAKEWORD(lpBuf[103], lpBuf[102]), MAKEWORD(lpBuf[101], lpBuf[100])));

	for (LONG i = 0; i < dwNumOfCopy; i++) {
		OutputInfoLogA(
			"\t       Pos of Root Dir Copy: %u\n",
			MAKELONG(MAKEWORD(lpBuf[107 + i * 4], lpBuf[106 + i * 4]),
			MAKEWORD(lpBuf[105 + i * 4], lpBuf[104 + i * 4])));
	}
}

VOID OutputFs3doDirectoryRecord(
	LPBYTE lpBuf,
	INT nLBA,
	PCHAR path,
	LONG directorySize
	)
{
	OutputInfoLogA(
		"==================== Directory Record, LBA[%06d, %#07x] ===================\n"
		"\tcurrentDir: %s\n"
		"\t==================== Directory Header ===================\n"
		"\t      nextBlock: %#08x\n"
		"\t      prevBlock: %#08x\n"
		"\t          flags: %u\n"
		"\t  directorySize: %u\n"
		"\tdirectoryOffset: %u\n"
		, nLBA, nLBA
		, path
		, MAKELONG(MAKEWORD(lpBuf[3], lpBuf[2]), MAKEWORD(lpBuf[1], lpBuf[0]))
		, MAKELONG(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], lpBuf[4]))
		, MAKELONG(MAKEWORD(lpBuf[11], lpBuf[10]), MAKEWORD(lpBuf[9], lpBuf[8]))
		, directorySize
		, MAKELONG(MAKEWORD(lpBuf[19], lpBuf[18]), MAKEWORD(lpBuf[17], lpBuf[16])));

	LONG cur = THREEDO_DIR_HEADER_SIZE;
	LONG lastCopy = 0;
	CHAR fname[32] = { 0 };
	while (cur < directorySize) {
		LPBYTE dirEnt = lpBuf + cur;
		strncpy(fname, (PCHAR)&dirEnt[32], sizeof(fname));
		lastCopy = MAKELONG(MAKEWORD(dirEnt[67], dirEnt[66]), MAKEWORD(dirEnt[65], dirEnt[64]));
		cur += THREEDO_DIR_ENTRY_SIZE;
		OutputInfoLogA(
			"\t==================== Directory Entry ===================\n"
			"\t            flags: %#010x\n"
			"\t               id: %#08x\n"
			"\t              ext: %c%c%c%c\n"
			"\t        blockSize: %u\n"
			"\t entryLengthBytes: %u\n"
			"\tentryLengthBlocks: %u\n"
			"\t            burst: %u\n"
			"\t              gap: %u\n"
			"\t         fileName: %s\n"
			"\t         copy num: %u\n"
			"\t         data pos: %u\n"
			, MAKELONG(MAKEWORD(dirEnt[3], dirEnt[2]), MAKEWORD(dirEnt[1], dirEnt[0]))
			, MAKELONG(MAKEWORD(dirEnt[7], dirEnt[6]), MAKEWORD(dirEnt[5], dirEnt[4]))
			, dirEnt[8], dirEnt[9], dirEnt[10], dirEnt[11]
			, MAKELONG(MAKEWORD(dirEnt[15], dirEnt[14]), MAKEWORD(dirEnt[13], dirEnt[12]))
			, MAKELONG(MAKEWORD(dirEnt[19], dirEnt[18]), MAKEWORD(dirEnt[17], dirEnt[16]))
			, MAKELONG(MAKEWORD(dirEnt[23], dirEnt[22]), MAKEWORD(dirEnt[21], dirEnt[20]))
			, MAKELONG(MAKEWORD(dirEnt[27], dirEnt[26]), MAKEWORD(dirEnt[25], dirEnt[24]))
			, MAKELONG(MAKEWORD(dirEnt[31], dirEnt[30]), MAKEWORD(dirEnt[29], dirEnt[28]))
			, fname
			, lastCopy
			, MAKELONG(MAKEWORD(dirEnt[71], dirEnt[70]), MAKEWORD(dirEnt[69], dirEnt[68])));
		for (LONG i = 0; i < lastCopy; i++) {
			LPBYTE pCopyPos = lpBuf + cur + sizeof(LONG) * i;
			OutputInfoLogA("\t    data copy pos: %u\n"
				, MAKELONG(MAKEWORD(pCopyPos[3], pCopyPos[2]), MAKEWORD(pCopyPos[1], pCopyPos[0])));
			cur += sizeof(LONG);
		}
	}
}

VOID OutputFsPceStuff(
	LPBYTE lpBuf,
	INT nLBA
	)
{
	OutputInfoLogA(
		"============== PCE Warning msg, all stuff, LBA[%06d, %#07x] ==============\n",
		nLBA, nLBA);
	CHAR str[39] = { 0 };
	size_t len = 0;
	for (size_t idx = 0; idx < 805; idx += len) {
		PCHAR ptr = (PCHAR)&lpBuf[idx];
		if (ptr[0] == -1) {
			len = 1;
			continue;
		}
		len = strlen(ptr) + 1;
		if (len < sizeof(str)) {
			strncpy(str, ptr, len);
			OutputInfoLogA("\t%s\n", str);
		}
	}
}

VOID OutputFsPceBootSector(
	LPBYTE lpBuf,
	INT nLBA
	)
{
	CHAR str[24] = { 0 };
	strncpy(str, (PCHAR)&lpBuf[32], sizeof(str));
	CHAR str2[50] = { 0 };
	strncpy(str2, (PCHAR)&lpBuf[56], sizeof(str2));
	CHAR str3[17] = { 0 };
	strncpy(str3, (PCHAR)&lpBuf[106], sizeof(str3) - 1);
	CHAR str4[7] = { 0 };
	strncpy(str4, (PCHAR)&lpBuf[122], sizeof(str4) - 1);
	OutputInfoLogA(
		"==================== PCE Boot Sector, LBA[%06d, %#07x] ====================\n"
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
		lpBuf[0], lpBuf[1], lpBuf[2],
		lpBuf[3],
		MAKEWORD(lpBuf[4], lpBuf[5]),
		MAKEWORD(lpBuf[6], lpBuf[7]),
		lpBuf[8], lpBuf[9], lpBuf[10], lpBuf[11], lpBuf[12],
		lpBuf[13],
		lpBuf[14], lpBuf[15], lpBuf[16],
		lpBuf[17],
		MAKEWORD(lpBuf[18], lpBuf[19]),
		lpBuf[20], lpBuf[21], lpBuf[22],
		lpBuf[23],
		lpBuf[24],
		lpBuf[25], lpBuf[26], lpBuf[27], lpBuf[28], lpBuf[29], lpBuf[30], lpBuf[31],
		str, str2, str3, str4
		);
}

VOID OutputTocForGD(
	PDISC pDisc
	)
{
	OutputDiscLogA(
		"================================= TOC For GD ==================================\n");
	for (BYTE r = pDisc->GDROM_TOC.FirstTrack; r < pDisc->GDROM_TOC.LastTrack; r++) {
		OutputDiscLogA("\tTrack %2u, Ctl %u, Mode %u"
			, pDisc->GDROM_TOC.TrackData[r].TrackNumber
			, pDisc->GDROM_TOC.TrackData[r].Control
			, pDisc->GDROM_TOC.TrackData[r].Adr);
		if (pDisc->GDROM_TOC.TrackData[r].TrackNumber == 3) {
			OutputDiscLogA(", LBA %6d-%6d, Length %6d\n"
				, pDisc->GDROM_TOC.TrackData[r].Address - 150
				, pDisc->GDROM_TOC.TrackData[r + 1].Address - 1 - 300
				, pDisc->GDROM_TOC.TrackData[r + 1].Address - pDisc->GDROM_TOC.TrackData[r].Address);
		}
		else if (pDisc->GDROM_TOC.TrackData[r].TrackNumber == pDisc->GDROM_TOC.LastTrack) {
			OutputDiscLogA(", LBA %6d-%6d, Length %6d\n"
				, pDisc->GDROM_TOC.TrackData[r].Address - 375
				, pDisc->GDROM_TOC.Length - 1 - 150
				, pDisc->GDROM_TOC.Length - pDisc->GDROM_TOC.TrackData[r].Address + 225);
		}
		else if (pDisc->GDROM_TOC.TrackData[r].TrackNumber == pDisc->GDROM_TOC.LastTrack - 1) {
			OutputDiscLogA(", LBA %6d-%6d, Length %6d\n"
				, pDisc->GDROM_TOC.TrackData[r].Address - 300
				, pDisc->GDROM_TOC.TrackData[r + 1].Address - 1 - 375
				, pDisc->GDROM_TOC.TrackData[r + 1].Address - 75 - pDisc->GDROM_TOC.TrackData[r].Address);
		}
		else {
			OutputDiscLogA(", LBA %6d-%6d, Length %6d\n"
				, pDisc->GDROM_TOC.TrackData[r].Address - 300
				, pDisc->GDROM_TOC.TrackData[r + 1].Address - 1 - 300
				, pDisc->GDROM_TOC.TrackData[r + 1].Address - pDisc->GDROM_TOC.TrackData[r].Address);
		}
		pDisc->SCSI.lpFirstLBAListOnToc[r] = pDisc->GDROM_TOC.TrackData[r].Address;
	}
	OutputDiscLogA(
		"                                                 Total %6d\n"
		, pDisc->GDROM_TOC.Length - 150 - 45000);
}

VOID OutputTocWithPregap(
	PDISC pDisc
	)
{
	OutputDiscLogA(
		"=============================== TOC with pregap ===============================\n");
	for (UINT r = 0; r < pDisc->SCSI.toc.LastTrack; r++) {
		OutputDiscLogA(
			"\tTrack %2u, Ctl %u, Mode %u", r + 1,
			pDisc->SUB.lpCtlList[r], pDisc->MAIN.lpModeList[r]);
		for (UINT k = 0; k < MAXIMUM_NUMBER_INDEXES; k++) {
			if (pDisc->SUB.lpFirstLBAListOnSub[r][k] != -1) {
				OutputDiscLogA(", Index%u %6d", k,
					pDisc->SUB.lpFirstLBAListOnSub[r][k]);
			}
			else if (k == 0) {
				OutputDiscLogA(",              ");
			}
		}
		OutputDiscLogA("\n");
	}
}

VOID OutputCDOffset(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	BOOL bGetDrive,
	INT nDriveSampleOffset,
	INT nDriveOffset
	)
{
	OutputDiscLogA("======= Offset");
	if (bGetDrive) {
		OutputDiscLogA(
			"(Drive offset data referes to http://www.accuraterip.com) =======");
	}
	if (pExtArg->bAdd && pDisc->SCSI.bAudioOnly) {
		pDisc->MAIN.nCombinedOffset += pExtArg->nAudioCDOffsetNum * 4;
		OutputDiscLogA(
			"\n"
			"\t       Combined Offset(Byte) %6d, (Samples) %5d\n"
			"\t-         Drive Offset(Byte) %6d, (Samples) %5d\n"
			"\t----------------------------------------------------\n"
			"\t User Specified Offset(Byte) %6d, (Samples) %5d\n",
			pDisc->MAIN.nCombinedOffset, pDisc->MAIN.nCombinedOffset / 4,
			nDriveOffset, nDriveSampleOffset,
			pDisc->MAIN.nCombinedOffset - nDriveOffset,
			(pDisc->MAIN.nCombinedOffset - nDriveOffset) / 4);
	}
	else {
		OutputDiscLogA(
			"\n"
			"\t Combined Offset(Byte) %6d, (Samples) %5d\n"
			"\t-   Drive Offset(Byte) %6d, (Samples) %5d\n"
			"\t----------------------------------------------\n"
			"\t       CD Offset(Byte) %6d, (Samples) %5d\n",
			pDisc->MAIN.nCombinedOffset, pDisc->MAIN.nCombinedOffset / 4,
			nDriveOffset, nDriveSampleOffset,
			pDisc->MAIN.nCombinedOffset - nDriveOffset,
			(pDisc->MAIN.nCombinedOffset - nDriveOffset) / 4);
	}
}

VOID OutputCDC2Error296(
	LOG_TYPE type,
	LPBYTE lpBuf,
	INT nLBA
	)
{
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(type);
#endif
	OutputLogA(type,
		"======================== c2 error, LBA[%06d, %#07x] =======================\n"
		"\t         +0 +1 +2 +3 +4 +5 +6 +7\n", nLBA, nLBA);

	for (INT i = 0; i < CD_RAW_READ_C2_SIZE; i += 8) {
		OutputLogA(type,
			"\t%3x(%3d) %02x %02x %02x %02x %02x %02x %02x %02x\n",
			i, i, lpBuf[i], lpBuf[i + 1], lpBuf[i + 2], lpBuf[i + 3],
			lpBuf[i + 4], lpBuf[i + 5], lpBuf[i + 6], lpBuf[i + 7]);
	}
}

VOID OutputCDMain2352(
	LPBYTE lpBuf,
	INT nLBA
	)
{
	OutputDiscLogA(
		"===================== Main Channel, LBA[%06d, %#07x] ======================\n"
		"\t          +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n", nLBA, nLBA);

	for (INT i = 0; i < CD_RAW_SECTOR_SIZE; i += 16) {
		OutputDiscLogA(
			"\t%3x(%4d) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			i, i, lpBuf[i], lpBuf[i + 1], lpBuf[i + 2], lpBuf[i + 3], lpBuf[i + 4], lpBuf[i + 5],
			lpBuf[i + 6], lpBuf[i + 7], lpBuf[i + 8], lpBuf[i + 9], lpBuf[i + 10], lpBuf[i + 11],
			lpBuf[i + 12], lpBuf[i + 13], lpBuf[i + 14], lpBuf[i + 15]);
	}
}

VOID OutputCDSub96Align(
	LPBYTE lpBuf,
	INT nLBA
	)
{
	OutputDiscLogA(
		"====================== Sub Channel, LBA[%06d, %#07x] ======================\n"
		"\t  +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B\n", nLBA, nLBA);

	for (INT i = 0, ch = 0x50; i < CD_RAW_READ_SUBCODE_SIZE; i += 12, ch++) {
		OutputDiscLogA(
			"\t%c %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			ch, lpBuf[i], lpBuf[i + 1], lpBuf[i + 2], lpBuf[i + 3], lpBuf[i + 4], lpBuf[i + 5],
			lpBuf[i + 6], lpBuf[i + 7], lpBuf[i + 8], lpBuf[i + 9], lpBuf[i + 10], lpBuf[i + 11]);
	}
}

VOID OutputCDSub96Raw(
	LOG_TYPE type,
	LPBYTE lpBuf,
	INT nLBA
	)
{
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(type);
#endif
	OutputLogA(type,
		"==================== Sub Channel(Raw), LBA[%06d, %#07x] ===================\n"
		"\t    +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n", nLBA, nLBA);

	for (INT i = 0; i < CD_RAW_READ_SUBCODE_SIZE; i += 16) {
		OutputLogA(type,
			"\t%3X %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			i, lpBuf[i], lpBuf[i + 1], lpBuf[i + 2], lpBuf[i + 3], lpBuf[i + 4], lpBuf[i + 5],
			lpBuf[i + 6], lpBuf[i + 7], lpBuf[i + 8], lpBuf[i + 9], lpBuf[i + 10], lpBuf[i + 11],
			lpBuf[i + 12], lpBuf[i + 13], lpBuf[i + 14], lpBuf[i + 15]);
	}
}

VOID OutputCDSubToLog(
	PDISC pDisc,
	LPBYTE lpSubcode,
	LPBYTE lpSubcodeRaw,
	INT nLBA,
	INT byTrackNum,
	FILE* fpParse
	)
{
	CONST INT BufSize = 256;
	_TCHAR str[BufSize] = { 0 };
	// Ctl
	switch ((lpSubcode[12] >> 4) & 0x0f) {
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
	switch (lpSubcode[12] & 0x0f) {
	case ADR_ENCODES_CURRENT_POSITION:
		if (nLBA < -150 && lpSubcode[13] == 0) {
			// lead-in area
			if (lpSubcode[14] == 0xa0) {
				_sntprintf(str2, BufSize,
					_T("Point[%02x], AMSF[%02x:%02x:%02x], TrackNumOf1stTrack[%02x], ProgramAreaFormat[%02x], "),
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17], lpSubcode[19], lpSubcode[20]);
			}
			else if (lpSubcode[14] == 0xa1) {
				_sntprintf(str2, BufSize,
					_T("Point[%02x], AMSF[%02x:%02x:%02x], TrackNumOfLastTrack[%02x], "),
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17], lpSubcode[19]);
			}
			else if (lpSubcode[14] == 0xa2) {
				_sntprintf(str2, BufSize,
					_T("Point[%02x], AMSF[%02x:%02x:%02x], StartTimeOfLead-out[%02x:%02x:%02x], "),
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17],
					lpSubcode[19], lpSubcode[20], lpSubcode[21]);
			}
			else {
				_sntprintf(str2, BufSize,
					_T("Point[%02x], AMSF[%02x:%02x:%02x], TrackStartTime[%02x:%02x:%02x], "),
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17],
					lpSubcode[19], lpSubcode[20], lpSubcode[21]);
			}
		}
		else if (lpSubcode[13] == 0xaa) {
			// lead-out area
			_sntprintf(str2, BufSize,
				_T("LeadOut  , Idx[%02x], RMSF[%02x:%02x:%02x], AMSF[%02x:%02x:%02x], "),
				lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17],
				lpSubcode[19], lpSubcode[20], lpSubcode[21]);
		}
		else {
			_sntprintf(str2, BufSize,
				_T("Track[%02x], Idx[%02x], RMSF[%02x:%02x:%02x], AMSF[%02x:%02x:%02x], "),
				lpSubcode[13], lpSubcode[14], lpSubcode[15], lpSubcode[16],
				lpSubcode[17], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
		}
		break;
	case ADR_ENCODES_MEDIA_CATALOG: {
		_TCHAR str[META_CATALOG_SIZE] = { 0 };
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0,
			pDisc->SUB.szCatalog, META_CATALOG_SIZE, str, META_CATALOG_SIZE);
#else
		strncpy(str, pDisc->SUB.szCatalog, META_CATALOG_SIZE);
#endif
		_sntprintf(str2, BufSize,
			_T("MediaCatalogNumber [%13s], AMSF[     :%02x], "), str, lpSubcode[21]);
		break;
	}
	case ADR_ENCODES_ISRC: {
		_TCHAR str[META_ISRC_SIZE] = { 0 };
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0,
			pDisc->SUB.pszISRC[byTrackNum - 1], META_ISRC_SIZE, str, META_ISRC_SIZE);
#else
		strncpy(str, pDisc->SUB.pszISRC[byTrackNum - 1], META_ISRC_SIZE);
#endif
		_sntprintf(str2, BufSize,
			_T("ItnStdRecordingCode [%12s], AMSF[     :%02x], "), str, lpSubcode[21]);
		break;
	}
	case 5:
		if (lpSubcode[13] == 0) {
			if (lpSubcode[14] == 0xb0) {
				_sntprintf(str2, BufSize,
					_T("Point[%02x], StartTimeForTheNextSession[%02x:%02x:%02x], NumberOfDifferentMode-5[%02x], OutermostLead-out[%02x:%02x:%02x], "),
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17],
					lpSubcode[18], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
			}
			else if (lpSubcode[14] == 0xb1) {
				_sntprintf(str2, BufSize,
					_T("Point[%02x], NumberOfSkipIntervalPointers[%02x], NumberOfSkipTrackAssignmentsInPoint[%02x], "),
					lpSubcode[14], lpSubcode[19], lpSubcode[20]);
			}
			else if (lpSubcode[14] == 0xb2 || lpSubcode[14] == 0xb3 || lpSubcode[14] == 0xb4) {
				_sntprintf(str2, BufSize,
					_T("Point[%02x], TrackNumberToSkipUponPlayback[%02x %02x %02x %02x %02x %02x %02x], "),
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17],
					lpSubcode[18], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
			}
			else if (lpSubcode[14] == 0xc0) {
				_sntprintf(str2, BufSize,
					_T("Point[%02x], OptimumRecordingPower[%02x], StartTimeOfTheFirstLead-in[%02x:%02x:%02x], "),
					lpSubcode[14], lpSubcode[15], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
			}
			else if (lpSubcode[14] == 0xc1) {
				_sntprintf(str2, BufSize,
					_T("Point[%02x], CopyOfInfoFromA1Point[%02x %02x %02x %02x %02x %02x %02x], "),
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17],
					lpSubcode[18], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
			}
			else {
				_sntprintf(str2, BufSize,
					_T("Point[%02x], SkipIntervalStopTime[%02x:%02x:%02x], SkipIntervalStartTime[%02x:%02x:%02x], "),
					lpSubcode[14], lpSubcode[15], lpSubcode[16], lpSubcode[17],
					lpSubcode[19], lpSubcode[20], lpSubcode[21]);
			}
		}
		else if (lpSubcode[13] == 0xaa) {
			// lead-out area
			_sntprintf(str2, BufSize,
				_T("LeadOutAdr5, Track[%02u], Idx[%02x], StartTime[%02x:%02x:%02x], "),
				BcdToDec((BYTE)(lpSubcode[14] >> 4 & 0x0f)), lpSubcode[14] & 0x0f,
				lpSubcode[19], lpSubcode[20], lpSubcode[21]);
		}
		break;
	default:
		_sntprintf(str2, BufSize,
			_T("Track[%02x], Idx[%02x], RMSF[%02x:%02x:%02x], AMSF[%02x:%02x:%02x], "),
			lpSubcode[13], lpSubcode[14], lpSubcode[15], lpSubcode[16],
			lpSubcode[17], lpSubcode[19], lpSubcode[20], lpSubcode[21]);
		break;
	}

	SUB_R_TO_W scRW[4] = { 0 };
	BYTE tmpCode[24] = { 0 };
	_TCHAR str3[128] = { 0 };
	_tcsncat(str3, _T("RtoW["), 5);
	for (INT i = 0; i < 4; i++) {
		for (INT j = 0; j < 24; j++) {
			tmpCode[j] = (BYTE)(*(lpSubcodeRaw + (i * 24 + j)) & 0x3f);
		}
		memcpy(&scRW[i], tmpCode, sizeof(scRW[i]));
		switch (scRW[i].command) {
		case 0: // MODE 0, ITEM 0
			_tcsncat(str3, _T("0"), 1);
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
