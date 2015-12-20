/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once

#define DRIVE_VENDER_ID_SIZE	(8)
#define DRIVE_PRODUCT_ID_SIZE	(16)

#define DISC_RAW_READ_SIZE		(2048)
#define CD_RAW_SECTOR_SIZE		(2352)
#define CD_RAW_READ_C2_294_SIZE	(294)
#define CD_RAW_SECTOR_WITH_C2_294_SIZE				(2352 + 294)
#define CD_RAW_SECTOR_WITH_C2_294_AND_SUBCODE_SIZE	(2352 + 294 + 96)

#define MAXIMUM_NUMBER_INDEXES		(100)
#define SESSION_TO_SESSION_SKIP_LBA (11400)
#define FIRST_LBA_FOR_GD			(45000)

#define SYNC_SIZE				(12)
#define HEADER_SIZE				(4)
#define MAINHEADER_MODE1_SIZE	(SYNC_SIZE + HEADER_SIZE)
#define SUBHEADER_SIZE			(8)

#define THREEDO_DIR_HEADER_SIZE		(20)
#define THREEDO_DIR_ENTRY_SIZE		(72)

#define META_CATALOG_SIZE		(13 + 1)
#define META_ISRC_SIZE			(12 + 1)
#define META_CDTEXT_SIZE		(80 + 1)

// PLEXTOR specified command
#if 0
#define SCSIOP_PLXTR_GET_AUTH		(0xD4)
#define SCSIOP_PLXTR_SEND_AUTH		(0xD5)
#endif
#define SCSIOP_PLXTR_READ_CDDA		(0xD8)
#if 0
#define SCSIOP_PLXTR_PLEXERASER		(0xE3)
#define SCSIOP_PLXTR_AS_RD			(0xE4)
#define SCSIOP_PLXTR_AS_WR			(0xE5)
#define SCSIOP_SBC_FLUSH_CACHE		(0xE7)
#endif
#define SCSIOP_PLXTR_EXTEND			(0xE9)
#if 0
#define SCSIOP_PLXTR_QCHECK			(0xEA)
#define SCSIOP_PLXTR_PREC_SPD		(0xEB)
#define SCSIOP_PLXTR_EXTEND2		(0xED)
#endif
#define SCSIOP_PLXTR_RESET			(0xEE)
#define SCSIOP_PLXTR_READ_EEPROM	(0xF1)
#if 0
#define SCSIOP_PLXTR_SCAN_TA_FETE	(0xF3)
#define SCSIOP_PLXTR_FETE_READOUT	(0xF5)
#endif

#if 0
#define PLXTR_FLAG_GET_MODE		(0x00)
#define PLXTR_FLAG_SET_MODE		(0x10)
#endif
#define PLXTR_FLAG_SPEED_READ	(0xBB)

// SONY,APPLE specified command
// ftp://ftp.t10.org/t10/document.95/95-104r0.pdf
#define SCSIOP_SONY_READ_CDDA			(0xD8)
#if 0
#define SCSIOP_SONY_READ_CDDA_MSF		(0xD9)
#define SCSIOP_SONY_READ_ALL_SUBCODES	(0xDF)
#define SCSIOP_SONY_WRITE_START			(0x50)
#define SCSIOP_SONY_WRITE_CONTINUE		(0x51)
#define SCSIOP_SONY_DISCONTINUE			(0x52)
#define SCSIOP_SONY_READ_MASTER_CUE		(0x59)
#define SCSIOP_SONY_CLOSE_TRACK			(0x5A)
#define SCSIOP_SONY_FINALIZE			(0x5B)
#define SCSIOP_SONY_FLUSH				(0x5D)
#define SCSIOP_SONY_RESERVE_TRACK		(0x53)
#define SCSIOP_SONY_WRITE_TRACK			(0x5E)
#define SCSIOP_SONY_RECOVER_TRACK		(0x5F)
#endif

// PIONEER specified command
// http://www.pioneerelectronics.com/pio/pe/images/portal/cit_3424/31636562SCSI-2RefManV31.pdf
#if 0
#define SCSIOP_PIONEER_SCAN_CD				(0xCD)
#endif
#define SCSIOP_PIONEER_READ_CDDA			(0xD8)
#if 0
#define SCSIOP_PIONEER_READ_CDDA_MSF		(0xD9)
#define SCSIOP_PIONEER_SET_CD_SPEED			(0xDA)
#define SCSIOP_PIONEER_READ_CDXA			(0xDB)
#define SCSIOP_PIONEER_READ_ALL_SUBCODE		(0xDF)
#define SCSIOP_PIONEER_READ_DRIVE_STATUS	(0xE0)
#define SCSIOP_PIONEER_WRITE_CDP			(0xE3)
#define SCSIOP_PIONEER_READ_CDP				(0xE4)
#endif

#define RETURNED_EXIST_C2_ERROR				(FALSE)
#define RETURNED_NO_C2_ERROR_1ST			(TRUE)
#define RETURNED_NO_C2_ERROR_BUT_BYTE_ERROR	(2)
#define RETURNED_CONTINUE					(3)
#define RETURNED_SKIP_LBA					(4)
#define RETURNED_FALSE						(5)

#define SCRAMBLED	(1)
#define UNSCRAMBLED (0)

#define MAKEDWORD(a, b)      ((DWORD)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))

struct _LOG_FILE;
typedef struct _LOG_FILE LOG_FILE;
struct _EXT_ARG;
typedef struct _EXT_ARG *PEXT_ARG;
struct _DEVICE;
typedef struct _DEVICE *PDEVICE;
struct _DISC;
typedef struct _DISC *PDISC;
struct _DISC_PER_SECTOR;
typedef struct _DISC_PER_SECTOR *PDISC_PER_SECTOR;
struct _MAIN_HEADER;
typedef struct _MAIN_HEADER *PMAIN_HEADER;
struct _SUB_Q;
typedef struct _SUB_Q *PSUB_Q;
struct _C2_ERROR_PER_SECTOR;
typedef struct _C2_ERROR_PER_SECTOR *PC2_ERROR_PER_SECTOR;
struct _READ_CD_FLAG;
typedef struct _READ_CD_FLAG READ_CD_FLAG, *PREAD_CD_FLAG;
