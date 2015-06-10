/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */

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
	INT nTrackNum,
	INT nMaxTrackNum
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

void OutputC2Error296(
	CONST PUCHAR pBuf,
	INT nLBA,
	FILE* fpLog
	);

void OutputMain2352(
	CONST PUCHAR pBuf,
	INT nLBA,
	FILE* fpLog
	);

void OutputDriveSpeed(
	PCDROM_SET_SPEED pSetspeed,
	FILE* fpLog
	);

void OutputScsiAdress(
	PDEVICE_DATA pDevData,
	FILE* fpLog
	);

void OutputStorageAdaptorDescriptor(
	PDEVICE_DATA pDevData,
	FILE* fpLog
	);

void OutputInquiryData(
	PINQUIRYDATA pInquiry,
	PDEVICE_DATA pDevData,
	FILE* fpLog
	);

void OutputDVDStructureFormat(
	CONST PUCHAR pFormat, 
	size_t i,
	CONST PUCHAR pStructure,
	CONST PUSHORT pStructureLength,
	PINT nDVDSectorSize,
	FILE* fpLog
	);

void OutputFeatureNumber(
	CONST PUCHAR pConf,
	ULONG ulAllLen,
	size_t uiSize,
	PDEVICE_DATA pDevData,
	FILE* fpLog
	);

void OutputFeatureProfileType(
	FILE* fpLog,
	USHORT usFeatureProfileType
	);

void OutputParsingSubfile(
	LPCTSTR pszSubfile
	);

void OutputScsiStatus(
	PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER swb,
	PUCHAR byScsiStatus,
	LPCTSTR pszFuncname,
	INT nLineNum
	);

void OutputSense(
	UCHAR byKey,
	UCHAR byAsc,
	UCHAR byAscq
	);

void OutputAlignSub96(
	CONST PUCHAR pBuf,
	INT nLBA,
	FILE* fpLog
	);

void OutputRawSub96(
	CONST PUCHAR pBuf,
	INT nLBA,
	FILE* fpLog
	);

void OutputSubcode(
	INT nLBA,
	INT nTrackNum,
	CONST PUCHAR Subcode,
	CONST PUCHAR SubcodeOrg,
	FILE* fpParse
	);

void OutputTocFull(
	CONST CDROM_TOC_FULL_TOC_DATA* fullToc,
	CONST CDROM_TOC_FULL_TOC_DATA_BLOCK* pTocData,
	size_t uiTocEntries,
	PDISC_DATA pDiscData,
	FILE* fpCcd,
	FILE* fpLog
	);

void OutputVolumeDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	);

void OutputBootRecord(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	);

bool OutputTagFormat(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	);

void OutputPrimaryVolumeDescriptorForISO9660(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	);

void OutputPrimaryVolumeDescriptorForJoliet(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	);

void OutputVolumePartitionDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	);

void OutputVolumeStructureDescriptorFormat(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	);

void OutputVolumeRecognitionSequence(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	);

void OutputBootDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	);

void OutputAnchorVolumeDescriptorPointer(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	);

void OutputPrimaryVolumeDescriptorForUDF(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	);

void OutputImplementationUseVolumeDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	);

void OutputPartitionDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	);

void OutputLogicalVolumeDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	);

void OutputUnallocatedSpaceDescriptor(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	);

void OutputVolumeDescriptorPointer(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	);

void OutputVolumeDescriptorSequence(
	INT idx,
	CONST PUCHAR buf,
	FILE* fpLog
	);

void OutputCopyrightManagementInformation(
	INT nLBA,
	INT i,
	PUCHAR pBuf2,
	FILE* fpLog
	);

void SetAlbumTitle(
	LPCSTR pszString
	);

void SetISRCToString(
	CONST PUCHAR Subcode,
	INT nTrackNum,
	LPTSTR pszOutString,
	BOOL bCopy
	);

void SetMCNToString(
	CONST PUCHAR Subcode,
	LPTSTR pszOutString,
	BOOL bCopy
	);

void SetPerformer(
	LPCSTR pszString
	);

void SetSongWriter(
	LPCSTR pszString
	);

void SetTitle(
	LPCSTR pszString,
	INT idx
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
	FILE* fpCcd
	);

void WriteCcdFileForCDText(
	size_t cdTextSize,
	FILE* fpCcd
	);

void WriteCcdFileForCDTextEntry(
	size_t t,
	CONST CDROM_TOC_CD_TEXT_DATA_BLOCK* pDesc,
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
	size_t a,
	CONST CDROM_TOC_FULL_TOC_DATA_BLOCK* toc,
	FILE* fpCcd
	);

void WriteCcdFileForTrack(
	INT nTrackNum,
	UCHAR byModeNum,
	BOOL bISRC,
	FILE* fpCcd
	);

void WriteCcdFileForTrackIndex(
	LONG index,
	LONG lba,
	FILE* fpCcd
	);

void WriteCueFile(
	BOOL bCatalog,
	BOOL bCDG,
	LPCTSTR pszFilename,
	INT nTrackNum,
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
	INT nLBA,
	INT nFixStartLBA,
	INT nFixEndLBA,
	size_t uiShift,
	PINT* aLBAStart,
	PUCHAR pBuf,
	FILE* fpImg
	);

void WriteSubChannel(
	PDEVICE_DATA pDevData,
	PDISC_DATA pDiscData,
	INT nLBA,
	UCHAR byCurrentTrackNum,
	PUCHAR pBuf,
	PUCHAR Subcode,
	PUCHAR SubcodeRaw,
	FILE* fpSub,
	FILE* fpParse,
	FILE* fpCdg
	);
