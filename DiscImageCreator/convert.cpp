/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"

/*
	<src>
		pBuf[2352-2447] & 0x80 -> P channel
		pBuf[2352-2447] & 0x40 -> Q channel
		pBuf[2352-2447] & 0x20 -> R channel
		pBuf[2352-2447] & 0x10 -> S channel
		pBuf[2352-2447] & 0x08 -> T channel
		pBuf[2352-2447] & 0x04 -> U channel
		pBuf[2352-2447] & 0x02 -> V channel
		pBuf[2352-2447] & 0x01 -> W channel
	<dst>
		Subcode[ 0-11] -> P channel
		Subcode[12-23] -> Q channel
		Subcode[24-35] -> R channel
		Subcode[36-47] -> S channel
		Subcode[48-59] -> T channel
		Subcode[60-71] -> U channel
		Subcode[72-83] -> V channel
		Subcode[84-95] -> W channel
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
	CONST PUCHAR pColumnSubcode,
	PUCHAR pRowSubcode
	)
{
	ZeroMemory(pRowSubcode, CD_RAW_READ_SUBCODE_SIZE);
	INT nRow = 0;
	for(INT bitNum = 0; bitNum < CHAR_BIT; bitNum++) {
		for(INT nColumn = 0; nColumn < CD_RAW_READ_SUBCODE_SIZE; nRow++) {
			UINT nMask = 0x80;
			for(INT nShift = 0; nShift < CHAR_BIT; nShift++, nColumn++) {
				INT n = nShift - bitNum;
				if(n > 0) {
					pRowSubcode[nRow] |= (pColumnSubcode[nColumn] >> n) & nMask;
				}
				else {
					pRowSubcode[nRow] |= (pColumnSubcode[nColumn] << abs(n)) & nMask;
				}
				nMask >>= 1;
			}
		}
	}
	return TRUE;
}

/*
	<src>
		pRowSubcode[ 0-11] -> P channel
		pRowSubcode[12-23] -> Q channel
		pRowSubcode[24-35] -> R channel
		pRowSubcode[36-47] -> S channel
		pRowSubcode[48-59] -> T channel
		pRowSubcode[60-71] -> U channel
		pRowSubcode[72-83] -> V channel
		pRowSubcode[84-95] -> W channel
	<dst>
		pColumnSubcode[0-96] & 0x80 -> P channel
		pColumnSubcode[0-96] & 0x40 -> Q channel
		pColumnSubcode[0-96] & 0x20 -> R channel
		pColumnSubcode[0-96] & 0x10 -> S channel
		pColumnSubcode[0-96] & 0x08 -> T channel
		pColumnSubcode[0-96] & 0x04 -> U channel
		pColumnSubcode[0-96] & 0x02 -> V channel
		pColumnSubcode[0-96] & 0x01 -> W channel
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
	CONST PUCHAR pRowSubcode,
	PUCHAR pColumnSubcode
	)
{
	INT nRow = 0;
	UINT nMask = 0x80;
	for(INT bitNum = 0; bitNum < CHAR_BIT; bitNum++) {
		for(INT nColumn = 0; nColumn < CD_RAW_READ_SUBCODE_SIZE; nRow++) {
			for(INT nShift = 0; nShift < CHAR_BIT; nShift++, nColumn++) {
				INT n = nShift - bitNum;
				if(n > 0) {
					pColumnSubcode[nColumn] |= (pRowSubcode[nRow] << n) & nMask;
				}
				else {
					pColumnSubcode[nColumn] |= (pRowSubcode[nRow] >> abs(n)) & nMask;
				}
			}
		}
		nMask >>= 1;
	}
	return TRUE;
}

UCHAR BcdToDec(
	UCHAR bySrc
	)
{
	return (UCHAR)(((bySrc >> 4) & 0x0F) * 10 + (bySrc & 0x0F));
}

UCHAR DecToBcd(
	UCHAR bySrc
	)
{
	INT m = 0;
	INT n = bySrc;
	m += n / 10;
	n -= m * 10;
	return (UCHAR)(m << 4 | n);
}

INT MSFtoLBA(
	UCHAR byFrame, 
	UCHAR bySecond, 
	UCHAR byMinute
	)
{
	return byFrame + 75 * (bySecond + 60 * byMinute);
}

void LBAtoMSF(
	INT nLBA, 
	PUCHAR byFrame, 
	PUCHAR bySecond, 
	PUCHAR byMinute
	)
{
	*byFrame = (UCHAR)(nLBA % 75);
	nLBA /= 75;
	*bySecond = (UCHAR)(nLBA % 60);
	nLBA /= 60;
	*byMinute = (UCHAR)(nLBA);
}

void LittleToBig(
	_TCHAR* out,
	CONST _TCHAR* in,
	INT cnt
	)
{
	for(INT a = 0; a < cnt; a++) {
		REVERSE_BYTES_SHORT(&out[a], &in[a]);
	}
}
