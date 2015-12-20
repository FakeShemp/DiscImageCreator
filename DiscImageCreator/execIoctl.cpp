/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "execIoctl.h"
#include "output.h"
#include "outputIoctlLog.h"

BOOL DiskGetMediaTypes(
	PDEVICE pDevice,
	LPCTSTR pszPath
	)
{
	FILE* fp = CreateOrOpenFileW(
		pszPath, NULL, NULL, NULL, NULL, _T(".bin"), _T("wb"), 0, 0);
	if (!fp) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}

	DISK_GEOMETRY geom[20] = { 0 };
	DWORD dwReturned = 0;
	BOOL bRet = DeviceIoControl(pDevice->hDevice,
		IOCTL_DISK_GET_MEDIA_TYPES, NULL, 0, &geom, sizeof(geom), &dwReturned, 0);
	if (bRet) {
		OutputFloppyInfo(geom, dwReturned / sizeof(DISK_GEOMETRY));
		bRet = DeviceIoControl(pDevice->hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, 
			NULL, 0, &geom, sizeof(DISK_GEOMETRY), &dwReturned, 0);
		if (bRet) {
			OutputFloppyInfo(geom, 1);
			DWORD dwFloppySize = geom[0].Cylinders.LowPart *
				geom[0].TracksPerCylinder * geom[0].SectorsPerTrack * geom[0].BytesPerSector;
			LPBYTE lpBuf = (LPBYTE)calloc(dwFloppySize, sizeof(BYTE));
			if (!lpBuf) {
				FcloseAndNull(fp);
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
			DWORD dwBytesRead = 0;
			bRet = ReadFile(pDevice->hDevice, lpBuf, dwFloppySize, &dwBytesRead, 0);
			if (bRet) {
				fwrite(lpBuf, sizeof(BYTE), dwFloppySize, fp);
			}
			FreeAndNull(lpBuf);
		}
		else {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		}
	}
	else {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	FcloseAndNull(fp);
	return TRUE;
}

BOOL ScsiGetAddress(
	PDEVICE pDevice
	)
{
	DWORD dwReturned = 0;
	BOOL bRet = DeviceIoControl(pDevice->hDevice,
		IOCTL_SCSI_GET_ADDRESS, &pDevice->address, sizeof(SCSI_ADDRESS),
		&pDevice->address, sizeof(SCSI_ADDRESS), &dwReturned, NULL);
	if (bRet) {
		OutputScsiAddress(pDevice);
	}
	else {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	// Because USB drive failed
	return TRUE;
}

BOOL ScsiPassThroughDirect(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	LPVOID lpCdb,
	BYTE byCdbLength,
	LPVOID pvBuffer,
	DWORD dwBufferLength,
	LPBYTE byScsiStatus,
	LPCTSTR pszFuncName,
	LONG lLineNum
	)
{
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER swb = { 0 };
	swb.ScsiPassThroughDirect.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
	swb.ScsiPassThroughDirect.PathId = pDevice->address.PathId;
	swb.ScsiPassThroughDirect.TargetId = pDevice->address.TargetId;
	swb.ScsiPassThroughDirect.Lun = pDevice->address.Lun;
	swb.ScsiPassThroughDirect.CdbLength = byCdbLength;
	swb.ScsiPassThroughDirect.SenseInfoLength = SENSE_BUFFER_SIZE;
	swb.ScsiPassThroughDirect.DataIn = SCSI_IOCTL_DATA_IN;
	swb.ScsiPassThroughDirect.DataTransferLength = dwBufferLength;
	swb.ScsiPassThroughDirect.TimeOutValue = pDevice->dwTimeOutValue;
	swb.ScsiPassThroughDirect.DataBuffer = pvBuffer;
	swb.ScsiPassThroughDirect.SenseInfoOffset = 
		offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, SenseData);
	memcpy(swb.ScsiPassThroughDirect.Cdb, lpCdb, byCdbLength);

	DWORD dwLength = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
	DWORD dwReturned = 0;
	BOOL bRet = TRUE;
	BOOL bNoSense = FALSE;
	SetLastError(NO_ERROR);
	if (!DeviceIoControl(pDevice->hDevice, IOCTL_SCSI_PASS_THROUGH_DIRECT,
		&swb, dwLength, &swb, dwLength, &dwReturned, NULL)) {
		OutputLastErrorNumAndString(pszFuncName, lLineNum);
		bRet = FALSE;
		if (!pExtArg->byReadContinue || !_tcscmp(_T("SetCDSpeed"), pszFuncName)) {
			// When semaphore time out occurred, if doesn't execute sleep,
			// UNIT_ATTENSION errors occurs next ScsiPassThroughDirect executing.
			DWORD millisec = 25000;
			OutputErrorString(_T("Sleep at %d milliseconds...\n"), millisec);
			Sleep(millisec);
		}
	}
	else {
		if (swb.SenseData.SenseKey == SCSI_SENSE_NO_SENSE &&
			swb.SenseData.AdditionalSenseCode == SCSI_ADSENSE_NO_SENSE &&
			swb.SenseData.AdditionalSenseCodeQualifier == 0x00) {
			bNoSense = TRUE;
		}
		if (swb.ScsiPassThroughDirect.ScsiStatus >= SCSISTAT_CHECK_CONDITION &&
			!bNoSense) {
			OutputErrorString(_T("[F:%s][L:%d] OperationCode: %#04x\n"),
				pszFuncName, lLineNum, swb.ScsiPassThroughDirect.Cdb[0]);
			OutputScsiStatus(swb.ScsiPassThroughDirect.ScsiStatus);
			OutputSenseData(&swb.SenseData);
		}
	}
	if (bNoSense) {
		*byScsiStatus = SCSISTAT_GOOD;
	}
	else {
		*byScsiStatus = swb.ScsiPassThroughDirect.ScsiStatus;
	}
	return bRet;
}

BOOL StorageQueryProperty(
	PDEVICE pDevice,
	LPBOOL lpBusTypeUSB
	)
{
	STORAGE_PROPERTY_QUERY query;
	query.QueryType = PropertyStandardQuery;
	query.PropertyId = StorageAdapterProperty;

	STORAGE_DESCRIPTOR_HEADER header = { 0 };
	DWORD dwReturned = 0;
	if (!DeviceIoControl(pDevice->hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
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
	BOOL bRet = DeviceIoControl(pDevice->hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
		&query, sizeof(STORAGE_PROPERTY_QUERY), adapterDescriptor, header.Size, &dwReturned, FALSE);
	if (bRet) {
		OutputStorageAdaptorDescriptor(adapterDescriptor, lpBusTypeUSB);
		pDevice->uiMaxTransferLength = adapterDescriptor->MaximumTransferLength;
		pDevice->AlignmentMask = (UINT_PTR)(adapterDescriptor->AlignmentMask);
	}
	else {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
	}
	FreeAndNull(adapterDescriptor);
	return bRet;
}
