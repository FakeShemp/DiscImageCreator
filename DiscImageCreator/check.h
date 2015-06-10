/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */

BOOL IsValidRelativeTime(
	CONST SUB_Q_DATA* prevSubQ,
	CONST SUB_Q_DATA* subQ,
	CONST PUCHAR Subcode
	);

BOOL IsValidAbsoluteTime(
	CONST SUB_Q_DATA* prevSubQ,
	CONST SUB_Q_DATA* subQ,
	CONST PUCHAR Subcode,
	INT nLBA
	);

BOOL IsValidControl(
	CONST SUB_Q_DATA* prevPrevSubQ,
	CONST SUB_Q_DATA* prevSubQ,
	CONST SUB_Q_DATA* subQ,
	UCHAR byEndCtl
	);

BOOL IsValidDataHeader(
	CONST PUCHAR src
	);

BOOL IsValidIndex(
	CONST SUB_Q_DATA* prevPrevSubQ,
	CONST SUB_Q_DATA* prevSubQ,
	CONST SUB_Q_DATA* subQ,
	PBOOL bPrevIndex,
	PBOOL bPrevPrevIndex
	);

BOOL IsValidISRC(
	CONST PUCHAR Subcode
	);

BOOL IsValidMCN(
	CONST PUCHAR Subcode
	);

BOOL IsValidTrackNumber(
	CONST SUB_Q_DATA* prevPrevSubQ,
	CONST SUB_Q_DATA* prevSubQ,
	CONST SUB_Q_DATA* subQ,
	UCHAR byFirstTrackNum,
	UCHAR byLastTrackNum,
	PBOOL bPrevTrackNum
	);

BOOL Is3DOData(
	CONST PUCHAR src
	);

BOOL CheckAndFixSubchannel(
	PCDROM_TOC toc,
	INT nLBA,
	PUCHAR Subcode,
	PSUB_Q_DATA subQ,
	PSUB_Q_DATA prevSubQ,
	PSUB_Q_DATA prevPrevSubQ,
	PUCHAR byCurrentTrackNum,
	PBOOL bCatalog,
	PBOOL aISRC,
	PUCHAR aEndCtl,
	PINT* aLBAStart,
	PINT* aLBAOfDataTrack,
	FILE* fpLog
	);