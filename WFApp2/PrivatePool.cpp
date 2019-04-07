/******************************************************************************
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) Microsoft.  All rights reserved.

This source code is only intended as a supplement to Microsoft Development
Tools and/or WinHelp documentation.  See these sources for detailed information
regarding the Microsoft samples programs.
******************************************************************************/


#include "PrivatePool.h"   // From Private Pool Setup
#include "Strsafe.h"                 // For StringCbGets()

#include <vector>

namespace PrivatePool {

        void onIdentifyOrDelete(__in bool DeleteEnrollment);

        void displayIdentity(
                __in PWINBIO_IDENTITY Identity,
                __in WINBIO_BIOMETRIC_SUBTYPE SubFactor
        );

        void onEnrollOrPractice(__in bool CommitEnrollment);
        bool selectSubFactorMenu(__out PWINBIO_BIOMETRIC_SUBTYPE SubFactor);
        bool getUlongValue(__out PULONG Value);

        HRESULT install_private_storage(PWINBIO_UNIT_SCHEMA unit_array, SIZE_T selected_unit)
        {
                WINBIO_UUID databaseId = PRIVATE_POOL_DATABASE_ID;

                if (is_private_storage_installed(&databaseId))
                        return S_OK;

                HRESULT hr;

                BioHelper::POOL_CONFIGURATION derivedConfig = {};
                hr = BioHelper::CreateCompatibleConfiguration(
                        &unit_array[selected_unit],
                        &derivedConfig);
                if (FAILED(hr))
                        return hr;

                WINBIO_STORAGE_SCHEMA storageSchema = {};
                storageSchema.DatabaseId = PRIVATE_POOL_DATABASE_ID;
                storageSchema.DataFormat = derivedConfig.DataFormat;
                storageSchema.Attributes = derivedConfig.DatabaseAttributes;
                hr = BioHelper::RegisterDatabase(&storageSchema);
                
                return hr;
        }

        HRESULT uninstall_private_storage(PWINBIO_UNIT_SCHEMA unit_array, SIZE_T selected_unit)
        {
                WINBIO_UUID databaseId = PRIVATE_POOL_DATABASE_ID;

                if (!is_private_storage_installed(&databaseId))
                        return S_OK;

                HRESULT hr;
                
                bool configRemoved = false;
                hr = BioHelper::UnregisterPrivateConfiguration(
                        &unit_array[selected_unit],
                        &databaseId, &configRemoved);
                
                if (SUCCEEDED(hr))
                        hr = BioHelper::UnregisterDatabase(&databaseId);
                
                return hr;
        }

        HRESULT add_unit_to_private_storage(PWINBIO_UNIT_SCHEMA unit_array, SIZE_T selected_unit)
        {
                HRESULT hr;
                WINBIO_UUID databaseId = PRIVATE_POOL_DATABASE_ID;

                if (!is_private_storage_installed(&databaseId))
                        return E_INVALIDARG;

                BioHelper::POOL_CONFIGURATION derivedConfig = {};
                hr = BioHelper::CreateCompatibleConfiguration(
                        &unit_array[selected_unit],
                        &derivedConfig
                );
                if (SUCCEEDED(hr))
                {
                        derivedConfig.DatabaseId = PRIVATE_POOL_DATABASE_ID;
                        hr = BioHelper::RegisterPrivateConfiguration(
                                &unit_array[selected_unit],
                                &derivedConfig
                        );
                }
                        
                return hr;
        }

        HRESULT remove_unit_from_private_storage(PWINBIO_UNIT_SCHEMA unit_array, SIZE_T selected_unit)
        {
                bool configRemoved = false;
                HRESULT hr;

                SIZE_T selectedUnit = 0;
                WINBIO_UUID targetDatabase = PRIVATE_POOL_DATABASE_ID;
                hr = BioHelper::UnregisterPrivateConfiguration(
                        &unit_array[selected_unit],
                        &targetDatabase,
                        &configRemoved);

                return hr;
        }

        bool is_private_storage_installed(__in WINBIO_UUID *DatabaseId)
        {
                if (DatabaseId == NULL)
                        return false;
                
                bool is_installed = false;
                WINBIO_STORAGE_SCHEMA *storageArray = NULL;
                SIZE_T storageCount = 0;

                HRESULT hr = WinBioEnumDatabases(WINBIO_TYPE_FINGERPRINT, &storageArray, &storageCount);

                if (FAILED(hr))
                        return is_installed;
                
                for (SIZE_T i = 0; i < storageCount; ++i)
                        if (storageArray[i].DatabaseId == *DatabaseId)
                        {
                                is_installed = true;
                                break;
                        }
                
                WinBioFree(storageArray);
                storageArray = NULL;
                storageCount = 0;
                
                return is_installed;
        }

        
        void onIdentifyOrDelete(__in bool DeleteEnrollment)
        {
                WINBIO_UNIT_ID unitIdArray[1] = {};
                SIZE_T unitIdCount = ARRAYSIZE(unitIdArray);

                WINBIO_UNIT_SCHEMA *unitSchemaArray = NULL;
                SIZE_T unitSchemaCount = 0;
                HRESULT hr = WinBioEnumBiometricUnits(
                        WINBIO_TYPE_FINGERPRINT,
                        &unitSchemaArray,
                        &unitSchemaCount
                );
                if (SUCCEEDED(hr))
                {
                        SIZE_T selectedUnit = 0;
                        // Build the array of unit IDs that will make up the private pool
                        unitIdArray[0] = unitSchemaArray[selectedUnit].UnitId;
                        unitIdCount = 1;
                        WinBioFree(unitSchemaArray);
                        unitSchemaArray = NULL;
                        unitSchemaCount = 0;
                }
                if (SUCCEEDED(hr))
                {
                        WINBIO_UUID privateDatabaseId = PRIVATE_POOL_DATABASE_ID;
                        WINBIO_SESSION_HANDLE sessionHandle = NULL;
                        hr = WinBioOpenSession(
                                WINBIO_TYPE_FINGERPRINT,
                                WINBIO_POOL_PRIVATE,
                                WINBIO_FLAG_BASIC,
                                unitIdArray,
                                unitIdCount,
                                &privateDatabaseId,
                                &sessionHandle
                        );
                        if (SUCCEEDED(hr))
                        {
                                wprintf(L"\nIdentify yourself by swiping your finger on the sensor...\n");

                                WINBIO_UNIT_ID unitId = 0;
                                WINBIO_REJECT_DETAIL rejectDetail = 0;
                                WINBIO_BIOMETRIC_SUBTYPE subFactor = WINBIO_SUBTYPE_NO_INFORMATION;
                                WINBIO_IDENTITY identity = {};

                                hr = WinBioIdentify(
                                        sessionHandle,
                                        &unitId,
                                        &identity,
                                        &subFactor,
                                        &rejectDetail
                                );
                                wprintf(L"\n- Swipe processed - Unit ID: %d\n", unitId);
                                if (FAILED(hr))
                                {
                                        if (hr == WINBIO_E_UNKNOWN_ID)
                                        {
                                                wprintf(L"\n- Unknown identity.\n");
                                                hr = S_OK;
                                        }
                                        else if (hr == WINBIO_E_BAD_CAPTURE)
                                        {
                                                wprintf(L"\n- Bad capture.\n");
                                                wprintf(
                                                        L"- %s\n",
                                                        BioHelper::ConvertRejectDetailToString(rejectDetail)
                                                );
                                                hr = S_OK;
                                        }
                                }
                                else
                                {
                                        // Display identity
                                        displayIdentity(&identity, subFactor);

                                        // Delete identity
                                        if (DeleteEnrollment)
                                        {
                                                wprintf(L"\nDeleting template...\n");
                                                hr = WinBioDeleteTemplate(
                                                        sessionHandle,
                                                        unitId,
                                                        &identity,
                                                        subFactor
                                                );
                                                if (SUCCEEDED(hr))
                                                {
                                                        wprintf(L"\n- Template removed from private database.\n");
                                                }
                                        }
                                }
                                WinBioCloseSession(sessionHandle);
                                sessionHandle = NULL;
                        }
                }

                // to-do:  return hr here


        }


        /*
        int _tmain(int argc, _TCHAR* argv[])
        {
                wprintf(L"\nWinBio Private Pool Enrollment\n\n"));

                if (argc < 2)
                {
                        wprintf(L"Usage: %s -enroll | -practice\n"), argv[0]);
                        wprintf(
                                L"\n")
                                L"  -enroll       create enrollment for a new finger\n")
                                L"  -practice     perform enrollment, but discard results\n")
                                L"\n")
                        );
                }
                else
                {
                        if (_tcsicmp(argv[1], L"-enroll")) == 0)
                        {
                                onEnrollOrPractice(true);
                        }
                        else if (_tcsicmp(argv[1], L"-practice")) == 0)
                        {
                                onEnrollOrPractice(false);
                        }
                        else
                        {
                                wprintf(L"*** Error - Unknown command option: %s\n"), argv[1]);
                        }
                }

                return 0;
        }
        */

        void onEnrollOrPractice(
                __in bool CommitEnrollment
        )
        {
                WINBIO_UNIT_ID unitIdArray[1] = {};
                SIZE_T unitIdCount = ARRAYSIZE(unitIdArray);

                WINBIO_UNIT_SCHEMA *unitSchemaArray = NULL;
                SIZE_T unitSchemaCount = 0;
                HRESULT hr = WinBioEnumBiometricUnits(
                        WINBIO_TYPE_FINGERPRINT,
                        &unitSchemaArray,
                        &unitSchemaCount
                );
                if (SUCCEEDED(hr))
                {
                        SIZE_T selectedUnit = 0;

                        // Build the array of Unit IDs that will make
                        // up the private pool
                        unitIdArray[0] = unitSchemaArray[selectedUnit].UnitId;
                        unitIdCount = 1;

                        WinBioFree(unitSchemaArray);
                        unitSchemaArray = NULL;
                        unitSchemaCount = 0;
                }
                if (SUCCEEDED(hr))
                {
                        WINBIO_UUID privateDatabaseId = PRIVATE_POOL_DATABASE_ID;
                        WINBIO_SESSION_HANDLE sessionHandle = NULL;
                        hr = WinBioOpenSession(
                                WINBIO_TYPE_FINGERPRINT,
                                WINBIO_POOL_PRIVATE,
                                WINBIO_FLAG_BASIC,
                                unitIdArray,
                                unitIdCount,
                                &privateDatabaseId,
                                &sessionHandle
                        );
                        if (SUCCEEDED(hr))
                        {
                                WINBIO_BIOMETRIC_SUBTYPE subFactor = WINBIO_SUBTYPE_NO_INFORMATION;
                                if (selectSubFactorMenu(&subFactor) &&
                                        subFactor != WINBIO_SUBTYPE_NO_INFORMATION &&
                                        subFactor != WINBIO_SUBTYPE_ANY)
                                {
                                        //
                                        // Locate sensor
                                        //
                                        wprintf(L"\nTap the sensor once when you're ready to begin enrolling...\n\n");
                                        WINBIO_UNIT_ID unitId = 0;
                                        hr = WinBioLocateSensor(sessionHandle, &unitId);
                                        if (SUCCEEDED(hr))
                                        {
                                                //
                                                // Enroll begin
                                                //
                                                wprintf(L"\n(Begining enrollment sequence)\n\n");
                                                hr = WinBioEnrollBegin(
                                                        sessionHandle,
                                                        subFactor,
                                                        unitId
                                                );
                                                if (SUCCEEDED(hr))
                                                {
                                                        SIZE_T swipeCount = 0;
                                                        for (swipeCount = 1;; ++swipeCount)
                                                        {
                                                                wprintf(
                                                                        L"Swipe your finger on the sensor to capture %s sample.\n",
                                                                        (swipeCount == 1) ? L"the first" : L"another"
                                                                );

                                                                WINBIO_REJECT_DETAIL rejectDetail = 0;
                                                                hr = WinBioEnrollCapture(
                                                                        sessionHandle,
                                                                        &rejectDetail
                                                                );
                                                                wprintf(L"   Sample %d captured from unit number %d.\n", swipeCount, unitId);
                                                                if (hr == WINBIO_I_MORE_DATA)
                                                                {
                                                                        wprintf(L"   More data required.\n\n");
                                                                        continue;
                                                                }
                                                                if (FAILED(hr))
                                                                {
                                                                        if (hr == WINBIO_E_BAD_CAPTURE)
                                                                        {
                                                                                wprintf(L"*** Error - Bad capture.\n");
                                                                                wprintf(
                                                                                        L"- %s\n",
                                                                                        BioHelper::ConvertRejectDetailToString(rejectDetail)
                                                                                );
                                                                                hr = S_OK;
                                                                                continue;
                                                                        }
                                                                        else
                                                                        {
                                                                                break;
                                                                        }
                                                                }
                                                                else
                                                                {
                                                                        wprintf(L"   Template completed.\n\n");
                                                                        break;
                                                                }
                                                        }
                                                        wprintf(L"\n");
                                                        if (SUCCEEDED(hr))
                                                        {
                                                                if (CommitEnrollment)
                                                                {
                                                                        // ENROLL
                                                                        WINBIO_IDENTITY identity = {};
                                                                        BOOLEAN isNewTemplate = FALSE;

                                                                        wprintf(L"Committing enrollment...\n");
                                                                        hr = WinBioEnrollCommit(sessionHandle, &identity, &isNewTemplate);
                                                                        if (SUCCEEDED(hr))
                                                                        {
                                                                                wprintf(L"Enrollment committed to database\n");
                                                                                displayIdentity(&identity, subFactor);
                                                                        }
                                                                }
                                                                else
                                                                {
                                                                        // PRACTICE
                                                                        wprintf(L"Discarding enrollment...\n");
                                                                        hr = WinBioEnrollDiscard(sessionHandle);
                                                                        if (SUCCEEDED(hr))
                                                                        {
                                                                                wprintf(L"Template discarded.\n");
                                                                        }
                                                                }
                                                        }
                                                }
                                        }
                                }
                                else
                                {
                                        hr = E_INVALIDARG;
                                }

                                WinBioCloseSession(sessionHandle);
                                sessionHandle = NULL;
                        }
                }

                // to-do: return hr here
        }

        bool selectSubFactorMenu(
                __out PWINBIO_BIOMETRIC_SUBTYPE SubFactor
        )
        {
                static const WINBIO_BIOMETRIC_SUBTYPE subFactors[] = {
                        WINBIO_ANSI_381_POS_RH_THUMB,
                        WINBIO_ANSI_381_POS_RH_INDEX_FINGER,
                        WINBIO_ANSI_381_POS_RH_MIDDLE_FINGER,
                        WINBIO_ANSI_381_POS_RH_RING_FINGER,
                        WINBIO_ANSI_381_POS_RH_LITTLE_FINGER,
                        WINBIO_ANSI_381_POS_LH_THUMB,
                        WINBIO_ANSI_381_POS_LH_INDEX_FINGER,
                        WINBIO_ANSI_381_POS_LH_MIDDLE_FINGER,
                        WINBIO_ANSI_381_POS_LH_RING_FINGER,
                        WINBIO_ANSI_381_POS_LH_LITTLE_FINGER,
                };
                wprintf(L"\n- Select a sub-factor:\n\n");

                for (SIZE_T index = 0; index < ARRAYSIZE(subFactors); ++index)
                {
                        wprintf(
                                L"    %2d - %s\n",
                                (index + 1),
                                BioHelper::ConvertSubFactorToString(subFactors[index])
                        );
                }

                ULONG chosen = 0;
                if (getUlongValue(&chosen) &&
                        (chosen - 1) < ARRAYSIZE(subFactors))
                {
                        *SubFactor = subFactors[chosen - 1];
                        return true;
                }
                return false;
        }

        bool getUlongValue(
                __out PULONG Value
        )
        {
                ULONG value = 1L;

                *Value = value;
                return true;

        }

        void displayIdentity(
                __in PWINBIO_IDENTITY Identity,
                __in WINBIO_BIOMETRIC_SUBTYPE SubFactor
        )
        {
                wprintf(L"\n- Identity: ");
                switch (Identity->Type)
                {
                case WINBIO_ID_TYPE_NULL:
                        wprintf(L"NULL value\n");
                        break;

                case WINBIO_ID_TYPE_WILDCARD:
                        wprintf(L"WILDCARD value\n");
                        if (Identity->Value.Wildcard != WINBIO_IDENTITY_WILDCARD)
                        {
                                wprintf(
                                        L"\n*** Error: Invalid wildcard marker (0x%08x)\n",
                                        Identity->Value.Wildcard
                                );
                        }
                        break;

                case WINBIO_ID_TYPE_GUID:
                        wprintf(L"GUID\n");
                        wprintf(
                                L"    Value:      {%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}\n",
                                Identity->Value.TemplateGuid.Data1,
                                Identity->Value.TemplateGuid.Data2,
                                Identity->Value.TemplateGuid.Data3,
                                Identity->Value.TemplateGuid.Data4[0],
                                Identity->Value.TemplateGuid.Data4[1],
                                Identity->Value.TemplateGuid.Data4[2],
                                Identity->Value.TemplateGuid.Data4[3],
                                Identity->Value.TemplateGuid.Data4[4],
                                Identity->Value.TemplateGuid.Data4[5],
                                Identity->Value.TemplateGuid.Data4[6],
                                Identity->Value.TemplateGuid.Data4[7]
                        );
                        break;

                default:
                        wprintf(L"(Invalid type)\n");
                        // invalid type
                        break;
                }
                wprintf(
                        L"    Subfactor:  %s\n",
                        BioHelper::ConvertSubFactorToString(SubFactor)
                );
        }

}
