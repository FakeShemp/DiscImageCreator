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

FILE* CreateOrOpenFile(
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
	OutputLogString(fpLog, "c2 error LBA %d\n", nLBA);

	for(INT i = 0; i < CD_RAW_READ_C2_SIZE; i+=8) {
		OutputLogString(fpLog, 
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
	OutputLogString(fpLog, "Main Channel LBA %d\n", nLBA);
	OutputLogString(fpLog, "\t    +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n");

	for(INT i = 0; i < CD_RAW_SECTOR_SIZE; i += 16) {
		OutputLogString(fpLog, 
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
	OutputLogString(fpLog, "Device Info\n");
	OutputLogString(fpLog, "\tDeviceType:");
	switch(pInquiry[0] & 0x1F) {
	case 5:
		OutputLogString(fpLog, "CD/DVD device\n");
		break;
	default:
		OutputLogString(fpLog, "Other device\n");
		break;
	}
	OutputLogString(fpLog, "\tDeviceTypeQualifier:%d\n", pInquiry[0] >> 5 & 0x3);

	OutputLogString(fpLog, "\tDeviceTypeModifier:%d\n", pInquiry[1] & 0x7F);
	OutputLogString(fpLog, "\tRemovableMedia:%s\n",
		(pInquiry[1] >> 7 & 0x1) == 0 ? "No" : "Yes");

	OutputLogString(fpLog, "\tVersions:%d\n", pInquiry[2]);

	OutputLogString(fpLog, "\tResponseDataFormat:%d\n", pInquiry[3] & 0x0F);
	OutputLogString(fpLog, "\tHiSupport:%s\n",
		(pInquiry[3] >> 4 & 0x1) == 0 ? "No" : "Yes");
	OutputLogString(fpLog, "\tNormACA:%s\n",
		(pInquiry[3] >> 5 & 0x1) == 0 ? "No" : "Yes");
	OutputLogString(fpLog, "\tTerminateTask:%s\n",
		(pInquiry[3] >> 6 & 0x1) == 0 ? "No" : "Yes");
	OutputLogString(fpLog, "\tAERC:%s\n",
		(pInquiry[3] >> 7 & 0x1) == 0 ? "No" : "Yes");

	OutputLogString(fpLog, "\tAdditionalLength:%d\n", pInquiry[4]);

	OutputLogString(fpLog, "\tMediumChanger:%s\n",
		(pInquiry[6] >> 3 & 0x1) == 0 ? "No" : "Yes");
	OutputLogString(fpLog, "\tMultiPort:%s\n",
		(pInquiry[6] >> 4 & 0x1) == 0 ? "No" : "Yes");
	OutputLogString(fpLog, "\tEnclosureServices:%s\n",
		(pInquiry[6] >> 6 & 0x1) == 0 ? "No" : "Yes");

	OutputLogString(fpLog, "\tSoftReset:%s\n",
		(pInquiry[7] & 0x1) == 0 ? "No" : "Yes");
	OutputLogString(fpLog, "\tCommandQueue:%s\n",
		(pInquiry[7] >> 1 & 0x1) == 0 ? "No" : "Yes");
	OutputLogString(fpLog, "\tLinkedCommands:%s\n",
		(pInquiry[7] >> 3 & 0x1) == 0 ? "No" : "Yes");
	OutputLogString(fpLog, "\tRelativeAddressing:%s\n",
		(pInquiry[7] >> 7 & 0x1) == 0 ? "No" : "Yes");

	OutputLogString(fpLog, "\tVendorId:");
	INT i = 8;
	while(i < 16) {
		OutputLogString(fpLog, "%c", pInquiry[i]);
		pszVendorId[i-8] = pInquiry[i++];
	}
	OutputLogString(fpLog, "\n");

	OutputLogString(fpLog, "\tProductId:"); 
	while(i < 32) {
		OutputLogString(fpLog, "%c", pInquiry[i]);
		pszProductId[i-16] = pInquiry[i++];
	}
	OutputLogString(fpLog, "\n");

	OutputLogString(fpLog, "\tProductRevisionLevel:"); 
	while(i < 36) {
		OutputLogString(fpLog, "%c", pInquiry[i++]);
	}
	OutputLogString(fpLog, "\n");

	OutputLogString(fpLog, "\tVendorSpecific:"); 
	while(i < 56) {
		OutputLogString(fpLog, "%c", pInquiry[i++]);
	}
	OutputLogString(fpLog, "\n");
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
		OutputLogString(fpLog, "\tPhysicalFormatInformation\n");
		OutputLogString(fpLog, "\t\tBookVersion:%d\n", 
			pStructure[4] & 0x0F);
		LPCSTR lpBookType[] = {"DVD-ROM", "DVD-RAM", "DVD-R", "DVD-RW", 
			"HD DVD-ROM", "HD DVD-RAM", "HD DVD-R", "DVD+RW", "DVD+R"};
		OutputLogString(fpLog, "\t\tBookType:%s\n", 
			lpBookType[pStructure[4] & 0xF0]);

		LPCSTR lpMaximumRate[] = {"2.52Mbps", "5.04Mbps", 
			"10.08Mbps", "20.16Mbps", "30.24Mbps"};
		OutputLogString(fpLog, "\t\tMinimumRate:%s\n", 
			lpMaximumRate[pStructure[5] & 0x0F]);
		OutputLogString(fpLog, "\t\tDiskSize:%s\n", 
			(pStructure[5] & 0xF0) == 0 ? "120mm" : "80mm");

		LPCSTR lpLayerType[] = {"Layer contains embossed data", 
			"Layer contains recordable data", 
			"Layer contains rewritable data"};
		OutputLogString(fpLog, "\t\tLayerType:%s\n", 
			lpLayerType[pStructure[6] & 0x0F]);
		OutputLogString(fpLog, "\t\tTrackPath:%s\n", 
			(pStructure[6] & 0x10) == 0 
			? "Parallel Track Path" : "Opposite Track Path");
		OutputLogString(fpLog, "\t\tNumberOfLayers:%s\n", 
			(pStructure[6] & 0x60) == 0 
			? "Single Layer" : "Double Layer");

		LPCSTR lpTrackDensity[] = {"0.74μm/track", "0.80μm/track", 
			"0.615μm/track", "0.40μm/track", "0.34μm/track"};
		OutputLogString(fpLog, "\t\tTrackDensity:%s\n", 
			lpTrackDensity[pStructure[7] & 0x0F]);
		LPCSTR lpLinearDensity[] = {"0.267μm/bit", "0.293μm/bit", 
			"0.409 to 0.435μm/bit", "", "0.280 to 0.291μm/bit", 
			"0.153μm/bit", "0130 to 0.140μm/bit", "", 
			"0.353μm/bit"};
		OutputLogString(fpLog, "\t\tLinearDensity:%s\n", 
			lpLinearDensity[pStructure[7] & 0xF0]);

		LONG ulStartSectorNum = MAKELONG(MAKEWORD(pStructure[11], 
			pStructure[10]), MAKEWORD(pStructure[9], pStructure[8]));
		OutputLogString(fpLog, "\t\tStartDataSector:%d(0x%X)\n", 
			ulStartSectorNum, ulStartSectorNum);
		LONG ulEndSectorNum = MAKELONG(MAKEWORD(pStructure[15], 
			pStructure[14]), MAKEWORD(pStructure[13], pStructure[12]));
		OutputLogString(fpLog, "\t\tEndDataSector:%d(0x%X)\n", 
			ulEndSectorNum, ulEndSectorNum);
		*nDVDSectorSize = ulEndSectorNum - ulStartSectorNum + 1;
		INT ulEndSectorLayer0 = MAKELONG(MAKEWORD(pStructure[19], 
			pStructure[18]), MAKEWORD(pStructure[17], pStructure[16]));
		OutputLogString(fpLog, "\t\tEndLayerZeroSector:%X\n", 
			ulEndSectorLayer0);

		OutputLogString(fpLog, "\t\tBCAFlag:%s\n", 
			(pStructure[20] & 0x80) == 0 ? "None" : "Exist");

		OutputLogString(fpLog, "\t\tMediaSpecific:");
		for(ULONG k = 0; k < 2031; k++) {
			OutputLogString(fpLog, "%02X", pStructure[21+k]);
		}
		OutputLogString(fpLog, "\n");
		break;
	}
	case DvdCopyrightDescriptor:
		OutputLogString(fpLog, "\tCopyrightProtectionType:");
		switch(pStructure[4]) {
		case 0:
			OutputLogString(fpLog, "No\n");
			break;
		case 1:
			OutputLogString(fpLog, "CSS/CPPM\n");
			break;
		case 2:
			OutputLogString(fpLog, "CPRM\n");
			break;
		case 3:
			OutputLogString(fpLog, "AACS with HD DVD content\n");
			break;
		case 10:
			OutputLogString(fpLog, "AACS with BD content\n");
			break;
		default:
			OutputLogString(fpLog, "Unknown:[%02X]\n", pStructure[4]);
			break;
		}
		OutputLogString(fpLog, 
			"\tRegionManagementInformation:%02X\n", pStructure[5]);
		break;
	case DvdDiskKeyDescriptor:
		OutputLogString(fpLog, "\tDiskKeyData:");
		for(ULONG k = 0; k < 2048; k++) {
			OutputLogString(fpLog, "%02X", pStructure[4+k]);
		}
		OutputLogString(fpLog, "\n");
		break;
	case DvdBCADescriptor:
		OutputLogString(fpLog, "\tBCAInformation:");
		for(ULONG k = 0; 
			k < pStructureLength[i] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
			OutputLogString(fpLog, "%02X", pStructure[4+k]);
		}
		OutputLogString(fpLog, "\n");
		break;
	case DvdManufacturerDescriptor:
		OutputLogString(fpLog, "\tManufacturingInformation:");
		for(ULONG k = 0; k < 2048; k++) {
			OutputLogString(fpLog, "%02X", pStructure[4+k]);
		}
		OutputLogString(fpLog, "\n");
		break;
	case 6:
		OutputLogString(fpLog, "\tmedia ID:");
		for(ULONG k = 0; 
			k < pStructureLength[i] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
			OutputLogString(fpLog, "%02X", pStructure[4+k]);
		}
		OutputLogString(fpLog, "\n");
		break;
	case 7:
		OutputLogString(fpLog, "\tMedia Key Block Total Packs %d", pStructure[3]);
		OutputLogString(fpLog, "\tmedia key block:");
		for(ULONG k = 0; 
			k < pStructureLength[i] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
			OutputLogString(fpLog, "%02X", pStructure[4+k]);
		}
		OutputLogString(fpLog, "\n");
		break;
	default:
		OutputLogString(fpLog, "Unknown:[%02X]\n", pFormat[i]);
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
			OutputLogString(fpLog, "\tFeatureProfileList\n");
			OutputLogString(fpLog, "\t\t");
			while(n < pConf[uiSize+3]) {
				OutputFeatureProfileType(fpLog, 
					MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n]));
				OutputLogString(fpLog, ", ");
				n += sizeof(FEATURE_DATA_PROFILE_LIST_EX);
			}
			OutputLogString(fpLog, "\n");
			n += sizeof(FEATURE_HEADER);
			break;
		case FeatureCore: {
			OutputLogString(fpLog, "\tFeatureCore\n");
			OutputLogString(fpLog, "\t\tPhysicalInterface:");
			lVal = MAKELONG(MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]), 
				MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n]));
			switch(lVal) {
			case 0:
				OutputLogString(fpLog, "Unspecified\n");
				break;
			case 1:
				OutputLogString(fpLog, "SCSI Family\n");
				break;
			case 2:
				OutputLogString(fpLog, "ATAPI\n");
				break;
			case 3:
				OutputLogString(fpLog, "IEEE 1394 - 1995\n");
				break;
			case 4:
				OutputLogString(fpLog, "IEEE 1394A\n");
				break;
			case 5:
				OutputLogString(fpLog, "Fibre Channel\n");
				break;
			case 6:
				OutputLogString(fpLog, "IEEE 1394B\n");
				break;
			case 7:
				OutputLogString(fpLog, "Serial ATAPI\n");
				break;
			case 8:
				OutputLogString(fpLog, "USB (both 1.1 and 2.0)\n");
				break;
			case 0xFFFF:
				OutputLogString(fpLog, "Vendor Unique\n");
				break;
			default:
				OutputLogString(fpLog, "Reserved:[%08d]\n", lVal);
				break;
			}
			OutputLogString(fpLog, "\t\tDeviceBusyEvent:%s\n", 
				(pConf[uiSize+8+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tINQUIRY2:%s\n", 
				(pConf[uiSize+8+n] & 0x02) == 0x02 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		}
		case FeatureMorphing:
			OutputLogString(fpLog, "\tFeatureMorphing\n");
			OutputLogString(fpLog, "\t\tAsynchronous:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tOCEvent:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureRemovableMedium:
			OutputLogString(fpLog, "\tFeatureRemovableMedium\n");
			OutputLogString(fpLog, "\t\tLockable:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tDBML:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tDefaultToPrevent:%s\n", 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tEject:%s\n", 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tLoad:%s\n", 
				(pConf[uiSize+4+n] & 0x10) == 0x10 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tLoadingMechanism:"); 
			switch(pConf[uiSize+4+n] >> 5 & 0x07) {
			case 0:
				OutputLogString(fpLog, "Caddy/Slot type loading mechanism\n");
				break;
			case 1:
				OutputLogString(fpLog, "Tray type loading mechanism\n");
				break;
			case 2:
				OutputLogString(fpLog, "Pop-up type loading mechanism\n");
				break;
			case 4:
				OutputLogString(fpLog, 
					"Embedded changer with individually changeable discs\n");
				break;
			case 5:
				OutputLogString(fpLog, 
					"Embedded changer using a magazine mechanism\n");
				break;
			default:
				OutputLogString(fpLog, 
					"Reserved:[%08d]\n", pConf[uiSize+4+n] >> 5 & 0x07);
				break;
			}
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureWriteProtect:
			OutputLogString(fpLog, "\tFeatureWriteProtect\n");
			OutputLogString(fpLog, "\t\tSupportsSWPPBit:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tSupportsPersistentWriteProtect:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tWriteInhibitDCB:%s\n", 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tDiscWriteProtectPAC:%s\n", 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureRandomReadable:
			OutputLogString(fpLog, "\tFeatureRandomReadable\n");
			lVal = MAKELONG(MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]), 
				MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n]));
			OutputLogString(fpLog, "\t\tLogicalBlockSize:%d\n", lVal);
			wVal = MAKEWORD(pConf[uiSize+9+n], pConf[uiSize+8+n]);
			OutputLogString(fpLog, "\t\tBlocking:%d\n", wVal);
			OutputLogString(fpLog, "\t\tErrorRecoveryPagePresent:%s\n", 
				(pConf[uiSize+10+n] & 0x01) == 0x01 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureMultiRead:
			OutputLogString(fpLog, "\tFeatureMultiRead\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureCdRead:
			OutputLogString(fpLog, "\tFeatureCdRead\n");
			OutputLogString(fpLog, "\t\tCDText:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			*bCanCDText = pConf[uiSize+4+n] & 0x01;
			OutputLogString(fpLog, "\t\tC2ErrorData:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			*bC2ErrorData = (pConf[uiSize+4+n] & 0x02) >> 1;
			OutputLogString(fpLog, "\t\tDigitalAudioPlay:%s\n", 
				(pConf[uiSize+4+n] & 0x80) == 0x80 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdRead:
			OutputLogString(fpLog, "\tFeatureDvdRead\n");
			OutputLogString(fpLog, "\t\tMulti110:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tDualDashR:%s\n", 
				(pConf[uiSize+6+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tDualDashRW:%s\n", 
				(pConf[uiSize+6+n] & 0x02) == 0x02 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureRandomWritable:
			OutputLogString(fpLog, "\tFeatureRandomWritable\n");
			lVal = MAKELONG(MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]), 
				MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n]));
			OutputLogString(fpLog, "\t\tLastLBA:%d\n", lVal);
			lVal = MAKELONG(MAKEWORD(pConf[uiSize+11+n], pConf[uiSize+10+n]), 
				MAKEWORD(pConf[uiSize+9+n], pConf[uiSize+8+n]));
			OutputLogString(fpLog, "\t\tLogicalBlockSize:%d\n", lVal);
			wVal = MAKEWORD(pConf[uiSize+13+n], pConf[uiSize+12+n]);
			OutputLogString(fpLog, "\t\tBlocking:%d\n", wVal);
			OutputLogString(fpLog, "\t\tErrorRecoveryPagePresent:%s\n", 
				(pConf[uiSize+14+n] & 0x01) == 0x01 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureIncrementalStreamingWritable:
			OutputLogString(fpLog, "\tFeatureIncrementalStreamingWritable\n");
			wVal = MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n]);
			OutputLogString(fpLog, 
				"\t\tDataTypeSupported:[%s]\n", wVal >= 1 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tBufferUnderrunFree:%s\n", 
				(pConf[uiSize+6+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tAddressModeReservation:%s\n", 
				(pConf[uiSize+6+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tTrackRessourceInformation:%s\n", 
				(pConf[uiSize+6+n] & 0x04) == 0x04 ? "Yes" : "No");
			OutputLogString(fpLog, 
				"\t\tNumberOfLinkSizes:%d\n", pConf[uiSize+7+n]);
			for(INT i = 1; i <= pConf[uiSize+7+n]; i++) {
				OutputLogString(fpLog, 
					"\t\tLinkSize%d:%d\n", i, pConf[uiSize+7+i+n]);
			}
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureSectorErasable:
			OutputLogString(fpLog, "\tFeatureSectorErasable\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureFormattable:
			OutputLogString(fpLog, "\tFeatureFormattable\n");
			OutputLogString(fpLog, "\t\tFullCertification:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tQuickCertification:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tSpareAreaExpansion:%s\n", 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tRENoSpareAllocated:%s\n", 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tRRandomWritable:%s\n", 
				(pConf[uiSize+8+n] & 0x01) == 0x01 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDefectManagement:
			OutputLogString(fpLog, "\tFeatureDefectManagement\n");
			OutputLogString(fpLog, "\t\tSupplimentalSpareArea:%s\n", 
				(pConf[uiSize+4+n] & 0x80) == 0x80 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureWriteOnce:
			OutputLogString(fpLog, "\tFeatureWriteOnce\n");
			lVal = MAKELONG(MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]), 
				MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n]));
			OutputLogString(fpLog, "\t\tLogicalBlockSize:%d\n", lVal);
			wVal = MAKEWORD(pConf[uiSize+9+n], pConf[uiSize+8+n]);
			OutputLogString(fpLog, "\t\tBlocking:%d\n", wVal);
			OutputLogString(fpLog, "\t\tErrorRecoveryPagePresent:%s\n", 
				(pConf[uiSize+10+n] & 0x01) == 0x01 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureRestrictedOverwrite:
			OutputLogString(fpLog, "\tFeatureRestrictedOverwrite\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureCdrwCAVWrite:
			OutputLogString(fpLog, "\tFeatureCdrwCAVWrite\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureMrw:
			OutputLogString(fpLog, "\tFeatureMrw\n");
			OutputLogString(fpLog, "\t\tWrite:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tDvdPlusRead:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tDvdPlusWrite:%s\n", 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureEnhancedDefectReporting:
			OutputLogString(fpLog, "\tFeatureEnhancedDefectReporting\n");
			OutputLogString(fpLog, "\t\tDRTDMSupported:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogString(fpLog, 
				"\t\tNumberOfDBICacheZones:%d\n", pConf[uiSize+5+n]);
			wVal = MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]);
			OutputLogString(fpLog, "\t\tNumberOfEntries:%d\n", wVal);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdPlusRW:
			OutputLogString(fpLog, "\tFeatureDvdPlusRW\n");
			OutputLogString(fpLog, "\t\tWrite:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tCloseOnly:%s\n", 
				(pConf[uiSize+5+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tQuickStart:%s\n", 
				(pConf[uiSize+5+n] & 0x02) == 0x02 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdPlusR:
			OutputLogString(fpLog, "\tFeatureDvdPlusR\n");
			OutputLogString(fpLog, "\t\tWrite:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureRigidRestrictedOverwrite:
			OutputLogString(fpLog, "\tFeatureRigidRestrictedOverwrite\n");
			OutputLogString(fpLog, "\t\tBlank:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tIntermediate:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tDefectStatusDataRead:%s\n", 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tDefectStatusDataGenerate:%s\n", 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureCdTrackAtOnce:
			OutputLogString(fpLog, "\tFeatureCdTrackAtOnce\n");
			OutputLogString(fpLog, "\t\tRWSubchannelsRecordable:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tCdRewritable:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tTestWriteOk:%s\n", 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tRWSubchannelPackedOk:%s\n", 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tRWSubchannelRawOk:%s\n", 
				(pConf[uiSize+4+n] & 0x10) == 0x10 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tBufferUnderrunFree:%s\n", 
				(pConf[uiSize+4+n] & 0x40) == 0x40 ? "Yes" : "No");
			wVal = MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]);
			OutputLogString(fpLog, "\t\tDataTypeSupported:[%s]\n", 
				wVal >= 1 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureCdMastering:
			OutputLogString(fpLog, "\tFeatureCdMastering\n");
			OutputLogString(fpLog, "\t\tRWSubchannelsRecordable:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tCdRewritable:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tTestWriteOk:%s\n", 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tRRawRecordingOk:%s\n", 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tRawMultiSessionOk:%s\n", 
				(pConf[uiSize+4+n] & 0x10) == 0x10 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tSessionAtOnceOk:%s\n", 
				(pConf[uiSize+4+n] & 0x20) == 0x20 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tBufferUnderrunFree:%s\n", 
				(pConf[uiSize+4+n] & 0x40) == 0x40 ? "Yes" : "No");
			lVal = MAKELONG(MAKEWORD(0, pConf[uiSize+7+n]), 
				MAKEWORD(pConf[uiSize+6+n], pConf[uiSize+5+n]));
			OutputLogString(fpLog, "\t\tMaximumCueSheetLength:%d\n", lVal);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdRecordableWrite:
			OutputLogString(fpLog, "\tFeatureDvdRecordableWrite\n");
			OutputLogString(fpLog, "\t\tDVD_RW:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tTestWrite:%s\n", 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tRDualLayer:%s\n", 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tBufferUnderrunFree:%s\n", 
				(pConf[uiSize+4+n] & 0x40) == 0x40 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureLayerJumpRecording:
			OutputLogString(fpLog, "\tFeatureLayerJumpRecording\n");
			OutputLogString(fpLog, 
				"\t\tNumberOfLinkSizes:%d\n", pConf[uiSize+7+n]);
			for(INT i = 1; i <= pConf[uiSize+7+n]; i++) {
				OutputLogString(fpLog, 
					"\t\tLinkSize%d:%d\n", i, pConf[uiSize+7+i+n]);
			}
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureCDRWMediaWriteSupport:
			OutputLogString(fpLog, "\tFeatureCDRWMediaWriteSupport\n");
			OutputLogString(fpLog, "\t\tSubtype0:%s\n", 
				(pConf[uiSize+5+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tSubtype1:%s\n", 
				(pConf[uiSize+5+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tSubtype2:%s\n", 
				(pConf[uiSize+5+n] & 0x04) == 0x04 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tSubtype3:%s\n", 
				(pConf[uiSize+5+n] & 0x08) == 0x08 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tSubtype4:%s\n", 
				(pConf[uiSize+5+n] & 0x10) == 0x10 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tSubtype5:%s\n", 
				(pConf[uiSize+5+n] & 0x20) == 0x20 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tSubtype6:%s\n", 
				(pConf[uiSize+5+n] & 0x40) == 0x40 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tSubtype7:%s\n", 
				(pConf[uiSize+5+n] & 0x80) == 0x80 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureBDRPseudoOverwrite:
			OutputLogString(fpLog, "\tFeatureBDRPseudoOverwrite\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdPlusRWDualLayer:
			OutputLogString(fpLog, "\tFeatureDvdPlusRWDualLayer\n");
			OutputLogString(fpLog, "\t\tWrite:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tCloseOnly:%s\n", 
				(pConf[uiSize+5+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tQuickStart:%s\n", 
				(pConf[uiSize+5+n] & 0x02) == 0x02 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdPlusRDualLayer:
			OutputLogString(fpLog, "\tFeatureDvdPlusRDualLayer\n");
			OutputLogString(fpLog, "\t\tWrite:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureBDRead:
			OutputLogString(fpLog, "\tFeatureBDRead\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureBDWrite:
			OutputLogString(fpLog, "\tFeatureBDWrite\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureTSR:
			OutputLogString(fpLog, "\tFeatureTSR\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureHDDVDRead:
			OutputLogString(fpLog, "\tFeatureHDDVDRead\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureHDDVDWrite:
			OutputLogString(fpLog, "\tFeatureHDDVDWrite\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureHybridDisc:
			OutputLogString(fpLog, "\tFeatureHybridDisc\n");
			OutputLogString(fpLog, "\t\tResetImmunity:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeaturePowerManagement:
			OutputLogString(fpLog, "\tFeaturePowerManagement\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureSMART:
			OutputLogString(fpLog, "\tFeatureSMART\n");
			OutputLogString(fpLog, 
				"\t\tFaultFailureReportingPagePresent:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureEmbeddedChanger:
			OutputLogString(fpLog, "\tFeatureEmbeddedChanger\n");
			OutputLogString(fpLog, "\t\tSupportsDiscPresent:%s\n", 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tSideChangeCapable:%s\n", 
				(pConf[uiSize+4+n] & 0x10) == 0x10 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tHighestSlotNumber [%d]\n", 
				pConf[uiSize+7+n] & 0x1F);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureCDAudioAnalogPlay:
			OutputLogString(fpLog, "\tFeatureCDAudioAnalogPlay\n");
			OutputLogString(fpLog, "\t\tSeperateVolume:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tSeperateChannelMute:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tScanSupported:%s\n", 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? "Yes" : "No");
			wVal = MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]);
			OutputLogString(fpLog, "\t\tNumerOfVolumeLevels:%d\n", wVal);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureMicrocodeUpgrade:
			OutputLogString(fpLog, "\tFeatureMicrocodeUpgrade\n");
			OutputLogString(fpLog, "\t\tM5:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureTimeout:
			OutputLogString(fpLog, "\tFeatureTimeout\n");
			OutputLogString(fpLog, "\t\tGroup3:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			wVal = MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]);
			OutputLogString(fpLog, "\t\tUnitLength:%d\n", wVal);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdCSS:
			OutputLogString(fpLog, "\tFeatureDvdCSS\n");
			OutputLogString(fpLog, "\t\tCssVersion:%d\n", pConf[uiSize+7+n]);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureRealTimeStreaming:
			OutputLogString(fpLog, "\tFeatureRealTimeStreaming\n");
			OutputLogString(fpLog, "\t\tStreamRecording:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tWriteSpeedInGetPerf:%s\n", 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tWriteSpeedInMP2A:%s\n", 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tSetCDSpeed:%s\n", 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tReadBufferCapacityBlock:%s\n", 
				(pConf[uiSize+4+n] & 0x10) == 0x10 ? "Yes" : "No");
			OutputLogString(fpLog, "\t\tSetMinimumPerformance:%s\n", 
				(pConf[uiSize+4+n] & 0x20) == 0x20 ? "Yes" : "No");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureLogicalUnitSerialNumber:
			OutputLogString(fpLog, "\tFeatureLogicalUnitSerialNumber\n");
			OutputLogString(fpLog, "\t\tSerialNumber:");
			for(INT i = 0; i < pConf[uiSize+3+n]; i++) {
				OutputLogString(fpLog, "%c", pConf[uiSize+4+i+n]);
			}
			OutputLogString(fpLog, "\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureMediaSerialNumber:
			OutputLogString(fpLog, "\tFeatureMediaSerialNumber\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDiscControlBlocks:
			OutputLogString(fpLog, "\tFeatureDiscControlBlocks\n");
			for(INT i = 0; i < pConf[uiSize+3+n]; i+=4) {
				OutputLogString(fpLog, "\t\tContentDescriptor %02d:", i/4);
				lVal = MAKELONG(MAKEWORD(pConf[uiSize+7+i+n], pConf[uiSize+6+i+n]), 
					MAKEWORD(pConf[uiSize+5+i+n], pConf[uiSize+4+i+n]));
				OutputLogString(fpLog, "%08d\n", lVal);
			}
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdCPRM:
			OutputLogString(fpLog, "\tFeatureDvdCPRM\n");
			OutputLogString(fpLog, "\t\tCPRMVersion:%d\n", pConf[uiSize+7+n]);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureFirmwareDate:
			OutputLogString(fpLog, "\tFeatureFirmwareDate\n");
			lVal = MAKELONG(MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]), 
				MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n]));
			OutputLogString(fpLog, "\t\tYear:%d\n", lVal);
			wVal = MAKEWORD(pConf[uiSize+9+n], pConf[uiSize+8+n]);
			OutputLogString(fpLog, "\t\tMonth:%d\n", wVal);
			wVal = MAKEWORD(pConf[uiSize+11+n], pConf[uiSize+10+n]);
			OutputLogString(fpLog, "\t\tDay:%d\n", wVal);
			wVal = MAKEWORD(pConf[uiSize+13+n], pConf[uiSize+12+n]);
			OutputLogString(fpLog, "\t\tHour:%d\n", wVal);
			wVal = MAKEWORD(pConf[uiSize+15+n], pConf[uiSize+14+n]);
			OutputLogString(fpLog, "\t\tMinute:%d\n", wVal);
			wVal = MAKEWORD(pConf[uiSize+17+n], pConf[uiSize+16+n]);
			OutputLogString(fpLog, "\t\tSeconds:%d\n", wVal);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureAACS:
			OutputLogString(fpLog, "\tFeatureAACS\n");
			OutputLogString(fpLog, "\t\tBindingNonceGeneration:%s\n", 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? "Yes" : "No");
			OutputLogString(fpLog, 
				"\t\tBindingNonceBlockCount:%d\n", pConf[uiSize+5+n]);
			OutputLogString(fpLog, 
				"\t\tNumberOfAGIDs:%d\n", pConf[uiSize+6+n] & 0x0F);
			OutputLogString(fpLog, "\t\tAACSVersion:%d\n", pConf[uiSize+7+n]);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureVCPS:
			OutputLogString(fpLog, "\tFeatureVCPS\n");
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		default:
			if(0xff00 <= nCode && nCode <= 0xffff) {
				OutputLogString(fpLog, 
					"\tVendor Specific. FeatureCode[0x%04X]\n", nCode);
				OutputLogString(fpLog, "\t\tVendorSpecificData:[");
			}
			else {
				OutputLogString(fpLog, 
					"\tReserved. FeatureCode[0x%04X]\n", nCode);
				OutputLogString(fpLog, "\t\tData:[");
			}
			for(INT i = 0; i < pConf[uiSize+3+n]; i++) {
				OutputLogString(fpLog, "%02X ", pConf[uiSize+4+i+n]);
			}
			OutputLogString(fpLog, "]\n");
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
			OutputLogString(fpLog, "Invalid");
			break;
		case ProfileNonRemovableDisk:
			OutputLogString(fpLog, "NonRemovableDisk");
			break;
		case ProfileRemovableDisk:
			OutputLogString(fpLog, "RemovableDisk");
			break;
		case ProfileMOErasable:
			OutputLogString(fpLog, "MOErasable");
			break;
		case ProfileMOWriteOnce:
			OutputLogString(fpLog, "MOWriteOnce");
			break;
		case ProfileAS_MO:
			OutputLogString(fpLog, "AS_MO");
			break;
		case ProfileCdrom:
			OutputLogString(fpLog, "CD-ROM");
			break;
		case ProfileCdRecordable:
			OutputLogString(fpLog, "CD-R");
			break;
		case ProfileCdRewritable:
			OutputLogString(fpLog, "CD-RW");
			break;
		case ProfileDvdRom:
			OutputLogString(fpLog, "DVD-ROM");
			break;
		case ProfileDvdRecordable:
			OutputLogString(fpLog, "DVD-R");
			break;
		case ProfileDvdRam:
			OutputLogString(fpLog, "DVD-RAM");
			break;
		case ProfileDvdRewritable:
			OutputLogString(fpLog, "DVD-RW");
			break;
		case ProfileDvdRWSequential:
			OutputLogString(fpLog, "DVD-RW Sequential");
			break;
		case ProfileDvdDashRDualLayer:
			OutputLogString(fpLog, "DVD-R DL");
			break;
		case ProfileDvdDashRLayerJump:
			OutputLogString(fpLog, "DVD-R LayerJump");
			break;
		case ProfileDvdPlusRW:
			OutputLogString(fpLog, "DVD+RW");
			break;
		case ProfileDvdPlusR:
			OutputLogString(fpLog, "DVD+R");
			break;
		case ProfileDvdPlusRWDualLayer:
			OutputLogString(fpLog, "DVD+RW DL");
			break;
		case ProfileDvdPlusRDualLayer:
			OutputLogString(fpLog, "DVD+R DL");
			break;
		case ProfileBDRom:
			OutputLogString(fpLog, "BD-ROM");
			break;
		case ProfileBDRSequentialWritable:
			OutputLogString(fpLog, "BD-SW");
			break;
		case ProfileBDRRandomWritable:
			OutputLogString(fpLog, "BD-RW");
			break;
		case ProfileBDRewritable:
			OutputLogString(fpLog, "BD-R");
			break;
		case ProfileHDDVDRom:
			OutputLogString(fpLog, "HD DVD");
			break;
		case ProfileHDDVDRecordable:
			OutputLogString(fpLog, "HD DVD-R");
			break;
		case ProfileHDDVDRam:
			OutputLogString(fpLog, "HD DVD-RAM");
			break;
		case ProfileHDDVDRewritable:
			OutputLogString(fpLog, "HD DVD-RW");
			break;
		case ProfileHDDVDRDualLayer:
			OutputLogString(fpLog, "HD DVD-R DL");
			break;
		case ProfileHDDVDRWDualLayer:
			OutputLogString(fpLog, "HD DVD-RW DL");
			break;
		case ProfileNonStandard:
			OutputLogString(fpLog, "NonStandard");
			break;
		default:
			OutputLogString(fpLog, "Reserved [%X]", usFeatureProfileType);
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
	FILE* fpSub = CreateOrOpenFile(pszSubfile, NULL, NULL, _T(".sub"), _T("rb"), 0, 0);
	FILE* fpParse = CreateOrOpenFile(pszSubfile, NULL, NULL, _T(".sub.txt"), _T("w"), 0, 0);
	if (!fpSub || !fpParse) {
		OutputErrorString("Failed to open file .sub [F:%s][L:%d]", 
			__FUNCTION__, __LINE__);
		return;
	}
	ULONG datasize = GetFilesize(fpSub, 0);
	PUCHAR data = (PUCHAR)malloc(datasize);
	if(!data) {
		OutputErrorString("Cannot alloc memory [F:%s][L:%d]\n", 
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
			INT arrayScsiStatus[][2] = {
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
			for(INT i = 0; i < sizeof(arrayScsiStatus) / sizeof(INT) / 2; i++) {
				if(swb->ScsiPassThroughDirect.ScsiStatus == arrayScsiStatus[i][0]) {
					OutputErrorString("\nSCSI bus status codes:%02X-%s [F:%s][L:%d]\n", 
						arrayScsiStatus[i][0], (CHAR*)arrayScsiStatus[i][1], 
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

void OutputSense(
	UCHAR byKey,
	UCHAR byAsc,
	UCHAR byAscq
	)
{
	OutputErrorString("Sense data, Key:Asc:Ascq: %02X:%02X:%02X", byKey, byAsc, byAscq);

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
	for(INT i = 0; i < sizeof(aSenseKey) / sizeof(INT) / 2; i++) {
		if(byKey == aSenseKey[i][0]) {
			OutputErrorString("(%s.", (CHAR*)aSenseKey[i][1]);
			break;
		}
	}
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
	BOOL bRet = FALSE;
	for(INT i = 0; i < sizeof(aSenseAscAscq) / sizeof(INT) / 3 - 1; i++) {
		if(byAsc == aSenseAscAscq[i][0]) {
			do {
				if(byAscq == aSenseAscAscq[i][1]) {
					OutputErrorString(" %s)", (CHAR*)aSenseAscAscq[i][2]);
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
		OutputErrorString(" VENDER UNIQUE ERROR)");
	}
	OutputErrorString("\n");
}

void OutputSub96(
	CONST PUCHAR pBuf,
	INT nLBA,
	FILE* fpLog
	)
{
	OutputLogString(fpLog, "Sub Channel LBA %d\n", nLBA);
	OutputLogString(fpLog, "\t  +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B\n");

	for(INT i = 0, ch = 0x50; i < CD_RAW_READ_SUBCODE_SIZE; i += 12, ch++) {
		OutputLogString(fpLog, 
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

void OutputVolumeDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	OutputLogString(fpLog, "Volume Descriptor\n");
	// 0 is Boot Record. 
	// 1 is Primary Volume Descriptor. 
	// 2 is Supplementary Volume Descriptor. 
	// 3 is Volume Partition Descriptor. 
	// 4-254 is reserved. 
	// 255 is Volume Descriptor Set Terminator.
	OutputLogString(fpLog, "\tVolume Descriptor Type		%d\n", buf[idx]);
	CHAR str[128+1];
	ZeroMemory(str, sizeof(str));
	strncpy(str, (CHAR*)&buf[idx+1], 5); str[5] = '\0';
	OutputLogString(fpLog, "\tStandard Identifier			[%s]\n", str);
	OutputLogString(fpLog, "\tVolume Descriptor Version	%d\n", buf[idx+6]);
	
	if(buf[idx] == 0) {
		strncpy(str, (CHAR*)&buf[idx+7], 32); str[32] = '\0';
		OutputLogString(fpLog, "\tBoot System Identifier		[%s]\n", str);
		strncpy(str, (CHAR*)&buf[idx+39], 32); str[32] = '\0';
		OutputLogString(fpLog, "\tBoot Identifier				[%s]\n", str);
	}
	else if(buf[idx] == 1 || buf[idx] == 2) {
		if(buf[idx] == 2) {
			OutputLogString(fpLog, 
				"\tVolume Flags					%d\n", buf[idx+6]);
		}
		strncpy(str, (CHAR*)&buf[idx+8], 32); str[32] = '\0';
		OutputLogString(fpLog, "\tSystem Identifier			[%s]\n", str);
		strncpy(str, (CHAR*)&buf[idx+40], 32); str[32] = '\0';
		OutputLogString(fpLog, "\tVolume Identifier			[%s]\n", str);
		OutputLogString(fpLog, "\tVolume Space Size			%d\n", buf[idx+80]);
		if(buf[idx+0] == 2) {
			strncpy(str, (CHAR*)&buf[idx+88], 32); str[32] = '\0';
			OutputLogString(fpLog, 
				"\tEscape Sequences				[%s]\n", str);
		}
		OutputLogString(fpLog, "\tVolume Set Size				%d\n", 
			MAKEWORD(buf[idx+123], buf[idx+122]));
		OutputLogString(fpLog, "\tVolume Sequence Number		%d\n", 
			MAKEWORD(buf[idx+127], buf[idx+126]));
		OutputLogString(fpLog, "\tLogical Block Size			%d\n", 
			MAKEWORD(buf[idx+131], buf[idx+130]));
		OutputLogString(fpLog, "\tPath Table Size				%d\n", 
			MAKELONG(MAKEWORD(buf[idx+139], buf[idx+138]), 
			MAKEWORD(buf[idx+137], buf[idx+136])));
		OutputLogString(fpLog, 
			"\tLocation of Occurrence of Type L Path Table				%d\n", 
			MAKEWORD(buf[idx+143], buf[idx+142]));
		OutputLogString(fpLog, 
			"\tLocation of Optional Occurrence of Type L Path Table	%d\n", 
			MAKEWORD(buf[idx+147], buf[idx+146]));
		OutputLogString(fpLog, 
			"\tLocation of Occurrence of Type M Path Table				%d\n", 
			MAKEWORD(buf[idx+151], buf[idx+150]));
		OutputLogString(fpLog, 
			"\tLocation of Optional Occurrence of Type M Path Table	%d\n", 
			MAKEWORD(buf[idx+155], buf[idx+154]));

		OutputLogString(fpLog, 
			"\tDirectory Record\n");
		OutputLogString(fpLog, 
			"\t\tLength of Directory Record			%d\n", buf[idx+156]);
		OutputLogString(fpLog, 
			"\t\tExtended Attribute Record Length	%d\n", buf[idx+157]);
		OutputLogString(fpLog, 
			"\t\tLocation of Extent					%d\n", 
			MAKELONG(MAKEWORD(buf[idx+165], buf[idx+164]), 
			MAKEWORD(buf[idx+163], buf[idx+162])));
		OutputLogString(fpLog, 
			"\t\tData Length							%d\n", 
			MAKELONG(MAKEWORD(buf[idx+173], buf[idx+172]), 
			MAKEWORD(buf[idx+171], buf[idx+170])));
		OutputLogString(fpLog, 
			"\t\tRecording Date and Time				%d-%02d-%02d %02d:%02d:%02d +%02d\n", 
			buf[idx+174] + 1900, buf[idx+175], buf[idx+176], 
			buf[idx+177], buf[idx+178], buf[idx+179], buf[idx+180]);
		OutputLogString(fpLog, 
			"\t\tFile Flags							%d\n", buf[idx+181]);
		OutputLogString(fpLog, 
			"\t\tFile Unit Size						%d\n", buf[idx+182]);
		OutputLogString(fpLog, 
			"\t\tInterleave Gap Size					%d\n", buf[idx+183]);
		OutputLogString(fpLog, 
			"\t\tVolume Sequence Number				%d\n", 
			MAKEWORD(buf[idx+187], buf[idx+186]));
		OutputLogString(fpLog, 
			"\t\tLength of File Identifier			%d\n", buf[idx+188]);
		OutputLogString(fpLog, 
			"\t\tFile Identifier						%d\n", buf[idx+189]);
		strncpy(str, (CHAR*)&buf[idx+190], 128); str[128] = '\0';
		OutputLogString(fpLog, "\tVolume Set Identifier		[%s]\n", str);
		strncpy(str, (CHAR*)&buf[idx+318], 128); str[128] = '\0';
		OutputLogString(fpLog, "\tPublisher Identifier		[%s]\n", str);
		strncpy(str, (CHAR*)&buf[idx+446], 128); str[128] = '\0';
		OutputLogString(fpLog, "\tData Preparer Identifier	[%s]\n", str);
		strncpy(str, (CHAR*)&buf[idx+574], 128); str[128] = '\0';
		OutputLogString(fpLog, "\tApplication Identifier		[%s]\n", str);
		strncpy(str, (CHAR*)&buf[idx+702], 37); str[37] = '\0';
		OutputLogString(fpLog, "\tCopyright File Identifier		[%s]\n", str);
		strncpy(str, (CHAR*)&buf[idx+702], 37); str[37] = '\0';
		OutputLogString(fpLog, "\tAbstract File Identifier		[%s]\n", str);
		strncpy(str, (CHAR*)&buf[idx+776], 37); str[37] = '\0';
		OutputLogString(fpLog, "\tBibliographic File Identifier	[%s]\n", str);

		CHAR year[4+1], month[2+1], day[2+1], hour[2+1], time[2+1], second[2+1], milisecond[2+1];
		strncpy(year, (CHAR*)&buf[idx+813], 4); year[4] = '\0';
		strncpy(month, (CHAR*)&buf[idx+817], 2); month[2] = '\0';
		strncpy(day, (CHAR*)&buf[idx+819], 2); day[2] = '\0';
		strncpy(hour, (CHAR*)&buf[idx+821], 2); hour[2] = '\0';
		strncpy(time, (CHAR*)&buf[idx+823], 2); time[2] = '\0';
		strncpy(second, (CHAR*)&buf[idx+825], 2); second[2] = '\0';
		strncpy(milisecond, (CHAR*)&buf[idx+827], 2); milisecond[2] = '\0';
		OutputLogString(fpLog, 
			"\tVolume Creation Date and Time		%s-%s-%s %s:%s:%s.%s +%d\n", 
			year, month, day, hour, time, second, milisecond, buf[idx+829]);
		strncpy(year, (CHAR*)&buf[idx+830], 4); year[4] = '\0';
		strncpy(month, (CHAR*)&buf[idx+834], 2); month[2] = '\0';
		strncpy(day, (CHAR*)&buf[idx+836], 2); day[2] = '\0';
		strncpy(hour, (CHAR*)&buf[idx+838], 2); hour[2] = '\0';
		strncpy(time, (CHAR*)&buf[idx+840], 2); time[2] = '\0';
		strncpy(second, (CHAR*)&buf[idx+842], 2); second[2] = '\0';
		strncpy(milisecond, (CHAR*)&buf[idx+844], 2); milisecond[2] = '\0';
		OutputLogString(fpLog, 
			"\tVolume Modification Date and Time	%s-%s-%s %s:%s:%s.%s +%d\n", 
			year, month, day, hour, time, second, milisecond, buf[idx+846]);
		strncpy(year, (CHAR*)&buf[idx+847], 4); year[4] = '\0';
		strncpy(month, (CHAR*)&buf[idx+851], 2); month[2] = '\0';
		strncpy(day, (CHAR*)&buf[idx+853], 2); day[2] = '\0';
		strncpy(hour, (CHAR*)&buf[idx+855], 2); hour[2] = '\0';
		strncpy(time, (CHAR*)&buf[idx+857], 2); time[2] = '\0';
		strncpy(second, (CHAR*)&buf[idx+859], 2); second[2] = '\0';
		strncpy(milisecond, (CHAR*)&buf[idx+861], 2); milisecond[2] = '\0';
		OutputLogString(fpLog, 
			"\tVolume Expiration Date and Time		%s-%s-%s %s:%s:%s.%s +%d\n", 
			year, month, day, hour, time, second, milisecond, buf[idx+863]);
		strncpy(year, (CHAR*)&buf[idx+864], 4); year[4] = '\0';
		strncpy(month, (CHAR*)&buf[idx+868], 2); month[2] = '\0';
		strncpy(day, (CHAR*)&buf[idx+870], 2); day[2] = '\0';
		strncpy(hour, (CHAR*)&buf[idx+872], 2); hour[2] = '\0';
		strncpy(time, (CHAR*)&buf[idx+874], 2); time[2] = '\0';
		strncpy(second, (CHAR*)&buf[idx+876], 2); second[2] = '\0';
		strncpy(milisecond, (CHAR*)&buf[idx+878], 2); milisecond[2] = '\0';
		OutputLogString(fpLog, 
			"\tVolume Effective Date and Time		%s-%s-%s %s:%s:%s.%s +%d\n", 
			year, month, day, hour, time, second, milisecond, buf[idx+880]);
		OutputLogString(fpLog, 
			"\tFile Structure Version				%d\n", buf[idx+881]);
	}
	else if(buf[idx] == 3) {
		strncpy(str, (CHAR*)&buf[idx+8], 32); str[32] = '\0';
		OutputLogString(fpLog, "\tSystem Identifier				[%s]\n", str);
		strncpy(str, (CHAR*)&buf[idx+40], 32); str[32] = '\0';
		OutputLogString(fpLog, "\tVolume Partition Identifier	[%s]\n", str);
		OutputLogString(fpLog, "\tVolume Partition Location		%d\n", 
			MAKELONG(MAKEWORD(buf[idx+79], buf[idx+78]), 
			MAKEWORD(buf[idx+77], buf[idx+76])));
		OutputLogString(fpLog, "\tVolume Partition Size			%d\n", 
			MAKELONG(MAKEWORD(buf[idx+87], buf[idx+86]), 
			MAKEWORD(buf[idx+85], buf[idx+84])));
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
