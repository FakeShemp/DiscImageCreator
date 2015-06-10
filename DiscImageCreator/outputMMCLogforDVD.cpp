/*
* This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
*/
#include "stdafx.h"
#include "struct.h"
#include "output.h"
#include "outputMMCLogforCD.h"

VOID OutputFsVolumeStructureDescriptorFormat(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	OutputDiscLogA(
		"Volume Recognition Sequence\n"
		"\t                               Structure Type: %u\n"
		"\t                          Standard Identifier: %.5s\n"
		"\t                            Structure Version: %u\n",
		lpBuf[nIdx],
		(PCHAR)&lpBuf[nIdx + 1],
		lpBuf[nIdx + 6]);
}

VOID OutputFsRecordingDateAndTime(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	OutputDiscLogA(
		"\tRecording Date and Time: %x %u-%02u-%02u %02u:%02u:%02u.%u.%u.%u\n",
		MAKEWORD(lpBuf[nIdx], lpBuf[nIdx + 1]), MAKEWORD(lpBuf[nIdx + 2], lpBuf[nIdx + 3]),
		lpBuf[nIdx + 4], lpBuf[nIdx + 5], lpBuf[nIdx + 6], lpBuf[nIdx + 7], lpBuf[nIdx + 8],
		lpBuf[nIdx + 9], lpBuf[nIdx + 10], lpBuf[nIdx + 11]);
}

VOID OutputFsBootDescriptor(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	OutputDiscLogA(
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
		lpBuf[nIdx + 8],
		(PCHAR)&lpBuf[nIdx + 9],
		(PCHAR)&lpBuf[nIdx + 24],
		lpBuf[nIdx + 40],
		(PCHAR)&lpBuf[nIdx + 41],
		(PCHAR)&lpBuf[nIdx + 64],
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
	OutputDiscLogA(
		"\t               Flags: %u\n"
		"\t            Boot Use: ",
		MAKEWORD(lpBuf[nIdx + 108], lpBuf[nIdx + 109]));
	for (INT i = 142; i <= 2047; i++) {
		OutputDiscLogA("%x", lpBuf[nIdx + i]);
	}
	OutputDiscLogA("\n");
}

VOID OutputFsVolumeRecognitionSequence(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	if (lpBuf[nIdx] == 1 && !strncmp((PCHAR)&lpBuf[nIdx + 1], "CD001", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nIdx);
		OutputFsPrimaryVolumeDescriptorForISO9660(lpBuf, nIdx);
	}
	else if (lpBuf[nIdx] == 2 && !strncmp((PCHAR)&lpBuf[nIdx + 1], "CD001", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nIdx);
		OutputDiscLogA("\t                                 Volume Flags: %u\n", lpBuf[nIdx + 7]);
		OutputFsPrimaryVolumeDescriptorForJoliet(lpBuf, nIdx);
	}
	else if (lpBuf[nIdx] == 255 && !strncmp((PCHAR)&lpBuf[nIdx + 1], "CD001", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nIdx);
	}
	else if (lpBuf[nIdx] == 0 && !strncmp((PCHAR)&lpBuf[nIdx + 1], "BOOT2", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nIdx);
		OutputFsBootDescriptor(lpBuf, nIdx);
	}
	else if (lpBuf[nIdx] == 0 && !strncmp((PCHAR)&lpBuf[nIdx + 1], "BEA01", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nIdx);
	}
	else if (lpBuf[nIdx] == 0 && !strncmp((PCHAR)&lpBuf[nIdx + 1], "NSR02", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nIdx);
	}
	else if (lpBuf[nIdx] == 0 && !strncmp((PCHAR)&lpBuf[nIdx + 1], "NSR03", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nIdx);
	}
	else if (lpBuf[nIdx] == 0 && !strncmp((PCHAR)&lpBuf[nIdx + 1], "TEA01", 5)) {
		OutputFsVolumeStructureDescriptorFormat(lpBuf, nIdx);
	}
}

VOID OutputFsCharspec(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	OutputDiscLogA(
		"\t\t       Character Set Type: %u\n"
		"\t\tCharacter Set Information: %.23s\n",
		lpBuf[nIdx], (PCHAR)&lpBuf[nIdx + 1]);
}

VOID OutputFsExtentDescriptor(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	OutputDiscLogA(
		"\t\t  Extent Length: %u\n"
		"\t\tExtent Location: %u\n",
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
	OutputDiscLogA(
		"\t\t            Flags: %u\n"
		"\t\t       Identifier: %.23s\n"
		"\t\tIdentifier Suffix: ",
		lpBuf[nIdx],
		(PCHAR)&lpBuf[nIdx + 1]);
	for (INT i = 24; i <= 31; i++) {
		OutputDiscLogA("%x", lpBuf[nIdx + i]);
	}
	OutputDiscLogA("\n");
}

VOID OutputFsPrimaryVolumeDescriptorForUDF(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	OutputDiscLogA(
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
		MAKELONG(MAKEWORD(lpBuf[nIdx + 16], lpBuf[nIdx + 17]),
		MAKEWORD(lpBuf[nIdx + 18], lpBuf[nIdx + 19])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 20], lpBuf[nIdx + 21]),
		MAKEWORD(lpBuf[nIdx + 22], lpBuf[nIdx + 23])),
		(PCHAR)&lpBuf[nIdx + 24],
		MAKEWORD(lpBuf[nIdx + 56], lpBuf[nIdx + 57]),
		MAKEWORD(lpBuf[nIdx + 58], lpBuf[nIdx + 59]),
		MAKEWORD(lpBuf[nIdx + 60], lpBuf[nIdx + 61]),
		MAKEWORD(lpBuf[nIdx + 62], lpBuf[nIdx + 63]),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 64], lpBuf[nIdx + 65]),
		MAKEWORD(lpBuf[nIdx + 66], lpBuf[nIdx + 67])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 68], lpBuf[nIdx + 69]),
		MAKEWORD(lpBuf[nIdx + 70], lpBuf[nIdx + 71])),
		(PCHAR)&lpBuf[nIdx + 72]);

	OutputFsCharspec(lpBuf, nIdx + 200);

	OutputDiscLogA("\tExplanatory Character Set\n");
	OutputFsCharspec(lpBuf, nIdx + 264);

	OutputDiscLogA("\tVolume Abstract\n");
	OutputFsExtentDescriptor(lpBuf, nIdx + 328);

	OutputDiscLogA("\tVolume Copyright Notice\n");
	OutputFsExtentDescriptor(lpBuf, nIdx + 336);

	OutputDiscLogA("\tApplication Identifier\n");
	OutputFsRegid(lpBuf, nIdx + 344);

	OutputFsRecordingDateAndTime(lpBuf, nIdx + 376);

	OutputDiscLogA("\tImplementation Identifier\n");
	OutputFsRegid(lpBuf, nIdx + 388);
	OutputDiscLogA("\tImplementation Use: ");
	for (INT i = 420; i <= 483; i++) {
		OutputDiscLogA("%x", lpBuf[nIdx + i]);
	}
	OutputDiscLogA("\n");

	OutputDiscLogA(
		"\tPredecessor Volume Descriptor Sequence Location: %u\n",
		MAKELONG(MAKEWORD(lpBuf[nIdx + 484], lpBuf[nIdx + 485]),
		MAKEWORD(lpBuf[nIdx + 486], lpBuf[nIdx + 487])));
	OutputDiscLogA(
		"\t                                          Flags: %u\n",
		MAKEWORD(lpBuf[nIdx + 488], lpBuf[nIdx + 489]));
}

VOID OutputFsAnchorVolumeDescriptorPointer(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	OutputDiscLogA("\tMain Volume Descriptor Sequence Extent\n");
	OutputFsExtentDescriptor(lpBuf, nIdx + 16);
	OutputDiscLogA("\tReserve Volume Descriptor Sequence Extent\n");
	OutputFsExtentDescriptor(lpBuf, nIdx + 24);
}

VOID OutputFsVolumeDescriptorPointer(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	OutputDiscLogA(
		"\t     Volume Descriptor Sequence Number: %u\n"
		"\tNext Volume Descriptor Sequence Extent\n",
		MAKELONG(MAKEWORD(lpBuf[nIdx + 16], lpBuf[nIdx + 17]),
		MAKEWORD(lpBuf[nIdx + 18], lpBuf[nIdx + 19])));
	OutputFsExtentDescriptor(lpBuf, nIdx + 20);
}

VOID OutputFsImplementationUseVolumeDescriptor(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	OutputDiscLogA(
		"\tVolume Descriptor Sequence Number: %u\n"
		"\tImplementation Identifier\n",
		MAKELONG(MAKEWORD(lpBuf[nIdx + 16], lpBuf[nIdx + 17]),
		MAKEWORD(lpBuf[nIdx + 18], lpBuf[nIdx + 19])));

	OutputFsRegid(lpBuf, nIdx + 20);

	OutputDiscLogA("\tLVI Charset\n");
	OutputFsCharspec(lpBuf, nIdx + 52);

	OutputDiscLogA("\tLogical Volume Identifier: %.128s\n", (PCHAR)&lpBuf[nIdx + 116]);
	OutputDiscLogA("\t               LV Info 1: %.36s\n", (PCHAR)&lpBuf[nIdx + 244]);
	OutputDiscLogA("\t               LV Info 2: %.36s\n", (PCHAR)&lpBuf[nIdx + 280]);
	OutputDiscLogA("\t               LV Info 3: %.36s\n", (PCHAR)&lpBuf[nIdx + 316]);
	OutputDiscLogA("\tImplemention ID\n");
	OutputFsRegid(lpBuf, nIdx + 352);
	OutputDiscLogA("\tImplementation Use: ");
	for (INT i = 384; i <= 511; i++) {
		OutputDiscLogA("%x", lpBuf[nIdx + i]);
	}
	OutputDiscLogA("\n");
}

VOID OutputFsPartitionDescriptor(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	OutputDiscLogA(
		"\tVolume Descriptor Sequence Number: %u\n"
		"\t                  Partition Flags: %u\n"
		"\t                 Partition Number: %u\n"
		"\tPartition Contents\n",
		MAKELONG(MAKEWORD(lpBuf[nIdx + 16], lpBuf[nIdx + 17]),
		MAKEWORD(lpBuf[nIdx + 18], lpBuf[nIdx + 19])),
		MAKEWORD(lpBuf[nIdx + 20], lpBuf[nIdx + 21]),
		MAKEWORD(lpBuf[nIdx + 22], lpBuf[nIdx + 23]));

	OutputFsRegid(lpBuf, nIdx + 24);

	OutputDiscLogA("\tPartition Contents Use: ");
	for (INT i = 56; i <= 183; i++) {
		OutputDiscLogA("%x", lpBuf[nIdx + i]);
	}
	OutputDiscLogA("\n");
	OutputDiscLogA(
		"\t                Access Type: %u\n"
		"\tPartition Starting Location: %u\n"
		"\t           Partition Length: %u\n"
		"\tImplementation Identifier\n",
		MAKELONG(MAKEWORD(lpBuf[nIdx + 184], lpBuf[nIdx + 185]),
		MAKEWORD(lpBuf[nIdx + 186], lpBuf[nIdx + 187])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 188], lpBuf[nIdx + 189]),
		MAKEWORD(lpBuf[nIdx + 190], lpBuf[nIdx + 191])),
		MAKELONG(MAKEWORD(lpBuf[nIdx + 192], lpBuf[nIdx + 193]),
		MAKEWORD(lpBuf[nIdx + 194], lpBuf[nIdx + 195])));

	OutputFsRegid(lpBuf, nIdx + 196);
	OutputDiscLogA("\tImplementation Use: ");
	for (INT i = 228; i <= 355; i++) {
		OutputDiscLogA("%x", lpBuf[nIdx + i]);
	}
	OutputDiscLogA("\n");
}

VOID OutputFsLongAllocationDescriptor(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	OutputDiscLogA(
		"\tLongAllocationDescriptor\n"
		"\t\t             Extent Length: %u\n"
		"\t\t      Logical Block Number: %u\n"
		"\t\tPartition Reference Number: %u\n",
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
	OutputDiscLogA(
		"\tVolume Descriptor Sequence Number: %u\n"
		"\tDescriptor Character Set\n",
		MAKELONG(MAKEWORD(lpBuf[nIdx + 16], lpBuf[nIdx + 17]),
		MAKEWORD(lpBuf[nIdx + 18], lpBuf[nIdx + 19])));

	OutputFsCharspec(lpBuf, nIdx + 20);

	OutputDiscLogA(
		"\tLogical Volume Identifier: %.128s\n"
		"\t      Logical Block Size : %u\n"
		"\tDomain Identifier\n",
		(PCHAR)&lpBuf[nIdx + 84],
		MAKELONG(MAKEWORD(lpBuf[nIdx + 212], lpBuf[nIdx + 213]),
		MAKEWORD(lpBuf[nIdx + 214], lpBuf[nIdx + 215])));

	OutputFsCharspec(lpBuf, nIdx + 216);
	OutputFsLongAllocationDescriptor(lpBuf, nIdx + 248);

	LONG MT_L = MAKELONG(MAKEWORD(lpBuf[nIdx + 264], lpBuf[nIdx + 265]),
		MAKEWORD(lpBuf[nIdx + 266], lpBuf[nIdx + 267]));
	OutputDiscLogA(
		"\t        Map Table Length: %u\n"
		"\tNumber of Partition Maps: %u\n"
		"\tImplementation Identifier\n",
		MT_L,
		MAKELONG(MAKEWORD(lpBuf[nIdx + 268], lpBuf[nIdx + 269]),
		MAKEWORD(lpBuf[nIdx + 270], lpBuf[nIdx + 271])));

	OutputFsRegid(lpBuf, nIdx + 272);

	OutputDiscLogA("\tImplementation Use: ");
	for (INT i = 304; i <= 431; i++) {
		OutputDiscLogA("%x", lpBuf[nIdx + i]);
	}
	OutputDiscLogA("\n");
	OutputDiscLogA("\tIntegrity Sequence Extent\n");
	OutputFsExtentDescriptor(lpBuf, nIdx + 432);

	OutputDiscLogA("\tPartition Maps: ");
	for (INT i = 0; i < MT_L; i++) {
		OutputDiscLogA("%x", lpBuf[nIdx + 440 + i]);
	}
	OutputDiscLogA("\n");
}

VOID OutputFsUnallocatedSpaceDescriptor(
	LPBYTE lpBuf,
	INT nIdx
	)
{
	LONG N_AD = MAKELONG(MAKEWORD(lpBuf[nIdx + 20], lpBuf[nIdx + 21]),
		MAKEWORD(lpBuf[nIdx + 22], lpBuf[nIdx + 23]));
	OutputDiscLogA(
		"\tVolume Descriptor Sequence Number: %u\n"
		"\t Number of Allocation Descriptors: %u\n"
		"\tAllocation Descriptors\n",
		MAKELONG(MAKEWORD(lpBuf[nIdx + 16], lpBuf[nIdx + 17]),
		MAKEWORD(lpBuf[nIdx + 18], lpBuf[nIdx + 19])),
		N_AD);
	for (INT i = 0; i < N_AD * 8; i += 8) {
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
		OutputDiscLogA("Primary Volume Descriptor\n");
		break;
	case 2:
		OutputDiscLogA("Anchor Volume Descriptor Pointer\n");
		break;
	case 3:
		OutputDiscLogA("Volume Descriptor Pointer\n");
		break;
	case 4:
		OutputDiscLogA("Implementation Use Volume Descriptor\n");
		break;
	case 5:
		OutputDiscLogA("Partition Descriptor\n");
		break;
	case 6:
		OutputDiscLogA("Logical Volume Descriptor\n");
		break;
	case 7:
		OutputDiscLogA("Unallocated Space Descriptor\n");
		break;
	case 8:
		OutputDiscLogA("Terminating Descriptor\n");
		break;
	case 9:
		OutputDiscLogA("Logical Volume Integrity Descriptor\n");
		break;
	case 256:
		OutputDiscLogA("File Set Descriptor\n");
		break;
	case 257:
		OutputDiscLogA("File Identifier Descriptor\n");
		break;
	case 258:
		OutputDiscLogA("Allocation Extent Descriptor\n");
		break;
	case 259:
		OutputDiscLogA("Indirect Entry\n");
		break;
	case 260:
		OutputDiscLogA("Terminal Entry\n");
		break;
	case 261:
		OutputDiscLogA("File Entry\n");
		break;
	case 262:
		OutputDiscLogA("Extended Attribute Header Descriptor\n");
		break;
	case 263:
		OutputDiscLogA("Unallocated Space Entry\n");
		break;
	case 264:
		OutputDiscLogA("Space Bitmap Descriptor\n");
		break;
	case 265:
		OutputDiscLogA("Partition Integrity Entry\n");
		break;
	case 266:
		OutputDiscLogA("Extended File Entry\n");
		break;
	}

	OutputDiscLogA(
		"\t\t           Descriptor Version: %u\n"
		"\t\t                 Tag Checksum: %u\n"
		"\t\t            Tag Serial Number: %u\n"
		"\t\t               Descriptor CRC: %x\n"
		"\t\t        Descriptor CRC Length: %u\n"
		"\t\t                 Tag Location: %u\n",
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

VOID OutputMmcDVDLayerDescriptor(
	PDISC_DATA pDiscData,
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

	OutputDiscLogA(
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
		dvdLayer->commonHeader.TrackPath == 0 ? "Paallel Track Path" : "Opposite Track Path",
		dvdLayer->commonHeader.NumberOfLayers == 0 ? "Single Layer" : "Double Layer",
		lpTrackDensity[dvdLayer->commonHeader.TrackDensity],
		lpLinearDensity[dvdLayer->commonHeader.LinearDensity],
		dwStartSector, dwStartSector,
		dwEndSector, dwEndSector,
		dwEndSectorLayer0, dwEndSectorLayer0,
		dvdLayer->commonHeader.BCAFlag == 0 ? "None" : "Exist");

	for (WORD k = 0; k < sizeof(dvdLayer->MediaSpecific); k++) {
		OutputDiscLogA("%02x", dvdLayer->MediaSpecific[k]);
	}

	DWORD dwSector = 0;
	if (dvdLayer->commonHeader.TrackPath) {
		dwSector = dwEndSectorLayer0 - dwStartSector + 1;
	}
	else {
		dwSector = dwEndSector - dwStartSector + 1;
	}
	if (!bSuccesssReadToc) {
		pDiscData->SCSI.nAllLength += dwSector;
	}
	OutputDiscLogA("\n");
	OutputDiscLogA(
		"\t\t         L%u Sector: %8u (%#x)\n", uiNum, dwSector, dwSector);
}

VOID OutputMmcDVDCopyrightDescriptor(
	PDVD_COPYRIGHT_DESCRIPTOR dvdCopyright
	)
{
	OutputDiscLogA("\tCopyrightProtectionType: ");
	switch (dvdCopyright->CopyrightProtectionType) {
	case 0:
		OutputDiscLogA("No\n");
		break;
	case 1:
		OutputDiscLogA("CSS/CPPM\n");
		break;
	case 2:
		OutputDiscLogA("CPRM\n");
		break;
	case 3:
		OutputDiscLogA("AACS with HD DVD content\n");
		break;
	case 10:
		OutputDiscLogA("AACS with BD content\n");
		break;
	default:
		OutputDiscLogA("Unknown: %02x\n", dvdCopyright->CopyrightProtectionType);
		break;
	}
	OutputDiscLogA(
		"\tRegionManagementInformation: %02x\n", dvdCopyright->RegionManagementInformation);
}

VOID OutputMmcDVDCommonInfo(
	LPBYTE lpFormat,
	WORD wFormatLength,
	LPCSTR lpStr
	)
{
	OutputDiscLogA(lpStr);
	for (WORD k = 0; k < wFormatLength; k++) {
		OutputDiscLogA("%02x", lpFormat[k]);
	}
	OutputDiscLogA("\n");

}

VOID OutputMmcDVDDiskKeyDescriptor(
	PDVD_DISK_KEY_DESCRIPTOR dvdDiskKey
	)
{
	OutputMmcDVDCommonInfo(dvdDiskKey->DiskKeyData, sizeof(dvdDiskKey->DiskKeyData), "\tDiskKeyData: ");
}

VOID OutputMmcDVDBCADescriptor(
	PDVD_BCA_DESCRIPTOR dvdBca,
	WORD wFormatLength
	)
{
	OutputMmcDVDCommonInfo(dvdBca->BCAInformation, wFormatLength, "\tBCAInformation: ");
}

VOID OutputMmcDVDManufacturerDescriptor(
	PDVD_MANUFACTURER_DESCRIPTOR dvdManufacturer
	)
{
	OutputMmcDVDCommonInfo(dvdManufacturer->ManufacturingInformation,
		sizeof(dvdManufacturer->ManufacturingInformation), "\tManufacturingInformation: ");
}

VOID OutputMmcDVDMediaId(
	LPBYTE lpFormat,
	WORD wFormatLength
	)
{
	OutputMmcDVDCommonInfo(lpFormat, wFormatLength, "\tMedia ID: ");
}

VOID OutputMmcDVDMediaKeyBlock(
	LPBYTE lpFormat,
	WORD wFormatLength
	)
{
	OutputDiscLogA(
		"\tMedia Key Block Total Packs: %u"
		"\tmedia key block: ",
		lpFormat[3]);
	for (WORD k = 0; k < wFormatLength; k++) {
		OutputDiscLogA("%02x", lpFormat[k]);
	}
	OutputDiscLogA("\n");
}

VOID OutputMmcDVDRamDds(
	LPBYTE lpFormat,
	WORD wFormatLength
	)
{
	OutputMmcDVDCommonInfo(lpFormat, wFormatLength, "\tDVD-RAM DDS: ");
}

VOID OutputMmcDVDRamMediumStatus(
	PDVD_RAM_MEDIUM_STATUS dvdRamMeium
	)
{
	OutputDiscLogA(
		"\tDvdRamMediumStatus\n"
		"\t\t              PersistentWriteProtect: %s\n"
		"\t\t               CartridgeWriteProtect: %s\n"
		"\t\t           MediaSpecificWriteInhibit: %s\n"
		"\t\t                  CartridgeNotSealed: %s\n"
		"\t\t                    MediaInCartridge: %s\n"
		"\t\t              DiscTypeIdentification: %x\n"
		"\t\tMediaSpecificWriteInhibitInformation: %s\n",
		BOOLEAN_TO_STRING_YES_NO(dvdRamMeium->PersistentWriteProtect),
		BOOLEAN_TO_STRING_YES_NO(dvdRamMeium->CartridgeWriteProtect),
		BOOLEAN_TO_STRING_YES_NO(dvdRamMeium->MediaSpecificWriteInhibit),
		BOOLEAN_TO_STRING_YES_NO(dvdRamMeium->CartridgeNotSealed),
		BOOLEAN_TO_STRING_YES_NO(dvdRamMeium->MediaInCartridge),
		dvdRamMeium->DiscTypeIdentification,
		BOOLEAN_TO_STRING_YES_NO(dvdRamMeium->MediaSpecificWriteInhibitInformation));
}

VOID OutputMmcDVDRamSpareArea(
	PDVD_RAM_SPARE_AREA_INFORMATION dvdRamSpare
	)
{
	OutputDiscLogA(
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

VOID OutputMmcDVDRamRecordingType(
	PDVD_RAM_RECORDING_TYPE dvdRamRecording
	)
{
	OutputDiscLogA(
		"\tDvdRamRecordingType\n"
		"\t\tRealTimeData: %s\n",
		BOOLEAN_TO_STRING_YES_NO(dvdRamRecording->RealTimeData));
}

VOID OutputMmcDVDRmdLastBorderOut(
	LPBYTE lpFormat,
	WORD wFormatLength
	)
{
	OutputMmcDVDCommonInfo(lpFormat, wFormatLength, "\tRMD in last border-out: ");
}

VOID OutputMmcDVDRecordingManagementAreaData(
	PDVD_RECORDING_MANAGEMENT_AREA_DATA dvdRecordingMan,
	WORD wFormatLength
	)
{
	OutputDiscLogA(
		"\tDVD_RECORDING_MANAGEMENT_AREA_DATA\n"
		"\t\tLastRecordedRMASectorNumber: %u\n"
		"\t\t                   RMDBytes: ",
		MAKELONG(MAKEWORD(dvdRecordingMan->LastRecordedRMASectorNumber[3],
		dvdRecordingMan->LastRecordedRMASectorNumber[2]),
		MAKEWORD(dvdRecordingMan->LastRecordedRMASectorNumber[1],
		dvdRecordingMan->LastRecordedRMASectorNumber[0])));
	for (WORD k = 0; k < wFormatLength; k++) {
		OutputDiscLogA("%02x", dvdRecordingMan->RMDBytes[k]);
	}
	OutputDiscLogA("\n");
}

VOID OutputMmcDVDPreRecordedInformation(
	PDVD_PRERECORDED_INFORMATION dvdPreRecorded
	)
{
	OutputDiscLogA(
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

VOID OutputMmcDVDUniqueDiscIdentifer(
	PDVD_UNIQUE_DISC_IDENTIFIER dvdUnique
	)
{
	OutputDiscLogA(
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

VOID OutputMmcDVDAdipInformation(
	LPBYTE lpFormat,
	WORD wFormatLength
	)
{
	OutputMmcDVDCommonInfo(lpFormat, wFormatLength, "\tADIP information: ");
}

VOID OutputMmcDVDDualLayerRecordingInformation(
	PDVD_DUAL_LAYER_RECORDING_INFORMATION dvdDualLayer
	)
{
	OutputDiscLogA(
		"\tDvdDualLayerRecordingInformation\n"
		"\t\tLayer0SectorsImmutable: %s\n"
		"\t\t         Layer0Sectors: %u\n",
		BOOLEAN_TO_STRING_YES_NO(dvdDualLayer->Layer0SectorsImmutable),
		MAKELONG(MAKEWORD(dvdDualLayer->Layer0Sectors[3], dvdDualLayer->Layer0Sectors[2]),
		MAKEWORD(dvdDualLayer->Layer0Sectors[1], dvdDualLayer->Layer0Sectors[0])));
}
VOID OutputMmcDVDDualLayerMiddleZone(
	PDVD_DUAL_LAYER_MIDDLE_ZONE_START_ADDRESS dvdDualLayerMiddle
	)
{
	OutputDiscLogA(
		"\tDvdDualLayerMiddleZoneStartAddress\n"
		"\t\t                   InitStatus: %s\n"
		"\t\tShiftedMiddleAreaStartAddress: %u\n",
		BOOLEAN_TO_STRING_YES_NO(dvdDualLayerMiddle->InitStatus),
		MAKELONG(MAKEWORD(dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[3],
		dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[2]),
		MAKEWORD(dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[1],
		dvdDualLayerMiddle->ShiftedMiddleAreaStartAddress[0])));
}

VOID OutputMmcDVDDualLayerJumpInterval(
	PDVD_DUAL_LAYER_JUMP_INTERVAL_SIZE dvdDualLayerJump
	)
{
	OutputDiscLogA(
		"\tDvdDualLayerJumpIntervalSize\n"
		"\t\tJumpIntervalSize: %u\n",
		MAKELONG(MAKEWORD(dvdDualLayerJump->JumpIntervalSize[3],
		dvdDualLayerJump->JumpIntervalSize[2]),
		MAKEWORD(dvdDualLayerJump->JumpIntervalSize[1],
		dvdDualLayerJump->JumpIntervalSize[0])));
}

VOID OutputMmcDVDDualLayerManualLayerJump(
	PDVD_DUAL_LAYER_MANUAL_LAYER_JUMP dvdDualLayerMan
	)
{
	OutputDiscLogA(
		"\tDvdDualLayerManualLayerJump\n"
		"\t\tManualJumpLayerAddress: %u\n",
		MAKELONG(MAKEWORD(dvdDualLayerMan->ManualJumpLayerAddress[3],
		dvdDualLayerMan->ManualJumpLayerAddress[2]),
		MAKEWORD(dvdDualLayerMan->ManualJumpLayerAddress[1],
		dvdDualLayerMan->ManualJumpLayerAddress[0])));
}

VOID OutputMmcDVDDualLayerRemapping(
	PDVD_DUAL_LAYER_REMAPPING_INFORMATION dvdDualLayerRemapping
	)
{
	OutputDiscLogA(
		"\tDvdDualLayerRemappingInformation\n"
		"\t\tManualJumpLayerAddress: %u\n",
		MAKELONG(MAKEWORD(dvdDualLayerRemapping->RemappingAddress[3],
		dvdDualLayerRemapping->RemappingAddress[2]),
		MAKEWORD(dvdDualLayerRemapping->RemappingAddress[1],
		dvdDualLayerRemapping->RemappingAddress[0])));
}

VOID OutputMmcDVDDiscControlBlockHeader(
	PDVD_DISC_CONTROL_BLOCK_HEADER dvdDiscCtrlBlk
	)
{
	OutputDiscLogA(
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
		OutputDiscLogA("%c", dvdDiscCtrlBlk->VendorId[k]);
	}
	OutputDiscLogA("\n");
}

VOID OutputMmcDVDDiscControlBlockWriteInhibit(
	PDVD_DISC_CONTROL_BLOCK_WRITE_INHIBIT dvdDiscCtrlBlkWrite
	)
{
	OutputDiscLogA(
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
		OutputDiscLogA("%c", dvdDiscCtrlBlkWrite->UpdatePassword[k]);
	}
	OutputDiscLogA("\n");
}

VOID OutputMmcDVDDiscControlBlockSession(
	PDVD_DISC_CONTROL_BLOCK_SESSION dvdDiscCtrlBlkSession
	)
{
	OutputDiscLogA(
		"\tDVD_DISC_CONTROL_BLOCK_SESSION\n"
		"\t\tSessionNumber: %u\n"
		"\t\t       DiscID: \n",
		MAKEWORD(dvdDiscCtrlBlkSession->SessionNumber[1], dvdDiscCtrlBlkSession->SessionNumber[0]));
	for (WORD k = 0; k < sizeof(dvdDiscCtrlBlkSession->DiscID); k++) {
		OutputDiscLogA("%c", dvdDiscCtrlBlkSession->DiscID[k]);
	}
	OutputDiscLogA("\n");

	for (DWORD j = 0; j < sizeof(dvdDiscCtrlBlkSession->SessionItem); j++) {
		OutputDiscLogA(
			"\t\t  SessionItem: %u\n"
			"\t\t\t     AsByte: ", j);
		for (WORD k = 0; k < sizeof(dvdDiscCtrlBlkSession->SessionItem[j].AsByte); k++) {
			OutputDiscLogA("%c", dvdDiscCtrlBlkSession->SessionItem[j].AsByte[k]);
		}
		OutputDiscLogA("\n");
	}
}

VOID OutputMmcDVDDiscControlBlockList(
	PDVD_DISC_CONTROL_BLOCK_LIST dvdDiscCtrlBlkList,
	WORD wFormatLength
	)
{
	OutputDiscLogA(
		"\tDVD_DISC_CONTROL_BLOCK_LIST\n"
		"\t\tReadabldDCBs: %s\n"
		"\t\tWritableDCBs: %s\n",
		BOOLEAN_TO_STRING_YES_NO(dvdDiscCtrlBlkList->ReadabldDCBs),
		BOOLEAN_TO_STRING_YES_NO(dvdDiscCtrlBlkList->WritableDCBs));
	OutputDiscLogA(
		"\t\tDVD_DISC_CONTROL_BLOCK_LIST_DCB: ");
	for (WORD k = 0; k < wFormatLength - sizeof(DVD_DISC_CONTROL_BLOCK_LIST); k++) {
		OutputDiscLogA("%u",
			MAKELONG(MAKEWORD(dvdDiscCtrlBlkList->Dcbs[k].DcbIdentifier[3], dvdDiscCtrlBlkList->Dcbs[k].DcbIdentifier[2]),
			MAKEWORD(dvdDiscCtrlBlkList->Dcbs[k].DcbIdentifier[1], dvdDiscCtrlBlkList->Dcbs[k].DcbIdentifier[0])));
	}
	OutputDiscLogA("\n");

}

VOID OutputMmcDVDMtaEccBlock(
	LPBYTE lpFormat,
	WORD wFormatLength
	)
{
	OutputMmcDVDCommonInfo(lpFormat, wFormatLength, "\tMTA ECC Block: ");
}

VOID OutputMmcDVDWriteProtectionStatus(
	PDVD_WRITE_PROTECTION_STATUS dvdWrite
	)
{
	OutputDiscLogA(
		"\tDVD_WRITE_PROTECTION_STATUS\n"
		"\t\tSoftwareWriteProtectUntilPowerdown: %s\n"
		"\t\t       MediaPersistentWriteProtect: %s\n"
		"\t\t             CartridgeWriteProtect: %s\n"
		"\t\t         MediaSpecificWriteProtect: %s\n",
		BOOLEAN_TO_STRING_YES_NO(dvdWrite->SoftwareWriteProtectUntilPowerdown),
		BOOLEAN_TO_STRING_YES_NO(dvdWrite->MediaPersistentWriteProtect),
		BOOLEAN_TO_STRING_YES_NO(dvdWrite->CartridgeWriteProtect),
		BOOLEAN_TO_STRING_YES_NO(dvdWrite->MediaSpecificWriteProtect));
}

VOID OutputMmcDVDAACSVolumeIdentifier(
	LPBYTE lpFormat,
	WORD wFormatLength
	)
{
	OutputMmcDVDCommonInfo(lpFormat, wFormatLength, "\tAACS Volume Identifiers: ");
}

VOID OutputMmcDVDPreRecordedAACSMediaSerialNumber(
	LPBYTE lpFormat,
	WORD wFormatLength
	)
{
	OutputMmcDVDCommonInfo(lpFormat, wFormatLength, "\tPreRecorded AACS Media Serial Number: ");
}

VOID OutputMmcDVDAACSMediaIdentifier(
	LPBYTE lpFormat,
	WORD wFormatLength
	)
{
	OutputMmcDVDCommonInfo(lpFormat, wFormatLength, "\tAACS Media Identifier: ");
}

VOID OutputMmcDVDAACSMediaKeyBlock(
	LPBYTE lpFormat,
	WORD wFormatLength
	)
{
	OutputMmcDVDCommonInfo(lpFormat, wFormatLength, "\tAACS Media Key Block: ");
}

VOID OutputMmcDVDListOfRecognizedFormatLayers(
	PDVD_LIST_OF_RECOGNIZED_FORMAT_LAYERS_TYPE_CODE dvdListOf
	)
{
	OutputDiscLogA(
		"\t\tNumberOfRecognizedFormatLayers: %u\n"
		"\t\t             OnlineFormatlayer: %u\n"
		"\t\t            DefaultFormatLayer: %u\n",
		dvdListOf->NumberOfRecognizedFormatLayers,
		dvdListOf->OnlineFormatlayer,
		dvdListOf->DefaultFormatLayer);
}

VOID OutputMmcDVDStructureFormat(
	PDISC_DATA pDiscData,
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
		OutputMmcDVDLayerDescriptor(pDiscData,
			(PDVD_FULL_LAYER_DESCRIPTOR)lpFormat, lpLayerNum, uiNum, bSuccesssReadToc);
		break;
	case DvdCopyrightDescriptor:
		OutputMmcDVDCopyrightDescriptor((PDVD_COPYRIGHT_DESCRIPTOR)lpFormat);
		break;
	case DvdDiskKeyDescriptor:
		OutputMmcDVDDiskKeyDescriptor((PDVD_DISK_KEY_DESCRIPTOR)lpFormat);
		break;
	case DvdBCADescriptor:
		OutputMmcDVDBCADescriptor((PDVD_BCA_DESCRIPTOR)lpFormat, wFormatLength);
		break;
	case DvdManufacturerDescriptor:
		OutputMmcDVDManufacturerDescriptor((PDVD_MANUFACTURER_DESCRIPTOR)lpFormat);
		break;
	case 0x06:
		OutputMmcDVDMediaId(lpFormat, wFormatLength);
		break;
	case 0x07:
		OutputMmcDVDMediaKeyBlock(lpFormat, wFormatLength);
		break;
	case 0x08:
		OutputMmcDVDRamDds(lpFormat, wFormatLength);
		break;
	case 0x09:
		OutputMmcDVDRamMediumStatus((PDVD_RAM_MEDIUM_STATUS)lpFormat);
		break;
	case 0x0a:
		OutputMmcDVDRamSpareArea((PDVD_RAM_SPARE_AREA_INFORMATION)lpFormat);
		break;
	case 0x0b:
		OutputMmcDVDRamRecordingType((PDVD_RAM_RECORDING_TYPE)lpFormat);
		break;
	case 0x0c:
		OutputMmcDVDRmdLastBorderOut(lpFormat, wFormatLength);
		break;
	case 0x0d:
		OutputMmcDVDRecordingManagementAreaData((PDVD_RECORDING_MANAGEMENT_AREA_DATA)lpFormat, wFormatLength);
		break;
	case 0x0e:
		OutputMmcDVDPreRecordedInformation((PDVD_PRERECORDED_INFORMATION)lpFormat);
		break;
	case 0x0f:
		OutputMmcDVDUniqueDiscIdentifer((PDVD_UNIQUE_DISC_IDENTIFIER)lpFormat);
		break;
	case 0x11:
		OutputMmcDVDAdipInformation(lpFormat, wFormatLength);
		break;
		// formats 0x12, 0x15 are is unstructured in public spec
		// formats 0x13, 0x14, 0x16 through 0x18 are not yet defined
		// formats 0x19, 0x1A are HD DVD-R
		// formats 0x1B through 0x1F are not yet defined
	case 0x20:
		OutputMmcDVDDualLayerRecordingInformation((PDVD_DUAL_LAYER_RECORDING_INFORMATION)lpFormat);
		break;
	case 0x21:
		OutputMmcDVDDualLayerMiddleZone((PDVD_DUAL_LAYER_MIDDLE_ZONE_START_ADDRESS)lpFormat);
		break;
	case 0x22:
		OutputMmcDVDDualLayerJumpInterval((PDVD_DUAL_LAYER_JUMP_INTERVAL_SIZE)lpFormat);
		break;
	case 0x23:
		OutputMmcDVDDualLayerManualLayerJump((PDVD_DUAL_LAYER_MANUAL_LAYER_JUMP)lpFormat);
		break;
	case 0x24:
		OutputMmcDVDDualLayerRemapping((PDVD_DUAL_LAYER_REMAPPING_INFORMATION)lpFormat);
		break;
		// formats 0x25 through 0x2F are not yet defined
	case 0x30:
	{
		OutputMmcDVDDiscControlBlockHeader((PDVD_DISC_CONTROL_BLOCK_HEADER)lpFormat);
		WORD len = wFormatLength;
		if (len == sizeof(DVD_DISC_CONTROL_BLOCK_WRITE_INHIBIT)) {
			OutputMmcDVDDiscControlBlockWriteInhibit((PDVD_DISC_CONTROL_BLOCK_WRITE_INHIBIT)lpFormat);
		}
		else if (len == sizeof(DVD_DISC_CONTROL_BLOCK_SESSION)) {
			OutputMmcDVDDiscControlBlockSession((PDVD_DISC_CONTROL_BLOCK_SESSION)lpFormat);
		}
		else if (len == sizeof(DVD_DISC_CONTROL_BLOCK_LIST)) {
			OutputMmcDVDDiscControlBlockList((PDVD_DISC_CONTROL_BLOCK_LIST)lpFormat, wFormatLength);
		}
		break;
	}
	case 0x31:
		OutputMmcDVDMtaEccBlock(lpFormat, wFormatLength);
		break;
		// formats 0x32 through 0xBF are not yet defined
	case 0xc0:
		OutputMmcDVDWriteProtectionStatus((PDVD_WRITE_PROTECTION_STATUS)lpFormat);
		break;
		// formats 0xC1 through 0x7F are not yet defined
	case 0x80:
		OutputMmcDVDAACSVolumeIdentifier(lpFormat, wFormatLength);
		break;
	case 0x81:
		OutputMmcDVDPreRecordedAACSMediaSerialNumber(lpFormat, wFormatLength);
		break;
	case 0x82:
		OutputMmcDVDAACSMediaIdentifier(lpFormat, wFormatLength);
		break;
	case 0x83:
		OutputMmcDVDAACSMediaKeyBlock(lpFormat, wFormatLength);
		break;
		// formats 0x84 through 0x8F are not yet defined
	case 0x90:
		OutputMmcDVDListOfRecognizedFormatLayers((PDVD_LIST_OF_RECOGNIZED_FORMAT_LAYERS_TYPE_CODE)lpFormat);
		break;
		// formats 0x91 through 0xFE are not yet defined
	default:
		OutputDiscLogA("\tUnknown: %02x\n", byFormatCode);
		break;
	}
}

VOID OutputMmcDVDCopyrightManagementInformation(
	PDVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR dvdCopyright,
	INT nLBA
	)
{
	OutputDiscLogA("\t\tLBA %7u, ", nLBA);
	if ((dvdCopyright->CPR_MAI & 0x80) == 0x80) {
		OutputDiscLogA("CPM exists");
		if ((dvdCopyright->CPR_MAI & 0x40) == 0x40) {
			switch (dvdCopyright->CPR_MAI & 0x0f) {
			case 0:
				OutputDiscLogA(", the sector is scrambled by CSS");
				break;
			case 0x01:
				OutputDiscLogA(", the sector is encrypted by CPPM");
				break;
			default:
				OutputDiscLogA(", reserved");
			}
		}
		else {
			OutputDiscLogA(", CSS or CPPM doesn't exists in this sector");
		}
		switch (dvdCopyright->CPR_MAI & 0x30) {
		case 0:
			OutputDiscLogA(", copying is permitted without restriction\n");
			break;
		case 0x10:
			OutputDiscLogA(", reserved\n");
			break;
		case 0x20:
			OutputDiscLogA(", one generation of copies may be made\n");
			break;
		case 0x30:
			OutputDiscLogA(", no copying is permitted\n");
			break;
		default:
			OutputDiscLogA("\n");
		}
	}
	else {
		OutputDiscLogA("CPM doesn't exist\n");
	}
}
