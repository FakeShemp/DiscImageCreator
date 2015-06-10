/*
 * This code is using "RFC 1952 GZIP File Format Specification."
 */
#include "crc32.h"

void make_crc_table(
	unsigned int* crc_table
	)
{
	unsigned long c;
	int n, k;
	for (n = 0; n < 256; n++) {
		c = (unsigned long) n;
		for (k = 0; k < 8; k++) {
			if (c & 1) {
				c = 0xedb88320L ^ (c >> 1);
			} else {
				c = c >> 1;
			}
		}
		crc_table[n] = c;
	}
#if 0
	crc_table_computed = 1;
#endif
}

unsigned long update_crc(
	unsigned int* crc_table,
	unsigned long crc,
	unsigned char* buf,
	int len
	)
{
	unsigned long c = crc ^ 0xffffffffL;
	int n;
#if 0
	if (!crc_table_computed) {
		make_crc_table();
	}
#endif
	for (n = 0; n < len; n++) {
		c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
	}
	return c ^ 0xffffffffL;
}
