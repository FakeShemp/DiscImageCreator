/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"

_TCHAR szCatalog[META_CATALOG_SIZE+1];
_TCHAR szISRC[MAXIMUM_NUMBER_TRACKS][META_ISRC_SIZE+1];
_TCHAR szPerformer[META_STRING_SIZE+1];
_TCHAR szSongWriter[META_STRING_SIZE+1];
_TCHAR szAlbumTitle[META_STRING_SIZE+1];
_TCHAR szTitle[MAXIMUM_NUMBER_TRACKS][META_STRING_SIZE+1];

FILE* CreateOrOpenFileW(
	LPCTSTR pszSrcPath,
	LPTSTR pszOutPath,
	LPTSTR pszFileNameWithoutPath,
	LPTSTR pszFileNameWithoutPathAndExt,
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
	if(pszFileNameWithoutPathAndExt != NULL) {
		// size of pszFileNameWithoutPathAndExt must be _MAX_PATH.
		ZeroMemory(pszFileNameWithoutPathAndExt, _MAX_FNAME);
		_tsplitpath(szDstPath, drive, dir, fname, ext);
		_stprintf(pszFileNameWithoutPathAndExt, _T("%s"), fname);
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
	OutputLogString(fpLog, _T("c2 error LBA %d\n"), nLBA);

	for(INT i = 0; i < CD_RAW_READ_C2_SIZE; i+=8) {
		OutputLogString(fpLog, 
			_T("\t%02X %02X %02X %02X %02X %02X %02X %02X\n"), 
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
	OutputLogString(fpLog, _T("Main Channel LBA %d\n"), nLBA);
	OutputLogString(fpLog, _T("\t    +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n"));

	for(INT i = 0; i < CD_RAW_SECTOR_SIZE; i += 16) {
		OutputLogString(fpLog, 
			_T("\t%3X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n"), 
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
	OutputLogString(fpLog, _T("Device Info\n"));
	OutputLogString(fpLog, _T("\tDeviceType:"));
	switch(pInquiry[0] & 0x1F) {
	case 5:
		OutputLogString(fpLog, _T("CD/DVD device\n"));
		break;
	default:
		OutputLogString(fpLog, _T("Other device\n"));
		break;
	}
	OutputLogString(fpLog, _T("\tDeviceTypeQualifier:%d\n"), pInquiry[0] >> 5 & 0x3);

	OutputLogString(fpLog, _T("\tDeviceTypeModifier:%d\n"), pInquiry[1] & 0x7F);
	OutputLogString(fpLog, _T("\tRemovableMedia:%s\n"),
		(pInquiry[1] >> 7 & 0x1) == 0 ? _T("No") :_T( "Yes"));

	OutputLogString(fpLog, _T("\tVersions:%d\n"), pInquiry[2]);

	OutputLogString(fpLog, _T("\tResponseDataFormat:%d\n"), pInquiry[3] & 0x0F);
	OutputLogString(fpLog, _T("\tHiSupport:%s\n"),
		(pInquiry[3] >> 4 & 0x1) == 0 ? _T("No") :_T( "Yes"));
	OutputLogString(fpLog, _T("\tNormACA:%s\n"),
		(pInquiry[3] >> 5 & 0x1) == 0 ? _T("No") :_T( "Yes"));
	OutputLogString(fpLog, _T("\tTerminateTask:%s\n"),
		(pInquiry[3] >> 6 & 0x1) == 0 ? _T("No") :_T( "Yes"));
	OutputLogString(fpLog, _T("\tAERC:%s\n"),
		(pInquiry[3] >> 7 & 0x1) == 0 ? _T("No") :_T( "Yes"));

	OutputLogString(fpLog, _T("\tAdditionalLength:%d\n"), pInquiry[4]);

	OutputLogString(fpLog, _T("\tMediumChanger:%s\n"),
		(pInquiry[6] >> 3 & 0x1) == 0 ? _T("No") :_T( "Yes"));
	OutputLogString(fpLog, _T("\tMultiPort:%s\n"),
		(pInquiry[6] >> 4 & 0x1) == 0 ? _T("No") :_T( "Yes"));
	OutputLogString(fpLog, _T("\tEnclosureServices:%s\n"),
		(pInquiry[6] >> 6 & 0x1) == 0 ? _T("No") :_T( "Yes"));

	OutputLogString(fpLog, _T("\tSoftReset:%s\n"),
		(pInquiry[7] & 0x1) == 0 ? _T("No") :_T( "Yes"));
	OutputLogString(fpLog, _T("\tCommandQueue:%s\n"),
		(pInquiry[7] >> 1 & 0x1) == 0 ? _T("No") :_T( "Yes"));
	OutputLogString(fpLog, _T("\tLinkedCommands:%s\n"),
		(pInquiry[7] >> 3 & 0x1) == 0 ? _T("No") :_T( "Yes"));
	OutputLogString(fpLog, _T("\tRelativeAddressing:%s\n"),
		(pInquiry[7] >> 7 & 0x1) == 0 ? _T("No") :_T( "Yes"));

	OutputLogString(fpLog, _T("\tVendorId:"));
	INT i = 8;
	while(i < 16) {
		OutputLogString(fpLog, _T("%c"), pInquiry[i]);
		pszVendorId[i-8] = pInquiry[i++];
	}
	OutputLogString(fpLog, _T("\n"));

	OutputLogString(fpLog, _T("\tProductId:")); 
	while(i < 32) {
		OutputLogString(fpLog, _T("%c"), pInquiry[i]);
		pszProductId[i-16] = pInquiry[i++];
	}
	OutputLogString(fpLog, _T("\n"));

	OutputLogString(fpLog, _T("\tProductRevisionLevel:")); 
	while(i < 36) {
		OutputLogString(fpLog, _T("%c"), pInquiry[i++]);
	}
	OutputLogString(fpLog, _T("\n"));

	OutputLogString(fpLog, _T("\tVendorSpecific:")); 
	while(i < 56) {
		OutputLogString(fpLog, _T("%c"), pInquiry[i++]);
	}
	OutputLogString(fpLog, _T("\n"));
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
		OutputLogString(fpLog, _T("\tPhysicalFormatInformation\n"));
		OutputLogString(fpLog, _T("\t\tBookVersion:%d\n"), pStructure[4] & 0x0F);
		LPCTSTR lpBookType[] = {
			_T("DVD-ROM"), _T("DVD-RAM"), _T("DVD-R"), _T("DVD-RW"), _T("HD DVD-ROM"), _T("HD DVD-RAM"),
			_T("HD DVD-R"), _T("Reserved"), _T("Reserved"),	_T("DVD+RW"), _T("DVD+R"), _T("Reserved"),
			_T("Reserved"), _T("DVD+RW DL"), _T("DVD+R DL"), _T("Reserved")
		};
		OutputLogString(fpLog, _T("\t\tBookType:%s\n"), lpBookType[pStructure[4]>>4&0x0F]);

		LPCTSTR lpMaximumRate[] = {
			_T("2.52Mbps"), _T("5.04Mbps"), _T("10.08Mbps"), _T("20.16Mbps"), _T("30.24Mbps"), _T("Reserved"),
			_T("Reserved"), _T("Reserved"), _T("Reserved"), _T("Reserved"), _T("Reserved"), _T("Reserved"),
			_T("Reserved"), _T("Reserved"), _T("Not Specified")
		};
		OutputLogString(fpLog, _T("\t\tMinimumRate:%s\n"), lpMaximumRate[pStructure[5]&0x0F]);
		OutputLogString(fpLog, _T("\t\tDiskSize:%s\n"), (pStructure[5] & 0xF0) == 0 ? _T("120mm") : _T("80mm"));

		LPCTSTR lpLayerType[] = {
			_T("Layer contains embossed data"), _T("Layer contains recordable data"), 
			_T("Layer contains rewritable data"), _T("Reserved")
		};
		OutputLogString(fpLog, _T("\t\tLayerType:%s\n"), lpLayerType[pStructure[6]&0x0F]);
		OutputLogString(fpLog, _T("\t\tTrackPath:%s\n"), 
			(pStructure[6] & 0x10) == 0 ? _T("Parallel Track Path") : _T("Opposite Track Path"));
		OutputLogString(fpLog, _T("\t\tNumberOfLayers:%s\n"), 
			(pStructure[6] & 0x60) == 0 ? _T("Single Layer") : _T("Double Layer"));

		LPCTSTR lpTrackDensity[] = {
			_T("0.74μm/track"), _T("0.80μm/track"), _T("0.615μm/track"), _T("0.40μm/track"),
			_T("0.34μm/track"), _T("Reserved"), _T("Reserved"), _T("Reserved"), _T("Reserved"), _T("Reserved"),
			_T("Reserved"), _T("Reserved"), _T("Reserved"), _T("Reserved"), _T("Reserved"), _T("Reserved")
		};
		OutputLogString(fpLog, _T("\t\tTrackDensity:%s\n"), lpTrackDensity[pStructure[7]&0x0F]);

		LPCTSTR lpLinearDensity[] = {
			_T("0.267μm/bit"), _T("0.293μm/bit"), _T("0.409 to 0.435μm/bit"), _T("Reserved"),
			_T("0.280 to 0.291μm/bit"), _T("0.153μm/bit"), _T("0.130 to 0.140μm/bit"),
			_T("Reserved"), _T("0.353μm/bit"), _T("Reserved"), _T("Reserved"), _T("Reserved"),
			_T("Reserved"), _T("Reserved"), _T("Reserved")
		};
		OutputLogString(fpLog, _T("\t\tLinearDensity:%s\n"), lpLinearDensity[pStructure[7]>>4&0x0F]);

		LONG ulStartSectorNum = MAKELONG(MAKEWORD(pStructure[11], 
			pStructure[10]), MAKEWORD(pStructure[9], pStructure[8]));
		OutputLogString(fpLog, _T("\t\tStartDataSector:%d(0x%X)\n"), 
			ulStartSectorNum, ulStartSectorNum);
		LONG ulEndSectorNum = MAKELONG(MAKEWORD(pStructure[15], 
			pStructure[14]), MAKEWORD(pStructure[13], pStructure[12]));
		OutputLogString(fpLog, _T("\t\tEndDataSector:%d(0x%X)\n"), 
			ulEndSectorNum, ulEndSectorNum);
		*nDVDSectorSize = ulEndSectorNum - ulStartSectorNum + 1;
		INT ulEndSectorLayer0 = MAKELONG(MAKEWORD(pStructure[19], 
			pStructure[18]), MAKEWORD(pStructure[17], pStructure[16]));
		OutputLogString(fpLog, _T("\t\tEndLayerZeroSector:%X\n"), ulEndSectorLayer0);

		OutputLogString(fpLog, _T("\t\tBCAFlag:%s\n"), 
			(pStructure[20] & 0x80) == 0 ? _T("None") : _T("Exist"));

		OutputLogString(fpLog, _T("\t\tMediaSpecific:"));
		for(ULONG k = 0; k < 2031; k++) {
			OutputLogString(fpLog, _T("%02X"), pStructure[21+k]);
		}
		OutputLogString(fpLog, _T("\n"));
		break;
	}
	case DvdCopyrightDescriptor:
		OutputLogString(fpLog, _T("\tCopyrightProtectionType:"));
		switch(pStructure[4]) {
		case 0:
			OutputLogString(fpLog, _T("No\n"));
			break;
		case 1:
			OutputLogString(fpLog, _T("CSS/CPPM\n"));
			break;
		case 2:
			OutputLogString(fpLog, _T("CPRM\n"));
			break;
		case 3:
			OutputLogString(fpLog, _T("AACS with HD DVD content\n"));
			break;
		case 10:
			OutputLogString(fpLog, _T("AACS with BD content\n"));
			break;
		default:
			OutputLogString(fpLog, _T("Unknown:[%02X]\n"), pStructure[4]);
			break;
		}
		OutputLogString(fpLog, 
			_T("\tRegionManagementInformation:%02X\n"), pStructure[5]);
		break;
	case DvdDiskKeyDescriptor:
		OutputLogString(fpLog, _T("\tDiskKeyData:"));
		for(ULONG k = 0; k < 2048; k++) {
			OutputLogString(fpLog, _T("%02X"), pStructure[4+k]);
		}
		OutputLogString(fpLog, _T("\n"));
		break;
	case DvdBCADescriptor:
		OutputLogString(fpLog, _T("\tBCAInformation:"));
		for(ULONG k = 0; 
			k < pStructureLength[i] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
			OutputLogString(fpLog, _T("%02X"), pStructure[4+k]);
		}
		OutputLogString(fpLog, _T("\n"));
		break;
	case DvdManufacturerDescriptor:
		OutputLogString(fpLog, _T("\tManufacturingInformation:"));
		for(ULONG k = 0; k < 2048; k++) {
			OutputLogString(fpLog, _T("%02X"), pStructure[4+k]);
		}
		OutputLogString(fpLog, _T("\n"));
		break;
	case 6:
		OutputLogString(fpLog, _T("\tmedia ID:"));
		for(ULONG k = 0; 
			k < pStructureLength[i] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
			OutputLogString(fpLog, _T("%02X"), pStructure[4+k]);
		}
		OutputLogString(fpLog, _T("\n"));
		break;
	case 7:
		OutputLogString(fpLog, _T("\tMedia Key Block Total Packs %d"), pStructure[3]);
		OutputLogString(fpLog, _T("\tmedia key block:"));
		for(ULONG k = 0; 
			k < pStructureLength[i] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
			OutputLogString(fpLog, _T("%02X"), pStructure[4+k]);
		}
		OutputLogString(fpLog, _T("\n"));
		break;
	default:
		OutputLogString(fpLog, _T("Unknown:[%02X]\n"), pFormat[i]);
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
			OutputLogString(fpLog, _T("\tFeatureProfileList\n"));
			OutputLogString(fpLog, _T("\t\t"));
			while(n < pConf[uiSize+3]) {
				OutputFeatureProfileType(fpLog, 
					MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n]));
				OutputLogString(fpLog, _T(", "));
				n += sizeof(FEATURE_DATA_PROFILE_LIST_EX);
			}
			OutputLogString(fpLog, _T("\n"));
			n += sizeof(FEATURE_HEADER);
			break;
		case FeatureCore: {
			OutputLogString(fpLog, _T("\tFeatureCore\n"));
			OutputLogString(fpLog, _T("\t\tPhysicalInterface:"));
			lVal = MAKELONG(MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]), 
				MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n]));
			switch(lVal) {
			case 0:
				OutputLogString(fpLog, _T("Unspecified\n"));
				break;
			case 1:
				OutputLogString(fpLog, _T("SCSI Family\n"));
				break;
			case 2:
				OutputLogString(fpLog, _T("ATAPI\n"));
				break;
			case 3:
				OutputLogString(fpLog, _T("IEEE 1394 - 1995\n"));
				break;
			case 4:
				OutputLogString(fpLog, _T("IEEE 1394A\n"));
				break;
			case 5:
				OutputLogString(fpLog, _T("Fibre Channel\n"));
				break;
			case 6:
				OutputLogString(fpLog, _T("IEEE 1394B\n"));
				break;
			case 7:
				OutputLogString(fpLog, _T("Serial ATAPI\n"));
				break;
			case 8:
				OutputLogString(fpLog, _T("USB (both 1.1 and 2.0)\n"));
				break;
			case 0xFFFF:
				OutputLogString(fpLog, _T("Vendor Unique\n"));
				break;
			default:
				OutputLogString(fpLog, _T("Reserved:[%08d]\n"), lVal);
				break;
			}
			OutputLogString(fpLog, _T("\t\tDeviceBusyEvent:%s\n"), 
				(pConf[uiSize+8+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tINQUIRY2:%s\n"), 
				(pConf[uiSize+8+n] & 0x02) == 0x02 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		}
		case FeatureMorphing:
			OutputLogString(fpLog, _T("\tFeatureMorphing\n"));
			OutputLogString(fpLog, _T("\t\tAsynchronous:%s\n"), 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tOCEvent:%s\n"), 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureRemovableMedium:
			OutputLogString(fpLog, _T("\tFeatureRemovableMedium\n"));
			OutputLogString(fpLog, _T("\t\tLockable:%s\n"), 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tDBML:%s\n"), 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tDefaultToPrevent:%s\n"), 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tEject:%s\n"), 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tLoad:%s\n"), 
				(pConf[uiSize+4+n] & 0x10) == 0x10 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tLoadingMechanism:")); 
			switch(pConf[uiSize+4+n] >> 5 & 0x07) {
			case 0:
				OutputLogString(fpLog, _T("Caddy/Slot type loading mechanism\n"));
				break;
			case 1:
				OutputLogString(fpLog, _T("Tray type loading mechanism\n"));
				break;
			case 2:
				OutputLogString(fpLog, _T("Pop-up type loading mechanism\n"));
				break;
			case 4:
				OutputLogString(fpLog, 
					_T("Embedded changer with individually changeable discs\n"));
				break;
			case 5:
				OutputLogString(fpLog, 
					_T("Embedded changer using a magazine mechanism\n"));
				break;
			default:
				OutputLogString(fpLog, 
					_T("Reserved:[%08d]\n"), pConf[uiSize+4+n] >> 5 & 0x07);
				break;
			}
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureWriteProtect:
			OutputLogString(fpLog, _T("\tFeatureWriteProtect\n"));
			OutputLogString(fpLog, _T("\t\tSupportsSWPPBit:%s\n"), 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tSupportsPersistentWriteProtect:%s\n"), 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tWriteInhibitDCB:%s\n"), 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tDiscWriteProtectPAC:%s\n"), 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureRandomReadable:
			OutputLogString(fpLog, _T("\tFeatureRandomReadable\n"));
			lVal = MAKELONG(MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]), 
				MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n]));
			OutputLogString(fpLog, _T("\t\tLogicalBlockSize:%d\n"), lVal);
			wVal = MAKEWORD(pConf[uiSize+9+n], pConf[uiSize+8+n]);
			OutputLogString(fpLog, _T("\t\tBlocking:%d\n"), wVal);
			OutputLogString(fpLog, _T("\t\tErrorRecoveryPagePresent:%s\n"), 
				(pConf[uiSize+10+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureMultiRead:
			OutputLogString(fpLog, _T("\tFeatureMultiRead\n"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureCdRead:
			OutputLogString(fpLog, _T("\tFeatureCdRead\n"));
			OutputLogString(fpLog, _T("\t\tCDText:%s\n"), 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			*bCanCDText = (BOOL)(pConf[uiSize+4+n] & 0x01);
			OutputLogString(fpLog, _T("\t\tC2ErrorData:%s\n"), 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? _T("Yes") : _T("No"));
			*bC2ErrorData = (BOOL)((pConf[uiSize+4+n] & 0x02) >> 1);
			OutputLogString(fpLog, _T("\t\tDigitalAudioPlay:%s\n"), 
				(pConf[uiSize+4+n] & 0x80) == 0x80 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdRead:
			OutputLogString(fpLog, _T("\tFeatureDvdRead\n"));
			OutputLogString(fpLog, _T("\t\tMulti110:%s\n"), 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tDualDashR:%s\n"), 
				(pConf[uiSize+6+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tDualDashRW:%s\n"), 
				(pConf[uiSize+6+n] & 0x02) == 0x02 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureRandomWritable:
			OutputLogString(fpLog, _T("\tFeatureRandomWritable\n"));
			lVal = MAKELONG(MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]), 
				MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n]));
			OutputLogString(fpLog, _T("\t\tLastLBA:%d\n"), lVal);
			lVal = MAKELONG(MAKEWORD(pConf[uiSize+11+n], pConf[uiSize+10+n]), 
				MAKEWORD(pConf[uiSize+9+n], pConf[uiSize+8+n]));
			OutputLogString(fpLog, _T("\t\tLogicalBlockSize:%d\n"), lVal);
			wVal = MAKEWORD(pConf[uiSize+13+n], pConf[uiSize+12+n]);
			OutputLogString(fpLog, _T("\t\tBlocking:%d\n"), wVal);
			OutputLogString(fpLog, _T("\t\tErrorRecoveryPagePresent:%s\n"), 
				(pConf[uiSize+14+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureIncrementalStreamingWritable:
			OutputLogString(fpLog, _T("\tFeatureIncrementalStreamingWritable\n"));
			wVal = MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n]);
			OutputLogString(fpLog, 
				_T("\t\tDataTypeSupported:[%s]\n"), wVal >= 1 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tBufferUnderrunFree:%s\n"), 
				(pConf[uiSize+6+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tAddressModeReservation:%s\n"), 
				(pConf[uiSize+6+n] & 0x02) == 0x02 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tTrackRessourceInformation:%s\n"), 
				(pConf[uiSize+6+n] & 0x04) == 0x04 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, 
				_T("\t\tNumberOfLinkSizes:%d\n"), pConf[uiSize+7+n]);
			for(INT i = 1; i <= pConf[uiSize+7+n]; i++) {
				OutputLogString(fpLog, 
					_T("\t\tLinkSize%d:%d\n"), i, pConf[uiSize+7+i+n]);
			}
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureSectorErasable:
			OutputLogString(fpLog, _T("\tFeatureSectorErasable\n"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureFormattable:
			OutputLogString(fpLog, _T("\tFeatureFormattable\n"));
			OutputLogString(fpLog, _T("\t\tFullCertification:%s\n"), 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tQuickCertification:%s\n"), 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tSpareAreaExpansion:%s\n"), 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tRENoSpareAllocated:%s\n"), 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tRRandomWritable:%s\n"), 
				(pConf[uiSize+8+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDefectManagement:
			OutputLogString(fpLog, _T("\tFeatureDefectManagement\n"));
			OutputLogString(fpLog, _T("\t\tSupplimentalSpareArea:%s\n"), 
				(pConf[uiSize+4+n] & 0x80) == 0x80 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureWriteOnce:
			OutputLogString(fpLog, _T("\tFeatureWriteOnce\n"));
			lVal = MAKELONG(MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]), 
				MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n]));
			OutputLogString(fpLog, _T("\t\tLogicalBlockSize:%d\n"), lVal);
			wVal = MAKEWORD(pConf[uiSize+9+n], pConf[uiSize+8+n]);
			OutputLogString(fpLog, _T("\t\tBlocking:%d\n"), wVal);
			OutputLogString(fpLog, _T("\t\tErrorRecoveryPagePresent:%s\n"), 
				(pConf[uiSize+10+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureRestrictedOverwrite:
			OutputLogString(fpLog, _T("\tFeatureRestrictedOverwrite\n"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureCdrwCAVWrite:
			OutputLogString(fpLog, _T("\tFeatureCdrwCAVWrite\n"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureMrw:
			OutputLogString(fpLog, _T("\tFeatureMrw\n"));
			OutputLogString(fpLog, _T("\t\tWrite:%s\n"), 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tDvdPlusRead:%s\n"), 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tDvdPlusWrite:%s\n"), 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureEnhancedDefectReporting:
			OutputLogString(fpLog, _T("\tFeatureEnhancedDefectReporting\n"));
			OutputLogString(fpLog, _T("\t\tDRTDMSupported:%s\n"), 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, 
				_T("\t\tNumberOfDBICacheZones:%d\n"), pConf[uiSize+5+n]);
			wVal = MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]);
			OutputLogString(fpLog, _T("\t\tNumberOfEntries:%d\n"), wVal);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdPlusRW:
			OutputLogString(fpLog, _T("\tFeatureDvdPlusRW\n"));
			OutputLogString(fpLog, _T("\t\tWrite:%s\n"), 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tCloseOnly:%s\n"), 
				(pConf[uiSize+5+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tQuickStart:%s\n"), 
				(pConf[uiSize+5+n] & 0x02) == 0x02 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdPlusR:
			OutputLogString(fpLog, _T("\tFeatureDvdPlusR\n"));
			OutputLogString(fpLog, _T("\t\tWrite:%s\n"), 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureRigidRestrictedOverwrite:
			OutputLogString(fpLog, _T("\tFeatureRigidRestrictedOverwrite\n"));
			OutputLogString(fpLog, _T("\t\tBlank:%s\n"), 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tIntermediate:%s\n"), 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tDefectStatusDataRead:%s\n"), 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tDefectStatusDataGenerate:%s\n"), 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureCdTrackAtOnce:
			OutputLogString(fpLog, _T("\tFeatureCdTrackAtOnce\n"));
			OutputLogString(fpLog, _T("\t\tRWSubchannelsRecordable:%s\n"), 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tCdRewritable:%s\n"), 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tTestWriteOk:%s\n"), 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tRWSubchannelPackedOk:%s\n"), 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tRWSubchannelRawOk:%s\n"), 
				(pConf[uiSize+4+n] & 0x10) == 0x10 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tBufferUnderrunFree:%s\n"), 
				(pConf[uiSize+4+n] & 0x40) == 0x40 ? _T("Yes") : _T("No"));
			wVal = MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]);
			OutputLogString(fpLog, _T("\t\tDataTypeSupported:[%s]\n"), 
				wVal >= 1 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureCdMastering:
			OutputLogString(fpLog, _T("\tFeatureCdMastering\n"));
			OutputLogString(fpLog, _T("\t\tRWSubchannelsRecordable:%s\n"), 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tCdRewritable:%s\n"), 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tTestWriteOk:%s\n"), 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tRRawRecordingOk:%s\n"), 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tRawMultiSessionOk:%s\n"), 
				(pConf[uiSize+4+n] & 0x10) == 0x10 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tSessionAtOnceOk:%s\n"), 
				(pConf[uiSize+4+n] & 0x20) == 0x20 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tBufferUnderrunFree:%s\n"), 
				(pConf[uiSize+4+n] & 0x40) == 0x40 ? _T("Yes") : _T("No"));
			lVal = MAKELONG(MAKEWORD(0, pConf[uiSize+7+n]), 
				MAKEWORD(pConf[uiSize+6+n], pConf[uiSize+5+n]));
			OutputLogString(fpLog, _T("\t\tMaximumCueSheetLength:%d\n"), lVal);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdRecordableWrite:
			OutputLogString(fpLog, _T("\tFeatureDvdRecordableWrite\n"));
			OutputLogString(fpLog, _T("\t\tDVD_RW:%s\n"), 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tTestWrite:%s\n"), 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tRDualLayer:%s\n"), 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tBufferUnderrunFree:%s\n"), 
				(pConf[uiSize+4+n] & 0x40) == 0x40 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureLayerJumpRecording:
			OutputLogString(fpLog, _T("\tFeatureLayerJumpRecording\n"));
			OutputLogString(fpLog, 
				_T("\t\tNumberOfLinkSizes:%d\n"), pConf[uiSize+7+n]);
			for(INT i = 1; i <= pConf[uiSize+7+n]; i++) {
				OutputLogString(fpLog, 
					_T("\t\tLinkSize%d:%d\n"), i, pConf[uiSize+7+i+n]);
			}
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureCDRWMediaWriteSupport:
			OutputLogString(fpLog, _T("\tFeatureCDRWMediaWriteSupport\n"));
			OutputLogString(fpLog, _T("\t\tSubtype0:%s\n"), 
				(pConf[uiSize+5+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tSubtype1:%s\n"), 
				(pConf[uiSize+5+n] & 0x02) == 0x02 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tSubtype2:%s\n"), 
				(pConf[uiSize+5+n] & 0x04) == 0x04 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tSubtype3:%s\n"), 
				(pConf[uiSize+5+n] & 0x08) == 0x08 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tSubtype4:%s\n"), 
				(pConf[uiSize+5+n] & 0x10) == 0x10 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tSubtype5:%s\n"), 
				(pConf[uiSize+5+n] & 0x20) == 0x20 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tSubtype6:%s\n"), 
				(pConf[uiSize+5+n] & 0x40) == 0x40 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tSubtype7:%s\n"), 
				(pConf[uiSize+5+n] & 0x80) == 0x80 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureBDRPseudoOverwrite:
			OutputLogString(fpLog, _T("\tFeatureBDRPseudoOverwrite\n"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdPlusRWDualLayer:
			OutputLogString(fpLog, _T("\tFeatureDvdPlusRWDualLayer\n"));
			OutputLogString(fpLog, _T("\t\tWrite:%s\n"), 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tCloseOnly:%s\n"), 
				(pConf[uiSize+5+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tQuickStart:%s\n"), 
				(pConf[uiSize+5+n] & 0x02) == 0x02 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdPlusRDualLayer:
			OutputLogString(fpLog, _T("\tFeatureDvdPlusRDualLayer\n"));
			OutputLogString(fpLog, _T("\t\tWrite:%s\n"), 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureBDRead:
			OutputLogString(fpLog, _T("\tFeatureBDRead\n"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureBDWrite:
			OutputLogString(fpLog, _T("\tFeatureBDWrite\n"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureTSR:
			OutputLogString(fpLog, _T("\tFeatureTSR\n"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureHDDVDRead:
			OutputLogString(fpLog, _T("\tFeatureHDDVDRead\n"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureHDDVDWrite:
			OutputLogString(fpLog, _T("\tFeatureHDDVDWrite\n"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureHybridDisc:
			OutputLogString(fpLog, _T("\tFeatureHybridDisc\n"));
			OutputLogString(fpLog, _T("\t\tResetImmunity:%s\n"), 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeaturePowerManagement:
			OutputLogString(fpLog, _T("\tFeaturePowerManagement\n"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureSMART:
			OutputLogString(fpLog, _T("\tFeatureSMART\n"));
			OutputLogString(fpLog, 
				_T("\t\tFaultFailureReportingPagePresent:%s\n"), 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureEmbeddedChanger:
			OutputLogString(fpLog, _T("\tFeatureEmbeddedChanger\n"));
			OutputLogString(fpLog, _T("\t\tSupportsDiscPresent:%s\n"), 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tSideChangeCapable:%s\n"), 
				(pConf[uiSize+4+n] & 0x10) == 0x10 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tHighestSlotNumber [%d]\n"), 
				pConf[uiSize+7+n] & 0x1F);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureCDAudioAnalogPlay:
			OutputLogString(fpLog, _T("\tFeatureCDAudioAnalogPlay\n"));
			OutputLogString(fpLog, _T("\t\tSeperateVolume:%s\n"), 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tSeperateChannelMute:%s\n"), 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tScanSupported:%s\n"), 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? _T("Yes") : _T("No"));
			wVal = MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]);
			OutputLogString(fpLog, _T("\t\tNumerOfVolumeLevels:%d\n"), wVal);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureMicrocodeUpgrade:
			OutputLogString(fpLog, _T("\tFeatureMicrocodeUpgrade\n"));
			OutputLogString(fpLog, _T("\t\tM5:%s\n"), 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureTimeout:
			OutputLogString(fpLog, _T("\tFeatureTimeout\n"));
			OutputLogString(fpLog, _T("\t\tGroup3:%s\n"), 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			wVal = MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]);
			OutputLogString(fpLog, _T("\t\tUnitLength:%d\n"), wVal);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdCSS:
			OutputLogString(fpLog, _T("\tFeatureDvdCSS\n"));
			OutputLogString(fpLog, _T("\t\tCssVersion:%d\n"), pConf[uiSize+7+n]);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureRealTimeStreaming:
			OutputLogString(fpLog, _T("\tFeatureRealTimeStreaming\n"));
			OutputLogString(fpLog, _T("\t\tStreamRecording:%s\n"), 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tWriteSpeedInGetPerf:%s\n"), 
				(pConf[uiSize+4+n] & 0x02) == 0x02 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tWriteSpeedInMP2A:%s\n"), 
				(pConf[uiSize+4+n] & 0x04) == 0x04 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tSetCDSpeed:%s\n"), 
				(pConf[uiSize+4+n] & 0x08) == 0x08 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tReadBufferCapacityBlock:%s\n"), 
				(pConf[uiSize+4+n] & 0x10) == 0x10 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, _T("\t\tSetMinimumPerformance:%s\n"), 
				(pConf[uiSize+4+n] & 0x20) == 0x20 ? _T("Yes") : _T("No"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureLogicalUnitSerialNumber:
			OutputLogString(fpLog, _T("\tFeatureLogicalUnitSerialNumber\n"));
			OutputLogString(fpLog, _T("\t\tSerialNumber:"));
			for(INT i = 0; i < pConf[uiSize+3+n]; i++) {
				OutputLogString(fpLog, _T("%c"), pConf[uiSize+4+i+n]);
			}
			OutputLogString(fpLog, _T("\n"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureMediaSerialNumber:
			OutputLogString(fpLog, _T("\tFeatureMediaSerialNumber\n"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDiscControlBlocks:
			OutputLogString(fpLog, _T("\tFeatureDiscControlBlocks\n"));
			for(INT i = 0; i < pConf[uiSize+3+n]; i+=4) {
				OutputLogString(fpLog, _T("\t\tContentDescriptor %02d:"), i/4);
				lVal = MAKELONG(MAKEWORD(pConf[uiSize+7+i+n], pConf[uiSize+6+i+n]), 
					MAKEWORD(pConf[uiSize+5+i+n], pConf[uiSize+4+i+n]));
				OutputLogString(fpLog, _T("%08d\n"), lVal);
			}
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureDvdCPRM:
			OutputLogString(fpLog, _T("\tFeatureDvdCPRM\n"));
			OutputLogString(fpLog, _T("\t\tCPRMVersion:%d\n"), pConf[uiSize+7+n]);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureFirmwareDate:
			OutputLogString(fpLog, _T("\tFeatureFirmwareDate\n"));
			lVal = MAKELONG(MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]), 
				MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n]));
			OutputLogString(fpLog, _T("\t\tYear:%d\n"), lVal);
			wVal = MAKEWORD(pConf[uiSize+9+n], pConf[uiSize+8+n]);
			OutputLogString(fpLog, _T("\t\tMonth:%d\n"), wVal);
			wVal = MAKEWORD(pConf[uiSize+11+n], pConf[uiSize+10+n]);
			OutputLogString(fpLog, _T("\t\tDay:%d\n"), wVal);
			wVal = MAKEWORD(pConf[uiSize+13+n], pConf[uiSize+12+n]);
			OutputLogString(fpLog, _T("\t\tHour:%d\n"), wVal);
			wVal = MAKEWORD(pConf[uiSize+15+n], pConf[uiSize+14+n]);
			OutputLogString(fpLog, _T("\t\tMinute:%d\n"), wVal);
			wVal = MAKEWORD(pConf[uiSize+17+n], pConf[uiSize+16+n]);
			OutputLogString(fpLog, _T("\t\tSeconds:%d\n"), wVal);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureAACS:
			OutputLogString(fpLog, _T("\tFeatureAACS\n"));
			OutputLogString(fpLog, _T("\t\tBindingNonceGeneration:%s\n"), 
				(pConf[uiSize+4+n] & 0x01) == 0x01 ? _T("Yes") : _T("No"));
			OutputLogString(fpLog, 
				_T("\t\tBindingNonceBlockCount:%d\n"), pConf[uiSize+5+n]);
			OutputLogString(fpLog, 
				_T("\t\tNumberOfAGIDs:%d\n"), pConf[uiSize+6+n] & 0x0F);
			OutputLogString(fpLog, _T("\t\tAACSVersion:%d\n"), pConf[uiSize+7+n]);
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		case FeatureVCPS:
			OutputLogString(fpLog, _T("\tFeatureVCPS\n"));
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
			break;
		default:
			if(0xff00 <= nCode && nCode <= 0xffff) {
				OutputLogString(fpLog, 
					_T("\tVendor Specific. FeatureCode[0x%04X]\n"), nCode);
				OutputLogString(fpLog, _T("\t\tVendorSpecificData:["));
			}
			else {
				OutputLogString(fpLog, 
					_T("\tReserved. FeatureCode[0x%04X]\n"), nCode);
				OutputLogString(fpLog, _T("\t\tData:["));
			}
			for(INT i = 0; i < pConf[uiSize+3+n]; i++) {
				OutputLogString(fpLog, _T("%02X "), pConf[uiSize+4+i+n]);
			}
			OutputLogString(fpLog, _T("]\n"));
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
			OutputLogString(fpLog, _T("Invalid"));
			break;
		case ProfileNonRemovableDisk:
			OutputLogString(fpLog, _T("NonRemovableDisk"));
			break;
		case ProfileRemovableDisk:
			OutputLogString(fpLog, _T("RemovableDisk"));
			break;
		case ProfileMOErasable:
			OutputLogString(fpLog, _T("MOErasable"));
			break;
		case ProfileMOWriteOnce:
			OutputLogString(fpLog, _T("MOWriteOnce"));
			break;
		case ProfileAS_MO:
			OutputLogString(fpLog, _T("AS_MO"));
			break;
		case ProfileCdrom:
			OutputLogString(fpLog, _T("CD-ROM"));
			break;
		case ProfileCdRecordable:
			OutputLogString(fpLog, _T("CD-R"));
			break;
		case ProfileCdRewritable:
			OutputLogString(fpLog, _T("CD-RW"));
			break;
		case ProfileDvdRom:
			OutputLogString(fpLog, _T("DVD-ROM"));
			break;
		case ProfileDvdRecordable:
			OutputLogString(fpLog, _T("DVD-R"));
			break;
		case ProfileDvdRam:
			OutputLogString(fpLog, _T("DVD-RAM"));
			break;
		case ProfileDvdRewritable:
			OutputLogString(fpLog, _T("DVD-RW"));
			break;
		case ProfileDvdRWSequential:
			OutputLogString(fpLog, _T("DVD-RW Sequential"));
			break;
		case ProfileDvdDashRDualLayer:
			OutputLogString(fpLog, _T("DVD-R DL"));
			break;
		case ProfileDvdDashRLayerJump:
			OutputLogString(fpLog, _T("DVD-R LayerJump"));
			break;
		case ProfileDvdPlusRW:
			OutputLogString(fpLog, _T("DVD+RW"));
			break;
		case ProfileDvdPlusR:
			OutputLogString(fpLog, _T("DVD+R"));
			break;
		case ProfileDvdPlusRWDualLayer:
			OutputLogString(fpLog, _T("DVD+RW DL"));
			break;
		case ProfileDvdPlusRDualLayer:
			OutputLogString(fpLog, _T("DVD+R DL"));
			break;
		case ProfileBDRom:
			OutputLogString(fpLog, _T("BD-ROM"));
			break;
		case ProfileBDRSequentialWritable:
			OutputLogString(fpLog, _T("BD-SW"));
			break;
		case ProfileBDRRandomWritable:
			OutputLogString(fpLog, _T("BD-RW"));
			break;
		case ProfileBDRewritable:
			OutputLogString(fpLog, _T("BD-R"));
			break;
		case ProfileHDDVDRom:
			OutputLogString(fpLog, _T("HD DVD"));
			break;
		case ProfileHDDVDRecordable:
			OutputLogString(fpLog, _T("HD DVD-R"));
			break;
		case ProfileHDDVDRam:
			OutputLogString(fpLog, _T("HD DVD-RAM"));
			break;
		case ProfileHDDVDRewritable:
			OutputLogString(fpLog, _T("HD DVD-RW"));
			break;
		case ProfileHDDVDRDualLayer:
			OutputLogString(fpLog, _T("HD DVD-R DL"));
			break;
		case ProfileHDDVDRWDualLayer:
			OutputLogString(fpLog, _T("HD DVD-RW DL"));
			break;
		case ProfileNonStandard:
			OutputLogString(fpLog, _T("NonStandard"));
			break;
		default:
			OutputLogString(fpLog, _T("Reserved [%X]"), usFeatureProfileType);
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
	FILE* fpSub = CreateOrOpenFileW(pszSubfile, NULL, NULL, NULL, _T(".sub"), _T("rb"), 0, 0);
#ifdef UNICODE
	FILE* fpParse = CreateOrOpenFileW(pszSubfile, NULL, NULL, NULL, _T(".sub.txt"), _T("w, ccs=UTF-8"), 0, 0);
#else
	FILE* fpParse = CreateOrOpenFileW(pszSubfile, NULL, NULL, NULL, _T(".sub.txt"), _T("w"), 0, 0);
#endif
	if (!fpSub || !fpParse) {
		OutputErrorString(_T("Failed to open file .sub [F:%s][L:%d]"), 
			_T(__FUNCTION__), __LINE__);
		return;
	}
	ULONG datasize = GetFilesize(fpSub, 0);
	PUCHAR data = (PUCHAR)malloc(datasize);
	if(!data) {
		OutputErrorString(_T("Cannot alloc memory [F:%s][L:%d]\n"), 
			_T(__FUNCTION__), __LINE__);
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
		OutputString(_T("\rParse sub(Size) %6d/%6d"), i, datasize);
	}
	OutputString(_T("\n"));
	FcloseAndNull(fpParse);
}

#ifdef WIN64
	typedef INT64 INT_T;
#else
	typedef INT INT_T;
#endif

void OutputScsiStatus(
	CONST PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER swb,
	PUCHAR byScsiStatus,
	LPCTSTR pszFuncname,
	INT nLineNum
	)
{
	INT_T aScsiStatus[][2] = {
		{SCSISTAT_GOOD, (INT_T)_T("GOOD")}, 
		{SCSISTAT_CHECK_CONDITION, (INT_T)_T("CHECK_CONDITION")}, 
		{SCSISTAT_CONDITION_MET, (INT_T)_T("CONDITION_MET")}, 
		{SCSISTAT_BUSY, (INT_T)_T("BUSY")}, 
		{SCSISTAT_INTERMEDIATE, (INT_T)_T("INTERMEDIATE")}, 
		{SCSISTAT_INTERMEDIATE_COND_MET, (INT_T)_T("INTERMEDIATE_COND_MET")}, 
		{SCSISTAT_RESERVATION_CONFLICT, (INT_T)_T("RESERVATION_CONFLICT")}, 
		{SCSISTAT_COMMAND_TERMINATED, (INT_T)_T("COMMAND_TERMINATED")}, 
		{SCSISTAT_QUEUE_FULL, (INT_T)_T("QUEUE_FULL")}
	};
	if(swb->ScsiPassThroughDirect.ScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		UCHAR key = (UCHAR)(swb->SenseInfoBuffer[2] & 0x0F);
		UCHAR ASC = swb->SenseInfoBuffer[12];
		UCHAR ASCQ = swb->SenseInfoBuffer[13];
		if(key != SCSI_SENSE_NO_SENSE || ASC != SCSI_ADSENSE_NO_SENSE || ASCQ != 0x00) {
			for(INT i = 0; i < sizeof(aScsiStatus) / sizeof(INT) / 2; i++) {
				if(swb->ScsiPassThroughDirect.ScsiStatus == aScsiStatus[i][0]) {
					OutputErrorString(
						_T("\nSCSI bus status codes:%02X-%s [F:%s][L:%d]\n"), 
						aScsiStatus[i][0], (_TCHAR*)aScsiStatus[i][1], 
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
	INT_T aSenseKey[][2] = {
		{SCSI_SENSE_NO_SENSE, (INT_T)_T("NO_SENSE")},
		{SCSI_SENSE_RECOVERED_ERROR, (INT_T)_T("RECOVERED_ERROR")}, 
		{SCSI_SENSE_NOT_READY, (INT_T)_T("NOT_READY")}, 
		{SCSI_SENSE_MEDIUM_ERROR, (INT_T)_T("MEDIUM_ERROR")}, 
		{SCSI_SENSE_HARDWARE_ERROR, (INT_T)_T("HARDWARE_ERROR")}, 
		{SCSI_SENSE_ILLEGAL_REQUEST, (INT_T)_T("ILLEGAL_REQUEST")}, 
		{SCSI_SENSE_UNIT_ATTENTION, (INT_T)_T("UNIT_ATTENTION")}, 
		{SCSI_SENSE_DATA_PROTECT, (INT_T)_T("DATA_PROTECT")}, 
		{SCSI_SENSE_BLANK_CHECK, (INT_T)_T("BLANK_CHECK")}, 
		{SCSI_SENSE_UNIQUE, (INT_T)_T("UNIQUE")}, 
		{SCSI_SENSE_COPY_ABORTED, (INT_T)_T("COPY_ABORTED")}, 
		{SCSI_SENSE_ABORTED_COMMAND, (INT_T)_T("ABORTED_COMMAND")}, 
		{SCSI_SENSE_EQUAL, (INT_T)_T("EQUAL")}, 
		{SCSI_SENSE_VOL_OVERFLOW, (INT_T)_T("VOL_OVERFLOW")}, 
		{SCSI_SENSE_MISCOMPARE, (INT_T)_T("MISCOMPARE")} 
	};
	// only C/DVD Device (MMC-6)
	INT_T aSenseAscAscq[][3] = {
		{SCSI_ADSENSE_NO_SENSE, 0x00, (INT_T)_T("NO SENSE")},
		{SCSI_ADSENSE_NO_SEEK_COMPLETE, 0x00, (INT_T)_T("NO SEEK COMPLETE")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_CAUSE_NOT_REPORTABLE, (INT_T)_T("LUN_NOT_READY - CAUSE_NOT_REPORTABLE")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_BECOMING_READY, (INT_T)_T("LUN_NOT_READY - BECOMING_READY")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_INIT_COMMAND_REQUIRED, (INT_T)_T("LUN_NOT_READY - INIT_COMMAND_REQUIRED")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_MANUAL_INTERVENTION_REQUIRED, (INT_T)_T("LUN_NOT_READY - MANUAL_INTERVENTION_REQUIRED")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_FORMAT_IN_PROGRESS, (INT_T)_T("LUN_NOT_READY - FORMAT_IN_PROGRESS")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_OPERATION_IN_PROGRESS, (INT_T)_T("LUN_NOT_READY - OPERATION_IN_PROGRESS")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_LONG_WRITE_IN_PROGRESS, (INT_T)_T("LUN_NOT_READY - LONG_WRITE_IN_PROGRESS")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x09, (INT_T)_T("LUN_NOT_READY - SELF-TEST IN PROGRESS")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x0A, (INT_T)_T("LUN_NOT_READY - ASYMMETRIC ACCESS STATE TRANSITION")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x0B, (INT_T)_T("LUN_NOT_READY - TARGET PORT IN STANDBY STATE")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x0C, (INT_T)_T("LUN_NOT_READY - TARGET PORT IN UNAVAILABLE STATE")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x10, (INT_T)_T("LUN_NOT_READY - AUXILIARY MEMORY NOT ACCESSIBLE")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x11, (INT_T)_T("LUN_NOT_READY - NOTIFY (ENABLE SPINUP) REQUIRED")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x13, (INT_T)_T("LUN_NOT_READY - SA CREATION IN PROGRESS")}, 
		{0x05, 0x00, (INT_T)_T("LOGICAL UNIT DOES NOT RESPOND TO SELECTION")}, 
		{0x06, 0x00, (INT_T)_T("NO REFERENCE POSITION FOUND")}, 
		{0x07, 0x00, (INT_T)_T("MULTIPLE PERIPHERAL DEVICES SELECTED")}, 
		{SCSI_ADSENSE_LUN_COMMUNICATION, SCSI_SENSEQ_COMM_FAILURE, (INT_T)_T("LUN_COMMUNICATION - COMM_FAILURE")}, 
		{SCSI_ADSENSE_LUN_COMMUNICATION, SCSI_SENSEQ_COMM_TIMEOUT, (INT_T)_T("LUN_COMMUNICATION - COMM_TIMEOUT")}, 
		{SCSI_ADSENSE_LUN_COMMUNICATION, SCSI_SENSEQ_COMM_PARITY_ERROR, (INT_T)_T("LUN_COMMUNICATION - COMM_PARITY_ERROR")}, 
		{SCSI_ADSENSE_LUN_COMMUNICATION, SCSI_SESNEQ_COMM_CRC_ERROR, (INT_T)_T("LUN_COMMUNICATION - COMM_CRC_ERROR")}, 
		{SCSI_ADSENSE_LUN_COMMUNICATION, SCSI_SENSEQ_UNREACHABLE_TARGET, (INT_T)_T("LUN_COMMUNICATION - UNREACHABLE_TARGET")}, 
		{0x09, 0x00, (INT_T)_T("TRACK FOLLOWING ERROR")}, 
		{0x09, 0x01, (INT_T)_T("TRACKING SERVO FAILURE")}, 
		{0x09, 0x02, (INT_T)_T("FOCUS SERVO FAILURE")}, 
		{0x09, 0x03, (INT_T)_T("SPINDLE SERVO FAILURE")}, 
		{0x09, 0x04, (INT_T)_T("HEAD SELECT FAULT")}, 
		{0x0A, 0x00, (INT_T)_T("ERROR LOG OVERFLOW")}, 
		{0x0B, 0x00, (INT_T)_T("WARNING")}, 
		{0x0B, 0x01, (INT_T)_T("WARNING - SPECIFIED TEMPERATURE EXCEEDED")}, 
		{0x0B, 0x02, (INT_T)_T("WARNING - ENCLOSURE DEGRADED")}, 
		{0x0B, 0x03, (INT_T)_T("WARNING - BACKGROUND SELF-TEST FAILED")}, 
		{0x0B, 0x04, (INT_T)_T("WARNING - BACKGROUND PRE-SCAN DETECTED MEDIUM ERROR")}, 
		{0x0B, 0x05, (INT_T)_T("WARNING - BACKGROUND MEDIUM SCAN DETECTED MEDIUM ERROR")}, 
		{0x0B, 0x06, (INT_T)_T("WARNING - NON-VOLATILE CACHE NOW VOLATILE")}, 
		{0x0B, 0x07, (INT_T)_T("WARNING - DEGRADED POWER TO NON-VOLATILE CACHE")}, 
		{0x0B, 0x08, (INT_T)_T("WARNING - POWER LOSS EXPECTED")}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x00, (INT_T)_T("WRITE ERROR")}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x06, (INT_T)_T("BLOCK NOT COMPRESSIBLE")}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x07, (INT_T)_T("WRITE ERROR - RECOVERY NEEDED")}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x08, (INT_T)_T("WRITE ERROR - RECOVERY FAILED")}, 
		{SCSI_ADSENSE_WRITE_ERROR, SCSI_SENSEQ_LOSS_OF_STREAMING, (INT_T)_T("WRITE ERROR - LOSS OF STREAMING")}, 
		{SCSI_ADSENSE_WRITE_ERROR, SCSI_SENSEQ_PADDING_BLOCKS_ADDED, (INT_T)_T("WRITE ERROR - PADDING BLOCKS ADDED")}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x0B, (INT_T)_T("AUXILIARY MEMORY WRITE ERROR")}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x0C, (INT_T)_T("WRITE ERROR - UNEXPECTED UNSOLICITED DATA")}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x0D, (INT_T)_T("WRITE ERROR - NOT ENOUGH UNSOLICITED DATA")}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x0F, (INT_T)_T("DEFECTS IN ERROR WINDOW")}, 
		{0x0D, 0x00, (INT_T)_T("ERROR DETECTED BY THIRD PARTY TEMPORARY INITIATOR")}, 
		{0x0D, 0x01, (INT_T)_T("THIRD PARTY DEVICE FAILURE")}, 
		{0x0D, 0x02, (INT_T)_T("COPY TARGET DEVICE NOT REACHABLE")}, 
		{0x0D, 0x03, (INT_T)_T("INCORRECT COPY TARGET DEVICE TYPE")}, 
		{0x0D, 0x04, (INT_T)_T("COPY TARGET DEVICE DATA UNDERRUN")}, 
		{0x0D, 0x05, (INT_T)_T("COPY TARGET DEVICE DATA OVERRUN")}, 
		{0x0E, 0x00, (INT_T)_T("INVALID INFORMATION UNIT")}, 
		{0x0E, 0x01, (INT_T)_T("INFORMATION UNIT TOO SHORT")}, 
		{0x0E, 0x02, (INT_T)_T("INFORMATION UNIT TOO LONG")}, 
		{0x0E, 0x03, (INT_T)_T("INVALID FIELD IN COMMAND INFORMATION UNIT")}, 
		{0x11, 0x00, (INT_T)_T("UNRECOVERED READ ERROR")}, 
		{0x11, 0x01, (INT_T)_T("READ RETRIES EXHAUSTED")}, 
		{0x11, 0x02, (INT_T)_T("ERROR TOO LONG TO CORRECT")}, 
		{0x11, 0x05, (INT_T)_T("L-EC UNCORRECTABLE ERROR")}, 
		{0x11, 0x06, (INT_T)_T("CIRC UNRECOVERED ERROR")}, 
		{0x11, 0x0D, (INT_T)_T("DE-COMPRESSION CRC ERROR")}, 
		{0x11, 0x0E, (INT_T)_T("CANNOT DECOMPRESS USING DECLARED ALGORITHM")}, 
		{0x11, 0x0F, (INT_T)_T("ERROR READING UPC/EAN NUMBER")}, 
		{0x11, 0x10, (INT_T)_T("ERROR READING ISRC NUMBER")}, 
		{0x11, 0x11, (INT_T)_T("READ ERROR - LOSS OF STREAMING")}, 
		{0x11, 0x12, (INT_T)_T("AUXILIARY MEMORY READ ERROR")}, 
		{0x11, 0x13, (INT_T)_T("READ ERROR - FAILED RETRANSMISSION REQUEST")}, 
		{SCSI_ADSENSE_TRACK_ERROR, 0x00, (INT_T)_T("TRACK_ERROR - RECORDED ENTITY NOT FOUND")}, 
		{SCSI_ADSENSE_TRACK_ERROR, 0x01, (INT_T)_T("TRACK_ERROR - RECORD NOT FOUND")}, 
		{SCSI_ADSENSE_SEEK_ERROR, 0x00, (INT_T)_T("SEEK_ERROR - RANDOM POSITIONING ERROR")}, 
		{SCSI_ADSENSE_SEEK_ERROR, 0x01, (INT_T)_T("SEEK_ERROR - MECHANICAL POSITIONING ERROR")}, 
		{SCSI_ADSENSE_SEEK_ERROR, 0x02, (INT_T)_T("SEEK_ERROR - POSITIONING ERROR DETECTED BY READ OF MEDIUM")}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x00, (INT_T)_T("REC_DATA_NOECC - RECOVERED DATA WITH NO ERROR CORRECTION APPLIED")}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x01, (INT_T)_T("REC_DATA_NOECC - RECOVERED DATA WITH RETRIES")}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x02, (INT_T)_T("REC_DATA_NOECC - RECOVERED DATA WITH POSITIVE HEAD OFFSET")}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x03, (INT_T)_T("REC_DATA_NOECC - RECOVERED DATA WITH NEGATIVE HEAD OFFSET")}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x04, (INT_T)_T("REC_DATA_NOECC - RECOVERED DATA WITH RETRIES AND/OR CIRC APPLIED")}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x05, (INT_T)_T("REC_DATA_NOECC - RECOVERED DATA USING PREVIOUS SECTOR ID")}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x07, (INT_T)_T("REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - RECOMMEND REASSIGNMENT")}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x08, (INT_T)_T("REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - RECOMMEND REWRITE")}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x09, (INT_T)_T("REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - DATA REWRITTEN")}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x00, (INT_T)_T("REC_DATA_ECC - RECOVERED DATA WITH ERROR CORRECTION APPLIED")}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x01, (INT_T)_T("REC_DATA_ECC - RECOVERED DATA WITH ERROR CORR. & RETRIES APPLIED")}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x02, (INT_T)_T("REC_DATA_ECC - RECOVERED DATA - DATA AUTO-REALLOCATED")}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x03, (INT_T)_T("REC_DATA_ECC - RECOVERED DATA WITH CIRC")}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x04, (INT_T)_T("REC_DATA_ECC - RECOVERED DATA WITH L-EC")}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x05, (INT_T)_T("REC_DATA_ECC - RECOVERED DATA - RECOMMEND REASSIGNMENT")}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x06, (INT_T)_T("REC_DATA_ECC - RECOVERED DATA - RECOMMEND REWRITE")}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x08, (INT_T)_T("REC_DATA_ECC - RECOVERED DATA WITH LINKING")}, 
		{SCSI_ADSENSE_PARAMETER_LIST_LENGTH, 0x00, (INT_T)_T("PARAMETER LIST LENGTH ERROR")}, 
		{0x1B, 0x00, (INT_T)_T("SYNCHRONOUS DATA TRANSFER ERROR")}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x00, (INT_T)_T("INVALID COMMAND OPERATION CODE")}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x01, (INT_T)_T("ILLEGAL_COMMAND - ACCESS DENIED - INITIATOR PENDING-ENROLLED")}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x02, (INT_T)_T("ILLEGAL_COMMAND - ACCESS DENIED - NO ACCESS RIGHTS")}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x03, (INT_T)_T("ILLEGAL_COMMAND - ACCESS DENIED - INVALID MGMT ID KEY")}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x08, (INT_T)_T("ILLEGAL_COMMAND - ACCESS DENIED - ENROLLMENT CONFLICT")}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x09, (INT_T)_T("ILLEGAL_COMMAND - ACCESS DENIED - INVALID LU IDENTIFIER")}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x0A, (INT_T)_T("ILLEGAL_COMMAND - ACCESS DENIED - INVALID PROXY TOKEN")}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x0B, (INT_T)_T("ILLEGAL_COMMAND - ACCESS DENIED - ACL LUN CONFLICT")}, 
		{SCSI_ADSENSE_ILLEGAL_BLOCK, 0x00, (INT_T)_T("LOGICAL BLOCK ADDRESS OUT OF RANGE")}, 
		{SCSI_ADSENSE_ILLEGAL_BLOCK, SCSI_SENSEQ_ILLEGAL_ELEMENT_ADDR, (INT_T)_T("ILLEGAL_BLOCK - MISCOMPARE DURING VERIFY OPERATION")}, 
		{SCSI_ADSENSE_ILLEGAL_BLOCK, 0x02, (INT_T)_T("ILLEGAL_BLOCK - INVALID ADDRESS FOR WRITE")}, 
		{SCSI_ADSENSE_ILLEGAL_BLOCK, 0x03, (INT_T)_T("ILLEGAL_BLOCK - INVALID WRITE CROSSING LAYER JUMP")}, 
		{0x23, 0x00, (INT_T)_T("INVALID TOKEN OPERATION, CAUSE NOT REPORTABLE")}, 
		{SCSI_ADSENSE_INVALID_CDB, 0x00, (INT_T)_T("INVALID FIELD IN CDB")}, 
		{SCSI_ADSENSE_INVALID_CDB, 0x01, (INT_T)_T("CDB DECRYPTION ERROR")}, 
		{SCSI_ADSENSE_INVALID_CDB, 0x08, (INT_T)_T("INVALID XCDB")}, 
		{SCSI_ADSENSE_INVALID_LUN, 0x00, (INT_T)_T("LOGICAL UNIT NOT SUPPORTED")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x00, (INT_T)_T("INVALID FIELD IN PARAMETER LIST")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x01, (INT_T)_T("PARAMETER NOT SUPPORTED")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x02, (INT_T)_T("PARAMETER VALUE INVALID")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x03, (INT_T)_T("THRESHOLD PARAMETERS NOT SUPPORTED")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x04, (INT_T)_T("INVALID RELEASE OF PERSISTENT RESERVATION")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x05, (INT_T)_T("DATA DECRYPTION ERROR")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x06, (INT_T)_T("TOO MANY TARGET DESCRIPTORS")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x07, (INT_T)_T("UNSUPPORTED SEGMENT DESCRIPTOR TYPE CODE")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x08, (INT_T)_T("TOO MANY SEGMENT DESCRIPTORS")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x09, (INT_T)_T("UNSUPPORTED SEGMENT DESCRIPTOR TYPE CODE")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x0A, (INT_T)_T("UNEXPECTED INEXACT SEGMENT")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x0B, (INT_T)_T("INLINE DATA LENGTH EXCEEDED")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x0C, (INT_T)_T("INVALID OPERATION FOR COPY SOURCE OR DESTINATION")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x0D, (INT_T)_T("COPY SEGMENT GRANULARITY VIOLATION")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x0E, (INT_T)_T("INVALID PARAMETER WHILE PORT IS ENABLED")}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x00, (INT_T)_T("WRITE PROTECTED")}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x01, (INT_T)_T("HARDWARE WRITE PROTECTED")}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x02, (INT_T)_T("LOGICAL UNIT SOFTWARE WRITE PROTECTED")}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x03, (INT_T)_T("ASSOCIATED WRITE PROTECT")}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x04, (INT_T)_T("PERSISTENT WRITE PROTECT")}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x05, (INT_T)_T("PERMANENT WRITE PROTECT")}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x06, (INT_T)_T("CONDITIONAL WRITE PROTECT")}, 
		{SCSI_ADSENSE_MEDIUM_CHANGED, 0x00, (INT_T)_T("NOT READY TO READY CHANGE, MEDIUM MAY HAVE CHANGED")}, 
		{SCSI_ADSENSE_MEDIUM_CHANGED, 0x01, (INT_T)_T("IMPORT OR EXPORT ELEMENT ACCESSED")}, 
		{SCSI_ADSENSE_MEDIUM_CHANGED, 0x02, (INT_T)_T("FORMAT-LAYER MAY HAVE CHANGED")}, 
		{SCSI_ADSENSE_BUS_RESET, 0x00, (INT_T)_T("POWER ON, RESET, OR BUS DEVICE RESET OCCURRED")}, 
		{SCSI_ADSENSE_BUS_RESET, 0x01, (INT_T)_T("POWER ON OCCURRED")}, 
		{SCSI_ADSENSE_BUS_RESET, 0x02, (INT_T)_T("SCSI BUS RESET OCCURRED")}, 
		{SCSI_ADSENSE_BUS_RESET, 0x03, (INT_T)_T("BUS DEVICE RESET FUNCTION OCCURRED")}, 
		{SCSI_ADSENSE_BUS_RESET, 0x04, (INT_T)_T("DEVICE INTERNAL RESET")}, 
		{SCSI_ADSENSE_BUS_RESET, 0x05, (INT_T)_T("TRANSCEIVER MODE CHANGED TO SINGLE-ENDED")}, 
		{SCSI_ADSENSE_BUS_RESET, 0x06, (INT_T)_T("TRANSCEIVER MODE CHANGED TO LVD")}, 
		{SCSI_ADSENSE_BUS_RESET, 0x07, (INT_T)_T("I_T NEXUS LOSS OCCURRED")}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x00, (INT_T)_T("PARAMETERS CHANGED")}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x01, (INT_T)_T("MODE PARAMETERS CHANGED")}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x02, (INT_T)_T("LOG PARAMETERS CHANGED")}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x03, (INT_T)_T("RESERVATIONS PREEMPTED")}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x04, (INT_T)_T("RESERVATIONS RELEASED")}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x05, (INT_T)_T("REGISTRATIONS PREEMPTED")}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x06, (INT_T)_T("ASYMMETRIC ACCESS STATE CHANGED")}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x07, (INT_T)_T("IMPLICIT ASYMMETRIC ACCESS STATE TRANSITION FAILED")}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x08, (INT_T)_T("PRIORITY CHANGED")}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x14, (INT_T)_T("SA CREATION CAPABILITIES DATA HAS CHANGED")}, 
		{0x2B, 0x00, (INT_T)_T("COPY CANNOT EXECUTE SINCE HOST CANNOT DISCONNECT")}, 
		{0x2C, 0x00, (INT_T)_T("COMMAND SEQUENCE ERROR")}, 
		{0x2C, 0x03, (INT_T)_T("CURRENT PROGRAM AREA IS NOT EMPTY")}, 
		{0x2C, 0x04, (INT_T)_T("CURRENT PROGRAM AREA IS EMPTY")}, 
		{0x2C, 0x06, (INT_T)_T("PERSISTENT PREVENT CONFLICT")}, 
		{0x2C, 0x07, (INT_T)_T("PREVIOUS BUSY STATUS")}, 
		{0x2C, 0x08, (INT_T)_T("PREVIOUS TASK SET FULL STATUS")}, 
		{0x2C, 0x09, (INT_T)_T("PREVIOUS RESERVATION CONFLICT STATUS")}, 
		{SCSI_ADSENSE_INSUFFICIENT_TIME_FOR_OPERATION, 0x00, (INT_T)_T("INSUFFICIENT TIME FOR OPERATION")}, 
		{0x2F, 0x00, (INT_T)_T("COMMANDS CLEARED BY ANOTHER INITIATOR")}, 
		{0x2F, 0x02, (INT_T)_T("COMMANDS CLEARED BY DEVICE SERVER")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x00, (INT_T)_T("INCOMPATIBLE MEDIUM INSTALLED")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x01, (INT_T)_T("CANNOT READ MEDIUM - UNKNOWN FORMAT")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x02, (INT_T)_T("CANNOT READ MEDIUM - INCOMPATIBLE FORMAT")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x03, (INT_T)_T("CLEANING CARTRIDGE INSTALLED")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x04, (INT_T)_T("CANNOT WRITE MEDIUM - UNKNOWN FORMAT")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x05, (INT_T)_T("CANNOT WRITE MEDIUM - INCOMPATIBLE FORMAT")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x06, (INT_T)_T("CANNOT FORMAT MEDIUM - INCOMPATIBLE MEDIUM")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x07, (INT_T)_T("CLEANING FAILURE")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x08, (INT_T)_T("CANNOT WRITE - APPLICATION CODE MISMATCH")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x09, (INT_T)_T("CURRENT SESSION NOT FIXATED FOR APPEND")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x0A, (INT_T)_T("CLEANING REQUEST REJECTED")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x10, (INT_T)_T("MEDIUM NOT FORMATTED")}, 
		{0x31, 0x00, (INT_T)_T("MEDIUM FORMAT CORRUPTED")}, 
		{0x31, 0x01, (INT_T)_T("FORMAT COMMAND FAILED")}, 
		{0x31, 0x02, (INT_T)_T("ZONED FORMATTING FAILED DUE TO SPARE LINKING")}, 
		{0x34, 0x00, (INT_T)_T("ENCLOSURE FAILURE")}, 
		{0x35, 0x00, (INT_T)_T("ENCLOSURE SERVICES FAILURE")}, 
		{0x35, 0x01, (INT_T)_T("UNSUPPORTED ENCLOSURE FUNCTION")}, 
		{0x35, 0x02, (INT_T)_T("ENCLOSURE SERVICES UNAVAILABLE")}, 
		{0x35, 0x03, (INT_T)_T("ENCLOSURE SERVICES TRANSFER FAILURE")}, 
		{0x35, 0x04, (INT_T)_T("ENCLOSURE SERVICES TRANSFER REFUSED")}, 
		{0x35, 0x05, (INT_T)_T("ENCLOSURE SERVICES CHECKSUM ERROR")}, 
		{0x37, 0x00, (INT_T)_T("ROUNDED PARAMETER")}, 
		{0x39, 0x00, (INT_T)_T("SAVING PARAMETERS NOT SUPPORTED")}, 
		{SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0x00, (INT_T)_T("MEDIUM NOT PRESENT")}, 
		{SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0x01, (INT_T)_T("MEDIUM NOT PRESENT - TRAY CLOSED")}, 
		{SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0x02, (INT_T)_T("MEDIUM NOT PRESENT - TRAY OPEN")}, 
		{SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0x03, (INT_T)_T("MEDIUM NOT PRESENT - LOADABLE")}, 
		{SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0x04, (INT_T)_T("MEDIUM NOT PRESENT - MEDIUM AUXILIARY MEMORY ACCESSIBLE")}, 
		{SCSI_ADSENSE_POSITION_ERROR, SCSI_SENSEQ_DESTINATION_FULL, (INT_T)_T("MEDIUM DESTINATION ELEMENT FULL")}, 
		{SCSI_ADSENSE_POSITION_ERROR, SCSI_SENSEQ_SOURCE_EMPTY, (INT_T)_T("MEDIUM SOURCE ELEMENT EMPTY")}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x0F, (INT_T)_T("END OF MEDIUM REACHED")}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x11, (INT_T)_T("MEDIUM MAGAZINE NOT ACCESSIBLE")}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x12, (INT_T)_T("MEDIUM MAGAZINE REMOVED")}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x13, (INT_T)_T("MEDIUM MAGAZINE INSERTED")}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x14, (INT_T)_T("MEDIUM MAGAZINE LOCKED")}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x15, (INT_T)_T("MEDIUM MAGAZINE UNLOCKED")}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x16, (INT_T)_T("MECHANICAL POSITIONING OR CHANGER ERROR")}, 
		{0x3D, 0x00, (INT_T)_T("INVALID BITS IN IDENTIFY MESSAGE")}, 
		{0x3E, 0x00, (INT_T)_T("LOGICAL UNIT HAS NOT SELF-CONFIGURED YET")}, 
		{0x3E, 0x01, (INT_T)_T("LOGICAL UNIT FAILURE")}, 
		{0x3E, 0x02, (INT_T)_T("TIMEOUT ON LOGICAL UNIT")}, 
		{0x3E, 0x03, (INT_T)_T("LOGICAL UNIT FAILED SELF-TEST")}, 
		{0x3E, 0x04, (INT_T)_T("LOGICAL UNIT UNABLE TO UPDATE SELF-TEST LOG")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_TARGET_OPERATING_CONDITIONS_CHANGED, (INT_T)_T("TARGET OPERATING CONDITIONS HAVE CHANGED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_MICROCODE_CHANGED, (INT_T)_T("MICROCODE HAS BEEN CHANGED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_OPERATING_DEFINITION_CHANGED, (INT_T)_T("CHANGED OPERATING DEFINITION")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_INQUIRY_DATA_CHANGED, (INT_T)_T("INQUIRY DATA HAS CHANGED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_COMPONENT_DEVICE_ATTACHED, (INT_T)_T("COMPONENT DEVICE ATTACHED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_DEVICE_IDENTIFIER_CHANGED, (INT_T)_T("DEVICE IDENTIFIER CHANGED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_REDUNDANCY_GROUP_MODIFIED, (INT_T)_T("REDUNDANCY GROUP CREATED OR MODIFIED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_REDUNDANCY_GROUP_DELETED, (INT_T)_T("REDUNDANCY GROUP DELETED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_SPARE_MODIFIED, (INT_T)_T("SPARE CREATED OR MODIFIED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_SPARE_DELETED, (INT_T)_T("SPARE DELETED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_VOLUME_SET_MODIFIED, (INT_T)_T("VOLUME SET CREATED OR MODIFIED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_VOLUME_SET_DELETED, (INT_T)_T("VOLUME SET DELETED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_VOLUME_SET_DEASSIGNED, (INT_T)_T("VOLUME SET DEASSIGNED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_VOLUME_SET_REASSIGNED, (INT_T)_T("VOLUME SET REASSIGNED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_REPORTED_LUNS_DATA_CHANGED, (INT_T)_T("REPORTED LUNS DATA HAS CHANGED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_ECHO_BUFFER_OVERWRITTEN, (INT_T)_T("ECHO pBuf OVERWRITTEN")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_MEDIUM_LOADABLE, (INT_T)_T("MEDIUM LOADABLE")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_MEDIUM_AUXILIARY_MEMORY_ACCESSIBLE, (INT_T)_T("MEDIUM AUXILIARY MEMORY ACCESSIBLE")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, 0x12, (INT_T)_T("iSCSI IP ADDRESS ADDED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, 0x13, (INT_T)_T("iSCSI IP ADDRESS REMOVED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, 0x14, (INT_T)_T("iSCSI IP ADDRESS CHANGED")}, 
		{0x43, 0x00, (INT_T)_T("MESSAGE ERROR")}, 
		{0x44, 0x00, (INT_T)_T("INTERNAL TARGET FAILURE")}, 
		{0x45, 0x00, (INT_T)_T("SELECT OR RESELECT FAILURE")}, 
		{0x46, 0x00, (INT_T)_T("UNSUCCESSFUL SOFT RESET")}, 
		{0x47, 0x00, (INT_T)_T("SCSI PARITY ERROR")}, 
		{0x47, 0x01, (INT_T)_T("DATA PHASE CRC ERROR DETECTED")}, 
		{0x47, 0x02, (INT_T)_T("SCSI PARITY ERROR DETECTED DURING ST DATA PHASE")}, 
		{0x47, 0x03, (INT_T)_T("INFORMATION UNIT iuCRC ERROR DETECTED")}, 
		{0x47, 0x04, (INT_T)_T("ASYNCHRONOUS INFORMATION PROTECTION ERROR DETECTED")}, 
		{0x47, 0x05, (INT_T)_T("PROTOCOL SERVICE CRC ERROR")}, 
		{0x47, 0x7F, (INT_T)_T("SOME COMMANDS CLEARED BY ISCSI PROTOCOL EVENT")}, 
		{0x48, 0x00, (INT_T)_T("INITIATOR DETECTED ERROR MESSAGE RECEIVED")}, 
		{0x49, 0x00, (INT_T)_T("INVALID MESSAGE ERROR")}, 
		{0x4A, 0x00, (INT_T)_T("COMMAND PHASE ERROR")}, 
		{0x4B, 0x00, (INT_T)_T("DATA PHASE ERROR")}, 
		{0x4B, 0x01, (INT_T)_T("INVALID TARGET PORT TRANSFER TAG RECEIVED")}, 
		{0x4B, 0x02, (INT_T)_T("TOO MUCH WRITE DATA")}, 
		{0x4B, 0x03, (INT_T)_T("ACK/NAK TIMEOUT")}, 
		{0x4B, 0x04, (INT_T)_T("NAK RECEIVED")}, 
		{0x4B, 0x05, (INT_T)_T("DATA OFFSET ERROR")}, 
		{0x4B, 0x06, (INT_T)_T("INITIATOR RESPONSE TIMEOUT")}, 
		{0x4B, 0x07, (INT_T)_T("CONNECTION LOST")}, 
		{0x4C, 0x00, (INT_T)_T("LOGICAL UNIT FAILED SELF-CONFIGURATION")}, 
		{0x4E, 0x00, (INT_T)_T("OVERLAPPED COMMANDS ATTEMPTED")}, 
		{0x51, 0x00, (INT_T)_T("ERASE FAILURE")}, 
		{0x51, 0x01, (INT_T)_T("ERASE FAILURE - INCOMPLETE ERASE OPERATION DETECTED")}, 
		{0x53, 0x00, (INT_T)_T("MEDIA LOAD OR EJECT FAILED")}, 
		{0x53, 0x02, (INT_T)_T("MEDIUM REMOVAL PREVENTED")}, 
		{0x55, 0x02, (INT_T)_T("INSUFFICIENT RESERVATION RESOURCES")}, 
		{0x55, 0x03, (INT_T)_T("INSUFFICIENT RESOURCES")}, 
		{0x55, 0x04, (INT_T)_T("INSUFFICIENT REGISTRATION RESOURCES")}, 
		{0x55, 0x05, (INT_T)_T("INSUFFICIENT ACCESS CONTROL RESOURCES")}, 
		{0x55, 0x06, (INT_T)_T("AUXILIARY MEMORY OUT OF SPACE")}, 
		{0x55, 0x0B, (INT_T)_T("INSUFFICIENT POWER FOR OPERATION")}, 
		{0x57, 0x00, (INT_T)_T("UNABLE TO RECOVER TABLE-OF-CONTENTS")}, 
		{SCSI_ADSENSE_OPERATOR_REQUEST, SCSI_SENSEQ_STATE_CHANGE_INPUT, (INT_T)_T("OPERATOR REQUEST OR STATE CHANGE INPUT")}, 
		{SCSI_ADSENSE_OPERATOR_REQUEST, SCSI_SENSEQ_MEDIUM_REMOVAL, (INT_T)_T("OPERATOR MEDIUM REMOVAL REQUEST")}, 
		{SCSI_ADSENSE_OPERATOR_REQUEST, SCSI_SENSEQ_WRITE_PROTECT_ENABLE, (INT_T)_T("OPERATOR SELECTED WRITE PROTECT")}, 
		{SCSI_ADSENSE_OPERATOR_REQUEST, SCSI_SENSEQ_WRITE_PROTECT_DISABLE, (INT_T)_T("OPERATOR SELECTED WRITE PERMIT")}, 
		{0x5b, 0x00, (INT_T)_T("LOG EXCEPTION")}, 
		{0x5b, 0x01, (INT_T)_T("THRESHOLD CONDITION MET")}, 
		{0x5b, 0x02, (INT_T)_T("LOG COUNTER AT MAXIMUM")}, 
		{0x5b, 0x03, (INT_T)_T("LOG LIST CODES EXHAUSTED")}, 
		{SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED, 0x00, (INT_T)_T("FAILURE PREDICTION THRESHOLD EXCEEDED")}, 
		{SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED, 0x01, (INT_T)_T("MEDIA FAILURE PREDICTION THRESHOLD EXCEEDED")}, 
		{SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED, 0x02, (INT_T)_T("LOGICAL UNIT FAILURE PREDICTION THRESHOLD EXCEEDED")}, 
		{SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED, 0x03, (INT_T)_T("SPARE AREA EXHAUSTION PREDICTION THRESHOLD EXCEEDED")}, 
		{SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED, 0xFF, (INT_T)_T("FAILURE PREDICTION THRESHOLD EXCEEDED (FALSE)")}, 
		{0x5E, 0x00, (INT_T)_T("LOW POWER CONDITION ON")}, 
		{0x5E, 0x01, (INT_T)_T("IDLE CONDITION ACTIVATED BY TIMER")}, 
		{0x5E, 0x02, (INT_T)_T("STANDBY CONDITION ACTIVATED BY TIMER")}, 
		{0x5E, 0x03, (INT_T)_T("IDLE CONDITION ACTIVATED BY COMMAND")}, 
		{0x5E, 0x04, (INT_T)_T("STANDBY CONDITION ACTIVATED BY COMMAND")}, 
		{0x5E, 0x05, (INT_T)_T("IDLE_B CONDITION ACTIVATED BY TIMER")}, 
		{0x5E, 0x06, (INT_T)_T("IDLE_B CONDITION ACTIVATED BY COMMAND")}, 
		{0x5E, 0x07, (INT_T)_T("IDLE_C CONDITION ACTIVATED BY TIMER")}, 
		{0x5E, 0x08, (INT_T)_T("IDLE_C CONDITION ACTIVATED BY COMMAND")}, 
		{0x5E, 0x09, (INT_T)_T("STANDBY_Y CONDITION ACTIVATED BY TIMER")}, 
		{0x5E, 0x0A, (INT_T)_T("STANDBY_Y CONDITION ACTIVATED BY COMMAND")}, 
		{0x63, 0x00, (INT_T)_T("END OF USER AREA ENCOUNTERED ON THIS TRACK")}, 
		{0x63, 0x01, (INT_T)_T("PACKET DOES NOT FIT IN AVAILABLE SPACE")}, 
		{SCSI_ADSENSE_ILLEGAL_MODE_FOR_THIS_TRACK, 0x00, (INT_T)_T("ILLEGAL MODE FOR THIS TRACK")}, 
		{SCSI_ADSENSE_ILLEGAL_MODE_FOR_THIS_TRACK, 0x01, (INT_T)_T("INVALID PACKET SIZE")}, 
		{0x65, 0x00, (INT_T)_T("VOLTAGE FAULT")}, 
		{0x67, 0x0A, (INT_T)_T("SET TARGET PORT GROUPS COMMAND FAILED")}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_AUTHENTICATION_FAILURE, (INT_T)_T("COPY PROTECTION KEY EXCHANGE FAILURE - AUTHENTICATION FAILURE")}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_KEY_NOT_PRESENT, (INT_T)_T("COPY PROTECTION KEY EXCHANGE FAILURE - KEY NOT PRESENT")}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_KEY_NOT_ESTABLISHED, (INT_T)_T("COPY PROTECTION KEY EXCHANGE FAILURE - KEY NOT ESTABLISHED")}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_READ_OF_SCRAMBLED_SECTOR_WITHOUT_AUTHENTICATION, (INT_T)_T("READ OF SCRAMBLED SECTOR WITHOUT AUTHENTICATION")}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_MEDIA_CODE_MISMATCHED_TO_LOGICAL_UNIT, (INT_T)_T("MEDIA REGION CODE IS MISMATCHED TO LOGICAL UNIT REGION")}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_LOGICAL_UNIT_RESET_COUNT_ERROR, (INT_T)_T("DRIVE REGION MUST BE PERMANENT/REGION RESET COUNT ERROR")}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, 0x06, (INT_T)_T("INSUFFICIENT BLOCK COUNT FOR BINDING NONCE RECORDING")}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, 0x07, (INT_T)_T("CONFLICT IN BINDING NONCE RECORDING")}, 
		{0x72, 0x00, (INT_T)_T("SESSION FIXATION ERROR")}, 
		{0x72, 0x01, (INT_T)_T("SESSION FIXATION ERROR WRITING LEAD-IN")}, 
		{0x72, 0x02, (INT_T)_T("SESSION FIXATION ERROR WRITING LEAD-OUT")}, 
		{0x72, 0x03, (INT_T)_T("SESSION FIXATION ERROR - INCOMPLETE TRACK IN SESSION")}, 
		{0x72, 0x04, (INT_T)_T("EMPTY OR PARTIALLY WRITTEN RESERVED TRACK")}, 
		{0x72, 0x05, (INT_T)_T("NO MORE TRACK RESERVATIONS ALLOWED")}, 
		{0x72, 0x06, (INT_T)_T("RMZ EXTENSION IS NOT ALLOWED")}, 
		{0x72, 0x07, (INT_T)_T("NO MORE TEST ZONE EXTENSIONS ARE ALLOWED")}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, 0x00, (INT_T)_T("VOLTAGE FAULT")}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_POWER_CALIBRATION_AREA_ALMOST_FULL, (INT_T)_T("CD CONTROL ERROR")}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_POWER_CALIBRATION_AREA_FULL, (INT_T)_T("POWER CALIBRATION AREA ALMOST FULL")}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_POWER_CALIBRATION_AREA_ERROR, (INT_T)_T("POWER CALIBRATION AREA IS FULL")}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_PMA_RMA_UPDATE_FAILURE, (INT_T)_T("POWER CALIBRATION AREA ERROR")}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_PMA_RMA_IS_FULL, (INT_T)_T("PROGRAM MEMORY AREA UPDATE FAILURE")}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_PMA_RMA_ALMOST_FULL, (INT_T)_T("RMA/PMA IS ALMOST FULL")}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, 0x10, (INT_T)_T("CURRENT POWER CALIBRATION AREA ALMOST FULL")}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, 0x11, (INT_T)_T("CURRENT POWER CALIBRATION AREA IS FULL")}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, 0x17, (INT_T)_T("RDZ IS FULL")}, 
		{0x74, 0x08, (INT_T)_T("DIGITAL SIGNATURE VALIDATION FAILURE")}, 
		{0x74, 0x0C, (INT_T)_T("UNABLE TO DECRYPT PARAMETER LIST")}, 
		{0x74, 0x10, (INT_T)_T("SA CREATION PARAMETER VALUE INVALID")}, 
		{0x74, 0x11, (INT_T)_T("SA CREATION PARAMETER VALUE REJECTED")}, 
		{0x74, 0x12, (INT_T)_T("INVALID SA USAGE")}, 
		{0x74, 0x30, (INT_T)_T("SA CREATION PARAMETER NOT SUPPORTED")}, 
		{0x74, 0x40, (INT_T)_T("AUTHENTICATION FAILED")}, 
		{0x74, 0x71, (INT_T)_T("LOGICAL UNIT ACCESS NOT AUTHORIZED")} 
	};
	OutputErrorString(_T("Sense data, Key:Asc:Ascq: %02X:%02X:%02X"), byKey, byAsc, byAscq);

	for(INT i = 0; i < sizeof(aSenseKey) / sizeof(INT) / 2; i++) {
		if(byKey == aSenseKey[i][0]) {
			OutputErrorString(_T("(%s."), (_TCHAR*)aSenseKey[i][1]);
			break;
		}
	}
	BOOL bRet = FALSE;
	for(INT i = 0; i < sizeof(aSenseAscAscq) / sizeof(INT) / 3 - 1; i++) {
		if(byAsc == aSenseAscAscq[i][0]) {
			do {
				if(byAscq == aSenseAscAscq[i][1]) {
					OutputErrorString(_T(" %s)"), (_TCHAR*)aSenseAscAscq[i][2]);
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
		OutputErrorString(_T(" VENDER UNIQUE ERROR)"));
	}
	OutputErrorString(_T("\n"));
}

void OutputSub96(
	CONST PUCHAR pBuf,
	INT nLBA,
	FILE* fpLog
	)
{
	OutputLogString(fpLog, _T("Sub Channel LBA %d\n"), nLBA);
	OutputLogString(fpLog, _T("\t  +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B\n"));

	for(INT i = 0, ch = 0x50; i < CD_RAW_READ_SUBCODE_SIZE; i += 12, ch++) {
		OutputLogString(fpLog, 
			_T("\t%c %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n"), 
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
	_TCHAR str[256];
	ZeroMemory(str, sizeof(str));
	_stprintf(str, _T("LBA[%06d, 0x%05X], "), nLBA, nLBA);
	// Ctl
	switch((Subcode[12] >> 4) & 0x0F) {
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
	_TCHAR str2[256];
	ZeroMemory(str2, sizeof(str2));
	switch(Subcode[12] & 0x0F) {
	case ADR_ENCODES_CURRENT_POSITION:
		if(Subcode[13] == 0xAA) {
			_stprintf(str2, 
				_T("TOC[LeadOut    , Index-%02x, RelativeTime-%02x:%02x:%02x, AbsoluteTime-%02x:%02x:%02x] "), 
				Subcode[14], Subcode[15], Subcode[16], Subcode[17], 
				Subcode[19], Subcode[20], Subcode[21]);
		}
		else {
			_stprintf(str2, 
				_T("TOC[TrackNum-%02x, Index-%02x, RelativeTime-%02x:%02x:%02x, AbsoluteTime-%02x:%02x:%02x] "), 
				Subcode[13], Subcode[14], Subcode[15], Subcode[16], 
				Subcode[17], Subcode[19], Subcode[20], Subcode[21]);
		}
		break;
	case ADR_ENCODES_MEDIA_CATALOG:
		_stprintf(str2, 
			_T("Media Catalog Number (MCN)[%s        , AbsoluteTime-     :%02x] "), 
			szCatalog, Subcode[21]);
		break;
	case ADR_ENCODES_ISRC:
		_stprintf(str2, 
			_T("International Standard... (ISRC)[%s   , AbsoluteTime-     :%02x] "), 
			szISRC[nTrackNum-1], Subcode[21]);
		break;
	default:
		_stprintf(str2, 
			_T("TOC[TrackNum-%02x, Index-%02x, RelativeTime-%02x:%02x:%02x, AbsoluteTime-%02x:%02x:%02x] "), 
			Subcode[13], Subcode[14], Subcode[15], Subcode[16], 
			Subcode[17], Subcode[19], Subcode[20], Subcode[21]);
		break;
	}
	_tcscat(str, str2);

//	for(INT a = 0; a < CD_RAW_READ_SUBCODE_SIZE; a += 24) {
		switch(SubcodeRtoW[0] & 0x3F) {
		case 0:
			_tcscat(str, _T("RtoW:ZERO mode\n"));
			break;
		case 8:
			_tcscat(str, _T("RtoW:LINE-GRAPHICS mode\n"));
			break;
		case 9:
			_tcscat(str, _T("RtoW:TV-GRAPHICS mode\n"));
			break;
		case 10:
			_tcscat(str, _T("RtoW:EXTENDED-TV-GRAPHICS mode\n"));
			break;
		case 20:
			_tcscat(str, _T("RtoW:CD TEXT mode\n"));
			break;
		case 24:
			_tcscat(str, _T("RtoW:MIDI mode\n"));
			break;
		case 56:
			_tcscat(str, _T("RtoW:USER mode\n"));
			break;
		default:
			_tcscat(str, _T("\n"));
			break;
		}
//	}
	fwrite(str, sizeof(_TCHAR), _tcslen(str), fpParse);
}

void OutputTocFull(
	CONST CDROM_TOC_FULL_TOC_DATA* fullToc,
	CONST CDROM_TOC_FULL_TOC_DATA_BLOCK* pTocData,
	size_t uiTocEntries,
	INT* nLastLBAof1stSession,
	INT* nStartLBAof2ndSession,
	INT* aSessionNum,
	FILE* fpCcd,
	FILE* fpLog
	)
{
	OutputLogString(fpLog, _T("FULL TOC on SCSIOP_READ_TOC\n"));
	OutputLogString(fpLog, _T("\tFirstCompleteSession %d\n"), 
		fullToc->FirstCompleteSession);
	OutputLogString(fpLog, _T("\tLastCompleteSession %d\n"), 
		fullToc->LastCompleteSession);

	for(size_t a = 0; a < uiTocEntries; a++) {
		WriteCcdFileForEntry(a, pTocData, fpCcd);
		switch(pTocData[a].Point) {
		case 0xA0:
			OutputLogString(fpLog, _T("\tSession %d, FirstTrack %d\n"), 
				pTocData[a].SessionNumber, pTocData[a].Msf[0]);
			break;
		case 0xA1:
			OutputLogString(fpLog, _T("\tSession %d, LastTrack %d\n"), 
				pTocData[a].SessionNumber, pTocData[a].Msf[0]);
			break;
		case 0xA2:
			OutputLogString(fpLog, 
				_T("\tSession %d, Leadout MSF %02d:%02d:%02d\n"), 
				pTocData[a].SessionNumber, pTocData[a].Msf[0], 
				pTocData[a].Msf[1], pTocData[a].Msf[2]);
			if(pTocData[a].SessionNumber == 1) {
				*nLastLBAof1stSession = 
					MSFtoLBA(pTocData[a].Msf[2], pTocData[a].Msf[1], pTocData[a].Msf[0]) - 150;
			}
			break;
		case 0xB0:
			OutputLogString(fpLog, 
				_T("\tSession %d, NextSession MSF %02d:%02d:%02d, LastWritable MSF %02d:%02d:%02d\n"), 
				pTocData[a].SessionNumber, pTocData[a].MsfExtra[0], 
				pTocData[a].MsfExtra[1], pTocData[a].MsfExtra[2], 
				pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
			break;
		case 0xC0:
			OutputLogString(fpLog, 
				_T("\tSession %d, WriteLaserOutput %02d, FirstLeadin MSF %02d:%02d:%02d\n"), 
				pTocData[a].SessionNumber, pTocData[a].MsfExtra[0],	
				pTocData[a].Msf[0], pTocData[a].Msf[1], pTocData[a].Msf[2]);
			break;
		default:
			OutputLogString(fpLog, 
				_T("\tSession %d, Track %2d, MSF %02d:%02d:%02d\n"), 
				pTocData[a].SessionNumber, pTocData[a].Point, pTocData[a].Msf[0], 
				pTocData[a].Msf[1], pTocData[a].Msf[2]);
			if(pTocData[a].SessionNumber == 2) {
				*nStartLBAof2ndSession = MSFtoLBA(pTocData[a].Msf[2], 
					pTocData[a].Msf[1], pTocData[a].Msf[0]) - 150;
			}
			aSessionNum[pTocData[a].Point-1] = pTocData[a].SessionNumber;
			break;
		}
	}
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

// begin for CD
void OutputVolumeDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	OutputLogString(fpLog, _T("Volume Descriptor\n"));
	// 0 is Boot Record. 
	// 1 is Primary Volume Descriptor. 
	// 2 is Supplementary Volume Descriptor. 
	// 3 is Volume Partition Descriptor. 
	// 4-254 is reserved. 
	// 255 is Volume Descriptor Set Terminator.
	OutputLogString(fpLog, _T("\tVolume Descriptor Type		%d\n"), buf[idx]);
	_TCHAR str[CD_RAW_READ+1];
	ZeroMemory(str, sizeof(str));
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+1], 5, str, sizeof(str));
#else
	strncpy(str, (CHAR*)&buf[idx+1], 5); str[5] = '\0';
#endif
	OutputLogString(fpLog, _T("\tStandard Identifier			[%s]\n"), str);
	OutputLogString(fpLog, _T("\tVolume Descriptor Version	%d\n"), buf[idx+6]);
	
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
	_TCHAR str[2][32+1];
	ZeroMemory(str, sizeof(str));
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+7], 32, str[0], sizeof(str));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+39], 32, str[1], sizeof(str));
#else
	strncpy(str[0], (CHAR*)&buf[idx+7], 32); str[0][32] = '\0';
	strncpy(str[1], (CHAR*)&buf[idx+39], 32); str[1][32] = '\0';
#endif
	OutputLogString(fpLog, _T("\tBoot System Identifier		[%s]\n"), str);
	OutputLogString(fpLog, _T("\tBoot Identifier				[%s]\n"), str);
	OutputLogString(fpLog, _T("\tBoot System Use	"));
	for(INT i = 71; i <= 2047; i++) {
		OutputLogString(fpLog, _T("%x"), buf[idx+i]);
	}
	OutputLogString(fpLog, _T("\n"));
#ifdef _DEBUG
	UNREFERENCED_PARAMETER(fpLog);
#endif
}

void OutputPrimaryVolumeDescriptorForTime(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	_TCHAR year[4][4+1], month[4][2+1], day[4][2+1], hour[4][2+1], time[4][2+1], second[4][2+1], milisecond[4][2+1];
	ZeroMemory(year, sizeof(year));
	ZeroMemory(month, sizeof(month));
	ZeroMemory(day, sizeof(day));
	ZeroMemory(hour, sizeof(hour));
	ZeroMemory(time, sizeof(time));
	ZeroMemory(second, sizeof(second));
	ZeroMemory(milisecond, sizeof(milisecond));
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+813], 4, year[0], sizeof(year[0]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+817], 2, month[0], sizeof(month[0]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+819], 2, day[0], sizeof(day[0]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+821], 2, hour[0], sizeof(hour[0]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+823], 2, time[0], sizeof(time[0]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+825], 2, second[0], sizeof(second[0]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+827], 2, milisecond[0], sizeof(milisecond[0]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+830], 4, year[1], sizeof(year[1]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+834], 2, month[1], sizeof(month[1]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+836], 2, day[1], sizeof(day[1]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+838], 2, hour[1], sizeof(hour[1]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+840], 2, time[1], sizeof(time[1]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+842], 2, second[1], sizeof(second[1]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+844], 2, milisecond[1], sizeof(milisecond[1]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+847], 4, year[2], sizeof(year[2]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+851], 2, month[2], sizeof(month[2]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+853], 2, day[2], sizeof(day[2]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+855], 2, hour[2], sizeof(hour[2]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+857], 2, time[2], sizeof(time[2]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+859], 2, second[2], sizeof(second[2]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+861], 2, milisecond[2], sizeof(milisecond[2]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+864], 4, year[3], sizeof(year[3]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+868], 2, month[3], sizeof(month[3]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+870], 2, day[3], sizeof(day[3]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+872], 2, hour[3], sizeof(hour[3]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+874], 2, time[3], sizeof(time[3]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+876], 2, second[3], sizeof(second[3]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+878], 2, milisecond[3], sizeof(milisecond[3]));
#else
	strncpy(year[0], (CHAR*)&buf[idx+813], 4); year[0][4] = '\0';
	strncpy(month[0], (CHAR*)&buf[idx+817], 2); month[0][2] = '\0';
	strncpy(day[0], (CHAR*)&buf[idx+819], 2); day[0][2] = '\0';
	strncpy(hour[0], (CHAR*)&buf[idx+821], 2); hour[0][2] = '\0';
	strncpy(time[0], (CHAR*)&buf[idx+823], 2); time[0][2] = '\0';
	strncpy(second[0], (CHAR*)&buf[idx+825], 2); second[0][2] = '\0';
	strncpy(milisecond[0], (CHAR*)&buf[idx+827], 2); milisecond[0][2] = '\0';
	strncpy(year[1], (CHAR*)&buf[idx+830], 4); year[1][4] = '\0';
	strncpy(month[1], (CHAR*)&buf[idx+834], 2); month[1][2] = '\0';
	strncpy(day[1], (CHAR*)&buf[idx+836], 2); day[1][2] = '\0';
	strncpy(hour[1], (CHAR*)&buf[idx+838], 2); hour[1][2] = '\0';
	strncpy(time[1], (CHAR*)&buf[idx+840], 2); time[1][2] = '\0';
	strncpy(second[1], (CHAR*)&buf[idx+842], 2); second[1][2] = '\0';
	strncpy(milisecond[1], (CHAR*)&buf[idx+844], 2); milisecond[1][2] = '\0';
	strncpy(year[2], (CHAR*)&buf[idx+847], 4); year[2][4] = '\0';
	strncpy(month[2], (CHAR*)&buf[idx+851], 2); month[2][2] = '\0';
	strncpy(day[2], (CHAR*)&buf[idx+853], 2); day[2][2] = '\0';
	strncpy(hour[2], (CHAR*)&buf[idx+855], 2); hour[2][2] = '\0';
	strncpy(time[2], (CHAR*)&buf[idx+857], 2); time[2][2] = '\0';
	strncpy(second[2], (CHAR*)&buf[idx+859], 2); second[2][2] = '\0';
	strncpy(milisecond[2], (CHAR*)&buf[idx+861], 2); milisecond[2][2] = '\0';
	strncpy(year[3], (CHAR*)&buf[idx+864], 4); year[3][4] = '\0';
	strncpy(month[3], (CHAR*)&buf[idx+868], 2); month[3][2] = '\0';
	strncpy(day[3], (CHAR*)&buf[idx+870], 2); day[3][2] = '\0';
	strncpy(hour[3], (CHAR*)&buf[idx+872], 2); hour[3][2] = '\0';
	strncpy(time[3], (CHAR*)&buf[idx+874], 2); time[3][2] = '\0';
	strncpy(second[3], (CHAR*)&buf[idx+876], 2); second[3][2] = '\0';
	strncpy(milisecond[3], (CHAR*)&buf[idx+878], 2); milisecond[3][2] = '\0';
#endif
	OutputLogString(fpLog, 
		_T("\tVolume Creation Date and Time		%s-%s-%s %s:%s:%s.%s +%d\n"), 
		year[0], month[0], day[0], hour[0], time[0], second[0], milisecond[0], buf[idx+829]);
	OutputLogString(fpLog, 
		_T("\tVolume Modification Date and Time	%s-%s-%s %s:%s:%s.%s +%d\n"), 
		year[1], month[1], day[1], hour[1], time[1], second[1], milisecond[1], buf[idx+846]);
	OutputLogString(fpLog, 
		_T("\tVolume Expiration Date and Time		%s-%s-%s %s:%s:%s.%s +%d\n"), 
		year[2], month[2], day[2], hour[2], time[2], second[2], milisecond[2], buf[idx+863]);
	OutputLogString(fpLog, 
		_T("\tVolume Effective Date and Time		%s-%s-%s %s:%s:%s.%s +%d\n"), 
		year[3], month[3], day[3], hour[3], time[3], second[3], milisecond[3], buf[idx+880]);
	OutputLogString(fpLog, 
		_T("\tFile Structure Version				%d\n"), buf[idx+881]);
	OutputLogString(fpLog, _T("\tApplication Use	"));
	for(INT i = 883; i <= 1394; i++) {
		OutputLogString(fpLog, _T("%x"), buf[idx+i]);
	}
	OutputLogString(fpLog, _T("\n"));
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
	_TCHAR str[10][CD_RAW_READ+1];
	ZeroMemory(str, sizeof(str));
	if(buf[idx] == 2) {
		OutputLogString(fpLog, 
			_T("\tVolume Flags					%d\n"), buf[idx+7]);
	}
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+8], 32, str[0], sizeof(str[0]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+40], 32, str[1], sizeof(str[1]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+190], 128, str[2], sizeof(str[2]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+318], 128, str[3], sizeof(str[3]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+446], 128, str[4], sizeof(str[4]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+574], 128, str[5], sizeof(str[5]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+702], 37, str[6], sizeof(str[6]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+739], 37, str[7], sizeof(str[7]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+776], 37, str[8], sizeof(str[8]));
#else
	strncpy(str[0], (CHAR*)&buf[idx+8], 32); str[0][32] = '\0';
	strncpy(str[1], (CHAR*)&buf[idx+40], 32); str[1][32] = '\0';
	strncpy(str[2], (CHAR*)&buf[idx+190], 128); str[2][128] = '\0';
	strncpy(str[3], (CHAR*)&buf[idx+318], 128); str[3][128] = '\0';
	strncpy(str[4], (CHAR*)&buf[idx+446], 128); str[4][128] = '\0';
	strncpy(str[5], (CHAR*)&buf[idx+574], 128); str[5][128] = '\0';
	strncpy(str[6], (CHAR*)&buf[idx+702], 37); str[6][37] = '\0';
	strncpy(str[7], (CHAR*)&buf[idx+739], 37); str[7][37] = '\0';
	strncpy(str[8], (CHAR*)&buf[idx+776], 37); str[8][37] = '\0';
#endif
	OutputLogString(fpLog, _T("\tSystem Identifier		[%s]\n"), str[0]);
	OutputLogString(fpLog, _T("\tVolume Identifier		[%s]\n"), str[1]);
	OutputLogString(fpLog, _T("\tVolume Space Size		%d\n"), buf[idx+80]);
	if(buf[idx+0] == 2) {
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+88], 32, str[9], sizeof(str[9]));
#else
		strncpy(str[9], (CHAR*)&buf[idx+88], 32); str[9][32] = '\0';
#endif
		OutputLogString(fpLog, _T("\tEscape Sequences		[%s]\n"), str[9]);
	}
	OutputLogString(fpLog, _T("\tVolume Set Size			%d\n"), 
		MAKEWORD(buf[idx+123], buf[idx+122]));
	OutputLogString(fpLog, _T("\tVolume Sequence Number	%d\n"), 
		MAKEWORD(buf[idx+127], buf[idx+126]));
	OutputLogString(fpLog, _T("\tLogical Block Size		%d\n"), 
		MAKEWORD(buf[idx+131], buf[idx+130]));
	OutputLogString(fpLog, _T("\tPath Table Size			%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+139], buf[idx+138]), 
		MAKEWORD(buf[idx+137], buf[idx+136])));
	OutputLogString(fpLog, 
		_T("\tLocation of Occurrence of Type L Path Table				%d\n"), 
		MAKEWORD(buf[idx+143], buf[idx+142]));
	OutputLogString(fpLog, 
		_T("\tLocation of Optional Occurrence of Type L Path Table	%d\n"), 
		MAKEWORD(buf[idx+147], buf[idx+146]));
	OutputLogString(fpLog, 
		_T("\tLocation of Occurrence of Type M Path Table				%d\n"), 
		MAKEWORD(buf[idx+151], buf[idx+150]));
	OutputLogString(fpLog, 
		_T("\tLocation of Optional Occurrence of Type M Path Table	%d\n"), 
		MAKEWORD(buf[idx+155], buf[idx+154]));

	OutputLogString(fpLog, _T("\tDirectory Record\n"));
	OutputLogString(fpLog, 
		_T("\t\tLength of Directory Record			%d\n"), buf[idx+156]);
	OutputLogString(fpLog, 
		_T("\t\tExtended Attribute Record Length	%d\n"), buf[idx+157]);
	OutputLogString(fpLog, 
		_T("\t\tLocation of Extent					%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+165], buf[idx+164]), 
		MAKEWORD(buf[idx+163], buf[idx+162])));
	OutputLogString(fpLog, 
		_T("\t\tData Length							%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+173], buf[idx+172]), 
		MAKEWORD(buf[idx+171], buf[idx+170])));
	OutputLogString(fpLog, 
		_T("\t\tRecording Date and Time				%d-%02d-%02d %02d:%02d:%02d +%02d\n"), 
		buf[idx+174] + 1900, buf[idx+175], buf[idx+176], 
		buf[idx+177], buf[idx+178], buf[idx+179], buf[idx+180]);
	OutputLogString(fpLog, 
		_T("\t\tFile Flags							%d\n"), buf[idx+181]);
	OutputLogString(fpLog, 
		_T("\t\tFile Unit Size						%d\n"), buf[idx+182]);
	OutputLogString(fpLog, 
		_T("\t\tInterleave Gap Size					%d\n"), buf[idx+183]);
	OutputLogString(fpLog, 
		_T("\t\tVolume Sequence Number				%d\n"), 
		MAKEWORD(buf[idx+187], buf[idx+186]));
	OutputLogString(fpLog, 
		_T("\t\tLength of File Identifier			%d\n"), buf[idx+188]);
	OutputLogString(fpLog, 
		_T("\t\tFile Identifier						%d\n"), buf[idx+189]);
	OutputLogString(fpLog, _T("\tVolume Set Identifier		[%s]\n"), str[2]);
	OutputLogString(fpLog, _T("\tPublisher Identifier		[%s]\n"), str[3]);
	OutputLogString(fpLog, _T("\tData Preparer Identifier	[%s]\n"), str[4]);
	OutputLogString(fpLog, _T("\tApplication Identifier		[%s]\n"), str[5]);
	OutputLogString(fpLog, _T("\tCopyright File Identifier		[%s]\n"), str[6]);
	OutputLogString(fpLog, _T("\tAbstract File Identifier		[%s]\n"), str[7]);
	OutputLogString(fpLog, _T("\tBibliographic File Identifier	[%s]\n"), str[8]);

	OutputPrimaryVolumeDescriptorForTime(idx, buf, fpLog);
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
	OutputLogString(fpLog, _T("\tSystem Identifier		[%s]\n"), str);
	LittleToBig(str, (_TCHAR*)&buf[idx+40], 16); str[16] = '\0';
	OutputLogString(fpLog, _T("\tVolume Identifier		[%s]\n"), str);
	OutputLogString(fpLog, _T("\tVolume Space Size		%d\n"), buf[idx+80]);
	if(buf[idx+0] == 2) {
		_TCHAR str2[32+1];
		_tcsncpy(str2, (_TCHAR*)&buf[idx+88], 32); str2[32] = '\0';
		OutputLogString(fpLog, _T("\tEscape Sequences		[%s]\n"), str2);
	}
	OutputLogString(fpLog, _T("\tVolume Set Size			%d\n"), 
		MAKEWORD(buf[idx+123], buf[idx+122]));
	OutputLogString(fpLog, _T("\tVolume Sequence Number	%d\n"), 
		MAKEWORD(buf[idx+127], buf[idx+126]));
	OutputLogString(fpLog, _T("\tLogical Block Size		%d\n"), 
		MAKEWORD(buf[idx+131], buf[idx+130]));
	OutputLogString(fpLog, _T("\tPath Table Size			%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+139], buf[idx+138]), 
		MAKEWORD(buf[idx+137], buf[idx+136])));
	OutputLogString(fpLog, 
		_T("\tLocation of Occurrence of Type L Path Table				%d\n"), 
		MAKEWORD(buf[idx+143], buf[idx+142]));
	OutputLogString(fpLog, 
		_T("\tLocation of Optional Occurrence of Type L Path Table	%d\n"), 
		MAKEWORD(buf[idx+147], buf[idx+146]));
	OutputLogString(fpLog, 
		_T("\tLocation of Occurrence of Type M Path Table				%d\n"), 
		MAKEWORD(buf[idx+151], buf[idx+150]));
	OutputLogString(fpLog, 
		_T("\tLocation of Optional Occurrence of Type M Path Table	%d\n"), 
		MAKEWORD(buf[idx+155], buf[idx+154]));

	OutputLogString(fpLog, _T("\tDirectory Record\n"));
	OutputLogString(fpLog, 
		_T("\t\tLength of Directory Record			%d\n"), buf[idx+156]);
	OutputLogString(fpLog, 
		_T("\t\tExtended Attribute Record Length	%d\n"), buf[idx+157]);
	OutputLogString(fpLog, 
		_T("\t\tLocation of Extent					%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+165], buf[idx+164]), 
		MAKEWORD(buf[idx+163], buf[idx+162])));
	OutputLogString(fpLog, 
		_T("\t\tData Length							%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+173], buf[idx+172]), 
		MAKEWORD(buf[idx+171], buf[idx+170])));
	OutputLogString(fpLog, 
		_T("\t\tRecording Date and Time				%d-%02d-%02d %02d:%02d:%02d +%02d\n"), 
		buf[idx+174] + 1900, buf[idx+175], buf[idx+176], 
		buf[idx+177], buf[idx+178], buf[idx+179], buf[idx+180]);
	OutputLogString(fpLog, 
		_T("\t\tFile Flags							%d\n"), buf[idx+181]);
	OutputLogString(fpLog, 
		_T("\t\tFile Unit Size						%d\n"), buf[idx+182]);
	OutputLogString(fpLog, 
		_T("\t\tInterleave Gap Size					%d\n"), buf[idx+183]);
	OutputLogString(fpLog, 
		_T("\t\tVolume Sequence Number				%d\n"), 
		MAKEWORD(buf[idx+187], buf[idx+186]));
	OutputLogString(fpLog, 
		_T("\t\tLength of File Identifier			%d\n"), buf[idx+188]);
	OutputLogString(fpLog, 
		_T("\t\tFile Identifier						%d\n"), buf[idx+189]);
	LittleToBig(str, (_TCHAR*)&buf[idx+190], 64); str[64] = '\0';
	OutputLogString(fpLog, _T("\tVolume Set Identifier		[%s]\n"), str);
	LittleToBig(str, (_TCHAR*)&buf[idx+318], 64); str[64] = '\0';
	OutputLogString(fpLog, _T("\tPublisher Identifier		[%s]\n"), str);
	LittleToBig(str, (_TCHAR*)&buf[idx+446], 64); str[64] = '\0';
	OutputLogString(fpLog, _T("\tData Preparer Identifier	[%s]\n"), str);
	LittleToBig(str, (_TCHAR*)&buf[idx+574], 64); str[64] = '\0';
	OutputLogString(fpLog, _T("\tApplication Identifier		[%s]\n"), str);
	LittleToBig(str, (_TCHAR*)&buf[idx+702], 18); str[18] = '\0';
	OutputLogString(fpLog, _T("\tCopyright File Identifier		[%s]\n"), str);
	LittleToBig(str, (_TCHAR*)&buf[idx+702], 18); str[18] = '\0';
	OutputLogString(fpLog, _T("\tAbstract File Identifier		[%s]\n"), str);
	LittleToBig(str, (_TCHAR*)&buf[idx+776], 18); str[18] = '\0';
	OutputLogString(fpLog, _T("\tBibliographic File Identifier	[%s]\n"), str);

	OutputPrimaryVolumeDescriptorForTime(idx, buf, fpLog);
}

void OutputVolumePartitionDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	_TCHAR str[2][CD_RAW_READ+1];
	ZeroMemory(str, sizeof(str));
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+8], 32, str[0], sizeof(str[0]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+40], 32, str[1], sizeof(str[1]));
#else
	strncpy(str[0], (CHAR*)&buf[idx+8], 32); str[0][32] = '\0';
	strncpy(str[1], (CHAR*)&buf[idx+40], 32); str[1][32] = '\0';
#endif
	OutputLogString(fpLog, _T("\tSystem Identifier				[%s]\n"), str[0]);
	OutputLogString(fpLog, _T("\tVolume Partition Identifier	[%s]\n"), str[1]);
	OutputLogString(fpLog, _T("\tVolume Partition Location		%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+79], buf[idx+78]), 
		MAKEWORD(buf[idx+77], buf[idx+76])));
	OutputLogString(fpLog, _T("\tVolume Partition Size			%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+87], buf[idx+86]), 
		MAKEWORD(buf[idx+85], buf[idx+84])));
	OutputLogString(fpLog, _T("\tSystem Use	"));
	for(INT i = 88; i <= 2047; i++) {
		OutputLogString(fpLog, _T("%x"), buf[idx+i]);
	}
	OutputLogString(fpLog, _T("\n"));
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
	OutputLogString(fpLog, _T("Volume Recognition Sequence\n"));
	OutputLogString(fpLog, _T("\tStructure Type		%d\n"), buf[idx]);
	_TCHAR str[128+1];
	ZeroMemory(str, sizeof(str));
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+1], 5, str, sizeof(str));
#else
	strncpy(str, (CHAR*)&buf[idx+1], 5); str[5] = '\0';
#endif
	OutputLogString(fpLog, _T("\tStandard Identifier	[%s]\n"), str);
	OutputLogString(fpLog, _T("\tStructure Version	%d\n"), buf[idx+6]);
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
	_TCHAR str[128+1];
	ZeroMemory(str, sizeof(str));
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+1], 5, str, sizeof(str));
#else
	strncpy(str, (CHAR*)&buf[idx+1], 5); str[5] = '\0';
#endif
	if(buf[idx] == 1 && !_tcscmp(str, _T("CD001"))) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
		OutputPrimaryVolumeDescriptorForISO9660(idx, buf, fpLog);
	}
	else if(buf[idx] == 2 && !_tcscmp(str, _T("CD001"))) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
		OutputLogString(fpLog, _T("\tVolume Flags		%d\n"), buf[idx+7]);
		OutputPrimaryVolumeDescriptorForJoliet(idx, buf, fpLog);
	}
	else if(buf[idx] == 255 && !_tcscmp(str, _T("CD001"))) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
	}
	else if(buf[idx] == 0 && !_tcscmp(str, _T("BOOT2"))) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
		OutputBootDescriptor(idx, buf, fpLog);
	}
	else if(buf[idx] == 0 && !_tcscmp(str, _T("BEA01"))) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
	}
	else if(buf[idx] == 0 && !_tcscmp(str, _T("NSR02"))) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
	}
	else if(buf[idx] == 0 && !_tcscmp(str, _T("NSR03"))) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
	}
	else if(buf[idx] == 0 && !_tcscmp(str, _T("TEA01"))) {
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
	OutputLogString(fpLog, _T("\tRecording Date and Time\n"));
	OutputLogString(fpLog, _T("\t\t%x %d-%d-%d %d:%d:%d.%d.%d.%d\n"),
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
	OutputLogString(fpLog, _T("\tArchitecture Type\n"));
	OutputLogString(fpLog, _T("\t\tFlags	%d\n"), buf[idx+8]);

	_TCHAR str[4][CD_RAW_READ+1];
	ZeroMemory(str, sizeof(str));
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+9], 23, str[0], sizeof(str[0]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+24], 8, str[1], sizeof(str[1]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+41], 23, str[2], sizeof(str[2]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+64], 8, str[3], sizeof(str[3]));
#else
	strncpy(str[0], (CHAR*)&buf[idx+9], 23); str[0][23] = '\0';
	strncpy(str[1], (CHAR*)&buf[idx+24], 8); str[1][8] = '\0';
	strncpy(str[2], (CHAR*)&buf[idx+41], 23); str[2][23] = '\0';
	strncpy(str[3], (CHAR*)&buf[idx+64], 8); str[3][8] = '\0';
#endif
	OutputLogString(fpLog, _T("\t\tIdentifier	[%s]\n"), str[0]);
	OutputLogString(fpLog, _T("\t\tIdentifier Suffix	[%s]\n"), str[1]);

	OutputLogString(fpLog, _T("\tBoot Identifier\n"));
	OutputLogString(fpLog, _T("\t\tFlags	%d\n"), buf[idx+40]);
	OutputLogString(fpLog, _T("\t\tIdentifier	[%s]\n"), str[2]);
	OutputLogString(fpLog, _T("\t\tIdentifier Suffix	[%s]\n"), str[3]);

	OutputLogString(fpLog, _T("\tBoot Extent Location	%d\n"),
		MAKELONG(MAKEWORD(buf[idx+75], buf[idx+74]), 
		MAKEWORD(buf[idx+73], buf[idx+72])));
	OutputLogString(fpLog, _T("\tBoot Extent Length	%d\n"),
		MAKELONG(MAKEWORD(buf[idx+79], buf[idx+78]), 
		MAKEWORD(buf[idx+77], buf[idx+76])));
	OutputLogString(fpLog, _T("\tLoad Address	%d%d\n"),
		MAKELONG(MAKEWORD(buf[idx+87], buf[idx+86]), 
		MAKEWORD(buf[idx+85], buf[idx+84])),
		MAKELONG(MAKEWORD(buf[idx+83], buf[idx+82]), 
		MAKEWORD(buf[idx+81], buf[idx+80])));
	OutputLogString(fpLog, _T("\tStart Address	%d%d\n"),
		MAKELONG(MAKEWORD(buf[idx+87], buf[idx+86]), 
		MAKEWORD(buf[idx+85], buf[idx+84])),
		MAKELONG(MAKEWORD(buf[idx+83], buf[idx+82]), 
		MAKEWORD(buf[idx+81], buf[idx+80])));
	OutputRecordingDateAndTime(idx + 96, buf, fpLog);
	OutputLogString(fpLog, _T("\tFlags %d\n"), MAKEWORD(buf[idx+109], buf[idx+108]));
	OutputLogString(fpLog, _T("\tBoot Use	"));
	for(INT i = 142; i <= 2047; i++) {
		OutputLogString(fpLog, _T("%x"), buf[idx+i]);
	}
	OutputLogString(fpLog, _T("\n"));
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
	OutputLogString(fpLog, _T("\t\tCharacter Set Type			%d\n"), buf[idx]);
	_TCHAR str[23+1];
	ZeroMemory(str, sizeof(str));
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+1], 23, str, sizeof(str));
#else
	strncpy(str, (CHAR*)&buf[idx+1], 23); str[23] = '\0';
#endif
	OutputLogString(fpLog, _T("\t\tCharacter Set Information	[%s]\n"), str);
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
	OutputLogString(fpLog, _T("\t\tExtent Length	%d\n"),
		MAKELONG(MAKEWORD(buf[idx], buf[idx+1]), 
		MAKEWORD(buf[idx+2], buf[idx+3])));
	OutputLogString(fpLog, _T("\t\tExtent Location	%d\n"), 
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
	OutputLogString(fpLog, _T("\t\tFlags				%d\n"), buf[idx]);
	_TCHAR str[23+1];
	ZeroMemory(str, sizeof(str));
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+1], 23, str, sizeof(str));
#else
	strncpy(str, (CHAR*)&buf[idx+1], 23); str[23] = '\0';
#endif
	OutputLogString(fpLog, _T("\t\tIdentifier			[%s]\n"), str);
	OutputLogString(fpLog, _T("\t\tIdentifier Suffix	"));
	for(INT i = 24; i <= 31; i++) {
		OutputLogString(fpLog, _T("%x"), buf[idx+i]);
	}
	OutputLogString(fpLog, _T("\n"));
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
	OutputLogString(fpLog, _T("\tVolume Descriptor Sequence Number	%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+16], buf[idx+17]), 
		MAKEWORD(buf[idx+18], buf[idx+19])));
	OutputLogString(fpLog, _T("\tPrimary Volume Descriptor Number	%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+20], buf[idx+21]), 
		MAKEWORD(buf[idx+22], buf[idx+23])));

	_TCHAR str[2][128+1];
	ZeroMemory(str, sizeof(str));
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+24], 32, str[0], sizeof(str[0]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+72], 128, str[1], sizeof(str[1]));
#else
	strncpy(str[0], (CHAR*)&buf[idx+24], 32); str[0][32] = '\0';
	strncpy(str[1], (CHAR*)&buf[idx+72], 128); str[1][128] = '\0';
#endif
	OutputLogString(fpLog, _T("\tVolume Identifier				[%s]\n"), str[0]);
	OutputLogString(fpLog, _T("\tVolume Sequence Number			%d\n"), 
		MAKEWORD(buf[idx+56], buf[idx+57]));
	OutputLogString(fpLog, _T("\tMaximum Volume Sequence Number	%d\n"), 
		MAKEWORD(buf[idx+58], buf[idx+59]));
	OutputLogString(fpLog, _T("\tInterchange Level				%d\n"), 
		MAKEWORD(buf[idx+60], buf[idx+61]));
	OutputLogString(fpLog, _T("\tMaximum Interchange Level		%d\n"), 
		MAKEWORD(buf[idx+62], buf[idx+63]));
	OutputLogString(fpLog, _T("\tCharacter Set List				%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+65], buf[idx+64]), 
		MAKEWORD(buf[idx+67], buf[idx+66])));
	OutputLogString(fpLog, _T("\tMaximum Character Set List		%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+68], buf[idx+69]), 
		MAKEWORD(buf[idx+70], buf[idx+71])));
	OutputLogString(fpLog, _T("\tVolume Set Identifier			[%s]\n"), str[1]);
	OutputLogString(fpLog, _T("\tDescriptor Character Set\n"));
	OutputCharspec(idx + 200, buf, fpLog);

	OutputLogString(fpLog, _T("\tExplanatory Character Set\n"));
	OutputCharspec(idx + 264, buf, fpLog);

	OutputLogString(fpLog, _T("\tVolume Abstract\n"));
	OutputExtentDescriptor(idx + 328, buf, fpLog);

	OutputLogString(fpLog, _T("\tVolume Copyright Notice\n"));
	OutputExtentDescriptor(idx + 336, buf, fpLog);

	OutputLogString(fpLog, _T("\tApplication Identifier\n"));
	OutputRegid(idx + 344, buf, fpLog);

	OutputRecordingDateAndTime(idx + 376, buf, fpLog);

	OutputLogString(fpLog, _T("\tImplementation Identifier\n"));
	OutputRegid(idx + 388, buf, fpLog);
	OutputLogString(fpLog, _T("\tImplementation Use		"));
	for(INT i = 420; i <= 483; i++) {
		OutputLogString(fpLog, _T("%x"), buf[idx+i]);
	}
	OutputLogString(fpLog, _T("\n"));

	OutputLogString(fpLog, _T("\tPredecessor Volume Descriptor Sequence Location	%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+484], buf[idx+485]), 
		MAKEWORD(buf[idx+486], buf[idx+487])));
	OutputLogString(fpLog, _T("\tFlags											%d\n"), 
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
	OutputLogString(fpLog, _T("\tMain Volume Descriptor Sequence Extent\n"));
	OutputExtentDescriptor(idx + 16, buf, fpLog);
	OutputLogString(fpLog, _T("\tReserve Volume Descriptor Sequence Extent\n"));
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
	OutputLogString(fpLog, _T("\tVolume Descriptor Sequence Number	%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+16], buf[idx+17]), 
		MAKEWORD(buf[idx+18], buf[idx+19])));
	OutputLogString(fpLog, _T("\tNext Volume Descriptor Sequence Extent\n"));
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
	OutputLogString(fpLog, _T("\tVolume Descriptor Sequence Number	%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+16], buf[idx+17]), 
		MAKEWORD(buf[idx+18], buf[idx+19])));

	OutputLogString(fpLog, _T("\tImplementation Identifier\n"));
	OutputRegid(idx + 20, buf, fpLog);

	OutputLogString(fpLog, _T("\tLVI Charset\n"));
	OutputCharspec(idx + 52, buf, fpLog);

	_TCHAR str[4][128+1];
	ZeroMemory(str, sizeof(str));
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+116], 128, str[0], sizeof(str[0]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+244], 36, str[1], sizeof(str[1]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+280], 36, str[2], sizeof(str[2]));
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+316], 36, str[3], sizeof(str[3]));
#else
	strncpy(str[0], (CHAR*)&buf[idx+116], 128); str[0][128] = '\0';
	strncpy(str[1], (CHAR*)&buf[idx+244], 36); str[1][36] = '\0';
	strncpy(str[2], (CHAR*)&buf[idx+280], 36); str[2][36] = '\0';
	strncpy(str[3], (CHAR*)&buf[idx+316], 36); str[3][36] = '\0';
#endif
	OutputLogString(fpLog, _T("\tLogical Volume Identifier		[%s]\n"), str[0]);
	OutputLogString(fpLog, _T("\tLV Info 1			[%s]\n"), str[1]);
	OutputLogString(fpLog, _T("\tLV Info 2			[%s]\n"), str[2]);
	OutputLogString(fpLog, _T("\tLV Info 3			[%s]\n"), str[3]);
	OutputLogString(fpLog, _T("\tImplemention ID\n"));
	OutputRegid(idx + 352, buf, fpLog);
	OutputLogString(fpLog, _T("\tImplementation Use		"));
	for(INT i = 384; i <= 511; i++) {
		OutputLogString(fpLog, _T("%x"), buf[idx+i]);
	}
	OutputLogString(fpLog, _T("\n"));
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
	OutputLogString(fpLog, _T("\tVolume Descriptor Sequence Number	%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+16], buf[idx+17]), 
		MAKEWORD(buf[idx+18], buf[idx+19])));
	OutputLogString(fpLog, _T("\tPartition Flags						%d\n"), 
		MAKEWORD(buf[idx+20], buf[idx+21]));
	OutputLogString(fpLog, _T("\tPartition Number					%d\n"), 
		MAKEWORD(buf[idx+22], buf[idx+23]));
	OutputLogString(fpLog, _T("\tPartition Contents\n"));
	OutputRegid(idx + 24, buf, fpLog);

	OutputLogString(fpLog, _T("\tPartition Contents Use	"));
	for(INT i = 56; i <= 183; i++) {
		OutputLogString(fpLog, _T("%x"), buf[idx+i]);
	}
	OutputLogString(fpLog, _T("\n"));
	OutputLogString(fpLog, _T("\tAccess Type					%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+184], buf[idx+185]), 
		MAKEWORD(buf[idx+186], buf[idx+187])));
	OutputLogString(fpLog, _T("\tPartition Starting Location	%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+188], buf[idx+189]), 
		MAKEWORD(buf[idx+190], buf[idx+191])));
	OutputLogString(fpLog, _T("\tPartition Length			%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+192], buf[idx+193]), 
		MAKEWORD(buf[idx+194], buf[idx+195])));
	OutputLogString(fpLog, _T("\tImplementation Identifier\n"));
	OutputRegid(idx + 196, buf, fpLog);
	OutputLogString(fpLog, _T("\tImplementation Use		"));
	for(INT i = 228; i <= 355; i++) {
		OutputLogString(fpLog, _T("%x"), buf[idx+i]);
	}
	OutputLogString(fpLog, _T("\n"));
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
	OutputLogString(fpLog, _T("\tLongAllocationDescriptor\n"));
	OutputLogString(fpLog, _T("\t\tExtent Length				%d\n"), 
		MAKELONG(MAKEWORD(buf[idx], buf[idx+1]), 
		MAKEWORD(buf[idx+2], buf[idx+3])));
	OutputLogString(fpLog, _T("\t\tLogical Block Number		%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+4], buf[idx+5]), 
		MAKEWORD(buf[idx+6], buf[idx+7])));
	OutputLogString(fpLog, _T("\t\tPartition Reference Number	%d\n"), 
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
	OutputLogString(fpLog, _T("\tVolume Descriptor Sequence Number	%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+16], buf[idx+17]), 
		MAKEWORD(buf[idx+18], buf[idx+19])));
	OutputLogString(fpLog, _T("\tDescriptor Character Set\n"));
	OutputCharspec(idx + 20, buf, fpLog);

	_TCHAR str[128+1];
	ZeroMemory(str, sizeof(str));
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)&buf[idx+84], 128, str, sizeof(str));
#else
	strncpy(str, (CHAR*)&buf[idx+84], 128); str[128] = '\0';
#endif
	OutputLogString(fpLog, _T("\tLogical Volume Identifier		[%s]\n"), str);
	OutputLogString(fpLog, _T("\tLogical Block Size				%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+212], buf[idx+213]), 
		MAKEWORD(buf[idx+214], buf[idx+215])));
	OutputLogString(fpLog, _T("\tDomain Identifier\n"));
	OutputCharspec(idx + 216, buf, fpLog);
	OutputLongAllocationDescriptor(idx + 248, buf, fpLog);

	LONG MT_L = MAKELONG(MAKEWORD(buf[idx+264], buf[idx+265]), 
		MAKEWORD(buf[idx+266], buf[idx+267]));
	OutputLogString(fpLog, _T("\tMap Table Length				%d\n"), MT_L);
	OutputLogString(fpLog, _T("\tNumber of Partition Maps		%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+268], buf[idx+269]), 
		MAKEWORD(buf[idx+270], buf[idx+271])));
	OutputLogString(fpLog, _T("\tImplementation Identifier\n"));
	OutputRegid(idx + 272, buf, fpLog);

	OutputLogString(fpLog, _T("\tImplementation Use		"));
	for(INT i = 304; i <= 431; i++) {
		OutputLogString(fpLog, _T("%x"), buf[idx+i]);
	}
	OutputLogString(fpLog, _T("\n"));
	OutputLogString(fpLog, _T("\tIntegrity Sequence Extent\n"));
	OutputExtentDescriptor(idx + 432, buf, fpLog);

	OutputLogString(fpLog, _T("\tPartition Maps		"));
	for(INT i = 0; i < MT_L; i++) {
		OutputLogString(fpLog, _T("%x"), buf[idx+440+i]);
	}
	OutputLogString(fpLog, _T("\n"));
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
	OutputLogString(fpLog, _T("\tVolume Descriptor Sequence Number	%d\n"), 
		MAKELONG(MAKEWORD(buf[idx+16], buf[idx+17]), 
		MAKEWORD(buf[idx+18], buf[idx+19])));
	LONG N_AD = MAKELONG(MAKEWORD(buf[idx+20], buf[idx+21]), 
		MAKEWORD(buf[idx+22], buf[idx+23]));
	OutputLogString(fpLog, _T("\tNumber of Allocation Descriptors	%d\n"), N_AD);
	OutputLogString(fpLog, _T("\tAllocation Descriptors\n"));
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
		OutputLogString(fpLog, _T("Primary Volume Descriptor\n"));
		break;
	case 2:
		OutputLogString(fpLog, _T("Anchor Volume Descriptor Pointer\n"));
		break;
	case 3:
		OutputLogString(fpLog, _T("Volume Descriptor Pointer\n"));
		break;
	case 4:
		OutputLogString(fpLog, _T("Implementation Use Volume Descriptor\n"));
		break;
	case 5:
		OutputLogString(fpLog, _T("Partition Descriptor\n"));
		break;
	case 6:
		OutputLogString(fpLog, _T("Logical Volume Descriptor\n"));
		break;
	case 7:
		OutputLogString(fpLog, _T("Unallocated Space Descriptor\n"));
		break;
	case 8:
		OutputLogString(fpLog, _T("Terminating Descriptor\n"));
		break;
	case 9:
		OutputLogString(fpLog, _T("Logical Volume Integrity Descriptor\n"));
		break;
	case 256:
		OutputLogString(fpLog, _T("File Set Descriptor\n"));
		break;
	case 257:
		OutputLogString(fpLog, _T("File Identifier Descriptor\n"));
		break;
	case 258:
		OutputLogString(fpLog, _T("Allocation Extent Descriptor\n"));
		break;
	case 259:
		OutputLogString(fpLog, _T("Indirect Entry\n"));
		break;
	case 260:
		OutputLogString(fpLog, _T("Terminal Entry\n"));
		break;
	case 261:
		OutputLogString(fpLog, _T("File Entry\n"));
		break;
	case 262:
		OutputLogString(fpLog, _T("Extended Attribute Header Descriptor\n"));
		break;
	case 263:
		OutputLogString(fpLog, _T("Unallocated Space Entry\n"));
		break;
	case 264:
		OutputLogString(fpLog, _T("Space Bitmap Descriptor\n"));
		break;
	case 265:
		OutputLogString(fpLog, _T("Partition Integrity Entry\n"));
		break;
	case 266:
		OutputLogString(fpLog, _T("Extended File Entry\n"));
		break;
	}

	OutputLogString(fpLog, _T("\t\tDescriptor Version		%d\n"), 
		MAKEWORD(buf[idx+2], buf[idx+3]));
	OutputLogString(fpLog, _T("\t\tTag Checksum			%d\n"), buf[idx+4]);
	OutputLogString(fpLog, _T("\t\tTag Serial Number		%d\n"), 
		MAKEWORD(buf[idx+6], buf[idx+7]));
	OutputLogString(fpLog, _T("\t\tDescriptor CRC			%x\n"), 
		MAKEWORD(buf[idx+8], buf[idx+9]));
	OutputLogString(fpLog, _T("\t\tDescriptor CRC Length	%d\n"), 
		MAKEWORD(buf[idx+10], buf[idx+11]));
	OutputLogString(fpLog, _T("\t\tTag Location			%d\n"),
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
	OutputLogString(fpLog, _T("\t\tLBA %7u, "), nLBA + i);
	if((pBuf2[4] & 0x80) == 0) {
		OutputLogString(fpLog, _T("CPM don't exists"));
		if((pBuf2[4] & 0x30) == 0) {
			OutputLogString(fpLog, _T(", copying is permitted without restriction\n"));
		}
		else {
			OutputLogString(fpLog, _T("\n"));
		}
	}
	else if((pBuf2[4] & 0x80) == 0x80) {
		OutputLogString(fpLog, _T("CPM exists"));
		if((pBuf2[4] & 0x40) == 0) {
			OutputLogString(fpLog, _T(", CSS or CPPM don't exists in this sector"));
		}
		else if((pBuf2[4] & 0x40) == 0x40) {
			if((pBuf2[4] & 0x0F) == 0) {
				OutputLogString(fpLog, _T("the sector is scrambled by CSS"));
			}
			else if((pBuf2[4] & 0x0F) == 1) {
				OutputLogString(fpLog, _T("the sector is encrypted by CPPM"));
			}
		}
		if((pBuf2[4] & 0x30) == 0) {
			OutputLogString(fpLog, _T(", copying is permitted without restriction\n"));
		}
		else if((pBuf2[4] & 0x30) == 0x02) {
			OutputLogString(fpLog, _T(", one generation of copies may be made\n"));
		}
		else if((pBuf2[4] & 0x30) == 0x03) {
			OutputLogString(fpLog, _T(", no copying is permitted\n"));
		}
		else {
			OutputLogString(fpLog, _T("\n"));
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
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, pszString, META_STRING_SIZE, szAlbumTitle, sizeof(szAlbumTitle));
#else
	strncpy(szAlbumTitle, pszString, strlen(pszString));
#endif
	szAlbumTitle[META_STRING_SIZE] = '\0';
}

void SetISRCToString(
	CONST PUCHAR Subcode,
	INT nTrackNum,
	LPTSTR pszOutString,
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
	_stprintf(pszOutString, _T("%c%c%c%c%c%c%c%c%c%c%c%c"),
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
		_tcsncpy(szISRC[nTrackNum-1], pszOutString, _tcslen(pszOutString));
		szISRC[nTrackNum-1][12] = '\0';
	}
}

void SetMCNToString(
	CONST PUCHAR Subcode,
	LPTSTR pszOutString,
	BOOL bCopy
	)
{
	_stprintf(pszOutString, _T("%c%c%c%c%c%c%c%c%c%c%c%c%c"), 
		((Subcode[13] >> 4) & 0x0F) + 0x30, (Subcode[13] & 0x0F) + 0x30, 
		((Subcode[14] >> 4) & 0x0F) + 0x30, (Subcode[14] & 0x0F) + 0x30, 
		((Subcode[15] >> 4) & 0x0F) + 0x30, (Subcode[15] & 0x0F) + 0x30, 
		((Subcode[16] >> 4) & 0x0F) + 0x30, (Subcode[16] & 0x0F) + 0x30, 
		((Subcode[17] >> 4) & 0x0F) + 0x30, (Subcode[17] & 0x0F) + 0x30, 
		((Subcode[18] >> 4) & 0x0F) + 0x30, (Subcode[18] & 0x0F) + 0x30, 
		((Subcode[19] >> 4) & 0x0F) + 0x30);
	if(bCopy) {
		_tcsncpy(szCatalog, pszOutString, _tcslen(pszOutString));
		szCatalog[13] = '\0';
	}
}

void SetPerformer(
	LPCSTR pszString
	)
{
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, pszString, (INT)strlen(pszString), szPerformer, sizeof(szPerformer));
#else
	strncpy(szPerformer, pszString, strlen(pszString));
#endif
	szPerformer[META_STRING_SIZE] = '\0';
}

void SetSongWriter(
	LPCSTR pszString
	)
{
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, pszString, (INT)strlen(pszString), szSongWriter, sizeof(szSongWriter));
#else
	strncpy(szSongWriter, pszString, strlen(pszString));
#endif
	szSongWriter[META_STRING_SIZE] = '\0';
}

void SetTitle(
	LPCSTR pszString,
	INT idx
	)
{
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, pszString, (INT)strlen(pszString), szTitle[idx], sizeof(szTitle[idx]));
#else
	strncpy(szTitle[idx], pszString, strlen(pszString));
#endif
	szTitle[idx][META_STRING_SIZE] = '\0';
}

void WriteCcdFileForDisc(
	size_t tocEntries,
	UCHAR LastCompleteSession,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd, _T("[CloneCD]\n"));
	_ftprintf(fpCcd, _T("Version=3\n"));
	_ftprintf(fpCcd, _T("[Disc]\n"));
	_ftprintf(fpCcd, _T("TocEntries=%d\n"), tocEntries);
	_ftprintf(fpCcd, _T("Sessions=%d\n"), LastCompleteSession);
	_ftprintf(fpCcd, _T("DataTracksScrambled=%d\n"), 0); // TODO
}

void WriteCcdFileForDiscCDTextLength(
	size_t cdTextSize,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd, _T("CDTextLength=%d\n"), cdTextSize);
}

void WriteCcdFileForDiscCatalog(
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd, _T("CATALOG=%s\n"), szCatalog);
}

void WriteCcdFileForCDText(
	size_t cdTextSize,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd, _T("[CDText]\n"));
	_ftprintf(fpCcd, _T("Entries=%d\n"), cdTextSize);
}

void WriteCcdFileForCDTextEntry(
	size_t t,
	CONST CDROM_TOC_CD_TEXT_DATA_BLOCK* pDesc,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd, 
		_T("Entry %d=%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"), 
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
	_ftprintf(fpCcd, _T("[Session %d]\n"), SessionNumber);
}

void WriteCcdFileForSessionPregap(
	UCHAR mode,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd, _T("PreGapMode=%d\n"), mode);
	_ftprintf(fpCcd, _T("PreGapSubC=%d\n"), 0);	// TODO
}

void WriteCcdFileForEntry(
	size_t a,
	CONST CDROM_TOC_FULL_TOC_DATA_BLOCK* toc,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd, _T("[Entry %d]\n"), a);
	_ftprintf(fpCcd, _T("Session=%d\n"), toc[a].SessionNumber);
	_ftprintf(fpCcd, _T("Point=0x%02x\n"), toc[a].Point);
	_ftprintf(fpCcd, _T("ADR=0x%02x\n"), toc[a].Adr);
	_ftprintf(fpCcd, _T("Control=0x%02x\n"), toc[a].Control);
	_ftprintf(fpCcd, _T("TrackNo=%d\n"), toc[a].Reserved1);
	_ftprintf(fpCcd, _T("AMin=%d\n"), toc[a].MsfExtra[0]);
	_ftprintf(fpCcd, _T("ASec=%d\n"), toc[a].MsfExtra[1]);
	_ftprintf(fpCcd, _T("AFrame=%d\n"), toc[a].MsfExtra[2]);
	_ftprintf(fpCcd, _T("ALBA=%d\n"), MSFtoLBA(toc[a].MsfExtra[2], 
		toc[a].MsfExtra[1], toc[a].MsfExtra[0]) - 150);
	_ftprintf(fpCcd, _T("Zero=%d\n"), toc[a].Zero);
	_ftprintf(fpCcd, _T("PMin=%d\n"), toc[a].Msf[0]);
	_ftprintf(fpCcd, _T("PSec=%d\n"), toc[a].Msf[1]);
	_ftprintf(fpCcd, _T("PFrame=%d\n"), toc[a].Msf[2]);
	_ftprintf(fpCcd, _T("PLBA=%d\n"), 
		MSFtoLBA(toc[a].Msf[2], toc[a].Msf[1], toc[a].Msf[0]) - 150);
}

void WriteCcdFileForTrack(
	INT nTrackNum,
	UCHAR byModeNum,
	BOOL bISRC,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd, _T("[TRACK %d]\n"), nTrackNum);
	_ftprintf(fpCcd, _T("MODE=%d\n"), byModeNum);
	if(bISRC) {
		_ftprintf(fpCcd, _T("ISRC=%s\n"), szISRC[nTrackNum-1]);
	}
}

void WriteCcdFileForTrackIndex(
	LONG index,
	LONG lba,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd, _T("INDEX %d=%d\n"), index, lba);
}

void WriteCueFile(
	BOOL bCatalog,
	LPCTSTR pszFilename,
	INT nTrackNum,
	UCHAR byModeNum,
	BOOL bISRC,
	UCHAR byCtl,
	FILE* fpCue
	)
{
	if(bCatalog) {
		_ftprintf(fpCue, _T("CATALOG %s\n"), szCatalog);
	}
	if(_tcslen(szAlbumTitle) > 0) {
		_ftprintf(fpCue, _T("TITLE \"%s\"\n"), szAlbumTitle);
	}
	if(_tcslen(szPerformer) > 0) {
		_ftprintf(fpCue, _T("PERFORMER \"%s\"\n"), szPerformer);
	}
	if(_tcslen(szSongWriter) > 0) {
		_ftprintf(fpCue, _T("SONGWRITER \"%s\"\n"), szSongWriter);
	}
	_ftprintf(fpCue, _T("FILE \"%s\" BINARY\n"), pszFilename);

	if(byModeNum == DATA_BLOCK_MODE0) {
		_ftprintf(fpCue, _T("  TRACK %02d AUDIO\n"), nTrackNum);
		if(bISRC) {
			_ftprintf(fpCue, _T("    ISRC %s\n"), szISRC[nTrackNum-1]);
		}
		if(_tcslen(szTitle[nTrackNum-1]) > 0) {
			_ftprintf(fpCue, _T("    TITLE \"%s\"\n"), szTitle[nTrackNum-1]);
		}
		if((byCtl & AUDIO_WITH_PREEMPHASIS) == AUDIO_WITH_PREEMPHASIS ||
			(byCtl & DIGITAL_COPY_PERMITTED) == DIGITAL_COPY_PERMITTED ||
			(byCtl & TWO_FOUR_CHANNEL_AUDIO) == TWO_FOUR_CHANNEL_AUDIO) {
			_TCHAR aBuf[22];
			ZeroMemory(aBuf, sizeof(aBuf));
			_tcscat(aBuf, _T("    FLAGS"));
			switch(byCtl) {
			case AUDIO_WITH_PREEMPHASIS:
				_tcscat(aBuf, _T(" PRE\n"));
				break;
			case DIGITAL_COPY_PERMITTED:
				_tcscat(aBuf, _T(" DCP\n"));
				break;
			case DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
				_tcscat(aBuf, _T(" DCP PRE\n"));
				break;
			case TWO_FOUR_CHANNEL_AUDIO:
				_tcscat(aBuf, _T(" 4CH\n"));
				break;
			case TWO_FOUR_CHANNEL_AUDIO | AUDIO_WITH_PREEMPHASIS:
				_tcscat(aBuf, _T(" 4CH PRE\n"));
				break;
			case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED:
				_tcscat(aBuf, _T(" 4CH DCP\n"));
				break;
			case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
				_tcscat(aBuf, _T(" 4CH DCP PRE\n"));
				break;
			}
			fwrite(aBuf, sizeof(CHAR), _tcslen(aBuf), fpCue);
		}
	}
	else {
		_ftprintf(fpCue, _T("  TRACK %02d MODE%1d/2352\n"), nTrackNum, byModeNum);
		if((byCtl & DIGITAL_COPY_PERMITTED) == DIGITAL_COPY_PERMITTED) {
			_ftprintf(fpCue, _T("    FLAGS DCP\n"));
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
	_ftprintf(fpCue, _T("    INDEX %02d %02d:%02d:%02d\n"), 
		byIndex, byMinute, bySecond, byFrame);
}
