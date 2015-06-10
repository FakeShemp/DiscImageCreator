/*
 * This code is using source code below (algo.lzh\src\crc16.c).
 * homepage: http://oku.edu.mie-u.ac.jp/~okumura/algo/
 */
#pragma once
typedef unsigned char byte;

unsigned int GetCrc16CCITT(
	int n,
	byte c[]
	);
