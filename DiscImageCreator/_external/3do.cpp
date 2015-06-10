/*
 * This code is using source code below (3do_cp10.lzh\src\3DO_DIR.C).
 * homepage: http://kaele.com/~kashima/games/3do_dir.html
 *
 *	3do_dir.c	Recuseable Directory Listing for 3DO-CDROM
 *
 *			Copyright(C)1995 By H.Kashima    Sep.7/95
 */
#include "stdafx.h"
#include "3do.h"
#include "../execIoctl.h"
#include "../output.h"
#include "../struct.h"

int read_data(PDEVICE pDevice, CDB::_READ_CD* cdb, char *bufadr, unsigned long blkadr)
{
	BYTE byScsiStatus = 0;
	cdb->StartingLBA[0] = HIBYTE(HIWORD(blkadr));
	cdb->StartingLBA[1] = LOBYTE(HIWORD(blkadr));
	cdb->StartingLBA[2] = HIBYTE(LOWORD(blkadr));
	cdb->StartingLBA[3] = LOBYTE(LOWORD(blkadr));
	CDB::_READ_CD in = *cdb;
	if (!ScsiPassThroughDirect(pDevice, &in, CDB12GENERIC_LENGTH, bufadr,
		DISC_RAW_READ, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	return TRUE;
}

void RecuseDir(PDEVICE pDevice, CDB::_READ_CD* cdb, char* Current, unsigned long top)
{
	unsigned int	cur;
	unsigned long	root, dpage;
	unsigned long	fsize, bsize, blk, ncpy, ftype;
	char		Extention[5];
	char		Path[256];
	char		bufadr[2048];
	struct	TDO_DirHead	*dirhead;
	struct	TDO_EntHead	*enthead;
	struct	TDO_EntBody	*entbody;
	struct	TDO_EntTail	*enttail;

	root = top;
	Extention[4] = (char)NULL;

	OutputDiscLogA("Directory of %s, LBA[%06d, %#07x]\n", Current, root, root);
	OutputDiscLogA("\tFile name                 Ext  Size(byte)\n");
	OutputDiscLogA("\t------------------------------------------\n");
	for (;;) {
		read_data(pDevice, cdb, bufadr, root);
		cur = 0;

		dirhead = (struct TDO_DirHead *)bufadr;
		dpage = REVLONG(dirhead->CurPage);
		cur += sizeof(struct TDO_DirHead);

		for (;;) {
			enthead = (struct TDO_EntHead *)(bufadr + cur);
			ftype = REVLONG(enthead->Attr);
			strncpy(Extention, enthead->Extent, 4);
			cur += sizeof(struct TDO_EntHead);

			entbody = (struct TDO_EntBody *)(bufadr + cur);
			bsize = REVLONG(entbody->BSize);
			fsize = REVLONG(entbody->FSize);
			cur += sizeof(struct TDO_EntBody);

			enttail = (struct TDO_EntTail *)(bufadr + cur);
			ncpy = REVLONG(enttail->NofEntCpy);
			blk = REVLONG(enttail->StartBlk[0]);
			cur += 4 * (ncpy + 2);

			OutputDiscLogA("\t%-24s <%-4s> %9ld\n",
				entbody->FileName, Extention, fsize);
			/*	    printf("%s%s\t\t<%-4s> %8ld\n",
			Current, entbody->FileName, Extention, fsize);*/

			if (ftype & 0xff000000) break;
		}
		if (dpage == 0xffffffff) break;
		root++;
	}
	OutputDiscLogA("\n");

	root = top;
	for (;;) {
		read_data(pDevice, cdb, bufadr, root);
		cur = 0;

		dirhead = (struct TDO_DirHead *)bufadr;
		dpage = REVLONG(dirhead->CurPage);
		cur += sizeof(struct TDO_DirHead);

		for (;;) {
			enthead = (struct TDO_EntHead *)(bufadr + cur);
			ftype = REVLONG(enthead->Attr);
			cur += sizeof(struct TDO_EntHead);

			entbody = (struct TDO_EntBody *)(bufadr + cur);
			cur += sizeof(struct TDO_EntBody);

			enttail = (struct TDO_EntTail *)(bufadr + cur);
			ncpy = REVLONG(enttail->NofEntCpy);
			blk = REVLONG(enttail->StartBlk[0]);
			cur += 4 * (ncpy + 2);

			if ((ftype & 0x000000ff) == 7) {
				sprintf(Path, "%s%s/", Current, entbody->FileName);
				RecuseDir(pDevice, cdb, Path, blk);
			}

			if (ftype & 0xff000000) break;
		}
		if (dpage == 0xffffffff) break;
		root++;
	}
}