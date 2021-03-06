/*
 * This code is using below (algo.lzh\src\crc16.c).
 * homepage: http://oku.edu.mie-u.ac.jp/~okumura/algo/
 */

#pragma warning(push)
#pragma warning(disable:4018)
#include "crc16ccitt.h"

/***********************************************************
	crc16.c -- CRC
***********************************************************/
#include <limits.h>
#define CRCPOLY1  0x1021U  /* x^{16}+x^{12}+x^5+1 */

unsigned int GetCrc16CCITT(int n, byte c[])
{
	unsigned int i, j, r;

//	r = 0xFFFFU;
	r = 0;
	for (i = 0; i < n; i++) {
		r ^= (unsigned int)c[i] << (16 - CHAR_BIT);
		for (j = 0; j < CHAR_BIT; j++)
			if (r & 0x8000U) r = (r << 1) ^ CRCPOLY1;
			else             r <<= 1;
	}
	return ~r & 0xFFFFU;
}
#pragma warning(pop)
