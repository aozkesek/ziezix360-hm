#include "BioHelper.h"

typedef struct _SUBFACTOR_TEXT {
	WINBIO_BIOMETRIC_SUBTYPE SubFactor;
	LPCTSTR Text;
} SUBFACTOR_TEXT, *PSUBFACTOR_TEXT;

static const SUBFACTOR_TEXT g_SubFactorText[] = {
	{WINBIO_SUBTYPE_NO_INFORMATION,             L"(No information)"},
	{WINBIO_ANSI_381_POS_RH_THUMB,              L"RH thumb"},
	{WINBIO_ANSI_381_POS_RH_INDEX_FINGER,       L"RH index finger"},
	{WINBIO_ANSI_381_POS_RH_MIDDLE_FINGER,      L"RH middle finger"},
	{WINBIO_ANSI_381_POS_RH_RING_FINGER,        L"RH ring finger"},
	{WINBIO_ANSI_381_POS_RH_LITTLE_FINGER,      L"RH little finger"},
	{WINBIO_ANSI_381_POS_LH_THUMB,              L"LH thumb"},
	{WINBIO_ANSI_381_POS_LH_INDEX_FINGER,       L"LH index finger"},
	{WINBIO_ANSI_381_POS_LH_MIDDLE_FINGER,      L"LH middle finger"},
	{WINBIO_ANSI_381_POS_LH_RING_FINGER,        L"LH ring finger"},
	{WINBIO_ANSI_381_POS_LH_LITTLE_FINGER,      L"LH little finger"},
	{WINBIO_SUBTYPE_ANY,                        L"Any finger"},
};
static const SIZE_T k_SubFactorTextTableSize = sizeof(g_SubFactorText) / sizeof(SUBFACTOR_TEXT);


typedef struct _REJECT_DETAIL_TEXT {
	WINBIO_REJECT_DETAIL RejectDetail;
	LPCTSTR Text;
} REJECT_DETAIL_TEXT, *PREJECT_DETAIL_TEXT;

static const REJECT_DETAIL_TEXT g_RejectDetailText[] = {
	{WINBIO_FP_TOO_HIGH,        L"Scan your fingerprint a little lower."},
	{WINBIO_FP_TOO_LOW,         L"Scan your fingerprint a little higher."},
	{WINBIO_FP_TOO_LEFT,        L"Scan your fingerprint more to the right."},
	{WINBIO_FP_TOO_RIGHT,       L"Scan your fingerprint more to the left."},
	{WINBIO_FP_TOO_FAST,        L"Scan your fingerprint more slowly."},
	{WINBIO_FP_TOO_SLOW,        L"Scan your fingerprint more quickly."},
	{WINBIO_FP_POOR_QUALITY,    L"The quality of the fingerprint scan was not sufficient to make a match.  Check to make sure the sensor is clean."},
	{WINBIO_FP_TOO_SKEWED,      L"Hold your finger flat and straight when scanning your fingerprint."},
	{WINBIO_FP_TOO_SHORT,       L"Use a longer stroke when scanning your fingerprint."},
	{WINBIO_FP_MERGE_FAILURE,   L"Unable to merge samples into a single enrollment. Try to repeat the enrollment procedure from the beginning."},
};
static const SIZE_T k_RejectDetailTextTableSize = sizeof(g_RejectDetailText) / sizeof(REJECT_DETAIL_TEXT);

namespace BioHelper
{

	static HRESULT CompareConfiguration(
                __in HKEY SourceConfigList,
                __in LPWSTR SourceConfigKey,
                __in POOL_CONFIGURATION* TargetConfig,
                __out bool *IsEqual
		);

	

	static bool ConvertUuidToString(
                __in WINBIO_UUID *Uuid,
                __out LPWSTR UuidStringBuffer,
                __in SIZE_T UuidStringBufferLength,
                __in bool IncludeBraces
		);

	inline static bool IsKeyNameNumeric(__in LPWSTR KeyName, __in DWORD KeyNameLength)
	{
		if (KeyNameLength == 0)
		{
			return false;
		}
		else
		{
			for (DWORD i = 0; i < KeyNameLength; ++i)
			{
				if (!iswdigit(KeyName[i]))
				{
					return false;
				}
			}
			return true;
		}
	}

        HRESULT reg_get_value(HKEY hkey, LPCWCHAR key, DWORD flag, PVOID buffer, DWORD *size) {
                
                return HRESULT_FROM_WIN32(
                        RegGetValueW(hkey, NULL, key, flag, NULL, buffer, size)
                );
        }

        HRESULT reg_enum_key(HKEY hkey, DWORD index, LPWSTR key, LPDWORD size) {
                HRESULT hr = S_OK;
                LONG regStatus = RegEnumKeyExW(
                        hkey, index, key, size,
                        NULL, NULL, NULL, NULL);

                if (regStatus != ERROR_SUCCESS)
                {
                        if (regStatus != ERROR_NO_MORE_ITEMS)
                                hr = HRESULT_FROM_WIN32(regStatus);
                }

                return hr;
        }

        ULONG get_config_flag_from_sensor_mode(DWORD sensor_mode) {
                switch (sensor_mode)
                {
                case WINBIO_SENSOR_BASIC_MODE:
                        return WINBIO_FLAG_BASIC;
                        
                case WINBIO_SENSOR_ADVANCED_MODE:
                        return WINBIO_FLAG_ADVANCED;
                }

                return WINBIO_FLAG_DEFAULT;
        }

        DWORD get_sensor_mode_from_config_flag(ULONG config_flag) {
                if (config_flag & WINBIO_FLAG_BASIC)
                {
                        return WINBIO_SENSOR_BASIC_MODE;
                }
                else if (config_flag & WINBIO_FLAG_ADVANCED)
                {
                        return WINBIO_SENSOR_ADVANCED_MODE;
                }
                else
                {
                        return WINBIO_E_CONFIGURATION_FAILURE;
                }
        }

        HRESULT set_config_data(
                POOL_CONFIGURATION* Configuration, 
                WINBIO_STORAGE_SCHEMA* storageArray,
                SIZE_T storageCount,
                RPC_WSTR databaseIdString)
        {
                
                WINBIO_UUID databaseIdGuid;
                UuidFromStringW(databaseIdString, &databaseIdGuid);

                bool databaseFound = false;
                for (SIZE_T i = 0; i < storageCount; ++i)
                {
                        if (storageArray[i].DatabaseId == databaseIdGuid)
                        {
                                Configuration->DataFormat = storageArray[i].DataFormat;
                                Configuration->DatabaseAttributes = storageArray[i].Attributes;
                                databaseFound = true;
                                break;
                        }
                }
                if (!databaseFound)
                        return WINBIO_E_DATABASE_CANT_FIND;
                
                return S_OK;
        }

	HRESULT CreateCompatibleConfiguration(
                __in WINBIO_UNIT_SCHEMA* UnitSchema,
                __out POOL_CONFIGURATION* Configuration)
	{
		HRESULT hr = S_OK;

		if (!ARGUMENT_PRESENT(UnitSchema) || !ARGUMENT_PRESENT(Configuration))
			return E_POINTER;
		
		WINBIO_STORAGE_SCHEMA *storageArray = NULL;
		SIZE_T storageCount = 0;
		hr = WinBioEnumDatabases(WINBIO_TYPE_FINGERPRINT, &storageArray, &storageCount);
		if (FAILED(hr))
			return hr;
		
                //L"System\\CurrentControlSet\\Enum\\USB\\VID_138A&PID_0050\\2A1A42215CC0\\Device Parameters\\WinBio\\Configurations"

		std::wstring regPath = L"System\\CurrentControlSet\\Enum\\";
		regPath += UnitSchema->DeviceInstanceId;
		regPath += L"\\Device Parameters\\WinBio\\Configurations";

		HKEY configListKey = NULL;
		LONG regStatus = RegOpenKeyExW(
			HKEY_LOCAL_MACHINE,
			regPath.c_str(),
			0,
			KEY_READ,
			&configListKey
		);
		if (regStatus != ERROR_SUCCESS)
		{
			WinBioFree(storageArray);
			storageArray = NULL;
			storageCount = 0;
			return HRESULT_FROM_WIN32(regStatus);
		}

		DWORD subkeyIndex = 0;
		for (;;)
		{
			hr = S_OK;

			DWORD sensorMode = 0;
			ULONG configFlags = 0;
			DWORD systemSensor = 0;
			WINBIO_UUID dataFormat = {};
			ULONG attributes = 0;
			WCHAR sensorAdapter[MAX_PATH] = {};
			WCHAR engineAdapter[MAX_PATH] = {};
			WCHAR storageAdapter[MAX_PATH] = {};
			WCHAR subkeyName[MAX_PATH] = {};
			DWORD subkeyNameLength = ARRAYSIZE(subkeyName);
                        WCHAR databaseIdString[40] = {};

                        hr = reg_enum_key(
                                configListKey,
                                subkeyIndex,
                                subkeyName,
                                &subkeyNameLength);
                        
                        if (FAILED(hr))
                                break;

                        if (!IsKeyNameNumeric(subkeyName, subkeyNameLength))
                        {
                                subkeyIndex++;
                                continue;
                        }

			std::wstring configKeyPath = regPath + 
                                L"\\" + subkeyName;

			HKEY configKey = NULL;
			hr = HRESULT_FROM_WIN32(
				RegOpenKeyExW(
					HKEY_LOCAL_MACHINE,
					configKeyPath.c_str(),
					0,
					KEY_READ,
					&configKey));
                                 
                        if (FAILED(hr))
                                break;

			/*
				Extract values in this configuration
					SensorMode              - REG_DWORD
					SystemSensor            - REG_DWORD
					DatabaseId              - REG_SZ
					SensorAdapterBinary     - REG_SZ
					EngineAdapterBinary     - REG_SZ
					StorageAdapterBinary    - REG_SZ
			*/
                                
                        DWORD dataSize = sizeof(systemSensor);
                        hr = reg_get_value(
                                configKey, 
                                L"SystemSensor",
                                RRF_RT_REG_DWORD,
                                &systemSensor,
                                &dataSize);
                        if (FAILED(hr))
                                goto next_subkey;
                        if (systemSensor == 0)
                                goto next_subkey;
                                
                        dataSize = sizeof(sensorAdapter);
                        hr = reg_get_value(
                                configKey, 
                                L"SensorAdapterBinary",
                                RRF_RT_REG_SZ,
                                &sensorAdapter,
                                &dataSize);
                        if (FAILED(hr))
                                goto next_subkey;

                        dataSize = sizeof(engineAdapter);
                        hr = reg_get_value(
                                configKey, 
                                L"EngineAdapterBinary",
                                RRF_RT_REG_SZ,
                                &engineAdapter,
                                &dataSize);
                        if (FAILED(hr))
                                goto next_subkey;

                        dataSize = sizeof(storageAdapter);
                        hr = reg_get_value(
                                configKey, 
                                L"StorageAdapterBinary",
                                RRF_RT_REG_SZ,
                                &storageAdapter,
                                &dataSize);
                        if (FAILED(hr))
                                goto next_subkey;

                        dataSize = sizeof(sensorMode);
                        hr = reg_get_value(
                                configKey,
                                L"SensorMode",
                                RRF_RT_REG_DWORD,
                                &sensorMode,
                                &dataSize);
                        if (FAILED(hr))
                                goto next_subkey;

                        configFlags = get_config_flag_from_sensor_mode(sensorMode);

                        dataSize = sizeof(databaseIdString);
                        hr = reg_get_value(
                                configKey,
                                L"DatabaseId",
                                RRF_RT_REG_SZ,
                                &databaseIdString,
                                &dataSize);
                        if (FAILED(hr))
                                goto next_subkey;

                        // copy results to output structure - we only want this
                        // one if it's a derived from a system sensor config

                        hr = set_config_data(
                                Configuration, 
                                storageArray, storageCount, 
                                (RPC_WSTR)databaseIdString);
                        if (FAILED(hr))
                                goto next_subkey;

                        Configuration->ConfigurationFlags = configFlags;
                        wcscpy_s(Configuration->SensorAdapter, MAX_PATH, sensorAdapter);
                        wcscpy_s(Configuration->EngineAdapter, MAX_PATH, engineAdapter);
                        wcscpy_s(Configuration->StorageAdapter, MAX_PATH, storageAdapter);

                        RegCloseKey(configKey);
                        break;

                next_subkey:
                        subkeyIndex++;
                        RegCloseKey(configKey);
                        configKey = NULL;


			
		}
		
                RegCloseKey(configListKey);
		configListKey = NULL;

		if (storageArray != NULL)
		{
			WinBioFree(storageArray);
			storageArray = NULL;
			storageCount = 0;
		}

		return hr;
	}

	HRESULT RegisterDatabase(__in WINBIO_STORAGE_SCHEMA* StorageSchema)
	{
		/*
			HKLM\System\CurrentControlSet\Services\WbioSrvc\Databases\{guid} -- NOTE THE CURLY BRACES
				Attributes          - REG_DWORD
				AutoCreate          - REG_DWORD (1)
				AutoName            - REG_DWORD (1)     -- this is reset to zero when the service creates the DB
				BiometricType       - REG_DWORD (8)     -- WINBIO_TYPE_FINGERPRINT
				ConnectionString    - REG_SZ ""
				Filepath            - REG_SZ ""         -- set by service
				Format              - REG_SZ "guid"     -- NOTE: *NO* CURLY BRACES
				InitialSize         - REG_DWORD (32)
		*/
		HRESULT hr = S_OK;

		if (!ARGUMENT_PRESENT(StorageSchema))
			return E_POINTER;
		
		WCHAR databaseKeyName[MAX_PATH] = {};
		if (!ConvertUuidToString(
			&StorageSchema->DatabaseId,
			databaseKeyName,
			ARRAYSIZE(databaseKeyName),
			true))
		{
			return E_INVALIDARG;
		}

		WCHAR dataFormat[MAX_PATH] = {};
		if (!ConvertUuidToString(
			&StorageSchema->DataFormat,
			dataFormat,
			ARRAYSIZE(dataFormat),
			false))
		{
			return E_INVALIDARG;
		}

		HKEY databaseListKey = NULL;
		hr = HRESULT_FROM_WIN32(
			RegOpenKeyExW(
				HKEY_LOCAL_MACHINE,
				L"System\\CurrentControlSet\\Services\\WbioSrvc\\Databases",
				0,
				KEY_WRITE,
				&databaseListKey
			));
		if (FAILED(hr))
			return hr;
		
		HKEY newDatabaseKey = NULL;
		DWORD keyDisposition = 0;
		hr = HRESULT_FROM_WIN32(
			RegCreateKeyExW(
				databaseListKey,
				databaseKeyName,
				0,
				NULL,
				REG_OPTION_NON_VOLATILE,
				KEY_WRITE,
				NULL,
				&newDatabaseKey,
				&keyDisposition
			));
		if (SUCCEEDED(hr))
		{
			if (keyDisposition == REG_OPENED_EXISTING_KEY)
			{
				hr = WINBIO_E_DATABASE_ALREADY_EXISTS;
			}

			if (SUCCEEDED(hr))
			{
				hr = HRESULT_FROM_WIN32(
					RegSetValueExW(
						newDatabaseKey,
						L"Attributes",
						0,
						REG_DWORD,
						(LPBYTE)&StorageSchema->Attributes,
						sizeof(StorageSchema->Attributes)
					));
			}

			if (SUCCEEDED(hr))
			{
				DWORD autoCreate = 1;
				hr = HRESULT_FROM_WIN32(
					RegSetValueExW(
						newDatabaseKey,
						L"AutoCreate",
						0,
						REG_DWORD,
						(LPBYTE)&autoCreate,
						sizeof(autoCreate)
					));
			}

			if (SUCCEEDED(hr))
			{
				DWORD autoName = 1;
				hr = HRESULT_FROM_WIN32(
					RegSetValueExW(
						newDatabaseKey,
						L"AutoName",
						0,
						REG_DWORD,
						(LPBYTE)&autoName,
						sizeof(autoName)
					));
			}

			if (SUCCEEDED(hr))
			{
				WINBIO_BIOMETRIC_TYPE biometricType = WINBIO_TYPE_FINGERPRINT;
				hr = HRESULT_FROM_WIN32(
					RegSetValueExW(
						newDatabaseKey,
						L"BiometricType",
						0,
						REG_DWORD,
						(LPBYTE)&biometricType,
						sizeof(biometricType)
					));
			}

			if (SUCCEEDED(hr))
			{
				hr = HRESULT_FROM_WIN32(
					RegSetValueExW(
						newDatabaseKey,
						L"ConnectionString",
						0,
						REG_SZ,
						(LPBYTE)L"",
						sizeof(WCHAR)
					));
			}

			if (SUCCEEDED(hr))
			{
				hr = HRESULT_FROM_WIN32(
					RegSetValueExW(
						newDatabaseKey,
						L"FilePath",
						0,
						REG_SZ,
						(LPBYTE)L"",
						sizeof(WCHAR)
					));
			}

			if (SUCCEEDED(hr))
			{
				hr = HRESULT_FROM_WIN32(
					RegSetValueExW(
						newDatabaseKey,
						L"Format",
						0,
						REG_SZ,
						(LPBYTE)dataFormat,
						(DWORD)((wcsnlen_s(
							dataFormat,
							ARRAYSIZE(dataFormat)) + 1) * sizeof(WCHAR))
					));
			}

			if (SUCCEEDED(hr))
			{
				DWORD initialSize = 32;
				hr = HRESULT_FROM_WIN32(
					RegSetValueExW(
						newDatabaseKey,
						L"InitialSize",
						0,
						REG_DWORD,
						(LPBYTE)&initialSize,
						sizeof(initialSize)
					));
			}

			RegCloseKey(newDatabaseKey);
			newDatabaseKey = NULL;
		}

		RegCloseKey(databaseListKey);
		databaseListKey = NULL;
		return hr;
	}

	HRESULT UnregisterDatabase(__in WINBIO_UUID *DatabaseId)
	{
		HRESULT hr = S_OK;

		if (!ARGUMENT_PRESENT(DatabaseId))
		{
			return E_POINTER;
		}

		WCHAR databaseKeyName[MAX_PATH] = {};
		if (!ConvertUuidToString(
			DatabaseId,
			databaseKeyName,
			ARRAYSIZE(databaseKeyName),
			true
		))
		{
			return E_INVALIDARG;
		}

		WINBIO_STORAGE_SCHEMA *storageArray = NULL;
		SIZE_T storageCount = 0;
		hr = WinBioEnumDatabases(WINBIO_TYPE_FINGERPRINT, &storageArray, &storageCount);
		if (SUCCEEDED(hr))
		{
			WINBIO_STORAGE_SCHEMA *storageSchema = NULL;
			for (SIZE_T i = 0; i < storageCount; ++i)
			{
				if (storageArray[i].DatabaseId == *DatabaseId)
				{
					storageSchema = &storageArray[i];
					break;
				}
			}

			if (storageSchema == NULL)
			{
				hr = WINBIO_E_DATABASE_CANT_FIND;
			}
			else
			{
				HKEY databaseListKey = NULL;
				hr = HRESULT_FROM_WIN32(
					RegOpenKeyExW(
						HKEY_LOCAL_MACHINE,
						L"System\\CurrentControlSet\\Services\\WbioSrvc\\Databases",
						0,
						KEY_WRITE,
						&databaseListKey
					));
				if (SUCCEEDED(hr))
				{
					hr = HRESULT_FROM_WIN32(
						RegDeleteKeyExW(
							databaseListKey,
							databaseKeyName,
							KEY_WOW64_64KEY,
							0
						));
					if (SUCCEEDED(hr) &&
						wcsnlen_s(storageSchema->FilePath, ARRAYSIZE(storageSchema->FilePath)) > 0)
					{
						// delete the database file
						if (!DeleteFileW(storageSchema->FilePath))
						{
							hr = HRESULT_FROM_WIN32(GetLastError());
						}
					}

					RegCloseKey(databaseListKey);
					databaseListKey = NULL;
				}
			}

			WinBioFree(storageArray);
			storageArray = NULL;
			storageCount = 0;
		}

		return hr;
	}

	HRESULT RegisterPrivateConfiguration(
                __in WINBIO_UNIT_SCHEMA* UnitSchema,
                __in POOL_CONFIGURATION* Configuration)
	{
		HRESULT hr = S_OK;

		if (!ARGUMENT_PRESENT(UnitSchema) ||
			!ARGUMENT_PRESENT(Configuration))
		{
			return E_POINTER;
		}

                DWORD sensorMode = get_sensor_mode_from_config_flag(
                        Configuration->ConfigurationFlags);

		WCHAR databaseId[MAX_PATH];
		if (!ConvertUuidToString(
			&Configuration->DatabaseId,
			databaseId,
			ARRAYSIZE(databaseId),
			false))
		{
			return E_INVALIDARG;
		}

		std::wstring regPath = L"System\\CurrentControlSet\\Enum\\";
		regPath += UnitSchema->DeviceInstanceId;
		regPath += L"\\Device Parameters\\WinBio\\Configurations";

		HKEY configListKey = NULL;
		LONG regStatus = RegOpenKeyExW(
			HKEY_LOCAL_MACHINE,
			regPath.c_str(),
			0,
			KEY_READ | KEY_WRITE,
			&configListKey
		);
		if (regStatus != ERROR_SUCCESS)
		{
			return HRESULT_FROM_WIN32(regStatus);
		}

		LONG highestConfigKeyValue = -1;
		DWORD subkeyIndex = 0;
		for (;;)
		{
			hr = S_OK;

			WCHAR subkeyName[MAX_PATH] = {};
			DWORD subkeyNameLength = ARRAYSIZE(subkeyName);
			regStatus = RegEnumKeyExW(
				configListKey,
				subkeyIndex,
				(LPWSTR)&subkeyName,
				&subkeyNameLength,
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (regStatus != ERROR_SUCCESS)
			{
				if (regStatus == ERROR_NO_MORE_ITEMS)
				{
					hr = S_OK;
				}
				else
				{
					hr = HRESULT_FROM_WIN32(regStatus);
				}
				break;
			}

			if (IsKeyNameNumeric(subkeyName, subkeyNameLength))
			{
				// See if the config we're trying to register 
				// is already registered for this sensor
				bool collision = false;
				hr = CompareConfiguration(
					configListKey,
					subkeyName,
					Configuration,
					&collision
				);
				if (SUCCEEDED(hr) && collision)
				{
					hr = WINBIO_E_CONFIGURATION_FAILURE;
				}
				if (FAILED(hr))
				{
					break;
				}

				// Convert key name to number and see if 
				// it's bigger than the highest one we've
				// seen so far; if so, keep it
				LONG thisKey = _wtoi(subkeyName);
				highestConfigKeyValue = max(thisKey, highestConfigKeyValue);
			}
			++subkeyIndex;
		}

		if (SUCCEEDED(hr))
		{
			WCHAR newConfigKeyName[20] = {};
			_itow_s((highestConfigKeyValue + 1), newConfigKeyName, ARRAYSIZE(newConfigKeyName), 10);

			HKEY newConfigKey = NULL;
			DWORD keyDisposition = 0;
			hr = HRESULT_FROM_WIN32(
				RegCreateKeyExW(
					configListKey,
					newConfigKeyName,
					0,
					NULL,
					REG_OPTION_NON_VOLATILE,
					KEY_WRITE,
					NULL,
					&newConfigKey,
					&keyDisposition
				));
			if (SUCCEEDED(hr))
			{
				/*
					Create values for this configuration
						SensorMode              - REG_DWORD
						SystemSensor            - REG_DWORD (always zero for private configs)
						DatabaseId              - REG_SZ
						SensorAdapterBinary     - REG_SZ
						EngineAdapterBinary     - REG_SZ
						StorageAdapterBinary    - REG_SZ
				*/
				hr = HRESULT_FROM_WIN32(
					RegSetValueExW(
						newConfigKey,
						L"SensorMode",
						0,
						REG_DWORD,
						(LPBYTE)&sensorMode,
						sizeof(sensorMode)
					));

				if (SUCCEEDED(hr))
				{
					DWORD sytemSensor = 1;
					hr = HRESULT_FROM_WIN32(
						RegSetValueExW(
							newConfigKey,
							L"SystemSensor",
							0,
							REG_DWORD,
							(LPBYTE)&sytemSensor,
							sizeof(sytemSensor)
						));
				}

				if (SUCCEEDED(hr))
				{
					hr = HRESULT_FROM_WIN32(
						RegSetValueExW(
							newConfigKey,
							L"DatabaseId",
							0,
							REG_SZ,
							(LPBYTE)databaseId,
							(DWORD)((wcsnlen_s(
								databaseId,
								ARRAYSIZE(databaseId)) + 1) * sizeof(WCHAR))
						));
				}

				if (SUCCEEDED(hr))
				{
					hr = HRESULT_FROM_WIN32(
						RegSetValueExW(
							newConfigKey,
							L"SensorAdapterBinary",
							0,
							REG_SZ,
							(LPBYTE)Configuration->SensorAdapter,
							(DWORD)((wcsnlen_s(
								Configuration->SensorAdapter,
								ARRAYSIZE(Configuration->SensorAdapter)) + 1) * sizeof(WCHAR))
						));
				}

				if (SUCCEEDED(hr))
				{
					hr = HRESULT_FROM_WIN32(
						RegSetValueExW(
							newConfigKey,
							L"EngineAdapterBinary",
							0,
							REG_SZ,
							(LPBYTE)Configuration->EngineAdapter,
							(DWORD)((wcsnlen_s(
								Configuration->EngineAdapter,
								ARRAYSIZE(Configuration->EngineAdapter)) + 1) * sizeof(WCHAR))
						));
				}

				if (SUCCEEDED(hr))
				{
					hr = HRESULT_FROM_WIN32(
						RegSetValueExW(
							newConfigKey,
							L"StorageAdapterBinary",
							0,
							REG_SZ,
							(LPBYTE)Configuration->StorageAdapter,
							(DWORD)((wcsnlen_s(
								Configuration->StorageAdapter,
								ARRAYSIZE(Configuration->StorageAdapter)) + 1) * sizeof(WCHAR))
						));
				}

				RegCloseKey(newConfigKey);
				newConfigKey = NULL;
			}
		}

		RegCloseKey(configListKey);
		configListKey = NULL;

		return hr;
	}

	HRESULT UnregisterPrivateConfiguration(
                __in WINBIO_UNIT_SCHEMA* UnitSchema,
                __in WINBIO_UUID *DatabaseId,
                __out bool *ConfigurationRemoved)
	{
		HRESULT hr = S_OK;

		if (!ARGUMENT_PRESENT(UnitSchema) ||
			!ARGUMENT_PRESENT(DatabaseId))
		{
			return E_POINTER;
		}

		WCHAR targetDatabaseId[40];
		if (!ConvertUuidToString(
			DatabaseId,
			targetDatabaseId,
			ARRAYSIZE(targetDatabaseId),
			false
		))
		{
			return E_INVALIDARG;
		}

		std::wstring regPath = L"System\\CurrentControlSet\\Enum\\";
		regPath += UnitSchema->DeviceInstanceId;
		regPath += L"\\Device Parameters\\WinBio\\Configurations";

		HKEY configListKey = NULL;
		hr = HRESULT_FROM_WIN32(
			RegOpenKeyExW(
				HKEY_LOCAL_MACHINE,
				regPath.c_str(),
				0,
				KEY_READ | KEY_WRITE,
				&configListKey
			));
		if (FAILED(hr))
		{
			return hr;
		}

		bool configurationRemoved = false;
		DWORD subkeyIndex = 0;
		for (;;)
		{
			hr = S_OK;

			WCHAR configKeyName[MAX_PATH] = {};
			DWORD configKeyNameLength = ARRAYSIZE(configKeyName);
			LONG regStatus = RegEnumKeyExW(
				configListKey,
				subkeyIndex,
				(LPWSTR)&configKeyName,
				&configKeyNameLength,
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (regStatus != ERROR_SUCCESS)
			{
				if (regStatus == ERROR_NO_MORE_ITEMS)
				{
					hr = S_OK;
				}
				else
				{
					hr = HRESULT_FROM_WIN32(regStatus);
				}
				break;
			}

			if (IsKeyNameNumeric(configKeyName, configKeyNameLength))
			{
				WCHAR configDatabaseId[40] = {};
				DWORD dataSize = sizeof(configDatabaseId);
				hr = HRESULT_FROM_WIN32(
					RegGetValueW(
						configListKey,
						configKeyName,
						L"DatabaseId",
						RRF_RT_REG_SZ,
						NULL,
						&configDatabaseId,
						&dataSize
					));
				if (SUCCEEDED(hr) &&
					_wcsnicmp(configDatabaseId, targetDatabaseId, ARRAYSIZE(configDatabaseId)) == 0)
				{
					hr = HRESULT_FROM_WIN32(
						RegDeleteKeyExW(
							configListKey,
							configKeyName,
							KEY_WOW64_64KEY,
							0
						));
					if (SUCCEEDED(hr))
					{
						configurationRemoved = true;
					}
				}
			}
			if (SUCCEEDED(hr))
			{
				++subkeyIndex;
			}
			else
			{
				break;
			}
		}

		RegCloseKey(configListKey);
		configListKey = NULL;
		*ConfigurationRemoved = configurationRemoved;
		return hr;
	}

	static HRESULT CompareConfiguration(
                __in HKEY SourceConfigList,
                __in LPWSTR SourceConfigKey,
                __in POOL_CONFIGURATION* TargetConfig,
                __out bool *IsEqual)
	{

		if (SourceConfigList == NULL)
		{
			return E_INVALIDARG;
		}

		if (!ARGUMENT_PRESENT(SourceConfigKey) ||
			!ARGUMENT_PRESENT(TargetConfig) ||
			!ARGUMENT_PRESENT(IsEqual))
		{
			return E_POINTER;
		}

		WCHAR targetDatabaseId[40];
		if (!ConvertUuidToString(
			&TargetConfig->DatabaseId,
			targetDatabaseId,
			ARRAYSIZE(targetDatabaseId),
			false
		))
		{
			return E_INVALIDARG;
		}

		HKEY srcConfig = NULL;
		HRESULT hr = HRESULT_FROM_WIN32(
			RegOpenKeyExW(
				SourceConfigList,
				SourceConfigKey,
				0,
				KEY_READ,
				&srcConfig
			));
		if (SUCCEEDED(hr))
		{
			bool isEqual = true;

			WCHAR configDatabaseId[40] = {};
			DWORD dataSize = sizeof(configDatabaseId);
			hr = HRESULT_FROM_WIN32(
				RegGetValueW(
					srcConfig,
					NULL,
					L"DatabaseId",
					RRF_RT_REG_SZ,
					NULL,
					&configDatabaseId,
					&dataSize
				));
			if (SUCCEEDED(hr) &&
				_wcsnicmp(configDatabaseId, targetDatabaseId, ARRAYSIZE(configDatabaseId)) != 0)
			{
				isEqual = false;
			}

			RegCloseKey(srcConfig);
			srcConfig = NULL;

			if (SUCCEEDED(hr))
			{
				*IsEqual = isEqual;
			}
		}
		return hr;
	}

	static bool ConvertStringToUuid(
                __in LPWSTR UuidString,
                __out WINBIO_UUID *Uuid)
	{

		HRESULT hr = UuidToStringW(Uuid, (RPC_WSTR *)UuidString);
		if (FAILED(hr))
			return false;

		return true;
	}

	static bool ConvertUuidToString(
                __in WINBIO_UUID *Uuid,
                __out LPWSTR UuidStringBuffer,
                __in SIZE_T UuidStringBufferLength,
                __in bool IncludeBraces)
	{
		
		HRESULT hr = StringFromGUID2(*Uuid, UuidStringBuffer, UuidStringBufferLength);

		if (FAILED(hr))
			return false;

                if (!IncludeBraces) {
                        int i = 0;
                        for (i = 0; UuidStringBuffer[i + 1] != '}'; i++)
                                UuidStringBuffer[i] = UuidStringBuffer[i + 1];
                        UuidStringBuffer[i + 1] = 0;
                }
                        
		return true;
	}


	void ConvertErrorCodeToString(__in HRESULT ErrorCode, LPWSTR buffer, SIZE_T length)
	{
		DWORD messageLength = 0;

                HMODULE winbio_dll = LoadLibrary(L"winbio.dll");

                memset(buffer, 0, length);

                messageLength = FormatMessage(
                        FORMAT_MESSAGE_FROM_HMODULE| FORMAT_MESSAGE_FROM_SYSTEM,
			winbio_dll, 
                        ErrorCode, 0,
			buffer, length, NULL);
			
                if (messageLength > 0)
			buffer[messageLength] = L'\0';
		
                if (buffer == NULL)
			wsprintf(buffer, L"0x%08x", ErrorCode);	
                
                FreeLibrary(winbio_dll);
	}

	LPCTSTR ConvertSubFactorToString(__in WINBIO_BIOMETRIC_SUBTYPE SubFactor)
	{
		SIZE_T index = 0;
		for (index = 0; index < k_SubFactorTextTableSize; ++index)
		{
			if (g_SubFactorText[index].SubFactor == SubFactor)
			{
				return g_SubFactorText[index].Text;
			}
		}
		return L"<Unknown>";
	}

	LPCTSTR ConvertRejectDetailToString(__in WINBIO_REJECT_DETAIL RejectDetail)
	{
		SIZE_T index = 0;
		for (index = 0; index < k_RejectDetailTextTableSize; ++index)
		{
			if (g_RejectDetailText[index].RejectDetail == RejectDetail)
			{
				return g_RejectDetailText[index].Text;
			}
		}
		return L"Reason for failure couldn't be diagnosed.";
		}

}; 

