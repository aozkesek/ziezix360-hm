#pragma once
#include <windows.h>
#include <winbio.h>

#include <vector>
#include <string>
#include <iostream>

#ifndef ARGUMENT_PRESENT
#define ARGUMENT_PRESENT(x) (((x) != NULL))
#endif

namespace BioHelper
{

	typedef struct _POOL_CONFIGURATION {
		ULONG ConfigurationFlags;
		ULONG DatabaseAttributes;
		WINBIO_UUID DatabaseId;
		WINBIO_UUID DataFormat;
		WCHAR SensorAdapter[MAX_PATH];
		WCHAR EngineAdapter[MAX_PATH];
		WCHAR StorageAdapter[MAX_PATH];
	} POOL_CONFIGURATION, *PPOOL_CONFIGURATION;

	HRESULT CreateCompatibleConfiguration(
		__in WINBIO_UNIT_SCHEMA* UnitSchema,
		__out POOL_CONFIGURATION* Configuration);

	HRESULT RegisterDatabase(__in WINBIO_STORAGE_SCHEMA* StorageSchema);
	HRESULT UnregisterDatabase(__in WINBIO_UUID *DatabaseId);

	HRESULT RegisterPrivateConfiguration(
                __in WINBIO_UNIT_SCHEMA* UnitSchema,
		__in POOL_CONFIGURATION* Configuration);

	HRESULT UnregisterPrivateConfiguration(
                __in WINBIO_UNIT_SCHEMA* UnitSchema,
		__in WINBIO_UUID *DatabaseId,
		__out bool *ConfigurationRemoved);

	//
	// Display routines...
	//
	// Caller must release returned message 
	// buffer with LocalFree()
	void ConvertErrorCodeToString(__in HRESULT ErrorCode, LPWSTR buffer, SIZE_T length);
	LPCTSTR ConvertSubFactorToString(__in WINBIO_BIOMETRIC_SUBTYPE SubFactor);
	LPCTSTR ConvertRejectDetailToString(__in WINBIO_REJECT_DETAIL RejectDetail);

}; 

