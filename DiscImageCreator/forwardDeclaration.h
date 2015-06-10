/*
* This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
*/
#pragma once
// memo
// CONST const
// LPBOOL BOOL*
// CONST LPBOOL => BOOL* const
// CONST BOOL*  => CONST BOOL*

#define DRIVE_VENDER_ID_SIZE	(8)
#define DRIVE_PRODUCT_ID_SIZE	(16)

#define DISC_RAW_READ			(2048)
#define CD_RAW_SECTOR_SIZE		(2352)
#define CD_RAW_READ_C2_294_SIZE	(294)
#define CD_RAW_SECTOR_WITH_C2_294_SIZE (2352+294)
#define CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE (2352+294+96)

#define MAXIMUM_NUMBER_INDEXES	(100)

#define META_CATALOG_SIZE		(13 + 1)
#define META_ISRC_SIZE			(12 + 1)
#define META_CDTEXT_SIZE		(80 + 1)

#define SCSIOP_READ_D8 (0xD8)

#define RETURNED_C2_ERROR_EXIST		(FALSE)
#define RETURNED_C2_ERROR_1ST_NONE	(TRUE)
#define RETURNED_C2_ERROR_CONTINUE	(2)
#define RETURNED_C2_ERROR_SKIP_LBA	(3)
#define RETURNED_C2_ERROR_FALSE		(4)

struct _C2_ERROR_DATA;
typedef struct _C2_ERROR_DATA *PC2_ERROR_DATA;
struct _CD_OFFSET_DATA;
typedef struct _CD_OFFSET_DATA *PCD_OFFSET_DATA;
struct _DEVICE_DATA;
typedef struct _DEVICE_DATA *PDEVICE_DATA;
struct _DISC_DATA;
typedef struct _DISC_DATA *PDISC_DATA;
struct _EXT_ARG;
typedef struct _EXT_ARG *PEXT_ARG;
struct _LOG_FILE;
typedef struct _LOG_FILE LOG_FILE;
struct _READ_CD_FLAG;
typedef struct _READ_CD_FLAG READ_CD_FLAG, *PREAD_CD_FLAG;
struct _READ_CD_TRANSFER_DATA;
typedef struct _READ_CD_TRANSFER_DATA *PREAD_CD_TRANSFER_DATA;
struct _SUB_Q_DATA;
typedef struct _SUB_Q_DATA *PSUB_Q_DATA;
