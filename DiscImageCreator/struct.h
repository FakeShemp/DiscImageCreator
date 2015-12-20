/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"

// memo
//  CONST   => const
//  LPBOOL  => BOOL*
//  CONST LPBOOL => BOOL* const
//  CONST BOOL*  => const BOOL*

typedef struct _SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER {
	SCSI_PASS_THROUGH_DIRECT ScsiPassThroughDirect;
	SENSE_DATA SenseData;
} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;

typedef struct _CDVD_CAPABILITIES_PAGE_WITH_HEADER10 {
	MODE_PARAMETER_HEADER10 header;
	CDVD_CAPABILITIES_PAGE cdvd;
} CDVD_CAPABILITIES_PAGE_WITH_HEADER10, *PCDVD_CAPABILITIES_PAGE_WITH_HEADER10;

typedef struct _CDVD_CAPABILITIES_PAGE_WITH_HEADER {
	MODE_PARAMETER_HEADER header;
	CDVD_CAPABILITIES_PAGE cdvd;
} CDVD_CAPABILITIES_PAGE_WITH_HEADER, *PCDVD_CAPABILITIES_PAGE_WITH_HEADER;

typedef struct _LOG_FILE {
	FILE* fpDisc;
	FILE* fpVolDesc;
	FILE* fpDrive;
	FILE* fpMainInfo;
	FILE* fpMainError;
	FILE* fpSubInfo;
	FILE* fpSubError;
	FILE* fpC2Error;
} LOG_FILE, *PLOG_FILE;

typedef struct _EXT_ARG {
	BYTE byAdd;
	BYTE byBe;
	BYTE byD8;
	BYTE byC2;
	BYTE byCmi;
	BYTE byFua;
	BYTE byISRC;
	BYTE byMCN;
	BYTE byLibCrypt;
	BYTE byPre;
	BYTE byReverse;
	BYTE byReadContinue;
	INT nAudioCDOffsetNum;
	DWORD dwMaxRereadNum;
	DWORD dwMaxC2ErrorNum;
	DWORD dwRereadSpeedNum;
	DWORD dwTimeoutNum;
	DWORD dwSubAddionalNum;
} EXT_ARG, *PEXT_ARG;

typedef struct _DEVICE {
	HANDLE hDevice;
	SCSI_ADDRESS address;
	UINT_PTR AlignmentMask;
	UINT uiMaxTransferLength;
	CHAR szVendorId[DRIVE_VENDER_ID_SIZE];
	CHAR szProductId[DRIVE_PRODUCT_ID_SIZE];
	BYTE byPlxtrType;
	BYTE bySuccessReadToc;
	BYTE bySuccessReadTocFull;
	WORD wMaxReadSpeed;
	DWORD dwTimeOutValue;
	struct _TRANSFER {
		UINT uiTransferLen;
		DWORD dwBufLen;
		DWORD dwAllBufLen;
		DWORD dwAdditionalBufLen; // for PX-4824
		DWORD dwBufC2Offset;
		DWORD dwBufSubOffset;
	} TRANSFER, *PTRANSFER;
	struct _FEATURE {
		BYTE byCanCDText;
		BYTE byC2ErrorData;
		BYTE byModePage2a;
		BYTE bySetCDSpeed;
		BYTE byReadBufCapa;
		BYTE reserved[3];
	} FEATURE, *PFEATURE;
} DEVICE, *PDEVICE;

typedef struct _GDROM_TRACK_DATA {
	UCHAR Reserved;
	UCHAR Control : 4;
	UCHAR Adr : 4;
	UCHAR TrackNumber;
	UCHAR Reserved1;
	LONG Address;
} GDROM_TRACK_DATA, *PGDROM_TRACK_DATA;

// Don't define value of BYTE(1byte) or SHOUT(2byte) before CDROM_TOC structure
// Because Paragraph Boundary (under 4bit of start address of buffer must 0)
// reference
// http://msdn.microsoft.com/ja-jp/library/aa290049(v=vs.71).aspx
typedef struct _DISC {
	struct _SCSI {
		_declspec(align(4)) CDROM_TOC toc; // get at CDROM_READ_TOC_EX_FORMAT_TOC
		INT nAllLength;				// get at CDROM_READ_TOC_EX_FORMAT_TOC
		LPINT lpFirstLBAListOnToc;	// get at CDROM_READ_TOC_EX_FORMAT_TOC
		LPINT lpLastLBAListOnToc;	// get at CDROM_READ_TOC_EX_FORMAT_TOC
		INT nFirstLBAofDataTrack;	// get at CDROM_READ_TOC_EX_FORMAT_TOC
		INT nLastLBAofDataTrack;	// get at CDROM_READ_TOC_EX_FORMAT_TOC
		BYTE byFirstDataTrack;		// get at CDROM_READ_TOC_EX_FORMAT_TOC
		BYTE byLastDataTrack;		// get at CDROM_READ_TOC_EX_FORMAT_TOC
		BYTE byAudioOnly;			// get at CDROM_READ_TOC_EX_FORMAT_TOC
		BYTE byCdi;					// get at CDROM_READ_TOC_EX_FORMAT_FULL_TOC
		LPBYTE lpSessionNumList;	// get at CDROM_READ_TOC_EX_FORMAT_FULL_TOC
		INT nFirstLBAofLeadout;		// get at CDROM_READ_TOC_EX_FORMAT_FULL_TOC
		INT nFirstLBAof2ndSession;	// get at CDROM_READ_TOC_EX_FORMAT_FULL_TOC
		LPSTR* pszTitle;			// get at CDROM_READ_TOC_EX_FORMAT_CDTEXT
		LPSTR* pszPerformer;		// get at CDROM_READ_TOC_EX_FORMAT_CDTEXT
		LPSTR* pszSongWriter;		// get at CDROM_READ_TOC_EX_FORMAT_CDTEXT
		WORD wCurrentMedia;			// get at SCSIOP_GET_CONFIGURATION
		BYTE reserved[2];
	} SCSI;
	struct _MAIN {
		INT nAdjustSectorNum;
		INT nCombinedOffset;
		UINT uiMainDataSlideSize;
		INT nOffsetStart;
		INT nOffsetEnd;
		INT nFixStartLBA;
		INT nFixEndLBA;
		INT nFixFirstLBAofLeadout;		// for sliding offset
		INT nFixFirstLBAof2ndSession;	// for sliding offset
		// 0 origin, max is last track num.
		LPBYTE lpModeList;
		BOOL bPathType; // use path table record
	} MAIN;
	struct _SUB {
		BYTE byDesync;
		BYTE byIndex0InTrack1;
		BYTE byCatalog;
		BYTE byISRC;
		INT nFirstLBAForMCN[3][2];
		INT nRangeLBAForMCN[3][2];
		INT nPrevMCNSector;
		CHAR szCatalog[META_CATALOG_SIZE];
		INT nFirstLBAForISRC[3][2];
		INT nRangeLBAForISRC[3][2];
		INT nPrevISRCSector;
		// 0 origin, max is last track num.
		LPSTR* pszISRC;
		// 0 origin, max is last track num.
		// toc indexes in priority. single ptr: LBA per track. double ptr: LBA per index
		LPINT* lpFirstLBAListOnSub;
		// 0 origin, max is last track num.
		// sub indexes in priority. single ptr: LBA per track. double ptr: LBA per index
		LPINT* lpFirstLBAListOnSubSync;
		// 0 origin, max is last track num.
		LPINT lpFirstLBAListOfDataTrackOnSub;
		// 0 origin, max is last track num.
		LPINT lpLastLBAListOfDataTrackOnSub;
		// 0 origin, max is last track num.
		LPBYTE lpCtlList;
		// 0 origin, max is last track num.
		LPBYTE lpEndCtlList;
		// 0 origin, max is last track num.
		LPBOOL lpISRCList;
		// 0 origin, max is last track num.
		LPBYTE lpRtoWList;
	} SUB;
	struct _GDROM_TOC {
		UCHAR reserved[2];
		UCHAR FirstTrack;
		UCHAR LastTrack;
		GDROM_TRACK_DATA TrackData[MAXIMUM_NUMBER_TRACKS];
		LONG Length;
	} GDROM_TOC;
	struct _PROTECT {
		BYTE reserved;
		BYTE byExist;
		BYTE byTmpForSafeDisc;
		CHAR name[12 + 1];
		// for skipping unreadable file
		struct _ERROR_SECTOR {
			INT nExtentPos;
			INT nNextExtentPos;
			INT nSectorSize;
		} ERROR_SECTOR;
		INT nPrevLBAOfPathTablePos; // for CodeLock
		INT nNextLBAOfLastVolDesc; // for CodeLock
		INT nExtentPosForExe[16]; // for CodeLock
		CHAR nameForExe[16][12 + 1]; // for CodeLock
		INT nCntForExe; // for CodeLock
	} PROTECT;
} DISC, *PDISC;

// This buffer stores all CD data (main + c2 + sub) obtained from SCSI read command
// Depending on the situation, this may store main, main + sub.
typedef struct _DATA_IN_CD {
	LPBYTE present;
	LPBYTE next;
	LPBYTE nextNext;
} DATA_IN_CD, *PDATA_IN_CD;

// This buffer stores the header of mode 1 of main channel obtained from DATA_IN_CD structure
// If it doesn't exist in DATA_IN_CD, set manually
typedef struct _MAIN_HEADER {
	BYTE prev[MAINHEADER_MODE1_SIZE];
	BYTE present[MAINHEADER_MODE1_SIZE];
} MAIN_HEADER, *PMAIN_HEADER;

// This buffer stores the aligned subcode obtained from DATA_IN_CD structure
typedef struct _SUBCODE {
	BYTE present[CD_RAW_READ_SUBCODE_SIZE];
	BYTE next[CD_RAW_READ_SUBCODE_SIZE];
	BYTE nextNext[CD_RAW_READ_SUBCODE_SIZE];
} SUBCODE, *PSUBCODE;

// This buffer stores the Q channel of SUBCODE structure
typedef struct _SUB_Q_PER_SECTOR {
	BYTE reserved;
	BYTE byCtl : 4;		// 13th byte
	BYTE byAdr : 4;		// 13th byte
	BYTE byTrackNum;	// 14th byte
	BYTE byIndex;		// 15th byte
	INT nRelativeTime;	// 16th - 18th byte
	INT nAbsoluteTime;	// 20th - 22nd byte
} SUB_Q_PER_SECTOR, *PSUB_Q_PER_SECTOR;

typedef struct _SUB_Q {
	SUB_Q_PER_SECTOR prevPrev;
	SUB_Q_PER_SECTOR prev;
	SUB_Q_PER_SECTOR present;
	SUB_Q_PER_SECTOR next;
	SUB_Q_PER_SECTOR nextNext;
} SUB_Q, *PSUB_Q;

typedef struct _DISC_PER_SECTOR {
	DATA_IN_CD data;
	MAIN_HEADER mainHeader;
	SUBCODE subcode;
	SUB_Q subQ;
} DISC_PER_SECTOR, *PDISC_PER_SECTOR;

// This buffer stores the R to W channel (only use to check)
typedef struct _SUB_R_TO_W {
	CHAR command;
	CHAR instruction;
	CHAR parityQ[2];
	CHAR data[16];
	CHAR parityP[4];
} SUB_R_TO_W, *PSUB_R_TO_W;

typedef struct _C2_ERROR_PER_SECTOR {
	BYTE reserved[2];
	BYTE byErrorFlag;
	BYTE byErrorFlagBackup;
	INT nErrorLBANum;
	INT nErrorLBANumBackup;
	UINT uiErrorBytePosCnt;
	UINT uiErrorBytePosCntBackup;
	PSHORT lpErrorBytePos;
	PSHORT lpErrorBytePosBackup;
	LPBYTE lpBufNoC2Sector;
	LPBYTE lpBufNoC2SectorBackup;
} C2_ERROR_PER_SECTOR, *PC2_ERROR_PER_SECTOR;
