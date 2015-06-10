/*
* This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
*/
#pragma once
#include "forwardDeclaration.h"

typedef struct _DEVICE_DATA {
	HANDLE hDevice;
	SCSI_ADDRESS address;
	UINT_PTR AlignmentMask;
	UINT uiMaxTransferLength;
	CHAR szVendorId[DRIVE_VENDER_ID_SIZE];
	CHAR szProductId[DRIVE_PRODUCT_ID_SIZE];
	BOOL bPlextor;
	union _PLEX_DRIVE_TYPE {
		BOOL bPlextorPX760A; // This flag don't use at present.
		BOOL bPlextorPX755A; // This flag don't use at present.
		BOOL bPlextorPX716A; // This flag don't use at present.
		BOOL bPlextorPX712A; // This flag don't use at present.
		BOOL bPlextorPX708A; // This flag don't use at present.
		BOOL bPlextorPX320A; // This flag don't use at present.
		BOOL bPlextorPXW5232A; // This flag don't use at present.
		BOOL bPlextorPXW5224A; // This flag don't use at present.
		BOOL bPlextorPXW4824A; // This flag don't use at present.
		BOOL bPlextorPXW4012A; // This flag don't use at present.
		BOOL bPlextorPXW2410A; // This flag don't use at present.
		BOOL bPlextorPXW1610A; // This flag don't use at present.
		BOOL bPlextorPXW1210A; // This flag don't use at present.
		BOOL bPlextorPXW8432T; // This flag don't use at present.
	} PLEX_DRIVE_TYPE, *PPLEX_DRIVE_TYPE;
	BOOL bCanCDText;
	BOOL bC2ErrorData;
} DEVICE_DATA, *PDEVICE_DATA;

typedef struct _DISC_DATA {
	CDROM_TOC toc;
	BOOL bSuccessReadToc;
	PUINT puiSessionNum;
	PINT pnTocStartLBA;
	PINT pnTocEndLBA;
	INT nAdjustSectorNum;
	INT nAllLength;
	INT nCombinedOffset;
	INT nFirstDataLBA;
	INT nLastLBAof1stSession;
	INT nStartLBAof2ndSession;
	BOOL bAudioOnly;
	WORD wCurrentMedia;
	_TCHAR szCatalog[META_CATALOG_SIZE];
	LPTSTR* pszISRC;
	LPTSTR* pszTitle;
	LPTSTR* pszPerformer;
	LPTSTR* pszSongWriter;
} DISC_DATA, *PDISC_DATA;

typedef struct _EXT_ARG {
	BOOL bC2;
	BOOL bFua;
	BOOL bIsrc;
	BOOL bAdd;
	BOOL bPre;
	BOOL bCmi;
	INT nAudioCDOffsetNum;
	UINT uiMaxRereadNum;
	UINT uiMaxC2ErrorNum;
	UINT uiRereadSpeedNum;
} EXT_ARG, *PEXT_ARG;

typedef struct _LOG_FILE {
	FILE* fpDrive;
	FILE* fpDisc;
	FILE* fpError;
} LOG_FILE, *PLOG_FILE;

typedef struct _READ_CD_TRANSFER_DATA {
	BYTE byTransferLen;
	DWORD dwBufLen;
	DWORD dwAllBufLen;
	DWORD dwBufC2Offset;
	DWORD dwBufSubOffset;
} READ_CD_TRANSFER_DATA, *PREAD_CD_TRANSFER_DATA;

#pragma pack(1)
typedef struct _SUB_Q_DATA {
	BYTE byCtl;
	BYTE byAdr;
	BYTE byTrackNum;
	BYTE byIndex;
	INT nRelativeTime;
	INT nAbsoluteTime;
	BYTE byMode;
} SUB_Q_DATA, *PSUB_Q_DATA;
# pragma pack ()

typedef struct _C2_ERROR_DATA {
	BOOL bErrorFlag;
	BOOL bErrorFlagBackup;
	CHAR cSlideSectorNum;
	CHAR cSlideSectorNumBackup;
	INT nErrorLBANum;
	INT nErrorLBANumBackup;
	UINT uiErrorBytePosCnt;
	UINT uiErrorBytePosCntBackup;
	PSHORT lpErrorBytePos;
	PSHORT lpErrorBytePosBackup;
	LPBYTE lpBufC2NoneSector;
	LPBYTE lpBufC2NoneSectorBackup;
} C2_ERROR_DATA, *PC2_ERROR_DATA;

typedef struct _CD_OFFSET_DATA {
	UINT uiMainDataSlideSize;
	INT nOffsetStart;
	INT nOffsetEnd;
	INT nFixStartLBA;
	INT nFixEndLBA;
} CD_OFFSET_DATA, *PCD_OFFSET_DATA;
