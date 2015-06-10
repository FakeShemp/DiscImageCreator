/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"

BOOL ReadFloppy(
	PDEVICE_DATA pDevData,
	LPCTSTR pszOutFile,
	FILE* fpLog
	)
{
	FILE* fp = CreateOrOpenFileW(pszOutFile, NULL, NULL, NULL, _T(".bin"), _T("wb"), 0, 0);
	if(!fp) {
		OutputErrorString(_T("Failed to open file .bin [L:%d]\n"), __LINE__);
		return FALSE;
	}

	DWORD bytesReturned = 0;
	DISK_GEOMETRY geom[20] = {0};
	DeviceIoControl(pDevData->hDevice,
		IOCTL_DISK_GET_MEDIA_TYPES, 0, 0, &geom, sizeof(geom), &bytesReturned, 0);

	OutputIoctlFloppyInfo(geom, fpLog);

	DWORD dwFloppySize = geom[0].Cylinders.LowPart *
		geom[0].TracksPerCylinder * geom[0].SectorsPerTrack * geom[0].BytesPerSector;
	PUCHAR pBuf = (PUCHAR)calloc(dwFloppySize, sizeof(UCHAR));
	if(pBuf == NULL) {
		return FALSE;
	}
	DWORD bytesRead = 0;
	BOOL bRet = ReadFile(pDevData->hDevice, pBuf, dwFloppySize, &bytesRead, 0);
	if(bRet) {
		fwrite(pBuf, dwFloppySize, sizeof(UCHAR), fp);
	}

	FcloseAndNull(fp);
	return TRUE;
}

BOOL ReadScsiGetAddress(
	PDEVICE_DATA pDevData,
	FILE* fpLog
	)
{
	ULONG ulReturned = 0;
	DeviceIoControl(pDevData->hDevice, IOCTL_SCSI_GET_ADDRESS,
		&pDevData->address, sizeof(SCSI_ADDRESS), &pDevData->address,
		sizeof(SCSI_ADDRESS), &ulReturned, NULL);
	OutputIoctlScsiAddress(pDevData, fpLog);

	return TRUE;
}

BOOL ReadStorageQueryProperty(
	PDEVICE_DATA pDevData,
	FILE* fpLog
	)
{
	ULONG ulReturned = 0;

	STORAGE_DESCRIPTOR_HEADER header = {0};
	STORAGE_PROPERTY_QUERY query;
	query.QueryType = PropertyStandardQuery;
	query.PropertyId = StorageAdapterProperty;

	if(!DeviceIoControl(pDevData->hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
		&query, sizeof(STORAGE_PROPERTY_QUERY), &header,
		sizeof(STORAGE_DESCRIPTOR_HEADER), &ulReturned, FALSE)) {
		return FALSE;
	}

	PSTORAGE_ADAPTER_DESCRIPTOR adapterDescriptor =
		(PSTORAGE_ADAPTER_DESCRIPTOR)calloc(header.Size, sizeof(UCHAR));
	if (!adapterDescriptor) {
		return FALSE;
	}

	DeviceIoControl(pDevData->hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
		&query, sizeof(STORAGE_PROPERTY_QUERY), adapterDescriptor,
		header.Size, &ulReturned, FALSE);

	OutputIoctlStorageAdaptorDescriptor(adapterDescriptor, fpLog);
	pDevData->AlignmentMask = (UINT_PTR)(adapterDescriptor->AlignmentMask);

	FreeAndNull(adapterDescriptor);

	return TRUE;
}
