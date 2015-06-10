/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "stdafx.h"
#include "struct.h"
#include "execIoctl.h"
#include "output.h"
#include "outputIoctlLog.h"

BOOL DiskGetMediaTypes(
	PDEVICE_DATA pDevData,
	LPCTSTR pszPath
	)
{
	FILE* fp = CreateOrOpenFileW(pszPath, NULL, NULL, NULL, _T(".bin"), _T("wb"), 0, 0);
	if (!fp) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}

	DWORD dwReturned = 0;
	DISK_GEOMETRY geom[20] = { 0 };
	BOOL bRet = DeviceIoControl(pDevData->hDevice,
		IOCTL_DISK_GET_MEDIA_TYPES, 0, 0, &geom, sizeof(geom), &dwReturned, 0);
	if (bRet) {
		OutputIoctlFloppyInfo(geom);
		DWORD dwFloppySize = geom[0].Cylinders.LowPart *
			geom[0].TracksPerCylinder * geom[0].SectorsPerTrack * geom[0].BytesPerSector;
		LPBYTE lpBuf = (LPBYTE)calloc(dwFloppySize, sizeof(BYTE));
		if (!lpBuf) {
			FcloseAndNull(fp);
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		DWORD dwBytesRead = 0;
		bRet = ReadFile(pDevData->hDevice, lpBuf, dwFloppySize, &dwBytesRead, 0);
		if (bRet) {
			fwrite(lpBuf, sizeof(BYTE), dwFloppySize, fp);
		}
		FreeAndNull(lpBuf);
	}
	else {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	FcloseAndNull(fp);
	return TRUE;
}

BOOL ScsiGetAddress(
	PDEVICE_DATA pDevData
	)
{
	DWORD dwReturned = 0;
	BOOL bRet = DeviceIoControl(pDevData->hDevice, IOCTL_SCSI_GET_ADDRESS,
		&pDevData->address, sizeof(SCSI_ADDRESS), &pDevData->address,
		sizeof(SCSI_ADDRESS), &dwReturned, NULL);
	if (bRet) {
		OutputIoctlScsiAddress(pDevData);
	}
	else {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	// Because USB drive failed
	return TRUE;
}

BOOL ScsiPassThroughDirect(
	PDEVICE_DATA pDevData,
	PVOID lpCdbCmd,
	BYTE byCdbCmdLength,
	PVOID pvBuffer,
	DWORD dwBufferLength,
	LPBYTE byScsiStatus,
	LPCTSTR pszFuncName,
	LONG lLineNum
	)
{
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER swb = { 0 };
	swb.ScsiPassThroughDirect.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
	swb.ScsiPassThroughDirect.PathId = pDevData->address.PathId;
	swb.ScsiPassThroughDirect.TargetId = pDevData->address.TargetId;
	swb.ScsiPassThroughDirect.Lun = pDevData->address.Lun;
	swb.ScsiPassThroughDirect.CdbLength = byCdbCmdLength;
	swb.ScsiPassThroughDirect.SenseInfoLength = SPTWB_SENSE_LENGTH;
	swb.ScsiPassThroughDirect.DataIn = SCSI_IOCTL_DATA_IN;
	swb.ScsiPassThroughDirect.DataTransferLength = dwBufferLength;
	swb.ScsiPassThroughDirect.TimeOutValue = 2;
	swb.ScsiPassThroughDirect.DataBuffer = pvBuffer;
	swb.ScsiPassThroughDirect.SenseInfoOffset = 
		offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, SenseInfoBuffer);
	memcpy(swb.ScsiPassThroughDirect.Cdb, lpCdbCmd, byCdbCmdLength);
	DWORD dwLength = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);

	SetLastError(NO_ERROR);

	DWORD dwReturned = 0;
	BOOL bRet = DeviceIoControl(pDevData->hDevice, IOCTL_SCSI_PASS_THROUGH_DIRECT,
		&swb, dwLength, &swb, dwLength, &dwReturned, NULL);
	*byScsiStatus = swb.ScsiPassThroughDirect.ScsiStatus;

	if (bRet) {
		OutputIoctlInfoScsiStatus(&swb, byScsiStatus, pszFuncName, lLineNum);
	}
	else {
		OutputLastErrorNumAndString(pszFuncName, lLineNum);
	}
	return bRet;
}

BOOL StorageQueryProperty(
	PDEVICE_DATA pDevData
	)
{
	DWORD dwReturned = 0;
	STORAGE_DESCRIPTOR_HEADER header = { 0 };
	STORAGE_PROPERTY_QUERY query;
	query.QueryType = PropertyStandardQuery;
	query.PropertyId = StorageAdapterProperty;

	if (!DeviceIoControl(pDevData->hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
		&query, sizeof(STORAGE_PROPERTY_QUERY), &header,
		sizeof(STORAGE_DESCRIPTOR_HEADER), &dwReturned, FALSE)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	PSTORAGE_ADAPTER_DESCRIPTOR adapterDescriptor =
		(PSTORAGE_ADAPTER_DESCRIPTOR)calloc(header.Size, sizeof(BYTE));
	if (!adapterDescriptor) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = DeviceIoControl(pDevData->hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
		&query, sizeof(STORAGE_PROPERTY_QUERY), adapterDescriptor,
		header.Size, &dwReturned, FALSE);
	if (bRet) {
		OutputIoctlStorageAdaptorDescriptor(adapterDescriptor);
		pDevData->uiMaxTransferLength = adapterDescriptor->MaximumTransferLength;
		pDevData->AlignmentMask = (UINT_PTR)(adapterDescriptor->AlignmentMask);
	}
	else {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	FreeAndNull(adapterDescriptor);
	return bRet;
}
