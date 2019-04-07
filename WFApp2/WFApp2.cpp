// WFApp2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "PrivatePool.h"

#include <iostream>

HRESULT enum_bsp(PWINBIO_BSP_SCHEMA *bsp_schemas);
HRESULT enum_units(PWINBIO_UNIT_SCHEMA *unit_schema);
HRESULT enum_storages(PWINBIO_STORAGE_SCHEMA *storage_schemas);
HRESULT open_session(WINBIO_UNIT_ID unit_id, bool is_private, PWINBIO_SESSION_HANDLE session_handle);
HRESULT create_and_configure_private(PWINBIO_UNIT_SCHEMA unit_schemas);
HRESULT delete_private(PWINBIO_UNIT_SCHEMA unit_schemas);

void capture_sample(WINBIO_SESSION_HANDLE session_handle, WINBIO_UNIT_ID unit_id);
void print_bio_service_providers();
void bio_callback_fn(PWINBIO_ASYNC_RESULT async_result);
void free_bio_address(PVOID address);
void print_error(HRESULT result_handle);
void print_guid(GUID guid);

typedef enum arg_verify {
        ARG_VERIFY_EQUAL, 
        ARG_VERIFY_START_WITH, 
        ARG_VERIFY_INCLUDES
} enum_arg_verify;

bool verify_arg(int argc, char **argv, const char *arg, enum_arg_verify method = ARG_VERIFY_EQUAL);

int main(int argc, char **argv)
{
	HRESULT hr;
	PWINBIO_UNIT_SCHEMA unit_schemas = NULL;
	PWINBIO_STORAGE_SCHEMA storage_schemas = NULL;
	WINBIO_SESSION_HANDLE session_handle = NULL;
        BioHelper::POOL_CONFIGURATION configs;
	
        if (verify_arg(argc, argv, "-p"))
                print_bio_service_providers();
                
        std::wcout << L"checking units..." << std::endl;
        hr = enum_units(&unit_schemas);
        if (FAILED(hr)) {
                print_error(hr);
                goto cleanup_and_exit;
        }

        if (verify_arg(argc, argv, "-c")) {
                hr = create_and_configure_private(unit_schemas);
                if (FAILED(hr))
                        print_error(hr);

                goto cleanup_and_exit;
        }
        
        if (verify_arg(argc, argv, "-d")) {
                hr = delete_private(unit_schemas);
                if (FAILED(hr))
                        print_error(hr);

                goto cleanup_and_exit;
        }

        std::wcout << L"checking storages..." << std::endl;
        hr = enum_storages(&storage_schemas);
        if (FAILED(hr)) {
                print_error(hr);
                goto cleanup_and_exit;
        }

        if (!verify_arg(argc, argv, "-o"))
                goto cleanup_and_exit;
        
        std::wcout << L"opening session..." << std::endl;
        hr = open_session(unit_schemas->UnitId, verify_arg(argc, argv, "-p"), &session_handle);
	if (FAILED(hr)) {
		print_error(hr);
		goto cleanup_and_exit;
	}
        
        capture_sample(session_handle, unit_schemas->UnitId);

        std::wcout << L"closing session..." << std::endl;
	WinBioCloseSession(session_handle);
        
cleanup_and_exit:

	free_bio_address(storage_schemas);
	free_bio_address(unit_schemas);

}

void print_bio_service_providers() {
        HRESULT hr;
        PWINBIO_BSP_SCHEMA bsp_schemas = NULL;

        std::wcout << L"checking providers..." << std::endl;
        hr = enum_bsp(&bsp_schemas);
        if (FAILED(hr))
                print_error(hr);
        else 
                free_bio_address(bsp_schemas);

}

bool verify_arg(int argc, char **argv, const char *arg, enum arg_verify method) {
        // argv => -key1=value1 -key2=value2 ...
       
        if (argc <= 1)
                return false;

        for (int i = 1; i < argc; i++) {
                std::string _arg = argv[i];
                
                switch (method) {
                case ARG_VERIFY_EQUAL:
                        if (_arg.compare(arg) == 0)
                                return true;
                        else
                                return false;

                case ARG_VERIFY_START_WITH:
                        if (_arg.find(arg) == 0)
                                return true;
                        else
                                return false;

                case ARG_VERIFY_INCLUDES:
                        if (_arg.find(arg) >= 0)
                                return true;
                        else
                                return false;

                }
                
        }

        return false;
}

void capture_sample(WINBIO_SESSION_HANDLE session_handle, WINBIO_UNIT_ID unit_id) {

        /*

        Be aware that some fingerprint sensor vendor don't support the StandardDataBlock. If your sensor is one of those, then you won't be able to recover the image data, unless the vendor has published their data format somewhere.

If you DO have a StandardDataBlock, this is how to get pointers to the relevant blocks from the offsets:

    PWINBIO_BIR_HEADER BirHeader = (PWINBIO_BIR_HEADER)(((PBYTE)sample) + sample->HeaderBlock.Offset);
    PWINBIO_BDB_ANSI_381_HEADER AnsiBdbHeader = (PWINBIO_BDB_ANSI_381_HEADER)(((PBYTE)sample) + sample->StandardDataBlock.Offset);
    PWINBIO_BDB_ANSI_381_RECORD AnsiBdbRecord = (PWINBIO_BDB_ANSI_381_RECORD)(((PBYTE)AnsiBdbHeader) + sizeof(WINBIO_BDB_ANSI_381_HEADER));

    DWORD Width = AnsiBdbRecord->HorizontalLineLength;  // Width of image in pixels
    DWORD Height = AnsiBdbRecord->VerticalLineLength;     // Height of image in pixels

The fingerprint image data is just a grayscale bitmap. Assuming you have a correctly-sized bitmap available, here's how to draw the bitmap. (Note, I'm assuming here that the fingerprint image is 8 bits per pixel. Actually, the pixel width is stored in the AnsiBdbHeader, and a real application would have to use the value found there to calculate the proper X and Y values, based on pixel size.)

    PBYTE firstPixel = (PBYTE)((PBYTE)AnsiBdbRecord) + sizeof(WINBIO_BDB_ANSI_381_RECORD);
    for (DWORD y = 0; y < Height; ++y)
    {
        for (DWORD x = 0; x < Width; ++x)
        {
            BYTE sat = firstPixel[(y * Width) + x];
            SetPixel( hDCMem, x, y, RGB(sat, sat, sat));
        }
    }

        */
        HRESULT hr;
        PWINBIO_BIR bir_sample = NULL;
        SIZE_T bir_sample_size = 0;
        WINBIO_REJECT_DETAIL reject_detail;

        std::wcout << L"capturing sample..." << std::endl;
        hr = WinBioCaptureSample(
                session_handle,
                WINBIO_NO_PURPOSE_AVAILABLE,
                WINBIO_DATA_FLAG_RAW,
                &unit_id,
                &bir_sample,
                &bir_sample_size,
                &reject_detail);
        
        if (FAILED(hr)) {
                print_error(hr);
                return;
        }
        
        PWINBIO_BIR_HEADER bir_header = 
                (PWINBIO_BIR_HEADER)(((PBYTE)bir_sample) + bir_sample->HeaderBlock.Offset);

        USHORT valid_fields_mask = bir_header->ValidFields;
        if (valid_fields_mask & WINBIO_BIR_FIELD_NEVER_VALID) {
                std::wcout << "malformat BIR found." << bir_sample_size << std::endl;
                free_bio_address(bir_sample);
                return;
        }


        std::wcout << "collected sample size is " << bir_sample_size << std::endl;
        free_bio_address(bir_sample);
        
}
HRESULT open_session(WINBIO_UNIT_ID unit_id, bool is_private, PWINBIO_SESSION_HANDLE session_handle) {

	HRESULT hr = 0;
	WCHAR sz_db_id[64];
        const GUID *db_id = 
                is_private ? &PrivatePool::PRIVATE_POOL_DATABASE_ID : WINBIO_DB_DEFAULT;
        WINBIO_POOL_TYPE pool_type =
                is_private ? WINBIO_POOL_PRIVATE : WINBIO_POOL_SYSTEM;
        WINBIO_SESSION_FLAGS session_flag = 
                is_private ? WINBIO_FLAG_ADVANCED | WINBIO_FLAG_RAW : WINBIO_FLAG_DEFAULT | WINBIO_FLAG_RAW;
        
        SIZE_T unit_count = 
                is_private ? 1 : 0;

	hr = WinBioOpenSession(
		WINBIO_TYPE_FINGERPRINT, 
                pool_type, 
		session_flag,
		is_private ? &unit_id : NULL, 
                unit_count, 
                (GUID *)db_id,
                session_handle);
	if (FAILED(hr))
		return hr;


	return S_OK;

}

HRESULT enum_storages(PWINBIO_STORAGE_SCHEMA *storage_schemas) {
	HRESULT hr = 0;
	SIZE_T count = 0;

	hr = WinBioEnumDatabases(WINBIO_TYPE_FINGERPRINT, storage_schemas, &count);
	if (FAILED(hr))
		return hr;

	if (count == 0) {
		std::cout << " sorry, storage is not found." << std::endl;
		return E_UNEXPECTED;
	}

	std::wcout << "storage(s);" << std::endl;
	//for (int i = 0; i < count; i++) {
                PWINBIO_STORAGE_SCHEMA ss = storage_schemas[0];
		print_guid(ss->DatabaseId);
	//}

	return S_OK;
}

HRESULT enum_units(PWINBIO_UNIT_SCHEMA *unit_schemas) {
	HRESULT hr;
	SIZE_T count = 0;

	hr = WinBioEnumBiometricUnits(WINBIO_TYPE_FINGERPRINT, unit_schemas, &count);
	if (FAILED(hr))
		return hr;

	if (count == 0) {
		std::cout << " sorry, unit is not found." << std::endl;
		return E_UNEXPECTED;
	}

	std::wcout << "unit(s);" << std::endl;
	for (int i = 0; i < count; i++) {
		std::wcout << "\t" << unit_schemas[i]->Description
                        << std::endl << "\t" << unit_schemas[i]->DeviceInstanceId
                        << std::endl << "\t" << unit_schemas[i]->Manufacturer
                        << std::endl << "\t" << unit_schemas[i]->Model << std::endl;
	}

	return S_OK;
}

HRESULT enum_bsp(PWINBIO_BSP_SCHEMA *bsp_schemas) {
	SIZE_T bsp_count;
	HRESULT hr = WinBioEnumServiceProviders(WINBIO_TYPE_FINGERPRINT, bsp_schemas, &bsp_count);
	if (FAILED(hr))
		return hr;

	if (bsp_count == 0) {
		std::cout << " sorry, provider is not found." << std::endl;
		return E_UNEXPECTED;
	}

	std::wcout << "service provider(s);" << std::endl;
	for (int i = 0; i < bsp_count; i++) {
		std::wcout << "\t" << bsp_schemas[i]->Description << "("
			<< bsp_schemas[i]->Vendor << ", "
			<< bsp_schemas[i]->Version.MajorVersion << "."
			<< bsp_schemas[i]->Version.MinorVersion << ")" << std::endl;
	}

	return S_OK;
}

HRESULT create_and_configure_private(PWINBIO_UNIT_SCHEMA unit_schemas) {
        HRESULT hr;

        std::wcout << L"checking and installing the private storage..." << std::endl;
        hr = PrivatePool::install_private_storage(unit_schemas, 0);
        if (FAILED(hr))
                return hr;

        std::wcout << L"adding the unit to private storage..." << std::endl;
        hr = PrivatePool::add_unit_to_private_storage(unit_schemas, 0);
        return hr;



}

HRESULT delete_private(PWINBIO_UNIT_SCHEMA unit_schemas) {

        HRESULT hr;

        std::wcout << L"removing unit from private storage..." << std::endl;
        hr = PrivatePool::remove_unit_from_private_storage(unit_schemas, 0);
        if (FAILED(hr))
                print_error(hr);

        std::wcout << L"checking and uninstalling private storage..." << std::endl;
        hr = PrivatePool::uninstall_private_storage(unit_schemas, 0);
        return hr;
}

void bio_callback_fn(PWINBIO_ASYNC_RESULT async_result) {


	std::wcout << "got this one: " << async_result->Operation << std::endl;

	switch (async_result->Operation) {
	case WINBIO_OPERATION_OPEN_FRAMEWORK: 
		
		break;


	}

	if (async_result != NULL)
		WinBioFree(async_result);

}

void print_error(HRESULT error_code) {
        WCHAR buffer[4096];
        BioHelper::ConvertErrorCodeToString(error_code, buffer, 4095);
        std::wcout << std::endl << "RESULT: " << buffer << std::endl;
	
}

void print_guid(GUID guid) {
        std::wcout << "{";
        std::wcout.fill('0');
        std::wcout.width(8);
        std::wcout << std::hex << std::uppercase << guid.Data1;
        std::wcout.width(1);
        std::wcout << "-";
        std::wcout.width(4);
        std::wcout << std::hex << std::uppercase << guid.Data2;
        std::wcout.width(1);
        std::wcout << "-";
        std::wcout.width(4);
        std::wcout << std::hex << std::uppercase << guid.Data3;
        std::wcout.width(1);
        std::wcout << "-";
        std::wcout.width(2);
        std::wcout << std::hex << std::uppercase << guid.Data4[0];
        std::wcout.width(2);
        std::wcout << std::hex << std::uppercase << guid.Data4[1];
        std::wcout.width(1);
        std::wcout << "-";
        std::wcout.width(2);
        std::wcout << std::hex << std::uppercase << guid.Data4[2];
        std::wcout.width(2);
        std::wcout << std::hex << std::uppercase << guid.Data4[3];
        std::wcout.width(2);
        std::wcout << std::hex << std::uppercase << guid.Data4[4];
        std::wcout.width(2);
        std::wcout << std::hex << std::uppercase << guid.Data4[5];
        std::wcout.width(2);
        std::wcout << std::hex << std::uppercase << guid.Data4[6];
        std::wcout.width(2);
        std::wcout << std::hex << std::uppercase << guid.Data4[7];
        std::wcout.width(1);
        std::wcout << "}" << std::endl;
}

void free_bio_address(PVOID address) {
	if (address == NULL)
                return;

	WinBioFree(address);
}
