/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "forwardDeclaration.h"

BOOL AlignRowSubcode(
	LPBYTE lpColumnSubcode,
	LPBYTE lpRowSubcode
	);

BOOL AlignColumnSubcode(
	LPBYTE lpRowSubcode,
	LPBYTE lpColumnSubcode
	);

BYTE BcdToDec(
	BYTE bySrc
	);

BYTE DecToBcd(
	BYTE bySrc
	);

INT MSFtoLBA(
	BYTE byMinute,
	BYTE bySecond,
	BYTE byFrame
	);

VOID LBAtoMSF(
	INT nLBA,
	LPBYTE byMinute,
	LPBYTE bySecond,
	LPBYTE byFrame
	);

VOID LittleToBig(
	_TCHAR* pOut,
	_TCHAR* pIn,
	INT nCnt
	);

LPVOID ConvParagraphBoundary(
	PDEVICE_DATA pDevData,
	LPBYTE pv
	);
