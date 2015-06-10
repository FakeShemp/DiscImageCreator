/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */

INT AlignSubcode(
	CONST PUCHAR pBuf,
	PUCHAR Subcode,
	PUCHAR SubcodeRtoW
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

#define WORDSWAP(w) (LOBYTE(w) << 8 | HIBYTE(w))
#define DWORDSWAP(dw) (LOBYTE(LOWORD(dw)) << 24 | HIBYTE(LOWORD(dw)) << 16 | LOBYTE(HIWORD(dw)) << 8 | HIBYTE(HIWORD(dw)))

void LittleToBig(
	_TCHAR* out,
	CONST _TCHAR* in,
	INT cnt
	);