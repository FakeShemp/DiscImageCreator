/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"

_TCHAR szCatalog[META_CATALOG_SIZE+1] = {0};
_TCHAR szISRC[MAXIMUM_NUMBER_TRACKS][META_ISRC_SIZE+1] = {0};
_TCHAR szPerformer[META_STRING_SIZE+1] = {0};
_TCHAR szSongWriter[META_STRING_SIZE+1] = {0};
_TCHAR szAlbumTitle[META_STRING_SIZE+1] = {0};
_TCHAR szTitle[MAXIMUM_NUMBER_TRACKS][META_STRING_SIZE+1] = {0};

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
	TCHAR szDstPath[_MAX_PATH] = {0};
	TCHAR drive[_MAX_DRIVE] = {0};
	TCHAR dir[_MAX_DIR] = {0};
	TCHAR fname[_MAX_FNAME] = {0};
	TCHAR ext[_MAX_EXT] = {0};

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
	FILE* fp = _tfopen(szDstPath, pszMode);
#ifdef UNICODE
	// delete bom
	fseek(fp, 0, SEEK_SET);
#endif
	return fp;
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
	CHAR szDstPath[_MAX_PATH] = {0};
	CHAR drive[_MAX_DRIVE] = {0};
	CHAR dir[_MAX_DIR] = {0};
	CHAR fname[_MAX_FNAME] = {0};
	CHAR ext[_MAX_EXT] = {0};

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

FILE* OpenProgrammabledFile(
	LPCTSTR pszFilename,
	LPCTSTR pszMode
	)
{
	FILE* fp = NULL;
	TCHAR dir[MAX_PATH] = {0};
	::GetModuleFileName(NULL, dir, MAX_PATH);

	TCHAR* pdest = _tcsrchr(dir, '\\');
	if(pdest) {
		pdest[0] = NULL;
		TCHAR buf[MAX_PATH] = {0};
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
		OutputLogString(fpLog, _T(
			"\t%02x %02x %02x %02x %02x %02x %02x %02x\n"), 
			pBuf[i], pBuf[i+1], pBuf[i+2], pBuf[i+3], 
			pBuf[i+4], pBuf[i+5], pBuf[i+6], pBuf[i+7]);
	}
}

void OutputMain2352(
	CONST PUCHAR pBuf,
	INT nLBA,
	FILE* fpLog
	)
{
	OutputLogString(fpLog, 
		_T("Main Channel LBA %d\n")
		_T("\t    +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n"),
		nLBA);

	for(INT i = 0; i < CD_RAW_SECTOR_SIZE; i += 16) {
		OutputLogString(fpLog, 
			_T("\t%3X %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"), 
			i, pBuf[i], pBuf[i+1], pBuf[i+2], pBuf[i+3], pBuf[i+4], pBuf[i+5],
			pBuf[i+6], pBuf[i+7], pBuf[i+8], pBuf[i+9], pBuf[i+10], pBuf[i+11], 
			pBuf[i+12], pBuf[i+13], pBuf[i+14], pBuf[i+15]);
	}
}

void OutputDriveSpeed(
	PCDROM_SET_SPEED pSetspeed,
	FILE* fpLog
	)
{
	OutputLogString(fpLog,
		_T("Drive speed\n")
		_T("\t    RequestType: %s\n")
		_T("\t      ReadSpeed: %uKB/sec\n")
		_T("\t     WriteSpeed: %uKB/sec\n")
		_T("\tRotationControl: %s\n"),
		pSetspeed->RequestType == 0 ? _T("CdromSetSpeed") : _T("CdromSetStreaming"),
		pSetspeed->ReadSpeed,
		pSetspeed->WriteSpeed,
		pSetspeed->RotationControl == 0 ? _T("CdromDefaultRotation") : _T("CdromCAVRotation")
		);
}

void OutputScsiAdress(
	PDEVICE_DATA pDevData,
	FILE* fpLog
	)
{
	OutputLogString(fpLog, 
		_T("IOCTL_SCSI_GET_ADDRESS\n")
		_T("\t    Length: %x\n")
		_T("\tPortNumber: %x\n")
		_T("\t    PathId: %x\n")
		_T("\t  TargetId: %x\n")
		_T("\t       Lun: %x\n"),
		pDevData->adress.Length,
		pDevData->adress.PortNumber,
		pDevData->adress.PathId,
		pDevData->adress.TargetId,
		pDevData->adress.Lun);
}

void OutputStorageAdaptorDescriptor(
	PDEVICE_DATA pDevData,
	FILE* fpLog
	)
{
	OutputLogString(fpLog,
		_T("STORAGE ADAPTER DESCRIPTOR DATA\n")
		_T("\t              Version: %08x\n")
		_T("\t            TotalSize: %08x\n")
		_T("\tMaximumTransferLength: %08x (bytes)\n")
		_T("\t MaximumPhysicalPages: %08x\n")
		_T("\t        AlignmentMask: %08x\n")
		_T("\t       AdapterUsesPio: %s\n")
		_T("\t     AdapterScansDown: %s\n")
		_T("\t      CommandQueueing: %s\n")
		_T("\t  AcceleratedTransfer: %s\n")
		_T("\t      BusMajorVersion: %04x\n")
		_T("\t      BusMinorVersion: %04x\n"),
		pDevData->adapterDescriptor->Version,
		pDevData->adapterDescriptor->Size,
		pDevData->adapterDescriptor->MaximumTransferLength,
		pDevData->adapterDescriptor->MaximumPhysicalPages,
		pDevData->adapterDescriptor->AlignmentMask,
		BOOLEAN_TO_STRING_TRUE_FALSE(pDevData->adapterDescriptor->AdapterUsesPio),
		BOOLEAN_TO_STRING_TRUE_FALSE(pDevData->adapterDescriptor->AdapterScansDown),
		BOOLEAN_TO_STRING_TRUE_FALSE(pDevData->adapterDescriptor->CommandQueueing),
		BOOLEAN_TO_STRING_TRUE_FALSE(pDevData->adapterDescriptor->AcceleratedTransfer),
		pDevData->adapterDescriptor->BusMajorVersion,
		pDevData->adapterDescriptor->BusMinorVersion);
}

void OutputInquiryData(
	PINQUIRYDATA pInquiry,
	PDISC_DATA pDiscData,
	FILE* fpLog
	)
{
	OutputLogString(fpLog,
		_T("Device Info\n")
		_T("\t          DeviceType: "));
	switch(pInquiry->DeviceType) {
	case READ_ONLY_DIRECT_ACCESS_DEVICE:
		OutputLogString(fpLog, _T("CD/DVD device\n"));
		break;
	default:
		OutputLogString(fpLog, _T("Other device\n"));
		break;
	}
	OutputLogString(fpLog,
		_T("\t DeviceTypeQualifier: "));
	switch(pInquiry->DeviceTypeQualifier) {
	case DEVICE_QUALIFIER_ACTIVE:
		OutputLogString(fpLog, _T("Active\n"));
		break;
	case DEVICE_QUALIFIER_NOT_ACTIVE:
		OutputLogString(fpLog, _T("Not Active\n"));
		break;
	case DEVICE_QUALIFIER_NOT_SUPPORTED:
		OutputLogString(fpLog, _T("Not Supported\n"));
		break;
	default:
		OutputLogString(fpLog, _T("\n"));
		break;
	}

	OutputLogString(fpLog,
		_T("\t  DeviceTypeModifier: %x\n")
		_T("\t      RemovableMedia: %s\n")
		_T("\t            Versions: %x\n")
		_T("\t  ResponseDataFormat: %x\n")
		_T("\t           HiSupport: %s\n")
		_T("\t             NormACA: %s\n")
		_T("\t       TerminateTask: %s\n")
		_T("\t                AERC: %s\n")
		_T("\t    AdditionalLength: %x\n")
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
		BOOLEAN_TO_STRING_YES_NO(pInquiry->RelativeAddressing)
		);

	strncpy(pDiscData->pszVendorId, (PCHAR)pInquiry->VendorId, sizeof(pInquiry->VendorId));
	strncpy(pDiscData->pszProductId, (PCHAR)pInquiry->ProductId, sizeof(pInquiry->ProductId));
#ifdef UNICODE
	TCHAR buf1[8] = {0};
	TCHAR buf2[16] = {0};
	TCHAR buf3[4] = {0};
	TCHAR buf4[20] = {0};
	MultiByteToWideChar(CP_ACP, 0, 
		(PCHAR)pInquiry->VendorId, sizeof(pInquiry->VendorId), buf1, sizeof(buf1));
	MultiByteToWideChar(CP_ACP, 0, 
		(PCHAR)pInquiry->ProductId, sizeof(pInquiry->ProductId), buf2, sizeof(buf2));
	MultiByteToWideChar(CP_ACP, 0, 
		(PCHAR)pInquiry->ProductRevisionLevel, sizeof(pInquiry->ProductRevisionLevel), buf3, sizeof(buf3));
	MultiByteToWideChar(CP_ACP, 0, 
		(PCHAR)pInquiry->VendorSpecific, sizeof(pInquiry->VendorSpecific), buf4, sizeof(buf4));
	OutputLogString(fpLog,
		_T("\t            VendorId: %.8s\n")
		_T("\t           ProductId: %.16s\n")
		_T("\tProductRevisionLevel: %.4s\n")
		_T("\t      VendorSpecific: %.20s\n"),
		buf1,
		buf2,
		buf3,
		buf4);
#else
	OutputLogString(fpLog,
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
		LPCTSTR lpBookType[] = {
			_T("DVD-ROM"), _T("DVD-RAM"), _T("DVD-R"), _T("DVD-RW"),
			_T("HD DVD-ROM"), _T("HD DVD-RAM"), _T("HD DVD-R"), _T("Reserved"),
			_T("Reserved"), _T("DVD+RW"), _T("DVD+R"), _T("Reserved"),
			_T("Reserved"), _T("DVD+RW DL"), _T("DVD+R DL"), _T("Reserved")
		};

		LPCTSTR lpMaximumRate[] = {
			_T("2.52Mbps"), _T("5.04Mbps"), _T("10.08Mbps"), _T("20.16Mbps"),
			_T("30.24Mbps"), _T("Reserved"),	_T("Reserved"), _T("Reserved"),
			_T("Reserved"), _T("Reserved"),	_T("Reserved"), _T("Reserved"),
			_T("Reserved"), _T("Reserved"), _T("Not Specified")
		};

		LPCTSTR lpLayerType[] = {
			_T("Layer contains embossed data"), _T("Layer contains recordable data"), 
			_T("Layer contains rewritable data"), _T("Reserved")
		};

		LPCTSTR lpTrackDensity[] = {
			_T("0.74μm/track"), _T("0.80μm/track"), _T("0.615μm/track"),
			_T("0.40μm/track"), _T("0.34μm/track"), _T("Reserved"),
			_T("Reserved"), _T("Reserved"), _T("Reserved"), _T("Reserved"),
			_T("Reserved"), _T("Reserved"), _T("Reserved"), _T("Reserved"),
			_T("Reserved"), _T("Reserved")
		};

		LPCTSTR lpLinearDensity[] = {
			_T("0.267μm/bit"), _T("0.293μm/bit"), _T("0.409 to 0.435μm/bit"),
			_T("Reserved"),	_T("0.280 to 0.291μm/bit"), _T("0.153μm/bit"),
			_T("0.130 to 0.140μm/bit"), _T("Reserved"), _T("0.353μm/bit"),
			_T("Reserved"), _T("Reserved"), _T("Reserved"),	_T("Reserved"),
			_T("Reserved"), _T("Reserved")
		};

		LONG ulStartSectorNum = MAKELONG(MAKEWORD(pStructure[11], 
			pStructure[10]), MAKEWORD(pStructure[9], pStructure[8]));

		LONG ulEndSectorNum = MAKELONG(MAKEWORD(pStructure[15], 
			pStructure[14]), MAKEWORD(pStructure[13], pStructure[12]));

		*nDVDSectorSize = ulEndSectorNum - ulStartSectorNum + 1;
		INT ulEndSectorLayer0 = MAKELONG(MAKEWORD(pStructure[19], 
			pStructure[18]), MAKEWORD(pStructure[17], pStructure[16]));

		OutputLogString(fpLog, _T(
			"\tPhysicalFormatInformation\n")
			_T("\t\t       BookVersion: %d\n")
			_T("\t\t          BookType: %s\n")
			_T("\t\t       MinimumRate: %s\n")
			_T("\t\t          DiskSize: %s\n")
			_T("\t\t         LayerType: %s\n")
			_T("\t\t         TrackPath: %s\n")
			_T("\t\t    NumberOfLayers: %s\n")
			_T("\t\t      TrackDensity: %s\n")
			_T("\t\t     LinearDensity: %s\n")
			_T("\t\t   StartDataSector: %d(0x%x)\n")
			_T("\t\t     EndDataSector: %d(0x%x)\n")
			_T("\t\tEndLayerZeroSector: %x\n")
			_T("\t\t           BCAFlag: %s\n")
			_T("\t\t     MediaSpecific: "),
			pStructure[4] & 0x0F,
			lpBookType[pStructure[4]>>4&0x0F],
			lpMaximumRate[pStructure[5]&0x0F],
			(pStructure[5] & 0xF0) == 0 ? _T("120mm") : _T("80mm"),
			lpLayerType[pStructure[6]&0x0F],
			(pStructure[6] & 0x10) == 0 ? _T("Parallel Track Path") : _T("Opposite Track Path"),
			(pStructure[6] & 0x60) == 0 ? _T("Single Layer") : _T("Double Layer"),
			lpTrackDensity[pStructure[7]&0x0F],
			lpLinearDensity[pStructure[7]>>4&0x0F],
			ulStartSectorNum, ulStartSectorNum,
			ulEndSectorNum, ulEndSectorNum,
			ulEndSectorLayer0,
			(pStructure[20] & 0x80) == 0 ? _T("None") : _T("Exist"));

		for(ULONG k = 0; k < 2031; k++) {
			OutputLogString(fpLog, _T("%02x"), pStructure[21+k]);
		}
		OutputLogString(fpLog, _T("\n"));
		break;
	}
	case DvdCopyrightDescriptor:
		OutputLogString(fpLog, _T(
			"\tCopyrightProtectionType: "));
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
			OutputLogString(fpLog, _T("Unknown: %02x\n"), pStructure[4]);
			break;
		}
		OutputLogString(fpLog, 
			_T("\tRegionManagementInformation: %02x\n"), pStructure[5]);
		break;
	case DvdDiskKeyDescriptor:
		OutputLogString(fpLog, _T("\tDiskKeyData: "));
		for(ULONG k = 0; k < 2048; k++) {
			OutputLogString(fpLog, _T("%02x"), pStructure[4+k]);
		}
		OutputLogString(fpLog, _T("\n"));
		break;
	case DvdBCADescriptor:
		OutputLogString(fpLog, _T("\tBCAInformation: "));
		for(ULONG k = 0; 
			k < pStructureLength[i] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
			OutputLogString(fpLog, _T("%02x"), pStructure[4+k]);
		}
		OutputLogString(fpLog, _T("\n"));
		break;
	case DvdManufacturerDescriptor:
		OutputLogString(fpLog, _T("\tManufacturingInformation: "));
		for(ULONG k = 0; k < 2048; k++) {
			OutputLogString(fpLog, _T("%02x"), pStructure[4+k]);
		}
		OutputLogString(fpLog, _T("\n"));
		break;
	case 6:
		OutputLogString(fpLog, _T("\tmedia ID: "));
		for(ULONG k = 0; 
			k < pStructureLength[i] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
			OutputLogString(fpLog, _T("%02x"), pStructure[4+k]);
		}
		OutputLogString(fpLog, _T("\n"));
		break;
	case 7:
		OutputLogString(fpLog,
			_T("\tMedia Key Block Total Packs: %d")
			_T("\tmedia key block: "),
			pStructure[3]);
		for(ULONG k = 0; 
			k < pStructureLength[i] - sizeof(DVD_DESCRIPTOR_HEADER); k++) {
			OutputLogString(fpLog, _T("%02x"), pStructure[4+k]);
		}
		OutputLogString(fpLog, _T("\n"));
		break;
	default:
		OutputLogString(fpLog, _T("\tUnknown: %02x\n"), pFormat[i]);
		break;
	}
}

void OutputFeatureNumber(
	CONST PUCHAR pConf,
	ULONG ulAllLen,
	size_t uiSize,
	PDISC_DATA pDiscData,
	FILE* fpLog
	)
{
	ULONG n = 0;
	LONG lVal = 0;
	WORD wVal = 0;
	while(n < ulAllLen - uiSize) { 
		WORD nCode = MAKEWORD(pConf[uiSize+1+n], pConf[uiSize+0+n]);
		switch(nCode) {
		case FeatureProfileList:
			OutputLogString(fpLog,
				_T("\tFeatureProfileList\n")
				_T("\t\t"));
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
			OutputLogString(fpLog,
				_T("\tFeatureCore\n")
				_T("\t\tPhysicalInterface: "));
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
				OutputLogString(fpLog, _T("Reserved: %08d\n"), lVal);
				break;
			}
			OutputLogString(fpLog, 
				_T("\t\t  DeviceBusyEvent: %s\n")
				_T("\t\t         INQUIRY2: %s\n"), 
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+8+n] & 0x01),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+8+n] & 0x02));
			break;
		}
		case FeatureMorphing:
			OutputLogString(fpLog,
				_T("\tFeatureMorphing\n")
				_T("\t\tAsynchronous: %s\n")
				_T("\t\t     OCEvent: %s\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x01),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x02));
			break;
		case FeatureRemovableMedium:
			OutputLogString(fpLog,
				_T("\tFeatureRemovableMedium\n")
				_T("\t\t        Lockable: %s\n")
				_T("\t\tDefaultToPrevent: %s\n")
				_T("\t\t           Eject: %s\n")
				_T("\t\tLoadingMechanism: "),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x01),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x04),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x08));
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
					_T("Reserved: %08d\n"), pConf[uiSize+4+n] >> 5 & 0x07);
				break;
			}
			break;
		case FeatureWriteProtect:
			OutputLogString(fpLog,
				_T("\tFeatureWriteProtect\n")
				_T("\t\t               SupportsSWPPBit: %s\n")
				_T("\t\tSupportsPersistentWriteProtect: %s\n")
				_T("\t\t               WriteInhibitDCB: %s\n")
				_T("\t\t           DiscWriteProtectPAC: %s\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x01),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x02),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x04),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x08));
			break;
		case FeatureRandomReadable:
			lVal = MAKELONG(MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]), 
				MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n]));
			wVal = MAKEWORD(pConf[uiSize+9+n], pConf[uiSize+8+n]);
			OutputLogString(fpLog,
				_T("\tFeatureRandomReadable\n")
				_T("\t\t        LogicalBlockSize: %d\n")
				_T("\t\t                Blocking: %d\n")
				_T("\t\tErrorRecoveryPagePresent: %s\n"),
				lVal,
				wVal,
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+10+n] & 0x01));
			break;
		case FeatureMultiRead:
			OutputLogString(fpLog, _T("\tFeatureMultiRead\n"));
			break;
		case FeatureCdRead:
			OutputLogString(fpLog,
				_T("\tFeatureCdRead\n")
				_T("\t\t          CDText: %s\n")
				_T("\t\t     C2ErrorData: %s\n")
				_T("\t\tDigitalAudioPlay: %s\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x01),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x02),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x80));
			pDiscData->bCanCDText = (BOOL)(pConf[uiSize+4+n] & 0x01);
			pDiscData->bC2ErrorData = (BOOL)((pConf[uiSize+4+n] & 0x02) >> 1);
			break;
		case FeatureDvdRead:
			OutputLogString(fpLog,
				_T("\tFeatureDvdRead\n")
				_T("\t\t  Multi110: %s\n")
				_T("\t\t DualDashR: %s\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x01),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+6+n] & 0x01));
			break;
		case FeatureRandomWritable:
			OutputLogString(fpLog,
				_T("\tFeatureRandomWritable\n")
				_T("\t\t                 LastLBA: %d\n")
				_T("\t\t        LogicalBlockSize: %d\n")
				_T("\t\t                Blocking: %d\n")
				_T("\t\tErrorRecoveryPagePresent: %s\n"),
				MAKELONG(MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]), 
					MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n])),
				MAKELONG(MAKEWORD(pConf[uiSize+11+n], pConf[uiSize+10+n]), 
					MAKEWORD(pConf[uiSize+9+n], pConf[uiSize+8+n])),
				MAKEWORD(pConf[uiSize+13+n], pConf[uiSize+12+n]),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+14+n] & 0x01));
			break;
		case FeatureIncrementalStreamingWritable:
			OutputLogString(fpLog,
				_T("\tFeatureIncrementalStreamingWritable\n")
				_T("\t\t        DataTypeSupported: %s\n")
				_T("\t\t       BufferUnderrunFree: %s\n")
				_T("\t\t   AddressModeReservation: %s\n")
				_T("\t\tTrackRessourceInformation: %s\n")
				_T("\t\t        NumberOfLinkSizes: %d\n"),
				BOOLEAN_TO_STRING_YES_NO(MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n])),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+6+n] & 0x01),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+6+n] & 0x02),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+6+n] & 0x04),
				pConf[uiSize+7+n]
				);
			for(INT i = 1; i <= pConf[uiSize+7+n]; i++) {
				OutputLogString(fpLog, 
					_T("\t\tLinkSize%d: %d\n"), i, pConf[uiSize+7+i+n]);
			}
			break;
		case FeatureSectorErasable:
			OutputLogString(fpLog, _T("\tFeatureSectorErasable\n"));
			break;
		case FeatureFormattable:
			OutputLogString(fpLog,
				_T("\tFeatureFormattable\n")
				_T("\t\t FullCertification: %s\n")
				_T("\t\tQuickCertification: %s\n")
				_T("\t\tSpareAreaExpansion: %s\n")
				_T("\t\tRENoSpareAllocated: %s\n")
				_T("\t\t   RRandomWritable: %s\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x01),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x02),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x04),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x08),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+8+n] & 0x01));
			break;
		case FeatureDefectManagement:
			OutputLogString(fpLog,
				_T("\tFeatureDefectManagement\n")
				_T("\t\tSupplimentalSpareArea: %s\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x80));
			break;
		case FeatureWriteOnce:
			OutputLogString(fpLog,
				_T("\tFeatureWriteOnce\n")
				_T("\t\t        LogicalBlockSize: %d\n")
				_T("\t\t                Blocking: %d\n")
				_T("\t\tErrorRecoveryPagePresent: %s\n"),
				MAKELONG(MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]), 
					MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n])),
				MAKEWORD(pConf[uiSize+9+n], pConf[uiSize+8+n]),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+10+n] & 0x01));
			break;
		case FeatureRestrictedOverwrite:
			OutputLogString(fpLog, _T("\tFeatureRestrictedOverwrite\n"));
			break;
		case FeatureCdrwCAVWrite:
			OutputLogString(fpLog, _T("\tFeatureCdrwCAVWrite\n"));
			break;
		case FeatureMrw:
			OutputLogString(fpLog,
				_T("\tFeatureMrw\n")
				_T("\t\t       Write: %s\n")
				_T("\t\t DvdPlusRead: %s\n")
				_T("\t\tDvdPlusWrite: %s\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x01),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x02),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x04));
			break;
		case FeatureEnhancedDefectReporting:
			OutputLogString(fpLog,
				_T("\tFeatureEnhancedDefectReporting\n")
				_T("\t\t       DRTDMSupported: %s\n")
				_T("\t\tNumberOfDBICacheZones: %d\n")
				_T("\t\t      NumberOfEntries: %d\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x01),
				pConf[uiSize+5+n],
				MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]));
			break;
		case FeatureDvdPlusRW:
			OutputLogString(fpLog,
				_T("\tFeatureDvdPlusRW\n")
				_T("\t\t     Write: %s\n")
				_T("\t\t CloseOnly: %s\n")
				_T("\t\tQuickStart: %s\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x01),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+5+n] & 0x01),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+5+n] & 0x02));
			break;
		case FeatureDvdPlusR:
			OutputLogString(fpLog,
				_T("\tFeatureDvdPlusR\n")
				_T("\t\tWrite: %s\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x01));
			break;
		case FeatureRigidRestrictedOverwrite:
			OutputLogString(fpLog,
				_T("\tFeatureRigidRestrictedOverwrite\n")
				_T("\t\t                   Blank: %s\n")
				_T("\t\t            Intermediate: %s\n")
				_T("\t\t    DefectStatusDataRead: %s\n")
				_T("\t\tDefectStatusDataGenerate: %s\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x01),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x02),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x04),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x08));
			break;
		case FeatureCdTrackAtOnce:
			OutputLogString(fpLog,
				_T("\tFeatureCdTrackAtOnce\n")
				_T("\t\tRWSubchannelsRecordable: %s\n")
				_T("\t\t           CdRewritable: %s\n")
				_T("\t\t            TestWriteOk: %s\n")
				_T("\t\t   RWSubchannelPackedOk: %s\n")
				_T("\t\t      RWSubchannelRawOk: %s\n")
				_T("\t\t     BufferUnderrunFree: %s\n")
				_T("\t\t      DataTypeSupported: %s\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x01),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x02),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x04),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x08),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x10),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x40),
				BOOLEAN_TO_STRING_YES_NO(MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n])));
			break;
		case FeatureCdMastering:
			OutputLogString(fpLog,
				_T("\tFeatureCdMastering\n")
				_T("\t\tRWSubchannelsRecordable: %s\n")
				_T("\t\t           CdRewritable: %s\n")
				_T("\t\t            TestWriteOk: %s\n")
				_T("\t\t        RRawRecordingOk: %s\n")
				_T("\t\t      RawMultiSessionOk: %s\n")
				_T("\t\t        SessionAtOnceOk: %s\n")
				_T("\t\t     BufferUnderrunFree: %s\n")
				_T("\t\t  MaximumCueSheetLength: %d\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x01),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x02),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x04),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x08),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x10),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x20),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x40),
				MAKELONG(MAKEWORD(0, pConf[uiSize+7+n]), 
					MAKEWORD(pConf[uiSize+6+n], pConf[uiSize+5+n]))
				);
			break;
		case FeatureDvdRecordableWrite:
			OutputLogString(fpLog,
				_T("\tFeatureDvdRecordableWrite\n")
				_T("\t\t            DVD_RW: %s\n")
				_T("\t\t         TestWrite: %s\n")
				_T("\t\t        RDualLayer: %s\n")
				_T("\t\tBufferUnderrunFree: %s\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x02),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x04),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x08),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x40));
			break;
		case FeatureLayerJumpRecording:
			OutputLogString(fpLog,
				_T("\tFeatureLayerJumpRecording\n")
				_T("\t\tNumberOfLinkSizes: %d\n"),
				pConf[uiSize+7+n]);
			for(INT i = 1; i <= pConf[uiSize+7+n]; i++) {
				OutputLogString(fpLog, 
					_T("\t\tLinkSize%d: %d\n"), i, pConf[uiSize+7+i+n]);
			}
			break;
		case FeatureCDRWMediaWriteSupport:
			OutputLogString(fpLog, _T(
				"\tFeatureCDRWMediaWriteSupport\n"));
			for(UINT i = 1, a = 0; i < 0x100; i<<=1, a++) { 
				OutputLogString(fpLog, _T(
					"\t\tSubtype %d: %s\n"),
				a, BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+5+n] & i));
			}
			break;
		case FeatureBDRPseudoOverwrite:
			OutputLogString(fpLog, _T("\tFeatureBDRPseudoOverwrite\n"));
			break;
		case FeatureDvdPlusRWDualLayer:
			OutputLogString(fpLog,
				_T("\tFeatureDvdPlusRWDualLayer\n")
				_T("\t\t     Write: %s\n")
				_T("\t\t CloseOnly: %s\n")
				_T("\t\tQuickStart: %s\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x01),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+5+n] & 0x01),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+5+n] & 0x02));
			break;
		case FeatureDvdPlusRDualLayer:
			OutputLogString(fpLog, _T("\tFeatureDvdPlusRDualLayer\n"));
			OutputLogString(fpLog, _T("\t\tWrite: %s\n"), 
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x01));
			break;
		case FeatureBDRead:
			// TODO
			OutputLogString(fpLog, _T("\tFeatureBDRead\n"));
			break;
		case FeatureBDWrite:
			// TODO
			OutputLogString(fpLog, _T("\tFeatureBDWrite\n"));
			break;
		case FeatureTSR:
			OutputLogString(fpLog, _T("\tFeatureTSR\n"));
			break;
		case FeatureHDDVDRead:
			// TODO
			OutputLogString(fpLog, _T("\tFeatureHDDVDRead\n"));
			break;
		case FeatureHDDVDWrite:
			// TODO
			OutputLogString(fpLog, _T("\tFeatureHDDVDWrite\n"));
			break;
		case FeatureHybridDisc:
			OutputLogString(fpLog, 
				_T("\tFeatureHybridDisc\n")
				_T("\t\tResetImmunity: %s\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x01));
			break;
		case FeaturePowerManagement:
			OutputLogString(fpLog, _T("\tFeaturePowerManagement\n"));
			break;
		case FeatureSMART:
			OutputLogString(fpLog,
				_T("\tFeatureSMART\n")
				_T("\t\tFaultFailureReportingPagePresent: %s\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x01));
			break;
		case FeatureEmbeddedChanger:
			OutputLogString(fpLog,
				_T("\tFeatureEmbeddedChanger\n")
				_T("\t\tSupportsDiscPresent: %s\n")
				_T("\t\t  SideChangeCapable: %s\n")
				_T("\t\t  HighestSlotNumber: %d\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x04),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x10),
				pConf[uiSize+7+n] & 0x1F);
			break;
		case FeatureCDAudioAnalogPlay:
			OutputLogString(fpLog,
				_T("\tFeatureCDAudioAnalogPlay\n")
				_T("\t\t     SeperateVolume: %s\n")
				_T("\t\tSeperateChannelMute: %s\n")
				_T("\t\t      ScanSupported: %s\n")
				_T("\t\tNumerOfVolumeLevels: %d\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x01),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x02),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x04),
				MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]));
			break;
		case FeatureMicrocodeUpgrade:
			OutputLogString(fpLog,
				_T("\tFeatureMicrocodeUpgrade\n")
				_T("\t\tM5: %s\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x01));
			break;
		case FeatureTimeout:
			OutputLogString(fpLog,
				_T("\tFeatureTimeout\n")
				_T("\t\t    Group3: %s\n")
				_T("\t\tUnitLength: %d\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x01),
				MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]));
			break;
		case FeatureDvdCSS:
			OutputLogString(fpLog,
				_T("\tFeatureDvdCSS\n")
				_T("\t\tCssVersion: %d\n"),
				pConf[uiSize+7+n]);
			break;
		case FeatureRealTimeStreaming:
			OutputLogString(fpLog,
				_T("\tFeatureRealTimeStreaming\n")
				_T("\t\t        StreamRecording: %s\n")
				_T("\t\t    WriteSpeedInGetPerf: %s\n")
				_T("\t\t       WriteSpeedInMP2A: %s\n")
				_T("\t\t             SetCDSpeed: %s\n")
				_T("\t\tReadBufferCapacityBlock: %s\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x01),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x02),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x04),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x08),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x10));
			break;
		case FeatureLogicalUnitSerialNumber:
			OutputLogString(fpLog,
				_T("\tFeatureLogicalUnitSerialNumber\n")
				_T("\t\tSerialNumber: "));
			for(INT i = 0; i < pConf[uiSize+3+n]; i++) {
				OutputLogString(fpLog, _T("%c"), pConf[uiSize+4+i+n]);
			}
			OutputLogString(fpLog, _T("\n"));
			break;
		case FeatureMediaSerialNumber:
			OutputLogString(fpLog, _T("\tFeatureMediaSerialNumber\n"));
			break;
		case FeatureDiscControlBlocks:
			OutputLogString(fpLog, _T("\tFeatureDiscControlBlocks\n"));
			for(INT i = 0; i < pConf[uiSize+3+n]; i+=4) {
				OutputLogString(fpLog, _T(
					"\t\tContentDescriptor %02d: %08d\n"), i / 4, 
					MAKELONG(MAKEWORD(pConf[uiSize+7+i+n], pConf[uiSize+6+i+n]), 
						MAKEWORD(pConf[uiSize+5+i+n], pConf[uiSize+4+i+n])));
			}
			break;
		case FeatureDvdCPRM:
			OutputLogString(fpLog,
				_T("\tFeatureDvdCPRM\n")
				_T("\t\tCPRMVersion: %d\n"),
				pConf[uiSize+7+n]);
			break;
		case FeatureFirmwareDate:
			OutputLogString(fpLog, _T(
				"\tFeatureFirmwareDate: %04d-%02d-%02d %02d:%02d:%02d\n"),
				MAKELONG(MAKEWORD(pConf[uiSize+7+n], pConf[uiSize+6+n]), 
					MAKEWORD(pConf[uiSize+5+n], pConf[uiSize+4+n])),
				MAKEWORD(pConf[uiSize+9+n], pConf[uiSize+8+n]),
				MAKEWORD(pConf[uiSize+11+n], pConf[uiSize+10+n]),
				MAKEWORD(pConf[uiSize+13+n], pConf[uiSize+12+n]),
				MAKEWORD(pConf[uiSize+15+n], pConf[uiSize+14+n]),
				MAKEWORD(pConf[uiSize+17+n], pConf[uiSize+16+n]));
			break;
		case FeatureAACS:
			OutputLogString(fpLog,
				_T("\tFeatureAACS\n")
				_T("\t\tBindingNonceGeneration: %s\n")
				_T("\t\tBindingNonceBlockCount: %d\n")
				_T("\t\t         NumberOfAGIDs: %d\n")
				_T("\t\t           AACSVersion: %d\n"),
				BOOLEAN_TO_STRING_YES_NO(pConf[uiSize+4+n] & 0x01),
				pConf[uiSize+5+n],
				pConf[uiSize+6+n] & 0x0F,
				pConf[uiSize+7+n]);
			break;
		case FeatureVCPS:
			OutputLogString(fpLog, _T("\tFeatureVCPS\n"));
			break;
		default:
			if(0xFF00 <= nCode && nCode <= 0xFFFF) {
				OutputLogString(fpLog,
					_T("\tVendor Specific. FeatureCode[0x%04X]\n")
					_T("\t\tVendorSpecificData: "), nCode);
			}
			else {
				OutputLogString(fpLog,
					_T("\tReserved. FeatureCode[0x%04X]\n")
					_T("\t\tData: "), nCode);
			}
			for(INT i = 0; i < pConf[uiSize+3+n]; i++) {
				OutputLogString(fpLog, _T("%02x"), pConf[uiSize+4+i+n]);
			}
			OutputLogString(fpLog, _T("\n"));
			break;
		}
		if(nCode != FeatureProfileList) {
			n += pConf[uiSize+3+n] + sizeof(FEATURE_HEADER);
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
			OutputLogString(fpLog, _T("Reserved [%x]"), usFeatureProfileType);
			break;
	}
}

void OutputParsingSubfile(
	LPCTSTR pszSubfile
	)
{
	FILE* fpSub = CreateOrOpenFileW(pszSubfile, NULL, NULL, NULL, _T(".sub"), _T("rb"), 0, 0);
	FILE* fpParse = CreateOrOpenFileW(pszSubfile, NULL, NULL, NULL, _T(".sub.txt"), _T(WFLAG), 0, 0);
	if (!fpSub || !fpParse) {
		OutputErrorString(_T("Failed to open file .sub [F:%s][L: %d]"), 
			_T(__FUNCTION__), __LINE__);
		return;
	}
	ULONG datasize = GetFilesize(fpSub, 0);
	PUCHAR data = (PUCHAR)malloc(datasize);
	if(!data) {
		OutputErrorString(_T("Cannot alloc memory [F:%s][L: %d]\n"), 
			_T(__FUNCTION__), __LINE__);
		return;
	}
	fread(data, sizeof(char), datasize, fpSub);
	FcloseAndNull(fpSub);

	// TODO:RtoW don't use in present
	UCHAR SubcodeRtoW[CD_RAW_READ_SUBCODE_SIZE] = {0};
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

void OutputScsiStatus(
	CONST PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER swb,
	PUCHAR byScsiStatus,
	LPCTSTR pszFuncname,
	INT nLineNum
	)
{
	_INT aScsiStatus[][2] = {
		{SCSISTAT_GOOD, (_INT)_T("GOOD")}, 
		{SCSISTAT_CHECK_CONDITION, (_INT)_T("CHECK_CONDITION")}, 
		{SCSISTAT_CONDITION_MET, (_INT)_T("CONDITION_MET")}, 
		{SCSISTAT_BUSY, (_INT)_T("BUSY")}, 
		{SCSISTAT_INTERMEDIATE, (_INT)_T("INTERMEDIATE")}, 
		{SCSISTAT_INTERMEDIATE_COND_MET, (_INT)_T("INTERMEDIATE_COND_MET")}, 
		{SCSISTAT_RESERVATION_CONFLICT, (_INT)_T("RESERVATION_CONFLICT")}, 
		{SCSISTAT_COMMAND_TERMINATED, (_INT)_T("COMMAND_TERMINATED")}, 
		{SCSISTAT_QUEUE_FULL, (_INT)_T("QUEUE_FULL")}
	};
	if(swb->ScsiPassThroughDirect.ScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		UCHAR key = (UCHAR)(swb->SenseInfoBuffer[2] & 0x0F);
		UCHAR ASC = swb->SenseInfoBuffer[12];
		UCHAR ASCQ = swb->SenseInfoBuffer[13];
		if(key != SCSI_SENSE_NO_SENSE || ASC != SCSI_ADSENSE_NO_SENSE || ASCQ != 0x00) {
			for(INT i = 0; i < sizeof(aScsiStatus) / sizeof(INT) / 2; i++) {
				if(swb->ScsiPassThroughDirect.ScsiStatus == aScsiStatus[i][0]) {
					OutputErrorString(
						_T("\nSCSI bus status codes:%02x-%s [F:%s][L: %d]\n"), 
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
	_INT aSenseKey[][2] = {
		{SCSI_SENSE_NO_SENSE, (_INT)_T("NO_SENSE")},
		{SCSI_SENSE_RECOVERED_ERROR, (_INT)_T("RECOVERED_ERROR")}, 
		{SCSI_SENSE_NOT_READY, (_INT)_T("NOT_READY")}, 
		{SCSI_SENSE_MEDIUM_ERROR, (_INT)_T("MEDIUM_ERROR")}, 
		{SCSI_SENSE_HARDWARE_ERROR, (_INT)_T("HARDWARE_ERROR")}, 
		{SCSI_SENSE_ILLEGAL_REQUEST, (_INT)_T("ILLEGAL_REQUEST")}, 
		{SCSI_SENSE_UNIT_ATTENTION, (_INT)_T("UNIT_ATTENTION")}, 
		{SCSI_SENSE_DATA_PROTECT, (_INT)_T("DATA_PROTECT")}, 
		{SCSI_SENSE_BLANK_CHECK, (_INT)_T("BLANK_CHECK")}, 
		{SCSI_SENSE_UNIQUE, (_INT)_T("UNIQUE")}, 
		{SCSI_SENSE_COPY_ABORTED, (_INT)_T("COPY_ABORTED")}, 
		{SCSI_SENSE_ABORTED_COMMAND, (_INT)_T("ABORTED_COMMAND")}, 
		{SCSI_SENSE_EQUAL, (_INT)_T("EQUAL")}, 
		{SCSI_SENSE_VOL_OVERFLOW, (_INT)_T("VOL_OVERFLOW")}, 
		{SCSI_SENSE_MISCOMPARE, (_INT)_T("MISCOMPARE")} 
	};
	// only C/DVD Device (MMC-6)
	_INT aSenseAscAscq[][3] = {
		{SCSI_ADSENSE_NO_SENSE, 0x00, (_INT)_T("NO SENSE")},
		{SCSI_ADSENSE_NO_SEEK_COMPLETE, 0x00, (_INT)_T("NO SEEK COMPLETE")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_CAUSE_NOT_REPORTABLE, (_INT)_T("LUN_NOT_READY - CAUSE_NOT_REPORTABLE")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_BECOMING_READY, (_INT)_T("LUN_NOT_READY - BECOMING_READY")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_INIT_COMMAND_REQUIRED, (_INT)_T("LUN_NOT_READY - INIT_COMMAND_REQUIRED")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_MANUAL_INTERVENTION_REQUIRED, (_INT)_T("LUN_NOT_READY - MANUAL_INTERVENTION_REQUIRED")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_FORMAT_IN_PROGRESS, (_INT)_T("LUN_NOT_READY - FORMAT_IN_PROGRESS")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_OPERATION_IN_PROGRESS, (_INT)_T("LUN_NOT_READY - OPERATION_IN_PROGRESS")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_LONG_WRITE_IN_PROGRESS, (_INT)_T("LUN_NOT_READY - LONG_WRITE_IN_PROGRESS")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x09, (_INT)_T("LUN_NOT_READY - SELF-TEST IN PROGRESS")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x0A, (_INT)_T("LUN_NOT_READY - ASYMMETRIC ACCESS STATE TRANSITION")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x0B, (_INT)_T("LUN_NOT_READY - TARGET PORT IN STANDBY STATE")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x0C, (_INT)_T("LUN_NOT_READY - TARGET PORT IN UNAVAILABLE STATE")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x10, (_INT)_T("LUN_NOT_READY - AUXILIARY MEMORY NOT ACCESSIBLE")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x11, (_INT)_T("LUN_NOT_READY - NOTIFY (ENABLE SPINUP) REQUIRED")}, 
		{SCSI_ADSENSE_LUN_NOT_READY, 0x13, (_INT)_T("LUN_NOT_READY - SA CREATION IN PROGRESS")}, 
		{0x05, 0x00, (_INT)_T("LOGICAL UNIT DOES NOT RESPOND TO SELECTION")}, 
		{0x06, 0x00, (_INT)_T("NO REFERENCE POSITION FOUND")}, 
		{0x07, 0x00, (_INT)_T("MULTIPLE PERIPHERAL DEVICES SELECTED")}, 
		{SCSI_ADSENSE_LUN_COMMUNICATION, SCSI_SENSEQ_COMM_FAILURE, (_INT)_T("LUN_COMMUNICATION - COMM_FAILURE")}, 
		{SCSI_ADSENSE_LUN_COMMUNICATION, SCSI_SENSEQ_COMM_TIMEOUT, (_INT)_T("LUN_COMMUNICATION - COMM_TIMEOUT")}, 
		{SCSI_ADSENSE_LUN_COMMUNICATION, SCSI_SENSEQ_COMM_PARITY_ERROR, (_INT)_T("LUN_COMMUNICATION - COMM_PARITY_ERROR")}, 
		{SCSI_ADSENSE_LUN_COMMUNICATION, SCSI_SESNEQ_COMM_CRC_ERROR, (_INT)_T("LUN_COMMUNICATION - COMM_CRC_ERROR")}, 
		{SCSI_ADSENSE_LUN_COMMUNICATION, SCSI_SENSEQ_UNREACHABLE_TARGET, (_INT)_T("LUN_COMMUNICATION - UNREACHABLE_TARGET")}, 
		{0x09, 0x00, (_INT)_T("TRACK FOLLOWING ERROR")}, 
		{0x09, 0x01, (_INT)_T("TRACKING SERVO FAILURE")}, 
		{0x09, 0x02, (_INT)_T("FOCUS SERVO FAILURE")}, 
		{0x09, 0x03, (_INT)_T("SPINDLE SERVO FAILURE")}, 
		{0x09, 0x04, (_INT)_T("HEAD SELECT FAULT")}, 
		{0x0A, 0x00, (_INT)_T("ERROR LOG OVERFLOW")}, 
		{0x0B, 0x00, (_INT)_T("WARNING")}, 
		{0x0B, 0x01, (_INT)_T("WARNING - SPECIFIED TEMPERATURE EXCEEDED")}, 
		{0x0B, 0x02, (_INT)_T("WARNING - ENCLOSURE DEGRADED")}, 
		{0x0B, 0x03, (_INT)_T("WARNING - BACKGROUND SELF-TEST FAILED")}, 
		{0x0B, 0x04, (_INT)_T("WARNING - BACKGROUND PRE-SCAN DETECTED MEDIUM ERROR")}, 
		{0x0B, 0x05, (_INT)_T("WARNING - BACKGROUND MEDIUM SCAN DETECTED MEDIUM ERROR")}, 
		{0x0B, 0x06, (_INT)_T("WARNING - NON-VOLATILE CACHE NOW VOLATILE")}, 
		{0x0B, 0x07, (_INT)_T("WARNING - DEGRADED POWER TO NON-VOLATILE CACHE")}, 
		{0x0B, 0x08, (_INT)_T("WARNING - POWER LOSS EXPECTED")}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x00, (_INT)_T("WRITE ERROR")}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x06, (_INT)_T("BLOCK NOT COMPRESSIBLE")}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x07, (_INT)_T("WRITE ERROR - RECOVERY NEEDED")}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x08, (_INT)_T("WRITE ERROR - RECOVERY FAILED")}, 
		{SCSI_ADSENSE_WRITE_ERROR, SCSI_SENSEQ_LOSS_OF_STREAMING, (_INT)_T("WRITE ERROR - LOSS OF STREAMING")}, 
		{SCSI_ADSENSE_WRITE_ERROR, SCSI_SENSEQ_PADDING_BLOCKS_ADDED, (_INT)_T("WRITE ERROR - PADDING BLOCKS ADDED")}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x0B, (_INT)_T("AUXILIARY MEMORY WRITE ERROR")}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x0C, (_INT)_T("WRITE ERROR - UNEXPECTED UNSOLICITED DATA")}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x0D, (_INT)_T("WRITE ERROR - NOT ENOUGH UNSOLICITED DATA")}, 
		{SCSI_ADSENSE_WRITE_ERROR, 0x0F, (_INT)_T("DEFECTS IN ERROR WINDOW")}, 
		{0x0D, 0x00, (_INT)_T("ERROR DETECTED BY THIRD PARTY TEMPORARY INITIATOR")}, 
		{0x0D, 0x01, (_INT)_T("THIRD PARTY DEVICE FAILURE")}, 
		{0x0D, 0x02, (_INT)_T("COPY TARGET DEVICE NOT REACHABLE")}, 
		{0x0D, 0x03, (_INT)_T("INCORRECT COPY TARGET DEVICE TYPE")}, 
		{0x0D, 0x04, (_INT)_T("COPY TARGET DEVICE DATA UNDERRUN")}, 
		{0x0D, 0x05, (_INT)_T("COPY TARGET DEVICE DATA OVERRUN")}, 
		{0x0E, 0x00, (_INT)_T("INVALID INFORMATION UNIT")}, 
		{0x0E, 0x01, (_INT)_T("INFORMATION UNIT TOO SHORT")}, 
		{0x0E, 0x02, (_INT)_T("INFORMATION UNIT TOO LONG")}, 
		{0x0E, 0x03, (_INT)_T("INVALID FIELD IN COMMAND INFORMATION UNIT")}, 
		{0x11, 0x00, (_INT)_T("UNRECOVERED READ ERROR")}, 
		{0x11, 0x01, (_INT)_T("READ RETRIES EXHAUSTED")}, 
		{0x11, 0x02, (_INT)_T("ERROR TOO LONG TO CORRECT")}, 
		{0x11, 0x05, (_INT)_T("L-EC UNCORRECTABLE ERROR")}, 
		{0x11, 0x06, (_INT)_T("CIRC UNRECOVERED ERROR")}, 
		{0x11, 0x0D, (_INT)_T("DE-COMPRESSION CRC ERROR")}, 
		{0x11, 0x0E, (_INT)_T("CANNOT DECOMPRESS USING DECLARED ALGORITHM")}, 
		{0x11, 0x0F, (_INT)_T("ERROR READING UPC/EAN NUMBER")}, 
		{0x11, 0x10, (_INT)_T("ERROR READING ISRC NUMBER")}, 
		{0x11, 0x11, (_INT)_T("READ ERROR - LOSS OF STREAMING")}, 
		{0x11, 0x12, (_INT)_T("AUXILIARY MEMORY READ ERROR")}, 
		{0x11, 0x13, (_INT)_T("READ ERROR - FAILED RETRANSMISSION REQUEST")}, 
		{SCSI_ADSENSE_TRACK_ERROR, 0x00, (_INT)_T("TRACK_ERROR - RECORDED ENTITY NOT FOUND")}, 
		{SCSI_ADSENSE_TRACK_ERROR, 0x01, (_INT)_T("TRACK_ERROR - RECORD NOT FOUND")}, 
		{SCSI_ADSENSE_SEEK_ERROR, 0x00, (_INT)_T("SEEK_ERROR - RANDOM POSITIONING ERROR")}, 
		{SCSI_ADSENSE_SEEK_ERROR, 0x01, (_INT)_T("SEEK_ERROR - MECHANICAL POSITIONING ERROR")}, 
		{SCSI_ADSENSE_SEEK_ERROR, 0x02, (_INT)_T("SEEK_ERROR - POSITIONING ERROR DETECTED BY READ OF MEDIUM")}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x00, (_INT)_T("REC_DATA_NOECC - RECOVERED DATA WITH NO ERROR CORRECTION APPLIED")}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x01, (_INT)_T("REC_DATA_NOECC - RECOVERED DATA WITH RETRIES")}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x02, (_INT)_T("REC_DATA_NOECC - RECOVERED DATA WITH POSITIVE HEAD OFFSET")}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x03, (_INT)_T("REC_DATA_NOECC - RECOVERED DATA WITH NEGATIVE HEAD OFFSET")}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x04, (_INT)_T("REC_DATA_NOECC - RECOVERED DATA WITH RETRIES AND/OR CIRC APPLIED")}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x05, (_INT)_T("REC_DATA_NOECC - RECOVERED DATA USING PREVIOUS SECTOR ID")}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x07, (_INT)_T("REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - RECOMMEND REASSIGNMENT")}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x08, (_INT)_T("REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - RECOMMEND REWRITE")}, 
		{SCSI_ADSENSE_REC_DATA_NOECC, 0x09, (_INT)_T("REC_DATA_NOECC - RECOVERED DATA WITHOUT ECC - DATA REWRITTEN")}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x00, (_INT)_T("REC_DATA_ECC - RECOVERED DATA WITH ERROR CORRECTION APPLIED")}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x01, (_INT)_T("REC_DATA_ECC - RECOVERED DATA WITH ERROR CORR. & RETRIES APPLIED")}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x02, (_INT)_T("REC_DATA_ECC - RECOVERED DATA - DATA AUTO-REALLOCATED")}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x03, (_INT)_T("REC_DATA_ECC - RECOVERED DATA WITH CIRC")}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x04, (_INT)_T("REC_DATA_ECC - RECOVERED DATA WITH L-EC")}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x05, (_INT)_T("REC_DATA_ECC - RECOVERED DATA - RECOMMEND REASSIGNMENT")}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x06, (_INT)_T("REC_DATA_ECC - RECOVERED DATA - RECOMMEND REWRITE")}, 
		{SCSI_ADSENSE_REC_DATA_ECC, 0x08, (_INT)_T("REC_DATA_ECC - RECOVERED DATA WITH LINKING")}, 
		{SCSI_ADSENSE_PARAMETER_LIST_LENGTH, 0x00, (_INT)_T("PARAMETER LIST LENGTH ERROR")}, 
		{0x1B, 0x00, (_INT)_T("SYNCHRONOUS DATA TRANSFER ERROR")}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x00, (_INT)_T("INVALID COMMAND OPERATION CODE")}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x01, (_INT)_T("ILLEGAL_COMMAND - ACCESS DENIED - INITIATOR PENDING-ENROLLED")}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x02, (_INT)_T("ILLEGAL_COMMAND - ACCESS DENIED - NO ACCESS RIGHTS")}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x03, (_INT)_T("ILLEGAL_COMMAND - ACCESS DENIED - INVALID MGMT ID KEY")}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x08, (_INT)_T("ILLEGAL_COMMAND - ACCESS DENIED - ENROLLMENT CONFLICT")}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x09, (_INT)_T("ILLEGAL_COMMAND - ACCESS DENIED - INVALID LU IDENTIFIER")}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x0A, (_INT)_T("ILLEGAL_COMMAND - ACCESS DENIED - INVALID PROXY TOKEN")}, 
		{SCSI_ADSENSE_ILLEGAL_COMMAND, 0x0B, (_INT)_T("ILLEGAL_COMMAND - ACCESS DENIED - ACL LUN CONFLICT")}, 
		{SCSI_ADSENSE_ILLEGAL_BLOCK, 0x00, (_INT)_T("LOGICAL BLOCK ADDRESS OUT OF RANGE")}, 
		{SCSI_ADSENSE_ILLEGAL_BLOCK, SCSI_SENSEQ_ILLEGAL_ELEMENT_ADDR, (_INT)_T("ILLEGAL_BLOCK - MISCOMPARE DURING VERIFY OPERATION")}, 
		{SCSI_ADSENSE_ILLEGAL_BLOCK, 0x02, (_INT)_T("ILLEGAL_BLOCK - INVALID ADDRESS FOR WRITE")}, 
		{SCSI_ADSENSE_ILLEGAL_BLOCK, 0x03, (_INT)_T("ILLEGAL_BLOCK - INVALID WRITE CROSSING LAYER JUMP")}, 
		{0x23, 0x00, (_INT)_T("INVALID TOKEN OPERATION, CAUSE NOT REPORTABLE")}, 
		{SCSI_ADSENSE_INVALID_CDB, 0x00, (_INT)_T("INVALID FIELD IN CDB")}, 
		{SCSI_ADSENSE_INVALID_CDB, 0x01, (_INT)_T("CDB DECRYPTION ERROR")}, 
		{SCSI_ADSENSE_INVALID_CDB, 0x08, (_INT)_T("INVALID XCDB")}, 
		{SCSI_ADSENSE_INVALID_LUN, 0x00, (_INT)_T("LOGICAL UNIT NOT SUPPORTED")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x00, (_INT)_T("INVALID FIELD IN PARAMETER LIST")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x01, (_INT)_T("PARAMETER NOT SUPPORTED")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x02, (_INT)_T("PARAMETER VALUE INVALID")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x03, (_INT)_T("THRESHOLD PARAMETERS NOT SUPPORTED")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x04, (_INT)_T("INVALID RELEASE OF PERSISTENT RESERVATION")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x05, (_INT)_T("DATA DECRYPTION ERROR")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x06, (_INT)_T("TOO MANY TARGET DESCRIPTORS")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x07, (_INT)_T("UNSUPPORTED SEGMENT DESCRIPTOR TYPE CODE")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x08, (_INT)_T("TOO MANY SEGMENT DESCRIPTORS")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x09, (_INT)_T("UNSUPPORTED SEGMENT DESCRIPTOR TYPE CODE")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x0A, (_INT)_T("UNEXPECTED INEXACT SEGMENT")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x0B, (_INT)_T("INLINE DATA LENGTH EXCEEDED")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x0C, (_INT)_T("INVALID OPERATION FOR COPY SOURCE OR DESTINATION")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x0D, (_INT)_T("COPY SEGMENT GRANULARITY VIOLATION")}, 
		{SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST, 0x0E, (_INT)_T("INVALID PARAMETER WHILE PORT IS ENABLED")}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x00, (_INT)_T("WRITE PROTECTED")}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x01, (_INT)_T("HARDWARE WRITE PROTECTED")}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x02, (_INT)_T("LOGICAL UNIT SOFTWARE WRITE PROTECTED")}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x03, (_INT)_T("ASSOCIATED WRITE PROTECT")}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x04, (_INT)_T("PERSISTENT WRITE PROTECT")}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x05, (_INT)_T("PERMANENT WRITE PROTECT")}, 
		{SCSI_ADSENSE_WRITE_PROTECT, 0x06, (_INT)_T("CONDITIONAL WRITE PROTECT")}, 
		{SCSI_ADSENSE_MEDIUM_CHANGED, 0x00, (_INT)_T("NOT READY TO READY CHANGE, MEDIUM MAY HAVE CHANGED")}, 
		{SCSI_ADSENSE_MEDIUM_CHANGED, 0x01, (_INT)_T("IMPORT OR EXPORT ELEMENT ACCESSED")}, 
		{SCSI_ADSENSE_MEDIUM_CHANGED, 0x02, (_INT)_T("FORMAT-LAYER MAY HAVE CHANGED")}, 
		{SCSI_ADSENSE_BUS_RESET, 0x00, (_INT)_T("POWER ON, RESET, OR BUS DEVICE RESET OCCURRED")}, 
		{SCSI_ADSENSE_BUS_RESET, 0x01, (_INT)_T("POWER ON OCCURRED")}, 
		{SCSI_ADSENSE_BUS_RESET, 0x02, (_INT)_T("SCSI BUS RESET OCCURRED")}, 
		{SCSI_ADSENSE_BUS_RESET, 0x03, (_INT)_T("BUS DEVICE RESET FUNCTION OCCURRED")}, 
		{SCSI_ADSENSE_BUS_RESET, 0x04, (_INT)_T("DEVICE INTERNAL RESET")}, 
		{SCSI_ADSENSE_BUS_RESET, 0x05, (_INT)_T("TRANSCEIVER MODE CHANGED TO SINGLE-ENDED")}, 
		{SCSI_ADSENSE_BUS_RESET, 0x06, (_INT)_T("TRANSCEIVER MODE CHANGED TO LVD")}, 
		{SCSI_ADSENSE_BUS_RESET, 0x07, (_INT)_T("I_T NEXUS LOSS OCCURRED")}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x00, (_INT)_T("PARAMETERS CHANGED")}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x01, (_INT)_T("MODE PARAMETERS CHANGED")}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x02, (_INT)_T("LOG PARAMETERS CHANGED")}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x03, (_INT)_T("RESERVATIONS PREEMPTED")}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x04, (_INT)_T("RESERVATIONS RELEASED")}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x05, (_INT)_T("REGISTRATIONS PREEMPTED")}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x06, (_INT)_T("ASYMMETRIC ACCESS STATE CHANGED")}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x07, (_INT)_T("IMPLICIT ASYMMETRIC ACCESS STATE TRANSITION FAILED")}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x08, (_INT)_T("PRIORITY CHANGED")}, 
		{SCSI_ADSENSE_PARAMETERS_CHANGED, 0x14, (_INT)_T("SA CREATION CAPABILITIES DATA HAS CHANGED")}, 
		{0x2B, 0x00, (_INT)_T("COPY CANNOT EXECUTE SINCE HOST CANNOT DISCONNECT")}, 
		{0x2C, 0x00, (_INT)_T("COMMAND SEQUENCE ERROR")}, 
		{0x2C, 0x03, (_INT)_T("CURRENT PROGRAM AREA IS NOT EMPTY")}, 
		{0x2C, 0x04, (_INT)_T("CURRENT PROGRAM AREA IS EMPTY")}, 
		{0x2C, 0x06, (_INT)_T("PERSISTENT PREVENT CONFLICT")}, 
		{0x2C, 0x07, (_INT)_T("PREVIOUS BUSY STATUS")}, 
		{0x2C, 0x08, (_INT)_T("PREVIOUS TASK SET FULL STATUS")}, 
		{0x2C, 0x09, (_INT)_T("PREVIOUS RESERVATION CONFLICT STATUS")}, 
		{SCSI_ADSENSE_INSUFFICIENT_TIME_FOR_OPERATION, 0x00, (_INT)_T("INSUFFICIENT TIME FOR OPERATION")}, 
		{0x2F, 0x00, (_INT)_T("COMMANDS CLEARED BY ANOTHER INITIATOR")}, 
		{0x2F, 0x02, (_INT)_T("COMMANDS CLEARED BY DEVICE SERVER")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x00, (_INT)_T("INCOMPATIBLE MEDIUM INSTALLED")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x01, (_INT)_T("CANNOT READ MEDIUM - UNKNOWN FORMAT")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x02, (_INT)_T("CANNOT READ MEDIUM - INCOMPATIBLE FORMAT")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x03, (_INT)_T("CLEANING CARTRIDGE INSTALLED")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x04, (_INT)_T("CANNOT WRITE MEDIUM - UNKNOWN FORMAT")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x05, (_INT)_T("CANNOT WRITE MEDIUM - INCOMPATIBLE FORMAT")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x06, (_INT)_T("CANNOT FORMAT MEDIUM - INCOMPATIBLE MEDIUM")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x07, (_INT)_T("CLEANING FAILURE")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x08, (_INT)_T("CANNOT WRITE - APPLICATION CODE MISMATCH")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x09, (_INT)_T("CURRENT SESSION NOT FIXATED FOR APPEND")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x0A, (_INT)_T("CLEANING REQUEST REJECTED")}, 
		{SCSI_ADSENSE_INVALID_MEDIA, 0x10, (_INT)_T("MEDIUM NOT FORMATTED")}, 
		{0x31, 0x00, (_INT)_T("MEDIUM FORMAT CORRUPTED")}, 
		{0x31, 0x01, (_INT)_T("FORMAT COMMAND FAILED")}, 
		{0x31, 0x02, (_INT)_T("ZONED FORMATTING FAILED DUE TO SPARE LINKING")}, 
		{0x34, 0x00, (_INT)_T("ENCLOSURE FAILURE")}, 
		{0x35, 0x00, (_INT)_T("ENCLOSURE SERVICES FAILURE")}, 
		{0x35, 0x01, (_INT)_T("UNSUPPORTED ENCLOSURE FUNCTION")}, 
		{0x35, 0x02, (_INT)_T("ENCLOSURE SERVICES UNAVAILABLE")}, 
		{0x35, 0x03, (_INT)_T("ENCLOSURE SERVICES TRANSFER FAILURE")}, 
		{0x35, 0x04, (_INT)_T("ENCLOSURE SERVICES TRANSFER REFUSED")}, 
		{0x35, 0x05, (_INT)_T("ENCLOSURE SERVICES CHECKSUM ERROR")}, 
		{0x37, 0x00, (_INT)_T("ROUNDED PARAMETER")}, 
		{0x39, 0x00, (_INT)_T("SAVING PARAMETERS NOT SUPPORTED")}, 
		{SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0x00, (_INT)_T("MEDIUM NOT PRESENT")}, 
		{SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0x01, (_INT)_T("MEDIUM NOT PRESENT - TRAY CLOSED")}, 
		{SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0x02, (_INT)_T("MEDIUM NOT PRESENT - TRAY OPEN")}, 
		{SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0x03, (_INT)_T("MEDIUM NOT PRESENT - LOADABLE")}, 
		{SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0x04, (_INT)_T("MEDIUM NOT PRESENT - MEDIUM AUXILIARY MEMORY ACCESSIBLE")}, 
		{SCSI_ADSENSE_POSITION_ERROR, SCSI_SENSEQ_DESTINATION_FULL, (_INT)_T("MEDIUM DESTINATION ELEMENT FULL")}, 
		{SCSI_ADSENSE_POSITION_ERROR, SCSI_SENSEQ_SOURCE_EMPTY, (_INT)_T("MEDIUM SOURCE ELEMENT EMPTY")}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x0F, (_INT)_T("END OF MEDIUM REACHED")}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x11, (_INT)_T("MEDIUM MAGAZINE NOT ACCESSIBLE")}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x12, (_INT)_T("MEDIUM MAGAZINE REMOVED")}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x13, (_INT)_T("MEDIUM MAGAZINE INSERTED")}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x14, (_INT)_T("MEDIUM MAGAZINE LOCKED")}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x15, (_INT)_T("MEDIUM MAGAZINE UNLOCKED")}, 
		{SCSI_ADSENSE_POSITION_ERROR, 0x16, (_INT)_T("MECHANICAL POSITIONING OR CHANGER ERROR")}, 
		{0x3D, 0x00, (_INT)_T("INVALID BITS IN IDENTIFY MESSAGE")}, 
		{0x3E, 0x00, (_INT)_T("LOGICAL UNIT HAS NOT SELF-CONFIGURED YET")}, 
		{0x3E, 0x01, (_INT)_T("LOGICAL UNIT FAILURE")}, 
		{0x3E, 0x02, (_INT)_T("TIMEOUT ON LOGICAL UNIT")}, 
		{0x3E, 0x03, (_INT)_T("LOGICAL UNIT FAILED SELF-TEST")}, 
		{0x3E, 0x04, (_INT)_T("LOGICAL UNIT UNABLE TO UPDATE SELF-TEST LOG")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_TARGET_OPERATING_CONDITIONS_CHANGED, (_INT)_T("TARGET OPERATING CONDITIONS HAVE CHANGED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_MICROCODE_CHANGED, (_INT)_T("MICROCODE HAS BEEN CHANGED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_OPERATING_DEFINITION_CHANGED, (_INT)_T("CHANGED OPERATING DEFINITION")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_INQUIRY_DATA_CHANGED, (_INT)_T("INQUIRY DATA HAS CHANGED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_COMPONENT_DEVICE_ATTACHED, (_INT)_T("COMPONENT DEVICE ATTACHED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_DEVICE_IDENTIFIER_CHANGED, (_INT)_T("DEVICE IDENTIFIER CHANGED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_REDUNDANCY_GROUP_MODIFIED, (_INT)_T("REDUNDANCY GROUP CREATED OR MODIFIED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_REDUNDANCY_GROUP_DELETED, (_INT)_T("REDUNDANCY GROUP DELETED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_SPARE_MODIFIED, (_INT)_T("SPARE CREATED OR MODIFIED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_SPARE_DELETED, (_INT)_T("SPARE DELETED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_VOLUME_SET_MODIFIED, (_INT)_T("VOLUME SET CREATED OR MODIFIED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_VOLUME_SET_DELETED, (_INT)_T("VOLUME SET DELETED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_VOLUME_SET_DEASSIGNED, (_INT)_T("VOLUME SET DEASSIGNED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_VOLUME_SET_REASSIGNED, (_INT)_T("VOLUME SET REASSIGNED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_REPORTED_LUNS_DATA_CHANGED, (_INT)_T("REPORTED LUNS DATA HAS CHANGED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_ECHO_BUFFER_OVERWRITTEN, (_INT)_T("ECHO pBuf OVERWRITTEN")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_MEDIUM_LOADABLE, (_INT)_T("MEDIUM LOADABLE")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, SCSI_SENSEQ_MEDIUM_AUXILIARY_MEMORY_ACCESSIBLE, (_INT)_T("MEDIUM AUXILIARY MEMORY ACCESSIBLE")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, 0x12, (_INT)_T("iSCSI IP ADDRESS ADDED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, 0x13, (_INT)_T("iSCSI IP ADDRESS REMOVED")}, 
		{SCSI_ADSENSE_OPERATING_CONDITIONS_CHANGED, 0x14, (_INT)_T("iSCSI IP ADDRESS CHANGED")}, 
		{0x43, 0x00, (_INT)_T("MESSAGE ERROR")}, 
		{0x44, 0x00, (_INT)_T("INTERNAL TARGET FAILURE")}, 
		{0x45, 0x00, (_INT)_T("SELECT OR RESELECT FAILURE")}, 
		{0x46, 0x00, (_INT)_T("UNSUCCESSFUL SOFT RESET")}, 
		{0x47, 0x00, (_INT)_T("SCSI PARITY ERROR")}, 
		{0x47, 0x01, (_INT)_T("DATA PHASE CRC ERROR DETECTED")}, 
		{0x47, 0x02, (_INT)_T("SCSI PARITY ERROR DETECTED DURING ST DATA PHASE")}, 
		{0x47, 0x03, (_INT)_T("INFORMATION UNIT iuCRC ERROR DETECTED")}, 
		{0x47, 0x04, (_INT)_T("ASYNCHRONOUS INFORMATION PROTECTION ERROR DETECTED")}, 
		{0x47, 0x05, (_INT)_T("PROTOCOL SERVICE CRC ERROR")}, 
		{0x47, 0x7F, (_INT)_T("SOME COMMANDS CLEARED BY ISCSI PROTOCOL EVENT")}, 
		{0x48, 0x00, (_INT)_T("INITIATOR DETECTED ERROR MESSAGE RECEIVED")}, 
		{0x49, 0x00, (_INT)_T("INVALID MESSAGE ERROR")}, 
		{0x4A, 0x00, (_INT)_T("COMMAND PHASE ERROR")}, 
		{0x4B, 0x00, (_INT)_T("DATA PHASE ERROR")}, 
		{0x4B, 0x01, (_INT)_T("INVALID TARGET PORT TRANSFER TAG RECEIVED")}, 
		{0x4B, 0x02, (_INT)_T("TOO MUCH WRITE DATA")}, 
		{0x4B, 0x03, (_INT)_T("ACK/NAK TIMEOUT")}, 
		{0x4B, 0x04, (_INT)_T("NAK RECEIVED")}, 
		{0x4B, 0x05, (_INT)_T("DATA OFFSET ERROR")}, 
		{0x4B, 0x06, (_INT)_T("INITIATOR RESPONSE TIMEOUT")}, 
		{0x4B, 0x07, (_INT)_T("CONNECTION LOST")}, 
		{0x4C, 0x00, (_INT)_T("LOGICAL UNIT FAILED SELF-CONFIGURATION")}, 
		{0x4E, 0x00, (_INT)_T("OVERLAPPED COMMANDS ATTEMPTED")}, 
		{0x51, 0x00, (_INT)_T("ERASE FAILURE")}, 
		{0x51, 0x01, (_INT)_T("ERASE FAILURE - INCOMPLETE ERASE OPERATION DETECTED")}, 
		{0x53, 0x00, (_INT)_T("MEDIA LOAD OR EJECT FAILED")}, 
		{0x53, 0x02, (_INT)_T("MEDIUM REMOVAL PREVENTED")}, 
		{0x55, 0x02, (_INT)_T("INSUFFICIENT RESERVATION RESOURCES")}, 
		{0x55, 0x03, (_INT)_T("INSUFFICIENT RESOURCES")}, 
		{0x55, 0x04, (_INT)_T("INSUFFICIENT REGISTRATION RESOURCES")}, 
		{0x55, 0x05, (_INT)_T("INSUFFICIENT ACCESS CONTROL RESOURCES")}, 
		{0x55, 0x06, (_INT)_T("AUXILIARY MEMORY OUT OF SPACE")}, 
		{0x55, 0x0B, (_INT)_T("INSUFFICIENT POWER FOR OPERATION")}, 
		{0x57, 0x00, (_INT)_T("UNABLE TO RECOVER TABLE-OF-CONTENTS")}, 
		{SCSI_ADSENSE_OPERATOR_REQUEST, SCSI_SENSEQ_STATE_CHANGE_INPUT, (_INT)_T("OPERATOR REQUEST OR STATE CHANGE INPUT")}, 
		{SCSI_ADSENSE_OPERATOR_REQUEST, SCSI_SENSEQ_MEDIUM_REMOVAL, (_INT)_T("OPERATOR MEDIUM REMOVAL REQUEST")}, 
		{SCSI_ADSENSE_OPERATOR_REQUEST, SCSI_SENSEQ_WRITE_PROTECT_ENABLE, (_INT)_T("OPERATOR SELECTED WRITE PROTECT")}, 
		{SCSI_ADSENSE_OPERATOR_REQUEST, SCSI_SENSEQ_WRITE_PROTECT_DISABLE, (_INT)_T("OPERATOR SELECTED WRITE PERMIT")}, 
		{0x5b, 0x00, (_INT)_T("LOG EXCEPTION")}, 
		{0x5b, 0x01, (_INT)_T("THRESHOLD CONDITION MET")}, 
		{0x5b, 0x02, (_INT)_T("LOG COUNTER AT MAXIMUM")}, 
		{0x5b, 0x03, (_INT)_T("LOG LIST CODES EXHAUSTED")}, 
		{SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED, 0x00, (_INT)_T("FAILURE PREDICTION THRESHOLD EXCEEDED")}, 
		{SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED, 0x01, (_INT)_T("MEDIA FAILURE PREDICTION THRESHOLD EXCEEDED")}, 
		{SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED, 0x02, (_INT)_T("LOGICAL UNIT FAILURE PREDICTION THRESHOLD EXCEEDED")}, 
		{SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED, 0x03, (_INT)_T("SPARE AREA EXHAUSTION PREDICTION THRESHOLD EXCEEDED")}, 
		{SCSI_ADSENSE_FAILURE_PREDICTION_THRESHOLD_EXCEEDED, 0xFF, (_INT)_T("FAILURE PREDICTION THRESHOLD EXCEEDED (FALSE)")}, 
		{0x5E, 0x00, (_INT)_T("LOW POWER CONDITION ON")}, 
		{0x5E, 0x01, (_INT)_T("IDLE CONDITION ACTIVATED BY TIMER")}, 
		{0x5E, 0x02, (_INT)_T("STANDBY CONDITION ACTIVATED BY TIMER")}, 
		{0x5E, 0x03, (_INT)_T("IDLE CONDITION ACTIVATED BY COMMAND")}, 
		{0x5E, 0x04, (_INT)_T("STANDBY CONDITION ACTIVATED BY COMMAND")}, 
		{0x5E, 0x05, (_INT)_T("IDLE_B CONDITION ACTIVATED BY TIMER")}, 
		{0x5E, 0x06, (_INT)_T("IDLE_B CONDITION ACTIVATED BY COMMAND")}, 
		{0x5E, 0x07, (_INT)_T("IDLE_C CONDITION ACTIVATED BY TIMER")}, 
		{0x5E, 0x08, (_INT)_T("IDLE_C CONDITION ACTIVATED BY COMMAND")}, 
		{0x5E, 0x09, (_INT)_T("STANDBY_Y CONDITION ACTIVATED BY TIMER")}, 
		{0x5E, 0x0A, (_INT)_T("STANDBY_Y CONDITION ACTIVATED BY COMMAND")}, 
		{0x63, 0x00, (_INT)_T("END OF USER AREA ENCOUNTERED ON THIS TRACK")}, 
		{0x63, 0x01, (_INT)_T("PACKET DOES NOT FIT IN AVAILABLE SPACE")}, 
		{SCSI_ADSENSE_ILLEGAL_MODE_FOR_THIS_TRACK, 0x00, (_INT)_T("ILLEGAL MODE FOR THIS TRACK")}, 
		{SCSI_ADSENSE_ILLEGAL_MODE_FOR_THIS_TRACK, 0x01, (_INT)_T("INVALID PACKET SIZE")}, 
		{0x65, 0x00, (_INT)_T("VOLTAGE FAULT")}, 
		{0x67, 0x0A, (_INT)_T("SET TARGET PORT GROUPS COMMAND FAILED")}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_AUTHENTICATION_FAILURE, (_INT)_T("COPY PROTECTION KEY EXCHANGE FAILURE - AUTHENTICATION FAILURE")}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_KEY_NOT_PRESENT, (_INT)_T("COPY PROTECTION KEY EXCHANGE FAILURE - KEY NOT PRESENT")}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_KEY_NOT_ESTABLISHED, (_INT)_T("COPY PROTECTION KEY EXCHANGE FAILURE - KEY NOT ESTABLISHED")}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_READ_OF_SCRAMBLED_SECTOR_WITHOUT_AUTHENTICATION, (_INT)_T("READ OF SCRAMBLED SECTOR WITHOUT AUTHENTICATION")}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_MEDIA_CODE_MISMATCHED_TO_LOGICAL_UNIT, (_INT)_T("MEDIA REGION CODE IS MISMATCHED TO LOGICAL UNIT REGION")}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, SCSI_SENSEQ_LOGICAL_UNIT_RESET_COUNT_ERROR, (_INT)_T("DRIVE REGION MUST BE PERMANENT/REGION RESET COUNT ERROR")}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, 0x06, (_INT)_T("INSUFFICIENT BLOCK COUNT FOR BINDING NONCE RECORDING")}, 
		{SCSI_ADSENSE_COPY_PROTECTION_FAILURE, 0x07, (_INT)_T("CONFLICT IN BINDING NONCE RECORDING")}, 
		{0x72, 0x00, (_INT)_T("SESSION FIXATION ERROR")}, 
		{0x72, 0x01, (_INT)_T("SESSION FIXATION ERROR WRITING LEAD-IN")}, 
		{0x72, 0x02, (_INT)_T("SESSION FIXATION ERROR WRITING LEAD-OUT")}, 
		{0x72, 0x03, (_INT)_T("SESSION FIXATION ERROR - INCOMPLETE TRACK IN SESSION")}, 
		{0x72, 0x04, (_INT)_T("EMPTY OR PARTIALLY WRITTEN RESERVED TRACK")}, 
		{0x72, 0x05, (_INT)_T("NO MORE TRACK RESERVATIONS ALLOWED")}, 
		{0x72, 0x06, (_INT)_T("RMZ EXTENSION IS NOT ALLOWED")}, 
		{0x72, 0x07, (_INT)_T("NO MORE TEST ZONE EXTENSIONS ARE ALLOWED")}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, 0x00, (_INT)_T("VOLTAGE FAULT")}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_POWER_CALIBRATION_AREA_ALMOST_FULL, (_INT)_T("CD CONTROL ERROR")}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_POWER_CALIBRATION_AREA_FULL, (_INT)_T("POWER CALIBRATION AREA ALMOST FULL")}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_POWER_CALIBRATION_AREA_ERROR, (_INT)_T("POWER CALIBRATION AREA IS FULL")}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_PMA_RMA_UPDATE_FAILURE, (_INT)_T("POWER CALIBRATION AREA ERROR")}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_PMA_RMA_IS_FULL, (_INT)_T("PROGRAM MEMORY AREA UPDATE FAILURE")}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, SCSI_SENSEQ_PMA_RMA_ALMOST_FULL, (_INT)_T("RMA/PMA IS ALMOST FULL")}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, 0x10, (_INT)_T("CURRENT POWER CALIBRATION AREA ALMOST FULL")}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, 0x11, (_INT)_T("CURRENT POWER CALIBRATION AREA IS FULL")}, 
		{SCSI_ADSENSE_POWER_CALIBRATION_ERROR, 0x17, (_INT)_T("RDZ IS FULL")}, 
		{0x74, 0x08, (_INT)_T("DIGITAL SIGNATURE VALIDATION FAILURE")}, 
		{0x74, 0x0C, (_INT)_T("UNABLE TO DECRYPT PARAMETER LIST")}, 
		{0x74, 0x10, (_INT)_T("SA CREATION PARAMETER VALUE INVALID")}, 
		{0x74, 0x11, (_INT)_T("SA CREATION PARAMETER VALUE REJECTED")}, 
		{0x74, 0x12, (_INT)_T("INVALID SA USAGE")}, 
		{0x74, 0x30, (_INT)_T("SA CREATION PARAMETER NOT SUPPORTED")}, 
		{0x74, 0x40, (_INT)_T("AUTHENTICATION FAILED")}, 
		{0x74, 0x71, (_INT)_T("LOGICAL UNIT ACCESS NOT AUTHORIZED")} 
	};
	OutputErrorString(_T("Sense data, Key:Asc:Ascq:%02x:%02x:%02x"), byKey, byAsc, byAscq);

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

void OutputAlignSub96(
	CONST PUCHAR pBuf,
	INT nLBA,
	FILE* fpLog
	)
{
	OutputLogString(fpLog, _T("Sub Channel LBA %d\n"), nLBA);
	OutputLogString(fpLog, _T("\t  +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B\n"));

	for(INT i = 0, ch = 0x50; i < CD_RAW_READ_SUBCODE_SIZE; i += 12, ch++) {
		OutputLogString(fpLog, 
			_T("\t%c %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"), 
			ch,	pBuf[i], pBuf[i+1], pBuf[i+2], pBuf[i+3], pBuf[i+4], pBuf[i+5],
			pBuf[i+6], pBuf[i+7], pBuf[i+8], pBuf[i+9], pBuf[i+10], pBuf[i+11]);
	}
}

void OutputRawSub96(
	CONST PUCHAR pBuf,
	INT nLBA,
	FILE* fpLog
	)
{
	OutputLogString(fpLog, _T("Sub Channel(Raw) LBA %d\n"), nLBA);
	OutputLogString(fpLog, _T("\t    +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n"));

	for(INT i = 0; i < CD_RAW_READ_SUBCODE_SIZE; i += 16) {
		OutputLogString(fpLog, 
			_T("\t%3X %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"), 
			i, pBuf[i], pBuf[i+1], pBuf[i+2], pBuf[i+3], pBuf[i+4], pBuf[i+5],
			pBuf[i+6], pBuf[i+7], pBuf[i+8], pBuf[i+9], pBuf[i+10], pBuf[i+11], 
			pBuf[i+12], pBuf[i+13], pBuf[i+14], pBuf[i+15]);
	}
}

void OutputSubcode(
	INT nLBA,
	INT nTrackNum,
	CONST PUCHAR Subcode,
	CONST PUCHAR SubcodeOrg,
	FILE* fpParse
	)
{
	_TCHAR str[256] = {0};
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
	_TCHAR str2[256] = {0};
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

	// EN 60908:1999 Page25-
	typedef struct _SubcodeRtoW {
        CHAR command;
        CHAR instruction;
        CHAR parityQ[2];
        CHAR data[16];
        CHAR parityP[4];
	} SubcodeRtoW;

	SubcodeRtoW scRW[4] = {0};
	UCHAR tmpCode[24] = {0};
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 24; j++) {
			tmpCode[j] = (UCHAR)(*(SubcodeOrg + (i * 24 + j)) & 0x3F);
		}
		memcpy(&scRW[i], tmpCode, sizeof(scRW[i]));

		switch(scRW[i].command) {
		case 0: // MODE 0, ITEM 0
			_tcscat(str, _T("RtoW:ZERO mode"));
			break;
		case 8: // MODE 1, ITEM 0
			_tcscat(str, _T("RtoW:LINE-GRAPHICS mode"));
			break;
		case 9: // MODE 1, ITEM 1
			_tcscat(str, _T("RtoW:TV-GRAPHICS mode"));
			break;
		case 10: // MODE 1, ITEM 2
			_tcscat(str, _T("RtoW:EXTENDED-TV-GRAPHICS mode"));
			break;
		case 20: // MODE 2, ITEM 4
			_tcscat(str, _T("RtoW:CD TEXT mode"));
			break;
		case 24: // MODE 3, ITEM 0
			_tcscat(str, _T("RtoW:MIDI mode"));
			break;
		case 56: // MODE 7, ITEM 0
			_tcscat(str, _T("RtoW:USER mode"));
			break;
		default:
			break;
		}
		if(i < 3) {
			_tcscat(str, _T(", "));
		}
		else {
			_tcscat(str, _T("\n"));
		}
	}
	fwrite(str, sizeof(_TCHAR), _tcslen(str), fpParse);
}

void OutputTocFull(
	CONST CDROM_TOC_FULL_TOC_DATA* fullToc,
	CONST CDROM_TOC_FULL_TOC_DATA_BLOCK* pTocData,
	size_t uiTocEntries,
	PDISC_DATA pDiscData,
	FILE* fpCcd,
	FILE* fpLog
	)
{
	OutputLogString(fpLog,
		_T("FULL TOC on SCSIOP_READ_TOC\n")
		_T("\tFirstCompleteSession: %d\n")
		_T("\t LastCompleteSession: %d\n"),
		fullToc->FirstCompleteSession,
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
				pDiscData->nLastLBAof1stSession = 
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
				pDiscData->nStartLBAof2ndSession = MSFtoLBA(pTocData[a].Msf[2], 
					pTocData[a].Msf[1], pTocData[a].Msf[0]) - 150;
			}
			pDiscData->aSessionNum[pTocData[a].Point-1] = pTocData[a].SessionNumber;
			break;
		}
	}
}

// begin for CD
void OutputVolumeDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	// 0 is Boot Record. 
	// 1 is Primary Volume Descriptor. 
	// 2 is Supplementary Volume Descriptor. 
	// 3 is Volume Partition Descriptor. 
	// 4-254 is reserved. 
	// 255 is Volume Descriptor Set Terminator.
	OutputLogString(fpLog,
		_T("Volume Descriptor\n")
		_T("\t                              Volume Descriptor Type: %d\n"),
		buf[idx]);
	_TCHAR str[5] = {0};
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+1], 5, str, sizeof(str));
#else
	strncpy(str, (PCHAR)&buf[idx+1], 5);
#endif
	OutputLogString(fpLog,
		_T("\t                                 Standard Identifier: %.5s\n")
		_T("\t                           Volume Descriptor Version: %d\n"),
		str,
		buf[idx+6]);
	
	if(buf[idx] == 0) {
		OutputBootRecord(idx, buf, fpLog);
	}
	else if(buf[idx] == 1) {
		OutputPrimaryVolumeDescriptorForISO9660(idx, buf, fpLog);
	}
	else if(buf[idx] == 2) {
		OutputPrimaryVolumeDescriptorForJoliet(idx, buf, fpLog);
	}
	else if(buf[idx] == 3) {
		OutputVolumePartitionDescriptor(idx, buf, fpLog);
	}
	else if(buf[idx] == 255) {
	}
}

void OutputBootRecord(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	_TCHAR str[2][32] = {0};
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+7], 32, str[0], sizeof(str));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+39], 32, str[1], sizeof(str));
#else
	strncpy(str[0], (PCHAR)&buf[idx+7], 32);
	strncpy(str[1], (PCHAR)&buf[idx+39], 32);
#endif
	OutputLogString(fpLog,
		_T("\t                              Boot System Identifier: %s\n")
		_T("\t                                     Boot Identifier: %s\n")
		_T("\t                                     Boot System Use: "),
		str[0],
		str[1]);
	for(INT i = 71; i <= 2047; i++) {
		OutputLogString(fpLog, _T("%x"), buf[idx+i]);
	}
	OutputLogString(fpLog, _T("\n"));
}

void OutputPrimaryVolumeDescriptorForTime(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	_TCHAR year[4][4] = {0};
	_TCHAR month[4][2] = {0};
	_TCHAR day[4][2] = {0};
	_TCHAR hour[4][2] = {0};
	_TCHAR time[4][2] = {0};
	_TCHAR second[4][2] = {0};
	_TCHAR milisecond[4][2] = {0};
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+813], 4, year[0], sizeof(year[0]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+817], 2, month[0], sizeof(month[0]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+819], 2, day[0], sizeof(day[0]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+821], 2, hour[0], sizeof(hour[0]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+823], 2, time[0], sizeof(time[0]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+825], 2, second[0], sizeof(second[0]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+827], 2, milisecond[0], sizeof(milisecond[0]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+830], 4, year[1], sizeof(year[1]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+834], 2, month[1], sizeof(month[1]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+836], 2, day[1], sizeof(day[1]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+838], 2, hour[1], sizeof(hour[1]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+840], 2, time[1], sizeof(time[1]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+842], 2, second[1], sizeof(second[1]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+844], 2, milisecond[1], sizeof(milisecond[1]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+847], 4, year[2], sizeof(year[2]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+851], 2, month[2], sizeof(month[2]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+853], 2, day[2], sizeof(day[2]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+855], 2, hour[2], sizeof(hour[2]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+857], 2, time[2], sizeof(time[2]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+859], 2, second[2], sizeof(second[2]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+861], 2, milisecond[2], sizeof(milisecond[2]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+864], 4, year[3], sizeof(year[3]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+868], 2, month[3], sizeof(month[3]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+870], 2, day[3], sizeof(day[3]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+872], 2, hour[3], sizeof(hour[3]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+874], 2, time[3], sizeof(time[3]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+876], 2, second[3], sizeof(second[3]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+878], 2, milisecond[3], sizeof(milisecond[3]));
#else
	strncpy(year[0], (PCHAR)&buf[idx+813], 4);
	strncpy(month[0], (PCHAR)&buf[idx+817], 2);
	strncpy(day[0], (PCHAR)&buf[idx+819], 2);
	strncpy(hour[0], (PCHAR)&buf[idx+821], 2);
	strncpy(time[0], (PCHAR)&buf[idx+823], 2);
	strncpy(second[0], (PCHAR)&buf[idx+825], 2);
	strncpy(milisecond[0], (PCHAR)&buf[idx+827], 2);
	strncpy(year[1], (PCHAR)&buf[idx+830], 4);
	strncpy(month[1], (PCHAR)&buf[idx+834], 2);
	strncpy(day[1], (PCHAR)&buf[idx+836], 2);
	strncpy(hour[1], (PCHAR)&buf[idx+838], 2);
	strncpy(time[1], (PCHAR)&buf[idx+840], 2);
	strncpy(second[1], (PCHAR)&buf[idx+842], 2);
	strncpy(milisecond[1], (PCHAR)&buf[idx+844], 2);
	strncpy(year[2], (PCHAR)&buf[idx+847], 4);
	strncpy(month[2], (PCHAR)&buf[idx+851], 2);
	strncpy(day[2], (PCHAR)&buf[idx+853], 2);
	strncpy(hour[2], (PCHAR)&buf[idx+855], 2);
	strncpy(time[2], (PCHAR)&buf[idx+857], 2);
	strncpy(second[2], (PCHAR)&buf[idx+859], 2);
	strncpy(milisecond[2], (PCHAR)&buf[idx+861], 2);
	strncpy(year[3], (PCHAR)&buf[idx+864], 4);
	strncpy(month[3], (PCHAR)&buf[idx+868], 2);
	strncpy(day[3], (PCHAR)&buf[idx+870], 2);
	strncpy(hour[3], (PCHAR)&buf[idx+872], 2);
	strncpy(time[3], (PCHAR)&buf[idx+874], 2);
	strncpy(second[3], (PCHAR)&buf[idx+876], 2);
	strncpy(milisecond[3], (PCHAR)&buf[idx+878], 2);
#endif
	OutputLogString(fpLog,
		_T("\t                       Volume Creation Date and Time: %.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.2s +%d\n")
		_T("\t                   Volume Modification Date and Time: %.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.2s +%d\n")
		_T("\t                     Volume Expiration Date and Time: %.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.2s +%d\n")
		_T("\t                      Volume Effective Date and Time: %.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.2s +%d\n")
		_T("\t                              File Structure Version: %d\n")
		_T("\t                                     Application Use: "), 
		year[0], month[0], day[0], hour[0], time[0], second[0], milisecond[0], buf[idx+829],
		year[1], month[1], day[1], hour[1], time[1], second[1], milisecond[1], buf[idx+846],
		year[2], month[2], day[2], hour[2], time[2], second[2], milisecond[2], buf[idx+863],
		year[3], month[3], day[3], hour[3], time[3], second[3], milisecond[3], buf[idx+880],
		buf[idx+881]);
	for(INT i = 883; i <= 1394; i++) {
		OutputLogString(fpLog, _T("%x"), buf[idx+i]);
	}
	OutputLogString(fpLog, _T("\n"));
}

void OutputPrimaryVolumeDescriptorFor1(
	INT idx,
	CONST PUCHAR buf,
	_TCHAR str[][128],
	FILE* fpLog
	)
{
	OutputLogString(fpLog,
		_T("\t                                   System Identifier: %.32s\n")
		_T("\t                                   Volume Identifier: %.32s\n")
		_T("\t                                   Volume Space Size: %d\n"),
		str[0],
		str[1],
		buf[idx+80]);
}

void OutputPrimaryVolumeDescriptorFor2(
	INT idx,
	CONST PUCHAR buf,
	_TCHAR str[][128],
	FILE* fpLog
	)
{
	OutputLogString(fpLog, 
		_T("\t                                     Volume Set Size: %d\n")
		_T("\t                              Volume Sequence Number: %d\n")
		_T("\t                                  Logical Block Size: %d\n")
		_T("\t                                     Path Table Size: %d\n")
		_T("\t         Location of Occurrence of Type L Path Table: %d\n")
		_T("\tLocation of Optional Occurrence of Type L Path Table: %d\n")
		_T("\t         Location of Occurrence of Type M Path Table: %d\n")
		_T("\tLocation of Optional Occurrence of Type M Path Table: %d\n")
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
		_T("\t                               Volume Set Identifier: %.128s\n")
		_T("\t                                Publisher Identifier: %.128s\n")
		_T("\t                            Data Preparer Identifier: %.128s\n")
		_T("\t                              Application Identifier: %.128s\n")
		_T("\t                           Copyright File Identifier: %.37s\n")
		_T("\t                            Abstract File Identifier: %.37s\n")
		_T("\t                       Bibliographic File Identifier: %.37s\n"), 
		MAKEWORD(buf[idx+123], buf[idx+122]),
		MAKEWORD(buf[idx+127], buf[idx+126]),
		MAKEWORD(buf[idx+131], buf[idx+130]),
		MAKELONG(MAKEWORD(buf[idx+139], buf[idx+138]), 
			MAKEWORD(buf[idx+137], buf[idx+136])),
		MAKEWORD(buf[idx+143], buf[idx+142]),
		MAKEWORD(buf[idx+147], buf[idx+146]),
		MAKEWORD(buf[idx+151], buf[idx+150]),
		MAKEWORD(buf[idx+155], buf[idx+154]),
		buf[idx+156],
		buf[idx+157],
		MAKELONG(MAKEWORD(buf[idx+165], buf[idx+164]), 
			MAKEWORD(buf[idx+163], buf[idx+162])),
		MAKELONG(MAKEWORD(buf[idx+173], buf[idx+172]), 
			MAKEWORD(buf[idx+171], buf[idx+170])),
		buf[idx+174] + 1900, buf[idx+175], buf[idx+176], buf[idx+177], buf[idx+178], buf[idx+179], buf[idx+180],
		buf[idx+181],
		buf[idx+182],
		buf[idx+183],
		MAKEWORD(buf[idx+187], buf[idx+186]),
		buf[idx+188],
		buf[idx+189],
		str[2],
		str[3],
		str[4],
		str[5],
		str[6],
		str[7],
		str[8]);
}

void OutputPrimaryVolumeDescriptorForISO9660(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	_TCHAR str[10][128] = {0};
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+8], 32, str[0], sizeof(str[0]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+40], 32, str[1], sizeof(str[1]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+190], 128, str[2], sizeof(str[2]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+318], 128, str[3], sizeof(str[3]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+446], 128, str[4], sizeof(str[4]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+574], 128, str[5], sizeof(str[5]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+702], 37, str[6], sizeof(str[6]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+739], 37, str[7], sizeof(str[7]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+776], 37, str[8], sizeof(str[8]));
#else
	strncpy(str[0], (PCHAR)&buf[idx+8], 32);
	strncpy(str[1], (PCHAR)&buf[idx+40], 32);
	strncpy(str[2], (PCHAR)&buf[idx+190], 128);
	strncpy(str[3], (PCHAR)&buf[idx+318], 128);
	strncpy(str[4], (PCHAR)&buf[idx+446], 128);
	strncpy(str[5], (PCHAR)&buf[idx+574], 128);
	strncpy(str[6], (PCHAR)&buf[idx+702], 37);
	strncpy(str[7], (PCHAR)&buf[idx+739], 37);
	strncpy(str[8], (PCHAR)&buf[idx+776], 37);
#endif
	OutputPrimaryVolumeDescriptorFor1(idx, buf, str, fpLog);
	if(buf[idx+0] == 2) {
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+88], 32, str[9], sizeof(str[9]));
#else
		strncpy(str[9], (PCHAR)&buf[idx+88], 32);
#endif
		OutputLogString(fpLog, _T(
			"\t                                    Escape Sequences: %.32s\n"), str[9]);
	}

	OutputPrimaryVolumeDescriptorFor2(idx, buf, str, fpLog);
	OutputPrimaryVolumeDescriptorForTime(idx, buf, fpLog);
}

void OutputPrimaryVolumeDescriptorForJoliet(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	_TCHAR str[9][128] = {0};
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+8], 16, str[0], sizeof(str[0]));
	LittleToBig(str[0], (_TCHAR*)&buf[idx+8], 16);
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+40], 16, str[1], sizeof(str[1]));
	LittleToBig(str[1], (_TCHAR*)&buf[idx+40], 16);
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+190], 16, str[2], sizeof(str[2]));
	LittleToBig(str[2], (_TCHAR*)&buf[idx+190], 64);
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+318], 16, str[3], sizeof(str[3]));
	LittleToBig(str[3], (_TCHAR*)&buf[idx+318], 64);
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+446], 16, str[4], sizeof(str[4]));
	LittleToBig(str[4], (_TCHAR*)&buf[idx+446], 64);
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+574], 16, str[5], sizeof(str[5]));
	LittleToBig(str[5], (_TCHAR*)&buf[idx+574], 64);
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+702], 16, str[6], sizeof(str[6]));
	LittleToBig(str[6], (_TCHAR*)&buf[idx+702], 18);
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+739], 16, str[7], sizeof(str[7]));
	LittleToBig(str[7], (_TCHAR*)&buf[idx+739], 18);
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+776], 16, str[8], sizeof(str[8]));
	LittleToBig(str[8], (_TCHAR*)&buf[idx+776], 18);
#else
	_TCHAR str2[9][128+1] = {0};
	LittleToBig(str2[0], (_TCHAR*)&buf[idx+8], 32);
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)&str2[0], 32, str[0], sizeof(str[0]), NULL, NULL);
	LittleToBig(str2[1], (_TCHAR*)&buf[idx+40], 32);
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)&str2[1], 32, str[1], sizeof(str[1]), NULL, NULL);
	LittleToBig(str2[2], (_TCHAR*)&buf[idx+190], 128);
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)&str2[2], 128, str[2], sizeof(str[2]), NULL, NULL);
	LittleToBig(str2[3], (_TCHAR*)&buf[idx+318], 128);
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)&str2[3], 128, str[3], sizeof(str[3]), NULL, NULL);
	LittleToBig(str2[4], (_TCHAR*)&buf[idx+446], 128);
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)&str2[4], 128, str[4], sizeof(str[4]), NULL, NULL);
	LittleToBig(str2[5], (_TCHAR*)&buf[idx+574], 128);
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)&str2[5], 128, str[5], sizeof(str[5]), NULL, NULL);
	LittleToBig(str2[6], (_TCHAR*)&buf[idx+702], 36);
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)&str2[6], 36, str[6], sizeof(str[6]), NULL, NULL);
	LittleToBig(str2[7], (_TCHAR*)&buf[idx+739], 36);
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)&str2[7], 36, str[7], sizeof(str[7]), NULL, NULL);
	LittleToBig(str2[8], (_TCHAR*)&buf[idx+776], 36);
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)&str2[8], 36, str[8], sizeof(str[8]), NULL, NULL);
#endif
	OutputPrimaryVolumeDescriptorFor1(idx, buf, str, fpLog);

	_TCHAR str3[32] = {0};
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+88], 16, str3, sizeof(str3));
#else
	_tcsncpy(str3, (_TCHAR*)&buf[idx+88], 32);
#endif
	OutputLogString(fpLog, _T(
		"\t                                    Escape Sequences: %.32s\n"), str3);
	OutputPrimaryVolumeDescriptorFor2(idx, buf, str, fpLog);
	OutputPrimaryVolumeDescriptorForTime(idx, buf, fpLog);
}

void OutputVolumePartitionDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	_TCHAR str[2][32] = {0};
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+8], 32, str[0], sizeof(str[0]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+40], 32, str[1], sizeof(str[1]));
#else
	strncpy(str[0], (PCHAR)&buf[idx+8], 32);
	strncpy(str[1], (PCHAR)&buf[idx+40], 32);
#endif
	OutputLogString(fpLog,
		_T("\t          System Identifier: %.32s\n")
		_T("\tVolume Partition Identifier: %.32s\n")
		_T("\t  Volume Partition Location: %d\n")
		_T("\t      Volume Partition Size: %d\n")
		_T("\t                 System Use: "),
		str[0],
		str[1], 
		MAKELONG(MAKEWORD(buf[idx+79], buf[idx+78]), 
			MAKEWORD(buf[idx+77], buf[idx+76])), 
		MAKELONG(MAKEWORD(buf[idx+87], buf[idx+86]), 
			MAKEWORD(buf[idx+85], buf[idx+84])));
	for(INT i = 88; i <= 2047; i++) {
		OutputLogString(fpLog, _T("%x"), buf[idx+i]);
	}
	OutputLogString(fpLog, _T("\n"));
}
// end for CD

// begin for DVD
void OutputVolumeStructureDescriptorFormat(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	_TCHAR str[5] = {0};
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+1], 5, str, sizeof(str));
#else
	strncpy(str, (PCHAR)&buf[idx+1], 5);
#endif
	OutputLogString(fpLog,
		_T("Volume Recognition Sequence\n")
		_T("\t                                      Structure Type: %d\n")
		_T("\t                                 Standard Identifier: %.5s\n")
		_T("\t                                   Structure Version: %d\n"),
		buf[idx],
		str,
		buf[idx+6]);
}

void OutputVolumeRecognitionSequence(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	_TCHAR str[5] = {0};
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+1], 5, str, sizeof(str));
#else
	strncpy(str, (PCHAR)&buf[idx+1], 5);
#endif
	if(buf[idx] == 1 && !_tcsncmp(str, _T("CD001"), 5)) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
		OutputPrimaryVolumeDescriptorForISO9660(idx, buf, fpLog);
	}
	else if(buf[idx] == 2 && !_tcsncmp(str, _T("CD001"), 5)) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
		OutputLogString(fpLog, _T("\tVolume Flags: %d\n"), buf[idx+7]);
		OutputPrimaryVolumeDescriptorForJoliet(idx, buf, fpLog);
	}
	else if(buf[idx] == 255 && !_tcsncmp(str, _T("CD001"), 5)) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
	}
	else if(buf[idx] == 0 && !_tcsncmp(str, _T("BOOT2"), 5)) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
		OutputBootDescriptor(idx, buf, fpLog);
	}
	else if(buf[idx] == 0 && !_tcsncmp(str, _T("BEA01"), 5)) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
	}
	else if(buf[idx] == 0 && !_tcsncmp(str, _T("NSR02"), 5)) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
	}
	else if(buf[idx] == 0 && !_tcsncmp(str, _T("NSR03"), 5)) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
	}
	else if(buf[idx] == 0 && !_tcsncmp(str, _T("TEA01"), 5)) {
		OutputVolumeStructureDescriptorFormat(idx, buf, fpLog);
	}
}

void OutputRecordingDateAndTime(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	OutputLogString(fpLog, _T(
		"\tRecording Date and Time: %x %d-%d-%d %d: %d: %d.%d.%d.%d\n"),
		MAKEWORD(buf[idx], buf[idx+1]), MAKEWORD(buf[idx+2], buf[idx+3]),
		buf[idx+4], buf[idx+5], buf[idx+6], buf[idx+7], buf[idx+8],
		buf[idx+9], buf[idx+10], buf[idx+11]);
}

void OutputBootDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	OutputLogString(fpLog,
		_T("\tArchitecture Type\n")
		_T("\t\tFlags: %d\n"), buf[idx+8]);

	_TCHAR str[4][23] = {0};
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+9], 23, str[0], sizeof(str[0]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+24], 8, str[1], sizeof(str[1]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+41], 23, str[2], sizeof(str[2]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+64], 8, str[3], sizeof(str[3]));
#else
	strncpy(str[0], (PCHAR)&buf[idx+9], 23);
	strncpy(str[1], (PCHAR)&buf[idx+24], 8);
	strncpy(str[2], (PCHAR)&buf[idx+41], 23);
	strncpy(str[3], (PCHAR)&buf[idx+64], 8);
#endif
	OutputLogString(fpLog,
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
		str[0],
		str[1],
		buf[idx+40],
		str[2],
		str[3],
		MAKELONG(MAKEWORD(buf[idx+75], buf[idx+74]), 
			MAKEWORD(buf[idx+73], buf[idx+72])),
		MAKELONG(MAKEWORD(buf[idx+79], buf[idx+78]), 
			MAKEWORD(buf[idx+77], buf[idx+76])),
		MAKELONG(MAKEWORD(buf[idx+87], buf[idx+86]), 
			MAKEWORD(buf[idx+85], buf[idx+84])),
		MAKELONG(MAKEWORD(buf[idx+83], buf[idx+82]), 
			MAKEWORD(buf[idx+81], buf[idx+80])),
		MAKELONG(MAKEWORD(buf[idx+87], buf[idx+86]), 
			MAKEWORD(buf[idx+85], buf[idx+84])),
		MAKELONG(MAKEWORD(buf[idx+83], buf[idx+82]), 
			MAKEWORD(buf[idx+81], buf[idx+80])));

	OutputRecordingDateAndTime(idx + 96, buf, fpLog);
	OutputLogString(fpLog,
		_T("\t               Flags: %d\n")
		_T("\t            Boot Use: "),
		MAKEWORD(buf[idx+109], buf[idx+108]));
	for(INT i = 142; i <= 2047; i++) {
		OutputLogString(fpLog, _T("%x"), buf[idx+i]);
	}
	OutputLogString(fpLog, _T("\n"));
}

void OutputCharspec(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	_TCHAR str[23] = {0};
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+1], 23, str, sizeof(str));
#else
	strncpy(str, (PCHAR)&buf[idx+1], 23);
#endif
	OutputLogString(fpLog,
		_T("\t\t       Character Set Type: %d\n")
		_T("\t\tCharacter Set Information: %.23s\n"),
		buf[idx], str);
}

void OutputExtentDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	OutputLogString(fpLog,
		_T("\t\t  Extent Length: %d\n")
		_T("\t\tExtent Location: %d\n"),
		MAKELONG(MAKEWORD(buf[idx], buf[idx+1]), 
			MAKEWORD(buf[idx+2], buf[idx+3])), 
		MAKELONG(MAKEWORD(buf[idx+4], buf[idx+5]), 
			MAKEWORD(buf[idx+6], buf[idx+7])));
}

void OutputRegid(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	_TCHAR str[23+1] = {0};
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+1], 23, str, sizeof(str));
#else
	strncpy(str, (PCHAR)&buf[idx+1], 23); str[23] = '\0';
#endif
	OutputLogString(fpLog,
		 _T("\t\t            Flags: %d\n")
		 _T("\t\t       Identifier: %.23s\n")
		 _T("\t\tIdentifier Suffix: "),
		buf[idx],
		str);
	for(INT i = 24; i <= 31; i++) {
		OutputLogString(fpLog, _T("%x"), buf[idx+i]);
	}
	OutputLogString(fpLog, _T("\n"));
}

void OutputPrimaryVolumeDescriptorForUDF(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	_TCHAR str[2][128] = {0};
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+24], 32, str[0], sizeof(str[0]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+72], 128, str[1], sizeof(str[1]));
#else
	strncpy(str[0], (PCHAR)&buf[idx+24], 32);
	strncpy(str[1], (PCHAR)&buf[idx+72], 128);
#endif
	OutputLogString(fpLog,
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
		MAKELONG(MAKEWORD(buf[idx+16], buf[idx+17]), 
			MAKEWORD(buf[idx+18], buf[idx+19])), 
		MAKELONG(MAKEWORD(buf[idx+20], buf[idx+21]), 
			MAKEWORD(buf[idx+22], buf[idx+23])),
		str[0], 
		MAKEWORD(buf[idx+56], buf[idx+57]), 
		MAKEWORD(buf[idx+58], buf[idx+59]), 
		MAKEWORD(buf[idx+60], buf[idx+61]), 
		MAKEWORD(buf[idx+62], buf[idx+63]), 
		MAKELONG(MAKEWORD(buf[idx+65], buf[idx+64]), 
			MAKEWORD(buf[idx+67], buf[idx+66])), 
		MAKELONG(MAKEWORD(buf[idx+68], buf[idx+69]), 
			MAKEWORD(buf[idx+70], buf[idx+71])),
		str[1]);

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
	OutputLogString(fpLog, _T("\tImplementation Use: "));
	for(INT i = 420; i <= 483; i++) {
		OutputLogString(fpLog, _T("%x"), buf[idx+i]);
	}
	OutputLogString(fpLog, _T("\n"));

	OutputLogString(fpLog, _T(
		"\tPredecessor Volume Descriptor Sequence Location: %d\n"), 
		MAKELONG(MAKEWORD(buf[idx+484], buf[idx+485]), 
			MAKEWORD(buf[idx+486], buf[idx+487])));
	OutputLogString(fpLog, _T(
		"\t                                          Flags: %d\n"), 
		MAKEWORD(buf[idx+488], buf[idx+489]));
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
}

void OutputVolumeDescriptorPointer(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	OutputLogString(fpLog, 
		_T("\t     Volume Descriptor Sequence Number: %d\n")
		_T("\tNext Volume Descriptor Sequence Extent\n"), 
		MAKELONG(MAKEWORD(buf[idx+16], buf[idx+17]), 
			MAKEWORD(buf[idx+18], buf[idx+19])));
	OutputExtentDescriptor(idx + 20, buf, fpLog);
}

void OutputImplementationUseVolumeDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	OutputLogString(fpLog,
		_T("\tVolume Descriptor Sequence Number: %d\n")
		_T("\tImplementation Identifier\n"), 
		MAKELONG(MAKEWORD(buf[idx+16], buf[idx+17]), 
		MAKEWORD(buf[idx+18], buf[idx+19])));

	OutputRegid(idx + 20, buf, fpLog);

	OutputLogString(fpLog, _T("\tLVI Charset\n"));
	OutputCharspec(idx + 52, buf, fpLog);

	_TCHAR str[4][128] = {0};
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+116], 128, str[0], sizeof(str[0]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+244], 36, str[1], sizeof(str[1]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+280], 36, str[2], sizeof(str[2]));
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+316], 36, str[3], sizeof(str[3]));
#else
	strncpy(str[0], (PCHAR)&buf[idx+116], 128);
	strncpy(str[1], (PCHAR)&buf[idx+244], 36);
	strncpy(str[2], (PCHAR)&buf[idx+280], 36);
	strncpy(str[3], (PCHAR)&buf[idx+316], 36);
#endif
	OutputLogString(fpLog, _T("\tLogical Volume Identifier: %.128s\n"), str[0]);
	for(INT i = 1; i < 4; i++) {
		OutputLogString(fpLog, _T("\t               LV Info %d: %.36s\n"), i, str[1]);
	}
	OutputLogString(fpLog, _T("\tImplemention ID\n"));
	OutputRegid(idx + 352, buf, fpLog);
	OutputLogString(fpLog, _T("\tImplementation Use: "));
	for(INT i = 384; i <= 511; i++) {
		OutputLogString(fpLog, _T("%x"), buf[idx+i]);
	}
	OutputLogString(fpLog, _T("\n"));
}

void OutputPartitionDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	OutputLogString(fpLog,
		_T("\tVolume Descriptor Sequence Number: %d\n")
		_T("\t                  Partition Flags: %d\n")
		_T("\t                 Partition Number: %d\n")
		_T("\tPartition Contents\n"), 
		MAKELONG(MAKEWORD(buf[idx+16], buf[idx+17]), 
			MAKEWORD(buf[idx+18], buf[idx+19])), 
		MAKEWORD(buf[idx+20], buf[idx+21]), 
		MAKEWORD(buf[idx+22], buf[idx+23]));
	
	OutputRegid(idx + 24, buf, fpLog);

	OutputLogString(fpLog, _T("\tPartition Contents Use: "));
	for(INT i = 56; i <= 183; i++) {
		OutputLogString(fpLog, _T("%x"), buf[idx+i]);
	}
	OutputLogString(fpLog, _T("\n"));
	OutputLogString(fpLog,
		_T("\t                Access Type: %d\n")
		_T("\tPartition Starting Location: %d\n")
		_T("\t           Partition Length: %d\n")
		_T("\tImplementation Identifier\n"), 
		MAKELONG(MAKEWORD(buf[idx+184], buf[idx+185]), 
			MAKEWORD(buf[idx+186], buf[idx+187])), 
		MAKELONG(MAKEWORD(buf[idx+188], buf[idx+189]), 
			MAKEWORD(buf[idx+190], buf[idx+191])), 
		MAKELONG(MAKEWORD(buf[idx+192], buf[idx+193]), 
			MAKEWORD(buf[idx+194], buf[idx+195])));
	
	OutputRegid(idx + 196, buf, fpLog);
	OutputLogString(fpLog, _T("\tImplementation Use: "));
	for(INT i = 228; i <= 355; i++) {
		OutputLogString(fpLog, _T("%x"), buf[idx+i]);
	}
	OutputLogString(fpLog, _T("\n"));
}

void OutputLongAllocationDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	OutputLogString(fpLog,
		_T("\tLongAllocationDescriptor\n")
		_T("\t\t             Extent Length: %d\n")
		_T("\t\t      Logical Block Number: %d\n")
		_T("\t\tPartition Reference Number: %d\n"), 
		MAKELONG(MAKEWORD(buf[idx], buf[idx+1]), 
			MAKEWORD(buf[idx+2], buf[idx+3])), 
		MAKELONG(MAKEWORD(buf[idx+4], buf[idx+5]), 
			MAKEWORD(buf[idx+6], buf[idx+7])), 
		MAKEWORD(buf[idx+8], buf[idx+9]));
}

void OutputLogicalVolumeDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	OutputLogString(fpLog,
		_T("\tVolume Descriptor Sequence Number: %d\n")
		_T("\tDescriptor Character Set\n"), 
		MAKELONG(MAKEWORD(buf[idx+16], buf[idx+17]), 
			MAKEWORD(buf[idx+18], buf[idx+19])));
	
	OutputCharspec(idx + 20, buf, fpLog);

	_TCHAR str[128] = {0};
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, (PCHAR)&buf[idx+84], 128, str, sizeof(str));
#else
	strncpy(str, (PCHAR)&buf[idx+84], 128);
#endif
	OutputLogString(fpLog, 
		_T("\tLogical Volume Identifier: %.128s\n")
		_T("\t      Logical Block Size : %d\n")
		_T("\tDomain Identifier\n"),
		str, 
		MAKELONG(MAKEWORD(buf[idx+212], buf[idx+213]), 
			MAKEWORD(buf[idx+214], buf[idx+215])));
	
	OutputCharspec(idx + 216, buf, fpLog);
	OutputLongAllocationDescriptor(idx + 248, buf, fpLog);

	LONG MT_L = MAKELONG(MAKEWORD(buf[idx+264], buf[idx+265]), 
		MAKEWORD(buf[idx+266], buf[idx+267]));
	OutputLogString(fpLog,
		_T("\t        Map Table Length: %d\n")
		_T("\tNumber of Partition Maps: %d\n")
		_T("\tImplementation Identifier\n"),
		MT_L, 
		MAKELONG(MAKEWORD(buf[idx+268], buf[idx+269]), 
		MAKEWORD(buf[idx+270], buf[idx+271])));
	
	OutputRegid(idx + 272, buf, fpLog);

	OutputLogString(fpLog, _T("\tImplementation Use: "));
	for(INT i = 304; i <= 431; i++) {
		OutputLogString(fpLog, _T("%x"), buf[idx+i]);
	}
	OutputLogString(fpLog, _T("\n"));
	OutputLogString(fpLog, _T("\tIntegrity Sequence Extent\n"));
	OutputExtentDescriptor(idx + 432, buf, fpLog);

	OutputLogString(fpLog, _T("\tPartition Maps: "));
	for(INT i = 0; i < MT_L; i++) {
		OutputLogString(fpLog, _T("%x"), buf[idx+440+i]);
	}
	OutputLogString(fpLog, _T("\n"));
}

void OutputUnallocatedSpaceDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	LONG N_AD = MAKELONG(MAKEWORD(buf[idx+20], buf[idx+21]), 
		MAKEWORD(buf[idx+22], buf[idx+23]));
	OutputLogString(fpLog, 
		_T("\tVolume Descriptor Sequence Number: %d\n")
		_T("\t Number of Allocation Descriptors: %d\n")
		_T("\tAllocation Descriptors\n"), 
		MAKELONG(MAKEWORD(buf[idx+16], buf[idx+17]), 
			MAKEWORD(buf[idx+18], buf[idx+19])),
		N_AD);
	for(INT i = 0; i < N_AD * 8; i+=8) {
		OutputExtentDescriptor(idx + 24 + i, buf, fpLog);
	}
}

void OutputVolumeDescriptorSequence(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	)
{
	USHORT usTagId = MAKEWORD(buf[idx], buf[idx+1]);
	if(usTagId == 0 || (10 <= usTagId && usTagId <= 255) || 267 <= usTagId) {
		return;
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

	OutputLogString(fpLog,
		_T("\t\t           Descriptor Version: %d\n")
		_T("\t\t                 Tag Checksum: %d\n")
		_T("\t\t            Tag Serial Number: %d\n")
		_T("\t\t               Descriptor CRC: %x\n")
		_T("\t\t        Descriptor CRC Length: %d\n")
		_T("\t\t                 Tag Location: %d\n"), 
		MAKEWORD(buf[idx+2], buf[idx+3]),
		buf[idx+4], 
		MAKEWORD(buf[idx+6], buf[idx+7]), 
		MAKEWORD(buf[idx+8], buf[idx+9]), 
		MAKEWORD(buf[idx+10], buf[idx+11]),
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
	return;
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
	_ftprintf(fpCcd,
		_T("[CloneCD]\n")
		_T("Version=3\n")
		_T("[Disc]\n")
		_T("TocEntries=%d\n")
		_T("Sessions=%d\n")
		_T("DataTracksScrambled=%d\n"),
		tocEntries,
		LastCompleteSession,
		0); // TODO
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
	_ftprintf(fpCcd,
		_T("[CDText]\n")
		_T("Entries=%d\n"),
		cdTextSize);
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
	_ftprintf(fpCcd,
		_T("PreGapMode=%d\n")
		_T("PreGapSubC=%d\n"),
		mode,
		0);	// TODO
}

void WriteCcdFileForEntry(
	size_t a,
	CONST CDROM_TOC_FULL_TOC_DATA_BLOCK* toc,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd,
		_T("[Entry %d]\n")
		_T("Session=%d\n")
		_T("Point=0x%02x\n")
		_T("ADR=0x%02x\n")
		_T("Control=0x%02x\n")
		_T("TrackNo=%d\n")
		_T("AMin=%d\n")
		_T("ASec=%d\n")
		_T("AFrame=%d\n")
		_T("ALBA=%d\n")
		_T("Zero=%d\n")
		_T("PMin=%d\n")
		_T("PSec=%d\n")
		_T("PFrame=%d\n")
		_T("PLBA=%d\n"),
		a,
		toc[a].SessionNumber,
		toc[a].Point,
		toc[a].Adr,
		toc[a].Control,
		toc[a].Reserved1,
		toc[a].MsfExtra[0],
		toc[a].MsfExtra[1],
		toc[a].MsfExtra[2],
		MSFtoLBA(toc[a].MsfExtra[2], toc[a].MsfExtra[1], toc[a].MsfExtra[0]) - 150,
		toc[a].Zero,
		toc[a].Msf[0],
		toc[a].Msf[1],
		toc[a].Msf[2], 
		MSFtoLBA(toc[a].Msf[2], toc[a].Msf[1], toc[a].Msf[0]) - 150);
}

void WriteCcdFileForTrack(
	INT nTrackNum,
	UCHAR byModeNum,
	BOOL bISRC,
	FILE* fpCcd
	)
{
	_ftprintf(fpCcd,
		_T("[TRACK %d]\n")
		_T("MODE=%d\n"),
		nTrackNum,
		byModeNum);
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
	BOOL bCDG,
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
		if(bCDG) {
			_ftprintf(fpCue, _T("  TRACK %02d CDG\n"), nTrackNum);
		}
		else {
			_ftprintf(fpCue, _T("  TRACK %02d AUDIO\n"), nTrackNum);
		}
		if(bISRC) {
			_ftprintf(fpCue, _T("    ISRC %s\n"), szISRC[nTrackNum-1]);
		}
		if(_tcslen(szTitle[nTrackNum-1]) > 0) {
			_ftprintf(fpCue, _T("    TITLE \"%s\"\n"), szTitle[nTrackNum-1]);
		}
		if((byCtl & AUDIO_WITH_PREEMPHASIS) == AUDIO_WITH_PREEMPHASIS ||
			(byCtl & DIGITAL_COPY_PERMITTED) == DIGITAL_COPY_PERMITTED ||
			(byCtl & TWO_FOUR_CHANNEL_AUDIO) == TWO_FOUR_CHANNEL_AUDIO) {
			_TCHAR aBuf[22] = {0};
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

void WriteMainChannel(
	PDISC_DATA pDiscData,
	INT nLBA,
	INT nFixStartLBA,
	INT nFixEndLBA,
	size_t uiShift,
	PINT* aLBAStart,
	PUCHAR pBuf,
	FILE* fpImg
	)
{
	if(nFixStartLBA <= nLBA && nLBA < pDiscData->nLength + nFixEndLBA) {
		// first sector
		if(nLBA == nFixStartLBA) {
			fwrite(pBuf + uiShift, sizeof(UCHAR), 
				CD_RAW_SECTOR_SIZE - uiShift, fpImg);
			aLBAStart[0][0] = -150;
			aLBAStart[0][1] = nLBA - nFixStartLBA;
		}
		// last sector
		else if(nLBA == pDiscData->nLength + nFixEndLBA - 1) {
			fwrite(pBuf, sizeof(UCHAR), uiShift, fpImg);
		}
		else {
			if(pDiscData->nStartLBAof2ndSession != -1 &&
				nLBA == pDiscData->nStartLBAof2ndSession) {
				if(pDiscData->nCombinedOffset > 0) {
					ZeroMemory(pBuf, (size_t)pDiscData->nCombinedOffset);
				}
				else if(pDiscData->nCombinedOffset < 0) {
					// todo
				}
			}
			fwrite(pBuf, sizeof(UCHAR), CD_RAW_SECTOR_SIZE, fpImg);
		}
	}
}

void WriteSubChannel(
	INT nLBA,
	UCHAR byCurrentTrackNum,
	PUCHAR pBuf,
	PUCHAR Subcode,
	PUCHAR SubcodeRaw,
	FILE* fpSub,
	FILE* fpParse,
	FILE* fpCdg
	)
{
	fwrite(Subcode, sizeof(UCHAR), CD_RAW_READ_SUBCODE_SIZE, fpSub);
	OutputSubcode(nLBA, byCurrentTrackNum, 
		Subcode, pBuf + CD_RAW_SECTOR_SIZE, fpParse);
	if(fpCdg != NULL) {
		fwrite(SubcodeRaw, sizeof(UCHAR), CD_RAW_READ_SUBCODE_SIZE, fpCdg);
	}
}
