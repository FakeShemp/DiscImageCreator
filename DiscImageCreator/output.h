/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */

#define BOOLEAN_TO_STRING_TRUE_FALSE(_b_) \
	( (_b_) ? _T("True") : _T("False") )

#define BOOLEAN_TO_STRING_YES_NO(_b_) \
	( (_b_) ? _T("Yes") : _T("No") )

#define OutputString(str, ...) \
		{ \
			_tprintf(str, __VA_ARGS__); \
		}
#ifdef _DEBUG
#define OutputErrorString(str, ...) \
		{ \
			_TCHAR c[4096]; \
			_stprintf(c, str, __VA_ARGS__); \
			OutputDebugString(c); \
		}
#define OutputLogString(fp, str, ...) \
		{ \
			UNREFERENCED_PARAMETER(fp); \
			_TCHAR c[4096]; \
			_stprintf(c, str, __VA_ARGS__); \
			OutputDebugString(c); \
		}
#else
#define OutputErrorString(str, ...) \
		{ \
			_ftprintf(stderr, str, __VA_ARGS__); \
		}
#define OutputLogString(fp, str, ...) \
		{ \
			_ftprintf(fp, str, __VA_ARGS__); \
		}
#endif

#define FcloseAndNull(fp) \
		{ \
			if(fp) { \
				fclose(fp); \
				fp = NULL; \
			} \
		}

#define FreeAndNull(buf) \
		{ \
			if(buf) { \
				free(buf); \
				buf = NULL; \
			} \
		}

FILE* CreateOrOpenFileW(
	LPCTSTR pszSrcPath,
	LPTSTR pszOutPath,
	LPTSTR pszFileNameWithoutPath,
	LPTSTR pszFileNameWithoutPathAndExt,
	LPCTSTR pszExt,
	LPCTSTR pszMode,
	UINT nTrackNum,
	UINT nMaxTrackNum
	);

FILE* CreateOrOpenFileA(
	LPCSTR pszSrcPath,
	LPSTR pszOutPath,
	LPSTR pszFileNameWithoutPath,
	LPCSTR pszExt,
	LPCSTR pszMode,
	INT nTrackNum,
	INT nMaxTrackNum
	);

FILE* OpenProgrammabledFile(
	LPCTSTR pszFilename,
	LPCTSTR pszMode
	);

VOID OutputIoctlInfoScsiStatus(
	CONST PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER swb,
	PUCHAR byScsiStatus,
	LPCTSTR pszFuncname,
	INT nLineNum
	);

VOID OutputIoctlInfoSense(
	UCHAR byKey,
	UCHAR byAsc,
	UCHAR byAscq
	);

VOID OutputIoctlScsiAddress(
	PDEVICE_DATA pDevData,
	FILE* fpLog
	);

VOID OutputIoctlStorageAdaptorDescriptor(
	PSTORAGE_ADAPTER_DESCRIPTOR adapterDescriptor,
	FILE* fpLog
	);

VOID OutputIoctlFloppyInfo(
	PDISK_GEOMETRY geom,
	FILE* fpLog
	);

VOID OutputMmcBufferCapacity(
	PUCHAR buf,
	FILE* fpLog
	);

VOID OutputMmcDriveSpeed(
	PCDROM_SET_SPEED pSetspeed,
	FILE* fpLog
	);

VOID OutputMmcDiscInformation(
	CONST PUCHAR pInfo,
	FILE* fpLog
	);

VOID OutputMmcFeatureNumber(
	PDEVICE_DATA pDevData,
	CONST PUCHAR pConf,
	ULONG ulAllLen,
	size_t uiSize,
	FILE* fpLog
	);

VOID OutputMmcFeatureProfileType(
	USHORT usFeatureProfileType,
	FILE* fpLog
	);

VOID OutputMmcInquiryData(
	PDEVICE_DATA pDevData,
	PINQUIRYDATA pInquiry,
	FILE* fpLog
	);

VOID OutputMmcToc(
	PDISC_DATA pDiscData,
	FILE* fpLog
	);

VOID OutputMmcTocFull(
	PDISC_DATA pDiscData,
	CONST PCDROM_TOC_FULL_TOC_DATA fullToc,
	CONST PCDROM_TOC_FULL_TOC_DATA_BLOCK pTocData,
	size_t uiTocEntries,
	FILE* fpCcd,
	FILE* fpLog
	);

VOID OutputMmcTocCDText(
	PDISC_DATA pDiscData,
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	PCHAR pTmpText,
	size_t uiTocTextEntries,
	size_t allTextSize,
	FILE* fpLog
	);

VOID OutputMmcTocCDWText(
	PDISC_DATA pDiscData,
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	PCHAR pTmpText,
	size_t uiFirstEntries,
	size_t uiTocTextEntries,
	size_t allTextSize,
	FILE* fpLog
	);

VOID OutputMmcCdC2Error296(
	CONST PUCHAR pBuf,
	INT nLBA,
	FILE* fpLog
	);

VOID OutputMmcCdMain2352(
	CONST PUCHAR pBuf,
	INT nLBA,
	FILE* fpLog
	);

VOID OutputMmcCdSub96Align(
	CONST PUCHAR pBuf,
	INT nLBA,
	FILE* fpLog
	);

VOID OutputMmcCdSub96Raw(
	CONST PUCHAR pBuf,
	INT nLBA,
	FILE* fpLog
	);

VOID OutputMmcCdSubToLog(
	PDISC_DATA pDiscData,
	CONST PUCHAR Subcode,
	CONST PUCHAR SubcodeOrg,
	INT nLBA,
	INT nTrackNum,
	FILE* fpParse
	);

VOID OutputMmcDVDStructureFormat(
	PDISC_DATA pDiscData,
	INT nNum,
	CONST PUCHAR pFormat, 
	CONST PUCHAR pStructure,
	CONST PUSHORT pStructureLength,
	PUCHAR nLayerNum,
	PINT nDVDSectorSize,
	size_t i,
	FILE* fpLog
	);

VOID OutputMmcDVDCopyrightManagementInformation(
	PUCHAR pBuf2,
	INT nLBA,
	INT i,
	FILE* fpLog
	);

VOID OutputFsVolumeDescriptor(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

VOID OutputFsBootRecord(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

bool OutputFsTagFormat(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

VOID OutputFsPrimaryVolumeDescriptorForISO9660(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

VOID OutputFsPrimaryVolumeDescriptorForJoliet(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

VOID OutputFsVolumePartitionDescriptor(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

VOID OutputFsVolumeStructureDescriptorFormat(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

VOID OutputFsVolumeRecognitionSequence(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

VOID OutputFsBootDescriptor(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

VOID OutputFsAnchorVolumeDescriptorPointer(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

VOID OutputFsPrimaryVolumeDescriptorForUDF(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

VOID OutputFsImplementationUseVolumeDescriptor(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

VOID OutputFsPartitionDescriptor(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

VOID OutputFsLogicalVolumeDescriptor(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

VOID OutputFsUnallocatedSpaceDescriptor(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

VOID OutputFsVolumeDescriptorPointer(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

VOID OutputFsVolumeDescriptorSequence(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

VOID OutputFsMasterDirectoryBlocks(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

VOID SetISRCToString(
	PDISC_DATA pDiscData,
	CONST PUCHAR Subcode,
	LPTSTR pszOutString,
	INT nTrackNum,
	BOOL bCopy
	);

VOID SetMCNToString(
	PDISC_DATA pDiscData,
	CONST PUCHAR Subcode,
	LPTSTR pszOutString,
	BOOL bCopy
	);

VOID WriteCcdFileForDisc(
	size_t tocEntries,
	UCHAR LastCompleteSession,
	FILE* fpCcd
	);

VOID WriteCcdFileForDiscCDTextLength(
	size_t cdTextSize,
	FILE* fpCcd
	);

VOID WriteCcdFileForDiscCatalog(
	PDISC_DATA pDiscData,
	FILE* fpCcd
	);

VOID WriteCcdFileForCDText(
	size_t cdTextSize,
	FILE* fpCcd
	);

VOID WriteCcdFileForCDTextEntry(
	CONST PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	size_t uiTocTextEntries,
	FILE* fpCcd
	);

VOID WriteCcdFileForSession(
	UCHAR SessionNumber,
	FILE* fpCcd
	);

VOID WriteCcdFileForSessionPregap(
	UCHAR mode,
	FILE* fpCcd
	);

VOID WriteCcdFileForEntry(
	CONST PCDROM_TOC_FULL_TOC_DATA_BLOCK toc,
	size_t a,
	FILE* fpCcd
	);

VOID WriteCcdFileForTrack(
	PDISC_DATA pDiscData,
	UINT nTrackNum,
	UCHAR byModeNum,
	BOOL bISRC,
	UCHAR byCtl,
	FILE* fpCcd
	);

VOID WriteCcdFileForTrackIndex(
	LONG index,
	LONG lba,
	FILE* fpCcd
	);

VOID WriteCueFileFirst(
	PDISC_DATA pDiscData,
	BOOL bCatalog,
	FILE* fpCue
	);

VOID WriteCueFile(
	PDISC_DATA pDiscData,
	LPCTSTR pszFilename,
	BOOL bCDG,
	UINT nTrackNum,
	UCHAR byModeNum, 
	BOOL bISRC,
	UCHAR byCtl,
	FILE* fpCue
	);

VOID WriteCueFileForIndex(
	UCHAR byIndex,
	UCHAR byFrame, 
	UCHAR bySecond,
	UCHAR byMinute,
	FILE* fpCue
	);

VOID WriteMainChannel(
	PDISC_DATA pDiscData,
	PUCHAR pBuf,
	PINT* aLBAStart,
	INT nLBA,
	INT nFixStartLBA,
	INT nFixEndLBA,
	size_t uiShift,
	FILE* fpImg
	);

VOID WriteSubChannel(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	PUCHAR pBuf,
	PUCHAR Subcode,
	PUCHAR SubcodeRaw,
	INT nLBA,
	UCHAR byCurrentTrackNum,
	FILE* fpSub,
	FILE* fpParse,
	FILE* fpCdg
	);

VOID WriteParsingSubfile(
	LPCTSTR pszSubfile
	);
