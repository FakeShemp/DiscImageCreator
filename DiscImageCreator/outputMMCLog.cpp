/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"
#include "struct.h"
#include "convert.h"
#include "output.h"
#include "outputMMCLog.h"
#include "set.h"


VOID OutputMmcBufferCapacity(
	PREAD_BUFFER_CAPACITY_DATA pReadBufCapaData
	)
{
	OutputDriveLogA(
		"Read Buffer Capacity\n"
		"\t  Length of the Buffer: %uKByte\n"
		"\tBlank Length of Buffer: %uKByte\n",
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
	LPCSTR lpDiscStatus[] = {
		"Empty Disc", "Incomplete Disc",
		"Finalized Disc", "Others"
	};
	LPCSTR lpStateOfLastSession[] = {
		"Empty Session", "Incomplete Session",
		"Reserved / Damaged Session", "Complete Session"
	};
	LPCSTR lpBGFormatStatus[] = {
		"The disc is neither CD-RW nor DVD+RW",
		"A background format was started",
		"A background format is in progress",
		"Background formatting has completed"
	};
	OutputDiscLogA(
		"DiscInformation\n"
		"\t                      Disc Status: %s\n"
		"\t              Last Session Status: %s\n"
		"\t                         Erasable: %s\n"
		"\t               First Track Number: %u\n"
		"\t               Number of Sessions: %u\n"
		"\t         Last Session First Track: %u\n"
		"\t          Last Session Last Track: %u\n"
		"\t         Background Format Status: %s\n"
		"\t                        Dirty Bit: %s\n"
		"\t            Unrestricted Use Disc: %s\n"
		"\t              Disc Bar Code Valid: %s\n"
		"\t                    Disc ID Valid: %s\n"
		"\t                        Disc Type: ",
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
		OutputDiscLogA("CD-DA or CD-ROM Disc\n");
		break;
	case 0x10:
		OutputDiscLogA("CD-I Disc\n");
		break;
	case 0x20:
		OutputDiscLogA("CD-ROM XA Disc\n");
		break;
	case 0xff:
		OutputDiscLogA("Undefined\n");
		break;
	default:
		OutputDiscLogA("Reserved\n");
		break;
	}
	if (pDiscInformation->DID_V) {
		OutputDiscLogA(
			"\t              Disc Identification: %u%u%u%u\n",
			pDiscInformation->DiskIdentification[0],
			pDiscInformation->DiskIdentification[1],
			pDiscInformation->DiskIdentification[2],
			pDiscInformation->DiskIdentification[3]);
	}
	OutputDiscLogA(
		"\t  Last Session Lead-in Start Time: %02x:%02x:%02x:%02x\n"
		"\tLast Possible Lead-out Start Time: %02x:%02x:%02x:%02x\n",
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
			"\t                    Disc Bar Code: %u%u%u%u%u%u%u%u\n",
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
		"\t             Number of OPC Tables: %u\n",
		pDiscInformation->NumberOPCEntries);
	if (pDiscInformation->NumberOPCEntries) {
		OutputDiscLogA(
			"\t                        OPC Table\n");
	}
	for (INT i = 0; i < pDiscInformation->NumberOPCEntries; i++) {
		OutputDiscLogA(
			"\t\t                          Speed: %u%u\n"
			"\t\t                     OPC Values: %u%u%u%u%u%u\n",
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
	OutputDriveLogA(
		"Drive speed\n"
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

VOID OutputMmcFeatureCore(
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
	case 0xFFFF:
		OutputDriveLogA("Vendor Unique\n");
		break;
	default:
		OutputDriveLogA("Reserved: %08d\n", lVal);
		break;
	}
	OutputDriveLogA(
		"\t\t  DeviceBusyEvent: %s\n"
		"\t\t         INQUIRY2: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pCore->DeviceBusyEvent),
		BOOLEAN_TO_STRING_YES_NO(pCore->INQUIRY2));
}

VOID OutputMmcFeatureMorphing(
	PFEATURE_DATA_MORPHING pMorphing
	)
{
	OutputDriveLogA(
		"\tFeatureMorphing\n"
		"\t\tAsynchronous: %s\n"
		"\t\t     OCEvent: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pMorphing->Asynchronous),
		BOOLEAN_TO_STRING_YES_NO(pMorphing->OCEvent));
}

VOID OutputMmcFeatureRemovableMedium(
	PFEATURE_DATA_REMOVABLE_MEDIUM pRemovableMedium
	)
{
	OutputDriveLogA(
		"\tFeatureRemovableMedium\n"
		"\t\t        Lockable: %s\n"
		"\t\tDefaultToPrevent: %s\n"
		"\t\t           Eject: %s\n"
		"\t\tLoadingMechanism: ",
		BOOLEAN_TO_STRING_YES_NO(pRemovableMedium->Lockable),
		BOOLEAN_TO_STRING_YES_NO(pRemovableMedium->DefaultToPrevent),
		BOOLEAN_TO_STRING_YES_NO(pRemovableMedium->Eject));
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

VOID OutputMmcFeatureWriteProtect(
	PFEATURE_DATA_WRITE_PROTECT pWriteProtect
	)
{
	OutputDriveLogA(
		"\tFeatureWriteProtect\n"
		"\t\t               SupportsSWPPBit: %s\n"
		"\t\tSupportsPersistentWriteProtect: %s\n"
		"\t\t               WriteInhibitDCB: %s\n"
		"\t\t           DiscWriteProtectPAC: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pWriteProtect->SupportsSWPPBit),
		BOOLEAN_TO_STRING_YES_NO(pWriteProtect->SupportsPersistentWriteProtect),
		BOOLEAN_TO_STRING_YES_NO(pWriteProtect->WriteInhibitDCB),
		BOOLEAN_TO_STRING_YES_NO(pWriteProtect->DiscWriteProtectPAC));
}

VOID OutputMmcFeatureRandomReadable(
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
		BOOLEAN_TO_STRING_YES_NO(pRandomReadable->ErrorRecoveryPagePresent));
}

VOID OutputMmcFeatureMultiRead(
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

VOID OutputMmcFeatureCdRead(
	PFEATURE_DATA_CD_READ pCDRead
	)
{
	OutputDriveLogA(
		"\tFeatureCdRead\n"
		"\t\t          CDText: %s\n"
		"\t\t     C2ErrorData: %s\n"
		"\t\tDigitalAudioPlay: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pCDRead->CDText),
		BOOLEAN_TO_STRING_YES_NO(pCDRead->C2ErrorData),
		BOOLEAN_TO_STRING_YES_NO(pCDRead->DigitalAudioPlay));
}

VOID OutputMmcFeatureDvdRead(
	PFEATURE_DATA_DVD_READ pDVDRead
	)
{
	OutputDriveLogA(
		"\tFeatureDvdRead\n"
		"\t\t  Multi110: %s\n"
		"\t\t DualDashR: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pDVDRead->Multi110),
		BOOLEAN_TO_STRING_YES_NO(pDVDRead->DualDashR));
}

VOID OutputMmcFeatureRandomWritable(
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
		BOOLEAN_TO_STRING_YES_NO(pRandomWritable->ErrorRecoveryPagePresent));
}

VOID OutputMmcFeatureIncrementalStreamingWritable(
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
		BOOLEAN_TO_STRING_YES_NO(pIncremental->BufferUnderrunFree),
		BOOLEAN_TO_STRING_YES_NO(pIncremental->AddressModeReservation),
		BOOLEAN_TO_STRING_YES_NO(pIncremental->TrackRessourceInformation),
		pIncremental->NumberOfLinkSizes);
	for (INT i = 0; i < pIncremental->NumberOfLinkSizes; i++) {
		OutputDriveLogA(
			"\t\tLinkSize%u: %u\n", i, pIncremental->LinkSize[i]);
	}
}

VOID OutputMmcFeatureSectorErasable(
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

VOID OutputMmcFeatureFormattable(
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
	OutputDriveLogA(
		"\tFeatureDefectManagement\n"
		"\t\tSupplimentalSpareArea: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pDefect->SupplimentalSpareArea));
}

VOID OutputMmcFeatureWriteOnce(
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
		BOOLEAN_TO_STRING_YES_NO(pWriteOnce->ErrorRecoveryPagePresent));
}

VOID OutputMmcFeatureRestrictedOverwrite(
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

VOID OutputMmcFeatureCdrwCAVWrite(
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

VOID OutputMmcFeatureMrw(
	PFEATURE_DATA_MRW pMrw
	)
{
	OutputDriveLogA(
		"\tFeatureMrw\n"
		"\t\t       Write: %s\n"
		"\t\t DvdPlusRead: %s\n"
		"\t\tDvdPlusWrite: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pMrw->Write),
		BOOLEAN_TO_STRING_YES_NO(pMrw->DvdPlusRead),
		BOOLEAN_TO_STRING_YES_NO(pMrw->DvdPlusWrite));
}

VOID OutputMmcFeatureEnhancedDefectReporting(
	PFEATURE_ENHANCED_DEFECT_REPORTING pEnhanced
	)
{
	OutputDriveLogA(
		"\tFeatureEnhancedDefectReporting\n"
		"\t\t       DRTDMSupported: %s\n"
		"\t\tNumberOfDBICacheZones: %u\n"
		"\t\t      NumberOfEntries: %u\n",
		BOOLEAN_TO_STRING_YES_NO(pEnhanced->DRTDMSupported),
		pEnhanced->NumberOfDBICacheZones,
		MAKEWORD(pEnhanced->NumberOfEntries[1], pEnhanced->NumberOfEntries[0]));
}

VOID OutputMmcFeatureDvdPlusRW(
	PFEATURE_DATA_DVD_PLUS_RW pDVDPLUSRW
	)
{
	OutputDriveLogA(
		"\tFeatureDvdPlusRW\n"
		"\t\t     Write: %s\n"
		"\t\t CloseOnly: %s\n"
		"\t\tQuickStart: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pDVDPLUSRW->Write),
		BOOLEAN_TO_STRING_YES_NO(pDVDPLUSRW->CloseOnly),
		BOOLEAN_TO_STRING_YES_NO(pDVDPLUSRW->QuickStart));
}

VOID OutputMmcFeatureDvdPlusR(
	PFEATURE_DATA_DVD_PLUS_R pDVDPLUSR
	)
{
	OutputDriveLogA(
		"\tFeatureDvdPlusR\n"
		"\t\tWrite: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pDVDPLUSR->Write));
}

VOID OutputMmcFeatureRigidRestrictedOverwrite(
	PFEATURE_DATA_DVD_RW_RESTRICTED_OVERWRITE pDVDRWRestricted
	)
{
	OutputDriveLogA(
		"\tFeatureRigidRestrictedOverwrite\n"
		"\t\t                   Blank: %s\n"
		"\t\t            Intermediate: %s\n"
		"\t\t    DefectStatusDataRead: %s\n"
		"\t\tDefectStatusDataGenerate: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pDVDRWRestricted->Blank),
		BOOLEAN_TO_STRING_YES_NO(pDVDRWRestricted->Intermediate),
		BOOLEAN_TO_STRING_YES_NO(pDVDRWRestricted->DefectStatusDataRead),
		BOOLEAN_TO_STRING_YES_NO(pDVDRWRestricted->DefectStatusDataGenerate));
}

VOID OutputMmcFeatureCdTrackAtOnce(
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
	OutputDriveLogA(
		"\tFeatureDvdRecordableWrite\n"
		"\t\t            DVD_RW: %s\n"
		"\t\t         TestWrite: %s\n"
		"\t\t        RDualLayer: %s\n"
		"\t\tBufferUnderrunFree: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pDVDRecordable->DVD_RW),
		BOOLEAN_TO_STRING_YES_NO(pDVDRecordable->TestWrite),
		BOOLEAN_TO_STRING_YES_NO(pDVDRecordable->RDualLayer),
		BOOLEAN_TO_STRING_YES_NO(pDVDRecordable->BufferUnderrunFree));
}

VOID OutputMmcFeatureLayerJumpRecording(
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

VOID OutputMmcFeatureCDRWMediaWriteSupport(
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
	OutputDriveLogA(
		"\tFeatureDvdPlusRWDualLayer\n"
		"\t\t     Write: %s\n"
		"\t\t CloseOnly: %s\n"
		"\t\tQuickStart: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pDVDPlusRWDL->Write),
		BOOLEAN_TO_STRING_YES_NO(pDVDPlusRWDL->CloseOnly),
		BOOLEAN_TO_STRING_YES_NO(pDVDPlusRWDL->QuickStart));
}

VOID OutputMmcFeatureDvdPlusRDualLayer(
	PFEATURE_DATA_DVD_PLUS_R_DUAL_LAYER pDVDPlusRDL
	)
{
	OutputDriveLogA(
		"\tFeatureDvdPlusRDualLayer\n"
		"\t\tWrite: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pDVDPlusRDL->Write));
}

VOID OutputMmcFeatureHybridDisc(
	PFEATURE_HYBRID_DISC pHybridDisc
	)
{
	OutputDriveLogA(
		"\tFeatureHybridDisc\n"
		"\t\tResetImmunity: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pHybridDisc->ResetImmunity));
}

VOID OutputMmcFeaturePowerManagement(
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

VOID OutputMmcFeatureSMART(
	PFEATURE_DATA_SMART pSmart
	)
{
	OutputDriveLogA(
		"\tFeatureSMART\n"
		"\t\tFaultFailureReportingPagePresent: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pSmart->FaultFailureReportingPagePresent));
}

VOID OutputMmcFeatureEmbeddedChanger(
	PFEATURE_DATA_EMBEDDED_CHANGER pEmbedded
	)
{
	OutputDriveLogA(
		"\tFeatureEmbeddedChanger\n"
		"\t\tSupportsDiscPresent: %s\n"
		"\t\t  SideChangeCapable: %s\n"
		"\t\t  HighestSlotNumber: %u\n",
		BOOLEAN_TO_STRING_YES_NO(pEmbedded->SupportsDiscPresent),
		BOOLEAN_TO_STRING_YES_NO(pEmbedded->SideChangeCapable),
		pEmbedded->HighestSlotNumber);
}

VOID OutputMmcFeatureCDAudioAnalogPlay(
	PFEATURE_DATA_CD_AUDIO_ANALOG_PLAY pCDAudio
	)
{
	OutputDriveLogA(
		"\tFeatureCDAudioAnalogPlay\n"
		"\t\t     SeperateVolume: %s\n"
		"\t\tSeperateChannelMute: %s\n"
		"\t\t      ScanSupported: %s\n"
		"\t\tNumerOfVolumeLevels: %u\n",
		BOOLEAN_TO_STRING_YES_NO(pCDAudio->SeperateVolume),
		BOOLEAN_TO_STRING_YES_NO(pCDAudio->SeperateChannelMute),
		BOOLEAN_TO_STRING_YES_NO(pCDAudio->ScanSupported),
		MAKEWORD(pCDAudio->NumerOfVolumeLevels[1], pCDAudio->NumerOfVolumeLevels[0]));
}

VOID OutputMmcFeatureMicrocodeUpgrade(
	PFEATURE_DATA_MICROCODE_UPDATE pMicrocode
	)
{
	OutputDriveLogA(
		"\tFeatureMicrocodeUpgrade\n"
		"\t\tM5: %s\n",
		BOOLEAN_TO_STRING_YES_NO(pMicrocode->M5));
}

VOID OutputMmcFeatureTimeout(
	PFEATURE_DATA_TIMEOUT pTimeOut
	)
{
	OutputDriveLogA(
		"\tFeatureTimeout\n"
		"\t\t    Group3: %s\n"
		"\t\tUnitLength: %u\n",
		BOOLEAN_TO_STRING_YES_NO(pTimeOut->Group3),
		MAKEWORD(pTimeOut->UnitLength[1], pTimeOut->UnitLength[0]));
}

VOID OutputMmcFeatureDvdCSS(
	PFEATURE_DATA_DVD_CSS pDVDCss
	)
{
	OutputDriveLogA(
		"\tFeatureDvdCSS\n"
		"\t\tCssVersion: %u\n",
		pDVDCss->CssVersion);
}

VOID OutputMmcFeatureRealTimeStreaming(
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
	OutputDriveLogA(
		"\tFeatureLogicalUnitSerialNumber\n"
		"\t\tSerialNumber: ");
	for (INT i = 0; i < pLogical->Header.AdditionalLength; i++) {
		OutputDriveLogA("%c", pLogical->SerialNumber[i]);
	}
	OutputDriveLogA("\n");
}

VOID OutputMmcFeatureMediaSerialNumber(
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

VOID OutputMmcFeatureDiscControlBlocks(
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

VOID OutputMmcFeatureDvdCPRM(
	PFEATURE_DATA_DVD_CPRM pDVDCprm
	)
{
	OutputDriveLogA(
		"\tFeatureDvdCPRM\n"
		"\t\tCPRMVersion: %u\n",
		pDVDCprm->CPRMVersion);
}

VOID OutputMmcFeatureFirmwareDate(
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

VOID OutputMmcFeatureAACS(
	PFEATURE_DATA_AACS pAACS
	)
{
	OutputDriveLogA(
		"\tFeatureAACS\n"
		"\t\tBindingNonceGeneration: %s\n"
		"\t\tBindingNonceBlockCount: %u\n"
		"\t\t         NumberOfAGIDs: %u\n"
		"\t\t           AACSVersion: %u\n",
		BOOLEAN_TO_STRING_YES_NO(pAACS->BindingNonceGeneration),
		pAACS->BindingNonceBlockCount,
		pAACS->NumberOfAGIDs,
		pAACS->AACSVersion);
}

VOID OutputMmcFeatureVCPS(
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

VOID OutputMmcFeatureProfileList(
	PFEATURE_DATA_PROFILE_LIST pList
	)
{
	OutputDriveLogA("\tFeatureProfileList\n");
	for (UINT i = 0; i < pList->Header.AdditionalLength / sizeof(FEATURE_DATA_PROFILE_LIST_EX); i++) {
		OutputDriveLogA("\t\t");
		OutputMmcFeatureProfileType(
			MAKEWORD(pList->Profiles[i].ProfileNumber[1], pList->Profiles[i].ProfileNumber[0]));
		OutputDriveLogA("\n");
	}
}

VOID OutputMmcFeatureVendorSpecific(
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

VOID OutputMmcFeatureReserved(
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

VOID OutputMmcFeatureNumber(
	PDEVICE_DATA pDevData,
	LPBYTE lpConf,
	DWORD dwAllLen
	)
{
	DWORD n = 0;
	while (n < dwAllLen) {
		WORD wCode = MAKEWORD(lpConf[n + 1], lpConf[n]);
		switch (wCode) {
		case FeatureProfileList:
			OutputMmcFeatureProfileList((PFEATURE_DATA_PROFILE_LIST)&lpConf[n]);
			break;
		case FeatureCore:
			OutputMmcFeatureCore((PFEATURE_DATA_CORE)&lpConf[n]);
			break;
		case FeatureMorphing:
			OutputMmcFeatureMorphing((PFEATURE_DATA_MORPHING)&lpConf[n]);
			break;
		case FeatureRemovableMedium:
			OutputMmcFeatureRemovableMedium((PFEATURE_DATA_REMOVABLE_MEDIUM)&lpConf[n]);
			break;
		case FeatureWriteProtect:
			OutputMmcFeatureWriteProtect((PFEATURE_DATA_WRITE_PROTECT)&lpConf[n]);
			break;
		case FeatureRandomReadable:
			OutputMmcFeatureRandomReadable((PFEATURE_DATA_RANDOM_READABLE)&lpConf[n]);
			break;
		case FeatureMultiRead:
			OutputMmcFeatureMultiRead((PFEATURE_DATA_MULTI_READ)&lpConf[n]);
			break;
		case FeatureCdRead:
			OutputMmcFeatureCdRead((PFEATURE_DATA_CD_READ)&lpConf[n]);
			SetMmcFeatureCdRead((PFEATURE_DATA_CD_READ)&lpConf[n], pDevData);
			break;
		case FeatureDvdRead:
			OutputMmcFeatureDvdRead((PFEATURE_DATA_DVD_READ)&lpConf[n]);
			break;
		case FeatureRandomWritable:
			OutputMmcFeatureRandomWritable((PFEATURE_DATA_RANDOM_WRITABLE)&lpConf[n]);
			break;
		case FeatureIncrementalStreamingWritable:
			OutputMmcFeatureIncrementalStreamingWritable((PFEATURE_DATA_INCREMENTAL_STREAMING_WRITABLE)&lpConf[n]);
			break;
		case FeatureSectorErasable:
			OutputMmcFeatureSectorErasable((PFEATURE_DATA_SECTOR_ERASABLE)&lpConf[n]);
			break;
		case FeatureFormattable:
			OutputMmcFeatureFormattable((PFEATURE_DATA_FORMATTABLE)&lpConf[n]);
			break;
		case FeatureDefectManagement:
			OutputMmcFeatureDefectManagement((PFEATURE_DATA_DEFECT_MANAGEMENT)&lpConf[n]);
			break;
		case FeatureWriteOnce:
			OutputMmcFeatureWriteOnce((PFEATURE_DATA_WRITE_ONCE)&lpConf[n]);
			break;
		case FeatureRestrictedOverwrite:
			OutputMmcFeatureRestrictedOverwrite((PFEATURE_DATA_RESTRICTED_OVERWRITE)&lpConf[n]);
			break;
		case FeatureCdrwCAVWrite:
			OutputMmcFeatureCdrwCAVWrite((PFEATURE_DATA_CDRW_CAV_WRITE)&lpConf[n]);
			break;
		case FeatureMrw:
			OutputMmcFeatureMrw((PFEATURE_DATA_MRW)&lpConf[n]);
			break;
		case FeatureEnhancedDefectReporting:
			OutputMmcFeatureEnhancedDefectReporting((PFEATURE_ENHANCED_DEFECT_REPORTING)&lpConf[n]);
			break;
		case FeatureDvdPlusRW:
			OutputMmcFeatureDvdPlusRW((PFEATURE_DATA_DVD_PLUS_RW)&lpConf[n]);
			break;
		case FeatureDvdPlusR:
			OutputMmcFeatureDvdPlusR((PFEATURE_DATA_DVD_PLUS_R)&lpConf[n]);
			break;
		case FeatureRigidRestrictedOverwrite:
			OutputMmcFeatureRigidRestrictedOverwrite((PFEATURE_DATA_DVD_RW_RESTRICTED_OVERWRITE)&lpConf[n]);
			break;
		case FeatureCdTrackAtOnce:
			OutputMmcFeatureCdTrackAtOnce((PFEATURE_DATA_CD_TRACK_AT_ONCE)&lpConf[n]);
			break;
		case FeatureCdMastering:
			OutputMmcFeatureCdMastering((PFEATURE_DATA_CD_MASTERING)&lpConf[n]);
			break;
		case FeatureDvdRecordableWrite:
			OutputMmcFeatureDvdRecordableWrite((PFEATURE_DATA_DVD_RECORDABLE_WRITE)&lpConf[n]);
			break;
		case FeatureLayerJumpRecording:
			OutputMmcFeatureLayerJumpRecording((PFEATURE_DATA_LAYER_JUMP_RECORDING)&lpConf[n]);
			break;
		case FeatureCDRWMediaWriteSupport:
			OutputMmcFeatureCDRWMediaWriteSupport((PFEATURE_CD_RW_MEDIA_WRITE_SUPPORT)&lpConf[n]);
			break;
		case FeatureDvdPlusRWDualLayer:
			OutputMmcFeatureDvdPlusRWDualLayer((PFEATURE_DATA_DVD_PLUS_RW_DUAL_LAYER)&lpConf[n]);
			break;
		case FeatureDvdPlusRDualLayer:
			OutputMmcFeatureDvdPlusRDualLayer((PFEATURE_DATA_DVD_PLUS_R_DUAL_LAYER)&lpConf[n]);
			break;
		case FeatureHybridDisc:
			OutputMmcFeatureHybridDisc((PFEATURE_HYBRID_DISC)&lpConf[n]);
			break;
		case FeaturePowerManagement:
			OutputMmcFeaturePowerManagement((PFEATURE_DATA_POWER_MANAGEMENT)&lpConf[n]);
			break;
		case FeatureSMART:
			OutputMmcFeatureSMART((PFEATURE_DATA_SMART)&lpConf[n]);
			break;
		case FeatureEmbeddedChanger:
			OutputMmcFeatureEmbeddedChanger((PFEATURE_DATA_EMBEDDED_CHANGER)&lpConf[n]);
			break;
		case FeatureCDAudioAnalogPlay:
			OutputMmcFeatureCDAudioAnalogPlay((PFEATURE_DATA_CD_AUDIO_ANALOG_PLAY)&lpConf[n]);
			break;
		case FeatureMicrocodeUpgrade:
			OutputMmcFeatureMicrocodeUpgrade((PFEATURE_DATA_MICROCODE_UPDATE)&lpConf[n]);
			break;
		case FeatureTimeout:
			OutputMmcFeatureTimeout((PFEATURE_DATA_TIMEOUT)&lpConf[n]);
			break;
		case FeatureDvdCSS:
			OutputMmcFeatureDvdCSS((PFEATURE_DATA_DVD_CSS)&lpConf[n]);
			break;
		case FeatureRealTimeStreaming:
			OutputMmcFeatureRealTimeStreaming((PFEATURE_DATA_REAL_TIME_STREAMING)&lpConf[n]);
			break;
		case FeatureLogicalUnitSerialNumber:
			OutputMmcFeatureLogicalUnitSerialNumber((PFEATURE_DATA_LOGICAL_UNIT_SERIAL_NUMBER)&lpConf[n]);
			break;
		case FeatureMediaSerialNumber:
			OutputMmcFeatureMediaSerialNumber((PFEATURE_MEDIA_SERIAL_NUMBER)&lpConf[n]);
			break;
		case FeatureDiscControlBlocks:
			OutputMmcFeatureDiscControlBlocks((PFEATURE_DATA_DISC_CONTROL_BLOCKS)&lpConf[n]);
			break;
		case FeatureDvdCPRM:
			OutputMmcFeatureDvdCPRM((PFEATURE_DATA_DVD_CPRM)&lpConf[n]);
			break;
		case FeatureFirmwareDate:
			OutputMmcFeatureFirmwareDate((PFEATURE_DATA_FIRMWARE_DATE)&lpConf[n]);
			break;
		case FeatureAACS:
			OutputMmcFeatureAACS((PFEATURE_DATA_AACS)&lpConf[n]);
			break;
		case FeatureVCPS:
			OutputMmcFeatureVCPS((PFEATURE_VCPS)&lpConf[n]);
			break;
		default:
			if (0x0111 <= wCode && wCode <= 0xFEFF) {
				OutputMmcFeatureReserved((PFEATURE_DATA_RESERVED)&lpConf[n]);
			}
			else if (0xFF00 <= wCode && wCode <= 0xFFFF) {
				OutputMmcFeatureVendorSpecific((PFEATURE_DATA_VENDOR_SPECIFIC)&lpConf[n]);
			}
			break;
		}
		n += sizeof(FEATURE_HEADER) + lpConf[n + 3];
	}
}

VOID OutputMmcFeatureProfileType(
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
		OutputDriveLogA("DVD-R LayerJump");
		break;
	case ProfileDvdPlusRW:
		OutputDriveLogA("DVD+RW");
		break;
	case ProfileDvdPlusR:
		OutputDriveLogA("DVD+R");
		break;
	case ProfileDvdPlusRWDualLayer:
		OutputDriveLogA("DVD+RW DL");
		break;
	case ProfileDvdPlusRDualLayer:
		OutputDriveLogA("DVD+R DL");
		break;
	case ProfileNonStandard:
		OutputDriveLogA("NonStandard");
		break;
	default:
		OutputDriveLogA("Reserved [%x]", wFeatureProfileType);
		break;
	}
}

VOID OutputMmcInquiryData(
	PDEVICE_DATA pDevData,
	PINQUIRYDATA pInquiry
	)
{
	OutputDriveLogA(
		"InquiryData\n"
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
