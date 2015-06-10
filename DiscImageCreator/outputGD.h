/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once

BOOL DescrambleDataSector(
	LPCTSTR pszPath,
	INT nLBA
	);

BOOL SplitDescrambledFile(
	LPCTSTR pszPath
	);
