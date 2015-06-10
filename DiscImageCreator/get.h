/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */

BOOL GetDriveOffset(
	LPCSTR pszProductId,
	PINT nDriveOffset
	);

ULONG GetFilesize(
	FILE *fp,
	LONG nOffset
	);

UCHAR GetMode(
	CONST PUCHAR pBuf
	);

BOOL GetWriteOffset(
	CONST PUCHAR pBuf,
	INT nLBA,
	PINT nCombinedOffset
	);
