// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

#pragma warning( disable : 4514 )
#pragma warning( disable : 4711 )
#pragma warning( push )
#pragma warning( disable : 4668 )
#pragma warning( disable : 4820 )
#include <stddef.h>
#include <stdio.h>
#include <tchar.h>
#include <time.h>
#include <windows.h>

// SPTI(need Windows Driver Kit(wdk))
#include <ntddcdrm.h> // inc\api
#include <ntddcdvd.h> // inc\api
#include <ntddmmc.h> // inc\api
#include <ntddscsi.h> // inc\api
#define _NTSCSI_USER_MODE_
#include <scsi.h> // inc\ddk
//#include <dsm.h> // inc\ddk

// reference
//http://www.t10.org/lists/asc-num.htm
//http://www.t10.org/lists/op-num.htm

////////////////////////// start spliting from dsm.h //////////////////////////
//
// Structures for SCSI pass through and SCSI pass through direct.
//
//  IA64 requires 8-byte alignment for pointers,
//  but the IA64 NT kernel expects 16-byte alignment
//
#ifdef _WIN64
    #define PTRALIGN    DECLSPEC_ALIGN(16)
#else
    #define PTRALIGN    DECLSPEC_ALIGN(4)
#endif

#define SPTWB_SENSE_LENGTH  32
#define SPTWB_DATA_LENGTH   512

typedef struct _SCSI_PASS_THROUGH_WITH_BUFFERS {
    SCSI_PASS_THROUGH   ScsiPassThrough;
    PTRALIGN UCHAR      SenseInfoBuffer[SPTWB_SENSE_LENGTH];
    PTRALIGN UCHAR      DataBuffer[SPTWB_DATA_LENGTH];
} SCSI_PASS_THROUGH_WITH_BUFFERS, *PSCSI_PASS_THROUGH_WITH_BUFFERS;

typedef struct _SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER {
    SCSI_PASS_THROUGH_DIRECT    ScsiPassThroughDirect;
    PTRALIGN UCHAR              SenseInfoBuffer[SPTWB_SENSE_LENGTH];
} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;
/////////////////////////// end spliting from dsm.h ///////////////////////////
#pragma warning( pop )



// TODO: プログラムに必要な追加ヘッダーをここで参照してください。
#include "convert.h"
#include "execMMC.h"
#include "check.h"
#include "get.h"
#include "output.h"
#include "outputGD.h"
