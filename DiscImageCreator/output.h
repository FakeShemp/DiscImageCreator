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

void OutputIoctlInfoScsiStatus(
	CONST PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER swb,
	PUCHAR byScsiStatus,
	LPCTSTR pszFuncname,
	INT nLineNum
	);

void OutputIoctlInfoSense(
	UCHAR byKey,
	UCHAR byAsc,
	UCHAR byAscq
	);

void OutputIoctlScsiAddress(
	PDEVICE_DATA pDevData,
	FILE* fpLog
	);

void OutputIoctlStorageAdaptorDescriptor(
	PSTORAGE_ADAPTER_DESCRIPTOR adapterDescriptor,
	FILE* fpLog
	);

void OutputMmcInquiryData(
	PDEVICE_DATA pDevData,
	PINQUIRYDATA pInquiry,
	FILE* fpLog
	);

void OutputMmcDriveSpeed(
	PCDROM_SET_SPEED pSetspeed,
	FILE* fpLog
	);

void OutputMmcFeatureNumber(
	PDEVICE_DATA pDevData,
	CONST PUCHAR pConf,
	ULONG ulAllLen,
	size_t uiSize,
	FILE* fpLog
	);

void OutputMmcFeatureProfileType(
	USHORT usFeatureProfileType,
	FILE* fpLog
	);

void OutputMmcTocFull(
	PDISC_DATA pDiscData,
	CONST PCDROM_TOC_FULL_TOC_DATA fullToc,
	CONST PCDROM_TOC_FULL_TOC_DATA_BLOCK pTocData,
	size_t uiTocEntries,
	FILE* fpCcd,
	FILE* fpLog
	);

void OutputMmcTocCDText(
	PDISC_DATA pDiscData,
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	PCHAR pTmpText,
	size_t uiTocTextEntries,
	size_t allTextSize,
	FILE* fpLog
	);

void OutputMmcTocCDWText(
	PDISC_DATA pDiscData,
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	PCHAR pTmpText,
	size_t uiFirstEntries,
	size_t uiTocTextEntries,
	size_t allTextSize,
	FILE* fpLog
	);

void OutputMmcCdC2Error296(
	CONST PUCHAR pBuf,
	INT nLBA,
	FILE* fpLog
	);

void OutputMmcCdMain2352(
	CONST PUCHAR pBuf,
	INT nLBA,
	FILE* fpLog
	);

void OutputMmcCdSub96Align(
	CONST PUCHAR pBuf,
	INT nLBA,
	FILE* fpLog
	);

void OutputMmcCdSub96Raw(
	CONST PUCHAR pBuf,
	INT nLBA,
	FILE* fpLog
	);

void OutputMmcCdSubToLog(
	PDISC_DATA pDiscData,
	CONST PUCHAR Subcode,
	CONST PUCHAR SubcodeOrg,
	INT nLBA,
	INT nTrackNum,
	FILE* fpParse
	);

void OutputMmcDVDStructureFormat(
	CONST PUCHAR pFormat, 
	CONST PUCHAR pStructure,
	CONST PUSHORT pStructureLength,
	PINT nDVDSectorSize,
	size_t i,
	FILE* fpLog
	);

void OutputMmcDVDCopyrightManagementInformation(
	PUCHAR pBuf2,
	INT nLBA,
	INT i,
	FILE* fpLog
	);

void OutputFsVolumeDescriptor(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

void OutputFsBootRecord(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

bool OutputFsTagFormat(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

void OutputFsPrimaryVolumeDescriptorForISO9660(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

void OutputFsPrimaryVolumeDescriptorForJoliet(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

void OutputFsVolumePartitionDescriptor(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

void OutputFsVolumeStructureDescriptorFormat(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

void OutputFsVolumeRecognitionSequence(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

void OutputFsBootDescriptor(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

void OutputFsAnchorVolumeDescriptorPointer(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

void OutputFsPrimaryVolumeDescriptorForUDF(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

void OutputFsImplementationUseVolumeDescriptor(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

void OutputFsPartitionDescriptor(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

void OutputFsLogicalVolumeDescriptor(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

void OutputFsUnallocatedSpaceDescriptor(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

void OutputFsVolumeDescriptorPointer(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

void OutputFsVolumeDescriptorSequence(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

void OutputFsMasterDirectoryBlocks(
	CONST PUCHAR buf,
	INT idx,
	FILE* fpLog
	);

void SetISRCToString(
	PDISC_DATA pDiscData,
	CONST PUCHAR Subcode,
	LPTSTR pszOutString,
	INT nTrackNum,
	BOOL bCopy
	);

void SetMCNToString(
	PDISC_DATA pDiscData,
	CONST PUCHAR Subcode,
	LPTSTR pszOutString,
	BOOL bCopy
	);

void WriteCcdFileForDisc(
	size_t tocEntries,
	UCHAR LastCompleteSession,
	FILE* fpCcd
	);

void WriteCcdFileForDiscCDTextLength(
	size_t cdTextSize,
	FILE* fpCcd
	);

void WriteCcdFileForDiscCatalog(
	PDISC_DATA pDiscData,
	FILE* fpCcd
	);

void WriteCcdFileForCDText(
	size_t cdTextSize,
	FILE* fpCcd
	);

void WriteCcdFileForCDTextEntry(
	CONST PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	size_t uiTocTextEntries,
	FILE* fpCcd
	);

void WriteCcdFileForSession(
	UCHAR SessionNumber,
	FILE* fpCcd
	);

void WriteCcdFileForSessionPregap(
	UCHAR mode,
	FILE* fpCcd
	);

void WriteCcdFileForEntry(
	CONST PCDROM_TOC_FULL_TOC_DATA_BLOCK toc,
	size_t a,
	FILE* fpCcd
	);

void WriteCcdFileForTrack(
	PDISC_DATA pDiscData,
	UINT nTrackNum,
	UCHAR byModeNum,
	BOOL bISRC,
	FILE* fpCcd
	);

void WriteCcdFileForTrackIndex(
	LONG index,
	LONG lba,
	FILE* fpCcd
	);

void WriteCueFileFirst(
	PDISC_DATA pDiscData,
	BOOL bCatalog,
	FILE* fpCue
	);

void WriteCueFile(
	PDISC_DATA pDiscData,
	LPCTSTR pszFilename,
	BOOL bCDG,
	UINT nTrackNum,
	UCHAR byModeNum, 
	BOOL bISRC,
	UCHAR byCtl,
	FILE* fpCue
	);

void WriteCueFileForIndex(
	UCHAR byIndex,
	UCHAR byFrame, 
	UCHAR bySecond,
	UCHAR byMinute,
	FILE* fpCue
	);

void WriteMainChannel(
	PDISC_DATA pDiscData,
	PUCHAR pBuf,
	PINT* aLBAStart,
	INT nLBA,
	INT nFixStartLBA,
	INT nFixEndLBA,
	size_t uiShift,
	FILE* fpImg
	);

void WriteSubChannel(
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

void WriteParsingSubfile(
	LPCTSTR pszSubfile
	);
