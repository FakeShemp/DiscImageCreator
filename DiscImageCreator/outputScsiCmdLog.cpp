/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "convert.h"
#include "output.h"
#include "outputScsiCmdLog.h"
#include "set.h"

VOID OutputInquiry(
	PDEVICE pDevice,
	PINQUIRYDATA pInquiry
	)
{
	OutputDriveLogA(
		"================================= InquiryData =================================\n"
		"\t          DeviceType: ");
	switch (pInquiry->DeviceType) {
	case DIRECT_ACCESS_DEVICE:
		OutputDriveLogA("DirectAccessDevice (Floppy etc)\n");
		break;
	case READ_ONLY_DIRECT_ACCESS_DEVICE:
		OutputDriveLogA("ReadOnlyDirectAccessDevice (CD/DVD etc)\n");
		break;
	case OPTICAL_DEVICE:
		OutputDriveLogA("OpticalDisk\n");
		break;
	default:
		OutputDriveLogA("OtherDevice\n");
		break;
	}
	OutputDriveLogA(
		"\t DeviceTypeQualifier: ");
	switch (pInquiry->DeviceTypeQualifier) {
	case DEVICE_QUALIFIER_ACTIVE:
		OutputDriveLogA("Active\n");
		break;
	case DEVICE_QUALIFIER_NOT_ACTIVE:
		OutputDriveLogA("NotActive\n");
		break;
	case DEVICE_QUALIFIER_NOT_SUPPORTED:
		OutputDriveLogA("NotSupported\n");
		break;
	default:
		OutputDriveLogA("\n");
		break;
	}

	OutputDriveLogA(
		"\t  DeviceTypeModifier: %u\n"
		"\t      RemovableMedia: %s\n"
		"\t            Versions: %u\n"
		"\t  ResponseDataFormat: %u\n"
		"\t           HiSupport: %s\n"
		"\t             NormACA: %s\n"
		"\t       TerminateTask: %s\n"
		"\t                AERC: %s\n"
		"\t    AdditionalLength: %u\n"
		"\t       MediumChanger: %s\n"
		"\t           MultiPort: %s\n"
		"\t   EnclosureServices: %s\n"
		"\t           SoftReset: %s\n"
		"\t        CommandQueue: %s\n"
		"\t      LinkedCommands: %s\n"
		"\t  RelativeAddressing: %s\n",
		pInquiry->DeviceTypeModifier,
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->RemovableMedia),
		pInquiry->Versions,
		pInquiry->ResponseDataFormat,
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->HiSupport),
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->NormACA),
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->TerminateTask),
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->AERC),
		pInquiry->AdditionalLength,
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->MediumChanger),
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->MultiPort),
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->EnclosureServices),
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->SoftReset),
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->CommandQueue),
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->LinkedCommands),
		BOOLEAN_TO_STRING_YES_NO_A(pInquiry->RelativeAddressing));

	strncpy(pDevice->szVendorId,
		(PCHAR)pInquiry->VendorId, sizeof(pInquiry->VendorId));
	strncpy(pDevice->szProductId,
		(PCHAR)pInquiry->ProductId, sizeof(pInquiry->ProductId));
	OutputDriveLogA(
		"\t            VendorId: %.8s\n"
		"\t           ProductId: %.16s\n"
		"\tProductRevisionLevel: %.4s\n"
		"\t      VendorSpecific: %.20s\n",
		pInquiry->VendorId,
		pInquiry->ProductId,
		pInquiry->ProductRevisionLevel,
		pInquiry->VendorSpecific);
}

VOID OutputGetConfigurationHeader(
	PGET_CONFIGURATION_HEADER pConfigHeader
	)
{
	OutputDriveLogA(
		"============================== GetConfiguration ===============================\n"
		"\t    DataLength: %d\n"
		"\tCurrentProfile: "
		, MAKELONG(MAKEWORD(pConfigHeader->DataLength[3], pConfigHeader->DataLength[2]),
		MAKEWORD(pConfigHeader->DataLength[1], pConfigHeader->DataLength[0])));
	OutputGetConfigurationFeatureProfileType(
		MAKEWORD(pConfigHeader->CurrentProfile[1], pConfigHeader->CurrentProfile[0]));
	OutputDriveLogA("\n");
}

VOID OutputGetConfigurationFeatureProfileType(
	WORD wFeatureProfileType
	)
{
	switch (wFeatureProfileType) {
	case ProfileInvalid:
		OutputDriveLogA("Invalid");
		break;
	case ProfileNonRemovableDisk:
		OutputDriveLogA("NonRemovableDisk");
		break;
	case ProfileRemovableDisk:
		OutputDriveLogA("RemovableDisk");
		break;
	case ProfileMOErasable:
		OutputDriveLogA("MOErasable");
		break;
	case ProfileMOWriteOnce:
		OutputDriveLogA("MOWriteOnce");
		break;
	case ProfileAS_MO:
		OutputDriveLogA("AS_MO");
		break;
	case ProfileCdrom:
		OutputDriveLogA("CD-ROM");
		break;
	case ProfileCdRecordable:
		OutputDriveLogA("CD-R");
		break;
	case ProfileCdRewritable:
		OutputDriveLogA("CD-RW");
		break;
	case ProfileDvdRom:
		OutputDriveLogA("DVD-ROM");
		break;
	case ProfileDvdRecordable:
		OutputDriveLogA("DVD-R");
		break;
	case ProfileDvdRam:
		OutputDriveLogA("DVD-RAM");
		break;
	case ProfileDvdRewritable:
		OutputDriveLogA("DVD-RW");
		break;
	case ProfileDvdRWSequential:
		OutputDriveLogA("DVD-RW Sequential");
		break;
	case ProfileDvdDashRDualLayer:
		OutputDriveLogA("DVD-R DL");
		break;
	case ProfileDvdDashRLayerJump:
		OutputDriveLogA("DVD-R Layer Jump");
		break;
	case ProfileDvdPlusRW:
		OutputDriveLogA("DVD+RW");
		break;
	case ProfileDvdPlusR:
		OutputDriveLogA("DVD+R");
		break;
	case ProfileDDCdrom:
		OutputDriveLogA("DDCD-ROM");
		break;
	case ProfileDDCdRecordable:
		OutputDriveLogA("DDCD-R");
		break;
	case ProfileDDCdRewritable:
		OutputDriveLogA("DDCD-RW");
		break;
	case ProfileDvdPlusRWDualLayer:
		OutputDriveLogA("DVD+RW DL");
		break;
	case ProfileDvdPlusRDualLayer:
		OutputDriveLogA("DVD+R DL");
		break;
	case ProfileBDRom:
		OutputDriveLogA("BD-ROM");
		break;
	case ProfileBDRSequentialWritable:
		OutputDriveLogA("BD-R Sequential Writable");
		break;
	case ProfileBDRRandomWritable:
		OutputDriveLogA("BD-R Random Writable");
		break;
	case ProfileBDRewritable:
		OutputDriveLogA("BD-R");
		break;
	case ProfileHDDVDRom:
		OutputDriveLogA("HD DVD-ROM");
		break;
	case ProfileHDDVDRecordable:
		OutputDriveLogA("HD DVD-R");
		break;
	case ProfileHDDVDRam:
		OutputDriveLogA("HD DVD-RAM");
		break;
	case ProfileHDDVDRewritable:
		OutputDriveLogA("HD-DVD-RW");
		break;
	case ProfileHDDVDRDualLayer:
		OutputDriveLogA("HD-DVD-R DL");
		break;
	case ProfileHDDVDRWDualLayer:
		OutputDriveLogA("HD-DVD-RW DL");
		break;
	case ProfileNonStandard:
		OutputDriveLogA("NonStandard");
		break;
	default:
		OutputDriveLogA("Reserved [%#x]", wFeatureProfileType);
		break;
	}
}

VOID OutputGetConfigurationFeatureProfileList(
	PFEATURE_DATA_PROFILE_LIST pList
	)
{
	OutputDriveLogA("\tFeatureProfileList\n");
	for (UINT i = 0; i < pList->Header.AdditionalLength / sizeof(FEATURE_DATA_PROFILE_LIST_EX); i++) {
		OutputDriveLogA("\t\t");
		OutputGetConfigurationFeatureProfileType(
			MAKEWORD(pList->Profiles[i].ProfileNumber[1], pList->Profiles[i].ProfileNumber[0]));
		OutputDriveLogA("\n");
	}
}

VOID OutputGetConfigurationFeatureCore(
	PFEATURE_DATA_CORE pCore
	)
{
	OutputDriveLogA(
		"\tFeatureCore\n"
		"\t\tPhysicalInterface: ");
	LONG lVal = MAKELONG(
		MAKEWORD(pCore->PhysicalInterface[3], pCore->PhysicalInterface[2]),
		MAKEWORD(pCore->PhysicalInterface[1], pCore->PhysicalInterface[0]));
	switch (lVal) {
	case 0:
		OutputDriveLogA("Unspecified\n");
		break;
	case 1:
		OutputDriveLogA("SCSI Family\n");
		break;
	case 2:
		OutputDriveLogA("ATAPI\n");
		break;
	case 3:
		OutputDriveLogA("IEEE 1394 - 1995\n");
		break;
	case 4:
		OutputDriveLogA("IEEE 1394A\n");
		break;
	case 5:
		OutputDriveLogA("Fibre Channel\n");
		break;
	case 6:
		OutputDriveLogA("IEEE 1394B\n");
		break;
	case 7:
		OutputDriveLogA("Serial ATAPI\n");
		break;
	case 8:
		OutputDriveLogA("USB (both 1.1 and 2.0)\n");
		break;
	case 0xffff:
		OutputDriveLogA("Vendor Unique\n");
		break;
	default:
		OutputDriveLogA("Reserved: %08d\n", lVal);
		break;
	}
	OutputDriveLogA(
		"\t\t  DeviceBusyEvent: %s\n"
		"\t\t         INQUIRY2: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pCore->DeviceBusyEvent),
		BOOLEAN_TO_STRING_YES_NO_A(pCore->INQUIRY2));
}

VOID OutputGetConfigurationFeatureMorphing(
	PFEATURE_DATA_MORPHING pMorphing
	)
{
	OutputDriveLogA(
		"\tFeatureMorphing\n"
		"\t\tAsynchronous: %s\n"
		"\t\t     OCEvent: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pMorphing->Asynchronous),
		BOOLEAN_TO_STRING_YES_NO_A(pMorphing->OCEvent));
}

VOID OutputGetConfigurationFeatureRemovableMedium(
	PFEATURE_DATA_REMOVABLE_MEDIUM pRemovableMedium
	)
{
	OutputDriveLogA(
		"\tFeatureRemovableMedium\n"
		"\t\t        Lockable: %s\n"
		"\t\tDefaultToPrevent: %s\n"
		"\t\t           Eject: %s\n"
		"\t\tLoadingMechanism: ",
		BOOLEAN_TO_STRING_YES_NO_A(pRemovableMedium->Lockable),
		BOOLEAN_TO_STRING_YES_NO_A(pRemovableMedium->DefaultToPrevent),
		BOOLEAN_TO_STRING_YES_NO_A(pRemovableMedium->Eject));
	switch (pRemovableMedium->LoadingMechanism) {
	case 0:
		OutputDriveLogA("Caddy/Slot type loading mechanism\n");
		break;
	case 1:
		OutputDriveLogA("Tray type loading mechanism\n");
		break;
	case 2:
		OutputDriveLogA("Pop-up type loading mechanism\n");
		break;
	case 4:
		OutputDriveLogA(
			"Embedded changer with individually changeable discs\n");
		break;
	case 5:
		OutputDriveLogA(
			"Embedded changer using a magazine mechanism\n");
		break;
	default:
		OutputDriveLogA(
			"Reserved: %08d\n", pRemovableMedium->LoadingMechanism);
		break;
	}
}

VOID OutputGetConfigurationFeatureWriteProtect(
	PFEATURE_DATA_WRITE_PROTECT pWriteProtect
	)
{
	OutputDriveLogA(
		"\tFeatureWriteProtect\n"
		"\t\t               SupportsSWPPBit: %s\n"
		"\t\tSupportsPersistentWriteProtect: %s\n"
		"\t\t               WriteInhibitDCB: %s\n"
		"\t\t           DiscWriteProtectPAC: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pWriteProtect->SupportsSWPPBit),
		BOOLEAN_TO_STRING_YES_NO_A(pWriteProtect->SupportsPersistentWriteProtect),
		BOOLEAN_TO_STRING_YES_NO_A(pWriteProtect->WriteInhibitDCB),
		BOOLEAN_TO_STRING_YES_NO_A(pWriteProtect->DiscWriteProtectPAC));
}

VOID OutputGetConfigurationFeatureRandomReadable(
	PFEATURE_DATA_RANDOM_READABLE pRandomReadable
	)
{
	OutputDriveLogA(
		"\tFeatureRandomReadable\n"
		"\t\t        LogicalBlockSize: %u\n"
		"\t\t                Blocking: %u\n"
		"\t\tErrorRecoveryPagePresent: %s\n",
		MAKELONG(MAKEWORD(pRandomReadable->LogicalBlockSize[3], pRandomReadable->LogicalBlockSize[2]),
			MAKEWORD(pRandomReadable->LogicalBlockSize[1], pRandomReadable->LogicalBlockSize[0])),
		MAKEWORD(pRandomReadable->Blocking[1], pRandomReadable->Blocking[0]),
		BOOLEAN_TO_STRING_YES_NO_A(pRandomReadable->ErrorRecoveryPagePresent));
}

VOID OutputGetConfigurationFeatureMultiRead(
	PFEATURE_DATA_MULTI_READ pMultiRead
	)
{
	OutputDriveLogA(
		"\tFeatureMultiRead\n"
		"\t\t   Current: %u\n"
		"\t\tPersistent: %u\n"
		"\t\t   Version: %u\n",
		pMultiRead->Header.Current,
		pMultiRead->Header.Persistent,
		pMultiRead->Header.Version);
}

VOID OutputGetConfigurationFeatureCdRead(
	PFEATURE_DATA_CD_READ pCDRead
	)
{
	OutputDriveLogA(
		"\tFeatureCdRead\n"
		"\t\t          CDText: %s\n"
		"\t\t     C2ErrorData: %s\n"
		"\t\tDigitalAudioPlay: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pCDRead->CDText),
		BOOLEAN_TO_STRING_YES_NO_A(pCDRead->C2ErrorData),
		BOOLEAN_TO_STRING_YES_NO_A(pCDRead->DigitalAudioPlay));
}

VOID OutputGetConfigurationFeatureDvdRead(
	PFEATURE_DATA_DVD_READ pDVDRead
	)
{
	OutputDriveLogA(
		"\tFeatureDvdRead\n"
		"\t\t  Multi110: %s\n"
		"\t\t DualDashR: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pDVDRead->Multi110),
		BOOLEAN_TO_STRING_YES_NO_A(pDVDRead->DualDashR));
}

VOID OutputGetConfigurationFeatureRandomWritable(
	PFEATURE_DATA_RANDOM_WRITABLE pRandomWritable
	)
{
	OutputDriveLogA(
		"\tFeatureRandomWritable\n"
		"\t\t                 LastLBA: %u\n"
		"\t\t        LogicalBlockSize: %u\n"
		"\t\t                Blocking: %u\n"
		"\t\tErrorRecoveryPagePresent: %s\n",
		MAKELONG(MAKEWORD(pRandomWritable->LastLBA[3], pRandomWritable->LastLBA[2]),
			MAKEWORD(pRandomWritable->LastLBA[1], pRandomWritable->LastLBA[0])),
		MAKELONG(MAKEWORD(pRandomWritable->LogicalBlockSize[3], pRandomWritable->LogicalBlockSize[2]),
			MAKEWORD(pRandomWritable->LogicalBlockSize[1], pRandomWritable->LogicalBlockSize[0])),
		MAKEWORD(pRandomWritable->Blocking[1], pRandomWritable->Blocking[0]),
		BOOLEAN_TO_STRING_YES_NO_A(pRandomWritable->ErrorRecoveryPagePresent));
}

VOID OutputGetConfigurationFeatureIncrementalStreamingWritable(
	PFEATURE_DATA_INCREMENTAL_STREAMING_WRITABLE pIncremental
	)
{
	OutputDriveLogA(
		"\tFeatureIncrementalStreamingWritable\n"
		"\t\t        DataTypeSupported: %u\n"
		"\t\t       BufferUnderrunFree: %s\n"
		"\t\t   AddressModeReservation: %s\n"
		"\t\tTrackRessourceInformation: %s\n"
		"\t\t        NumberOfLinkSizes: %u\n",
		MAKEWORD(pIncremental->DataTypeSupported[1], pIncremental->DataTypeSupported[0]),
		BOOLEAN_TO_STRING_YES_NO_A(pIncremental->BufferUnderrunFree),
		BOOLEAN_TO_STRING_YES_NO_A(pIncremental->AddressModeReservation),
		BOOLEAN_TO_STRING_YES_NO_A(pIncremental->TrackRessourceInformation),
		pIncremental->NumberOfLinkSizes);
	for (INT i = 0; i < pIncremental->NumberOfLinkSizes; i++) {
		OutputDriveLogA(
			"\t\tLinkSize%u: %u\n", i, pIncremental->LinkSize[i]);
	}
}

VOID OutputGetConfigurationFeatureSectorErasable(
	PFEATURE_DATA_SECTOR_ERASABLE pSectorErasable
	)
{
	OutputDriveLogA(
		"\tFeatureSectorErasable\n"
		"\t\t   Current: %u\n"
		"\t\tPersistent: %u\n"
		"\t\t   Version: %u\n",
		pSectorErasable->Header.Current,
		pSectorErasable->Header.Persistent,
		pSectorErasable->Header.Version);
}

VOID OutputGetConfigurationFeatureFormattable(
	PFEATURE_DATA_FORMATTABLE pFormattable
	)
{
	OutputDriveLogA(
		"\tFeatureFormattable\n"
		"\t\t FullCertification: %s\n"
		"\t\tQuickCertification: %s\n"
		"\t\tSpareAreaExpansion: %s\n"
		"\t\tRENoSpareAllocated: %s\n"
		"\t\t   RRandomWritable: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pFormattable->FullCertification),
		BOOLEAN_TO_STRING_YES_NO_A(pFormattable->QuickCertification),
		BOOLEAN_TO_STRING_YES_NO_A(pFormattable->SpareAreaExpansion),
		BOOLEAN_TO_STRING_YES_NO_A(pFormattable->RENoSpareAllocated),
		BOOLEAN_TO_STRING_YES_NO_A(pFormattable->RRandomWritable));
}

VOID OutputGetConfigurationFeatureDefectManagement(
	PFEATURE_DATA_DEFECT_MANAGEMENT pDefect
	)
{
	OutputDriveLogA(
		"\tFeatureDefectManagement\n"
		"\t\tSupplimentalSpareArea: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pDefect->SupplimentalSpareArea));
}

VOID OutputGetConfigurationFeatureWriteOnce(
	PFEATURE_DATA_WRITE_ONCE pWriteOnce
	)
{
	OutputDriveLogA(
		"\tFeatureWriteOnce\n"
		"\t\t        LogicalBlockSize: %u\n"
		"\t\t                Blocking: %u\n"
		"\t\tErrorRecoveryPagePresent: %s\n",
		MAKELONG(MAKEWORD(pWriteOnce->LogicalBlockSize[3], pWriteOnce->LogicalBlockSize[2]),
			MAKEWORD(pWriteOnce->LogicalBlockSize[1], pWriteOnce->LogicalBlockSize[0])),
		MAKEWORD(pWriteOnce->Blocking[1], pWriteOnce->Blocking[0]),
		BOOLEAN_TO_STRING_YES_NO_A(pWriteOnce->ErrorRecoveryPagePresent));
}

VOID OutputGetConfigurationFeatureRestrictedOverwrite(
	PFEATURE_DATA_RESTRICTED_OVERWRITE pRestricted
	)
{
	OutputDriveLogA(
		"\tFeatureRestrictedOverwrite\n"
		"\t\t   Current: %u\n"
		"\t\tPersistent: %u\n"
		"\t\t   Version: %u\n",
		pRestricted->Header.Current,
		pRestricted->Header.Persistent,
		pRestricted->Header.Version);
}

VOID OutputGetConfigurationFeatureCdrwCAVWrite(
	PFEATURE_DATA_CDRW_CAV_WRITE pCDRW
	)
{
	OutputDriveLogA(
		"\tFeatureCdrwCAVWrite\n"
		"\t\t   Current: %u\n"
		"\t\tPersistent: %u\n"
		"\t\t   Version: %u\n",
		pCDRW->Header.Current,
		pCDRW->Header.Persistent,
		pCDRW->Header.Version);
}

VOID OutputGetConfigurationFeatureMrw(
	PFEATURE_DATA_MRW pMrw
	)
{
	OutputDriveLogA(
		"\tFeatureMrw\n"
		"\t\t       Write: %s\n"
		"\t\t DvdPlusRead: %s\n"
		"\t\tDvdPlusWrite: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pMrw->Write),
		BOOLEAN_TO_STRING_YES_NO_A(pMrw->DvdPlusRead),
		BOOLEAN_TO_STRING_YES_NO_A(pMrw->DvdPlusWrite));
}

VOID OutputGetConfigurationFeatureEnhancedDefectReporting(
	PFEATURE_ENHANCED_DEFECT_REPORTING pEnhanced
	)
{
	OutputDriveLogA(
		"\tFeatureEnhancedDefectReporting\n"
		"\t\t       DRTDMSupported: %s\n"
		"\t\tNumberOfDBICacheZones: %u\n"
		"\t\t      NumberOfEntries: %u\n",
		BOOLEAN_TO_STRING_YES_NO_A(pEnhanced->DRTDMSupported),
		pEnhanced->NumberOfDBICacheZones,
		MAKEWORD(pEnhanced->NumberOfEntries[1], pEnhanced->NumberOfEntries[0]));
}

VOID OutputGetConfigurationFeatureDvdPlusRW(
	PFEATURE_DATA_DVD_PLUS_RW pDVDPLUSRW
	)
{
	OutputDriveLogA(
		"\tFeatureDvdPlusRW\n"
		"\t\t     Write: %s\n"
		"\t\t CloseOnly: %s\n"
		"\t\tQuickStart: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pDVDPLUSRW->Write),
		BOOLEAN_TO_STRING_YES_NO_A(pDVDPLUSRW->CloseOnly),
		BOOLEAN_TO_STRING_YES_NO_A(pDVDPLUSRW->QuickStart));
}

VOID OutputGetConfigurationFeatureDvdPlusR(
	PFEATURE_DATA_DVD_PLUS_R pDVDPLUSR
	)
{
	OutputDriveLogA(
		"\tFeatureDvdPlusR\n"
		"\t\tWrite: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pDVDPLUSR->Write));
}

VOID OutputGetConfigurationFeatureRigidRestrictedOverwrite(
	PFEATURE_DATA_DVD_RW_RESTRICTED_OVERWRITE pDVDRWRestricted
	)
{
	OutputDriveLogA(
		"\tFeatureRigidRestrictedOverwrite\n"
		"\t\t                   Blank: %s\n"
		"\t\t            Intermediate: %s\n"
		"\t\t    DefectStatusDataRead: %s\n"
		"\t\tDefectStatusDataGenerate: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pDVDRWRestricted->Blank),
		BOOLEAN_TO_STRING_YES_NO_A(pDVDRWRestricted->Intermediate),
		BOOLEAN_TO_STRING_YES_NO_A(pDVDRWRestricted->DefectStatusDataRead),
		BOOLEAN_TO_STRING_YES_NO_A(pDVDRWRestricted->DefectStatusDataGenerate));
}

VOID OutputGetConfigurationFeatureCdTrackAtOnce(
	PFEATURE_DATA_CD_TRACK_AT_ONCE pCDTrackAtOnce
	)
{
	OutputDriveLogA(
		"\tFeatureCdTrackAtOnce\n"
		"\t\tRWSubchannelsRecordable: %s\n"
		"\t\t           CdRewritable: %s\n"
		"\t\t            TestWriteOk: %s\n"
		"\t\t   RWSubchannelPackedOk: %s\n"
		"\t\t      RWSubchannelRawOk: %s\n"
		"\t\t     BufferUnderrunFree: %s\n"
		"\t\t      DataTypeSupported: %u\n",
		BOOLEAN_TO_STRING_YES_NO_A(pCDTrackAtOnce->RWSubchannelsRecordable),
		BOOLEAN_TO_STRING_YES_NO_A(pCDTrackAtOnce->CdRewritable),
		BOOLEAN_TO_STRING_YES_NO_A(pCDTrackAtOnce->TestWriteOk),
		BOOLEAN_TO_STRING_YES_NO_A(pCDTrackAtOnce->RWSubchannelPackedOk),
		BOOLEAN_TO_STRING_YES_NO_A(pCDTrackAtOnce->RWSubchannelRawOk),
		BOOLEAN_TO_STRING_YES_NO_A(pCDTrackAtOnce->BufferUnderrunFree),
		MAKEWORD(pCDTrackAtOnce->DataTypeSupported[1], pCDTrackAtOnce->DataTypeSupported[0]));
}

VOID OutputGetConfigurationFeatureCdMastering(
	PFEATURE_DATA_CD_MASTERING pCDMastering
	)
{
	OutputDriveLogA(
		"\tFeatureCdMastering\n"
		"\t\tRWSubchannelsRecordable: %s\n"
		"\t\t           CdRewritable: %s\n"
		"\t\t            TestWriteOk: %s\n"
		"\t\t        RRawRecordingOk: %s\n"
		"\t\t      RawMultiSessionOk: %s\n"
		"\t\t        SessionAtOnceOk: %s\n"
		"\t\t     BufferUnderrunFree: %s\n"
		"\t\t  MaximumCueSheetLength: %u\n",
		BOOLEAN_TO_STRING_YES_NO_A(pCDMastering->RWSubchannelsRecordable),
		BOOLEAN_TO_STRING_YES_NO_A(pCDMastering->CdRewritable),
		BOOLEAN_TO_STRING_YES_NO_A(pCDMastering->TestWriteOk),
		BOOLEAN_TO_STRING_YES_NO_A(pCDMastering->RawRecordingOk),
		BOOLEAN_TO_STRING_YES_NO_A(pCDMastering->RawMultiSessionOk),
		BOOLEAN_TO_STRING_YES_NO_A(pCDMastering->SessionAtOnceOk),
		BOOLEAN_TO_STRING_YES_NO_A(pCDMastering->BufferUnderrunFree),
		MAKELONG(MAKEWORD(0, pCDMastering->MaximumCueSheetLength[2]),
			MAKEWORD(pCDMastering->MaximumCueSheetLength[1], pCDMastering->MaximumCueSheetLength[0])));
}

VOID OutputGetConfigurationFeatureDvdRecordableWrite(
	PFEATURE_DATA_DVD_RECORDABLE_WRITE pDVDRecordable
	)
{
	OutputDriveLogA(
		"\tFeatureDvdRecordableWrite\n"
		"\t\t            DVD_RW: %s\n"
		"\t\t         TestWrite: %s\n"
		"\t\t        RDualLayer: %s\n"
		"\t\tBufferUnderrunFree: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pDVDRecordable->DVD_RW),
		BOOLEAN_TO_STRING_YES_NO_A(pDVDRecordable->TestWrite),
		BOOLEAN_TO_STRING_YES_NO_A(pDVDRecordable->RDualLayer),
		BOOLEAN_TO_STRING_YES_NO_A(pDVDRecordable->BufferUnderrunFree));
}

VOID OutputGetConfigurationFeatureLayerJumpRecording(
	PFEATURE_DATA_LAYER_JUMP_RECORDING pLayerJumpRec
	)
{
	OutputDriveLogA(
		"\tFeatureLayerJumpRecording\n"
		"\t\tNumberOfLinkSizes: %u\n",
		pLayerJumpRec->NumberOfLinkSizes);
	for (INT i = 0; i < pLayerJumpRec->NumberOfLinkSizes; i++) {
		OutputDriveLogA(
			"\t\tLinkSize %u: %u\n", i, pLayerJumpRec->LinkSizes[i]);
	}
}

VOID OutputGetConfigurationFeatureCDRWMediaWriteSupport(
	PFEATURE_CD_RW_MEDIA_WRITE_SUPPORT pCDRWMediaWrite
	)
{
	OutputDriveLogA(
		"\tFeatureCDRWMediaWriteSupport\n"
		"\t\tSubtype 0: %s\n"
		"\t\tSubtype 1: %s\n"
		"\t\tSubtype 2: %s\n"
		"\t\tSubtype 3: %s\n"
		"\t\tSubtype 4: %s\n"
		"\t\tSubtype 5: %s\n"
		"\t\tSubtype 6: %s\n"
		"\t\tSubtype 7: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype0),
		BOOLEAN_TO_STRING_YES_NO_A(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype1),
		BOOLEAN_TO_STRING_YES_NO_A(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype2),
		BOOLEAN_TO_STRING_YES_NO_A(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype3),
		BOOLEAN_TO_STRING_YES_NO_A(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype4),
		BOOLEAN_TO_STRING_YES_NO_A(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype5),
		BOOLEAN_TO_STRING_YES_NO_A(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype6),
		BOOLEAN_TO_STRING_YES_NO_A(pCDRWMediaWrite->CDRWMediaSubtypeSupport.Subtype7));
}

VOID OutputGetConfigurationFeatureDvdPlusRWDualLayer(
	PFEATURE_DATA_DVD_PLUS_RW_DUAL_LAYER pDVDPlusRWDL
	)
{
	OutputDriveLogA(
		"\tFeatureDvdPlusRWDualLayer\n"
		"\t\t     Write: %s\n"
		"\t\t CloseOnly: %s\n"
		"\t\tQuickStart: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pDVDPlusRWDL->Write),
		BOOLEAN_TO_STRING_YES_NO_A(pDVDPlusRWDL->CloseOnly),
		BOOLEAN_TO_STRING_YES_NO_A(pDVDPlusRWDL->QuickStart));
}

VOID OutputGetConfigurationFeatureDvdPlusRDualLayer(
	PFEATURE_DATA_DVD_PLUS_R_DUAL_LAYER pDVDPlusRDL
	)
{
	OutputDriveLogA(
		"\tFeatureDvdPlusRDualLayer\n"
		"\t\tWrite: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pDVDPlusRDL->Write));
}

VOID OutputGetConfigurationFeatureHybridDisc(
	PFEATURE_HYBRID_DISC pHybridDisc
	)
{
	OutputDriveLogA(
		"\tFeatureHybridDisc\n"
		"\t\tResetImmunity: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pHybridDisc->ResetImmunity));
}

VOID OutputGetConfigurationFeaturePowerManagement(
	PFEATURE_DATA_POWER_MANAGEMENT pPower
	)
{
	OutputDriveLogA(
		"\tFeaturePowerManagement\n"
		"\t\t   Current: %u\n"
		"\t\tPersistent: %u\n"
		"\t\t   Version: %u\n",
		pPower->Header.Current,
		pPower->Header.Persistent,
		pPower->Header.Version);
}

VOID OutputGetConfigurationFeatureSMART(
	PFEATURE_DATA_SMART pSmart
	)
{
	OutputDriveLogA(
		"\tFeatureSMART\n"
		"\t\tFaultFailureReportingPagePresent: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pSmart->FaultFailureReportingPagePresent));
}

VOID OutputGetConfigurationFeatureEmbeddedChanger(
	PFEATURE_DATA_EMBEDDED_CHANGER pEmbedded
	)
{
	OutputDriveLogA(
		"\tFeatureEmbeddedChanger\n"
		"\t\tSupportsDiscPresent: %s\n"
		"\t\t  SideChangeCapable: %s\n"
		"\t\t  HighestSlotNumber: %u\n",
		BOOLEAN_TO_STRING_YES_NO_A(pEmbedded->SupportsDiscPresent),
		BOOLEAN_TO_STRING_YES_NO_A(pEmbedded->SideChangeCapable),
		pEmbedded->HighestSlotNumber);
}

VOID OutputGetConfigurationFeatureCDAudioAnalogPlay(
	PFEATURE_DATA_CD_AUDIO_ANALOG_PLAY pCDAudio
	)
{
	OutputDriveLogA(
		"\tFeatureCDAudioAnalogPlay\n"
		"\t\t     SeperateVolume: %s\n"
		"\t\tSeperateChannelMute: %s\n"
		"\t\t      ScanSupported: %s\n"
		"\t\tNumerOfVolumeLevels: %u\n",
		BOOLEAN_TO_STRING_YES_NO_A(pCDAudio->SeperateVolume),
		BOOLEAN_TO_STRING_YES_NO_A(pCDAudio->SeperateChannelMute),
		BOOLEAN_TO_STRING_YES_NO_A(pCDAudio->ScanSupported),
		MAKEWORD(pCDAudio->NumerOfVolumeLevels[1], pCDAudio->NumerOfVolumeLevels[0]));
}

VOID OutputGetConfigurationFeatureMicrocodeUpgrade(
	PFEATURE_DATA_MICROCODE_UPDATE pMicrocode
	)
{
	OutputDriveLogA(
		"\tFeatureMicrocodeUpgrade\n"
		"\t\tM5: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pMicrocode->M5));
}

VOID OutputGetConfigurationFeatureTimeout(
	PFEATURE_DATA_TIMEOUT pTimeOut
	)
{
	OutputDriveLogA(
		"\tFeatureTimeout\n"
		"\t\t    Group3: %s\n"
		"\t\tUnitLength: %u\n",
		BOOLEAN_TO_STRING_YES_NO_A(pTimeOut->Group3),
		MAKEWORD(pTimeOut->UnitLength[1], pTimeOut->UnitLength[0]));
}

VOID OutputGetConfigurationFeatureDvdCSS(
	PFEATURE_DATA_DVD_CSS pDVDCss
	)
{
	OutputDriveLogA(
		"\tFeatureDvdCSS\n"
		"\t\tCssVersion: %u\n",
		pDVDCss->CssVersion);
}

VOID OutputGetConfigurationFeatureRealTimeStreaming(
	PFEATURE_DATA_REAL_TIME_STREAMING pRealTimeStreaming
	)
{
	OutputDriveLogA(
		"\tFeatureRealTimeStreaming\n"
		"\t\t        StreamRecording: %s\n"
		"\t\t    WriteSpeedInGetPerf: %s\n"
		"\t\t       WriteSpeedInMP2A: %s\n"
		"\t\t             SetCDSpeed: %s\n"
		"\t\tReadBufferCapacityBlock: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(pRealTimeStreaming->StreamRecording),
		BOOLEAN_TO_STRING_YES_NO_A(pRealTimeStreaming->WriteSpeedInGetPerf),
		BOOLEAN_TO_STRING_YES_NO_A(pRealTimeStreaming->WriteSpeedInMP2A),
		BOOLEAN_TO_STRING_YES_NO_A(pRealTimeStreaming->SetCDSpeed),
		BOOLEAN_TO_STRING_YES_NO_A(pRealTimeStreaming->ReadBufferCapacityBlock));
}

VOID OutputGetConfigurationFeatureLogicalUnitSerialNumber(
	PFEATURE_DATA_LOGICAL_UNIT_SERIAL_NUMBER pLogical
	)
{
	OutputDriveLogA(
		"\tFeatureLogicalUnitSerialNumber\n"
		"\t\tSerialNumber: ");
	for (INT i = 0; i < pLogical->Header.AdditionalLength; i++) {
		OutputDriveLogA("%c", pLogical->SerialNumber[i]);
	}
	OutputDriveLogA("\n");
}

VOID OutputGetConfigurationFeatureMediaSerialNumber(
	PFEATURE_MEDIA_SERIAL_NUMBER pMediaSerialNumber
	)
{
	OutputDriveLogA(
		"\tFeatureMediaSerialNumber\n"
		"\t\t   Current: %u\n"
		"\t\tPersistent: %u\n"
		"\t\t   Version: %u\n",
		pMediaSerialNumber->Header.Current,
		pMediaSerialNumber->Header.Persistent,
		pMediaSerialNumber->Header.Version);
}

VOID OutputGetConfigurationFeatureDiscControlBlocks(
	PFEATURE_DATA_DISC_CONTROL_BLOCKS pDiscCtrlBlk
	)
{
	OutputDriveLogA("\tFeatureDiscControlBlocks\n");
	for (INT i = 0; i < pDiscCtrlBlk->Header.AdditionalLength; i++) {
		OutputDriveLogA(
			"\t\tContentDescriptor %02u: %08d\n", i,
			MAKELONG(
				MAKEWORD(pDiscCtrlBlk->Data[i].ContentDescriptor[3], pDiscCtrlBlk->Data[i].ContentDescriptor[2]),
				MAKEWORD(pDiscCtrlBlk->Data[i].ContentDescriptor[1], pDiscCtrlBlk->Data[i].ContentDescriptor[0])));
	}
}

VOID OutputGetConfigurationFeatureDvdCPRM(
	PFEATURE_DATA_DVD_CPRM pDVDCprm
	)
{
	OutputDriveLogA(
		"\tFeatureDvdCPRM\n"
		"\t\tCPRMVersion: %u\n",
		pDVDCprm->CPRMVersion);
}

VOID OutputGetConfigurationFeatureFirmwareDate(
	PFEATURE_DATA_FIRMWARE_DATE pFirmwareDate
	)
{
	OutputDriveLogA(
		"\tFeatureFirmwareDate: %04d-%02u-%02u %02u:%02u:%02u\n",
		MAKELONG(MAKEWORD(pFirmwareDate->Year[3], pFirmwareDate->Year[2]),
			MAKEWORD(pFirmwareDate->Year[1], pFirmwareDate->Year[0])),
		MAKEWORD(pFirmwareDate->Month[1], pFirmwareDate->Month[0]),
		MAKEWORD(pFirmwareDate->Day[1], pFirmwareDate->Day[0]),
		MAKEWORD(pFirmwareDate->Hour[1], pFirmwareDate->Hour[0]),
		MAKEWORD(pFirmwareDate->Minute[1], pFirmwareDate->Minute[0]),
		MAKEWORD(pFirmwareDate->Seconds[1], pFirmwareDate->Seconds[0]));
}

VOID OutputGetConfigurationFeatureAACS(
	PFEATURE_DATA_AACS pAACS
	)
{
	OutputDriveLogA(
		"\tFeatureAACS\n"
		"\t\tBindingNonceGeneration: %s\n"
		"\t\tBindingNonceBlockCount: %u\n"
		"\t\t         NumberOfAGIDs: %u\n"
		"\t\t           AACSVersion: %u\n",
		BOOLEAN_TO_STRING_YES_NO_A(pAACS->BindingNonceGeneration),
		pAACS->BindingNonceBlockCount,
		pAACS->NumberOfAGIDs,
		pAACS->AACSVersion);
}

VOID OutputGetConfigurationFeatureVCPS(
	PFEATURE_VCPS pVcps
	)
{
	OutputDriveLogA(
		"\tFeatureVCPS\n"
		"\t\t   Current: %u\n"
		"\t\tPersistent: %u\n"
		"\t\t   Version: %u\n",
		pVcps->Header.Current,
		pVcps->Header.Persistent,
		pVcps->Header.Version);
}

VOID OutputGetConfigurationFeatureReserved(
	PFEATURE_DATA_RESERVED pReserved
	)
{
	OutputDriveLogA(
		"\tReserved. FeatureCode[%#04x]\n"
		"\t\tData: ", MAKEWORD(pReserved->Header.FeatureCode[1], pReserved->Header.FeatureCode[0]));
	for (INT i = 0; i < pReserved->Header.AdditionalLength; i++) {
		OutputDriveLogA("%02x", pReserved->Data[i]);
	}
	OutputDriveLogA("\n");
}

VOID OutputGetConfigurationFeatureVendorSpecific(
	PFEATURE_DATA_VENDOR_SPECIFIC pVendorSpecific
	)
{
	OutputDriveLogA(
		"\tVendorSpecific. FeatureCode[%#04x]\n"
		"\t\tVendorSpecificData: ",
		MAKEWORD(pVendorSpecific->Header.FeatureCode[1], pVendorSpecific->Header.FeatureCode[0]));
	for (INT i = 0; i < pVendorSpecific->Header.AdditionalLength; i++) {
		OutputDriveLogA("%02x", pVendorSpecific->VendorSpecificData[i]);
	}
	OutputDriveLogA("\n");
}

VOID OutputGetConfigurationFeatureNumber(
	PDEVICE pDevice,
	LPBYTE lpConf,
	DWORD dwAllLen
	)
{
	DWORD n = 0;
	while (n < dwAllLen) {
		WORD wCode = MAKEWORD(lpConf[n + 1], lpConf[n]);
		switch (wCode) {
		case FeatureProfileList:
			OutputGetConfigurationFeatureProfileList((PFEATURE_DATA_PROFILE_LIST)&lpConf[n]);
			break;
		case FeatureCore:
			OutputGetConfigurationFeatureCore((PFEATURE_DATA_CORE)&lpConf[n]);
			break;
		case FeatureMorphing:
			OutputGetConfigurationFeatureMorphing((PFEATURE_DATA_MORPHING)&lpConf[n]);
			break;
		case FeatureRemovableMedium:
			OutputGetConfigurationFeatureRemovableMedium((PFEATURE_DATA_REMOVABLE_MEDIUM)&lpConf[n]);
			break;
		case FeatureWriteProtect:
			OutputGetConfigurationFeatureWriteProtect((PFEATURE_DATA_WRITE_PROTECT)&lpConf[n]);
			break;
		case FeatureRandomReadable:
			OutputGetConfigurationFeatureRandomReadable((PFEATURE_DATA_RANDOM_READABLE)&lpConf[n]);
			break;
		case FeatureMultiRead:
			OutputGetConfigurationFeatureMultiRead((PFEATURE_DATA_MULTI_READ)&lpConf[n]);
			break;
		case FeatureCdRead:
			OutputGetConfigurationFeatureCdRead((PFEATURE_DATA_CD_READ)&lpConf[n]);
			SetFeatureCdRead((PFEATURE_DATA_CD_READ)&lpConf[n], pDevice);
			break;
		case FeatureDvdRead:
			OutputGetConfigurationFeatureDvdRead((PFEATURE_DATA_DVD_READ)&lpConf[n]);
			break;
		case FeatureRandomWritable:
			OutputGetConfigurationFeatureRandomWritable((PFEATURE_DATA_RANDOM_WRITABLE)&lpConf[n]);
			break;
		case FeatureIncrementalStreamingWritable:
			OutputGetConfigurationFeatureIncrementalStreamingWritable((PFEATURE_DATA_INCREMENTAL_STREAMING_WRITABLE)&lpConf[n]);
			break;
		case FeatureSectorErasable:
			OutputGetConfigurationFeatureSectorErasable((PFEATURE_DATA_SECTOR_ERASABLE)&lpConf[n]);
			break;
		case FeatureFormattable:
			OutputGetConfigurationFeatureFormattable((PFEATURE_DATA_FORMATTABLE)&lpConf[n]);
			break;
		case FeatureDefectManagement:
			OutputGetConfigurationFeatureDefectManagement((PFEATURE_DATA_DEFECT_MANAGEMENT)&lpConf[n]);
			break;
		case FeatureWriteOnce:
			OutputGetConfigurationFeatureWriteOnce((PFEATURE_DATA_WRITE_ONCE)&lpConf[n]);
			break;
		case FeatureRestrictedOverwrite:
			OutputGetConfigurationFeatureRestrictedOverwrite((PFEATURE_DATA_RESTRICTED_OVERWRITE)&lpConf[n]);
			break;
		case FeatureCdrwCAVWrite:
			OutputGetConfigurationFeatureCdrwCAVWrite((PFEATURE_DATA_CDRW_CAV_WRITE)&lpConf[n]);
			break;
		case FeatureMrw:
			OutputGetConfigurationFeatureMrw((PFEATURE_DATA_MRW)&lpConf[n]);
			break;
		case FeatureEnhancedDefectReporting:
			OutputGetConfigurationFeatureEnhancedDefectReporting((PFEATURE_ENHANCED_DEFECT_REPORTING)&lpConf[n]);
			break;
		case FeatureDvdPlusRW:
			OutputGetConfigurationFeatureDvdPlusRW((PFEATURE_DATA_DVD_PLUS_RW)&lpConf[n]);
			break;
		case FeatureDvdPlusR:
			OutputGetConfigurationFeatureDvdPlusR((PFEATURE_DATA_DVD_PLUS_R)&lpConf[n]);
			break;
		case FeatureRigidRestrictedOverwrite:
			OutputGetConfigurationFeatureRigidRestrictedOverwrite((PFEATURE_DATA_DVD_RW_RESTRICTED_OVERWRITE)&lpConf[n]);
			break;
		case FeatureCdTrackAtOnce:
			OutputGetConfigurationFeatureCdTrackAtOnce((PFEATURE_DATA_CD_TRACK_AT_ONCE)&lpConf[n]);
			break;
		case FeatureCdMastering:
			OutputGetConfigurationFeatureCdMastering((PFEATURE_DATA_CD_MASTERING)&lpConf[n]);
			break;
		case FeatureDvdRecordableWrite:
			OutputGetConfigurationFeatureDvdRecordableWrite((PFEATURE_DATA_DVD_RECORDABLE_WRITE)&lpConf[n]);
			break;
		case FeatureLayerJumpRecording:
			OutputGetConfigurationFeatureLayerJumpRecording((PFEATURE_DATA_LAYER_JUMP_RECORDING)&lpConf[n]);
			break;
		case FeatureCDRWMediaWriteSupport:
			OutputGetConfigurationFeatureCDRWMediaWriteSupport((PFEATURE_CD_RW_MEDIA_WRITE_SUPPORT)&lpConf[n]);
			break;
		case FeatureDvdPlusRWDualLayer:
			OutputGetConfigurationFeatureDvdPlusRWDualLayer((PFEATURE_DATA_DVD_PLUS_RW_DUAL_LAYER)&lpConf[n]);
			break;
		case FeatureDvdPlusRDualLayer:
			OutputGetConfigurationFeatureDvdPlusRDualLayer((PFEATURE_DATA_DVD_PLUS_R_DUAL_LAYER)&lpConf[n]);
			break;
		case FeatureHybridDisc:
			OutputGetConfigurationFeatureHybridDisc((PFEATURE_HYBRID_DISC)&lpConf[n]);
			break;
		case FeaturePowerManagement:
			OutputGetConfigurationFeaturePowerManagement((PFEATURE_DATA_POWER_MANAGEMENT)&lpConf[n]);
			break;
		case FeatureSMART:
			OutputGetConfigurationFeatureSMART((PFEATURE_DATA_SMART)&lpConf[n]);
			break;
		case FeatureEmbeddedChanger:
			OutputGetConfigurationFeatureEmbeddedChanger((PFEATURE_DATA_EMBEDDED_CHANGER)&lpConf[n]);
			break;
		case FeatureCDAudioAnalogPlay:
			OutputGetConfigurationFeatureCDAudioAnalogPlay((PFEATURE_DATA_CD_AUDIO_ANALOG_PLAY)&lpConf[n]);
			break;
		case FeatureMicrocodeUpgrade:
			OutputGetConfigurationFeatureMicrocodeUpgrade((PFEATURE_DATA_MICROCODE_UPDATE)&lpConf[n]);
			break;
		case FeatureTimeout:
			OutputGetConfigurationFeatureTimeout((PFEATURE_DATA_TIMEOUT)&lpConf[n]);
			break;
		case FeatureDvdCSS:
			OutputGetConfigurationFeatureDvdCSS((PFEATURE_DATA_DVD_CSS)&lpConf[n]);
			break;
		case FeatureRealTimeStreaming:
			OutputGetConfigurationFeatureRealTimeStreaming((PFEATURE_DATA_REAL_TIME_STREAMING)&lpConf[n]);
			SetFeatureRealTimeStreaming((PFEATURE_DATA_REAL_TIME_STREAMING)&lpConf[n], pDevice);
			break;
		case FeatureLogicalUnitSerialNumber:
			OutputGetConfigurationFeatureLogicalUnitSerialNumber((PFEATURE_DATA_LOGICAL_UNIT_SERIAL_NUMBER)&lpConf[n]);
			break;
		case FeatureMediaSerialNumber:
			OutputGetConfigurationFeatureMediaSerialNumber((PFEATURE_MEDIA_SERIAL_NUMBER)&lpConf[n]);
			break;
		case FeatureDiscControlBlocks:
			OutputGetConfigurationFeatureDiscControlBlocks((PFEATURE_DATA_DISC_CONTROL_BLOCKS)&lpConf[n]);
			break;
		case FeatureDvdCPRM:
			OutputGetConfigurationFeatureDvdCPRM((PFEATURE_DATA_DVD_CPRM)&lpConf[n]);
			break;
		case FeatureFirmwareDate:
			OutputGetConfigurationFeatureFirmwareDate((PFEATURE_DATA_FIRMWARE_DATE)&lpConf[n]);
			break;
		case FeatureAACS:
			OutputGetConfigurationFeatureAACS((PFEATURE_DATA_AACS)&lpConf[n]);
			break;
		case FeatureVCPS:
			OutputGetConfigurationFeatureVCPS((PFEATURE_VCPS)&lpConf[n]);
			break;
		default:
			if (0x0111 <= wCode && wCode <= 0xfeff) {
				OutputGetConfigurationFeatureReserved((PFEATURE_DATA_RESERVED)&lpConf[n]);
			}
			else if (0xff00 <= wCode && wCode <= 0xffff) {
				OutputGetConfigurationFeatureVendorSpecific((PFEATURE_DATA_VENDOR_SPECIFIC)&lpConf[n]);
			}
			break;
		}
		n += sizeof(FEATURE_HEADER) + lpConf[n + 3];
	}
}

VOID OutputDiscInformation(
	PDISC_INFORMATION pDiscInformation
	)
{
	LPCSTR lpDiscStatus[] = {
		"Empty", "Incomplete", "Complete", "Others"
	};
	LPCSTR lpStateOfLastSession[] = {
		"Empty", "Incomplete", "Reserved / Damaged", "Complete"
	};
	LPCSTR lpBGFormatStatus[] = {
		"None", "Incomplete", "Running", "Complete"
	};
	OutputDiscLogA(
		"=============================== DiscInformation ===============================\n"
		"\t                       DiscStatus: %s\n"
		"\t                LastSessionStatus: %s\n"
		"\t                         Erasable: %s\n"
		"\t                 FirstTrackNumber: %u\n"
		"\t              NumberOfSessionsLsb: %u\n"
		"\t         LastSessionFirstTrackLsb: %u\n"
		"\t          LastSessionLastTrackLsb: %u\n"
		"\t                        MrwStatus: %s\n"
		"\t                      MrwDirtyBit: %s\n"
		"\t                  UnrestrictedUse: %s\n"
		"\t                 DiscBarCodeValid: %s\n"
		"\t                      DiscIDValid: %s\n"
		"\t                         DiscType: ",
		lpDiscStatus[pDiscInformation->DiscStatus],
		lpStateOfLastSession[pDiscInformation->LastSessionStatus],
		BOOLEAN_TO_STRING_YES_NO_A(pDiscInformation->Erasable),
		pDiscInformation->FirstTrackNumber,
		pDiscInformation->NumberOfSessionsLsb,
		pDiscInformation->LastSessionFirstTrackLsb,
		pDiscInformation->LastSessionLastTrackLsb,
		lpBGFormatStatus[pDiscInformation->MrwStatus],
		BOOLEAN_TO_STRING_YES_NO_A(pDiscInformation->MrwDirtyBit),
		BOOLEAN_TO_STRING_YES_NO_A(pDiscInformation->URU),
		BOOLEAN_TO_STRING_YES_NO_A(pDiscInformation->DBC_V),
		BOOLEAN_TO_STRING_YES_NO_A(pDiscInformation->DID_V));
	switch (pDiscInformation->DiscType) {
	case DISK_TYPE_CDDA:
		OutputDiscLogA("CD-DA or CD-ROM Disc\n");
		break;
	case DISK_TYPE_CDI:
		OutputDiscLogA("CD-I Disc\n");
		break;
	case DISK_TYPE_XA:
		OutputDiscLogA("CD-ROM XA Disc\n");
		break;
	case DISK_TYPE_UNDEFINED:
		OutputDiscLogA("Undefined\n");
		break;
	default:
		OutputDiscLogA("Reserved\n");
		break;
	}
	if (pDiscInformation->DID_V) {
		OutputDiscLogA(
			"\t               DiscIdentification: %u%u%u%u\n",
			pDiscInformation->DiskIdentification[0],
			pDiscInformation->DiskIdentification[1],
			pDiscInformation->DiskIdentification[2],
			pDiscInformation->DiskIdentification[3]);
	}
	OutputDiscLogA(
		"\t                LastSessionLeadIn: %02x:%02x:%02x:%02x\n"
		"\t     LastPossibleLeadOutStartTime: %02x:%02x:%02x:%02x\n",
		pDiscInformation->LastSessionLeadIn[0],
		pDiscInformation->LastSessionLeadIn[1],
		pDiscInformation->LastSessionLeadIn[2],
		pDiscInformation->LastSessionLeadIn[3],
		pDiscInformation->LastPossibleLeadOutStartTime[0],
		pDiscInformation->LastPossibleLeadOutStartTime[1],
		pDiscInformation->LastPossibleLeadOutStartTime[2],
		pDiscInformation->LastPossibleLeadOutStartTime[3]);
	if (pDiscInformation->DBC_V) {
		OutputDiscLogA(
			"\t                      DiscBarCode: %u%u%u%u%u%u%u%u\n",
			pDiscInformation->DiskBarCode[0],
			pDiscInformation->DiskBarCode[1],
			pDiscInformation->DiskBarCode[2],
			pDiscInformation->DiskBarCode[3],
			pDiscInformation->DiskBarCode[4],
			pDiscInformation->DiskBarCode[5],
			pDiscInformation->DiskBarCode[6],
			pDiscInformation->DiskBarCode[7]);
	}
	OutputDiscLogA(
		"\t                 NumberOPCEntries: %u\n",
		pDiscInformation->NumberOPCEntries);
	if (pDiscInformation->NumberOPCEntries) {
		OutputDiscLogA(
			"\t                         OPCTable\n");
	}
	for (INT i = 0; i < pDiscInformation->NumberOPCEntries; i++) {
		OutputDiscLogA(
			"\t\t                          Speed: %u%u\n"
			"\t\t                      OPCValues: %u%u%u%u%u%u\n",
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

VOID OutputModeParmeterHeader(
	PMODE_PARAMETER_HEADER pHeader
	)
{
	OutputDriveLogA(
		"============================== ModeParmeterHeader =============================\n"
		"\t         ModeDataLength: %u\n"
		"\t             MediumType: %u\n"
		"\tDeviceSpecificParameter: %u\n"
		"\t  BlockDescriptorLength: %u\n"
		, pHeader->ModeDataLength
		, pHeader->MediumType
		, pHeader->DeviceSpecificParameter
		, pHeader->BlockDescriptorLength);
}

VOID OutputModeParmeterHeader10(
	PMODE_PARAMETER_HEADER10 pHeader
	)
{
	OutputDriveLogA(
		"============================= ModeParmeterHeader10 ============================\n"
		"\t         ModeDataLength: %u\n"
		"\t             MediumType: %u\n"
		"\tDeviceSpecificParameter: %u\n"
		"\t  BlockDescriptorLength: %u\n"
		, MAKEWORD(pHeader->ModeDataLength[1],
		pHeader->ModeDataLength[0])
		, pHeader->MediumType
		, pHeader->DeviceSpecificParameter
		, MAKEWORD(pHeader->BlockDescriptorLength[1],
		pHeader->BlockDescriptorLength[0]));
}

VOID OutputCDVDCapabilitiesPage(
	PDEVICE pDevice,
	PCDVD_CAPABILITIES_PAGE cdvd
	)
{
	OutputDriveLogA(
		"================== CDVD Capabilities & Mechanism Status Page ==================\n"
		"\t              PageCode: %#04x\n"
		"\t                 PSBit: %s\n"
		"\t            PageLength: %u\n"
		"\t               CDRRead: %s\n"
		"\t               CDERead: %s\n"
		"\t               Method2: %s\n"
		"\t            DVDROMRead: %s\n"
		"\t              DVDRRead: %s\n"
		"\t            DVDRAMRead: %s\n"
		"\t              CDRWrite: %s\n"
		"\t              CDEWrite: %s\n"
		"\t             TestWrite: %s\n"
		"\t             DVDRWrite: %s\n"
		"\t           DVDRAMWrite: %s\n"
		"\t             AudioPlay: %s\n"
		"\t             Composite: %s\n"
		"\t        DigitalPortOne: %s\n"
		"\t        DigitalPortTwo: %s\n"
		"\t            Mode2Form1: %s\n"
		"\t            Mode2Form2: %s\n"
		"\t          MultiSession: %s\n"
		"\t    BufferUnderrunFree: %s\n"
		"\t                  CDDA: %s\n"
		"\t          CDDAAccurate: %s\n"
		"\t           RWSupported: %s\n"
		"\t       RWDeinterleaved: %s\n"
		"\t            C2Pointers: %s\n"
		"\t                  ISRC: %s\n"
		"\t                   UPC: %s\n"
		"\t    ReadBarCodeCapable: %s\n"
		"\t                  Lock: %s\n"
		"\t             LockState: %s\n"
		"\t         PreventJumper: %s\n"
		"\t                 Eject: %s\n"
		"\t  LoadingMechanismType: "
		, cdvd->PageCode
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->PSBit)
		, cdvd->PageLength
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->CDRRead)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->CDERead)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->Method2)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->DVDROMRead)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->DVDRRead)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->DVDRAMRead)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->CDRWrite)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->CDEWrite)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->TestWrite)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->DVDRWrite)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->DVDRAMWrite)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->AudioPlay)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->Composite)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->DigitalPortOne)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->DigitalPortTwo)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->Mode2Form1)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->Mode2Form2)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->MultiSession)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->BufferUnderrunFree)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->CDDA)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->CDDAAccurate)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->RWSupported)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->RWDeinterleaved)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->C2Pointers)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->ISRC)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->UPC)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->ReadBarCodeCapable)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->Lock)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->LockState)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->PreventJumper)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->Eject));

	switch (cdvd->LoadingMechanismType) {
	case LOADING_MECHANISM_CADDY:
		OutputDriveLogA("caddy\n")
		break;
	case LOADING_MECHANISM_TRAY:
		OutputDriveLogA("tray\n")
		break;
	case LOADING_MECHANISM_POPUP:
		OutputDriveLogA("popup\n")
		break;
	case LOADING_MECHANISM_INDIVIDUAL_CHANGER:
		OutputDriveLogA("individual changer\n")
		break;
	case LOADING_MECHANISM_CARTRIDGE_CHANGER:
		OutputDriveLogA("cartridge changer\n")
		break;
	default:
		OutputDriveLogA("unknown\n")
		break;
	}
	WORD rsm = MAKEWORD(cdvd->ReadSpeedMaximum[1],
		cdvd->ReadSpeedMaximum[0]);
	WORD rsc = MAKEWORD(cdvd->ReadSpeedCurrent[1],
		cdvd->ReadSpeedCurrent[0]);
	WORD wsm = MAKEWORD(cdvd->WriteSpeedMaximum[1],
		cdvd->WriteSpeedMaximum[0]);
	WORD wsc = MAKEWORD(cdvd->WriteSpeedCurrent[1],
		cdvd->WriteSpeedCurrent[0]);
	WORD bs = MAKEWORD(cdvd->BufferSize[1],
		cdvd->BufferSize[0]);
	OutputDriveLogA(
		"\t        SeparateVolume: %s\n"
		"\t   SeperateChannelMute: %s\n"
		"\t   SupportsDiskPresent: %s\n"
		"\t       SWSlotSelection: %s\n"
		"\t     SideChangeCapable: %s\n"
		"\t    RWInLeadInReadable: %s\n"
		"\t      ReadSpeedMaximum: %u (%ux)\n"
		"\t    NumberVolumeLevels: %u\n"
		"\t            BufferSize: %u\n"
		"\t      ReadSpeedCurrent: %u (%ux)\n"
		"\t                   BCK: %s\n"
		"\t                   RCK: %s\n"
		"\t                  LSBF: %s\n"
		"\t                Length: %u\n"
		"\t     WriteSpeedMaximum: %u (%ux)\n"
		"\t     WriteSpeedCurrent: %u (%ux)\n"
		"\tCopyManagementRevision: %u\n"
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->SeparateVolume)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->SeperateChannelMute)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->SupportsDiskPresent)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->SWSlotSelection)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->SideChangeCapable)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->RWInLeadInReadable)
		, rsm, rsm / 174
		, MAKEWORD(cdvd->NumberVolumeLevels[1],
			cdvd->NumberVolumeLevels[0])
		, bs
		, rsc, rsc / 174
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->BCK)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->RCK)
		, BOOLEAN_TO_STRING_YES_NO_A(cdvd->LSBF)
		, cdvd->Length
		, wsm, wsm / 174
		, wsc, wsc / 174
		, MAKEWORD(cdvd->CopyManagementRevision[1],
			cdvd->CopyManagementRevision[0]));
	pDevice->wMaxReadSpeed = rsm;
}

VOID OutputReadBufferCapacity(
	PREAD_BUFFER_CAPACITY_DATA pReadBufCapaData
	)
{
	OutputDriveLogA(
		"============================== ReadBufferCapacity =============================\n"
		"\t    TotalBufferSize: %uKByte\n"
		"\tAvailableBufferSize: %uKByte\n",
		MAKELONG(MAKEWORD(pReadBufCapaData->TotalBufferSize[3],
		pReadBufCapaData->TotalBufferSize[2]),
		MAKEWORD(pReadBufCapaData->TotalBufferSize[1],
		pReadBufCapaData->TotalBufferSize[0])) / 1024,
		MAKELONG(MAKEWORD(pReadBufCapaData->AvailableBufferSize[3],
		pReadBufCapaData->AvailableBufferSize[2]),
		MAKEWORD(pReadBufCapaData->AvailableBufferSize[1],
		pReadBufCapaData->AvailableBufferSize[0])) / 1024);
}

VOID OutputSetSpeed(
	PCDROM_SET_SPEED pSetspeed
	)
{
	OutputDriveLogA(
		"================================== SetSpeed ===================================\n"
		"\t    RequestType: %s\n"
		"\t      ReadSpeed: %uKB/sec\n"
		"\t     WriteSpeed: %uKB/sec\n"
		"\tRotationControl: %s\n",
		pSetspeed->RequestType == 0 ?
		"CdromSetSpeed" : "CdromSetStreaming",
		pSetspeed->ReadSpeed,
		pSetspeed->WriteSpeed,
		pSetspeed->RotationControl == 0 ?
		"CdromDefaultRotation" : "CdromCAVRotation");
}

VOID OutputEepromOverPX712(
	LPBYTE pBuf,
	PDWORD idx
	)
{
	OutputDriveLogA("\t    Silent Mode: ");
	if (pBuf[*idx] == 1) {
		OutputDriveLogA(
			"Enabled\n"
			"\t\t       Access Time: ");
		if (pBuf[*idx + 1] == 0) {
			OutputDriveLogA("Fast\n");
		}
		else if (pBuf[*idx + 1] == 2) {
			OutputDriveLogA("Slow\n");
		}
		OutputDriveLogA(
			"\t\t    Max Read Speed: %dx\n"
			"\t\t           Unknown: %02x\n"
			"\t\t   Max Write Speed: %dx\n"
			"\t\t           Unknown: %02x\n"
			"\t\t           Unknown: %02x\n"
			"\t\t  Tray Speed Eject: %02x (Low d0 - 80 High)\n"
			"\t\tTray Speed Loading: %02x (Low 2f - 7f High)\n",
			pBuf[*idx + 2], pBuf[*idx + 3], pBuf[*idx + 4], 
			pBuf[*idx + 5], pBuf[*idx + 6], pBuf[*idx + 7], pBuf[*idx + 8]);
	}
	else {
		OutputDriveLogA("Disable\n");
	}
	*idx += 9;
	DWORD tmp = *idx;
	OutputDriveLogA("\t        SecuRec: ");
	while (*idx < tmp + 20) {
		OutputDriveLogA("%02x ", pBuf[*idx]);
		*idx += 1;
	}
	OutputDriveLogA(
		"\n\t        Unknown: %x"
		"\n\t      SpeedRead: "
		, pBuf[*idx] >> 4 & 0x0f);
	INT sp = pBuf[*idx] & 0x0f;
	if (sp == 0) {
		OutputDriveLogA("Enable");
	}
	else if (sp == 0xf) {
		OutputDriveLogA("Disable");
	}
	OutputDriveLogA(
		"\n\t        Unknown: %x"
		"\n\t  Spindown Time: ",
		pBuf[*idx + 1]);
	switch (pBuf[*idx + 2]) {
	case 0:
		OutputDriveLogA("Infinite\n");
		break;
	case 1:
		OutputDriveLogA("125 ms\n");
		break;
	case 2:
		OutputDriveLogA("250 ms\n");
		break;
	case 3:
		OutputDriveLogA("500 ms\n");
		break;
	case 4:
		OutputDriveLogA("1 second\n");
		break;
	case 5:
		OutputDriveLogA("2 seconds\n");
		break;
	case 6:
		OutputDriveLogA("4 seconds\n");
		break;
	case 7:
		OutputDriveLogA("8 seconds\n");
		break;
	case 8:
		OutputDriveLogA("16 seconds\n");
		break;
	case 9:
		OutputDriveLogA("32 seconds\n");
		break;
	case 10:
		OutputDriveLogA("1 minite\n");
		break;
	case 11:
		OutputDriveLogA("2 minites\n");
		break;
	case 12:
		OutputDriveLogA("4 minites\n");
		break;
	case 13:
		OutputDriveLogA("8 minites\n");
		break;
	case 14:
		OutputDriveLogA("16 minites\n");
		break;
	case 15:
		OutputDriveLogA("32 minites\n");
		break;
	default:
		OutputDriveLogA("Unset\n");
		break;
	}
	*idx += 3;
	LONG ucr = 
		MAKELONG(MAKEWORD(pBuf[*idx + 5], pBuf[*idx + 4]), MAKEWORD(pBuf[*idx + 3], pBuf[*idx + 2]));
	LONG ucw = 
		MAKELONG(MAKEWORD(pBuf[*idx + 9], pBuf[*idx + 8]), MAKEWORD(pBuf[*idx + 7], pBuf[*idx + 6]));
	LONG udr = 
		MAKELONG(MAKEWORD(pBuf[*idx + 13], pBuf[*idx + 12]), MAKEWORD(pBuf[*idx + 11], pBuf[*idx + 10]));
	LONG udw = 
		MAKELONG(MAKEWORD(pBuf[*idx + 17], pBuf[*idx + 16]), MAKEWORD(pBuf[*idx + 15], pBuf[*idx + 14]));
	OutputDriveLogA(
		"\tDisc load count: %u\n"
		"\t   CD read time: %02u:%02u:%02u\n"
		"\t  CD write time: %02u:%02u:%02u\n"
		"\t  DVD read time: %02u:%02u:%02u\n"
		"\t DVD write time: %02u:%02u:%02u\n"
		, MAKEWORD(pBuf[*idx + 1], pBuf[*idx])
		, ucr / 3600, ucr / 60 % 60, ucr % 60
		, ucw / 3600, ucw / 60 % 60, ucw % 60
		, udr / 3600, udr / 60 % 60, udr % 60
		, udw / 3600, udw / 60 % 60, udw % 60);
	*idx += 18;
}

VOID OutputEepromUnknownByte(
	LPBYTE pBuf,
	DWORD startIdx,
	DWORD endIdx
	)
{
	if (startIdx <= endIdx) {
		OutputDriveLogA("\t        Unknown: ");
		for (DWORD i = startIdx; i <= endIdx; i++) {
			OutputDriveLogA("%02x ", pBuf[i]);
		}
		OutputDriveLogA("\n");
	}
}

VOID OutputEeprom(
	LPBYTE pBuf,
	DWORD tLen,
	INT nRoop,
	BOOL byPlxtrType
	)
{
	DWORD idx = 0;
	if (nRoop == 0) {
		OutputDriveLogA(
			"\t      Signature: %02x %02x\n"
			"\t       VendorId: %.8s\n"
			"\t      ProductId: %.16s\n"
			"\t   SerialNumber: %06u\n"
			, pBuf[0], pBuf[1]
			, (PCHAR)&pBuf[2]
			, (PCHAR)&pBuf[10]
			, strtoul((PCHAR)&pBuf[26], NULL, 16));
		OutputEepromUnknownByte(pBuf, 31, 40);

		switch (byPlxtrType) {
		case PLXTR_DRIVE_TYPE::PX760A:
		case PLXTR_DRIVE_TYPE::PX755A:
		case PLXTR_DRIVE_TYPE::PX716AL:
		case PLXTR_DRIVE_TYPE::PX716A:
		case PLXTR_DRIVE_TYPE::PX714A:
		case PLXTR_DRIVE_TYPE::PX712A:
			OutputDriveLogA("\t            TLA: %.4s\n", (PCHAR)&pBuf[41]);
			break;
		default:
			OutputEepromUnknownByte(pBuf, 41, 44);
			break;
		}
		OutputEepromUnknownByte(pBuf, 45, 107);

		switch (byPlxtrType) {
		case PLXTR_DRIVE_TYPE::PX760A:
		case PLXTR_DRIVE_TYPE::PX755A:
		case PLXTR_DRIVE_TYPE::PX716AL:
		case PLXTR_DRIVE_TYPE::PX716A:
		case PLXTR_DRIVE_TYPE::PX714A:
		case PLXTR_DRIVE_TYPE::PX712A:
			OutputEepromUnknownByte(pBuf, 108, 255);
			idx = 256;
			break;
		case PLXTR_DRIVE_TYPE::PX708A2:
		case PLXTR_DRIVE_TYPE::PX708A:
		case PLXTR_DRIVE_TYPE::PX704A:
		{
			OutputEepromUnknownByte(pBuf, 108, 114);
			LONG ucr = MAKELONG(MAKEWORD(pBuf[120], pBuf[119]), MAKEWORD(pBuf[118], pBuf[117]));
			LONG ucw = MAKELONG(MAKEWORD(pBuf[125], pBuf[124]), MAKEWORD(pBuf[123], pBuf[122]));
			OutputDriveLogA(
				"\tDisc load count: %u\n"
				"\t   CD read time: %02u:%02u:%02u\n"
				"\t        Unknown: %02x\n"
				"\t  CD write time: %02u:%02u:%02u\n"
				, MAKEWORD(pBuf[116], pBuf[115])
				, ucr / 3600, ucr / 60 % 60, ucr % 60
				, pBuf[121]
				, ucw / 3600, ucw / 60 % 60, ucw % 60);
			OutputEepromUnknownByte(pBuf, 126, 211);
			LONG udr =
				MAKELONG(MAKEWORD(pBuf[215], pBuf[214]), MAKEWORD(pBuf[213], pBuf[212]));
			LONG udw =
				MAKELONG(MAKEWORD(pBuf[219], pBuf[218]), MAKEWORD(pBuf[217], pBuf[216]));
			OutputDriveLogA(
				"\t  DVD read time: %02u:%02u:%02u\n"
				"\t DVD write time: %02u:%02u:%02u\n"
				, udr / 3600, udr / 60 % 60, udr % 60
				, udw / 3600, udw / 60 % 60, udw % 60);
			idx = 220;
			break;
		}
		case PLXTR_DRIVE_TYPE::PX320A:
		{
			OutputEepromUnknownByte(pBuf, 108, 123);
			LONG ucr = MAKELONG(MAKEWORD(pBuf[127], pBuf[126]), MAKEWORD(pBuf[125], pBuf[124]));
			OutputDriveLogA(
				"\t   CD read time: %02u:%02u:%02u\n"
				, ucr / 3600, ucr / 60 % 60, ucr % 60);
			OutputEepromUnknownByte(pBuf, 128, 187);
			LONG udr =
				MAKELONG(MAKEWORD(pBuf[191], pBuf[190]), MAKEWORD(pBuf[189], pBuf[188]));
			OutputDriveLogA(
				"\t  DVD read time: %02u:%02u:%02u\n"
				, udr / 3600, udr / 60 % 60, udr % 60);
			OutputEepromUnknownByte(pBuf, 192, 226);
			OutputDriveLogA(
				"\tDisc load count: %u\n"
				, MAKEWORD(pBuf[228], pBuf[227]));
			idx = 229;
			break;
		}
		case PLXTR_DRIVE_TYPE::PREMIUM2:
		case PLXTR_DRIVE_TYPE::PREMIUM:
		case PLXTR_DRIVE_TYPE::PXW5224A:
		case PLXTR_DRIVE_TYPE::PXW4824A:
		case PLXTR_DRIVE_TYPE::PXW4012A:
		case PLXTR_DRIVE_TYPE::PXW4012S:
		{
			LONG ucr = MAKELONG(MAKEWORD(pBuf[111], pBuf[110]), MAKEWORD(pBuf[109], pBuf[108]));
			LONG ucw = MAKELONG(MAKEWORD(pBuf[125], pBuf[124]), MAKEWORD(pBuf[123], pBuf[122]));
			OutputDriveLogA(
				"\t   CD read time: %02u:%02u:%02u\n"
				"\t        Unknown: %02x %02x %02x %02x %02x %02x %02x %02x\n"
				"\tDisc load count: %u\n"
				"\t  CD write time: %02u:%02u:%02u\n"
				, ucr / 3600, ucr / 60 % 60, ucr % 60
				, pBuf[112], pBuf[113], pBuf[114], pBuf[115], pBuf[116], pBuf[117], pBuf[118], pBuf[119]
				, MAKEWORD(pBuf[121], pBuf[120])
				, ucw / 3600, ucw / 60 % 60, ucw % 60);
			idx = 126;
			break;
		}
		case PLXTR_DRIVE_TYPE::PXW2410A:
		case PLXTR_DRIVE_TYPE::PXS88T:
		case PLXTR_DRIVE_TYPE::PXW1610A:
		case PLXTR_DRIVE_TYPE::PXW1210A:
		case PLXTR_DRIVE_TYPE::PXW1210S:
		{
			OutputEepromUnknownByte(pBuf, 108, 119);
			LONG ucw = MAKELONG(MAKEWORD(pBuf[125], pBuf[124]), MAKEWORD(pBuf[123], pBuf[122]));
			OutputDriveLogA(
				"\tDisc load count: %u\n"
				"\t  CD write time: %02u:%02u:%02u\n"
				, MAKEWORD(pBuf[121], pBuf[120])
				, ucw / 3600, ucw / 60 % 60, ucw % 60);
			idx = 126;
			break;
		}
		case PLXTR_DRIVE_TYPE::PXW124TS:
		case PLXTR_DRIVE_TYPE::PXW8432T:
		case PLXTR_DRIVE_TYPE::PXW8220T:
		case PLXTR_DRIVE_TYPE::PXW4220T:
		case PLXTR_DRIVE_TYPE::PXR820T:
		{
			OutputEepromUnknownByte(pBuf, 108, 121);
			LONG ucw = MAKELONG(MAKEWORD(pBuf[125], pBuf[124]), MAKEWORD(pBuf[123], pBuf[122]));
			OutputDriveLogA(
				"\t  CD write time: %02u:%02u:%02u\n"
				, ucw / 3600, ucw / 60 % 60, ucw % 60);
			idx = 126;
			break;
		}
		case PLXTR_DRIVE_TYPE::PXR412C:
		case PLXTR_DRIVE_TYPE::PX40TS:
		case PLXTR_DRIVE_TYPE::PX40TSUW:
		case PLXTR_DRIVE_TYPE::PX40TW:
		case PLXTR_DRIVE_TYPE::PX32TS:
		case PLXTR_DRIVE_TYPE::PX32CS:
		case PLXTR_DRIVE_TYPE::PX20TS:
		case PLXTR_DRIVE_TYPE::PX12TS:
		case PLXTR_DRIVE_TYPE::PX12CS:
		case PLXTR_DRIVE_TYPE::PX8XCS:
			OutputEepromUnknownByte(pBuf, 108, 125);
			idx = 126;
			break;
		}
		OutputEepromUnknownByte(pBuf, idx, tLen - 1);
	}
	else if (nRoop == 1 && byPlxtrType <= PLXTR_DRIVE_TYPE::PX714A) {
		OutputEepromOverPX712(pBuf, &idx);
		OutputEepromUnknownByte(pBuf, idx, 255);
	}
	else {
		OutputEepromUnknownByte(pBuf, idx, 255);
	}
}
