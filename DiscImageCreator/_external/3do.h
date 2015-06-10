/*
 * This code is using source code below (3do_cp10.lzh\src\3DO.H).
 * homepage: http://kaele.com/~kashima/games/3do_dir.html
 *
 *	3do.h		3DO CD-ROM FileSystem Structures
 *
 *			Analyzed by H.Kashima	Sep.7'95
 */
#pragma once
#include "../forwardDeclaration.h"

#define	REVLONG(x)	( ((unsigned long)x & 0x000000ff ) << 24 | \
	((unsigned long)x & 0x0000ff00) << 8 | \
	((unsigned long)x & 0x00ff0000) >> 8 | \
	((unsigned long)x & 0xff000000) >> 24)

struct	TDO_TopBlk	{
	char		unknown1[0x28];
	char		Mark_ID[8];
	char		unknown2[0x34];
	unsigned long	NofDirEnt;
	unsigned long	RootDir;
};

struct	TDO_DirHead	{
	unsigned long	CurPage;
	char		unknown1[8];
	unsigned long	EntSize;
	char		unknown2[4];
};

struct	TDO_EntHead	{
	unsigned long	Attr;
	char		unknown1[4];
	char		Extent[4];
};

struct	TDO_EntBody	{
	char		unknown2[4];
	unsigned long	FSize;
	unsigned long	BSize;
	char		unknown3[8];
	char		FileName[32];
};

struct	TDO_EntTail	{
	unsigned long	NofEntCpy;
	unsigned long	StartBlk[3];
};

void RecuseDir(
	PDEVICE_DATA pDevData, 
	CDB::_READ_CD* cdb, 
	char* Current, 
	unsigned long top
	);
	