/*
* This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
*/
#pragma once

typedef enum _EXEC_TYPE {
	rall,
	rgd,
	rd,
	ra,
	f,
	c,
	s,
#if 0
	dec,
	split,
#endif
	sub,
} EXEC_TYPE, *PEXEC_TYPE;

typedef struct _READ_CD_FLAG {
	typedef enum _SECTOR_TYPE {
		All = 0x00,
		CDDA = 0x01 << 2,
		Mode1 = 0x02 << 2,
		Mode2 = 0x03 << 2,
		Mode2form1 = 0x04 << 2,
		Mode2form2 = 0x05 << 2
	} SECTOR_TYPE, *PSECTOR_TYPE;

	typedef enum _MAIN_DATA {
		MainNone = 0,
		C2ErrorBlockData = 1 << 1,
		C2AndBlockErrorBits = 1 << 2,
		Edc = 1 << 3,
		UserData = 1 << 4,
		MainHeader = 1 << 5,
		SubHeader = 1 << 6,
		SyncData = 1 << 7
	} MAIN_DATA, *PMAIN_DATA;

	typedef enum _SUB_DATA {
		SubNone = 0,
		Raw = 1,
		Q = 1 << 1,
		Pack = 1 << 2
	} SUB_DATA, *PSUB_DATA;

	typedef enum _DATA_ORDER {
		MainC2Sub, // plextor, nec-based drive
		MainSubC2, // mediatek-based drive
	} DATA_ORDER, *PDATA_ORDER;
} READ_CD_FLAG, *PREAD_CD_FLAG;

typedef struct _READ_D8_FLAG {
	typedef enum _SUB_DATA {
		SubNone = 0,
		Sub16 = 1,				// Formatted Q data
		MainAndSub96 = 2,		// Main data + Raw P-W data
		Sub96 = 3,				// Raw P-W data
		MainAndC2AndSub96 = 8	// Main data + C2 error data + Raw P-W data
	} SUB_DATA, *PSUB_DATA;
} READ_D8_FLAG, *PREAD_D8_FLAG;
