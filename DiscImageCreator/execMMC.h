/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */

#define CD_RAW_READ				(2048)
#define CD_RAW_READ_C2_SIZE_294	(294)
#define CD_RAW_SECTOR_SIZE		(2352)
#define DVD_RAW_READ			(2064)
#define DRIVE_MAX_SPEED			(72)
#define MAXIMUM_NUMBER_INDEXES	(100)

#define SYNC_SIZE				(12)
#define HEADER_SIZE				(4)
#define SUBHEADER_SIZE			(8)

#define META_ISRC_SIZE			(12)
#define META_CDTEXT_SIZE		(80)

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
		Raw = 1,
		Q = 1 << 1,
		Pack = 1 << 2
	} SubData;
} READ_CD_FLAG;

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
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	LPCTSTR pszOutFile,
	FILE* fpLog,
	FILE* fpCcd
	);

BOOL ReadCDForSearchingOffset(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	FILE* fpLog
	);

BOOL ReadCDPartial(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	LPCTSTR pszOutFile,
	INT nStart,
	INT nEnd,
	_READ_CD_FLAG::_SectorType flg,
	BOOL bDC,
	BOOL bCheckReading
	);

BOOL ReadBufferCapacity(
	PDEVICE_DATA pDevData,
	FILE* fpLog
	);

BOOL ReadConfiguration(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	FILE* fpLog
	);

BOOL ReadDiscInformation(
	PDEVICE_DATA pDevData,
	FILE* fpLog
	);

BOOL ReadDVD(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	LPCTSTR pszOutFile,
	LPCTSTR pszOption,
	FILE* fpLog
	);

BOOL ReadDVDRaw(
	PDEVICE_DATA pDevData,
	LPCSTR pszVendorId,
	LPCTSTR pszOutFile,
	INT nDVDSectorSize
	);

BOOL ReadDVDStructure(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	PINT nDVDSectorSize,
	FILE* fpLog
	);

BOOL ReadInquiryData(
	PDEVICE_DATA pDevData,
	FILE* fpLog
	);

BOOL ReadTestUnitReady(
	PDEVICE_DATA pDevData
	);

BOOL ReadTOC(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	FILE* fpLog
	);

BOOL ReadTOCFull(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	FILE* fpLog,
	FILE* fpCcd
	);

BOOL ReadTOCText(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	FILE* fpLog,
	FILE* fpCcd
	);

BOOL SetCDSpeed(
	PDEVICE_DATA pDevData,
	INT nCDSpeedNum,
	FILE* fpLog
	);

BOOL StartStop(
	PDEVICE_DATA pDevData,
    UCHAR Start,
    UCHAR LoadEject
	);
