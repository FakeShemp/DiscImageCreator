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
INT AlignSubcode(
	CONST PUCHAR pBuf, 
	PUCHAR Subcode
	)
{
	ZeroMemory(Subcode, CD_RAW_READ_SUBCODE_SIZE);

	INT p = 0, q = 1, r = 2, s = 3, t = 4, u = 5, v = 6, w = 7;
	INT mask = 0x80;
	for(INT j = 0, k = 0; j < CD_RAW_READ_SUBCODE_SIZE; j++) {
		Subcode[k] |= (UCHAR)((pBuf[j] >> p) & mask);
		p++;
		if(j % 8 <= 1) {
			Subcode[k+12] |= (UCHAR)((pBuf[j] << q) & mask);
			if(q != 0) {
				q--;
			}
		}
		else {
			q++;
			Subcode[k+12] |= (UCHAR)((pBuf[j] >> q) & mask);
		}
		if(j % 8 <= 2) {
			Subcode[k+24] |= (UCHAR)((pBuf[j] << r) & mask);
			if(r != 0) {
				r--;
			}
		}
		else {
			r++;
			Subcode[k+24] |= (UCHAR)((pBuf[j] >> r) & mask);
		}
		if(j % 8 <= 3) {
			Subcode[k+36] |= (UCHAR)((pBuf[j] << s) & mask);
			if(s != 0) {
				s--;
			}
		}
		else {
			s++;
			Subcode[k+36] |= (UCHAR)((pBuf[j] >> s) & mask);
		}
		if(j % 8 <= 4) {
			Subcode[k+48] |= (UCHAR)((pBuf[j] << t) & mask);
			if(t != 0) {
				t--;
			}
		}
		else {
			t++;
			Subcode[k+48] |= (UCHAR)((pBuf[j] >> t) & mask);
		}
		if(j % 8 <= 5) {
			Subcode[k+60] |= (UCHAR)((pBuf[j] << u) & mask);
			if(u != 0) {
				u--;
			}
		}
		else {
			u++;
			Subcode[k+60] |= (UCHAR)((pBuf[j] >> u) & mask);
		}
		if(j % 8 <= 6) {
			Subcode[k+72] |= (UCHAR)((pBuf[j] << v) & mask);
			if(v != 0) {
				v--;
			}
		}
		else {
			v++;
			Subcode[k+72] |= (UCHAR)((pBuf[j] >> v) & mask);
		}
		Subcode[k+84] |= (UCHAR)((pBuf[j] << w) & mask);
		w--;
		mask /= 2;

		if(j % 8 == 7) {
			p = 0, q = 1, r = 2, s = 3, t = 4, u = 5, v = 6, w = 7;
			k++;
			mask = 0x80;
		}
	}
	return TRUE;
}

UCHAR BcdToDec(
	UCHAR bySrc
	)
{
	return (UCHAR)(((bySrc >> 4) & 0x0F) * 10 + (bySrc & 0x0F));
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
