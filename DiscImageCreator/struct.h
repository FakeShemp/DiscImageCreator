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

typedef struct _LOG_FILE {
	FILE* fpDrive;
	FILE* fpDisc;
	FILE* fpError;
	FILE* fpInfo;
} LOG_FILE, *PLOG_FILE;

typedef struct _EXT_ARG {
	BOOL bAdd;
	BOOL bC2;
	BOOL bCmi;
	BOOL bFua;
	BOOL bISRC;
	BOOL bMCN;
	BOOL bLibCrypt;
	BOOL bPre;
	BOOL bReverse;
	INT nAudioCDOffsetNum;
	DWORD dwMaxRereadNum;
	DWORD dwMaxC2ErrorNum;
	DWORD dwRereadSpeedNum;
} EXT_ARG, *PEXT_ARG;

typedef struct _DEVICE_DATA {
	HANDLE hDevice;
	SCSI_ADDRESS address;
	UINT_PTR AlignmentMask;
	UINT uiMaxTransferLength;
	CHAR szVendorId[DRIVE_VENDER_ID_SIZE];
	CHAR szProductId[DRIVE_PRODUCT_ID_SIZE];
	BOOL bCanCDText;
	BOOL bC2ErrorData;
	BOOL bSuccessReadToc;
	BYTE byPlexType;
	struct TRANSFER_DATA {
		UINT uiTransferLen;
		DWORD dwBufLen;
		DWORD dwAllBufLen;
		DWORD dwAdditionalBufLen; // for PX-4824
		DWORD dwBufC2Offset;
		DWORD dwBufSubOffset;
	} TRANSFER_DATA;
} DEVICE_DATA, *PDEVICE_DATA;

// Don't define value of BYTE(1byte) or SHOUT(2byte) before CDROM_TOC structure
// Because Paragraph Boundary (under 4bit of start address of buffer must 0)
// reference
// http://msdn.microsoft.com/ja-jp/library/aa290049(v=vs.71).aspx
typedef struct _DISC_DATA {
	struct _SCSI {
		_declspec(align(4)) CDROM_TOC toc; // get at CDROM_READ_TOC_EX_FORMAT_TOC
		BOOL bAudioOnly;			// get at CDROM_READ_TOC_EX_FORMAT_TOC
		LPINT lpFirstLBAListOnToc;	// get at CDROM_READ_TOC_EX_FORMAT_TOC
		LPINT lpLastLBAListOnToc;	// get at CDROM_READ_TOC_EX_FORMAT_TOC
		INT nFirstLBAofDataTrack;	// get at CDROM_READ_TOC_EX_FORMAT_TOC
		INT nLastLBAofDataTrack;	// get at CDROM_READ_TOC_EX_FORMAT_TOC
		BYTE byFirstDataTrack;		// get at CDROM_READ_TOC_EX_FORMAT_TOC
		BYTE byLastDataTrack;		// get at CDROM_READ_TOC_EX_FORMAT_TOC
		WORD wCurrentMedia;			// get at SCSIOP_GET_CONFIGURATION
		INT nAllLength;				// get at CDROM_READ_TOC_EX_FORMAT_TOC
		LPBYTE lpSessionNumList;	// get at CDROM_READ_TOC_EX_FORMAT_FULL_TOC
		INT nFirstLBAofLeadout;		// get at CDROM_READ_TOC_EX_FORMAT_FULL_TOC
		INT nFirstLBAof2ndSession;	// get at CDROM_READ_TOC_EX_FORMAT_FULL_TOC
		BOOL bCdi;					// get at CDROM_READ_TOC_EX_FORMAT_FULL_TOC
		LPTSTR* pszTitle;			// get at CDROM_READ_TOC_EX_FORMAT_CDTEXT
		LPTSTR* pszPerformer;		// get at CDROM_READ_TOC_EX_FORMAT_CDTEXT
		LPTSTR* pszSongWriter;		// get at CDROM_READ_TOC_EX_FORMAT_CDTEXT
	} SCSI;
	struct _SUB_CHANNEL {
		BOOL bDesync;
		BOOL bIndex0InTrack1;
		_TCHAR szCatalog[META_CATALOG_SIZE];
		// 0 origin, max is last track num.
		LPTSTR* pszISRC;
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
		LPBYTE lpModeList;
		// 0 origin, max is last track num.
		LPBOOL lpISRCList;
		// 0 origin, max is last track num.
		LPBYTE lpRtoWList;
	} SUB_CHANNEL;
	struct _MAIN_CHANNEL {
		INT nAdjustSectorNum;
		INT nCombinedOffset;
		UINT uiMainDataSlideSize;
		INT nOffsetStart;
		INT nOffsetEnd;
		INT nFixStartLBA;
		INT nFixEndLBA;
		INT nFixFirstLBAofLeadout;		// for sliding offset
		INT nFixFirstLBAof2ndSession;	// for sliding offset
	} MAIN_CHANNEL;
} DISC_DATA, *PDISC_DATA;

typedef struct _SUB_Q_DATA {
	BYTE byCtl : 4;		// 13th byte
	BYTE byAdr : 4;		// 13th byte
	BYTE byTrackNum;	// 14th byte
	BYTE byIndex;		// 15th byte
	BYTE byMode;		// 16th byte
	INT nRelativeTime;	// 17th - 19th byte
	INT nAbsoluteTime;	// 20th - 22nd byte
} SUB_Q_DATA, *PSUB_Q_DATA;

// EN 60908:1999 Page25-
typedef struct _SubcodeRtoW {
	CHAR command;
	CHAR instruction;
	CHAR parityQ[2];
	CHAR data[16];
	CHAR parityP[4];
} SUB_R_TO_W_DATA, *PSUB_R_TO_W_DATA;

typedef struct _C2_ERROR_DATA {
	SHORT sC2Offset;
	CHAR cSlideSectorNum;
} C2_ERROR_DATA, *PC2_ERROR_DATA;

typedef struct _C2_ERROR_DATA_PER_SECTOR {
	BOOL bErrorFlag;
	BOOL bErrorFlagBackup;
	INT nErrorLBANum;
	INT nErrorLBANumBackup;
	UINT uiErrorBytePosCnt;
	UINT uiErrorBytePosCntBackup;
	PSHORT lpErrorBytePos;
	PSHORT lpErrorBytePosBackup;
	LPBYTE lpBufC2NoneSector;
	LPBYTE lpBufC2NoneSectorBackup;
} C2_ERROR_DATA_PER_SECTOR, *PC2_ERROR_DATA_PER_SECTOR;
