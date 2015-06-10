/*
 * This code is using "RFC 1952 GZIP File Format Specification."
 */
#pragma once

#if 0
/* Flag: has the table been computed? Initially false. */
int crc_table_computed = 0;
#endif

void make_crc_table(
	unsigned int* crc_table
	);

unsigned long update_crc(
	unsigned int* crc_table,
	unsigned long crc,
	unsigned char* buf,
	int len
	);
