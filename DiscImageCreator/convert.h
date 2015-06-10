/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */

BOOL AlignRowSubcode(
	CONST PUCHAR pBuf,
	PUCHAR Subcode
	);

BOOL AlignColumnSubcode(
	CONST PUCHAR pRowSubcode,
	PUCHAR pColumnSubcode
	);

UCHAR BcdToDec(
	UCHAR bySrc
	);

UCHAR DecToBcd(
	UCHAR bySrc
	);

INT MSFtoLBA(
	UCHAR byFrame,
	UCHAR bySecond,
	UCHAR byMinute
	);

VOID LBAtoMSF(
	INT nLBA,
	PUCHAR byFrame,
	PUCHAR bySecond,
	PUCHAR byMinute
	);

VOID LittleToBig(
	_TCHAR* out,
	CONST _TCHAR* in,
	INT cnt
	);