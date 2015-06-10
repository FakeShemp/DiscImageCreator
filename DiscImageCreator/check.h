/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */

BOOL IsValidRelativeTime(
	CONST PSUB_Q_DATA prevSubQ,
	CONST PSUB_Q_DATA subQ,
	CONST PUCHAR Subcode
	);

BOOL IsValidAbsoluteTime(
	CONST PSUB_Q_DATA prevSubQ,
	CONST PSUB_Q_DATA subQ,
	CONST PUCHAR Subcode,
	INT nLBA
	);

BOOL IsValidControl(
	CONST PSUB_Q_DATA prevPrevSubQ,
	CONST PSUB_Q_DATA prevSubQ,
	CONST PSUB_Q_DATA subQ,
	UCHAR byEndCtl
	);

BOOL IsValidDataHeader(
	CONST PUCHAR src
	);

BOOL IsValidIndex(
	CONST PSUB_Q_DATA prevPrevSubQ,
	CONST PSUB_Q_DATA prevSubQ,
	CONST PSUB_Q_DATA subQ,
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
	CONST PSUB_Q_DATA prevPrevSubQ,
	CONST PSUB_Q_DATA prevSubQ,
	CONST PSUB_Q_DATA subQ,
	UCHAR byFirstTrackNum,
	UCHAR byLastTrackNum,
	PBOOL bPrevTrackNum
	);

BOOL Is3DOData(
	CONST PUCHAR src
	);

BOOL IsMacData(
	CONST PUCHAR src
	);

BOOL CheckAndFixSubchannel(
	PDISC_DATA pDiscData,
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
	INT nLBA,
	FILE* fpLog
	);