/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */

#define CD_RAW_READ				(2048)
#define CD_RAW_SECTOR_SIZE		(2352)
#define DVD_RAW_READ			(2064)
#define DRIVE_MAX_SPEED			(72)
#define MAXIMUM_NUMBER_INDEXES	(100)

#define SYNC_SIZE				(12)
#define HEADER_SIZE				(4)
#define SUBHEADER_SIZE			(8)

#define META_CATALOG_SIZE		(13)
#define META_ISRC_SIZE			(12)
#define META_STRING_SIZE		(80)

typedef struct _READ_CD_FLAG {
	enum _SectorType {
		All = 0x00,
		CDDA = 0x01 << 2,
		Mode1 = 0x02 << 2,
		Mode2 = 0x03 << 2,
		Mode2form1 = 0x04 << 2,
		Mode2form2 = 0x05 << 2
	} SectorType;

	enum _MainData {
		MainNone = 0,
		C2ErrorBlockData = 1 << 1,
		C2AndBlockErrorBits = 1 << 2,
		Edc = 1 << 3,
		UserData = 1 << 4,
		MainHeader = 1 << 5,
		SubHeader = 1 << 6,
		SyncData = 1 << 7
	} MainData;

	enum _SubData {
		SubNone = 0,
		PtoW = 1,
		Q = 1 << 1,
		RtoW = 1 << 3
	} SubData;
} READ_CD_FLAG;

typedef struct _CD_TEXT_INFO {
	CHAR Text[META_STRING_SIZE];
} CD_TEXT_INFO, *PCD_TEXT_INFO;

#pragma pack(1)
__declspec(align(1)) typedef struct _SUB_Q_DATA {
	UCHAR byCtl;
	UCHAR byAdr;
	UCHAR byTrackNum;
	UCHAR byIndex;
	INT nRelativeTime;
	INT nAbsoluteTime;
	UCHAR byMode;
} SUB_Q_DATA, *PSUB_Q_DATA;
# pragma pack ()

BOOL ReadCDAll(
	HANDLE hDevice,
	LPCTSTR pszOutFile,
	LPCSTR pszVendorId,
	INT nWriteOffset,
	INT nLength,
	BOOL bC2ErrorData,
	BOOL bAudioOnly,
	FILE* fpLog,
	FILE* fpCcd
	);

BOOL ReadCDForSearchingOffset(
	HANDLE hDevice,
	LPCSTR pszVendorId,
	LPCSTR pszProductId,
	PINT nCombinedOffset,
	PBOOL bAudioOnly,
	FILE* fpLog
	);

BOOL ReadCDPartial(
	HANDLE hDevice,
	LPCTSTR pszOutFile,
	LPCSTR pszVendorId,
	INT nStart,
	INT nEnd,
	_READ_CD_FLAG::_SectorType flg,
	BOOL bDC
	);

BOOL ReadConfiguration(
	HANDLE hDevice,
	PUSHORT pusCurrentMedia,
	PBOOL bCanCDText,
	PBOOL bC2ErrorData,
	FILE* fpLog
	);

BOOL ReadDeviceInfo(
	HANDLE hDevice,
	LPSTR pszVendorId,
	LPSTR pszProductId,
	FILE* fpLog
	);

BOOL ReadDVD(
	HANDLE hDevice,
	LPCTSTR pszOutFile,
	LPCTSTR pszOption,
	INT nDVDSectorSize,
	FILE* fpLog
	);
#if 0
BOOL ReadDVDRaw(
	HANDLE hDevice,
	LPCSTR pszVendorId,
	LPCTSTR pszOutFile,
	LPCTSTR pszOption,
	INT nDVDSectorSize,
	FILE* fpLog
	);
#endif
BOOL ReadDVDStructure(
	HANDLE hDevice,
	PINT nDVDSectorSize,
	FILE* fpLog
	);

BOOL ReadTestUnitReady(
	HANDLE hDevice
	);

BOOL ReadTOC(
	HANDLE hDevice,
	PINT nLength,
	FILE* fpLog
	);

BOOL ReadTOCFull(
	HANDLE hDevice,
	LPCSTR pszVendorId,
	LPCSTR pszProductId,
	BOOL bCanCDText,
	FILE* fpLog,
	FILE* fpCcd
	);

BOOL ReadTOCText(
	HANDLE hDevice,
	FILE* fpLog,
	FILE* fpCcd
	);

BOOL SetCDSpeed(
	HANDLE hDevice,
	INT nCDSpeedNum,
	FILE* fpLog
	);

BOOL StartStop(
	HANDLE hDevice,
    UCHAR Start,
    UCHAR LoadEject
	);
