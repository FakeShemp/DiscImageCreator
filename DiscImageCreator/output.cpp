/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"

CHAR szCatalog[META_CATALOG_SIZE+1];
CHAR szISRC[MAXIMUM_NUMBER_TRACKS][META_ISRC_SIZE+1];
CHAR szPerformer[META_STRING_SIZE+1];
CHAR szSongWriter[META_STRING_SIZE+1];
CHAR szAlbumTitle[META_STRING_SIZE+1];
CHAR szTitle[MAXIMUM_NUMBER_TRACKS][META_STRING_SIZE+1];

FILE* CreateOrOpenFileW(
	LPCTSTR pszSrcPath,
	LPTSTR pszOutPath,
	LPTSTR pszFileNameWithoutPath,
	LPCTSTR pszExt,
	LPCTSTR pszMode,
	INT nTrackNum,
	INT nMaxTrackNum
	)
{
	TCHAR szDstPath[_MAX_PATH];
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];

	ZeroMemory(szDstPath, sizeof(szDstPath));
	ZeroMemory(drive, sizeof(drive));
	ZeroMemory(dir, sizeof(dir));
	ZeroMemory(fname, sizeof(fname));
	ZeroMemory(ext, sizeof(ext));

	_tsplitpath(pszSrcPath, drive, dir, fname, ext);
	if(nMaxTrackNum <= 1) {
		_stprintf(szDstPath, _T("%s%s%s%s"), drive, dir, fname, pszExt);
	}
	else if(2 <= nMaxTrackNum && nMaxTrackNum <= 9) {
		_stprintf(szDstPath, 
			_T("%s%s%s (Track %d)%s"), drive, dir, fname, nTrackNum, pszExt);
	}
	else if(10 <= nMaxTrackNum) {
		_stprintf(szDstPath, 
			_T("%s%s%s (Track %02d)%s"), drive, dir, fname, nTrackNum, pszExt);
	}

	if(pszFileNameWithoutPath != NULL) {
		// size of pszFileNameWithoutPath must be _MAX_PATH.
		ZeroMemory(pszFileNameWithoutPath, _MAX_FNAME);
		_tsplitpath(szDstPath, drive, dir, fname, ext);
		_stprintf(pszFileNameWithoutPath, _T("%s%s"), fname, ext);
	}
	if(pszOutPath != NULL) {
		// size of pszOutPath must be _MAX_PATH.
		_tcsncpy(pszOutPath, szDstPath, _MAX_PATH);
	}
	return _tfopen(szDstPath, pszMode);
}

FILE* CreateOrOpenFileA(
	LPCSTR pszSrcPath,
	LPSTR pszOutPath,
	LPSTR pszFileNameWithoutPath,
	LPCSTR pszExt,
	LPCSTR pszMode,
	INT nTrackNum,
	INT nMaxTrackNum
	)
{
	CHAR szDstPath[_MAX_PATH];
	CHAR drive[_MAX_DRIVE];
	CHAR dir[_MAX_DIR];
	CHAR fname[_MAX_FNAME];
	CHAR ext[_MAX_EXT];

	ZeroMemory(szDstPath, sizeof(szDstPath));
	ZeroMemory(drive, sizeof(drive));
	ZeroMemory(dir, sizeof(dir));
	ZeroMemory(fname, sizeof(fname));
	ZeroMemory(ext, sizeof(ext));

	_splitpath(pszSrcPath, drive, dir, fname, ext);
	if(nMaxTrackNum <= 1) {
		sprintf(szDstPath, "%s%s%s%s", drive, dir, fname, pszExt);
	}
	else if(2 <= nMaxTrackNum && nMaxTrackNum <= 9) {
		sprintf(szDstPath, 
			"%s%s%s (Track %d)%s", drive, dir, fname, nTrackNum, pszExt);
	}
	else if(10 <= nMaxTrackNum) {
		sprintf(szDstPath, 
			"%s%s%s (Track %02d)%s", drive, dir, fname, nTrackNum, pszExt);
	}

	if(pszFileNameWithoutPath != NULL) {
		// size of pszFileNameWithoutPath must be _MAX_PATH.
		ZeroMemory(pszFileNameWithoutPath, _MAX_FNAME);
		_splitpath(szDstPath, drive, dir, fname, ext);
		sprintf(pszFileNameWithoutPath, "%s%s", fname, ext);
	}
	if(pszOutPath != NULL) {
		// size of pszOutPath must be _MAX_PATH.
		strncpy(pszOutPath, szDstPath, _MAX_PATH);
	}
	return fopen(szDstPath, pszMode);
}

void InitForOutput(
	void
	)
{
	ZeroMemory(szCatalog, sizeof(szCatalog));
	ZeroMemory(szISRC, sizeof(szISRC));
	ZeroMemory(szPerformer, sizeof(szPerformer));
	ZeroMemory(szSongWriter, sizeof(szSongWriter));
	ZeroMemory(szAlbumTitle, sizeof(szAlbumTitle));
	ZeroMemory(szTitle, sizeof(szTitle));
}

FILE* OpenProgrammabledFile(
	LPCTSTR pszFilename,
	LPCTSTR pszMode
	)
{
	FILE* fp = NULL;
	TCHAR dir[MAX_PATH];
	ZeroMemory(dir, sizeof(dir));
	::GetModuleFileName(NULL, dir, MAX_PATH);

	TCHAR* pdest = _tcsrchr(dir, '\\');
	if(pdest) {
		pdest[0] = NULL;
		TCHAR buf[MAX_PATH];
		ZeroMemory(buf, sizeof(buf));
		_stprintf(buf, _T("%s\\%s"), dir, pszFilename);
		fp = _tfopen(buf, pszMode);
	}
	return fp;
}

void OutputC2Error296(
	CONST PUCHAR pBuf,
	INT nLBA,
	FILE* fpLog
	)
{
	OutputLogStringA(fpLog, "c2 error LBA %d\n", nLBA);

	for(INT i = 0; i < CD_RAW_READ_C2_SIZE; i+=8) {
		OutputLogStringA(fpLog, 
			"\t%02X %02X %02X %02X %02X %02X %02X %02X\n", 
			pBuf[i], pBuf[i+1], pBuf[i+2], pBuf[i+3], pBuf[i+4], pBuf[i+5],
			pBuf[i+6], pBuf[i+7]);
	}
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputMain2352(
	CONST PUCHAR pBuf,
	INT nLBA,
	FILE* fpLog
	)
{
	OutputLogStringA(fpLog, "Main Channel LBA %d\n", nLBA);
	OutputLogStringA(fpLog, "\t    +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n");

	for(INT i = 0; i < CD_RAW_SECTOR_SIZE; i += 16) {
		OutputLogStringA(fpLog, 
			"\t%3X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n", 
			i, pBuf[i], pBuf[i+1], pBuf[i+2], pBuf[i+3], pBuf[i+4], pBuf[i+5],
			pBuf[i+6], pBuf[i+7], pBuf[i+8], pBuf[i+9], pBuf[i+10], pBuf[i+11], 
			pBuf[i+12], pBuf[i+13], pBuf[i+14], pBuf[i+15]);
	}
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputInquiryData(
	CONST PCHAR pInquiry,
	LPSTR pszVendorId,
	LPSTR pszProductId,
	FILE* fpLog
	)
{
	OutputLogStringA(fpLog, "Device Info\n");
	OutputLogStringA(fpLog, "\tDeviceType:");
	switch(pInquiry[0] & 0x1F) {
	case 5:
		OutputLogStringA(fpLog, "CD/DVD device\n");
		break;
	default:
		OutputLogStringA(fpLog, "Other device\n");
		break;
	}
	OutputLogStringA(fpLog, "\tDeviceTypeQualifier:%d\n", pInquiry[0] >> 5 & 0x3);

	OutputLogStringA(fpLog, "\tDeviceTypeModifier:%d\n", pInquiry[1] & 0x7F);
	OutputLogStringA(fpLog, "\tRemovableMedia:%s\n",
		(pInquiry[1] >> 7 & 0x1) == 0 ? "No" : "Yes");

	OutputLogStringA(fpLog, "\tVersions:%d\n", pInquiry[2]);

	OutputLogStringA(fpLog, "\tResponseDataFormat:%d\n", pInquiry[3] & 0x0F);
	OutputLogStringA(fpLog, "\tHiSupport:%s\n",
		(pInquiry[3] >> 4 & 0x1) == 0 ? "No" : "Yes");
	OutputLogStringA(fpLog, "\tNormACA:%s\n",
		(pInquiry[3] >> 5 & 0x1) == 0 ? "No" : "Yes");
	OutputLogStringA(fpLog, "\tTerminateTask:%s\n",
		(pInquiry[3] >> 6 & 0x1) == 0 ? "No" : "Yes");
	OutputLogStringA(fpLog, "\tAERC:%s\n",
		(pInquiry[3] >> 7 & 0x1) == 0 ? "No" : "Yes");

	OutputLogStringA(fpLog, "\tAdditionalLength:%d\n", pInquiry[4]);

	OutputLogStringA(fpLog, "\tMediumChanger:%s\n",
		(pInquiry[6] >> 3 & 0x1) == 0 ? "No" : "Yes");
	OutputLogStringA(fpLog, "\tMultiPort:%s\n",
		(pInquiry[6] >> 4 & 0x1) == 0 ? "No" : "Yes");
	OutputLogStringA(fpLog, "\tEnclosureServices:%s\n",
		(pInquiry[6] >> 6 & 0x1) == 0 ? "No" : "Yes");

	OutputLogStringA(fpLog, "\tSoftReset:%s\n",
		(pInquiry[7] & 0x1) == 0 ? "No" : "Yes");
	OutputLogStringA(fpLog, "\tCommandQueue:%s\n",
		(pInquiry[7] >> 1 & 0x1) == 0 ? "No" : "Yes");
	OutputLogStringA(fpLog, "\tLinkedCommands:%s\n",
		(pInquiry[7] >> 3 & 0x1) == 0 ? "No" : "Yes");
	OutputLogStringA(fpLog, "\tRelativeAddressing:%s\n",
		(pInquiry[7] >> 7 & 0x1) == 0 ? "No" : "Yes");

	OutputLogStringA(fpLog, "\tVendorId:");
	INT i = 8;
	while(i < 16) {
		OutputLogStringA(fpLog, "%c", pInquiry[i]);
		pszVendorId[i-8] = pInquiry[i++];
	}
	OutputLogStringA(fpLog, "\n");

	OutputLogStringA(fpLog, "\tProductId:"); 
	while(i < 32) {
		OutputLogStringA(fpLog, "%c", pInquiry[i]);
		pszProductId[i-16] = pInquiry[i++];
	}
	OutputLogStringA(fpLog, "\n");

	OutputLogStringA(fpLog, "\tProductRevisionLevel:"); 
	while(i < 36) {
		OutputLogStringA(fpLog, "%c", pInquiry[i++]);
	}
	OutputLogStringA(fpLog, "\n");

	OutputLogStringA(fpLog, "\tVendorSpecific:"); 
	while(i < 56) {
		OutputLogStringA(fpLog, "%c", pInquiry[i++]);
	}
	OutputLogStringA(fpLog, "\n");
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputDVDStructureFormat(
	CONST PUCHAR pFormat, 
	size_t i,
	CONST PUCHAR pStructure,
	CONST PUSHORT pStructureLength,
	PINT nDVDSectorSize,
	FILE* fpLog
	)
{
	switch(pFormat[i]) {
	case DvdPhysicalDescriptor:
	{
		OutputLogStringA(fpLog, "\tPhysicalFormatInformation\n");
		OutputLogStringA(fpLog, "\t\tBookVersion:%d\n", pStructure[4] & 0x0F);
		LPCSTR lpBookType[] = {
			"DVD-ROM", "DVD-RAM", "DVD-R", "DVD-RW", "HD DVD-ROM", "HD DVD-RAM",
			"HD DVD-R", "Reserved", "Reserved",	"DVD+RW", "DVD+R", "Reserved",
			"Reserved", "DVD+RW DL", "DVD+R DL", "Reserved"
		};
		OutputLogStringA(fpLog, "\t\tBookType:%s\n", lpBookType[pStructure[4]>>4&0x0F]);

		LPCSTR lpMaximumRate[] = {
			"2.52Mbps", "5.04Mbps", "10.08Mbps", "20.16Mbps", "30.24Mbps", "Reserved",
			"Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
			"Reserved", "Reserved", "Not Specified"
		};
		OutputLogStringA(fpLog, "\t\tMinimumRate:%s\n", lpMaximumRate[pStructure[5]&0x0F]);
		OutputLogStringA(fpLog, "\t\tDiskSize:%s\n", (pStructure[5] & 0xF0) == 0 ? "120mm" : "80mm");

		LPCSTR lpLayerType[] = {
			"Layer contains embossed data", "Layer contains recordable data", 
			"Layer contains rewritable data", "Reserved"
		};
		OutputLogStringA(fpLog, "\t\tLayerType:%s\n", lpLayerType[pStructure[6]&0x0F]);
		OutputLogStringA(fpLog, "\t\tTrackPath:%s\n", 
			(pStructure[6] & 0x10) == 0 ? "Parallel Track Path" : "Opposite Track Path");
		OutputLogStringA(fpLog, "\t\tNumberOfLayers:%s\n", 
			(pStructure[6] & 0x60) == 0 ? "Single Layer" : "Double Layer");

		LPCSTR lpTrackDensity[] = {
			"0.74μm/track", "0.80μm/track", "0.615μm/track", "0.40μm/track",
			"0.34μm/track", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
			"Reserved", "Reserved", "Reserved", "Reserved", "Reserved",	"Reserved"
		};
		OutputLogStringA(fpLog, "\t\tTrackDensity:%s\n", lpTrackDensity[pStructure[7]&0x0F]);

		LPCSTR lpLinearDensity[] = {
			"0.267μm/bit", "0.293μm/bit", "0.409 to 0.435μm/bit", "Reserved",
			"0.280 to 0.291μm/bit", "0.153μm/bit", "0.130 to 0.140μm/bit",
			"Reserved", "0.353μm/bit", "Reserved", "Reserved", "Reserved",
			"Reserved", "Reserved", "Reserved"
		};
		OutputLogStringA(fpLog, "\t\tLinearDensity:%s\n", lpLinearDensity[pStructure[7]>>4&0x0F]);

		LONG ulStartSectorNum = MAKELONG(MAKEWORD(pStructure[11], 
			pStructure[10]), MAKEWORD(pStructure[9], pStructure[8]));
		OutputLogStringA(fpLog, "\t\tStartDataSector:%d(0x%X)\n", 
			ulStartSectorNum, ulStartSectorNum);
		LONG ulEndSectorNum = MAKELONG(MAKEWORD(pStructure[15], 
			pStructure[14]), MAKEWORD(pStructure[13], pStructure[12]));
		OutputLogStringA(fpLog, "\t\tEndDataSector:%d(0x%X)\n", 
			ulEndSectorNum, ulEndSectorNum);
		*nDVDSectorSize = ulEndSectorNum - ulStartSectorNum + 1;
		INT ulEndSectorLayer0 = MAKELONG(MAKEWORD(pStructure[19], 
			pStructure[18]), MAKEWORD(pStructure[17], pStructure[16]));
		OutputLogStringA(fpLog, "\t\tEndLayerZeroSector:%X\n", ulEndSectorLayer0);

		OutputLogStringA(fpLog, "\t\tBCAFlag:%s\n", 
			(pStructure[20] & 0x80) == 0 ? "None" : "Exist");

		OutputLogStringA(fpLog, "\t\tMediaSpecific:");
		for(ULONG k = 0; k < 2031; k++) {
			OutputLogStringA(fpLog, "%02X", pStructure[21+k]);
		}
		OutputLogStringA(fpLog, "\n");
		break;
	}
	case DvdCopyrightDescriptor:
		OutputLogStringA(fpLog, "\tCopyrightProtectionType:");
		switch(pStructure[4]) {
		case 0:
			OutputLogStringA(fpLog, "No\n");
			break;
		case 1:
			OutputLogStringA(fpLog, "CSS/CPPM\n");
			break;
		case 2:
			OutputLogStringA(fpLog, "CPRM\n");
			break;
		case 3:
			OutputLogStringA(fpLog, "AACS with HD DVD content\n");
			break;
		case 10:
			OutputLogStringA(fpLog, "AACS with BD content\n");
			break;
		default:
			OutputLogStringA(fpLog, "Unknown:[%02X]\n", pStructure[4]);
			break;
		}
		OutputLogStringA(fpLog, 
			"\tRegionManagementInformation:%02X\n", pStructure[5]);
		break;
	case DvdDiskKeyDescriptor:
		OutputLogStringA(fpLog, "\tDiskKeyData:");
		for(ULONG k = 0; k < 2048; k++) {
			OutputLogStringA(fpLog, "%02X", pStructure[4+k]);
		}
		OutputLogStringA(fpLog, "\n");
		break;
	case DvdBCADescriptor:
		OutputLogStringA(fpLog, "\tBCAInformation:");
		for(ULONG k = 0; 
			k < pStructureLength[i] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
			OutputLogStringA(fpLog, "%02X", pStructure[4+k]);
		}
		OutputLogStringA(fpLog, "\n");
		break;
	case DvdManufacturerDescriptor:
		OutputLogStringA(fpLog, "\tManufacturingInformation:");
		for(ULONG k = 0; k < 2048; k++) {
			OutputLogStringA(fpLog, "%02X", pStructure[4+k]);
		}
		OutputLogStringA(fpLog, "\n");
		break;
	case 6:
		OutputLogStringA(fpLog, "\tmedia ID:");
		for(ULONG k = 0; 
			k < pStructureLength[i] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
			OutputLogStringA(fpLog, "%02X", pStructure[4+k]);
		}
		OutputLogStringA(fpLog, "\n");
		break;
	case 7:
		OutputLogStringA(fpLog, "\tMedia Key Block Total Packs %d", pStructure[3]);
		OutputLogStringA(fpLog, "\tmedia key block:");
		for(ULONG k = 0; 
			k < pStructureLength[i] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
			OutputLogStringA(fpLog, "%02X", pStructure[4+k]);
		}
		OutputLogStringA(fpLog, "\n");
		break;
	default:
		OutputLogStringA(fpLog, "Unknown:[%02X]\n", pFormat[i]);
		break;
	}
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputFeatureNumber(
	CONST PUCHAR pConf,
	LONG lAllLen,
	size_t uiSize,
	PBOOL bCanCDText,
	PBOOL bC2ErrorData,
	FILE* fpLog
	)
{
	LONG n = 0;
	LONG lVal = 0;
	WORD wVal = 0;
	while(n < lAllLen - (LONG)uiSize) { 
		WORD nCode = MAKEWORD(pConf[uiSize+1+n], pConf[uiSize+0+n]);
		switch(nCode) {
		case FeatureProfileList:
			OutputLogStringA(fpLog, "\tFeatureProfileList\n");
			OutputLogStringA(fpLog, "\t\t");
			while(n < pConf[uiSize+3]) {
				OutputFeatureProfileType(fpLog, 
					MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n]));
				OutputLogStringA(fpLog, ", ");
				n += sizeof(FEATURE_DATA_PROFILE_LIST_EX);
			}
			OutputLogStringA(fpLog, "\n");
			n += sizeof(FEATURE_HEADER);
			break;
		case FeatureCore: {
			OutputLogStringA(fpLog, "\tFeatureCore\n");
			OutputLogStringA(fpLog, "\t\tPhysicalInterface:");
			lVal = MAKELONG(MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]), 
				MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n]));
			switch(lVal) {
			case 0:
				OutputLogStringA(fpLog, "Unspecified\n");
				break;
			case 1:
				OutputLogStringA(fpLog, "SCSI Family\n");
				break;
			case 2:
				OutputLogStringA(fpLog, "ATAPI\n");
				break;
			case 3:
				OutputLogStringA(fpLog, "IEEE 1394 - 1995\n");
				break;
			case 4:
				OutputLogStringA(fpLog, "IEEE 1394A\n");
				break;
			case 5:
				OutputLogStringA(fpLog, "Fibre Channel\n");
				break;
			case 6:
				OutputLogStringA(fpLog, "IEEE 1394B\n");
				break;
			case 7:
				OutputLogStringA(fpLog, "Serial ATAPI\n");
				break;
			case 8:
				OutputLogStringA(fpLog, "USB (both 1.1 and 2.0)\n");
				break;
			case 0xFFFF:
				OutputLogStringA(fpLog, "Vendor Unique\n");
				break;
			default:
				OutputLogStringA(fpLog, "Reserved:[%08d]\n", lVal);
				break;
			}
			OutputLogStringA(fpLog, "\t\tDeviceBusyEvent:%s\n", 
				(pConf[uiSize+8+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tINQUIRY2:%s\n", 
				(pConf[uiSize+8+n] & 0x02) == 0x02 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		}
		case FeatureMorphing:
			OutputLogStringA(fpLog, "\tFeatureMorphing\n");
			OutputLogStringA(fpLog, "\t\tAsynchronous:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tOCEvent:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureRemovableMedium:
			OutputLogStringA(fpLog, "\tFeatureRemovableMedium\n");
			OutputLogStringA(fpLog, "\t\tLockable:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tDBML:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tDefaultToPrevent:%s\n", 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tEject:%s\n", 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tLoad:%s\n", 
				(pConf[uiSize+4+n] & 0x10) == 0x10 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tLoadingMechanism:"); 
			switch(pConf[uiSize+4+n] >> 5 & 0x07) {
			case 0:
				OutputLogStringA(fpLog, "Caddy/Slot type loading mechanism\n");
				break;
			case 1:
				OutputLogStringA(fpLog, "Tray type loading mechanism\n");
				break;
			case 2:
				OutputLogStringA(fpLog, "Pop-up type loading mechanism\n");
				break;
			case 4:
				OutputLogStringA(fpLog, 
					"Embedded changer with individually changeable discs\n");
				break;
			case 5:
				OutputLogStringA(fpLog, 
					"Embedded changer using a magazine mechanism\n");
				break;
			default:
				OutputLogStringA(fpLog, 
					"Reserved:[%08d]\n", pConf[uiSize+4+n] >> 5 & 0x07);
				break;
			}
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureWriteProtect:
			OutputLogStringA(fpLog, "\tFeatureWriteProtect\n");
			OutputLogStringA(fpLog, "\t\tSupportsSWPPBit:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tSupportsPersistentWriteProtect:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tWriteInhibitDCB:%s\n", 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tDiscWriteProtectPAC:%s\n", 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureRandomReadable:
			OutputLogStringA(fpLog, "\tFeatureRandomReadable\n");
			lVal = MAKELONG(MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]), 
				MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n]));
			OutputLogStringA(fpLog, "\t\tLogicalBlockSize:%d\n", lVal);
			wVal = MAKEWORD(pConf[uiSize+9+n], pConf[uiSize+8+n]);
			OutputLogStringA(fpLog, "\t\tBlocking:%d\n", wVal);
			OutputLogStringA(fpLog, "\t\tErrorRecoveryPagePresent:%s\n", 
				(pConf[uiSize+10+n] & 0x01) == 0x01 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureMultiRead:
			OutputLogStringA(fpLog, "\tFeatureMultiRead\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureCdRead:
			OutputLogStringA(fpLog, "\tFeatureCdRead\n");
			OutputLogStringA(fpLog, "\t\tCDText:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			*bCanCDText = pConf[uiSize+4+n] & 0x01;
			OutputLogStringA(fpLog, "\t\tC2ErrorData:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			*bC2ErrorData = (pConf[uiSize+4+n] & 0x02) >> 1;
			OutputLogStringA(fpLog, "\t\tDigitalAudioPlay:%s\n", 
				(pConf[uiSize+4+n] & 0x80) == 0x80 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdRead:
			OutputLogStringA(fpLog, "\tFeatureDvdRead\n");
			OutputLogStringA(fpLog, "\t\tMulti110:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tDualDashR:%s\n", 
				(pConf[uiSize+6+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tDualDashRW:%s\n", 
				(pConf[uiSize+6+n] & 0x02) == 0x02 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureRandomWritable:
			OutputLogStringA(fpLog, "\tFeatureRandomWritable\n");
			lVal = MAKELONG(MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]), 
				MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n]));
			OutputLogStringA(fpLog, "\t\tLastLBA:%d\n", lVal);
			lVal = MAKELONG(MAKEWORD(pConf[uiSize+11+n], pConf[uiSize+10+n]), 
				MAKEWORD(pConf[uiSize+9+n], pConf[uiSize+8+n]));
			OutputLogStringA(fpLog, "\t\tLogicalBlockSize:%d\n", lVal);
			wVal = MAKEWORD(pConf[uiSize+13+n], pConf[uiSize+12+n]);
			OutputLogStringA(fpLog, "\t\tBlocking:%d\n", wVal);
			OutputLogStringA(fpLog, "\t\tErrorRecoveryPagePresent:%s\n", 
				(pConf[uiSize+14+n] & 0x01) == 0x01 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureIncrementalStreamingWritable:
			OutputLogStringA(fpLog, "\tFeatureIncrementalStreamingWritable\n");
			wVal = MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n]);
			OutputLogStringA(fpLog, 
				"\t\tDataTypeSupported:[%s]\n", wVal >= 1 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tBufferUnderrunFree:%s\n", 
				(pConf[uiSize+6+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tAddressModeReservation:%s\n", 
				(pConf[uiSize+6+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tTrackRessourceInformation:%s\n", 
				(pConf[uiSize+6+n] & 0x04) == 0x04 ? "Yes" : "No");
			OutputLogStringA(fpLog, 
				"\t\tNumberOfLinkSizes:%d\n", pConf[uiSize+7+n]);
			for(INT i = 1; i <= pConf[uiSize+7+n]; i++) {
				OutputLogStringA(fpLog, 
					"\t\tLinkSize%d:%d\n", i, pConf[uiSize+7+i+n]);
			}
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureSectorErasable:
			OutputLogStringA(fpLog, "\tFeatureSectorErasable\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureFormattable:
			OutputLogStringA(fpLog, "\tFeatureFormattable\n");
			OutputLogStringA(fpLog, "\t\tFullCertification:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tQuickCertification:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tSpareAreaExpansion:%s\n", 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tRENoSpareAllocated:%s\n", 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tRRandomWritable:%s\n", 
				(pConf[uiSize+8+n] & 0x01) == 0x01 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDefectManagement:
			OutputLogStringA(fpLog, "\tFeatureDefectManagement\n");
			OutputLogStringA(fpLog, "\t\tSupplimentalSpareArea:%s\n", 
				(pConf[uiSize+4+n] & 0x80) == 0x80 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureWriteOnce:
			OutputLogStringA(fpLog, "\tFeatureWriteOnce\n");
			lVal = MAKELONG(MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]), 
				MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n]));
			OutputLogStringA(fpLog, "\t\tLogicalBlockSize:%d\n", lVal);
			wVal = MAKEWORD(pConf[uiSize+9+n], pConf[uiSize+8+n]);
			OutputLogStringA(fpLog, "\t\tBlocking:%d\n", wVal);
			OutputLogStringA(fpLog, "\t\tErrorRecoveryPagePresent:%s\n", 
				(pConf[uiSize+10+n] & 0x01) == 0x01 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureRestrictedOverwrite:
			OutputLogStringA(fpLog, "\tFeatureRestrictedOverwrite\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureCdrwCAVWrite:
			OutputLogStringA(fpLog, "\tFeatureCdrwCAVWrite\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureMrw:
			OutputLogStringA(fpLog, "\tFeatureMrw\n");
			OutputLogStringA(fpLog, "\t\tWrite:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tDvdPlusRead:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tDvdPlusWrite:%s\n", 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureEnhancedDefectReporting:
			OutputLogStringA(fpLog, "\tFeatureEnhancedDefectReporting\n");
			OutputLogStringA(fpLog, "\t\tDRTDMSupported:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogStringA(fpLog, 
				"\t\tNumberOfDBICacheZones:%d\n", pConf[uiSize+5+n]);
			wVal = MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]);
			OutputLogStringA(fpLog, "\t\tNumberOfEntries:%d\n", wVal);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdPlusRW:
			OutputLogStringA(fpLog, "\tFeatureDvdPlusRW\n");
			OutputLogStringA(fpLog, "\t\tWrite:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tCloseOnly:%s\n", 
				(pConf[uiSize+5+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tQuickStart:%s\n", 
				(pConf[uiSize+5+n] & 0x02) == 0x02 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdPlusR:
			OutputLogStringA(fpLog, "\tFeatureDvdPlusR\n");
			OutputLogStringA(fpLog, "\t\tWrite:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureRigidRestrictedOverwrite:
			OutputLogStringA(fpLog, "\tFeatureRigidRestrictedOverwrite\n");
			OutputLogStringA(fpLog, "\t\tBlank:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tIntermediate:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tDefectStatusDataRead:%s\n", 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tDefectStatusDataGenerate:%s\n", 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureCdTrackAtOnce:
			OutputLogStringA(fpLog, "\tFeatureCdTrackAtOnce\n");
			OutputLogStringA(fpLog, "\t\tRWSubchannelsRecordable:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tCdRewritable:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tTestWriteOk:%s\n", 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tRWSubchannelPackedOk:%s\n", 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tRWSubchannelRawOk:%s\n", 
				(pConf[uiSize+4+n] & 0x10) == 0x10 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tBufferUnderrunFree:%s\n", 
				(pConf[uiSize+4+n] & 0x40) == 0x40 ? "Yes" : "No");
			wVal = MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]);
			OutputLogStringA(fpLog, "\t\tDataTypeSupported:[%s]\n", 
				wVal >= 1 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureCdMastering:
			OutputLogStringA(fpLog, "\tFeatureCdMastering\n");
			OutputLogStringA(fpLog, "\t\tRWSubchannelsRecordable:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tCdRewritable:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tTestWriteOk:%s\n", 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tRRawRecordingOk:%s\n", 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tRawMultiSessionOk:%s\n", 
				(pConf[uiSize+4+n] & 0x10) == 0x10 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tSessionAtOnceOk:%s\n", 
				(pConf[uiSize+4+n] & 0x20) == 0x20 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tBufferUnderrunFree:%s\n", 
				(pConf[uiSize+4+n] & 0x40) == 0x40 ? "Yes" : "No");
			lVal = MAKELONG(MAKEWORD(0, pConf[uiSize+7+n]), 
				MAKEWORD(pConf[uiSize+6+n], pConf[uiSize+5+n]));
			OutputLogStringA(fpLog, "\t\tMaximumCueSheetLength:%d\n", lVal);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdRecordableWrite:
			OutputLogStringA(fpLog, "\tFeatureDvdRecordableWrite\n");
			OutputLogStringA(fpLog, "\t\tDVD_RW:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tTestWrite:%s\n", 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tRDualLayer:%s\n", 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tBufferUnderrunFree:%s\n", 
				(pConf[uiSize+4+n] & 0x40) == 0x40 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureLayerJumpRecording:
			OutputLogStringA(fpLog, "\tFeatureLayerJumpRecording\n");
			OutputLogStringA(fpLog, 
				"\t\tNumberOfLinkSizes:%d\n", pConf[uiSize+7+n]);
			for(INT i = 1; i <= pConf[uiSize+7+n]; i++) {
				OutputLogStringA(fpLog, 
					"\t\tLinkSize%d:%d\n", i, pConf[uiSize+7+i+n]);
			}
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureCDRWMediaWriteSupport:
			OutputLogStringA(fpLog, "\tFeatureCDRWMediaWriteSupport\n");
			OutputLogStringA(fpLog, "\t\tSubtype0:%s\n", 
				(pConf[uiSize+5+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tSubtype1:%s\n", 
				(pConf[uiSize+5+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tSubtype2:%s\n", 
				(pConf[uiSize+5+n] & 0x04) == 0x04 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tSubtype3:%s\n", 
				(pConf[uiSize+5+n] & 0x08) == 0x08 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tSubtype4:%s\n", 
				(pConf[uiSize+5+n] & 0x10) == 0x10 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tSubtype5:%s\n", 
				(pConf[uiSize+5+n] & 0x20) == 0x20 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tSubtype6:%s\n", 
				(pConf[uiSize+5+n] & 0x40) == 0x40 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tSubtype7:%s\n", 
				(pConf[uiSize+5+n] & 0x80) == 0x80 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureBDRPseudoOverwrite:
			OutputLogStringA(fpLog, "\tFeatureBDRPseudoOverwrite\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdPlusRWDualLayer:
			OutputLogStringA(fpLog, "\tFeatureDvdPlusRWDualLayer\n");
			OutputLogStringA(fpLog, "\t\tWrite:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tCloseOnly:%s\n", 
				(pConf[uiSize+5+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tQuickStart:%s\n", 
				(pConf[uiSize+5+n] & 0x02) == 0x02 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdPlusRDualLayer:
			OutputLogStringA(fpLog, "\tFeatureDvdPlusRDualLayer\n");
			OutputLogStringA(fpLog, "\t\tWrite:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureBDRead:
			OutputLogStringA(fpLog, "\tFeatureBDRead\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureBDWrite:
			OutputLogStringA(fpLog, "\tFeatureBDWrite\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureTSR:
			OutputLogStringA(fpLog, "\tFeatureTSR\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureHDDVDRead:
			OutputLogStringA(fpLog, "\tFeatureHDDVDRead\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureHDDVDWrite:
			OutputLogStringA(fpLog, "\tFeatureHDDVDWrite\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureHybridDisc:
			OutputLogStringA(fpLog, "\tFeatureHybridDisc\n");
			OutputLogStringA(fpLog, "\t\tResetImmunity:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeaturePowerManagement:
			OutputLogStringA(fpLog, "\tFeaturePowerManagement\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureSMART:
			OutputLogStringA(fpLog, "\tFeatureSMART\n");
			OutputLogStringA(fpLog, 
				"\t\tFaultFailureReportingPagePresent:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureEmbeddedChanger:
			OutputLogStringA(fpLog, "\tFeatureEmbeddedChanger\n");
			OutputLogStringA(fpLog, "\t\tSupportsDiscPresent:%s\n", 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tSideChangeCapable:%s\n", 
				(pConf[uiSize+4+n] & 0x10) == 0x10 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tHighestSlotNumber [%d]\n", 
				pConf[uiSize+7+n] & 0x1F);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureCDAudioAnalogPlay:
			OutputLogStringA(fpLog, "\tFeatureCDAudioAnalogPlay\n");
			OutputLogStringA(fpLog, "\t\tSeperateVolume:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tSeperateChannelMute:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tScanSupported:%s\n", 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? "Yes" : "No");
			wVal = MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]);
			OutputLogStringA(fpLog, "\t\tNumerOfVolumeLevels:%d\n", wVal);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureMicrocodeUpgrade:
			OutputLogStringA(fpLog, "\tFeatureMicrocodeUpgrade\n");
			OutputLogStringA(fpLog, "\t\tM5:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureTimeout:
			OutputLogStringA(fpLog, "\tFeatureTimeout\n");
			OutputLogStringA(fpLog, "\t\tGroup3:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			wVal = MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]);
			OutputLogStringA(fpLog, "\t\tUnitLength:%d\n", wVal);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdCSS:
			OutputLogStringA(fpLog, "\tFeatureDvdCSS\n");
			OutputLogStringA(fpLog, "\t\tCssVersion:%d\n", pConf[uiSize+7+n]);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureRealTimeStreaming:
			OutputLogStringA(fpLog, "\tFeatureRealTimeStreaming\n");
			OutputLogStringA(fpLog, "\t\tStreamRecording:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tWriteSpeedInGetPerf:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tWriteSpeedInMP2A:%s\n", 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tSetCDSpeed:%s\n", 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tReadBufferCapacityBlock:%s\n", 
				(pConf[uiSize+4+n] & 0x10) == 0x10 ? "Yes" : "No");
			OutputLogStringA(fpLog, "\t\tSetMinimumPerformance:%s\n", 
				(pConf[uiSize+4+n] & 0x20) == 0x20 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureLogicalUnitSerialNumber:
			OutputLogStringA(fpLog, "\tFeatureLogicalUnitSerialNumber\n");
			OutputLogStringA(fpLog, "\t\tSerialNumber:");
			for(INT i = 0; i < pConf[uiSize+3+n]; i++) {
				OutputLogStringA(fpLog, "%c", pConf[uiSize+4+i+n]);
			}
			OutputLogStringA(fpLog, "\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureMediaSerialNumber:
			OutputLogStringA(fpLog, "\tFeatureMediaSerialNumber\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDiscControlBlocks:
			OutputLogStringA(fpLog, "\tFeatureDiscControlBlocks\n");
			for(INT i = 0; i < pConf[uiSize+3+n]; i+=4) {
				OutputLogStringA(fpLog, "\t\tContentDescriptor %02d:", i/4);
				lVal = MAKELONG(MAKEWORD(pConf[uiSize+7+i+n], pConf[uiSize+6+i+n]), 
					MAKEWORD(pConf[uiSize+5+i+n], pConf[uiSize+4+i+n]));
				OutputLogStringA(fpLog, "%08d\n", lVal);
			}
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdCPRM:
			OutputLogStringA(fpLog, "\tFeatureDvdCPRM\n");
			OutputLogStringA(fpLog, "\t\tCPRMVersion:%d\n", pConf[uiSize+7+n]);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureFirmwareDate:
			OutputLogStringA(fpLog, "\tFeatureFirmwareDate\n");
			lVal = MAKELONG(MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]), 
				MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n]));
			OutputLogStringA(fpLog, "\t\tYear:%d\n", lVal);
			wVal = MAKEWORD(pConf[uiSize+9+n], pConf[uiSize+8+n]);
			OutputLogStringA(fpLog, "\t\tMonth:%d\n", wVal);
			wVal = MAKEWORD(pConf[uiSize+11+n], pConf[uiSize+10+n]);
			OutputLogStringA(fpLog, "\t\tDay:%d\n", wVal);
			wVal = MAKEWORD(pConf[uiSize+13+n], pConf[uiSize+12+n]);
			OutputLogStringA(fpLog, "\t\tHour:%d\n", wVal);
			wVal = MAKEWORD(pConf[uiSize+15+n], pConf[uiSize+14+n]);
			OutputLogStringA(fpLog, "\t\tMinute:%d\n", wVal);
			wVal = MAKEWORD(pConf[uiSize+17+n], pConf[uiSize+16+n]);
			OutputLogStringA(fpLog, "\t\tSeconds:%d\n", wVal);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureAACS:
			OutputLogStringA(fpLog, "\tFeatureAACS\n");
			OutputLogStringA(fpLog, "\t\tBindingNonceGeneration:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogStringA(fpLog, 
				"\t\tBindingNonceBlockCount:%d\n", pConf[uiSize+5+n]);
			OutputLogStringA(fpLog, 
				"\t\tNumberOfAGIDs:%d\n", pConf[uiSize+6+n] & 0x0F);
			OutputLogStringA(fpLog, "\t\tAACSVersion:%d\n", pConf[uiSize+7+n]);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureVCPS:
			OutputLogStringA(fpLog, "\tFeatureVCPS\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		default:
			if(0xff00 <= nCode && nCode <= 0xffff) {
				OutputLogStringA(fpLog, 
					"\tVendor Specific. FeatureCode[0x%04X]\n", nCode);
				OutputLogStringA(fpLog, "\t\tVendorSpecificData:[");
			}
			else {
				OutputLogStringA(fpLog, 
					"\tReserved. FeatureCode[0x%04X]\n", nCode);
				OutputLogStringA(fpLog, "\t\tData:[");
			}
			for(INT i = 0; i < pConf[uiSize+3+n]; i++) {
				OutputLogStringA(fpLog, "%02X ", pConf[uiSize+4+i+n]);
			}
			OutputLogStringA(fpLog, "]\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		}
	}
}

void OutputFeatureProfileType(
	FILE* fpLog,
	USHORT usFeatureProfileType
	)
{
	switch(usFeatureProfileType) {
		case ProfileInvalid:
			OutputLogStringA(fpLog, "Invalid");
			break;
		case ProfileNonRemovableDisk:
			OutputLogStringA(fpLog, "NonRemovableDisk");
			break;
		case ProfileRemovableDisk:
			OutputLogStringA(fpLog, "RemovableDisk");
			break;
		case ProfileMOErasable:
			OutputLogStringA(fpLog, "MOErasable");
			break;
		case ProfileMOWriteOnce:
			OutputLogStringA(fpLog, "MOWriteOnce");
			break;
		case ProfileAS_MO:
			OutputLogStringA(fpLog, "AS_MO");
			break;
		case ProfileCdrom:
			OutputLogStringA(fpLog, "CD-ROM");
			break;
		case ProfileCdRecordable:
			OutputLogStringA(fpLog, "CD-R");
			break;
		case ProfileCdRewritable:
			OutputLogStringA(fpLog, "CD-RW");
			break;
		case ProfileDvdRom:
			OutputLogStringA(fpLog, "DVD-ROM");
			break;
		case ProfileDvdRecordable:
			OutputLogStringA(fpLog, "DVD-R");
			break;
		case ProfileDvdRam:
			OutputLogStringA(fpLog, "DVD-RAM");
			break;
		case ProfileDvdRewritable:
			OutputLogStringA(fpLog, "DVD-RW");
			break;
		case ProfileDvdRWSequential:
			OutputLogStringA(fpLog, "DVD-RW Sequential");
			break;
		case ProfileDvdDashRDualLayer:
			OutputLogStringA(fpLog, "DVD-R DL");
			break;
		case ProfileDvdDashRLayerJump:
			OutputLogStringA(fpLog, "DVD-R LayerJump");
			break;
		case ProfileDvdPlusRW:
			OutputLogStringA(fpLog, "DVD+RW");
			break;
		case ProfileDvdPlusR:
			OutputLogStringA(fpLog, "DVD+R");
			break;
		case ProfileDvdPlusRWDualLayer:
			OutputLogStringA(fpLog, "DVD+RW DL");
			break;
		case ProfileDvdPlusRDualLayer:
			OutputLogStringA(fpLog, "DVD+R DL");
			break;
		case ProfileBDRom:
			OutputLogStringA(fpLog, "BD-ROM");
			break;
		case ProfileBDRSequentialWritable:
			OutputLogStringA(fpLog, "BD-SW");
			break;
		case ProfileBDRRandomWritable:
			OutputLogStringA(fpLog, "BD-RW");
			break;
		case ProfileBDRewritable:
			OutputLogStringA(fpLog, "BD-R");
			break;
		case ProfileHDDVDRom:
			OutputLogStringA(fpLog, "HD DVD");
			break;
		case ProfileHDDVDRecordable:
			OutputLogStringA(fpLog, "HD DVD-R");
			break;
		case ProfileHDDVDRam:
			OutputLogStringA(fpLog, "HD DVD-RAM");
			break;
		case ProfileHDDVDRewritable:
			OutputLogStringA(fpLog, "HD DVD-RW");
			break;
		case ProfileHDDVDRDualLayer:
			OutputLogStringA(fpLog, "HD DVD-R DL");
			break;
		case ProfileHDDVDRWDualLayer:
			OutputLogStringA(fpLog, "HD DVD-RW DL");
			break;
		case ProfileNonStandard:
			OutputLogStringA(fpLog, "NonStandard");
			break;
		default:
			OutputLogStringA(fpLog, "Reserved [%X]", usFeatureProfileType);
			break;
	}
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputParsingSubfile(
	LPCTSTR pszSubfile
	)
{
	FILE* fpSub = CreateOrOpenFileW(pszSubfile, NULL, NULL, _T(".sub"), _T("rb"), 0, 0);
	FILE* fpParse = CreateOrOpenFileW(pszSubfile, NULL, NULL, _T(".sub.txt"), _T("w"), 0, 0);
	if (!fpSub || !fpParse) {
		OutputErrorStringA("Failed to open file .sub [F:%s][L:%d]", 
			__FUNCTION__, __LINE__);
		return;
	}
	ULONG datasize = GetFilesize(fpSub, 0);
	PUCHAR data = (PUCHAR)malloc(datasize);
	if(!data) {
		OutputErrorStringA("Cannot alloc memory [F:%s][L:%d]\n", 
			__FUNCTION__, __LINE__);
		return;
	}
	fread(data, sizeof(char), datasize, fpSub);
	FcloseAndNull(fpSub);

	// TODO:RtoW don't use in present
	UCHAR SubcodeRtoW[CD_RAW_READ_SUBCODE_SIZE];
	ZeroMemory(SubcodeRtoW, sizeof(SubcodeRtoW));
	UCHAR byTrackNum = 1;
	for(INT i = 0, j = 0; i < (INT)datasize; i+=CD_RAW_READ_SUBCODE_SIZE, j++) {
		UCHAR byAdr = (UCHAR)(data[i+12] & 0x0F);
		if(byAdr == ADR_ENCODES_MEDIA_CATALOG) {
			SetMCNToString(&data[i], szCatalog, FALSE);
			szCatalog[13] = '\0';
		}
		else if(byAdr == ADR_ENCODES_ISRC) {
			SetISRCToString(&data[i], byTrackNum, szISRC[byTrackNum-1], FALSE);
			szISRC[byTrackNum-1][12] = '\0';
		}
		else {
			byTrackNum = BcdToDec(data[i+13]);
		}
		OutputSubcode(j, byTrackNum, &data[i], SubcodeRtoW, fpParse);
		printf("\rParse sub(Size) %6d/%6d", i, datasize);
	}
	printf("\n");
	FcloseAndNull(fpParse);
}

#ifdef WIN64
	INT64 aScsiStatus[][2] = {
		{SCSISTAT_GOOD, (INT64)"GOOD"}, 
		{SCSISTAT_CHECK_CONDITION, (INT64)"CHECK_CONDITION"}, 
		{SCSISTAT_CONDITION_MET, (INT64)"CONDITION_MET"}, 
		{SCSISTAT_BUSY, (INT64)"BUSY"}, 
		{SCSISTAT_INTERMEDIATE, (INT64)"INTERMEDIATE"}, 
		{SCSISTAT_INTERMEDIATE_COND_MET, (INT64)"INTERMEDIATE_COND_MET"}, 
		{SCSISTAT_RESERVATION_CONFLICT, (INT64)"RESERVATION_CONFLICT"}, 
		{SCSISTAT_COMMAND_TERMINATED, (INT64)"COMMAND_TERMINATED"}, 
		{SCSISTAT_QUEUE_FULL, (INT64)"QUEUE_FULL"}
	};
#else
	INT aScsiStatus[][2] = {
		{SCSISTAT_GOOD, (INT)"GOOD"}, 
		{SCSISTAT_CHECK_CONDITION, (INT)"CHECK_CONDITION"}, 
		{SCSISTAT_CONDITION_MET, (INT)"CONDITION_MET"}, 
		{SCSISTAT_BUSY, (INT)"BUSY"}, 
		{SCSISTAT_INTERMEDIATE, (INT)"INTERMEDIATE"}, 
		{SCSISTAT_INTERMEDIATE_COND_MET, (INT)"INTERMEDIATE_COND_MET"}, 
		{SCSISTAT_RESERVATION_CONFLICT, (INT)"RESERVATION_CONFLICT"}, 
		{SCSISTAT_COMMAND_TERMINATED, (INT)"COMMAND_TERMINATED"}, 
		{SCSISTAT_QUEUE_FULL, (INT)"QUEUE_FULL"}
	};
#endif

void OutputScsiStatus(
	CONST PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER swb,
	PUCHAR byScsiStatus,
	LPCSTR pszFuncname,
	INT nLineNum
	)
{
	if(swb->ScsiPassThroughDirect.ScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		UCHAR key = (UCHAR)(swb->SenseInfoBuffer[2] & 0x0F);
		UCHAR ASC = swb->SenseInfoBuffer[12];
		UCHAR ASCQ = swb->SenseInfoBuffer[13];
		if(key != SCSI_SENSE_NO_SENSE || ASC != SCSI_ADSENSE_NO_SENSE || ASCQ != 0x00) {
			for(INT i = 0; i < sizeof(aScsiStatus) / sizeof(INT) / 2; i++) {
				if(swb->ScsiPassThroughDirect.ScsiStatus == aScsiStatus[i][0]) {
					OutputErrorStringA(
						"\nSCSI bus status codes:%02X-%s [F:%s][L:%d]\n", 
						aScsiStatus[i][0], (CHAR*)aScsiStatus[i][1], 
						pszFuncname, nLineNum);
					OutputSense(key, ASC, ASCQ);
					break;
				}
			}
		}
		else {
			// for PLEXTOR PX-320A
			*byScsiStatus = SCSISTAT_GOOD;
		}
	}
}

#ifdef WIN64
	INT64 aSenseKey[][2] = {
		{SCSI_SENSE_NO_SENSE, (INT64)"NO_SENSE"},
		{SCSI_SENSE_RECOVERED_ERROR, (INT64)"RECOVERED_ERROR"}, 
		{SCSI_SENSE_NOT_READY, (INT64)"NOT_READY"}, 
		{SCSI_SENSE_MEDIUM_ERROR, (INT64)"MEDIUM_ERROR"}, 
		{SCSI_SENSE_HARDWARE_ERROR, (INT64)"HARDWARE_ERROR"}, 
		{SCSI_SENSE_ILLEGAL_REQUEST, (INT64)"ILLEGAL_REQUEST"}, 
		{SCSI_SENSE_UNIT_ATTENTION, (INT64)"UNIT_ATTENTION"}, 
		{SCSI_SENSE_DATA_PROTECT, (INT64)"DATA_PROTECT"}, 
		{SCSI_SENSE_BLANK_CHECK, (INT64)"BLANK_CHECK"}, 
		{SCSI_SENSE_UNIQUE, (INT64)"UNIQUE"}, 
		{SCSI_SENSE_COPY_ABORTED, (INT64)"COPY_ABORTED"}, 
		{SCSI_SENSE_ABORTED_COMMAND, (INT64)"ABORTED_COMMAND"}, 
		{SCSI_SENSE_EQUAL, (INT64)"EQUAL"}, 
		{SCSI_SENSE_VOL_OVERFLOW, (INT64)"VOL_OVERFLOW"}, 
		{SCSI_SENSE_MISCOMPARE, (INT64)"MISCOMPARE"} 
	};
	// only C/DVD Device (MMC-6)
	INT64 aSenseAscAscq[][3] = {
		{SCSI_ADSENSE_NO_SENSE, 0x00, (INT64)"NO SENSE"},
		{SCSI_ADSENSE_NO_SEEK_COMPLETE, 0x00, (INT64)"NO SEEK COMPLETE"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_CAUSE_NOT_REPORTABLE, (INT64)"LUN_NOT_READY - CAUSE_NOT_REPORTABLE"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_BECOMING_READY, (INT64)"LUN_NOT_READY - BECOMING_READY"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_INIT_COMMAND_REQUIRED, (INT64)"LUN_NOT_READY - INIT_COMMAND_REQUIRED"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_MANUAL_INTERVENTION_REQUIRED, (INT64)"LUN_NOT_READY - MANUAL_INTERVENTION_REQUIRED"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_FORMAT_IN_PROGRESS, (INT64)"LUN_NOT_READY - FORMAT_IN_PROGRESS"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_OPERATION_IN_PROGRESS, (INT64)"LUN_NOT_READY - OPERATION_IN_PROGRESS"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_LONG_WRITE_IN_PROGRESS, (INT64)"LUN_NOT_READY - LONG_WRITE_IN_PROGRESS"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x09, (INT64)"LUN_NOT_READY - SELF-TEST IN PROGRESS"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x0A, (INT64)"LUN_NOT_READY - ASYMMETRIC ACCESS STATE TRANSITION"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x0B, (INT64)"LUN_NOT_READY - TARGET PORT IN STANDBY STATE"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x0C, (INT64)"LUN_NOT_READY - TARGET PORT IN UNAVAILABLE STATE"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x10, (INT64)"LUN_NOT_READY - AUXILIARY MEMORY NOT ACCESSIBLE"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x11, (INT64)"LUN_NOT_READY - NOTIFY (ENABLE SPINUP) REQUIRED"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x13, (INT64)"LUN_NOT_READY - SA CREATION IN PROGRESS"}, 
		{0x05, 0x00, (INT64)"LOGICAL UNIT DOES NOT RESPOND TO SELECTION"}, 
		{0x06, 0x00, (INT64)"NO REFERENCE POSITION FOUND"}, 
		{0x07, 0x00, (INT64)"MULTIPLE PERIPHERAL DEVICES SELECTED"}, 
		{SCSI_ADSENSE_LUN_COMMUNICATION, SCSI_SENSEQ_COMM_FAILURE, (INT64)"LUN_COMMUNICATION - COMM_FAILURE"}, 
		{SCSI_ADSENSE_LUN_COMMUNICATION, SCSI_SENSEQ_COMM_TIMEOUT, (INT64)"LUN_COMMUNICATION - COMM_TIMEOUT"}, 
		{SCSI_ADSENSE_LUN_COMMUNICATION, SCSI_SENSEQ_COMM_PARITY_ERROR, (INT64)"LUN_COMMUNICATION - COMM_PARITY_ERROR"}, 
		{SCSI_ADSENSE_LUN_COMMUNICATION, SCSI_SESNEQ_COMM_CRC_ERROR, (INT64)"LUN_COMMUNICATION - COMM_CRC_ERROR"}, 
		{SCSI_ADSENSE_LUN_COMMUNICATION, SCSI_SENSEQ_UNREACHABLE_TARGET, (INT64)"LUN_COMMUNICATION - UNREACHABLE_TARGET"}, 
		{0x09, 0x00, (INT64)"TRACK FOLLOWING ERROR"}, 
		{0x09, 0x01, (INT64)"TRACKING SERVO FAILURE"}, 
		{0x09, 0x02, (INT64)"FOCUS SERVO FAILURE"}, 
		{0x09, 0x03, (INT64)"SPINDLE SERVO FAILURE"}, 
		{0x09, 0x04, (INT64)"HEAD SELECT FAULT"}, 
		{0x0A, 0x00, (INT64)"ERROR LOG OVERFLOW"}, 
		{0x0B, 0x00, (INT64)"WARNING"}, 
		{0x0B, 0x01, (INT64)"WARNING - SPECIFIED TEMPERATURE EXCEEDED"}, 
		{0x0B, 0x02, (INT64)"WARNING - ENCLOSURE DEGRADED"}, 
		{0x0B, 0x03, (INT64)"WARNING - BACKGROUND SELF-TEST FAILED"}, 
		{0x0B, 0x04, (INT64)"WARNING - BACKGROUND PRE-SCAN DETECTED MEDIUM ERROR"}, 
		{0x0B, 0x05, (INT64)"WARNING - BACKGROUND MEDIUM SCAN DETECTED MEDIUM ERROR"}, 
		{0x0B, 0x06, (INT64)"WARNING - NON-VOLATILE CACHE NOW VOLATILE"}, 
		{0x0B, 0x07, (INT64)"WARNING - DEGRADED POWER TO NON-VOLATILE CACHE"}, 
		{0x0B, 0x08, (INT64)"WARNING - POWER LOSS EXPECTED"}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x00, (INT64)"WRITE ERROR"}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x06, (INT64)"BLOCK NOT COMPRESSIBLE"}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x07, (INT64)"WRITE ERROR - RECOVERY NEEDED"}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x08, (INT64)"WRITE ERROR - RECOVERY FAILED"}, 
		{SCSI_ADSENSE_WRITE_ERROR, SCSI_SENSEQ_LOSS_OF_STREAMING, (INT64)"WRITE ERROR - LOSS OF STREAMING"}, 
		{SCSI_ADSENSE_WRITE_ERROR, SCSI_SENSEQ_PADDING_BLOCKS_ADDED, (INT64)"WRITE ERROR - PADDING BLOCKS ADDED"}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x0B, (INT64)"AUXILIARY MEMORY WRITE ERROR"}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x0C, (INT64)"WRITE ERROR - UNEXPECTED UNSOLICITED DATA"}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x0D, (INT64)"WRITE ERROR - NOT ENOUGH UNSOLICITED DATA"}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x0F, (INT64)"DEFECTS IN ERROR WINDOW"}, 
		{0x0D, 0x00, (INT64)"ERROR DETECTED BY THIRD PARTY TEMPORARY INITIATOR"}, 
		{0x0D, 0x01, (INT64)"THIRD PARTY DEVICE FAILURE"}, 
		{0x0D, 0x02, (INT64)"COPY TARGET DEVICE NOT REACHABLE"}, 
		{0x0D, 0x03, (INT64)"INCORRECT COPY TARGET DEVICE TYPE"}, 
		{0x0D, 0x04, (INT64)"COPY TARGET DEVICE DATA UNDERRUN"}, 
		{0x0D, 0x05, (INT64)"COPY TARGET DEVICE DATA OVERRUN"}, 
		{0x0E, 0x00, (INT64)"INVALID INFORMATION UNIT"}, 
		{0x0E, 0x01, (INT64)"INFORMATION UNIT TOO SHORT"}, 
		{0x0E, 0x02, (INT64)"INFORMATION UNIT TOO LONG"}, 
		{0x0E, 0x03, (INT64)"INVALID FIELD IN COMMAND INFORMATION UNIT"}, 
		{0x11, 0x00, (INT64)"UNRECOVERED READ ERROR"}, 
		{0x11, 0x01, (INT64)"READ RETRIES EXHAUSTED"}, 
		{0x11, 0x02, (INT64)"ERROR TOO LONG TO CORRECT"}, 
		{0x11, 0x05, (INT64)"L-EC UNCORRECTABLE ERROR"}, 
		{0x11, 0x06, (INT64)"CIRC UNRECOVERED ERROR"}, 
		{0x11, 0x0D, (INT64)"DE-COMPRESSION CRC ERROR"}, 
		{0x11, 0x0E, (INT64)"CANNOT DECOMPRESS USING DECLARED ALGORITHM"}, 
		{0x11, 0x0F, (INT64)"ERROR READING UPC/EAN NUMBER"}, 
		{0x11, 0x10, (INT64)"ERROR READING ISRC NUMBER"}, 
		{0x11, 0x11, (INT64)"READ ERROR - LOSS OF STREAMING"}, 
		{0x11, 0x12, (INT64)"AUXILIARY MEMORY READ ERROR"}, 
		{0x11, 0x13, (INT64)"READ ERROR - FAILED RETRANSMISSION REQUEST"}, 
		{SCSI_ADSENSE_TRACK_ERROR, 0x00, (INT64)"TRACK_ERROR - RECORDED ENTITY NOT FOUND"}, 
		{SCSI_ADSENSE_TRACK_ERROR, 0x01, (INT64)"TRACK_ERROR - RECORD NOT FOUND"}, 
		{SCSI_ADSENSE_SEEK_ERROR, 0x00, (INT64)"SEEK_ERROR - RANDOM POSITIONING ERROR"}, 
		{SCSI_ADSENSE_SEEK_ERROR, 0x01, (INT64)"SEEK_ERROR - MECHANICAL POSITIONING ERROR"}, 
		{SCSI_ADSENSE_SEEK_ERROR, 0x02, (INT64)"SEEK_ERROR - POSITIONING ERROR DETECTED BY READ OF MEDIUM"}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x00, (INT64)"REC_DATA_NOECC - RECOVERED DATA WITH NO ERROR CORRECTION APPLIED"}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x01, (INT64)"REC_DATA_NOECC - RECOVERED DATA WITH RETRIES"}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x02, (INT64)"REC_DATA_NOECC - RECOVERED DATA WITH POSITIVE HEAD OFFSET"}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x03, (INT64)"REC_DATA_NOECC - RECOVERED DATA WITH NEGATIVE HEAD OFFSET"}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x04, (INT64)"REC_DATA_NOECC - RECOVERED DATA WITH RETRIES AND/OR CIRC APPLIED"}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x05, (INT64)"REC_DATA_NOECC - RECOVERED DATA USING PREVIOUS SECTOR ID"}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x07, (INT64)"REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - RECOMMEND REASSIGNMENT"}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x08, (INT64)"REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - RECOMMEND REWRITE"}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x09, (INT64)"REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - DATA REWRITTEN"}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x00, (INT64)"REC_DATA_ECC - RECOVERED DATA WITH ERROR CORRECTION APPLIED"}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x01, (INT64)"REC_DATA_ECC - RECOVERED DATA WITH ERROR CORR. & RETRIES APPLIED"}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x02, (INT64)"REC_DATA_ECC - RECOVERED DATA - DATA AUTO-REALLOCATED"}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x03, (INT64)"REC_DATA_ECC - RECOVERED DATA WITH CIRC"}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x04, (INT64)"REC_DATA_ECC - RECOVERED DATA WITH L-EC"}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x05, (INT64)"REC_DATA_ECC - RECOVERED DATA - RECOMMEND REASSIGNMENT"}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x06, (INT64)"REC_DATA_ECC - RECOVERED DATA - RECOMMEND REWRITE"}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x08, (INT64)"REC_DATA_ECC - RECOVERED DATA WITH LINKING"}, 
		{SCSI_ADSENSE_PARAMETER_LIST_LENGTH, 0x00, (INT64)"PARAMETER LIST LENGTH ERROR"}, 
		{0x1B, 0x00, (INT64)"SYNCHRONOUS DATA TRANSFER ERROR"}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x00, (INT64)"INVALID COMMAND OPERATION CODE"}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x01, (INT64)"ILLEGAL_COMMAND - ACCESS DENIED - INITIATOR PENDING-ENROLLED"}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x02, (INT64)"ILLEGAL_COMMAND - ACCESS DENIED - NO ACCESS RIGHTS"}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x03, (INT64)"ILLEGAL_COMMAND - ACCESS DENIED - INVALID MGMT ID KEY"}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x08, (INT64)"ILLEGAL_COMMAND - ACCESS DENIED - ENROLLMENT CONFLICT"}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x09, (INT64)"ILLEGAL_COMMAND - ACCESS DENIED - INVALID LU IDENTIFIER"}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x0A, (INT64)"ILLEGAL_COMMAND - ACCESS DENIED - INVALID PROXY TOKEN"}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x0B, (INT64)"ILLEGAL_COMMAND - ACCESS DENIED - ACL LUN CONFLICT"}, 
		{SCSI_ADSENSE_ILLEGAL_BLOCK, 0x00, (INT64)"LOGICAL BLOCK ADDRESS OUT OF RANGE"}, 
		{SCSI_ADSENSE_ILLEGAL_BLOCK, SCSI_SENSEQ_ILLEGAL_ELEMENT_ADDR, (INT64)"ILLEGAL_BLOCK - MISCOMPARE DURING VERIFY OPERATION"}, 
		{SCSI_ADSENSE_ILLEGAL_BLOCK, 0x02, (INT64)"ILLEGAL_BLOCK - INVALID ADDRESS FOR WRITE"}, 
		{SCSI_ADSENSE_ILLEGAL_BLOCK, 0x03, (INT64)"ILLEGAL_BLOCK - INVALID WRITE CROSSING LAYER JUMP"}, 
		{0x23, 0x00, (INT64)"INVALID TOKEN OPERATION, CAUSE NOT REPORTABLE"}, 
		{SCSI_ADSENSE_INVALID_CDB, 0x00, (INT64)"INVALID FIELD IN CDB"}, 
		{SCSI_ADSENSE_INVALID_CDB, 0x01, (INT64)"CDB DECRYPTION ERROR"}, 
		{SCSI_ADSENSE_INVALID_CDB, 0x08, (INT64)"INVALID XCDB"}, 
		{SCSI_ADSENSE_INVALID_LUN, 0x00, (INT64)"LOGICAL UNIT NOT SUPPORTED"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x00, (INT64)"INVALID FIELD IN PARAMETER LIST"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x01, (INT64)"PARAMETER NOT SUPPORTED"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x02, (INT64)"PARAMETER VALUE INVALID"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x03, (INT64)"THRESHOLD PARAMETERS NOT SUPPORTED"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x04, (INT64)"INVALID RELEASE OF PERSISTENT RESERVATION"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x05, (INT64)"DATA DECRYPTION ERROR"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x06, (INT64)"TOO MANY TARGET DESCRIPTORS"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x07, (INT64)"UNSUPPORTED SEGMENT DESCRIPTOR TYPE CODE"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x08, (INT64)"TOO MANY SEGMENT DESCRIPTORS"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x09, (INT64)"UNSUPPORTED SEGMENT DESCRIPTOR TYPE CODE"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x0A, (INT64)"UNEXPECTED INEXACT SEGMENT"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x0B, (INT64)"INLINE DATA LENGTH EXCEEDED"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x0C, (INT64)"INVALID OPERATION FOR COPY SOURCE OR DESTINATION"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x0D, (INT64)"COPY SEGMENT GRANULARITY VIOLATION"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x0E, (INT64)"INVALID PARAMETER WHILE PORT IS ENABLED"}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x00, (INT64)"WRITE PROTECTED"}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x01, (INT64)"HARDWARE WRITE PROTECTED"}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x02, (INT64)"LOGICAL UNIT SOFTWARE WRITE PROTECTED"}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x03, (INT64)"ASSOCIATED WRITE PROTECT"}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x04, (INT64)"PERSISTENT WRITE PROTECT"}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x05, (INT64)"PERMANENT WRITE PROTECT"}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x06, (INT64)"CONDITIONAL WRITE PROTECT"}, 
		{SCSI_ADSENSE_MEDIUM_CHANGED, 0x00, (INT64)"NOT READY TO READY CHANGE, MEDIUM MAY HAVE CHANGED"}, 
		{SCSI_ADSENSE_MEDIUM_CHANGED, 0x01, (INT64)"IMPORT OR EXPORT ELEMENT ACCESSED"}, 
		{SCSI_ADSENSE_MEDIUM_CHANGED, 0x02, (INT64)"FORMAT-LAYER MAY HAVE CHANGED"}, 
		{SCSI_ADSENSE_BUS_RESET, 0x00, (INT64)"POWER ON, RESET, OR BUS DEVICE RESET OCCURRED"}, 
		{SCSI_ADSENSE_BUS_RESET, 0x01, (INT64)"POWER ON OCCURRED"}, 
		{SCSI_ADSENSE_BUS_RESET, 0x02, (INT64)"SCSI BUS RESET OCCURRED"}, 
		{SCSI_ADSENSE_BUS_RESET, 0x03, (INT64)"BUS DEVICE RESET FUNCTION OCCURRED"}, 
		{SCSI_ADSENSE_BUS_RESET, 0x04, (INT64)"DEVICE INTERNAL RESET"}, 
		{SCSI_ADSENSE_BUS_RESET, 0x05, (INT64)"TRANSCEIVER MODE CHANGED TO SINGLE-ENDED"}, 
		{SCSI_ADSENSE_BUS_RESET, 0x06, (INT64)"TRANSCEIVER MODE CHANGED TO LVD"}, 
		{SCSI_ADSENSE_BUS_RESET, 0x07, (INT64)"I_T NEXUS LOSS OCCURRED"}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x00, (INT64)"PARAMETERS CHANGED"}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x01, (INT64)"MODE PARAMETERS CHANGED"}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x02, (INT64)"LOG PARAMETERS CHANGED"}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x03, (INT64)"RESERVATIONS PREEMPTED"}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x04, (INT64)"RESERVATIONS RELEASED"}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x05, (INT64)"REGISTRATIONS PREEMPTED"}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x06, (INT64)"ASYMMETRIC ACCESS STATE CHANGED"}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x07, (INT64)"IMPLICIT ASYMMETRIC ACCESS STATE TRANSITION FAILED"}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x08, (INT64)"PRIORITY CHANGED"}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x14, (INT64)"SA CREATION CAPABILITIES DATA HAS CHANGED"}, 
		{0x2B, 0x00, (INT64)"COPY CANNOT EXECUTE SINCE HOST CANNOT DISCONNECT"}, 
		{0x2C, 0x00, (INT64)"COMMAND SEQUENCE ERROR"}, 
		{0x2C, 0x03, (INT64)"CURRENT PROGRAM AREA IS NOT EMPTY"}, 
		{0x2C, 0x04, (INT64)"CURRENT PROGRAM AREA IS EMPTY"}, 
		{0x2C, 0x06, (INT64)"PERSISTENT PREVENT CONFLICT"}, 
		{0x2C, 0x07, (INT64)"PREVIOUS BUSY STATUS"}, 
		{0x2C, 0x08, (INT64)"PREVIOUS TASK SET FULL STATUS"}, 
		{0x2C, 0x09, (INT64)"PREVIOUS RESERVATION CONFLICT STATUS"}, 
		{SCSI_ADSENSE_INSUFFICIENT_TIME_FOR_OPERATION, 0x00, (INT64)"INSUFFICIENT TIME FOR OPERATION"}, 
		{0x2F, 0x00, (INT64)"COMMANDS CLEARED BY ANOTHER INITIATOR"}, 
		{0x2F, 0x02, (INT64)"COMMANDS CLEARED BY DEVICE SERVER"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x00, (INT64)"INCOMPATIBLE MEDIUM INSTALLED"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x01, (INT64)"CANNOT READ MEDIUM - UNKNOWN FORMAT"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x02, (INT64)"CANNOT READ MEDIUM - INCOMPATIBLE FORMAT"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x03, (INT64)"CLEANING CARTRIDGE INSTALLED"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x04, (INT64)"CANNOT WRITE MEDIUM - UNKNOWN FORMAT"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x05, (INT64)"CANNOT WRITE MEDIUM - INCOMPATIBLE FORMAT"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x06, (INT64)"CANNOT FORMAT MEDIUM - INCOMPATIBLE MEDIUM"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x07, (INT64)"CLEANING FAILURE"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x08, (INT64)"CANNOT WRITE - APPLICATION CODE MISMATCH"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x09, (INT64)"CURRENT SESSION NOT FIXATED FOR APPEND"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x0A, (INT64)"CLEANING REQUEST REJECTED"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x10, (INT64)"MEDIUM NOT FORMATTED"}, 
		{0x31, 0x00, (INT64)"MEDIUM FORMAT CORRUPTED"}, 
		{0x31, 0x01, (INT64)"FORMAT COMMAND FAILED"}, 
		{0x31, 0x02, (INT64)"ZONED FORMATTING FAILED DUE TO SPARE LINKING"}, 
		{0x34, 0x00, (INT64)"ENCLOSURE FAILURE"}, 
		{0x35, 0x00, (INT64)"ENCLOSURE SERVICES FAILURE"}, 
		{0x35, 0x01, (INT64)"UNSUPPORTED ENCLOSURE FUNCTION"}, 
		{0x35, 0x02, (INT64)"ENCLOSURE SERVICES UNAVAILABLE"}, 
		{0x35, 0x03, (INT64)"ENCLOSURE SERVICES TRANSFER FAILURE"}, 
		{0x35, 0x04, (INT64)"ENCLOSURE SERVICES TRANSFER REFUSED"}, 
		{0x35, 0x05, (INT64)"ENCLOSURE SERVICES CHECKSUM ERROR"}, 
		{0x37, 0x00, (INT64)"ROUNDED PARAMETER"}, 
		{0x39, 0x00, (INT64)"SAVING PARAMETERS NOT SUPPORTED"}, 
		{SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0x00, (INT64)"MEDIUM NOT PRESENT"}, 
		{SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0x01, (INT64)"MEDIUM NOT PRESENT - TRAY CLOSED"}, 
		{SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0x02, (INT64)"MEDIUM NOT PRESENT - TRAY OPEN"}, 
		{SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0x03, (INT64)"MEDIUM NOT PRESENT - LOADABLE"}, 
		{SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0x04, (INT64)"MEDIUM NOT PRESENT - MEDIUM AUXILIARY MEMORY ACCESSIBLE"}, 
		{SCSI_ADSENSE_POSITION_ERROR, SCSI_SENSEQ_DESTINATION_FULL, (INT64)"MEDIUM DESTINATION ELEMENT FULL"}, 
		{SCSI_ADSENSE_POSITION_ERROR, SCSI_SENSEQ_SOURCE_EMPTY, (INT64)"MEDIUM SOURCE ELEMENT EMPTY"}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x0F, (INT64)"END OF MEDIUM REACHED"}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x11, (INT64)"MEDIUM MAGAZINE NOT ACCESSIBLE"}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x12, (INT64)"MEDIUM MAGAZINE REMOVED"}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x13, (INT64)"MEDIUM MAGAZINE INSERTED"}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x14, (INT64)"MEDIUM MAGAZINE LOCKED"}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x15, (INT64)"MEDIUM MAGAZINE UNLOCKED"}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x16, (INT64)"MECHANICAL POSITIONING OR CHANGER ERROR"}, 
		{0x3D, 0x00, (INT64)"INVALID BITS IN IDENTIFY MESSAGE"}, 
		{0x3E, 0x00, (INT64)"LOGICAL UNIT HAS NOT SELF-CONFIGURED YET"}, 
		{0x3E, 0x01, (INT64)"LOGICAL UNIT FAILURE"}, 
		{0x3E, 0x02, (INT64)"TIMEOUT ON LOGICAL UNIT"}, 
		{0x3E, 0x03, (INT64)"LOGICAL UNIT FAILED SELF-TEST"}, 
		{0x3E, 0x04, (INT64)"LOGICAL UNIT UNABLE TO UPDATE SELF-TEST LOG"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_TARGET_OPERATING_CONDITIONS_CHANGED, (INT64)"TARGET OPERATING CONDITIONS HAVE CHANGED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_MICROCODE_CHANGED, (INT64)"MICROCODE HAS BEEN CHANGED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_OPERATING_DEFINITION_CHANGED, (INT64)"CHANGED OPERATING DEFINITION"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_INQUIRY_DATA_CHANGED, (INT64)"INQUIRY DATA HAS CHANGED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_COMPONENT_DEVICE_ATTACHED, (INT64)"COMPONENT DEVICE ATTACHED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_DEVICE_IDENTIFIER_CHANGED, (INT64)"DEVICE IDENTIFIER CHANGED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_REDUNDANCY_GROUP_MODIFIED, (INT64)"REDUNDANCY GROUP CREATED OR MODIFIED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_REDUNDANCY_GROUP_DELETED, (INT64)"REDUNDANCY GROUP DELETED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_SPARE_MODIFIED, (INT64)"SPARE CREATED OR MODIFIED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_SPARE_DELETED, (INT64)"SPARE DELETED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_VOLUME_SET_MODIFIED, (INT64)"VOLUME SET CREATED OR MODIFIED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_VOLUME_SET_DELETED, (INT64)"VOLUME SET DELETED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_VOLUME_SET_DEASSIGNED, (INT64)"VOLUME SET DEASSIGNED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_VOLUME_SET_REASSIGNED, (INT64)"VOLUME SET REASSIGNED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_REPORTED_LUNS_DATA_CHANGED, (INT64)"REPORTED LUNS DATA HAS CHANGED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_ECHO_BUFFER_OVERWRITTEN, (INT64)"ECHO pBuf OVERWRITTEN"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_MEDIUM_LOADABLE, (INT64)"MEDIUM LOADABLE"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_MEDIUM_AUXILIARY_MEMORY_ACCESSIBLE, (INT64)"MEDIUM AUXILIARY MEMORY ACCESSIBLE"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, 0x12, (INT64)"iSCSI IP ADDRESS ADDED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, 0x13, (INT64)"iSCSI IP ADDRESS REMOVED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, 0x14, (INT64)"iSCSI IP ADDRESS CHANGED"}, 
		{0x43, 0x00, (INT64)"MESSAGE ERROR"}, 
		{0x44, 0x00, (INT64)"INTERNAL TARGET FAILURE"}, 
		{0x45, 0x00, (INT64)"SELECT OR RESELECT FAILURE"}, 
		{0x46, 0x00, (INT64)"UNSUCCESSFUL SOFT RESET"}, 
		{0x47, 0x00, (INT64)"SCSI PARITY ERROR"}, 
		{0x47, 0x01, (INT64)"DATA PHASE CRC ERROR DETECTED"}, 
		{0x47, 0x02, (INT64)"SCSI PARITY ERROR DETECTED DURING ST DATA PHASE"}, 
		{0x47, 0x03, (INT64)"INFORMATION UNIT iuCRC ERROR DETECTED"}, 
		{0x47, 0x04, (INT64)"ASYNCHRONOUS INFORMATION PROTECTION ERROR DETECTED"}, 
		{0x47, 0x05, (INT64)"PROTOCOL SERVICE CRC ERROR"}, 
		{0x47, 0x7F, (INT64)"SOME COMMANDS CLEARED BY ISCSI PROTOCOL EVENT"}, 
		{0x48, 0x00, (INT64)"INITIATOR DETECTED ERROR MESSAGE RECEIVED"}, 
		{0x49, 0x00, (INT64)"INVALID MESSAGE ERROR"}, 
		{0x4A, 0x00, (INT64)"COMMAND PHASE ERROR"}, 
		{0x4B, 0x00, (INT64)"DATA PHASE ERROR"}, 
		{0x4B, 0x01, (INT64)"INVALID TARGET PORT TRANSFER TAG RECEIVED"}, 
		{0x4B, 0x02, (INT64)"TOO MUCH WRITE DATA"}, 
		{0x4B, 0x03, (INT64)"ACK/NAK TIMEOUT"}, 
		{0x4B, 0x04, (INT64)"NAK RECEIVED"}, 
		{0x4B, 0x05, (INT64)"DATA OFFSET ERROR"}, 
		{0x4B, 0x06, (INT64)"INITIATOR RESPONSE TIMEOUT"}, 
		{0x4B, 0x07, (INT64)"CONNECTION LOST"}, 
		{0x4C, 0x00, (INT64)"LOGICAL UNIT FAILED SELF-CONFIGURATION"}, 
		{0x4E, 0x00, (INT64)"OVERLAPPED COMMANDS ATTEMPTED"}, 
		{0x51, 0x00, (INT64)"ERASE FAILURE"}, 
		{0x51, 0x01, (INT64)"ERASE FAILURE - INCOMPLETE ERASE OPERATION DETECTED"}, 
		{0x53, 0x00, (INT64)"MEDIA LOAD OR EJECT FAILED"}, 
		{0x53, 0x02, (INT64)"MEDIUM REMOVAL PREVENTED"}, 
		{0x55, 0x02, (INT64)"INSUFFICIENT RESERVATION RESOURCES"}, 
		{0x55, 0x03, (INT64)"INSUFFICIENT RESOURCES"}, 
		{0x55, 0x04, (INT64)"INSUFFICIENT REGISTRATION RESOURCES"}, 
		{0x55, 0x05, (INT64)"INSUFFICIENT ACCESS CONTROL RESOURCES"}, 
		{0x55, 0x06, (INT64)"AUXILIARY MEMORY OUT OF SPACE"}, 
		{0x55, 0x0B, (INT64)"INSUFFICIENT POWER FOR OPERATION"}, 
		{0x57, 0x00, (INT64)"UNABLE TO RECOVER TABLE-OF-CONTENTS"}, 
		{SCSI_ADSENSE_OPERATOR_REQUEST, SCSI_SENSEQ_STATE_CHANGE_INPUT, (INT64)"OPERATOR REQUEST OR STATE CHANGE INPUT"}, 
		{SCSI_ADSENSE_OPERATOR_REQUEST, SCSI_SENSEQ_MEDIUM_REMOVAL, (INT64)"OPERATOR MEDIUM REMOVAL REQUEST"}, 
		{SCSI_ADSENSE_OPERATOR_REQUEST, SCSI_SENSEQ_WRITE_PROTECT_ENABLE, (INT64)"OPERATOR SELECTED WRITE PROTECT"}, 
		{SCSI_ADSENSE_OPERATOR_REQUEST, SCSI_SENSEQ_WRITE_PROTECT_DISABLE, (INT64)"OPERATOR SELECTED WRITE PERMIT"}, 
		{0x5b, 0x00, (INT64)"LOG EXCEPTION"}, 
		{0x5b, 0x01, (INT64)"THRESHOLD CONDITION MET"}, 
		{0x5b, 0x02, (INT64)"LOG COUNTER AT MAXIMUM"}, 
		{0x5b, 0x03, (INT64)"LOG LIST CODES EXHAUSTED"}, 
		{SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED, 0x00, (INT64)"FAILURE PREDICTION THRESHOLD EXCEEDED"}, 
		{SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED, 0x01, (INT64)"MEDIA FAILURE PREDICTION THRESHOLD EXCEEDED"}, 
		{SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED, 0x02, (INT64)"LOGICAL UNIT FAILURE PREDICTION THRESHOLD EXCEEDED"}, 
		{SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED, 0x03, (INT64)"SPARE AREA EXHAUSTION PREDICTION THRESHOLD EXCEEDED"}, 
		{SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED, 0xFF, (INT64)"FAILURE PREDICTION THRESHOLD EXCEEDED (FALSE)"}, 
		{0x5E, 0x00, (INT64)"LOW POWER CONDITION ON"}, 
		{0x5E, 0x01, (INT64)"IDLE CONDITION ACTIVATED BY TIMER"}, 
		{0x5E, 0x02, (INT64)"STANDBY CONDITION ACTIVATED BY TIMER"}, 
		{0x5E, 0x03, (INT64)"IDLE CONDITION ACTIVATED BY COMMAND"}, 
		{0x5E, 0x04, (INT64)"STANDBY CONDITION ACTIVATED BY COMMAND"}, 
		{0x5E, 0x05, (INT64)"IDLE_B CONDITION ACTIVATED BY TIMER"}, 
		{0x5E, 0x06, (INT64)"IDLE_B CONDITION ACTIVATED BY COMMAND"}, 
		{0x5E, 0x07, (INT64)"IDLE_C CONDITION ACTIVATED BY TIMER"}, 
		{0x5E, 0x08, (INT64)"IDLE_C CONDITION ACTIVATED BY COMMAND"}, 
		{0x5E, 0x09, (INT64)"STANDBY_Y CONDITION ACTIVATED BY TIMER"}, 
		{0x5E, 0x0A, (INT64)"STANDBY_Y CONDITION ACTIVATED BY COMMAND"}, 
		{0x63, 0x00, (INT64)"END OF USER AREA ENCOUNTERED ON THIS TRACK"}, 
		{0x63, 0x01, (INT64)"PACKET DOES NOT FIT IN AVAILABLE SPACE"}, 
		{SCSI_ADSENSE_ILLEGAL_MODE_FOR_THIS_TRACK, 0x00, (INT64)"ILLEGAL MODE FOR THIS TRACK"}, 
		{SCSI_ADSENSE_ILLEGAL_MODE_FOR_THIS_TRACK, 0x01, (INT64)"INVALID PACKET SIZE"}, 
		{0x65, 0x00, (INT64)"VOLTAGE FAULT"}, 
		{0x67, 0x0A, (INT64)"SET TARGET PORT GROUPS COMMAND FAILED"}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_AUTHENTICATION_FAILURE, (INT64)"COPY PROTECTION KEY EXCHANGE FAILURE - AUTHENTICATION FAILURE"}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_KEY_NOT_PRESENT, (INT64)"COPY PROTECTION KEY EXCHANGE FAILURE - KEY NOT PRESENT"}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_KEY_NOT_ESTABLISHED, (INT64)"COPY PROTECTION KEY EXCHANGE FAILURE - KEY NOT ESTABLISHED"}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_READ_OF_SCRAMBLED_SECTOR_WITHOUT_AUTHENTICATION, (INT64)"READ OF SCRAMBLED SECTOR WITHOUT AUTHENTICATION"}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_MEDIA_CODE_MISMATCHED_TO_LOGICAL_UNIT, (INT64)"MEDIA REGION CODE IS MISMATCHED TO LOGICAL UNIT REGION"}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_LOGICAL_UNIT_RESET_COUNT_ERROR, (INT64)"DRIVE REGION MUST BE PERMANENT/REGION RESET COUNT ERROR"}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, 0x06, (INT64)"INSUFFICIENT BLOCK COUNT FOR BINDING NONCE RECORDING"}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, 0x07, (INT64)"CONFLICT IN BINDING NONCE RECORDING"}, 
		{0x72, 0x00, (INT64)"SESSION FIXATION ERROR"}, 
		{0x72, 0x01, (INT64)"SESSION FIXATION ERROR WRITING LEAD-IN"}, 
		{0x72, 0x02, (INT64)"SESSION FIXATION ERROR WRITING LEAD-OUT"}, 
		{0x72, 0x03, (INT64)"SESSION FIXATION ERROR - INCOMPLETE TRACK IN SESSION"}, 
		{0x72, 0x04, (INT64)"EMPTY OR PARTIALLY WRITTEN RESERVED TRACK"}, 
		{0x72, 0x05, (INT64)"NO MORE TRACK RESERVATIONS ALLOWED"}, 
		{0x72, 0x06, (INT64)"RMZ EXTENSION IS NOT ALLOWED"}, 
		{0x72, 0x07, (INT64)"NO MORE TEST ZONE EXTENSIONS ARE ALLOWED"}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, 0x00, (INT64)"VOLTAGE FAULT"}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_POWER_CALIBRATION_AREA_ALMOST_FULL, (INT64)"CD CONTROL ERROR"}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_POWER_CALIBRATION_AREA_FULL, (INT64)"POWER CALIBRATION AREA ALMOST FULL"}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_POWER_CALIBRATION_AREA_ERROR, (INT64)"POWER CALIBRATION AREA IS FULL"}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_PMA_RMA_UPDATE_FAILURE, (INT64)"POWER CALIBRATION AREA ERROR"}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_PMA_RMA_IS_FULL, (INT64)"PROGRAM MEMORY AREA UPDATE FAILURE"}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_PMA_RMA_ALMOST_FULL, (INT64)"RMA/PMA IS ALMOST FULL"}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, 0x10, (INT64)"CURRENT POWER CALIBRATION AREA ALMOST FULL"}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, 0x11, (INT64)"CURRENT POWER CALIBRATION AREA IS FULL"}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, 0x17, (INT64)"RDZ IS FULL"}, 
		{0x74, 0x08, (INT64)"DIGITAL SIGNATURE VALIDATION FAILURE"}, 
		{0x74, 0x0C, (INT64)"UNABLE TO DECRYPT PARAMETER LIST"}, 
		{0x74, 0x10, (INT64)"SA CREATION PARAMETER VALUE INVALID"}, 
		{0x74, 0x11, (INT64)"SA CREATION PARAMETER VALUE REJECTED"}, 
		{0x74, 0x12, (INT64)"INVALID SA USAGE"}, 
		{0x74, 0x30, (INT64)"SA CREATION PARAMETER NOT SUPPORTED"}, 
		{0x74, 0x40, (INT64)"AUTHENTICATION FAILED"}, 
		{0x74, 0x71, (INT64)"LOGICAL UNIT ACCESS NOT AUTHORIZED"} 
	};
#else
	INT aSenseKey[][2] = {
		{SCSI_SENSE_NO_SENSE, (INT)"NO_SENSE"},
		{SCSI_SENSE_RECOVERED_ERROR, (INT)"RECOVERED_ERROR"}, 
		{SCSI_SENSE_NOT_READY, (INT)"NOT_READY"}, 
		{SCSI_SENSE_MEDIUM_ERROR, (INT)"MEDIUM_ERROR"}, 
		{SCSI_SENSE_HARDWARE_ERROR, (INT)"HARDWARE_ERROR"}, 
		{SCSI_SENSE_ILLEGAL_REQUEST, (INT)"ILLEGAL_REQUEST"}, 
		{SCSI_SENSE_UNIT_ATTENTION, (INT)"UNIT_ATTENTION"}, 
		{SCSI_SENSE_DATA_PROTECT, (INT)"DATA_PROTECT"}, 
		{SCSI_SENSE_BLANK_CHECK, (INT)"BLANK_CHECK"}, 
		{SCSI_SENSE_UNIQUE, (INT)"UNIQUE"}, 
		{SCSI_SENSE_COPY_ABORTED, (INT)"COPY_ABORTED"}, 
		{SCSI_SENSE_ABORTED_COMMAND, (INT)"ABORTED_COMMAND"}, 
		{SCSI_SENSE_EQUAL, (INT)"EQUAL"}, 
		{SCSI_SENSE_VOL_OVERFLOW, (INT)"VOL_OVERFLOW"}, 
		{SCSI_SENSE_MISCOMPARE, (INT)"MISCOMPARE"} 
	};
	// only C/DVD Device (MMC-6)
	INT aSenseAscAscq[][3] = {
		{SCSI_ADSENSE_NO_SENSE, 0x00, (INT)"NO SENSE"},
		{SCSI_ADSENSE_NO_SEEK_COMPLETE, 0x00, (INT)"NO SEEK COMPLETE"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_CAUSE_NOT_REPORTABLE, (INT)"LUN_NOT_READY - CAUSE_NOT_REPORTABLE"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_BECOMING_READY, (INT)"LUN_NOT_READY - BECOMING_READY"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_INIT_COMMAND_REQUIRED, (INT)"LUN_NOT_READY - INIT_COMMAND_REQUIRED"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_MANUAL_INTERVENTION_REQUIRED, (INT)"LUN_NOT_READY - MANUAL_INTERVENTION_REQUIRED"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_FORMAT_IN_PROGRESS, (INT)"LUN_NOT_READY - FORMAT_IN_PROGRESS"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_OPERATION_IN_PROGRESS, (INT)"LUN_NOT_READY - OPERATION_IN_PROGRESS"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_LONG_WRITE_IN_PROGRESS, (INT)"LUN_NOT_READY - LONG_WRITE_IN_PROGRESS"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x09, (INT)"LUN_NOT_READY - SELF-TEST IN PROGRESS"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x0A, (INT)"LUN_NOT_READY - ASYMMETRIC ACCESS STATE TRANSITION"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x0B, (INT)"LUN_NOT_READY - TARGET PORT IN STANDBY STATE"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x0C, (INT)"LUN_NOT_READY - TARGET PORT IN UNAVAILABLE STATE"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x10, (INT)"LUN_NOT_READY - AUXILIARY MEMORY NOT ACCESSIBLE"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x11, (INT)"LUN_NOT_READY - NOTIFY (ENABLE SPINUP) REQUIRED"}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x13, (INT)"LUN_NOT_READY - SA CREATION IN PROGRESS"}, 
		{0x05, 0x00, (INT)"LOGICAL UNIT DOES NOT RESPOND TO SELECTION"}, 
		{0x06, 0x00, (INT)"NO REFERENCE POSITION FOUND"}, 
		{0x07, 0x00, (INT)"MULTIPLE PERIPHERAL DEVICES SELECTED"}, 
		{SCSI_ADSENSE_LUN_COMMUNICATION, SCSI_SENSEQ_COMM_FAILURE, (INT)"LUN_COMMUNICATION - COMM_FAILURE"}, 
		{SCSI_ADSENSE_LUN_COMMUNICATION, SCSI_SENSEQ_COMM_TIMEOUT, (INT)"LUN_COMMUNICATION - COMM_TIMEOUT"}, 
		{SCSI_ADSENSE_LUN_COMMUNICATION, SCSI_SENSEQ_COMM_PARITY_ERROR, (INT)"LUN_COMMUNICATION - COMM_PARITY_ERROR"}, 
		{SCSI_ADSENSE_LUN_COMMUNICATION, SCSI_SESNEQ_COMM_CRC_ERROR, (INT)"LUN_COMMUNICATION - COMM_CRC_ERROR"}, 
		{SCSI_ADSENSE_LUN_COMMUNICATION, SCSI_SENSEQ_UNREACHABLE_TARGET, (INT)"LUN_COMMUNICATION - UNREACHABLE_TARGET"}, 
		{0x09, 0x00, (INT)"TRACK FOLLOWING ERROR"}, 
		{0x09, 0x01, (INT)"TRACKING SERVO FAILURE"}, 
		{0x09, 0x02, (INT)"FOCUS SERVO FAILURE"}, 
		{0x09, 0x03, (INT)"SPINDLE SERVO FAILURE"}, 
		{0x09, 0x04, (INT)"HEAD SELECT FAULT"}, 
		{0x0A, 0x00, (INT)"ERROR LOG OVERFLOW"}, 
		{0x0B, 0x00, (INT)"WARNING"}, 
		{0x0B, 0x01, (INT)"WARNING - SPECIFIED TEMPERATURE EXCEEDED"}, 
		{0x0B, 0x02, (INT)"WARNING - ENCLOSURE DEGRADED"}, 
		{0x0B, 0x03, (INT)"WARNING - BACKGROUND SELF-TEST FAILED"}, 
		{0x0B, 0x04, (INT)"WARNING - BACKGROUND PRE-SCAN DETECTED MEDIUM ERROR"}, 
		{0x0B, 0x05, (INT)"WARNING - BACKGROUND MEDIUM SCAN DETECTED MEDIUM ERROR"}, 
		{0x0B, 0x06, (INT)"WARNING - NON-VOLATILE CACHE NOW VOLATILE"}, 
		{0x0B, 0x07, (INT)"WARNING - DEGRADED POWER TO NON-VOLATILE CACHE"}, 
		{0x0B, 0x08, (INT)"WARNING - POWER LOSS EXPECTED"}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x00, (INT)"WRITE ERROR"}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x06, (INT)"BLOCK NOT COMPRESSIBLE"}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x07, (INT)"WRITE ERROR - RECOVERY NEEDED"}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x08, (INT)"WRITE ERROR - RECOVERY FAILED"}, 
		{SCSI_ADSENSE_WRITE_ERROR, SCSI_SENSEQ_LOSS_OF_STREAMING, (INT)"WRITE ERROR - LOSS OF STREAMING"}, 
		{SCSI_ADSENSE_WRITE_ERROR, SCSI_SENSEQ_PADDING_BLOCKS_ADDED, (INT)"WRITE ERROR - PADDING BLOCKS ADDED"}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x0B, (INT)"AUXILIARY MEMORY WRITE ERROR"}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x0C, (INT)"WRITE ERROR - UNEXPECTED UNSOLICITED DATA"}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x0D, (INT)"WRITE ERROR - NOT ENOUGH UNSOLICITED DATA"}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x0F, (INT)"DEFECTS IN ERROR WINDOW"}, 
		{0x0D, 0x00, (INT)"ERROR DETECTED BY THIRD PARTY TEMPORARY INITIATOR"}, 
		{0x0D, 0x01, (INT)"THIRD PARTY DEVICE FAILURE"}, 
		{0x0D, 0x02, (INT)"COPY TARGET DEVICE NOT REACHABLE"}, 
		{0x0D, 0x03, (INT)"INCORRECT COPY TARGET DEVICE TYPE"}, 
		{0x0D, 0x04, (INT)"COPY TARGET DEVICE DATA UNDERRUN"}, 
		{0x0D, 0x05, (INT)"COPY TARGET DEVICE DATA OVERRUN"}, 
		{0x0E, 0x00, (INT)"INVALID INFORMATION UNIT"}, 
		{0x0E, 0x01, (INT)"INFORMATION UNIT TOO SHORT"}, 
		{0x0E, 0x02, (INT)"INFORMATION UNIT TOO LONG"}, 
		{0x0E, 0x03, (INT)"INVALID FIELD IN COMMAND INFORMATION UNIT"}, 
		{0x11, 0x00, (INT)"UNRECOVERED READ ERROR"}, 
		{0x11, 0x01, (INT)"READ RETRIES EXHAUSTED"}, 
		{0x11, 0x02, (INT)"ERROR TOO LONG TO CORRECT"}, 
		{0x11, 0x05, (INT)"L-EC UNCORRECTABLE ERROR"}, 
		{0x11, 0x06, (INT)"CIRC UNRECOVERED ERROR"}, 
		{0x11, 0x0D, (INT)"DE-COMPRESSION CRC ERROR"}, 
		{0x11, 0x0E, (INT)"CANNOT DECOMPRESS USING DECLARED ALGORITHM"}, 
		{0x11, 0x0F, (INT)"ERROR READING UPC/EAN NUMBER"}, 
		{0x11, 0x10, (INT)"ERROR READING ISRC NUMBER"}, 
		{0x11, 0x11, (INT)"READ ERROR - LOSS OF STREAMING"}, 
		{0x11, 0x12, (INT)"AUXILIARY MEMORY READ ERROR"}, 
		{0x11, 0x13, (INT)"READ ERROR - FAILED RETRANSMISSION REQUEST"}, 
		{SCSI_ADSENSE_TRACK_ERROR, 0x00, (INT)"TRACK_ERROR - RECORDED ENTITY NOT FOUND"}, 
		{SCSI_ADSENSE_TRACK_ERROR, 0x01, (INT)"TRACK_ERROR - RECORD NOT FOUND"}, 
		{SCSI_ADSENSE_SEEK_ERROR, 0x00, (INT)"SEEK_ERROR - RANDOM POSITIONING ERROR"}, 
		{SCSI_ADSENSE_SEEK_ERROR, 0x01, (INT)"SEEK_ERROR - MECHANICAL POSITIONING ERROR"}, 
		{SCSI_ADSENSE_SEEK_ERROR, 0x02, (INT)"SEEK_ERROR - POSITIONING ERROR DETECTED BY READ OF MEDIUM"}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x00, (INT)"REC_DATA_NOECC - RECOVERED DATA WITH NO ERROR CORRECTION APPLIED"}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x01, (INT)"REC_DATA_NOECC - RECOVERED DATA WITH RETRIES"}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x02, (INT)"REC_DATA_NOECC - RECOVERED DATA WITH POSITIVE HEAD OFFSET"}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x03, (INT)"REC_DATA_NOECC - RECOVERED DATA WITH NEGATIVE HEAD OFFSET"}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x04, (INT)"REC_DATA_NOECC - RECOVERED DATA WITH RETRIES AND/OR CIRC APPLIED"}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x05, (INT)"REC_DATA_NOECC - RECOVERED DATA USING PREVIOUS SECTOR ID"}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x07, (INT)"REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - RECOMMEND REASSIGNMENT"}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x08, (INT)"REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - RECOMMEND REWRITE"}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x09, (INT)"REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - DATA REWRITTEN"}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x00, (INT)"REC_DATA_ECC - RECOVERED DATA WITH ERROR CORRECTION APPLIED"}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x01, (INT)"REC_DATA_ECC - RECOVERED DATA WITH ERROR CORR. & RETRIES APPLIED"}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x02, (INT)"REC_DATA_ECC - RECOVERED DATA - DATA AUTO-REALLOCATED"}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x03, (INT)"REC_DATA_ECC - RECOVERED DATA WITH CIRC"}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x04, (INT)"REC_DATA_ECC - RECOVERED DATA WITH L-EC"}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x05, (INT)"REC_DATA_ECC - RECOVERED DATA - RECOMMEND REASSIGNMENT"}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x06, (INT)"REC_DATA_ECC - RECOVERED DATA - RECOMMEND REWRITE"}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x08, (INT)"REC_DATA_ECC - RECOVERED DATA WITH LINKING"}, 
		{SCSI_ADSENSE_PARAMETER_LIST_LENGTH, 0x00, (INT)"PARAMETER LIST LENGTH ERROR"}, 
		{0x1B, 0x00, (INT)"SYNCHRONOUS DATA TRANSFER ERROR"}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x00, (INT)"INVALID COMMAND OPERATION CODE"}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x01, (INT)"ILLEGAL_COMMAND - ACCESS DENIED - INITIATOR PENDING-ENROLLED"}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x02, (INT)"ILLEGAL_COMMAND - ACCESS DENIED - NO ACCESS RIGHTS"}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x03, (INT)"ILLEGAL_COMMAND - ACCESS DENIED - INVALID MGMT ID KEY"}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x08, (INT)"ILLEGAL_COMMAND - ACCESS DENIED - ENROLLMENT CONFLICT"}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x09, (INT)"ILLEGAL_COMMAND - ACCESS DENIED - INVALID LU IDENTIFIER"}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x0A, (INT)"ILLEGAL_COMMAND - ACCESS DENIED - INVALID PROXY TOKEN"}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x0B, (INT)"ILLEGAL_COMMAND - ACCESS DENIED - ACL LUN CONFLICT"}, 
		{SCSI_ADSENSE_ILLEGAL_BLOCK, 0x00, (INT)"LOGICAL BLOCK ADDRESS OUT OF RANGE"}, 
		{SCSI_ADSENSE_ILLEGAL_BLOCK, SCSI_SENSEQ_ILLEGAL_ELEMENT_ADDR, (INT)"ILLEGAL_BLOCK - MISCOMPARE DURING VERIFY OPERATION"}, 
		{SCSI_ADSENSE_ILLEGAL_BLOCK, 0x02, (INT)"ILLEGAL_BLOCK - INVALID ADDRESS FOR WRITE"}, 
		{SCSI_ADSENSE_ILLEGAL_BLOCK, 0x03, (INT)"ILLEGAL_BLOCK - INVALID WRITE CROSSING LAYER JUMP"}, 
		{0x23, 0x00, (INT)"INVALID TOKEN OPERATION, CAUSE NOT REPORTABLE"}, 
		{SCSI_ADSENSE_INVALID_CDB, 0x00, (INT)"INVALID FIELD IN CDB"}, 
		{SCSI_ADSENSE_INVALID_CDB, 0x01, (INT)"CDB DECRYPTION ERROR"}, 
		{SCSI_ADSENSE_INVALID_CDB, 0x08, (INT)"INVALID XCDB"}, 
		{SCSI_ADSENSE_INVALID_LUN, 0x00, (INT)"LOGICAL UNIT NOT SUPPORTED"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x00, (INT)"INVALID FIELD IN PARAMETER LIST"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x01, (INT)"PARAMETER NOT SUPPORTED"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x02, (INT)"PARAMETER VALUE INVALID"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x03, (INT)"THRESHOLD PARAMETERS NOT SUPPORTED"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x04, (INT)"INVALID RELEASE OF PERSISTENT RESERVATION"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x05, (INT)"DATA DECRYPTION ERROR"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x06, (INT)"TOO MANY TARGET DESCRIPTORS"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x07, (INT)"UNSUPPORTED SEGMENT DESCRIPTOR TYPE CODE"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x08, (INT)"TOO MANY SEGMENT DESCRIPTORS"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x09, (INT)"UNSUPPORTED SEGMENT DESCRIPTOR TYPE CODE"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x0A, (INT)"UNEXPECTED INEXACT SEGMENT"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x0B, (INT)"INLINE DATA LENGTH EXCEEDED"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x0C, (INT)"INVALID OPERATION FOR COPY SOURCE OR DESTINATION"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x0D, (INT)"COPY SEGMENT GRANULARITY VIOLATION"}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x0E, (INT)"INVALID PARAMETER WHILE PORT IS ENABLED"}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x00, (INT)"WRITE PROTECTED"}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x01, (INT)"HARDWARE WRITE PROTECTED"}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x02, (INT)"LOGICAL UNIT SOFTWARE WRITE PROTECTED"}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x03, (INT)"ASSOCIATED WRITE PROTECT"}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x04, (INT)"PERSISTENT WRITE PROTECT"}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x05, (INT)"PERMANENT WRITE PROTECT"}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x06, (INT)"CONDITIONAL WRITE PROTECT"}, 
		{SCSI_ADSENSE_MEDIUM_CHANGED, 0x00, (INT)"NOT READY TO READY CHANGE, MEDIUM MAY HAVE CHANGED"}, 
		{SCSI_ADSENSE_MEDIUM_CHANGED, 0x01, (INT)"IMPORT OR EXPORT ELEMENT ACCESSED"}, 
		{SCSI_ADSENSE_MEDIUM_CHANGED, 0x02, (INT)"FORMAT-LAYER MAY HAVE CHANGED"}, 
		{SCSI_ADSENSE_BUS_RESET, 0x00, (INT)"POWER ON, RESET, OR BUS DEVICE RESET OCCURRED"}, 
		{SCSI_ADSENSE_BUS_RESET, 0x01, (INT)"POWER ON OCCURRED"}, 
		{SCSI_ADSENSE_BUS_RESET, 0x02, (INT)"SCSI BUS RESET OCCURRED"}, 
		{SCSI_ADSENSE_BUS_RESET, 0x03, (INT)"BUS DEVICE RESET FUNCTION OCCURRED"}, 
		{SCSI_ADSENSE_BUS_RESET, 0x04, (INT)"DEVICE INTERNAL RESET"}, 
		{SCSI_ADSENSE_BUS_RESET, 0x05, (INT)"TRANSCEIVER MODE CHANGED TO SINGLE-ENDED"}, 
		{SCSI_ADSENSE_BUS_RESET, 0x06, (INT)"TRANSCEIVER MODE CHANGED TO LVD"}, 
		{SCSI_ADSENSE_BUS_RESET, 0x07, (INT)"I_T NEXUS LOSS OCCURRED"}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x00, (INT)"PARAMETERS CHANGED"}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x01, (INT)"MODE PARAMETERS CHANGED"}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x02, (INT)"LOG PARAMETERS CHANGED"}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x03, (INT)"RESERVATIONS PREEMPTED"}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x04, (INT)"RESERVATIONS RELEASED"}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x05, (INT)"REGISTRATIONS PREEMPTED"}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x06, (INT)"ASYMMETRIC ACCESS STATE CHANGED"}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x07, (INT)"IMPLICIT ASYMMETRIC ACCESS STATE TRANSITION FAILED"}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x08, (INT)"PRIORITY CHANGED"}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x14, (INT)"SA CREATION CAPABILITIES DATA HAS CHANGED"}, 
		{0x2B, 0x00, (INT)"COPY CANNOT EXECUTE SINCE HOST CANNOT DISCONNECT"}, 
		{0x2C, 0x00, (INT)"COMMAND SEQUENCE ERROR"}, 
		{0x2C, 0x03, (INT)"CURRENT PROGRAM AREA IS NOT EMPTY"}, 
		{0x2C, 0x04, (INT)"CURRENT PROGRAM AREA IS EMPTY"}, 
		{0x2C, 0x06, (INT)"PERSISTENT PREVENT CONFLICT"}, 
		{0x2C, 0x07, (INT)"PREVIOUS BUSY STATUS"}, 
		{0x2C, 0x08, (INT)"PREVIOUS TASK SET FULL STATUS"}, 
		{0x2C, 0x09, (INT)"PREVIOUS RESERVATION CONFLICT STATUS"}, 
		{SCSI_ADSENSE_INSUFFICIENT_TIME_FOR_OPERATION, 0x00, (INT)"INSUFFICIENT TIME FOR OPERATION"}, 
		{0x2F, 0x00, (INT)"COMMANDS CLEARED BY ANOTHER INITIATOR"}, 
		{0x2F, 0x02, (INT)"COMMANDS CLEARED BY DEVICE SERVER"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x00, (INT)"INCOMPATIBLE MEDIUM INSTALLED"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x01, (INT)"CANNOT READ MEDIUM - UNKNOWN FORMAT"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x02, (INT)"CANNOT READ MEDIUM - INCOMPATIBLE FORMAT"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x03, (INT)"CLEANING CARTRIDGE INSTALLED"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x04, (INT)"CANNOT WRITE MEDIUM - UNKNOWN FORMAT"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x05, (INT)"CANNOT WRITE MEDIUM - INCOMPATIBLE FORMAT"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x06, (INT)"CANNOT FORMAT MEDIUM - INCOMPATIBLE MEDIUM"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x07, (INT)"CLEANING FAILURE"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x08, (INT)"CANNOT WRITE - APPLICATION CODE MISMATCH"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x09, (INT)"CURRENT SESSION NOT FIXATED FOR APPEND"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x0A, (INT)"CLEANING REQUEST REJECTED"}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x10, (INT)"MEDIUM NOT FORMATTED"}, 
		{0x31, 0x00, (INT)"MEDIUM FORMAT CORRUPTED"}, 
		{0x31, 0x01, (INT)"FORMAT COMMAND FAILED"}, 
		{0x31, 0x02, (INT)"ZONED FORMATTING FAILED DUE TO SPARE LINKING"}, 
		{0x34, 0x00, (INT)"ENCLOSURE FAILURE"}, 
		{0x35, 0x00, (INT)"ENCLOSURE SERVICES FAILURE"}, 
		{0x35, 0x01, (INT)"UNSUPPORTED ENCLOSURE FUNCTION"}, 
		{0x35, 0x02, (INT)"ENCLOSURE SERVICES UNAVAILABLE"}, 
		{0x35, 0x03, (INT)"ENCLOSURE SERVICES TRANSFER FAILURE"}, 
		{0x35, 0x04, (INT)"ENCLOSURE SERVICES TRANSFER REFUSED"}, 
		{0x35, 0x05, (INT)"ENCLOSURE SERVICES CHECKSUM ERROR"}, 
		{0x37, 0x00, (INT)"ROUNDED PARAMETER"}, 
		{0x39, 0x00, (INT)"SAVING PARAMETERS NOT SUPPORTED"}, 
		{SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0x00, (INT)"MEDIUM NOT PRESENT"}, 
		{SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0x01, (INT)"MEDIUM NOT PRESENT - TRAY CLOSED"}, 
		{SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0x02, (INT)"MEDIUM NOT PRESENT - TRAY OPEN"}, 
		{SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0x03, (INT)"MEDIUM NOT PRESENT - LOADABLE"}, 
		{SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0x04, (INT)"MEDIUM NOT PRESENT - MEDIUM AUXILIARY MEMORY ACCESSIBLE"}, 
		{SCSI_ADSENSE_POSITION_ERROR, SCSI_SENSEQ_DESTINATION_FULL, (INT)"MEDIUM DESTINATION ELEMENT FULL"}, 
		{SCSI_ADSENSE_POSITION_ERROR, SCSI_SENSEQ_SOURCE_EMPTY, (INT)"MEDIUM SOURCE ELEMENT EMPTY"}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x0F, (INT)"END OF MEDIUM REACHED"}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x11, (INT)"MEDIUM MAGAZINE NOT ACCESSIBLE"}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x12, (INT)"MEDIUM MAGAZINE REMOVED"}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x13, (INT)"MEDIUM MAGAZINE INSERTED"}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x14, (INT)"MEDIUM MAGAZINE LOCKED"}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x15, (INT)"MEDIUM MAGAZINE UNLOCKED"}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x16, (INT)"MECHANICAL POSITIONING OR CHANGER ERROR"}, 
		{0x3D, 0x00, (INT)"INVALID BITS IN IDENTIFY MESSAGE"}, 
		{0x3E, 0x00, (INT)"LOGICAL UNIT HAS NOT SELF-CONFIGURED YET"}, 
		{0x3E, 0x01, (INT)"LOGICAL UNIT FAILURE"}, 
		{0x3E, 0x02, (INT)"TIMEOUT ON LOGICAL UNIT"}, 
		{0x3E, 0x03, (INT)"LOGICAL UNIT FAILED SELF-TEST"}, 
		{0x3E, 0x04, (INT)"LOGICAL UNIT UNABLE TO UPDATE SELF-TEST LOG"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_TARGET_OPERATING_CONDITIONS_CHANGED, (INT)"TARGET OPERATING CONDITIONS HAVE CHANGED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_MICROCODE_CHANGED, (INT)"MICROCODE HAS BEEN CHANGED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_OPERATING_DEFINITION_CHANGED, (INT)"CHANGED OPERATING DEFINITION"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_INQUIRY_DATA_CHANGED, (INT)"INQUIRY DATA HAS CHANGED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_COMPONENT_DEVICE_ATTACHED, (INT)"COMPONENT DEVICE ATTACHED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_DEVICE_IDENTIFIER_CHANGED, (INT)"DEVICE IDENTIFIER CHANGED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_REDUNDANCY_GROUP_MODIFIED, (INT)"REDUNDANCY GROUP CREATED OR MODIFIED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_REDUNDANCY_GROUP_DELETED, (INT)"REDUNDANCY GROUP DELETED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_SPARE_MODIFIED, (INT)"SPARE CREATED OR MODIFIED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_SPARE_DELETED, (INT)"SPARE DELETED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_VOLUME_SET_MODIFIED, (INT)"VOLUME SET CREATED OR MODIFIED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_VOLUME_SET_DELETED, (INT)"VOLUME SET DELETED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_VOLUME_SET_DEASSIGNED, (INT)"VOLUME SET DEASSIGNED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_VOLUME_SET_REASSIGNED, (INT)"VOLUME SET REASSIGNED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_REPORTED_LUNS_DATA_CHANGED, (INT)"REPORTED LUNS DATA HAS CHANGED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_ECHO_BUFFER_OVERWRITTEN, (INT)"ECHO pBuf OVERWRITTEN"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_MEDIUM_LOADABLE, (INT)"MEDIUM LOADABLE"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_MEDIUM_AUXILIARY_MEMORY_ACCESSIBLE, (INT)"MEDIUM AUXILIARY MEMORY ACCESSIBLE"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, 0x12, (INT)"iSCSI IP ADDRESS ADDED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, 0x13, (INT)"iSCSI IP ADDRESS REMOVED"}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, 0x14, (INT)"iSCSI IP ADDRESS CHANGED"}, 
		{0x43, 0x00, (INT)"MESSAGE ERROR"}, 
		{0x44, 0x00, (INT)"INTERNAL TARGET FAILURE"}, 
		{0x45, 0x00, (INT)"SELECT OR RESELECT FAILURE"}, 
		{0x46, 0x00, (INT)"UNSUCCESSFUL SOFT RESET"}, 
		{0x47, 0x00, (INT)"SCSI PARITY ERROR"}, 
		{0x47, 0x01, (INT)"DATA PHASE CRC ERROR DETECTED"}, 
		{0x47, 0x02, (INT)"SCSI PARITY ERROR DETECTED DURING ST DATA PHASE"}, 
		{0x47, 0x03, (INT)"INFORMATION UNIT iuCRC ERROR DETECTED"}, 
		{0x47, 0x04, (INT)"ASYNCHRONOUS INFORMATION PROTECTION ERROR DETECTED"}, 
		{0x47, 0x05, (INT)"PROTOCOL SERVICE CRC ERROR"}, 
		{0x47, 0x7F, (INT)"SOME COMMANDS CLEARED BY ISCSI PROTOCOL EVENT"}, 
		{0x48, 0x00, (INT)"INITIATOR DETECTED ERROR MESSAGE RECEIVED"}, 
		{0x49, 0x00, (INT)"INVALID MESSAGE ERROR"}, 
		{0x4A, 0x00, (INT)"COMMAND PHASE ERROR"}, 
		{0x4B, 0x00, (INT)"DATA PHASE ERROR"}, 
		{0x4B, 0x01, (INT)"INVALID TARGET PORT TRANSFER TAG RECEIVED"}, 
		{0x4B, 0x02, (INT)"TOO MUCH WRITE DATA"}, 
		{0x4B, 0x03, (INT)"ACK/NAK TIMEOUT"}, 
		{0x4B, 0x04, (INT)"NAK RECEIVED"}, 
		{0x4B, 0x05, (INT)"DATA OFFSET ERROR"}, 
		{0x4B, 0x06, (INT)"INITIATOR RESPONSE TIMEOUT"}, 
		{0x4B, 0x07, (INT)"CONNECTION LOST"}, 
		{0x4C, 0x00, (INT)"LOGICAL UNIT FAILED SELF-CONFIGURATION"}, 
		{0x4E, 0x00, (INT)"OVERLAPPED COMMANDS ATTEMPTED"}, 
		{0x51, 0x00, (INT)"ERASE FAILURE"}, 
		{0x51, 0x01, (INT)"ERASE FAILURE - INCOMPLETE ERASE OPERATION DETECTED"}, 
		{0x53, 0x00, (INT)"MEDIA LOAD OR EJECT FAILED"}, 
		{0x53, 0x02, (INT)"MEDIUM REMOVAL PREVENTED"}, 
		{0x55, 0x02, (INT)"INSUFFICIENT RESERVATION RESOURCES"}, 
		{0x55, 0x03, (INT)"INSUFFICIENT RESOURCES"}, 
		{0x55, 0x04, (INT)"INSUFFICIENT REGISTRATION RESOURCES"}, 
		{0x55, 0x05, (INT)"INSUFFICIENT ACCESS CONTROL RESOURCES"}, 
		{0x55, 0x06, (INT)"AUXILIARY MEMORY OUT OF SPACE"}, 
		{0x55, 0x0B, (INT)"INSUFFICIENT POWER FOR OPERATION"}, 
		{0x57, 0x00, (INT)"UNABLE TO RECOVER TABLE-OF-CONTENTS"}, 
		{SCSI_ADSENSE_OPERATOR_REQUEST, SCSI_SENSEQ_STATE_CHANGE_INPUT, (INT)"OPERATOR REQUEST OR STATE CHANGE INPUT"}, 
		{SCSI_ADSENSE_OPERATOR_REQUEST, SCSI_SENSEQ_MEDIUM_REMOVAL, (INT)"OPERATOR MEDIUM REMOVAL REQUEST"}, 
		{SCSI_ADSENSE_OPERATOR_REQUEST, SCSI_SENSEQ_WRITE_PROTECT_ENABLE, (INT)"OPERATOR SELECTED WRITE PROTECT"}, 
		{SCSI_ADSENSE_OPERATOR_REQUEST, SCSI_SENSEQ_WRITE_PROTECT_DISABLE, (INT)"OPERATOR SELECTED WRITE PERMIT"}, 
		{0x5b, 0x00, (INT)"LOG EXCEPTION"}, 
		{0x5b, 0x01, (INT)"THRESHOLD CONDITION MET"}, 
		{0x5b, 0x02, (INT)"LOG COUNTER AT MAXIMUM"}, 
		{0x5b, 0x03, (INT)"LOG LIST CODES EXHAUSTED"}, 
		{SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED, 0x00, (INT)"FAILURE PREDICTION THRESHOLD EXCEEDED"}, 
		{SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED, 0x01, (INT)"MEDIA FAILURE PREDICTION THRESHOLD EXCEEDED"}, 
		{SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED, 0x02, (INT)"LOGICAL UNIT FAILURE PREDICTION THRESHOLD EXCEEDED"}, 
		{SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED, 0x03, (INT)"SPARE AREA EXHAUSTION PREDICTION THRESHOLD EXCEEDED"}, 
		{SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED, 0xFF, (INT)"FAILURE PREDICTION THRESHOLD EXCEEDED (FALSE)"}, 
		{0x5E, 0x00, (INT)"LOW POWER CONDITION ON"}, 
		{0x5E, 0x01, (INT)"IDLE CONDITION ACTIVATED BY TIMER"}, 
		{0x5E, 0x02, (INT)"STANDBY CONDITION ACTIVATED BY TIMER"}, 
		{0x5E, 0x03, (INT)"IDLE CONDITION ACTIVATED BY COMMAND"}, 
		{0x5E, 0x04, (INT)"STANDBY CONDITION ACTIVATED BY COMMAND"}, 
		{0x5E, 0x05, (INT)"IDLE_B CONDITION ACTIVATED BY TIMER"}, 
		{0x5E, 0x06, (INT)"IDLE_B CONDITION ACTIVATED BY COMMAND"}, 
		{0x5E, 0x07, (INT)"IDLE_C CONDITION ACTIVATED BY TIMER"}, 
		{0x5E, 0x08, (INT)"IDLE_C CONDITION ACTIVATED BY COMMAND"}, 
		{0x5E, 0x09, (INT)"STANDBY_Y CONDITION ACTIVATED BY TIMER"}, 
		{0x5E, 0x0A, (INT)"STANDBY_Y CONDITION ACTIVATED BY COMMAND"}, 
		{0x63, 0x00, (INT)"END OF USER AREA ENCOUNTERED ON THIS TRACK"}, 
		{0x63, 0x01, (INT)"PACKET DOES NOT FIT IN AVAILABLE SPACE"}, 
		{SCSI_ADSENSE_ILLEGAL_MODE_FOR_THIS_TRACK, 0x00, (INT)"ILLEGAL MODE FOR THIS TRACK"}, 
		{SCSI_ADSENSE_ILLEGAL_MODE_FOR_THIS_TRACK, 0x01, (INT)"INVALID PACKET SIZE"}, 
		{0x65, 0x00, (INT)"VOLTAGE FAULT"}, 
		{0x67, 0x0A, (INT)"SET TARGET PORT GROUPS COMMAND FAILED"}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_AUTHENTICATION_FAILURE, (INT)"COPY PROTECTION KEY EXCHANGE FAILURE - AUTHENTICATION FAILURE"}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_KEY_NOT_PRESENT, (INT)"COPY PROTECTION KEY EXCHANGE FAILURE - KEY NOT PRESENT"}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_KEY_NOT_ESTABLISHED, (INT)"COPY PROTECTION KEY EXCHANGE FAILURE - KEY NOT ESTABLISHED"}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_READ_OF_SCRAMBLED_SECTOR_WITHOUT_AUTHENTICATION, (INT)"READ OF SCRAMBLED SECTOR WITHOUT AUTHENTICATION"}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_MEDIA_CODE_MISMATCHED_TO_LOGICAL_UNIT, (INT)"MEDIA REGION CODE IS MISMATCHED TO LOGICAL UNIT REGION"}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_LOGICAL_UNIT_RESET_COUNT_ERROR, (INT)"DRIVE REGION MUST BE PERMANENT/REGION RESET COUNT ERROR"}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, 0x06, (INT)"INSUFFICIENT BLOCK COUNT FOR BINDING NONCE RECORDING"}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, 0x07, (INT)"CONFLICT IN BINDING NONCE RECORDING"}, 
		{0x72, 0x00, (INT)"SESSION FIXATION ERROR"}, 
		{0x72, 0x01, (INT)"SESSION FIXATION ERROR WRITING LEAD-IN"}, 
		{0x72, 0x02, (INT)"SESSION FIXATION ERROR WRITING LEAD-OUT"}, 
		{0x72, 0x03, (INT)"SESSION FIXATION ERROR - INCOMPLETE TRACK IN SESSION"}, 
		{0x72, 0x04, (INT)"EMPTY OR PARTIALLY WRITTEN RESERVED TRACK"}, 
		{0x72, 0x05, (INT)"NO MORE TRACK RESERVATIONS ALLOWED"}, 
		{0x72, 0x06, (INT)"RMZ EXTENSION IS NOT ALLOWED"}, 
		{0x72, 0x07, (INT)"NO MORE TEST ZONE EXTENSIONS ARE ALLOWED"}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, 0x00, (INT)"VOLTAGE FAULT"}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_POWER_CALIBRATION_AREA_ALMOST_FULL, (INT)"CD CONTROL ERROR"}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_POWER_CALIBRATION_AREA_FULL, (INT)"POWER CALIBRATION AREA ALMOST FULL"}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_POWER_CALIBRATION_AREA_ERROR, (INT)"POWER CALIBRATION AREA IS FULL"}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_PMA_RMA_UPDATE_FAILURE, (INT)"POWER CALIBRATION AREA ERROR"}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_PMA_RMA_IS_FULL, (INT)"PROGRAM MEMORY AREA UPDATE FAILURE"}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_PMA_RMA_ALMOST_FULL, (INT)"RMA/PMA IS ALMOST FULL"}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, 0x10, (INT)"CURRENT POWER CALIBRATION AREA ALMOST FULL"}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, 0x11, (INT)"CURRENT POWER CALIBRATION AREA IS FULL"}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, 0x17, (INT)"RDZ IS FULL"}, 
		{0x74, 0x08, (INT)"DIGITAL SIGNATURE VALIDATION FAILURE"}, 
		{0x74, 0x0C, (INT)"UNABLE TO DECRYPT PARAMETER LIST"}, 
		{0x74, 0x10, (INT)"SA CREATION PARAMETER VALUE INVALID"}, 
		{0x74, 0x11, (INT)"SA CREATION PARAMETER VALUE REJECTED"}, 
		{0x74, 0x12, (INT)"INVALID SA USAGE"}, 
		{0x74, 0x30, (INT)"SA CREATION PARAMETER NOT SUPPORTED"}, 
		{0x74, 0x40, (INT)"AUTHENTICATION FAILED"}, 
		{0x74, 0x71, (INT)"LOGICAL UNIT ACCESS NOT AUTHORIZED"} 
	};
#endif

void OutputSense(
	UCHAR byKey,
	UCHAR byAsc,
	UCHAR byAscq
	)
{
	OutputErrorStringA("Sense data, Key:Asc:Ascq: %02X:%02X:%02X", byKey, byAsc, byAscq);

	for(INT i = 0; i < sizeof(aSenseKey) / sizeof(INT) / 2; i++) {
		if(byKey == aSenseKey[i][0]) {
			OutputErrorStringA("(%s.", (CHAR*)aSenseKey[i][1]);
			break;
		}
	}
	BOOL bRet = FALSE;
	for(INT i = 0; i < sizeof(aSenseAscAscq) / sizeof(INT) / 3 - 1; i++) {
		if(byAsc == aSenseAscAscq[i][0]) {
			do {
				if(byAscq == aSenseAscAscq[i][1]) {
					OutputErrorStringA(" %s)", (CHAR*)aSenseAscAscq[i][2]);
					bRet = TRUE;
					break;
				}
				else {
					i++;
				}
			} while(aSenseAscAscq[i][1] < aSenseAscAscq[i+1][1]);
		}
		if(bRet) {
			break;
		}
	}

	if(byAsc >= SCSI_ADSENSE_VENDOR_UNIQUE || byAscq >= 0x80) {
		OutputErrorStringA(" VENDER UNIQUE ERROR)");
	}
	OutputErrorStringA("\n");
}

void OutputSub96(
	CONST PUCHAR pBuf,
	INT nLBA,
	FILE* fpLog
	)
{
	OutputLogStringA(fpLog, "Sub Channel LBA %d\n", nLBA);
	OutputLogStringA(fpLog, "\t  +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B\n");

	for(INT i = 0, ch = 0x50; i < CD_RAW_READ_SUBCODE_SIZE; i += 12, ch++) {
		OutputLogStringA(fpLog, 
			"\t%c %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n", 
			ch,	pBuf[i], pBuf[i+1], pBuf[i+2], pBuf[i+3], pBuf[i+4], pBuf[i+5],
			pBuf[i+6], pBuf[i+7], pBuf[i+8], pBuf[i+9], pBuf[i+10], pBuf[i+11]);
	}
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputSubcode(
	INT nLBA,
	INT nTrackNum,
	CONST PUCHAR Subcode,
	CONST PUCHAR SubcodeRtoW,
	FILE* fpParse
	)
{
	CHAR str[256];
	ZeroMemory(str, sizeof(str));
	sprintf(str, "LBA[%06d, 0x%05X], ", nLBA, nLBA);
	// Ctl
	switch((Subcode[12] >> 4) & 0x0F) {
	case 0:
		strcat(str, "Audio, 2ch, Copy NG, Pre-emphasis No, ");
		break;
	case AUDIO_WITH_PREEMPHASIS:
		strcat(str, "Audio, 2ch, Copy NG, Pre-emphasis 50/15, ");
		break;
	case DIGITAL_COPY_PERMITTED:
		strcat(str, "Audio, 2ch, Copy OK, Pre-emphasis No, ");
		break;
	case DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
		strcat(str, "Audio, 2ch, Copy OK, Pre-emphasis 50/15, ");
		break;
	case AUDIO_DATA_TRACK:
		strcat(str, "Data, Copy NG, ");
		break;
	case AUDIO_DATA_TRACK | DIGITAL_COPY_PERMITTED:
		strcat(str, "Data, Copy OK, ");
		break;
	case TWO_FOUR_CHANNEL_AUDIO:
		strcat(str, "Audio, 4ch, Copy NG, Pre-emphasis No, ");
		break;
	case TWO_FOUR_CHANNEL_AUDIO | AUDIO_WITH_PREEMPHASIS:
		strcat(str, "Audio, 4ch, Copy NG, Pre-emphasis 50/15, ");
		break;
	case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED:
		strcat(str, "Audio, 4ch, Copy OK, Pre-emphasis No, ");
		break;
	case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
		strcat(str, "Audio, 4ch, Copy OK, Pre-emphasis 50/15, ");
		break;
	default:
		strcat(str, "Unknown, ");
		break;
	}

	// ADR
	CHAR str2[128];
	ZeroMemory(str2, sizeof(str2));
	switch(Subcode[12] & 0x0F) {
	case ADR_ENCODES_CURRENT_POSITION:
		if(Subcode[13] == 0xAA) {
			sprintf(str2, 
				"TOC[LeadOut    , Index-%02x, RelativeTime-%02x:%02x:%02x, AbsoluteTime-%02x:%02x:%02x] ", 
				Subcode[14], Subcode[15], Subcode[16], Subcode[17], 
				Subcode[19], Subcode[20], Subcode[21]);
		}
		else {
			sprintf(str2, 
				"TOC[TrackNum-%02x, Index-%02x, RelativeTime-%02x:%02x:%02x, AbsoluteTime-%02x:%02x:%02x] ", 
				Subcode[13], Subcode[14], Subcode[15], Subcode[16], 
				Subcode[17], Subcode[19], Subcode[20], Subcode[21]);
		}
		break;
	case ADR_ENCODES_MEDIA_CATALOG:
		sprintf(str2, 
			"Media Catalog Number (MCN)[%s        , AbsoluteTime-     :%02x] ", 
			szCatalog, Subcode[21]);
		break;
	case ADR_ENCODES_ISRC:
		sprintf(str2, 
			"International Standard... (ISRC)[%s   , AbsoluteTime-     :%02x] ", 
			szISRC[nTrackNum-1], Subcode[21]);
		break;
	default:
		sprintf(str2, 
			"TOC[TrackNum-%02x, Index-%02x, RelativeTime-%02x:%02x:%02x, AbsoluteTime-%02x:%02x:%02x] ", 
			Subcode[13], Subcode[14], Subcode[15], Subcode[16], 
			Subcode[17], Subcode[19], Subcode[20], Subcode[21]);
		break;
	}
	strcat(str, str2);

//	for(INT a = 0; a < CD_RAW_READ_SUBCODE_SIZE; a += 24) {
		switch(SubcodeRtoW[0] & 0x3F) {
		case 0:
			strcat(str, "RtoW:ZERO mode\n");
			break;
		case 8:
			strcat(str, "RtoW:LINE-GRAPHICS mode\n");
			break;
		case 9:
			strcat(str, "RtoW:TV-GRAPHICS mode\n");
			break;
		case 10:
			strcat(str, "RtoW:EXTENDED-TV-GRAPHICS mode\n");
			break;
		case 20:
			strcat(str, "RtoW:CD TEXT mode\n");
			break;
		case 24:
			strcat(str, "RtoW:MIDI mode\n");
			break;
		case 56:
			strcat(str, "RtoW:USER mode\n");
			break;
		default:
			strcat(str, "\n");
			break;
		}
//	}
	fwrite(str, sizeof(UCHAR), strlen(str), fpParse);
}

// begin for CD
void OutputVolumeDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	OutputLogStringA(fpLog, "Volume Descriptor\n");
	// 0 is Boot Record. 
	// 1 is Primary Volume Descriptor. 
	// 2 is Supplementary Volume Descriptor. 
	// 3 is Volume Partition Descriptor. 
	// 4-254 is reserved. 
	// 255 is Volume Descriptor Set Terminator.
	OutputLogStringA(fpLog, "\tVolume Descriptor Type		%d\n", buf[idx]);
	CHAR str[CD_RAW_READ+1];
	ZeroMemory(str, sizeof(str));
	strncpy(str, (CHAR*)&buf[idx+1], 5); str[5] = '\0';
	OutputLogStringA(fpLog, "\tStandard Identifier			[%s]\n", str);
	OutputLogStringA(fpLog, "\tVolume Descriptor Version	%d\n", buf[idx+6]);
	
	if(buf[idx] == 0) {
		OutputBootRecord(idx, buf, fpLog);
	}
	else if(buf[idx] == 1 || buf[idx] == 2) {
		OutputPrimaryVolumeDescriptorForISO9660(idx, buf, fpLog);
	}
	else if(buf[idx] == 3) {
		OutputVolumePartitionDescriptor(idx, buf, fpLog);
	}
	else if(buf[idx] == 255) {
	}
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputBootRecord(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	CHAR str[CD_RAW_READ+1];
	ZeroMemory(str, sizeof(str));
	strncpy(str, (CHAR*)&buf[idx+7], 32); str[32] = '\0';
	OutputLogStringA(fpLog, "\tBoot System Identifier		[%s]\n", str);
	strncpy(str, (CHAR*)&buf[idx+39], 32); str[32] = '\0';
	OutputLogStringA(fpLog, "\tBoot Identifier				[%s]\n", str);
	OutputLogStringA(fpLog, "\tBoot System Use	");
	for(INT i = 71; i <= 2047; i++) {
		OutputLogStringA(fpLog, "%x", buf[idx+i]);
	}
	OutputLogStringA(fpLog, "\n");
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputPrimaryVolumeDescriptorForISO9660(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	CHAR str[CD_RAW_READ+1];
	ZeroMemory(str, sizeof(str));
	if(buf[idx] == 2) {
		OutputLogStringA(fpLog, 
			"\tVolume Flags					%d\n", buf[idx+7]);
	}
	strncpy(str, (CHAR*)&buf[idx+8], 32); str[32] = '\0';
	OutputLogStringA(fpLog, "\tSystem Identifier		[%s]\n", str);
	strncpy(str, (CHAR*)&buf[idx+40], 32); str[32] = '\0';
	OutputLogStringA(fpLog, "\tVolume Identifier		[%s]\n", str);
	OutputLogStringA(fpLog, "\tVolume Space Size		%d\n", buf[idx+80]);
	if(buf[idx+0] == 2) {
		strncpy(str, (CHAR*)&buf[idx+88], 32); str[32] = '\0';
		OutputLogStringA(fpLog, "\tEscape Sequences		[%s]\n", str);
	}
	OutputLogStringA(fpLog, "\tVolume Set Size			%d\n", 
		MAKEWORD(buf[idx+123], buf[idx+122]));
	OutputLogStringA(fpLog, "\tVolume Sequence Number	%d\n", 
		MAKEWORD(buf[idx+127], buf[idx+126]));
	OutputLogStringA(fpLog, "\tLogical Block Size		%d\n", 
		MAKEWORD(buf[idx+131], buf[idx+130]));
	OutputLogStringA(fpLog, "\tPath Table Size			%d\n", 
		MAKELONG(MAKEWORD(buf[idx+139], buf[idx+138]), 
		MAKEWORD(buf[idx+137], buf[idx+136])));
	OutputLogStringA(fpLog, 
		"\tLocation of Occurrence of Type L Path Table				%d\n", 
		MAKEWORD(buf[idx+143], buf[idx+142]));
	OutputLogStringA(fpLog, 
		"\tLocation of Optional Occurrence of Type L Path Table	%d\n", 
		MAKEWORD(buf[idx+147], buf[idx+146]));
	OutputLogStringA(fpLog, 
		"\tLocation of Occurrence of Type M Path Table				%d\n", 
		MAKEWORD(buf[idx+151], buf[idx+150]));
	OutputLogStringA(fpLog, 
		"\tLocation of Optional Occurrence of Type M Path Table	%d\n", 
		MAKEWORD(buf[idx+155], buf[idx+154]));

	OutputLogStringA(fpLog, "\tDirectory Record\n");
	OutputLogStringA(fpLog, 
		"\t\tLength of Directory Record			%d\n", buf[idx+156]);
	OutputLogStringA(fpLog, 
		"\t\tExtended Attribute Record Length	%d\n", buf[idx+157]);
	OutputLogStringA(fpLog, 
		"\t\tLocation of Extent					%d\n", 
		MAKELONG(MAKEWORD(buf[idx+165], buf[idx+164]), 
		MAKEWORD(buf[idx+163], buf[idx+162])));
	OutputLogStringA(fpLog, 
		"\t\tData Length							%d\n", 
		MAKELONG(MAKEWORD(buf[idx+173], buf[idx+172]), 
		MAKEWORD(buf[idx+171], buf[idx+170])));
	OutputLogStringA(fpLog, 
		"\t\tRecording Date and Time				%d-%02d-%02d %02d:%02d:%02d +%02d\n", 
		buf[idx+174] + 1900, buf[idx+175], buf[idx+176], 
		buf[idx+177], buf[idx+178], buf[idx+179], buf[idx+180]);
	OutputLogStringA(fpLog, 
		"\t\tFile Flags							%d\n", buf[idx+181]);
	OutputLogStringA(fpLog, 
		"\t\tFile Unit Size						%d\n", buf[idx+182]);
	OutputLogStringA(fpLog, 
		"\t\tInterleave Gap Size					%d\n", buf[idx+183]);
	OutputLogStringA(fpLog, 
		"\t\tVolume Sequence Number				%d\n", 
		MAKEWORD(buf[idx+187], buf[idx+186]));
	OutputLogStringA(fpLog, 
		"\t\tLength of File Identifier			%d\n", buf[idx+188]);
	OutputLogStringA(fpLog, 
		"\t\tFile Identifier						%d\n", buf[idx+189]);
	strncpy(str, (CHAR*)&buf[idx+190], 128); str[128] = '\0';
	OutputLogStringA(fpLog, "\tVolume Set Identifier		[%s]\n", str);
	strncpy(str, (CHAR*)&buf[idx+318], 128); str[128] = '\0';
	OutputLogStringA(fpLog, "\tPublisher Identifier		[%s]\n", str);
	strncpy(str, (CHAR*)&buf[idx+446], 128); str[128] = '\0';
	OutputLogStringA(fpLog, "\tData Preparer Identifier	[%s]\n", str);
	strncpy(str, (CHAR*)&buf[idx+574], 128); str[128] = '\0';
	OutputLogStringA(fpLog, "\tApplication Identifier		[%s]\n", str);
	strncpy(str, (CHAR*)&buf[idx+702], 37); str[37] = '\0';
	OutputLogStringA(fpLog, "\tCopyright File Identifier		[%s]\n", str);
	strncpy(str, (CHAR*)&buf[idx+702], 37); str[37] = '\0';
	OutputLogStringA(fpLog, "\tAbstract File Identifier		[%s]\n", str);
	strncpy(str, (CHAR*)&buf[idx+776], 37); str[37] = '\0';
	OutputLogStringA(fpLog, "\tBibliographic File Identifier	[%s]\n", str);

	CHAR year[4+1], month[2+1], day[2+1], hour[2+1], time[2+1], second[2+1], milisecond[2+1];
	strncpy(year, (CHAR*)&buf[idx+813], 4); year[4] = '\0';
	strncpy(month, (CHAR*)&buf[idx+817], 2); month[2] = '\0';
	strncpy(day, (CHAR*)&buf[idx+819], 2); day[2] = '\0';
	strncpy(hour, (CHAR*)&buf[idx+821], 2); hour[2] = '\0';
	strncpy(time, (CHAR*)&buf[idx+823], 2); time[2] = '\0';
	strncpy(second, (CHAR*)&buf[idx+825], 2); second[2] = '\0';
	strncpy(milisecond, (CHAR*)&buf[idx+827], 2); milisecond[2] = '\0';
	OutputLogStringA(fpLog, 
		"\tVolume Creation Date and Time		%s-%s-%s %s:%s:%s.%s +%d\n", 
		year, month, day, hour, time, second, milisecond, buf[idx+829]);
	strncpy(year, (CHAR*)&buf[idx+830], 4); year[4] = '\0';
	strncpy(month, (CHAR*)&buf[idx+834], 2); month[2] = '\0';
	strncpy(day, (CHAR*)&buf[idx+836], 2); day[2] = '\0';
	strncpy(hour, (CHAR*)&buf[idx+838], 2); hour[2] = '\0';
	strncpy(time, (CHAR*)&buf[idx+840], 2); time[2] = '\0';
	strncpy(second, (CHAR*)&buf[idx+842], 2); second[2] = '\0';
	strncpy(milisecond, (CHAR*)&buf[idx+844], 2); milisecond[2] = '\0';
	OutputLogStringA(fpLog, 
		"\tVolume Modification Date and Time	%s-%s-%s %s:%s:%s.%s +%d\n", 
		year, month, day, hour, time, second, milisecond, buf[idx+846]);
	strncpy(year, (CHAR*)&buf[idx+847], 4); year[4] = '\0';
	strncpy(month, (CHAR*)&buf[idx+851], 2); month[2] = '\0';
	strncpy(day, (CHAR*)&buf[idx+853], 2); day[2] = '\0';
	strncpy(hour, (CHAR*)&buf[idx+855], 2); hour[2] = '\0';
	strncpy(time, (CHAR*)&buf[idx+857], 2); time[2] = '\0';
	strncpy(second, (CHAR*)&buf[idx+859], 2); second[2] = '\0';
	strncpy(milisecond, (CHAR*)&buf[idx+861], 2); milisecond[2] = '\0';
	OutputLogStringA(fpLog, 
		"\tVolume Expiration Date and Time		%s-%s-%s %s:%s:%s.%s +%d\n", 
		year, month, day, hour, time, second, milisecond, buf[idx+863]);
	strncpy(year, (CHAR*)&buf[idx+864], 4); year[4] = '\0';
	strncpy(month, (CHAR*)&buf[idx+868], 2); month[2] = '\0';
	strncpy(day, (CHAR*)&buf[idx+870], 2); day[2] = '\0';
	strncpy(hour, (CHAR*)&buf[idx+872], 2); hour[2] = '\0';
	strncpy(time, (CHAR*)&buf[idx+874], 2); time[2] = '\0';
	strncpy(second, (CHAR*)&buf[idx+876], 2); second[2] = '\0';
	strncpy(milisecond, (CHAR*)&buf[idx+878], 2); milisecond[2] = '\0';
	OutputLogStringA(fpLog, 
		"\tVolume Effective Date and Time		%s-%s-%s %s:%s:%s.%s +%d\n", 
		year, month, day, hour, time, second, milisecond, buf[idx+880]);
	OutputLogStringA(fpLog, 
		"\tFile Structure Version				%d\n", buf[idx+881]);
	OutputLogStringA(fpLog, "\tApplication Use	");
	for(INT i = 883; i <= 1394; i++) {
		OutputLogStringA(fpLog, "%x", buf[idx+i]);
	}
	OutputLogStringA(fpLog, "\n");
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputPrimaryVolumeDescriptorForJoliet(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	_TCHAR str[64+1];
	ZeroMemory(str, sizeof(str));
	LittleToBig(str, (_TCHAR*)&buf[idx+8], 16); str[16] = '\0';
	OutputLogStringW(fpLog, _T("\tSystem Identifier		[%s]\n"), str);
	LittleToBig(str, (_TCHAR*)&buf[idx+40], 16); str[16] = '\0';
	OutputLogStringW(fpLog, _T("\tVolume Identifier		[%s]\n"), str);
	OutputLogStringA(fpLog, "\tVolume Space Size		%d\n", buf[idx+80]);
	if(buf[idx+0] == 2) {
		CHAR str2[32+1];
		strncpy(str2, (CHAR*)&buf[idx+88], 32); str2[32] = '\0';
		OutputLogStringA(fpLog, "\tEscape Sequences		[%s]\n", str2);
	}
	OutputLogStringA(fpLog, "\tVolume Set Size			%d\n", 
		MAKEWORD(buf[idx+123], buf[idx+122]));
	OutputLogStringA(fpLog, "\tVolume Sequence Number	%d\n", 
		MAKEWORD(buf[idx+127], buf[idx+126]));
	OutputLogStringA(fpLog, "\tLogical Block Size		%d\n", 
		MAKEWORD(buf[idx+131], buf[idx+130]));
	OutputLogStringA(fpLog, "\tPath Table Size			%d\n", 
		MAKELONG(MAKEWORD(buf[idx+139], buf[idx+138]), 
		MAKEWORD(buf[idx+137], buf[idx+136])));
	OutputLogStringA(fpLog, 
		"\tLocation of Occurrence of Type L Path Table				%d\n", 
		MAKEWORD(buf[idx+143], buf[idx+142]));
	OutputLogStringA(fpLog, 
		"\tLocation of Optional Occurrence of Type L Path Table	%d\n", 
		MAKEWORD(buf[idx+147], buf[idx+146]));
	OutputLogStringA(fpLog, 
		"\tLocation of Occurrence of Type M Path Table				%d\n", 
		MAKEWORD(buf[idx+151], buf[idx+150]));
	OutputLogStringA(fpLog, 
		"\tLocation of Optional Occurrence of Type M Path Table	%d\n", 
		MAKEWORD(buf[idx+155], buf[idx+154]));

	OutputLogStringA(fpLog, "\tDirectory Record\n");
	OutputLogStringA(fpLog, 
		"\t\tLength of Directory Record			%d\n", buf[idx+156]);
	OutputLogStringA(fpLog, 
		"\t\tExtended Attribute Record Length	%d\n", buf[idx+157]);
	OutputLogStringA(fpLog, 
		"\t\tLocation of Extent					%d\n", 
		MAKELONG(MAKEWORD(buf[idx+165], buf[idx+164]), 
		MAKEWORD(buf[idx+163], buf[idx+162])));
	OutputLogStringA(fpLog, 
		"\t\tData Length							%d\n", 
		MAKELONG(MAKEWORD(buf[idx+173], buf[idx+172]), 
		MAKEWORD(buf[idx+171], buf[idx+170])));
	OutputLogStringA(fpLog, 
		"\t\tRecording Date and Time				%d-%02d-%02d %02d:%02d:%02d +%02d\n", 
		buf[idx+174] + 1900, buf[idx+175], buf[idx+176], 
		buf[idx+177], buf[idx+178], buf[idx+179], buf[idx+180]);
	OutputLogStringA(fpLog, 
		"\t\tFile Flags							%d\n", buf[idx+181]);
	OutputLogStringA(fpLog, 
		"\t\tFile Unit Size						%d\n", buf[idx+182]);
	OutputLogStringA(fpLog, 
		"\t\tInterleave Gap Size					%d\n", buf[idx+183]);
	OutputLogStringA(fpLog, 
		"\t\tVolume Sequence Number				%d\n", 
		MAKEWORD(buf[idx+187], buf[idx+186]));
	OutputLogStringA(fpLog, 
		"\t\tLength of File Identifier			%d\n", buf[idx+188]);
	OutputLogStringA(fpLog, 
		"\t\tFile Identifier						%d\n", buf[idx+189]);
	LittleToBig(str, (_TCHAR*)&buf[idx+190], 64); str[64] = '\0';
	OutputLogStringW(fpLog, _T("\tVolume Set Identifier		[%s]\n"), str);
	LittleToBig(str, (_TCHAR*)&buf[idx+318], 64); str[64] = '\0';
	OutputLogStringW(fpLog, _T("\tPublisher Identifier		[%s]\n"), str);
	LittleToBig(str, (_TCHAR*)&buf[idx+446], 64); str[64] = '\0';
	OutputLogStringW(fpLog, _T("\tData Preparer Identifier	[%s]\n"), str);
	LittleToBig(str, (_TCHAR*)&buf[idx+574], 64); str[64] = '\0';
	OutputLogStringW(fpLog, _T("\tApplication Identifier		[%s]\n"), str);
	LittleToBig(str, (_TCHAR*)&buf[idx+702], 18); str[18] = '\0';
	OutputLogStringW(fpLog, _T("\tCopyright File Identifier		[%s]\n"), str);
	LittleToBig(str, (_TCHAR*)&buf[idx+702], 18); str[18] = '\0';
	OutputLogStringW(fpLog, _T("\tAbstract File Identifier		[%s]\n"), str);
	LittleToBig(str, (_TCHAR*)&buf[idx+776], 18); str[18] = '\0';
	OutputLogStringW(fpLog, _T("\tBibliographic File Identifier	[%s]\n"), str);

	CHAR year[4+1], month[2+1], day[2+1], hour[2+1], time[2+1], second[2+1], milisecond[2+1];
	strncpy(year, (CHAR*)&buf[idx+813], 4); year[4] = '\0';
	strncpy(month, (CHAR*)&buf[idx+817], 2); month[2] = '\0';
	strncpy(day, (CHAR*)&buf[idx+819], 2); day[2] = '\0';
	strncpy(hour, (CHAR*)&buf[idx+821], 2); hour[2] = '\0';
	strncpy(time, (CHAR*)&buf[idx+823], 2); time[2] = '\0';
	strncpy(second, (CHAR*)&buf[idx+825], 2); second[2] = '\0';
	strncpy(milisecond, (CHAR*)&buf[idx+827], 2); milisecond[2] = '\0';
	OutputLogStringA(fpLog, 
		"\tVolume Creation Date and Time		%s-%s-%s %s:%s:%s.%s +%d\n", 
		year, month, day, hour, time, second, milisecond, buf[idx+829]);
	strncpy(year, (CHAR*)&buf[idx+830], 4); year[4] = '\0';
	strncpy(month, (CHAR*)&buf[idx+834], 2); month[2] = '\0';
	strncpy(day, (CHAR*)&buf[idx+836], 2); day[2] = '\0';
	strncpy(hour, (CHAR*)&buf[idx+838], 2); hour[2] = '\0';
	strncpy(time, (CHAR*)&buf[idx+840], 2); time[2] = '\0';
	strncpy(second, (CHAR*)&buf[idx+842], 2); second[2] = '\0';
	strncpy(milisecond, (CHAR*)&buf[idx+844], 2); milisecond[2] = '\0';
	OutputLogStringA(fpLog, 
		"\tVolume Modification Date and Time	%s-%s-%s %s:%s:%s.%s +%d\n", 
		year, month, day, hour, time, second, milisecond, buf[idx+846]);
	strncpy(year, (CHAR*)&buf[idx+847], 4); year[4] = '\0';
	strncpy(month, (CHAR*)&buf[idx+851], 2); month[2] = '\0';
	strncpy(day, (CHAR*)&buf[idx+853], 2); day[2] = '\0';
	strncpy(hour, (CHAR*)&buf[idx+855], 2); hour[2] = '\0';
	strncpy(time, (CHAR*)&buf[idx+857], 2); time[2] = '\0';
	strncpy(second, (CHAR*)&buf[idx+859], 2); second[2] = '\0';
	strncpy(milisecond, (CHAR*)&buf[idx+861], 2); milisecond[2] = '\0';
	OutputLogStringA(fpLog, 
		"\tVolume Expiration Date and Time		%s-%s-%s %s:%s:%s.%s +%d\n", 
		year, month, day, hour, time, second, milisecond, buf[idx+863]);
	strncpy(year, (CHAR*)&buf[idx+864], 4); year[4] = '\0';
	strncpy(month, (CHAR*)&buf[idx+868], 2); month[2] = '\0';
	strncpy(day, (CHAR*)&buf[idx+870], 2); day[2] = '\0';
	strncpy(hour, (CHAR*)&buf[idx+872], 2); hour[2] = '\0';
	strncpy(time, (CHAR*)&buf[idx+874], 2); time[2] = '\0';
	strncpy(second, (CHAR*)&buf[idx+876], 2); second[2] = '\0';
	strncpy(milisecond, (CHAR*)&buf[idx+878], 2); milisecond[2] = '\0';
	OutputLogStringA(fpLog, 
		"\tVolume Effective Date and Time		%s-%s-%s %s:%s:%s.%s +%d\n", 
		year, month, day, hour, time, second, milisecond, buf[idx+880]);
	OutputLogStringA(fpLog, 
		"\tFile Structure Version				%d\n", buf[idx+881]);
	OutputLogStringA(fpLog, "\tApplication Use	");
	for(INT i = 883; i <= 1394; i++) {
		OutputLogStringA(fpLog, "%x", buf[idx+i]);
	}
	OutputLogStringA(fpLog, "\n");
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputVolumePartitionDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	CHAR str[CD_RAW_READ+1];
	ZeroMemory(str, sizeof(str));
	strncpy(str, (CHAR*)&buf[idx+8], 32); str[32] = '\0';
	OutputLogStringA(fpLog, "\tSystem Identifier				[%s]\n", str);
	strncpy(str, (CHAR*)&buf[idx+40], 32); str[32] = '\0';
	OutputLogStringA(fpLog, "\tVolume Partition Identifier	[%s]\n", str);
	OutputLogStringA(fpLog, "\tVolume Partition Location		%d\n", 
		MAKELONG(MAKEWORD(buf[idx+79], buf[idx+78]), 
		MAKEWORD(buf[idx+77], buf[idx+76])));
	OutputLogStringA(fpLog, "\tVolume Partition Size			%d\n", 
		MAKELONG(MAKEWORD(buf[idx+87], buf[idx+86]), 
		MAKEWORD(buf[idx+85], buf[idx+84])));
	OutputLogStringA(fpLog, "\tSystem Use	");
	for(INT i = 88; i <= 2047; i++) {
		OutputLogStringA(fpLog, "%x", buf[idx+i]);
	}
	OutputLogStringA(fpLog, "\n");
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}
// end for CD

// begin for DVD
void OutputVolumeStructureDescriptorFormat(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	OutputLogStringA(fpLog, "Volume Recognition Sequence\n");
	CHAR str[128+1];
	ZeroMemory(str, sizeof(str));
	strncpy(str, (CHAR*)&buf[idx+1], 5); str[5] = '\0';
	OutputLogStringA(fpLog, "\tStructure Type		%d\n", buf[idx]);
	OutputLogStringA(fpLog, "\tStandard Identifier	[%s]\n", str);
	OutputLogStringA(fpLog, "\tStructure Version	%d\n", buf[idx+6]);
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputVolumeRecognitionSequence(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	CHAR str[128+1];
	ZeroMemory(str, sizeof(str));
	strncpy(str, (CHAR*)&buf[idx+1], 5); str[5] = '\0';

	if(buf[idx] == 1 && !strcmp(str, "CD001")) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
		OutputPrimaryVolumeDescriptorForISO9660(idx, buf, fpLog);
	}
	else if(buf[idx] == 2 && !strcmp(str, "CD001")) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
		OutputLogStringA(fpLog, "\tVolume Flags		%d\n", buf[idx+7]);
		OutputPrimaryVolumeDescriptorForJoliet(idx, buf, fpLog);
	}
	else if(buf[idx] == 255 && !strcmp(str, "CD001")) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
	}
	else if(buf[idx] == 0 && !strcmp(str, "BOOT2")) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
		OutputBootDescriptor(idx, buf, fpLog);
	}
	else if(buf[idx] == 0 && !strcmp(str, "BEA01")) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
	}
	else if(buf[idx] == 0 && !strcmp(str, "NSR02")) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
	}
	else if(buf[idx] == 0 && !strcmp(str, "NSR03")) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
	}
	else if(buf[idx] == 0 && !strcmp(str, "TEA01")) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
	}
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputRecordingDateAndTime(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	OutputLogStringA(fpLog, "\tRecording Date and Time\n");
	OutputLogStringA(fpLog, "\t\t%x %d-%d-%d %d:%d:%d.%d.%d.%d\n",
		MAKEWORD(buf[idx], buf[idx+1]), MAKEWORD(buf[idx+2], buf[idx+3]),
		buf[idx+4], buf[idx+5], buf[idx+6], buf[idx+7], buf[idx+8],
		buf[idx+9], buf[idx+10], buf[idx+11]);
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputBootDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	CHAR str[CD_RAW_READ+1];
	ZeroMemory(str, sizeof(str));
	OutputLogStringA(fpLog, "\tArchitecture Type\n");
	OutputLogStringA(fpLog, "\t\tFlags	%d\n", buf[idx+8]);
	strncpy(str, (CHAR*)&buf[idx+9], 23); str[23] = '\0';
	OutputLogStringA(fpLog, "\t\tIdentifier	[%s]\n", str);
	strncpy(str, (CHAR*)&buf[idx+24], 8); str[8] = '\0';
	OutputLogStringA(fpLog, "\t\tIdentifier Suffix	[%s]\n", str);

	OutputLogStringA(fpLog, "\tBoot Identifier\n");
	OutputLogStringA(fpLog, "\t\tFlags	%d\n", buf[idx+40]);
	strncpy(str, (CHAR*)&buf[idx+41], 23); str[23] = '\0';
	OutputLogStringA(fpLog, "\t\tIdentifier	[%s]\n", str);
	strncpy(str, (CHAR*)&buf[idx+64], 8); str[8] = '\0';
	OutputLogStringA(fpLog, "\t\tIdentifier Suffix	[%s]\n", str);

	OutputLogStringA(fpLog, "\tBoot Extent Location	%d\n",
		MAKELONG(MAKEWORD(buf[idx+75], buf[idx+74]), 
		MAKEWORD(buf[idx+73], buf[idx+72])));
	OutputLogStringA(fpLog, "\tBoot Extent Length	%d\n",
		MAKELONG(MAKEWORD(buf[idx+79], buf[idx+78]), 
		MAKEWORD(buf[idx+77], buf[idx+76])));
	OutputLogStringA(fpLog, "\tLoad Address	%d%d\n",
		MAKELONG(MAKEWORD(buf[idx+87], buf[idx+86]), 
		MAKEWORD(buf[idx+85], buf[idx+84])),
		MAKELONG(MAKEWORD(buf[idx+83], buf[idx+82]), 
		MAKEWORD(buf[idx+81], buf[idx+80])));
	OutputLogStringA(fpLog, "\tStart Address	%d%d\n",
		MAKELONG(MAKEWORD(buf[idx+87], buf[idx+86]), 
		MAKEWORD(buf[idx+85], buf[idx+84])),
		MAKELONG(MAKEWORD(buf[idx+83], buf[idx+82]), 
		MAKEWORD(buf[idx+81], buf[idx+80])));
	OutputRecordingDateAndTime(idx + 96, buf, fpLog);
	OutputLogStringA(fpLog, "\tFlags %d\n", MAKEWORD(buf[idx+109], buf[idx+108]));
	OutputLogStringA(fpLog, "\tBoot Use	");
	for(INT i = 142; i <= 2047; i++) {
		OutputLogStringA(fpLog, "%x", buf[idx+i]);
	}
	OutputLogStringA(fpLog, "\n");
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputCharspec(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	OutputLogStringA(fpLog, "\t\tCharacter Set Type			%d\n", buf[idx]);
	CHAR str[23+1];
	ZeroMemory(str, sizeof(str));
	strncpy(str, (CHAR*)&buf[idx+1], 23); str[23] = '\0';
	OutputLogStringA(fpLog, "\t\tCharacter Set Information	[%s]\n", str);
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputExtentDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	OutputLogStringA(fpLog, "\t\tExtent Length	%d\n",
		MAKELONG(MAKEWORD(buf[idx], buf[idx+1]), 
		MAKEWORD(buf[idx+2], buf[idx+3])));
	OutputLogStringA(fpLog, "\t\tExtent Location	%d\n", 
		MAKELONG(MAKEWORD(buf[idx+4], buf[idx+5]), 
		MAKEWORD(buf[idx+6], buf[idx+7])));
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputRegid(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	OutputLogStringA(fpLog, "\t\tFlags				%d\n", buf[idx]);
	CHAR str[23+1];
	ZeroMemory(str, sizeof(str));
	strncpy(str, (CHAR*)&buf[idx+1], 23); str[23] = '\0';
	OutputLogStringA(fpLog, "\t\tIdentifier			[%s]\n", str);
	OutputLogStringA(fpLog, "\t\tIdentifier Suffix	");
	for(INT i = 24; i <= 31; i++) {
		OutputLogStringA(fpLog, "%x", buf[idx+i]);
	}
	OutputLogStringA(fpLog, "\n");
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputPrimaryVolumeDescriptorForUDF(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	CHAR str[128+1];
	ZeroMemory(str, sizeof(str));
	OutputLogStringA(fpLog, "\tVolume Descriptor Sequence Number	%d\n", 
		MAKELONG(MAKEWORD(buf[idx+16], buf[idx+17]), 
		MAKEWORD(buf[idx+18], buf[idx+19])));
	OutputLogStringA(fpLog, "\tPrimary Volume Descriptor Number	%d\n", 
		MAKELONG(MAKEWORD(buf[idx+20], buf[idx+21]), 
		MAKEWORD(buf[idx+22], buf[idx+23])));
	strncpy(str, (CHAR*)&buf[idx+24], 32); str[32] = '\0';
	OutputLogStringA(fpLog, "\tVolume Identifier				[%s]\n", str);
	OutputLogStringA(fpLog, "\tVolume Sequence Number			%d\n", 
		MAKEWORD(buf[idx+56], buf[idx+57]));
	OutputLogStringA(fpLog, "\tMaximum Volume Sequence Number	%d\n", 
		MAKEWORD(buf[idx+58], buf[idx+59]));
	OutputLogStringA(fpLog, "\tInterchange Level				%d\n", 
		MAKEWORD(buf[idx+60], buf[idx+61]));
	OutputLogStringA(fpLog, "\tMaximum Interchange Level		%d\n", 
		MAKEWORD(buf[idx+62], buf[idx+63]));
	OutputLogStringA(fpLog, "\tCharacter Set List				%d\n", 
		MAKELONG(MAKEWORD(buf[idx+65], buf[idx+64]), 
		MAKEWORD(buf[idx+67], buf[idx+66])));
	OutputLogStringA(fpLog, "\tMaximum Character Set List		%d\n", 
		MAKELONG(MAKEWORD(buf[idx+68], buf[idx+69]), 
		MAKEWORD(buf[idx+70], buf[idx+71])));
	strncpy(str, (CHAR*)&buf[idx+72], 128); str[128] = '\0';
	OutputLogStringA(fpLog, "\tVolume Set Identifier			[%s]\n", str);
	OutputLogStringA(fpLog, "\tDescriptor Character Set\n");
	OutputCharspec(idx + 200, buf, fpLog);

	OutputLogStringA(fpLog, "\tExplanatory Character Set\n");
	OutputCharspec(idx + 264, buf, fpLog);

	OutputLogStringA(fpLog, "\tVolume Abstract\n");
	OutputExtentDescriptor(idx + 328, buf, fpLog);

	OutputLogStringA(fpLog, "\tVolume Copyright Notice\n");
	OutputExtentDescriptor(idx + 336, buf, fpLog);

	OutputLogStringA(fpLog, "\tApplication Identifier\n");
	OutputRegid(idx + 344, buf, fpLog);

	OutputRecordingDateAndTime(idx + 376, buf, fpLog);

	OutputLogStringA(fpLog, "\tImplementation Identifier\n");
	OutputRegid(idx + 388, buf, fpLog);
	OutputLogStringA(fpLog, "\tImplementation Use		");
	for(INT i = 420; i <= 483; i++) {
		OutputLogStringA(fpLog, "%x", buf[idx+i]);
	}
	OutputLogStringA(fpLog, "\n");

	OutputLogStringA(fpLog, "\tPredecessor Volume Descriptor Sequence Location	%d\n", 
		MAKELONG(MAKEWORD(buf[idx+484], buf[idx+485]), 
		MAKEWORD(buf[idx+486], buf[idx+487])));
	OutputLogStringA(fpLog, "\tFlags											%d\n", 
		MAKEWORD(buf[idx+488], buf[idx+489]));
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputAnchorVolumeDescriptorPointer(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	OutputLogStringA(fpLog, "\tMain Volume Descriptor Sequence Extent\n");
	OutputExtentDescriptor(idx + 16, buf, fpLog);
	OutputLogStringA(fpLog, "\tReserve Volume Descriptor Sequence Extent\n");
	OutputExtentDescriptor(idx + 24, buf, fpLog);
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputVolumeDescriptorPointer(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	OutputLogStringA(fpLog, "\tVolume Descriptor Sequence Number	%d\n", 
		MAKELONG(MAKEWORD(buf[idx+16], buf[idx+17]), 
		MAKEWORD(buf[idx+18], buf[idx+19])));
	OutputLogStringA(fpLog, "\tNext Volume Descriptor Sequence Extent\n");
	OutputExtentDescriptor(idx + 20, buf, fpLog);
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputImplementationUseVolumeDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	CHAR str[128+1];
	ZeroMemory(str, sizeof(str));
	OutputLogStringA(fpLog, "\tVolume Descriptor Sequence Number	%d\n", 
		MAKELONG(MAKEWORD(buf[idx+16], buf[idx+17]), 
		MAKEWORD(buf[idx+18], buf[idx+19])));

	OutputLogStringA(fpLog, "\tImplementation Identifier\n");
	OutputRegid(idx + 20, buf, fpLog);

	OutputLogStringA(fpLog, "\tLVI Charset\n");
	OutputCharspec(idx + 52, buf, fpLog);

	strncpy(str, (CHAR*)&buf[idx+116], 128); str[128] = '\0';
	OutputLogStringA(fpLog, "\tLogical Volume Identifier		[%s]\n", str);

	strncpy(str, (CHAR*)&buf[idx+244], 36); str[36] = '\0';
	OutputLogStringA(fpLog, "\tLV Info 1			[%s]\n", str);
	strncpy(str, (CHAR*)&buf[idx+280], 36); str[36] = '\0';
	OutputLogStringA(fpLog, "\tLV Info 2			[%s]\n", str);
	strncpy(str, (CHAR*)&buf[idx+316], 36); str[36] = '\0';
	OutputLogStringA(fpLog, "\tLV Info 3			[%s]\n", str);
	OutputLogStringA(fpLog, "\tImplemention ID\n");
	OutputRegid(idx + 352, buf, fpLog);
	OutputLogStringA(fpLog, "\tImplementation Use		");
	for(INT i = 384; i <= 511; i++) {
		OutputLogStringA(fpLog, "%x", buf[idx+i]);
	}
	OutputLogStringA(fpLog, "\n");
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputPartitionDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	OutputLogStringA(fpLog, "\tVolume Descriptor Sequence Number	%d\n", 
		MAKELONG(MAKEWORD(buf[idx+16], buf[idx+17]), 
		MAKEWORD(buf[idx+18], buf[idx+19])));
	OutputLogStringA(fpLog, "\tPartition Flags						%d\n", 
		MAKEWORD(buf[idx+20], buf[idx+21]));
	OutputLogStringA(fpLog, "\tPartition Number					%d\n", 
		MAKEWORD(buf[idx+22], buf[idx+23]));
	OutputLogStringA(fpLog, "\tPartition Contents\n");
	OutputRegid(idx + 24, buf, fpLog);

	OutputLogStringA(fpLog, "\tPartition Contents Use	");
	for(INT i = 56; i <= 183; i++) {
		OutputLogStringA(fpLog, "%x", buf[idx+i]);
	}
	OutputLogStringA(fpLog, "\n");
	OutputLogStringA(fpLog, "\tAccess Type					%d\n", 
		MAKELONG(MAKEWORD(buf[idx+184], buf[idx+185]), 
		MAKEWORD(buf[idx+186], buf[idx+187])));
	OutputLogStringA(fpLog, "\tPartition Starting Location	%d\n", 
		MAKELONG(MAKEWORD(buf[idx+188], buf[idx+189]), 
		MAKEWORD(buf[idx+190], buf[idx+191])));
	OutputLogStringA(fpLog, "\tPartition Length			%d\n", 
		MAKELONG(MAKEWORD(buf[idx+192], buf[idx+193]), 
		MAKEWORD(buf[idx+194], buf[idx+195])));
	OutputLogStringA(fpLog, "\tImplementation Identifier\n");
	OutputRegid(idx + 196, buf, fpLog);
	OutputLogStringA(fpLog, "\tImplementation Use		");
	for(INT i = 228; i <= 355; i++) {
		OutputLogStringA(fpLog, "%x", buf[idx+i]);
	}
	OutputLogStringA(fpLog, "\n");
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputLongAllocationDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	OutputLogStringA(fpLog, "\tLongAllocationDescriptor\n");
	OutputLogStringA(fpLog, "\t\tExtent Length				%d\n", 
		MAKELONG(MAKEWORD(buf[idx], buf[idx+1]), 
		MAKEWORD(buf[idx+2], buf[idx+3])));
	OutputLogStringA(fpLog, "\t\tLogical Block Number		%d\n", 
		MAKELONG(MAKEWORD(buf[idx+4], buf[idx+5]), 
		MAKEWORD(buf[idx+6], buf[idx+7])));
	OutputLogStringA(fpLog, "\t\tPartition Reference Number	%d\n", 
		MAKEWORD(buf[idx+8], buf[idx+9]));
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputLogicalVolumeDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	CHAR str[128+1];
	ZeroMemory(str, sizeof(str));
	OutputLogStringA(fpLog, "\tVolume Descriptor Sequence Number	%d\n", 
		MAKELONG(MAKEWORD(buf[idx+16], buf[idx+17]), 
		MAKEWORD(buf[idx+18], buf[idx+19])));
	OutputLogStringA(fpLog, "\tDescriptor Character Set\n");
	OutputCharspec(idx + 20, buf, fpLog);

	strncpy(str, (CHAR*)&buf[idx+84], 128); str[128] = '\0';
	OutputLogStringA(fpLog, "\tLogical Volume Identifier		[%s]\n", str);
	OutputLogStringA(fpLog, "\tLogical Block Size				%d\n", 
		MAKELONG(MAKEWORD(buf[idx+212], buf[idx+213]), 
		MAKEWORD(buf[idx+214], buf[idx+215])));
	OutputLogStringA(fpLog, "\tDomain Identifier\n");
	OutputCharspec(idx + 216, buf, fpLog);
	OutputLongAllocationDescriptor(idx + 248, buf, fpLog);

	LONG MT_L = MAKELONG(MAKEWORD(buf[idx+264], buf[idx+265]), 
		MAKEWORD(buf[idx+266], buf[idx+267]));
	OutputLogStringA(fpLog, "\tMap Table Length				%d\n", MT_L);
	OutputLogStringA(fpLog, "\tNumber of Partition Maps		%d\n", 
		MAKELONG(MAKEWORD(buf[idx+268], buf[idx+269]), 
		MAKEWORD(buf[idx+270], buf[idx+271])));
	OutputLogStringA(fpLog, "\tImplementation Identifier\n");
	OutputRegid(idx + 272, buf, fpLog);

	OutputLogStringA(fpLog, "\tImplementation Use		");
	for(INT i = 304; i <= 431; i++) {
		OutputLogStringA(fpLog, "%x", buf[idx+i]);
	}
	OutputLogStringA(fpLog, "\n");
	OutputLogStringA(fpLog, "\tIntegrity Sequence Extent\n");
	OutputExtentDescriptor(idx + 432, buf, fpLog);

	OutputLogStringA(fpLog, "\tPartition Maps		");
	for(INT i = 0; i < MT_L; i++) {
		OutputLogStringA(fpLog, "%x", buf[idx+440+i]);
	}
	OutputLogStringA(fpLog, "\n");
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputUnallocatedSpaceDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	OutputLogStringA(fpLog, "\tVolume Descriptor Sequence Number	%d\n", 
		MAKELONG(MAKEWORD(buf[idx+16], buf[idx+17]), 
		MAKEWORD(buf[idx+18], buf[idx+19])));
	LONG N_AD = MAKELONG(MAKEWORD(buf[idx+20], buf[idx+21]), 
		MAKEWORD(buf[idx+22], buf[idx+23]));
	OutputLogStringA(fpLog, "\tNumber of Allocation Descriptors	%d\n", N_AD);
	OutputLogStringA(fpLog, "\tAllocation Descriptors\n");
	for(INT i = 0; i < N_AD * 8; i+=8) {
		OutputExtentDescriptor(idx + 24 + i, buf, fpLog);
	}
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

bool OutputVolumeDescriptorSequence(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	USHORT usTagId = MAKEWORD(buf[idx], buf[idx+1]);
	if(usTagId == 0 || (10 <= usTagId && usTagId <= 255) || 267 <= usTagId) {
		return false;
	}
	switch(usTagId) {
	case 1:
		OutputLogStringA(fpLog, "Primary Volume Descriptor\n");
		break;
	case 2:
		OutputLogStringA(fpLog, "Anchor Volume Descriptor Pointer\n");
		break;
	case 3:
		OutputLogStringA(fpLog, "Volume Descriptor Pointer\n");
		break;
	case 4:
		OutputLogStringA(fpLog, "Implementation Use Volume Descriptor\n");
		break;
	case 5:
		OutputLogStringA(fpLog, "Partition Descriptor\n");
		break;
	case 6:
		OutputLogStringA(fpLog, "Logical Volume Descriptor\n");
		break;
	case 7:
		OutputLogStringA(fpLog, "Unallocated Space Descriptor\n");
		break;
	case 8:
		OutputLogStringA(fpLog, "Terminating Descriptor\n");
		break;
	case 9:
		OutputLogStringA(fpLog, "Logical Volume Integrity Descriptor\n");
		break;
	case 256:
		OutputLogStringA(fpLog, "File Set Descriptor\n");
		break;
	case 257:
		OutputLogStringA(fpLog, "File Identifier Descriptor\n");
		break;
	case 258:
		OutputLogStringA(fpLog, "Allocation Extent Descriptor\n");
		break;
	case 259:
		OutputLogStringA(fpLog, "Indirect Entry\n");
		break;
	case 260:
		OutputLogStringA(fpLog, "Terminal Entry\n");
		break;
	case 261:
		OutputLogStringA(fpLog, "File Entry\n");
		break;
	case 262:
		OutputLogStringA(fpLog, "Extended Attribute Header Descriptor\n");
		break;
	case 263:
		OutputLogStringA(fpLog, "Unallocated Space Entry\n");
		break;
	case 264:
		OutputLogStringA(fpLog, "Space Bitmap Descriptor\n");
		break;
	case 265:
		OutputLogStringA(fpLog, "Partition Integrity Entry\n");
		break;
	case 266:
		OutputLogStringA(fpLog, "Extended File Entry\n");
		break;
	}

	OutputLogStringA(fpLog, "\t\tDescriptor Version		%d\n", 
		MAKEWORD(buf[idx+2], buf[idx+3]));
	OutputLogStringA(fpLog, "\t\tTag Checksum			%d\n", buf[idx+4]);
	OutputLogStringA(fpLog, "\t\tTag Serial Number		%d\n", 
		MAKEWORD(buf[idx+6], buf[idx+7]));
	OutputLogStringA(fpLog, "\t\tDescriptor CRC			%x\n", 
		MAKEWORD(buf[idx+8], buf[idx+9]));
	OutputLogStringA(fpLog, "\t\tDescriptor CRC Length	%d\n", 
		MAKEWORD(buf[idx+10], buf[idx+11]));
	OutputLogStringA(fpLog, "\t\tTag Location			%d\n",
		MAKELONG(MAKEWORD(buf[idx+12], buf[idx+13]), 
		MAKEWORD(buf[idx+14], buf[idx+15])));
	switch(usTagId) {
	case 1:
		OutputPrimaryVolumeDescriptorForUDF(idx, buf, fpLog);
		break;
	case 2:
		OutputAnchorVolumeDescriptorPointer(idx, buf, fpLog);
		break;
	case 3:
		OutputVolumeDescriptorPointer(idx, buf, fpLog);
		break;
	case 4:
		OutputImplementationUseVolumeDescriptor(idx, buf, fpLog);
		break;
	case 5:
		OutputPartitionDescriptor(idx, buf, fpLog);
		break;
	case 6:
		OutputLogicalVolumeDescriptor(idx, buf, fpLog);
		break;
	case 7:
		OutputUnallocatedSpaceDescriptor(idx, buf, fpLog);
		break;
	}
	return true;
}
// end for DVD

void OutputCopyrightManagementInformation(
	INT nLBA,
	INT i,
	PUCHAR pBuf2,
	FILE* fpLog
	)
{
	OutputLogStringA(fpLog, "\t\tLBA %7u, ", nLBA + i);
	if((pBuf2[4] & 0x80) == 0) {
		OutputLogStringA(fpLog, "CPM don't exists");
		if((pBuf2[4] & 0x30) == 0) {
			OutputLogStringA(fpLog, ", copying is permitted without restriction\n");
		}
		else {
			OutputLogStringA(fpLog, "\n");
		}
	}
	else if((pBuf2[4] & 0x80) == 0x80) {
		OutputLogStringA(fpLog, "CPM exists");
		if((pBuf2[4] & 0x40) == 0) {
			OutputLogStringA(fpLog, ", CSS or CPPM don't exists in this sector");
		}
		else if((pBuf2[4] & 0x40) == 0x40) {
			if((pBuf2[4] & 0x0F) == 0) {
				OutputLogStringA(fpLog, "the sector is scrambled by CSS");
			}
			else if((pBuf2[4] & 0x0F) == 1) {
				OutputLogStringA(fpLog, "the sector is encrypted by CPPM");
			}
		}
		if((pBuf2[4] & 0x30) == 0) {
			OutputLogStringA(fpLog, ", copying is permitted without restriction\n");
		}
		else if((pBuf2[4] & 0x30) == 0x02) {
			OutputLogStringA(fpLog, ", one generation of copies may be made\n");
		}
		else if((pBuf2[4] & 0x30) == 0x03) {
			OutputLogStringA(fpLog, ", no copying is permitted\n");
		}
		else {
			OutputLogStringA(fpLog, "\n");
		}
	}
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void SetAlbumTitle(
	LPCSTR pszString
	)
{
	strncpy(szAlbumTitle, pszString, strlen(pszString));
	szAlbumTitle[META_STRING_SIZE] = '\0';
}

void SetISRCToString(
	CONST PUCHAR Subcode,
	INT nTrackNum,
	LPSTR pszOutString,
	BOOL bCopy
	)
{
	/*
	BYTE＼bit |	7	 6	  5	  4	  3	  2	  1	  0
	0	      |      Ctrl	    |		ADR
	1		  |		I01				    |(MSB) I02
	2		  |		I02　(LSB)  |(MSB)　　I03
	3		  |I03 (LSB)|			I04
	4		  |I05						|ZERO
	5		  |I06			    |I07
	6		  |I08			    |I09
	7		  |I10			    |I11
	8		  |I12			    |ZERO
	9	フレーム (=1/75秒) (CD全体の経過時間)(BCD)
	10	
	(MSB)
	CRC　P(x)=x16+x12+x5+x1
	(LSB)
	11

	I01 ～ I02 : 国名コード (6ビット)
	I03 ～ I05 : 登録者コード (6ビット)
	I06 ～ I07 : 記録年 (4ビット)
	I08 ～ I12 : シリアルナンバー (4ビット)

	これをASCIIコードに変換するには、それぞれに 0x30 を足してやります。
	*/
	sprintf(pszOutString, "%c%c%c%c%c%c%c%c%c%c%c%c",
		((Subcode[13] >> 2) & 0x3F) + 0x30, 
		(((Subcode[13] << 4) & 0x30) | ((Subcode[14] >> 4) & 0x0F)) + 0x30, 
		(((Subcode[14] << 2) & 0x3C) | ((Subcode[15] >> 6) & 0x03)) + 0x30, 
		(Subcode[15] & 0x3F) + 0x30, 
		((Subcode[16] >> 2) & 0x3F) + 0x30, 
		((Subcode[17] >> 4) & 0x0F) + 0x30, (Subcode[17] & 0x0F) + 0x30, 
		((Subcode[18] >> 4) & 0x0F) + 0x30, (Subcode[18] & 0x0F) + 0x30, 
		((Subcode[19] >> 4) & 0x0F) + 0x30, (Subcode[19] & 0x0F) + 0x30,
		((Subcode[20] >> 4) & 0x0F) + 0x30);
	if(bCopy) {
		strncpy(szISRC[nTrackNum-1], pszOutString, strlen(pszOutString));
		szISRC[nTrackNum-1][12] = '\0';
	}
}

void SetMCNToString(
	CONST PUCHAR Subcode,
	LPSTR pszOutString,
	BOOL bCopy
	)
{
	sprintf(pszOutString, "%c%c%c%c%c%c%c%c%c%c%c%c%c", 
		((Subcode[13] >> 4) & 0x0F) + 0x30, (Subcode[13] & 0x0F) + 0x30, 
		((Subcode[14] >> 4) & 0x0F) + 0x30, (Subcode[14] & 0x0F) + 0x30, 
		((Subcode[15] >> 4) & 0x0F) + 0x30, (Subcode[15] & 0x0F) + 0x30, 
		((Subcode[16] >> 4) & 0x0F) + 0x30, (Subcode[16] & 0x0F) + 0x30, 
		((Subcode[17] >> 4) & 0x0F) + 0x30, (Subcode[17] & 0x0F) + 0x30, 
		((Subcode[18] >> 4) & 0x0F) + 0x30, (Subcode[18] & 0x0F) + 0x30, 
		((Subcode[19] >> 4) & 0x0F) + 0x30);
	if(bCopy) {
		strncpy(szCatalog, pszOutString, strlen(pszOutString));
		szCatalog[13] = '\0';
	}
}

void SetPerformer(
	LPCSTR pszString
	)
{
	strncpy(szPerformer, pszString, strlen(pszString));
	szPerformer[META_STRING_SIZE] = '\0';
}

void SetSongWriter(
	LPCSTR pszString
	)
{
	strncpy(szSongWriter, pszString, strlen(pszString));
	szSongWriter[META_STRING_SIZE] = '\0';
}

void SetTitle(
	LPCSTR pszString,
	INT idx
	)
{
	strncpy(szTitle[idx], pszString, strlen(pszString));
	szTitle[idx][META_STRING_SIZE] = '\0';
}

void WriteCcdFileForDisc(
	size_t tocEntries,
	UCHAR LastCompleteSession,
	FILE* fpCcd
	)
{
	fprintf(fpCcd, "[CloneCD]\n");
	fprintf(fpCcd, "Version=3\n");
	fprintf(fpCcd, "[Disc]\n");
	fprintf(fpCcd, "TocEntries=%d\n", tocEntries);
	fprintf(fpCcd, "Sessions=%d\n", LastCompleteSession);
	fprintf(fpCcd, "DataTracksScrambled=%d\n", 0); // TODO
}

void WriteCcdFileForDiscCDTextLength(
	size_t cdTextSize,
	FILE* fpCcd
	)
{
	fprintf(fpCcd, "CDTextLength=%d\n", cdTextSize);
}

void WriteCcdFileForDiscCatalog(
	FILE* fpCcd
	)
{
	fprintf(fpCcd, "CATALOG=%s\n", szCatalog);
}

void WriteCcdFileForCDText(
	size_t cdTextSize,
	FILE* fpCcd
	)
{
	fprintf(fpCcd, "[CDText]\n");
	fprintf(fpCcd, "Entries=%d\n", cdTextSize);
}

void WriteCcdFileForCDTextEntry(
	size_t t,
	CONST CDROM_TOC_CD_TEXT_DATA_BLOCK* pDesc,
	FILE* fpCcd
	)
{
	fprintf(fpCcd, 
		"Entry %d=%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", 
		t, pDesc[t].PackType, pDesc[t].TrackNumber | pDesc[t].ExtensionFlag << 8, pDesc[t].SequenceNumber, 
		pDesc[t].CharacterPosition | pDesc[t].BlockNumber << 7 | pDesc[t].Unicode << 8,
		pDesc[t].Text[0], pDesc[t].Text[1], pDesc[t].Text[2], pDesc[t].Text[3], 
		pDesc[t].Text[4], pDesc[t].Text[5],	pDesc[t].Text[6], pDesc[t].Text[7], 
		pDesc[t].Text[8], pDesc[t].Text[9], pDesc[t].Text[10], pDesc[t].Text[11]);
}

void WriteCcdFileForSession(
	UCHAR SessionNumber,
	FILE* fpCcd
	)
{
	fprintf(fpCcd, "[Session %d]\n", SessionNumber);
}

void WriteCcdFileForSessionPregap(
	UCHAR mode,
	FILE* fpCcd
	)
{
	fprintf(fpCcd, "PreGapMode=%d\n", mode);
	fprintf(fpCcd, "PreGapSubC=%d\n", 0);	// TODO
}

void WriteCcdFileForEntry(
	size_t a,
	CONST CDROM_TOC_FULL_TOC_DATA_BLOCK* toc,
	FILE* fpCcd
	)
{
	fprintf(fpCcd, "[Entry %d]\n", a);
	fprintf(fpCcd, "Session=%d\n", toc[a].SessionNumber);
	fprintf(fpCcd, "Point=0x%02x\n", toc[a].Point);
	fprintf(fpCcd, "ADR=0x%02x\n", toc[a].Adr);
	fprintf(fpCcd, "Control=0x%02x\n", toc[a].Control);
	fprintf(fpCcd, "TrackNo=%d\n", toc[a].Reserved1);
	fprintf(fpCcd, "AMin=%d\n", toc[a].MsfExtra[0]);
	fprintf(fpCcd, "ASec=%d\n", toc[a].MsfExtra[1]);
	fprintf(fpCcd, "AFrame=%d\n", toc[a].MsfExtra[2]);
	fprintf(fpCcd, "ALBA=%d\n", MSFtoLBA(toc[a].MsfExtra[2], 
		toc[a].MsfExtra[1], toc[a].MsfExtra[0]) - 150);
	fprintf(fpCcd, "Zero=%d\n", toc[a].Zero);
	fprintf(fpCcd, "PMin=%d\n", toc[a].Msf[0]);
	fprintf(fpCcd, "PSec=%d\n", toc[a].Msf[1]);
	fprintf(fpCcd, "PFrame=%d\n", toc[a].Msf[2]);
	fprintf(fpCcd, "PLBA=%d\n", 
		MSFtoLBA(toc[a].Msf[2], toc[a].Msf[1], toc[a].Msf[0]) - 150);
}

void WriteCcdFileForTrack(
	INT nTrackNum,
	UCHAR byModeNum,
	BOOL bISRC,
	FILE* fpCcd
	)
{
	fprintf(fpCcd, "[TRACK %d]\n", nTrackNum);
	fprintf(fpCcd, "MODE=%d\n", byModeNum);
	if(bISRC) {
		fprintf(fpCcd, "ISRC=%s\n", szISRC[nTrackNum-1]);
	}
}

void WriteCcdFileForTrackIndex(
	LONG index,
	LONG lba,
	FILE* fpCcd
	)
{
	fprintf(fpCcd, "INDEX %d=%d\n", index, lba);
}

void WriteCueFile(
	BOOL bCatalog,
	LPCSTR pszFilename,
	INT nTrackNum,
	UCHAR byModeNum,
	BOOL bISRC,
	UCHAR byCtl,
	FILE* fpCue
	)
{
	if(bCatalog) {
		fprintf(fpCue, "CATALOG %s\n", szCatalog);
	}
	if(strlen(szAlbumTitle) > 0) {
		fprintf(fpCue, "TITLE \"%s\"\n", szAlbumTitle);
	}
	if(strlen(szPerformer) > 0) {
		fprintf(fpCue, "PERFORMER \"%s\"\n", szPerformer);
	}
	if(strlen(szSongWriter) > 0) {
		fprintf(fpCue, "SONGWRITER \"%s\"\n", szSongWriter);
	}
	fprintf(fpCue, "FILE \"%s\" BINARY\n", pszFilename);

	if(byModeNum == DATA_BLOCK_MODE0) {
		fprintf(fpCue, "  TRACK %02d AUDIO\n", nTrackNum);
		if(bISRC) {
			fprintf(fpCue, "    ISRC %s\n", szISRC[nTrackNum-1]);
		}
		if(strlen(szTitle[nTrackNum-1]) > 0) {
			fprintf(fpCue, "    TITLE \"%s\"\n", szTitle[nTrackNum-1]);
		}
		if((byCtl & AUDIO_WITH_PREEMPHASIS) == AUDIO_WITH_PREEMPHASIS ||
			(byCtl & DIGITAL_COPY_PERMITTED) == DIGITAL_COPY_PERMITTED ||
			(byCtl & TWO_FOUR_CHANNEL_AUDIO) == TWO_FOUR_CHANNEL_AUDIO) {
			CHAR aBuf[22];
			ZeroMemory(aBuf, sizeof(aBuf));
			strcat(aBuf, "    FLAGS");
			switch(byCtl) {
			case AUDIO_WITH_PREEMPHASIS:
				strcat(aBuf, " PRE\n");
				break;
			case DIGITAL_COPY_PERMITTED:
				strcat(aBuf, " DCP\n");
				break;
			case DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
				strcat(aBuf, " DCP PRE\n");
				break;
			case TWO_FOUR_CHANNEL_AUDIO:
				strcat(aBuf, " 4CH\n");
				break;
			case TWO_FOUR_CHANNEL_AUDIO | AUDIO_WITH_PREEMPHASIS:
				strcat(aBuf, " 4CH PRE\n");
				break;
			case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED:
				strcat(aBuf, " 4CH DCP\n");
				break;
			case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
				strcat(aBuf, " 4CH DCP PRE\n");
				break;
			}
			fwrite(aBuf, sizeof(CHAR), strlen(aBuf), fpCue);
		}
	}
	else {
		fprintf(fpCue, "  TRACK %02d MODE%1d/2352\n", nTrackNum, byModeNum);
		if((byCtl & DIGITAL_COPY_PERMITTED) == DIGITAL_COPY_PERMITTED) {
			fprintf(fpCue, "    FLAGS DCP\n");
		}
	}
}

void WriteCueFileForIndex(
	UCHAR byIndex,
	UCHAR byFrame, 
	UCHAR bySecond,
	UCHAR byMinute,
	FILE* fpCue
	)
{
	fprintf(fpCue, "    INDEX %02d %02d:%02d:%02d\n", 
		byIndex, byMinute, bySecond, byFrame);
}
