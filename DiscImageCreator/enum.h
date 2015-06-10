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
	standardOut = 1,
	standardErr = 1 << 1,
	fileDisc = 1 << 2,
	fileDrive = 1 << 3,
	fileSubError = 1 << 4,
	fileC2Error = 1 << 5,
	fileInfo = 1 << 6
} LOG_TYPE, *PLOG_TYPE;

// a naming rule of PLEXTOR drive
// Plefix
//    PX = PLEXTOR
// Infix
//   7xx = DVD+R drive, DVD+R write speed is xx (704 - 716)
//   3xx = DVD-ROM drive, CD-RW rewrite speed is yy
//     W = CD-RW drive
//      xxyy = CD-R write speed is xx, CD-RW rewrite speed is yy
//      xyzz = CD-R write speed is x, CD-RW rewrite speed is y, CD-ROM read speed is zz
//   Sxy = Slim drive, CD-R write speed is x, CD-RW rewrite speed is y
//  Rxyy = CD-R drive, CD-R write speed is x, CD-ROM read speed is yy
//    xx = CD-ROM drive, CD-ROM read speed is xx
// Suffix
//     A = ATA
//     L = Slot Loading
//     T = Tray
//     C = Caddy
//     S = Scsi
//     U = Ultra
//     W = Wide
typedef enum _PLXTR_DRIVE_TYPE {
	No,
	PX760A,
	PX755A,
	PX716AL,
	PX716A,
	PX714A,
	PX712A,
	PX708A2,
	PX708A,
	PX704A,
	PX320A,
	PREMIUM2,
	PREMIUM,
	PXW5224A,
	PXW4824A,
	PXW4012A,
	PXW4012S,
	PXW2410A,
	PXS88T,
	PXW1610A,
	PXW1210A,
	PXW1210S,
	PXW124TS,
	PXW8432T,
	PXW8220T,
	PXW4220T,
	PXR820T,
	PXR412C,
	PX40TS,
	PX40TSUW,
	PX40TW,
	PX32TS,
	PX32CS,
	PX20TS,
	PX12TS,
	PX12CS,
	PX8XCS
} PLXTR_DRIVE_TYPE, *PPLXTR_DRIVE_TYPE;

typedef enum _DRIVE_DATA_ORDER {
	NoC2,
	MainC2Sub, // PLEXTOR, nec-based drive
	MainSubC2 // mediatek-based drive
} DRIVE_DATA_ORDER, *PDRIVE_DATA_ORDER;

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
		NoSub = 0,
		Raw = 1,	// Raw P-W sub-channel data
		Q = 2,		// Formatted Q sub-channel data
		Pack = 4	// Corrected and de-interleaved R-W sub-channel data
		// 3, 5 to 7 is reserved
	} SUB_CHANNEL_SELECTION, *PSUB_CHANNEL_SELECTION;
} READ_CD_FLAG, *PREAD_CD_FLAG;

typedef enum _SUB_RTOW_TYPE {
	Zero = 0,
	CDG = 1,
	RFull = 1 << 1,
	SFull = 1 << 2,
	TFull = 1 << 3,
	UFull = 1 << 4,
	VFull = 1 << 5,
	WFull = 1 << 6,
	Full = RFull | SFull | TFull | UFull | VFull | WFull
} SUB_RTOW_TYPE, *PSUB_RTOW_TYPE;

typedef struct _PLXTR_READ_CDDA_FLAG {
	typedef enum _SUB_CHANNEL_SELECTION {
		NoSub = 0,
		MainQ = 1,		// Main data + Formatted Q sub-channel data
		MainPack = 2,	// Main data + Raw P-Q + Corrected and de-interleaved R-W sub-channel data
		Raw = 3,		// Raw P-W sub-channel data
		MainC2Raw = 8	// Main data + C2 error data + Raw P-W sub-channel data
		// 4 to 7, 9 to 255 is reserved
	} SUB_CHANNEL_SELECTION, *PSUB_CHANNEL_SELECTION;
} PLXTR_READ_CDDA_FLAG, *PPLXTR_READ_CDDA_FLAG;
