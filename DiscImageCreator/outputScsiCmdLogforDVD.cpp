/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "output.h"
#include "outputScsiCmdLogforCD.h"

VOID OutputFsVolumeStructureDescriptorFormat(
	LPBYTE lpBuf
	)
{
	OutputVolDescLogA(
		"========================= Volume Recognition Sequence =========================\n"
		"\t                               Structure Type: %u\n"
		"\t                          Standard Identifier: %.5s\n"
		"\t                            Structure Version: %u\n",
		lpBuf[0],
		(PCHAR)&lpBuf[1],
		lpBuf[6]);
}

VOID OutputFsRecordingDateAndTime(
	LPBYTE lpBuf
	)
{
	OutputVolDescLogA(
		"\tRecording Date and Time: %x %u-%02u-%02u %02u:%02u:%02u.%u.%u.%u\n",
		MAKEWORD(lpBuf[0], lpBuf[1]), MAKEWORD(lpBuf[2], lpBuf[3]), lpBuf[4],
		lpBuf[5], lpBuf[6], lpBuf[7], lpBuf[8], lpBuf[9], lpBuf[10], lpBuf[11]);
}

VOID OutputFsBootDescriptor(
	LPBYTE lpBuf
	)
{
	OutputVolDescLogA(
		"\tArchitecture Type\n"
		"\t\t            Flags: %u\n"
		"\t\t       Identifier: %.23s\n"
		"\t\tIdentifier Suffix: %.8s\n"
		"\tBoot Identifier\n"
		"\t\t            Flags: %u\n"
		"\t\t       Identifier: %.23s\n"
		"\t\tIdentifier Suffix: %.8s\n"
		"\tBoot Extent Location: %u\n"
		"\t  Boot Extent Length: %u\n"
		"\t        Load Address: %u%u\n"
		"\t       Start Address: %u%u\n",
		lpBuf[8],
		(PCHAR)&lpBuf[9],
		(PCHAR)&lpBuf[24],
		lpBuf[40],
		(PCHAR)&lpBuf[41],
		(PCHAR)&lpBuf[64],
		MAKELONG(MAKEWORD(lpBuf[72], lpBuf[73]), MAKEWORD(lpBuf[74], lpBuf[75])),
		MAKELONG(MAKEWORD(lpBuf[76], lpBuf[77]), MAKEWORD(lpBuf[78], lpBuf[79])),
		MAKELONG(MAKEWORD(lpBuf[80], lpBuf[81]), MAKEWORD(lpBuf[82], lpBuf[83])),
		MAKELONG(MAKEWORD(lpBuf[84], lpBuf[85]), MAKEWORD(lpBuf[86], lpBuf[87])),
		MAKELONG(MAKEWORD(lpBuf[88], lpBuf[89]), MAKEWORD(lpBuf[90], lpBuf[91])),
		MAKELONG(MAKEWORD(lpBuf[92], lpBuf[93]), MAKEWORD(lpBuf[94], lpBuf[95])));

	OutputFsRecordingDateAndTime(lpBuf + 96);
	OutputVolDescLogA(
		"\t               Flags: %u\n"
		"\t            Boot Use: ",
		MAKEWORD(lpBuf[108], lpBuf[109]));
	for (INT i = 142; i < 2048; i++) {
		OutputVolDescLogA("%x", lpBuf[i]);
	}
	OutputVolDescLogA("\n");
}

VOID OutputFsVolumeRecognitionSequence(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpBuf
	)
{
	if (lpBuf[0] == 1 && !strncmp((PCHAR)&lpBuf[1], "CD001", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf);
		OutputFsVolumeDescriptorForISO9660(pExtArg, pDisc, lpBuf);
	}
	else if (lpBuf[0] == 2 && !strncmp((PCHAR)&lpBuf[1], "CD001", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf);
		OutputVolDescLogA("\t                                 Volume Flags: %u\n", lpBuf[7]);
		OutputFsVolumeDescriptorForJoliet(pExtArg, pDisc, lpBuf);
	}
	else if (lpBuf[0] == 0xff && !strncmp((PCHAR)&lpBuf[1], "CD001", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf);
	}
	else if (lpBuf[0] == 0 && !strncmp((PCHAR)&lpBuf[1], "BOOT2", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf);
		OutputFsBootDescriptor(lpBuf);
	}
	else if (lpBuf[0] == 0 && !strncmp((PCHAR)&lpBuf[1], "BEA01", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf);
	}
	else if (lpBuf[0] == 0 && !strncmp((PCHAR)&lpBuf[1], "NSR02", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf);
	}
	else if (lpBuf[0] == 0 && !strncmp((PCHAR)&lpBuf[1], "NSR03", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf);
	}
	else if (lpBuf[0] == 0 && !strncmp((PCHAR)&lpBuf[1], "TEA01", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf);
	}
}

VOID OutputFsCharspec(
	LPBYTE lpBuf
	)
{
	OutputVolDescLogA(
		"\t\t       Character Set Type: %u\n"
		"\t\tCharacter Set Information: %.23s\n",
		lpBuf[0], (PCHAR)&lpBuf[1]);
}

VOID OutputFsExtentDescriptor(
	LPBYTE lpBuf
	)
{
	OutputVolDescLogA(
		"\t\t  Extent Length: %u\n"
		"\t\tExtent Location: %u\n",
		MAKELONG(MAKEWORD(lpBuf[0], lpBuf[1]), MAKEWORD(lpBuf[2], lpBuf[3])),
		MAKELONG(MAKEWORD(lpBuf[4], lpBuf[5]), MAKEWORD(lpBuf[6], lpBuf[7])));
}

VOID OutputFsRegid(
	LPBYTE lpBuf
	)
{
	OutputVolDescLogA(
		"\t\t            Flags: %u\n"
		"\t\t       Identifier: %.23s\n"
		"\t\tIdentifier Suffix: ",
		lpBuf[0],
		(PCHAR)&lpBuf[1]);
	for (INT i = 24; i < 32; i++) {
		OutputVolDescLogA("%x", lpBuf[i]);
	}
	OutputVolDescLogA("\n");
}

VOID OutputFsPrimaryVolumeDescriptorForUDF(
	LPBYTE lpBuf
	)
{
	OutputVolDescLogA(
		"\tVolume Descriptor Sequence Number: %u\n"
		"\t Primary Volume Descriptor Number: %u\n"
		"\t                Volume Identifier: %.32s\n"
		"\t           Volume Sequence Number: %u\n"
		"\t   Maximum Volume Sequence Number: %u\n"
		"\t                Interchange Level: %u\n"
		"\t        Maximum Interchange Level: %u\n"
		"\t               Character Set List: %u\n"
		"\t       Maximum Character Set List: %u\n"
		"\t            Volume Set Identifier: %.128s\n"
		"\tDescriptor Character Set\n",
		MAKELONG(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])),
		MAKELONG(MAKEWORD(lpBuf[20], lpBuf[21]), MAKEWORD(lpBuf[22], lpBuf[23])),
		(PCHAR)&lpBuf[24],
		MAKEWORD(lpBuf[56], lpBuf[57]),
		MAKEWORD(lpBuf[58], lpBuf[59]),
		MAKEWORD(lpBuf[60], lpBuf[61]),
		MAKEWORD(lpBuf[62], lpBuf[63]),
		MAKELONG(MAKEWORD(lpBuf[64], lpBuf[65]), MAKEWORD(lpBuf[66], lpBuf[67])),
		MAKELONG(MAKEWORD(lpBuf[68], lpBuf[69]), MAKEWORD(lpBuf[70], lpBuf[71])),
		(PCHAR)&lpBuf[72]);

	OutputFsCharspec(lpBuf + 200);

	OutputVolDescLogA("\tExplanatory Character Set\n");
	OutputFsCharspec(lpBuf + 264);

	OutputVolDescLogA("\tVolume Abstract\n");
	OutputFsExtentDescriptor(lpBuf + 328);

	OutputVolDescLogA("\tVolume Copyright Notice\n");
	OutputFsExtentDescriptor(lpBuf + 336);

	OutputVolDescLogA("\tApplication Identifier\n");
	OutputFsRegid(lpBuf + 344);

	OutputFsRecordingDateAndTime(lpBuf + 376);

	OutputVolDescLogA("\tImplementation Identifier\n");
	OutputFsRegid(lpBuf + 388);
	OutputVolDescLogA("\tImplementation Use: ");
	for (INT i = 420; i < 484; i++) {
		OutputVolDescLogA("%x", lpBuf[i]);
	}
	OutputVolDescLogA("\n");

	OutputVolDescLogA(
		"\tPredecessor Volume Descriptor Sequence Location: %u\n",
		MAKELONG(MAKEWORD(lpBuf[484], lpBuf[485]), MAKEWORD(lpBuf[486], lpBuf[487])));
	OutputVolDescLogA(
		"\t                                          Flags: %u\n",
		MAKEWORD(lpBuf[488], lpBuf[489]));
}

VOID OutputFsAnchorVolumeDescriptorPointer(
	LPBYTE lpBuf
	)
{
	OutputVolDescLogA("\tMain Volume Descriptor Sequence Extent\n");
	OutputFsExtentDescriptor(lpBuf + 16);
	OutputVolDescLogA("\tReserve Volume Descriptor Sequence Extent\n");
	OutputFsExtentDescriptor(lpBuf + 24);
}

VOID OutputFsVolumeDescriptorPointer(
	LPBYTE lpBuf
	)
{
	OutputVolDescLogA(
		"\t     Volume Descriptor Sequence Number: %u\n"
		"\tNext Volume Descriptor Sequence Extent\n",
		MAKELONG(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])));
	OutputFsExtentDescriptor(lpBuf + 20);
}

VOID OutputFsImplementationUseVolumeDescriptor(
	LPBYTE lpBuf
	)
{
	OutputVolDescLogA(
		"\tVolume Descriptor Sequence Number: %u\n"
		"\tImplementation Identifier\n",
		MAKELONG(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])));

	OutputFsRegid(lpBuf + 20);

	OutputVolDescLogA("\tLVI Charset\n");
	OutputFsCharspec(lpBuf + 52);

	OutputVolDescLogA(
		"\tLogical Volume Identifier: %.128s\n"
		"\t                LV Info 1: %.36s\n"
		"\t                LV Info 2: %.36s\n"
		"\t                LV Info 3: %.36s\n"
		"\tImplemention ID\n"
		, (PCHAR)&lpBuf[116]
		, (PCHAR)&lpBuf[244]
		, (PCHAR)&lpBuf[280]
		, (PCHAR)&lpBuf[316]);

	OutputFsRegid(lpBuf + 352);
	OutputVolDescLogA("\tImplementation Use: ");
	for (INT i = 384; i < 512; i++) {
		OutputVolDescLogA("%x", lpBuf[i]);
	}
	OutputVolDescLogA("\n");
}

VOID OutputFsPartitionDescriptor(
	LPBYTE lpBuf
	)
{
	OutputVolDescLogA(
		"\tVolume Descriptor Sequence Number: %u\n"
		"\t                  Partition Flags: %u\n"
		"\t                 Partition Number: %u\n"
		"\tPartition Contents\n",
		MAKELONG(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])),
		MAKEWORD(lpBuf[20], lpBuf[21]),
		MAKEWORD(lpBuf[22], lpBuf[23]));

	OutputFsRegid(lpBuf + 24);

	OutputVolDescLogA("\tPartition Contents Use: ");
	for (INT i = 56; i < 184; i++) {
		OutputVolDescLogA("%x", lpBuf[i]);
	}

	OutputVolDescLogA(
		"\n"
		"\t                Access Type: %u\n"
		"\tPartition Starting Location: %u\n"
		"\t           Partition Length: %u\n"
		"\tImplementation Identifier\n",
		MAKELONG(MAKEWORD(lpBuf[184], lpBuf[185]), MAKEWORD(lpBuf[186], lpBuf[187])),
		MAKELONG(MAKEWORD(lpBuf[188], lpBuf[189]), MAKEWORD(lpBuf[190], lpBuf[191])),
		MAKELONG(MAKEWORD(lpBuf[192], lpBuf[193]), MAKEWORD(lpBuf[194], lpBuf[195])));

	OutputFsRegid(lpBuf + 196);
	OutputVolDescLogA("\tImplementation Use: ");
	for (INT i = 228; i < 356; i++) {
		OutputVolDescLogA("%x", lpBuf[i]);
	}
	OutputVolDescLogA("\n");
}

VOID OutputFsLongAllocationDescriptor(
	LPBYTE lpBuf
	)
{
	OutputVolDescLogA(
		"\tLongAllocationDescriptor\n"
		"\t\t             Extent Length: %u\n"
		"\t\t      Logical Block Number: %u\n"
		"\t\tPartition Reference Number: %u\n",
		MAKELONG(MAKEWORD(lpBuf[0], lpBuf[1]), MAKEWORD(lpBuf[2], lpBuf[3])),
		MAKELONG(MAKEWORD(lpBuf[4], lpBuf[5]), MAKEWORD(lpBuf[6], lpBuf[7])),
		MAKEWORD(lpBuf[8], lpBuf[9]));
}

VOID OutputFsLogicalVolumeDescriptor(
	LPBYTE lpBuf
	)
{
	OutputVolDescLogA(
		"\tVolume Descriptor Sequence Number: %u\n"
		"\tDescriptor Character Set\n",
		MAKELONG(MAKEWORD(lpBuf[16], lpBuf[17]),
		MAKEWORD(lpBuf[18], lpBuf[19])));

	OutputFsCharspec(lpBuf + 20);

	OutputVolDescLogA(
		"\tLogical Volume Identifier: %.128s\n"
		"\t      Logical Block Size : %u\n"
		"\tDomain Identifier\n",
		(PCHAR)&lpBuf[84],
		MAKELONG(MAKEWORD(lpBuf[212], lpBuf[213]),
		MAKEWORD(lpBuf[214], lpBuf[215])));

	OutputFsCharspec(lpBuf + 216);
	OutputFsLongAllocationDescriptor(lpBuf + 248);

	LONG MT_L = MAKELONG(MAKEWORD(lpBuf[264], lpBuf[265]),
		MAKEWORD(lpBuf[266], lpBuf[267]));
	OutputVolDescLogA(
		"\t        Map Table Length: %u\n"
		"\tNumber of Partition Maps: %u\n"
		"\tImplementation Identifier\n",
		MT_L,
		MAKELONG(MAKEWORD(lpBuf[268], lpBuf[269]),
		MAKEWORD(lpBuf[270], lpBuf[271])));

	OutputFsRegid(lpBuf + 272);

	OutputVolDescLogA("\tImplementation Use: ");
	for (INT i = 304; i < 432; i++) {
		OutputVolDescLogA("%x", lpBuf[i]);
	}
	OutputVolDescLogA("\n");
	OutputVolDescLogA("\tIntegrity Sequence Extent\n");
	OutputFsExtentDescriptor(lpBuf + 432);

	OutputVolDescLogA("\tPartition Maps: ");
	for (INT i = 0; i < MT_L; i++) {
		OutputVolDescLogA("%x", lpBuf[440 + i]);
	}
	OutputVolDescLogA("\n");
}

VOID OutputFsUnallocatedSpaceDescriptor(
	LPBYTE lpBuf
	)
{
	LONG N_AD =
		MAKELONG(MAKEWORD(lpBuf[20], lpBuf[21]), MAKEWORD(lpBuf[22], lpBuf[23]));
	OutputVolDescLogA(
		"\tVolume Descriptor Sequence Number: %u\n"
		"\t Number of Allocation Descriptors: %u\n"
		"\tAllocation Descriptors\n",
		MAKELONG(MAKEWORD(lpBuf[16], lpBuf[17]), MAKEWORD(lpBuf[18], lpBuf[19])),
		N_AD);
	for (INT i = 0; i < N_AD * 8; i += 8) {
		OutputFsExtentDescriptor(lpBuf + 24 + i);
	}
}

VOID OutputFsVolumeDescriptorSequence(
	LPBYTE lpBuf
	)
{
	WORD wTagId = MAKEWORD(lpBuf[0], lpBuf[1]);
	if (wTagId == 0 || (10 <= wTagId && wTagId <= 255) || 267 <= wTagId) {
		return;
	}
	switch (wTagId) {
	case 1:
		OutputVolDescLogA(
			"========================== Primary Volume Descriptor ==========================\n");
		break;
	case 2:
		OutputVolDescLogA(
			"====================== Anchor Volume Descriptor Pointer =======================\n");
		break;
	case 3:
		OutputVolDescLogA(
			"========================== Volume Descriptor Pointer ==========================\n");
		break;
	case 4:
		OutputVolDescLogA(
			"==================== Implementation Use Volume Descriptor =====================\n");
		break;
	case 5:
		OutputVolDescLogA(
			"============================ Partition Descriptor =============================\n");
		break;
	case 6:
		OutputVolDescLogA(
			"========================== Logical Volume Descriptor ==========================\n");
		break;
	case 7:
		OutputVolDescLogA(
			"======================== Unallocated Space Descriptor =========================\n");
		break;
	case 8:
		OutputVolDescLogA(
			"=========================== Terminating Descriptor ============================\n");
		break;
	case 9:
		OutputVolDescLogA(
			"===================== Logical Volume Integrity Descriptor =====================\n");
		break;
	case 256:
		OutputVolDescLogA(
			"============================= File Set Descriptor =============================\n");
		break;
	case 257:
		OutputVolDescLogA(
			"========================= File Identifier Descriptor ==========================\n");
		break;
	case 258:
		OutputVolDescLogA(
			"======================== Allocation Extent Descriptor =========================\n");
		break;
	case 259:
		OutputVolDescLogA(
			"=============================== Indirect Entry ================================\n");
		break;
	case 260:
		OutputVolDescLogA(
			"=============================== Terminal Entry ================================\n");
		break;
	case 261:
		OutputVolDescLogA(
			"================================= File Entry ==================================\n");
		break;
	case 262:
		OutputVolDescLogA(
			"===================== Extended Attribute Header Descriptor ====================\n");
		break;
	case 263:
		OutputVolDescLogA(
			"=========================== Unallocated Space Entry ===========================\n");
		break;
	case 264:
		OutputVolDescLogA(
			"=========================== Space Bitmap Descriptor ===========================\n");
		break;
	case 265:
		OutputVolDescLogA(
			"========================== Partition Integrity Entry ==========================\n");
		break;
	case 266:
		OutputVolDescLogA(
			"============================= Extended File Entry =============================\n");
		break;
	}

	OutputVolDescLogA(
		"\t\t           Descriptor Version: %u\n"
		"\t\t                 Tag Checksum: %u\n"
		"\t\t            Tag Serial Number: %u\n"
		"\t\t               Descriptor CRC: %x\n"
		"\t\t        Descriptor CRC Length: %u\n"
		"\t\t                 Tag Location: %u\n",
		MAKEWORD(lpBuf[2], lpBuf[3]),
		lpBuf[4],
		MAKEWORD(lpBuf[6], lpBuf[7]),
		MAKEWORD(lpBuf[8], lpBuf[9]),
		MAKEWORD(lpBuf[10], lpBuf[11]),
		MAKELONG(MAKEWORD(lpBuf[12], lpBuf[13]), MAKEWORD(lpBuf[14], lpBuf[15])));

	switch (wTagId) {
	case 1:
		OutputFsPrimaryVolumeDescriptorForUDF(lpBuf);
		break;
	case 2:
		OutputFsAnchorVolumeDescriptorPointer(lpBuf);
		break;
	case 3:
		OutputFsVolumeDescriptorPointer(lpBuf);
		break;
	case 4:
		OutputFsImplementationUseVolumeDescriptor(lpBuf);
		break;
	case 5:
		OutputFsPartitionDescriptor(lpBuf);
		break;
	case 6:
		OutputFsLogicalVolumeDescriptor(lpBuf);
		break;
	case 7:
		OutputFsUnallocatedSpaceDescriptor(lpBuf);
		break;
	}
	return;
}
// end for DVD

VOID OutputDVDLayerDescriptor(
	PDISC pDisc,
	PDVD_FULL_LAYER_DESCRIPTOR dvdLayer,
	LPBYTE lpLayerNum,
	UINT uiNum,
	BOOL bSuccesssReadToc
	)
{
	LPCSTR lpBookType[] = {
		"DVD-ROM", "DVD-RAM", "DVD-R", "DVD-RW",
		"HD DVD-ROM", "HD DVD-RAM", "HD DVD-R", "Reserved",
		"Reserved", "DVD+RW", "DVD+R", "Reserved",
		"Reserved", "DVD+RW DL", "DVD+R DL", "Reserved"
	};

	LPCSTR lpMaximumRate[] = {
		"2.52 Mbps", "5.04 Mbps", "10.08 Mbps", "20.16 Mbps",
		"30.24 Mbps", "Reserved", "Reserved", "Reserved",
		"Reserved", "Reserved", "Reserved", "Reserved",
		"Reserved", "Reserved", "Reserved", "Not Specified"
	};

	LPCSTR lpLayerType[] = {
		"Unknown", "Layer contains embossed data", "Layer contains recordable data", "Unknown",
		"Layer contains rewritable data", "Unknown", "Unknown", "Unknown",
		"Reserved", "Unknown", "Unknown", "Unknown",
		"Unknown", "Unknown", "Unknown", "Unknown"
	};

	LPCSTR lpTrackDensity[] = {
		"0.74ƒÊm/track", "0.80ƒÊm/track", "0.615ƒÊm/track", "0.40ƒÊm/track",
		"0.34ƒÊm/track", "Reserved", "Reserved", "Reserved",
		"Reserved", "Reserved", "Reserved", "Reserved",
		"Reserved", "Reserved", "Reserved", "Reserved"
	};

	LPCSTR lpLinearDensity[] = {
		"0.267ƒÊm/bit", "0.293ƒÊm/bit", "0.409 to 0.435ƒÊm/bit", "Reserved",
		"0.280 to 0.291ƒÊm/bit", "0.153ƒÊm/bit", "0.130 to 0.140ƒÊm/bit", "Reserved",
		"0.353ƒÊm/bit", "Reserved", "Reserved", "Reserved",
		"Reserved", "Reserved", "Reserved", "Reserved"
	};

	*lpLayerNum = dvdLayer->commonHeader.NumberOfLayers;
	DWORD dwStartSector = dvdLayer->commonHeader.StartingDataSector;
	DWORD dwEndSector = dvdLayer->commonHeader.EndDataSector;
	DWORD dwEndSectorLayer0 = dvdLayer->commonHeader.EndLayerZeroSector;
	REVERSE_LONG(&dwStartSector);
	REVERSE_LONG(&dwEndSector);
	REVERSE_LONG(&dwEndSectorLayer0);

	OutputVolDescLogA(
		"\tPhysicalFormatInformation\n"
		"\t\t       BookVersion: %u\n"
		"\t\t          BookType: %s\n"
		"\t\t       MinimumRate: %s\n"
		"\t\t          DiskSize: %s\n"
		"\t\t         LayerType: %s\n"
		"\t\t         TrackPath: %s\n"
		"\t\t    NumberOfLayers: %s\n"
		"\t\t      TrackDensity: %s\n"
		"\t\t     LinearDensity: %s\n"
		"\t\t   StartDataSector: %8u (%#x)\n"
		"\t\t     EndDataSector: %8u (%#x)\n"
		"\t\tEndLayerZeroSector: %8u (%#x)\n"
		"\t\t           BCAFlag: %s\n"
		"\t\t     MediaSpecific: ",
		dvdLayer->commonHeader.BookVersion,
		lpBookType[dvdLayer->commonHeader.BookType],
		lpMaximumRate[dvdLayer->commonHeader.MinimumRate],
		dvdLayer->commonHeader.DiskSize == 0 ? "120mm" : "80mm",
		lpLayerType[dvdLayer->commonHeader.LayerType],
		dvdLayer->commonHeader.TrackPath == 0 ? "Parallel Track Path" : "Opposite Track Path",
		dvdLayer->commonHeader.NumberOfLayers == 0 ? "Single Layer" : "Double Layer",
		lpTrackDensity[dvdLayer->commonHeader.TrackDensity],
		lpLinearDensity[dvdLayer->commonHeader.LinearDensity],
		dwStartSector, dwStartSector,
		dwEndSector, dwEndSector,
		dwEndSectorLayer0, dwEndSectorLayer0,
		dvdLayer->commonHeader.BCAFlag == 0 ? "No" : "Exist");

	for (WORD k = 0; k < sizeof(dvdLayer->MediaSpecific); k++) {
		OutputVolDescLogA("%02x", dvdLayer->MediaSpecific[k]);
	}

	DWORD dwSector = 0;
	if (dvdLayer->commonHeader.TrackPath) {
		dwSector = dwEndSectorLayer0 - dwStartSector + 1;
	}
	else {
		dwSector = dwEndSector - dwStartSector + 1;
	}
	if (!bSuccesssReadToc) {
		pDisc->SCSI.nAllLength += dwSector;
	}

	OutputVolDescLogA(
		"\n"
		"\t\t         L%u Sector: %8u (%#x)\n", uiNum, dwSector, dwSector);
}

VOID OutputDVDCopyrightDescriptor(
	PDVD_COPYRIGHT_DESCRIPTOR dvdCopyright
	)
{
	OutputVolDescLogA("\tCopyrightProtectionType: ");
	switch (dvdCopyright->CopyrightProtectionType) {
	case 0:
		OutputVolDescLogA("No\n");
		break;
	case 1:
		OutputVolDescLogA("CSS/CPPM\n");
		break;
	case 2:
		OutputVolDescLogA("CPRM\n");
		break;
	case 3:
		OutputVolDescLogA("AACS with HD DVD content\n");
		break;
	case 10:
		OutputVolDescLogA("AACS with BD content\n");
		break;
	default:
		OutputVolDescLogA("Unknown: %02x\n", dvdCopyright->CopyrightProtectionType);
		break;
	}
	OutputVolDescLogA(
		"\tRegionManagementInformation: %02x\n"
		, dvdCopyright->RegionManagementInformation);
}

VOID OutputDVDCommonInfo(
	LPBYTE lpFormat,
	WORD wFormatLength,
	LPCSTR lpStr
	)
{
	OutputVolDescLogA(lpStr);
	for (WORD k = 0; k < wFormatLength; k++) {
		OutputVolDescLogA("%02x", lpFormat[k]);
	}
	OutputVolDescLogA("\n");

}

VOID OutputDVDDiskKeyDescriptor(
	PDVD_DISK_KEY_DESCRIPTOR dvdDiskKey
	)
{
	OutputDVDCommonInfo(dvdDiskKey->DiskKeyData,
		sizeof(dvdDiskKey->DiskKeyData), "\tDiskKeyData: ");
}

VOID OutputDVDBCADescriptor(
	PDVD_BCA_DESCRIPTOR dvdBca,
	WORD wFormatLength
	)
{
	OutputDVDCommonInfo(
		dvdBca->BCAInformation, wFormatLength, "\tBCAInformation: ");
}

VOID OutputDVDManufacturerDescriptor(
	PDVD_MANUFACTURER_DESCRIPTOR dvdManufacturer
	)
{
	OutputDVDCommonInfo(dvdManufacturer->ManufacturingInformation,
		sizeof(dvdManufacturer->ManufacturingInformation), "\tManufacturingInformation: ");
}

VOID OutputDVDMediaId(
	LPBYTE lpFormat,
	WORD wFormatLength
	)
{
	OutputDVDCommonInfo(lpFormat, wFormatLength, "\tMedia ID: ");
}

VOID OutputDVDMediaKeyBlock(
	LPBYTE lpFormat,
	WORD wFormatLength
	)
{
	OutputVolDescLogA(
		"\tMedia Key Block Total Packs: %u"
		"\tmedia key block: ",
		lpFormat[3]);
	for (WORD k = 0; k < wFormatLength; k++) {
		OutputVolDescLogA("%02x", lpFormat[k]);
	}
	OutputVolDescLogA("\n");
}

VOID OutputDVDRamDds(
	LPBYTE lpFormat,
	WORD wFormatLength
	)
{
	OutputDVDCommonInfo(lpFormat, wFormatLength, "\tDVD-RAM DDS: ");
}

VOID OutputDVDRamMediumStatus(
	PDVD_RAM_MEDIUM_STATUS dvdRamMeium
	)
{
	OutputVolDescLogA(
		"\tDvdRamMediumStatus\n"
		"\t\t              PersistentWriteProtect: %s\n"
		"\t\t               CartridgeWriteProtect: %s\n"
		"\t\t           MediaSpecificWriteInhibit: %s\n"
		"\t\t                  CartridgeNotSealed: %s\n"
		"\t\t                    MediaInCartridge: %s\n"
		"\t\t              DiscTypeIdentification: %x\n"
		"\t\tMediaSpecificWriteInhibitInformation: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(dvdRamMeium->PersistentWriteProtect),
		BOOLEAN_TO_STRING_YES_NO_A(dvdRamMeium->CartridgeWriteProtect),
		BOOLEAN_TO_STRING_YES_NO_A(dvdRamMeium->MediaSpecificWriteInhibit),
		BOOLEAN_TO_STRING_YES_NO_A(dvdRamMeium->CartridgeNotSealed),
		BOOLEAN_TO_STRING_YES_NO_A(dvdRamMeium->MediaInCartridge),
		dvdRamMeium->DiscTypeIdentification,
		BOOLEAN_TO_STRING_YES_NO_A(dvdRamMeium->MediaSpecificWriteInhibitInformation));
}

VOID OutputDVDRamSpareArea(
	PDVD_RAM_SPARE_AREA_INFORMATION dvdRamSpare
	)
{
	OutputVolDescLogA(
		"\tDvdRamSpareAreaInformation\n"
		"\t\t          FreePrimarySpareSectors: %u\n"
		"\t\t     FreeSupplementalSpareSectors: %u\n"
		"\t\tAllocatedSupplementalSpareSectors: %u\n",
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

VOID OutputDVDRamRecordingType(
	PDVD_RAM_RECORDING_TYPE dvdRamRecording
	)
{
	OutputVolDescLogA(
		"\tDvdRamRecordingType\n"
		"\t\tRealTimeData: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(dvdRamRecording->RealTimeData));
}

VOID OutputDVDRmdLastBorderOut(
	LPBYTE lpFormat,
	WORD wFormatLength
	)
{
	OutputDVDCommonInfo(lpFormat, wFormatLength, "\tRMD in last border-out: ");
}

VOID OutputDVDRecordingManagementAreaData(
	PDVD_RECORDING_MANAGEMENT_AREA_DATA dvdRecordingMan,
	WORD wFormatLength
	)
{
	OutputVolDescLogA(
		"\tDVD_RECORDING_MANAGEMENT_AREA_DATA\n"
		"\t\tLastRecordedRMASectorNumber: %u\n"
		"\t\t                   RMDBytes: ",
		MAKELONG(MAKEWORD(dvdRecordingMan->LastRecordedRMASectorNumber[3],
		dvdRecordingMan->LastRecordedRMASectorNumber[2]),
		MAKEWORD(dvdRecordingMan->LastRecordedRMASectorNumber[1],
		dvdRecordingMan->LastRecordedRMASectorNumber[0])));
	for (WORD k = 0; k < wFormatLength; k++) {
		OutputVolDescLogA("%02x", dvdRecordingMan->RMDBytes[k]);
	}
	OutputVolDescLogA("\n");
}

VOID OutputDVDPreRecordedInformation(
	PDVD_PRERECORDED_INFORMATION dvdPreRecorded
	)
{
	OutputVolDescLogA(
		"\tDVD_PRERECORDED_INFORMATION\n"
		"\t\t                      FieldID_1: %u\n"
		"\t\t            DiscApplicatiowCode: %u\n"
		"\t\t               DiscPhysicalCode: %u\n"
		"\t\tLastAddressOfDataRecordableArea: %u\n"
		"\t\t                  ExtensiowCode: %u\n"
		"\t\t                    PartVers1on: %u\n"
		"\t\t                      FieldID_2: %u\n"
		"\t\t               OpcSuggestedCode: %u\n"
		"\t\t                 WavelengthCode: %u\n"
		"\t\t              WriteStrategyCode: %u\n"
		"\t\t                      FieldID_3: %u\n"
		"\t\t               ManufacturerId_3: %c%c%c%c%c%c\n"
		"\t\t                      FieldID_4: %u\n"
		"\t\t               ManufacturerId_4: %c%c%c%c%c%c\n"
		"\t\t                      FieldID_5: %u\n"
		"\t\t               ManufacturerId_5: %c%c%c%c%c%c\n",
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

VOID OutputDVDUniqueDiscIdentifer(
	PDVD_UNIQUE_DISC_IDENTIFIER dvdUnique
	)
{
	OutputVolDescLogA(
		"\tDVD_UNIQUE_DISC_IDENTIFIER\n"
		"\t\tRandomNumber: %u\n"
		"\t\t     YMD HMS: %04d-%02u-%02u %02u:%02u:%02u\n",
		MAKEWORD(dvdUnique->RandomNumber[1], dvdUnique->RandomNumber[0]),
		MAKELONG(MAKEWORD(dvdUnique->Year[3], dvdUnique->Year[2]),
		MAKEWORD(dvdUnique->Year[1], dvdUnique->Year[0])),
		MAKEWORD(dvdUnique->Month[1], dvdUnique->Month[0]),
		MAKEWORD(dvdUnique->Day[1], dvdUnique->Day[0]),
		MAKEWORD(dvdUnique->Hour[1], dvdUnique->Hour[0]),
		MAKEWORD(dvdUnique->Minute[1], dvdUnique->Minute[0]),
		MAKEWORD(dvdUnique->Second[1], dvdUnique->Second[0]));
}

VOID OutputDVDAdipInformation(
	LPBYTE lpFormat,
	WORD wFormatLength
	)
{
	OutputDVDCommonInfo(lpFormat, wFormatLength, "\tADIP information: ");
}

VOID OutputDVDDualLayerRecordingInformation(
	PDVD_DUAL_LAYER_RECORDING_INFORMATION dvdDualLayer
	)
{
	OutputVolDescLogA(
		"\tDvdDualLayerRecordingInformation\n"
		"\t\tLayer0SectorsImmutable: %s\n"
		"\t\t         Layer0Sectors: %u\n",
		BOOLEAN_TO_STRING_YES_NO_A(dvdDualLayer->Layer0SectorsImmutable),
		MAKELONG(MAKEWORD(dvdDualLayer->Layer0Sectors[3], dvdDualLayer->Layer0Sectors[2]),
		MAKEWORD(dvdDualLayer->Layer0Sectors[1], dvdDualLayer->Layer0Sectors[0])));
}
VOID OutputDVDDualLayerMiddleZone(
	PDVD_DUAL_LAYER_MIDDLE_ZONE_START_ADDRESS dvdDualLayerMiddle
	)
{
	OutputVolDescLogA(
		"\tDvdDualLayerMiddleZoneStartAddress\n"
		"\t\t                   InitStatus: %s\n"
		"\t\tShiftedMiddleAreaStartAddress: %u\n",
		BOOLEAN_TO_STRING_YES_NO_A(dvdDualLayerMiddle->InitStatus),
		MAKELONG(MAKEWORD(dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[3],
		dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[2]),
		MAKEWORD(dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[1],
		dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[0])));
}

VOID OutputDVDDualLayerJumpInterval(
	PDVD_DUAL_LAYER_JUMP_INTERVAL_SIZE dvdDualLayerJump
	)
{
	OutputVolDescLogA(
		"\tDvdDualLayerJumpIntervalSize\n"
		"\t\tJumpIntervalSize: %u\n",
		MAKELONG(MAKEWORD(dvdDualLayerJump->JumpIntervalSize[3],
		dvdDualLayerJump->JumpIntervalSize[2]),
		MAKEWORD(dvdDualLayerJump->JumpIntervalSize[1],
		dvdDualLayerJump->JumpIntervalSize[0])));
}

VOID OutputDVDDualLayerManualLayerJump(
	PDVD_DUAL_LAYER_MANUAL_LAYER_JUMP dvdDualLayerMan
	)
{
	OutputVolDescLogA(
		"\tDvdDualLayerManualLayerJump\n"
		"\t\tManualJumpLayerAddress: %u\n",
		MAKELONG(MAKEWORD(dvdDualLayerMan->ManualJumpLayerAddress[3],
		dvdDualLayerMan->ManualJumpLayerAddress[2]),
		MAKEWORD(dvdDualLayerMan->ManualJumpLayerAddress[1],
		dvdDualLayerMan->ManualJumpLayerAddress[0])));
}

VOID OutputDVDDualLayerRemapping(
	PDVD_DUAL_LAYER_REMAPPING_INFORMATION dvdDualLayerRemapping
	)
{
	OutputVolDescLogA(
		"\tDvdDualLayerRemappingInformation\n"
		"\t\tManualJumpLayerAddress: %u\n",
		MAKELONG(MAKEWORD(dvdDualLayerRemapping->RemappingAddress[3],
		dvdDualLayerRemapping->RemappingAddress[2]),
		MAKEWORD(dvdDualLayerRemapping->RemappingAddress[1],
		dvdDualLayerRemapping->RemappingAddress[0])));
}

VOID OutputDVDDiscControlBlockHeader(
	PDVD_DISC_CONTROL_BLOCK_HEADER dvdDiscCtrlBlk
	)
{
	OutputVolDescLogA(
		"\tDVD_DISC_CONTROL_BLOCK_HEADER\n"
		"\t\tContentDescriptor: %u\n"
		"\t\t           AsByte: %u\n"
		"\t\t         VendorId: ",
		MAKELONG(MAKEWORD(dvdDiscCtrlBlk->ContentDescriptor[3],
		dvdDiscCtrlBlk->ContentDescriptor[2]),
		MAKEWORD(dvdDiscCtrlBlk->ContentDescriptor[1],
		dvdDiscCtrlBlk->ContentDescriptor[0])),
		MAKELONG(MAKEWORD(dvdDiscCtrlBlk->ProhibitedActions.AsByte[3],
		dvdDiscCtrlBlk->ProhibitedActions.AsByte[2]),
		MAKEWORD(dvdDiscCtrlBlk->ProhibitedActions.AsByte[1],
		dvdDiscCtrlBlk->ProhibitedActions.AsByte[0])));
	for (WORD k = 0; k < sizeof(dvdDiscCtrlBlk->VendorId); k++) {
		OutputVolDescLogA("%c", dvdDiscCtrlBlk->VendorId[k]);
	}
	OutputVolDescLogA("\n");
}

VOID OutputDVDDiscControlBlockWriteInhibit(
	PDVD_DISC_CONTROL_BLOCK_WRITE_INHIBIT dvdDiscCtrlBlkWrite
	)
{
	OutputVolDescLogA(
		"\tDVD_DISC_CONTROL_BLOCK_WRITE_INHIBIT\n"
		"\t\t      UpdateCount: %u\n"
		"\t\t           AsByte: %u\n"
		"\t\t   UpdatePassword: ",
		MAKELONG(MAKEWORD(dvdDiscCtrlBlkWrite->UpdateCount[3], dvdDiscCtrlBlkWrite->UpdateCount[2]),
		MAKEWORD(dvdDiscCtrlBlkWrite->UpdateCount[1], dvdDiscCtrlBlkWrite->UpdateCount[0])),
		MAKELONG(MAKEWORD(dvdDiscCtrlBlkWrite->WriteProtectActions.AsByte[3],
		dvdDiscCtrlBlkWrite->WriteProtectActions.AsByte[2]),
		MAKEWORD(dvdDiscCtrlBlkWrite->WriteProtectActions.AsByte[1],
		dvdDiscCtrlBlkWrite->WriteProtectActions.AsByte[0])));
	for (WORD k = 0; k < sizeof(dvdDiscCtrlBlkWrite->UpdatePassword); k++) {
		OutputVolDescLogA("%c", dvdDiscCtrlBlkWrite->UpdatePassword[k]);
	}
	OutputVolDescLogA("\n");
}

VOID OutputDVDDiscControlBlockSession(
	PDVD_DISC_CONTROL_BLOCK_SESSION dvdDiscCtrlBlkSession
	)
{
	OutputVolDescLogA(
		"\tDVD_DISC_CONTROL_BLOCK_SESSION\n"
		"\t\tSessionNumber: %u\n"
		"\t\t       DiscID: \n",
		MAKEWORD(dvdDiscCtrlBlkSession->SessionNumber[1], dvdDiscCtrlBlkSession->SessionNumber[0]));
	for (WORD k = 0; k < sizeof(dvdDiscCtrlBlkSession->DiscID); k++) {
		OutputVolDescLogA("%c", dvdDiscCtrlBlkSession->DiscID[k]);
	}
	OutputVolDescLogA("\n");

	for (DWORD j = 0; j < sizeof(dvdDiscCtrlBlkSession->SessionItem); j++) {
		OutputVolDescLogA(
			"\t\t  SessionItem: %u\n"
			"\t\t\t     AsByte: ", j);
		for (WORD k = 0; k < sizeof(dvdDiscCtrlBlkSession->SessionItem[j].AsByte); k++) {
			OutputVolDescLogA("%c", dvdDiscCtrlBlkSession->SessionItem[j].AsByte[k]);
		}
		OutputVolDescLogA("\n");
	}
}

VOID OutputDVDDiscControlBlockList(
	PDVD_DISC_CONTROL_BLOCK_LIST dvdDiscCtrlBlkList,
	WORD wFormatLength
	)
{
	OutputVolDescLogA(
		"\tDVD_DISC_CONTROL_BLOCK_LIST\n"
		"\t\tReadabldDCBs: %s\n"
		"\t\tWritableDCBs: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(dvdDiscCtrlBlkList->ReadabldDCBs),
		BOOLEAN_TO_STRING_YES_NO_A(dvdDiscCtrlBlkList->WritableDCBs));
	OutputVolDescLogA(
		"\t\tDVD_DISC_CONTROL_BLOCK_LIST_DCB: ");
	for (WORD k = 0; k < wFormatLength - sizeof(DVD_DISC_CONTROL_BLOCK_LIST); k++) {
		OutputVolDescLogA("%u",
			MAKELONG(MAKEWORD(dvdDiscCtrlBlkList->Dcbs[k].DcbIdentifier[3], dvdDiscCtrlBlkList->Dcbs[k].DcbIdentifier[2]),
			MAKEWORD(dvdDiscCtrlBlkList->Dcbs[k].DcbIdentifier[1], dvdDiscCtrlBlkList->Dcbs[k].DcbIdentifier[0])));
	}
	OutputVolDescLogA("\n");

}

VOID OutputDVDMtaEccBlock(
	LPBYTE lpFormat,
	WORD wFormatLength
	)
{
	OutputDVDCommonInfo(lpFormat, wFormatLength, "\tMTA ECC Block: ");
}

VOID OutputDVDWriteProtectionStatus(
	PDVD_WRITE_PROTECTION_STATUS dvdWrite
	)
{
	OutputVolDescLogA(
		"\tDVD_WRITE_PROTECTION_STATUS\n"
		"\t\tSoftwareWriteProtectUntilPowerdown: %s\n"
		"\t\t       MediaPersistentWriteProtect: %s\n"
		"\t\t             CartridgeWriteProtect: %s\n"
		"\t\t         MediaSpecificWriteProtect: %s\n",
		BOOLEAN_TO_STRING_YES_NO_A(dvdWrite->SoftwareWriteProtectUntilPowerdown),
		BOOLEAN_TO_STRING_YES_NO_A(dvdWrite->MediaPersistentWriteProtect),
		BOOLEAN_TO_STRING_YES_NO_A(dvdWrite->CartridgeWriteProtect),
		BOOLEAN_TO_STRING_YES_NO_A(dvdWrite->MediaSpecificWriteProtect));
}

VOID OutputDVDAACSVolumeIdentifier(
	LPBYTE lpFormat,
	WORD wFormatLength
	)
{
	OutputDVDCommonInfo(
		lpFormat, wFormatLength, "\tAACS Volume Identifiers: ");
}

VOID OutputDVDPreRecordedAACSMediaSerialNumber(
	LPBYTE lpFormat,
	WORD wFormatLength
	)
{
	OutputDVDCommonInfo(
		lpFormat, wFormatLength, "\tPreRecorded AACS Media Serial Number: ");
}

VOID OutputDVDAACSMediaIdentifier(
	LPBYTE lpFormat,
	WORD wFormatLength
	)
{
	OutputDVDCommonInfo(lpFormat, wFormatLength, "\tAACS Media Identifier: ");
}

VOID OutputDVDAACSMediaKeyBlock(
	LPBYTE lpFormat,
	WORD wFormatLength
	)
{
	OutputDVDCommonInfo(lpFormat, wFormatLength, "\tAACS Media Key Block: ");
}

VOID OutputDVDListOfRecognizedFormatLayers(
	PDVD_LIST_OF_RECOGNIZED_FORMAT_LAYERS_TYPE_CODE dvdListOf
	)
{
	OutputVolDescLogA(
		"\t\tNumberOfRecognizedFormatLayers: %u\n"
		"\t\t             OnlineFormatlayer: %u\n"
		"\t\t            DefaultFormatLayer: %u\n",
		dvdListOf->NumberOfRecognizedFormatLayers,
		dvdListOf->OnlineFormatlayer,
		dvdListOf->DefaultFormatLayer);
}

VOID OutputDVDStructureFormat(
	PDISC pDisc,
	BYTE byFormatCode,
	WORD wFormatLength,
	LPBYTE lpFormat,
	LPBYTE lpLayerNum,
	UINT uiNum,
	BOOL bSuccesssReadToc
	)
{
	switch (byFormatCode) {
	case DvdPhysicalDescriptor:
	case 0x10:
		OutputDVDLayerDescriptor(pDisc,
			(PDVD_FULL_LAYER_DESCRIPTOR)lpFormat, lpLayerNum, uiNum, bSuccesssReadToc);
		break;
	case DvdCopyrightDescriptor:
		OutputDVDCopyrightDescriptor((PDVD_COPYRIGHT_DESCRIPTOR)lpFormat);
		break;
	case DvdDiskKeyDescriptor:
		OutputDVDDiskKeyDescriptor((PDVD_DISK_KEY_DESCRIPTOR)lpFormat);
		break;
	case DvdBCADescriptor:
		OutputDVDBCADescriptor((PDVD_BCA_DESCRIPTOR)lpFormat, wFormatLength);
		break;
	case DvdManufacturerDescriptor:
		OutputDVDManufacturerDescriptor((PDVD_MANUFACTURER_DESCRIPTOR)lpFormat);
		break;
	case 0x06:
		OutputDVDMediaId(lpFormat, wFormatLength);
		break;
	case 0x07:
		OutputDVDMediaKeyBlock(lpFormat, wFormatLength);
		break;
	case 0x08:
		OutputDVDRamDds(lpFormat, wFormatLength);
		break;
	case 0x09:
		OutputDVDRamMediumStatus((PDVD_RAM_MEDIUM_STATUS)lpFormat);
		break;
	case 0x0a:
		OutputDVDRamSpareArea((PDVD_RAM_SPARE_AREA_INFORMATION)lpFormat);
		break;
	case 0x0b:
		OutputDVDRamRecordingType((PDVD_RAM_RECORDING_TYPE)lpFormat);
		break;
	case 0x0c:
		OutputDVDRmdLastBorderOut(lpFormat, wFormatLength);
		break;
	case 0x0d:
		OutputDVDRecordingManagementAreaData((PDVD_RECORDING_MANAGEMENT_AREA_DATA)lpFormat, wFormatLength);
		break;
	case 0x0e:
		OutputDVDPreRecordedInformation((PDVD_PRERECORDED_INFORMATION)lpFormat);
		break;
	case 0x0f:
		OutputDVDUniqueDiscIdentifer((PDVD_UNIQUE_DISC_IDENTIFIER)lpFormat);
		break;
	case 0x11:
		OutputDVDAdipInformation(lpFormat, wFormatLength);
		break;
		// formats 0x12, 0x15 are is unstructured in public spec
		// formats 0x13, 0x14, 0x16 through 0x18 are not yet defined
		// formats 0x19, 0x1A are HD DVD-R
		// formats 0x1B through 0x1F are not yet defined
	case 0x20:
		OutputDVDDualLayerRecordingInformation((PDVD_DUAL_LAYER_RECORDING_INFORMATION)lpFormat);
		break;
	case 0x21:
		OutputDVDDualLayerMiddleZone((PDVD_DUAL_LAYER_MIDDLE_ZONE_START_ADDRESS)lpFormat);
		break;
	case 0x22:
		OutputDVDDualLayerJumpInterval((PDVD_DUAL_LAYER_JUMP_INTERVAL_SIZE)lpFormat);
		break;
	case 0x23:
		OutputDVDDualLayerManualLayerJump((PDVD_DUAL_LAYER_MANUAL_LAYER_JUMP)lpFormat);
		break;
	case 0x24:
		OutputDVDDualLayerRemapping((PDVD_DUAL_LAYER_REMAPPING_INFORMATION)lpFormat);
		break;
		// formats 0x25 through 0x2F are not yet defined
	case 0x30:
	{
		OutputDVDDiscControlBlockHeader((PDVD_DISC_CONTROL_BLOCK_HEADER)lpFormat);
		WORD len = wFormatLength;
		if (len == sizeof(DVD_DISC_CONTROL_BLOCK_WRITE_INHIBIT)) {
			OutputDVDDiscControlBlockWriteInhibit((PDVD_DISC_CONTROL_BLOCK_WRITE_INHIBIT)lpFormat);
		}
		else if (len == sizeof(DVD_DISC_CONTROL_BLOCK_SESSION)) {
			OutputDVDDiscControlBlockSession((PDVD_DISC_CONTROL_BLOCK_SESSION)lpFormat);
		}
		else if (len == sizeof(DVD_DISC_CONTROL_BLOCK_LIST)) {
			OutputDVDDiscControlBlockList((PDVD_DISC_CONTROL_BLOCK_LIST)lpFormat, wFormatLength);
		}
		break;
	}
	case 0x31:
		OutputDVDMtaEccBlock(lpFormat, wFormatLength);
		break;
		// formats 0x32 through 0xBF are not yet defined
	case 0xc0:
		OutputDVDWriteProtectionStatus((PDVD_WRITE_PROTECTION_STATUS)lpFormat);
		break;
		// formats 0xC1 through 0x7F are not yet defined
	case 0x80:
		OutputDVDAACSVolumeIdentifier(lpFormat, wFormatLength);
		break;
	case 0x81:
		OutputDVDPreRecordedAACSMediaSerialNumber(lpFormat, wFormatLength);
		break;
	case 0x82:
		OutputDVDAACSMediaIdentifier(lpFormat, wFormatLength);
		break;
	case 0x83:
		OutputDVDAACSMediaKeyBlock(lpFormat, wFormatLength);
		break;
		// formats 0x84 through 0x8F are not yet defined
	case 0x90:
		OutputDVDListOfRecognizedFormatLayers((PDVD_LIST_OF_RECOGNIZED_FORMAT_LAYERS_TYPE_CODE)lpFormat);
		break;
		// formats 0x91 through 0xFE are not yet defined
	default:
		OutputVolDescLogA("\tUnknown: %02x\n", byFormatCode);
		break;
	}
}

VOID OutputDVDCopyrightManagementInformation(
	PDVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR dvdCopyright,
	INT nLBA
	)
{
	OutputVolDescLogA("\t\tLBA %7u, ", nLBA);
	if ((dvdCopyright->CPR_MAI & 0x80) == 0x80) {
		OutputVolDescLogA("CPM exists");
		if ((dvdCopyright->CPR_MAI & 0x40) == 0x40) {
			switch (dvdCopyright->CPR_MAI & 0x0f) {
			case 0:
				OutputVolDescLogA(", the sector is scrambled by CSS");
				break;
			case 0x01:
				OutputVolDescLogA(", the sector is encrypted by CPPM");
				break;
			default:
				OutputVolDescLogA(", reserved");
			}
		}
		else {
			OutputVolDescLogA(", CSS or CPPM doesn't exist in this sector");
		}
		switch (dvdCopyright->CPR_MAI & 0x30) {
		case 0:
			OutputVolDescLogA(", copying is permitted without restriction\n");
			break;
		case 0x10:
			OutputVolDescLogA(", reserved\n");
			break;
		case 0x20:
			OutputVolDescLogA(", one generation of copies may be made\n");
			break;
		case 0x30:
			OutputVolDescLogA(", no copying is permitted\n");
			break;
		default:
			OutputVolDescLogA("\n");
		}
	}
	else {
		OutputVolDescLogA("CPM doesn't exist\n");
	}
}
