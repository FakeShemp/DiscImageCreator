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
	BYTE byFrame,
	BYTE bySecond,
	BYTE byMinute
	);

VOID LBAtoMSF(
	INT nLBA,
	LPBYTE byFrame,
	LPBYTE bySecond,
	LPBYTE byMinute
	);

VOID LittleToBig(
	_TCHAR* pOut,
	_TCHAR* pIn,
	INT nCnt
	);

LPBYTE ConvParagraphBoundary(
	PDEVICE_DATA pDevData,
	LPBYTE pv
	);
