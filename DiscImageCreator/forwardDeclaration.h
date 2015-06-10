/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once

#define DRIVE_VENDER_ID_SIZE	(8)
#define DRIVE_PRODUCT_ID_SIZE	(16)
#define MINIMUM_DRIVE_BUF_SIZE	(512)

#define DISC_RAW_READ_SIZE		(2048)
#define CD_RAW_SECTOR_SIZE		(2352)
#define CD_RAW_READ_C2_294_SIZE	(294)
#define CD_RAW_SECTOR_WITH_C2_294_SIZE				(2352 + 294)
#define CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE	(2352 + 294 + 96)

#define MAXIMUM_NUMBER_INDEXES		(100)
#define SESSION_TO_SESSION_SKIP_LBA (11400)
#define FIRST_LBA_FOR_GD			(45000)

#define SYNC_SIZE		(12)
#define HEADER_SIZE		(4)
#define SUBHEADER_SIZE	(8)

#define THREEDO_DIR_HEADER_SIZE		(20)
#define THREEDO_DIR_ENTRY_SIZE		(72)

#define META_CATALOG_SIZE		(13 + 1)
#define META_ISRC_SIZE			(12 + 1)
#define META_CDTEXT_SIZE		(80 + 1)

#define SCSIOP_PLXTR_READ_CDDA		(0xD8)
#define SCSIOP_PLXTR_EXTEND			(0xE9)
#define SCSIOP_PLXTR_RESET			(0xEE)
#define SCSIOP_PLXTR_READ_EEPROM	(0xF1)

#if 0
#define PLXTR_FLAG_GET_MODE		(0x00)
#define PLXTR_FLAG_SET_MODE		(0x10)
#endif
#define PLXTR_FLAG_SPEED_READ	(0xBB)

#define RETURNED_EXIST_C2_ERROR				(FALSE)
#define RETURNED_NO_C2_ERROR_1ST			(TRUE)
#define RETURNED_NO_C2_ERROR_BUT_BYTE_ERROR	(2)
#define RETURNED_CONTINUE					(3)
#define RETURNED_SKIP_LBA					(4)
#define RETURNED_FALSE						(5)

#define MAKEDWORD(a, b)      ((DWORD)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))

struct _LOG_FILE;
typedef struct _LOG_FILE LOG_FILE;
struct _EXT_ARG;
typedef struct _EXT_ARG *PEXT_ARG;
struct _DEVICE;
typedef struct _DEVICE *PDEVICE;
struct _DISC;
typedef struct _DISC *PDISC;
struct _MAIN_HEADER;
typedef struct _MAIN_HEADER *PMAIN_HEADER;
struct _SUB_Q;
typedef struct _SUB_Q *PSUB_Q;
struct _C2_ERROR_PER_SECTOR;
typedef struct _C2_ERROR_PER_SECTOR *PC2_ERROR_PER_SECTOR;
struct _READ_CD_FLAG;
typedef struct _READ_CD_FLAG READ_CD_FLAG, *PREAD_CD_FLAG;
