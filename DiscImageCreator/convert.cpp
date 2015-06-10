/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"
#include "struct.h"
#include "convert.h"

/*
	<src>
		lpBuf[2352-2447] & 0x80 -> P channel
		lpBuf[2352-2447] & 0x40 -> Q channel
		lpBuf[2352-2447] & 0x20 -> R channel
		lpBuf[2352-2447] & 0x10 -> S channel
		lpBuf[2352-2447] & 0x08 -> T channel
		lpBuf[2352-2447] & 0x04 -> U channel
		lpBuf[2352-2447] & 0x02 -> V channel
		lpBuf[2352-2447] & 0x01 -> W channel
	<dst>
		lpSubcode[ 0-11] -> P channel
		lpSubcode[12-23] -> Q channel
		lpSubcode[24-35] -> R channel
		lpSubcode[36-47] -> S channel
		lpSubcode[48-59] -> T channel
		lpSubcode[60-71] -> U channel
		lpSubcode[72-83] -> V channel
		lpSubcode[84-95] -> W channel
*/
/*
	//p(0x80)	//0x80  0	//0x40 L1	//0x20 L2	//0x10 L3	//0x08 L4	//0x04 L5	//0x02 L6	//0x01 L7
	//q(0x40)	//0x80 R1	//0x40 0	//0x20 L1	//0x10 L2	//0x08 L3	//0x04 L4	//0x02 L5	//0x01 L6
	//r(0x20)	//0x80 R2	//0x40 R1	//0x20 0	//0x10 L1	//0x08 L2	//0x04 L3	//0x02 L4	//0x01 L5
	//s(0x10)	//0x80 R3	//0x40 R2	//0x20 R1	//0x10 0	//0x08 L1	//0x04 L2	//0x02 L3	//0x01 L4
	//t(0x08)	//0x80 R4	//0x40 R3	//0x20 R2	//0x10 R1	//0x08 0	//0x04 L1	//0x02 L2	//0x01 L3
	//u(0x04)	//0x80 R5	//0x40 R4	//0x20 R3	//0x10 R2	//0x08 R1	//0x04 0	//0x02 L1	//0x01 L2
	//v(0x02)	//0x80 R6	//0x40 R5	//0x20 R4	//0x10 R3	//0x08 R2	//0x04 R1	//0x02 0	//0x01 L1
	//w(0x01)	//0x80 R7	//0x40 R6	//0x20 R5	//0x10 R4	//0x08 R3	//0x04 R2	//0x02 R1	//0x01 0
*/
BOOL AlignRowSubcode(
	LPBYTE lpColumnSubcode,
	LPBYTE lpRowSubcode
	)
{
	ZeroMemory(lpRowSubcode, CD_RAW_READ_SUBCODE_SIZE);
	INT nRow = 0;
	for (INT bitNum = 0; bitNum < CHAR_BIT; bitNum++) {
		for (INT nColumn = 0; nColumn < CD_RAW_READ_SUBCODE_SIZE; nRow++) {
			UINT nMask = 0x80;
			for (INT nShift = 0; nShift < CHAR_BIT; nShift++, nColumn++) {
				INT n = nShift - bitNum;
				if (n > 0) {
					lpRowSubcode[nRow] |= (lpColumnSubcode[nColumn] >> n) & nMask;
				}
				else {
					lpRowSubcode[nRow] |= (lpColumnSubcode[nColumn] << abs(n)) & nMask;
				}
				nMask >>= 1;
			}
		}
	}
	return TRUE;
}

/*
	<src>
		lpRowSubcode[ 0-11] -> P channel
		lpRowSubcode[12-23] -> Q channel
		lpRowSubcode[24-35] -> R channel
		lpRowSubcode[36-47] -> S channel
		lpRowSubcode[48-59] -> T channel
		lpRowSubcode[60-71] -> U channel
		lpRowSubcode[72-83] -> V channel
		lpRowSubcode[84-95] -> W channel
	<dst>
		lpColumnSubcode[0-96] & 0x80 -> P channel
		lpColumnSubcode[0-96] & 0x40 -> Q channel
		lpColumnSubcode[0-96] & 0x20 -> R channel
		lpColumnSubcode[0-96] & 0x10 -> S channel
		lpColumnSubcode[0-96] & 0x08 -> T channel
		lpColumnSubcode[0-96] & 0x04 -> U channel
		lpColumnSubcode[0-96] & 0x02 -> V channel
		lpColumnSubcode[0-96] & 0x01 -> W channel
*/
/*
	//p(0x80)	//0x80  0	//0x40 L1	//0x20 L2	//0x10 L3	//0x08 L4	//0x04 L5	//0x02 L6	//0x01 L7
	//q(0x40)	//0x80 R1	//0x40 0	//0x20 L1	//0x10 L2	//0x08 L3	//0x04 L4	//0x02 L5	//0x01 L6
	//r(0x20)	//0x80 R2	//0x40 R1	//0x20 0	//0x10 L1	//0x08 L2	//0x04 L3	//0x02 L4	//0x01 L5
	//s(0x10)	//0x80 R3	//0x40 R2	//0x20 R1	//0x10 0	//0x08 L1	//0x04 L2	//0x02 L3	//0x01 L4
	//t(0x08)	//0x80 R4	//0x40 R3	//0x20 R2	//0x10 R1	//0x08 0	//0x04 L1	//0x02 L2	//0x01 L3
	//u(0x04)	//0x80 R5	//0x40 R4	//0x20 R3	//0x10 R2	//0x08 R1	//0x04 0	//0x02 L1	//0x01 L2
	//v(0x02)	//0x80 R6	//0x40 R5	//0x20 R4	//0x10 R3	//0x08 R2	//0x04 R1	//0x02 0	//0x01 L1
	//w(0x01)	//0x80 R7	//0x40 R6	//0x20 R5	//0x10 R4	//0x08 R3	//0x04 R2	//0x02 R1	//0x01 0
*/
BOOL AlignColumnSubcode(
	LPBYTE lpRowSubcode,
	LPBYTE lpColumnSubcode
	)
{
	INT nRow = 0;
	UINT nMask = 0x80;
	for (INT bitNum = 0; bitNum < CHAR_BIT; bitNum++) {
		for (INT nColumn = 0; nColumn < CD_RAW_READ_SUBCODE_SIZE; nRow++) {
			for (INT nShift = 0; nShift < CHAR_BIT; nShift++, nColumn++) {
				INT n = nShift - bitNum;
				if (n > 0) {
					lpColumnSubcode[nColumn] |= (lpRowSubcode[nRow] << n) & nMask;
				}
				else {
					lpColumnSubcode[nColumn] |= (lpRowSubcode[nRow] >> abs(n)) & nMask;
				}
			}
		}
		nMask >>= 1;
	}
	return TRUE;
}

BYTE BcdToDec(
	BYTE bySrc
	)
{
	return (BYTE)(((bySrc >> 4) & 0x0F) * 10 + (bySrc & 0x0F));
}

BYTE DecToBcd(
	BYTE bySrc
	)
{
	INT m = 0;
	INT n = bySrc;
	m += n / 10;
	n -= m * 10;
	return (BYTE)(m << 4 | n);
}

INT MSFtoLBA(
	BYTE byFrame, 
	BYTE bySecond, 
	BYTE byMinute
	)
{
	return byFrame + 75 * (bySecond + 60 * byMinute);
}

VOID LBAtoMSF(
	INT nLBA, 
	LPBYTE byFrame, 
	LPBYTE bySecond, 
	LPBYTE byMinute
	)
{
	*byFrame = (BYTE)(nLBA % 75);
	nLBA /= 75;
	*bySecond = (BYTE)(nLBA % 60);
	nLBA /= 60;
	*byMinute = (BYTE)(nLBA);
}

VOID LittleToBig(
	_TCHAR* pOut,
	_TCHAR* pIn,
	INT nCnt
	)
{
	for (INT a = 0; a < nCnt; a++) {
		REVERSE_BYTES_SHORT(&pOut[a], &pIn[a]);
	}
}

LPBYTE ConvParagraphBoundary(
	PDEVICE_DATA pDevData,
	LPBYTE pv
	)
{
	return (LPBYTE)(((UINT_PTR)pv + pDevData->AlignmentMask) & ~pDevData->AlignmentMask);
}
