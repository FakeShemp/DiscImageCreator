/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once

typedef enum _EXEC_TYPE {
	cd,
	dvd,
	gd,
	data,
	audio,
	floppy,
	stop,
	start,
	eject,
	closetray,
	reset,
	sub
} EXEC_TYPE, *PEXEC_TYPE;

typedef enum _LOG_TYPE {
	disc,
	drive,
	error,
	info
} LOG_TYPE, *PLOG_TYPE;

typedef enum _PLEX_DRIVE_TYPE {
	No,
	PX760,
	PX755,
	PX716,
	PX716AL,
	PX712,
	PX708,
	PX708A2,
	PX320,
	Premium2,
	PXW5232,
	PXW5224,
	PXW4824,
	PXW4012,
	PXW2410,
	PXW1610,
	PXW1210,
	PXW8432
} PLEX_DRIVE_TYPE, *PPLEX_DRIVE_TYPE;

typedef struct _READ_CD_FLAG {
	// use ExpectedSectorType : 3;
	typedef enum _EXPECTED_SECTOR_TYPE {
		All = 0,
		CDDA = 1,
		Mode1 = 2,
		Mode2 = 3,
		Mode2form1 = 4,
		Mode2form2 = 5
		// 6, 7 is reserved
	} EXPECTED_SECTOR_TYPE, *PEXPECTED_SECTOR_TYPE;

	// use ErrorFlags : 2;
	typedef enum _ERROR_FLAGS {
		NoC2 = 0,
		byte294 = 1,
		byte296 = 2
		// 3 is reserved
	} ERROR_FLAGS, *PERROR_FLAGS;

	// use HeaderCode : 2;
	typedef enum _HEADER_CODE {
		NoHeader = 0,
		SectorHeader = 1,
		SubHeader = 2,
		BothHeader = 3
	} HEADER_CODE, *PHEADER_CODE;

	// use SubChannelSelection : 3;
	typedef enum _SUB_CHANNEL_SELECTION {
		SubNone = 0,
		Raw = 1,	// Raw P-W sub-channel data
		Q = 2,		// Formatted Q sub-channel data
		Pack = 4	// Corrected and de-interleaved R-W sub-channel data
		// 3, 5 to 7 is reserved
	} SUB_CHANNEL_SELECTION, *PSUB_CHANNEL_SELECTION;
} READ_CD_FLAG, *PREAD_CD_FLAG;

typedef enum _DRIVE_DATA_ORDER {
	C2None,
	MainC2Sub, // plextor, nec-based drive
	MainSubC2 // mediatek-based drive
} DRIVE_DATA_ORDER, *PDRIVE_DATA_ORDER;

typedef enum _SUB_RTOW_TYPE {
	Zero,
	CDG,
	Fill
} SUB_RTOW_TYPE, *PSUB_RTOW_TYPE;

typedef struct _READ_D8_FLAG {
	typedef enum _SUB_CHANNEL_SELECTION {
		SubNone = 0,
		MainQ = 1,		// Main data + Formatted Q sub-channel data
		MainPack = 2,	// Main data + Raw P-Q + Corrected and de-interleaved R-W sub-channel data
		Raw = 3,		// Raw P-W sub-channel data
		MainC2Raw = 8	// Main data + C2 error data + Raw P-W sub-channel data
		// 4 to 7, 9 to 255 is reserved
	} SUB_CHANNEL_SELECTION, *PSUB_CHANNEL_SELECTION;
} READ_D8_FLAG, *PREAD_D8_FLAG;
