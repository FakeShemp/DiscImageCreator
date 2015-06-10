// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

// TODO: プログラムに必要な追加ヘッダーをここで参照してください。

#pragma warning(disable:4514 4710 4711 4820)
#pragma warning(push)
#pragma warning(disable:4324 4365 4668)
#include <stddef.h>
#include <stdio.h>
#include <shlwapi.h>
#include <tchar.h>
#include <time.h>
#if 0
#include <TlHelp32.h>
#endif
#include <windows.h>
#include <winioctl.h>
#include <WinSock.h>
#ifdef UNICODE
#include <fcntl.h>
#include <io.h>
#endif

// SPTI(need Windows Driver Kit(wdk))
#include <ntddcdrm.h> // inc\api
#include <ntddcdvd.h> // inc\api
#include <ntddmmc.h> // inc\api
#include <ntddscsi.h> // inc\api
#define _NTSCSI_USER_MODE_
#include <scsi.h> // inc\ddk
#undef _NTSCSI_USER_MODE_
#pragma warning(pop)

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
