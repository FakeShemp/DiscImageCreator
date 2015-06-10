/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */

INT AlignSubcode(
	CONST PUCHAR pBuf,
	PUCHAR Subcode
	);

UCHAR BcdToDec(
	UCHAR bySrc
	);

INT MSFtoLBA(
	UCHAR byFrame,
	UCHAR bySecond,
	UCHAR byMinute
	);

void LBAtoMSF(
	INT nLBA,
	PUCHAR byFrame,
	PUCHAR bySecond,
	PUCHAR byMinute
	);

void LittleToBig(
	_TCHAR* out,
	CONST _TCHAR* in,
	INT cnt
	);