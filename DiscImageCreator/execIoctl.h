/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */

BOOL ReadFloppy(
	PDEVICE_DATA pDevData,
	LPCTSTR pszOutFile,
	FILE* fpLog
	);

BOOL ReadScsiGetAddress(
	PDEVICE_DATA pDevData,
	FILE* fpLog
	);

BOOL ReadStorageQueryProperty(
	PDEVICE_DATA pDevData,
	FILE* fpLog
	);
