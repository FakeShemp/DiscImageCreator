/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */

USHORT GetCrc16CCITT(
	UCHAR c[],
	INT n
	);

BOOL GetDriveOffset(
	LPCSTR pszProductId,
	PINT nDriveOffset
	);

ULONG GetFilesize(
	LONG nOffset,
	FILE *fp
	);

UCHAR GetMode(
	CONST PUCHAR pBuf
	);

BOOL GetWriteOffset(
	PDISC_DATA pDiscData,
	CONST PUCHAR pBuf
	);
