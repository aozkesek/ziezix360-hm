// WFApp2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "WFApp2.h"

using namespace std;

int main(int argc, char **argv)
{
	HRESULT hr;
	PWINBIO_UNIT_SCHEMA unit_schemas = NULL;
	PWINBIO_STORAGE_SCHEMA storage_schemas = NULL;
	WINBIO_SESSION_HANDLE session_handle = NULL;
        BioHelper::POOL_CONFIGURATION configs;
	
        if (verify_arg(argc, argv, "-p"))
                print_bio_service_providers();
                
        wcout << L"checking units..." << endl;
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

        wcout << L"checking storages..." << endl;
        hr = enum_storages(&storage_schemas);
        if (FAILED(hr)) {
                print_error(hr);
                goto cleanup_and_exit;
        }

        if (!verify_arg(argc, argv, "-o"))
                goto cleanup_and_exit;
        
        wcout << L"opening session..." << endl;
        hr = open_session(unit_schemas->UnitId, verify_arg(argc, argv, "-p"), &session_handle);
	if (FAILED(hr)) {
		print_error(hr);
		goto cleanup_and_exit;
	}
        
        for (int i = 0; i < 5; i++) {
                cout << "swipe point finger, sequence = " << i << endl;
                capture_sample(session_handle, unit_schemas->UnitId, i);
                cout << "" << endl;
        }
        
        wcout << L"closing session..." << endl;
	WinBioCloseSession(session_handle);
        
cleanup_and_exit:

	free_bio_address(storage_schemas);
	free_bio_address(unit_schemas);

}

void capture_sample(WINBIO_SESSION_HANDLE session_handle, WINBIO_UNIT_ID unit_id, int sequence) {
        
        HRESULT hr;
        PWINBIO_BIR bir_sample = NULL;
        SIZE_T bir_sample_size = 0;
        WINBIO_REJECT_DETAIL reject_detail;

        wcout << L"capturing sample..." << endl;
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
                wcout << "malformat BIR found." << bir_sample_size << endl;
                free_bio_address(bir_sample);
                return;
        }
        
        PWINBIO_BDB_ANSI_381_HEADER ansi_header =
                (PWINBIO_BDB_ANSI_381_HEADER)(((PBYTE)bir_sample) + bir_sample->StandardDataBlock.Offset);
        PWINBIO_BDB_ANSI_381_RECORD ansi_record =
                (PWINBIO_BDB_ANSI_381_RECORD)(((PBYTE)ansi_header) + sizeof(WINBIO_BDB_ANSI_381_HEADER));

        DWORD image_w = ansi_record->HorizontalLineLength;
        DWORD image_h = ansi_record->VerticalLineLength;
        ULONG image_size = ansi_record->BlockLength - sizeof(WINBIO_BDB_ANSI_381_RECORD);
        UCHAR pixel_depth = ansi_header->PixelDepth;

        PBYTE image_buffer = ((PBYTE)ansi_record + sizeof(WINBIO_BDB_ANSI_381_RECORD));
        
        write_image_to_file(image_buffer, image_w, image_h, pixel_depth, image_size, sequence);
                
        wcout << "collected sample size is " << bir_sample_size << endl;
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
		cout << " sorry, storage is not found." << endl;
		return E_UNEXPECTED;
	}

	wcout << "storage(s);" << endl;
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
		cout << " sorry, unit is not found." << endl;
		return E_UNEXPECTED;
	}

	wcout << "unit(s);" << endl;
	for (int i = 0; i < count; i++) {
		wcout << "\t" << unit_schemas[i]->Description
                        << endl << "\t" << unit_schemas[i]->DeviceInstanceId
                        << endl << "\t" << unit_schemas[i]->Manufacturer
                        << endl << "\t" << unit_schemas[i]->Model << endl;
	}

	return S_OK;
}

HRESULT enum_bsp(PWINBIO_BSP_SCHEMA *bsp_schemas) {
	SIZE_T bsp_count;
	HRESULT hr = WinBioEnumServiceProviders(WINBIO_TYPE_FINGERPRINT, bsp_schemas, &bsp_count);
	if (FAILED(hr))
		return hr;

	if (bsp_count == 0) {
		cout << " sorry, provider is not found." << endl;
		return E_UNEXPECTED;
	}

	wcout << "service provider(s);" << endl;
	for (int i = 0; i < bsp_count; i++) {
		wcout << "\t" << bsp_schemas[i]->Description << "("
			<< bsp_schemas[i]->Vendor << ", "
			<< bsp_schemas[i]->Version.MajorVersion << "."
			<< bsp_schemas[i]->Version.MinorVersion << ")" << endl;
	}

	return S_OK;
}

HRESULT create_and_configure_private(PWINBIO_UNIT_SCHEMA unit_schemas) {
        HRESULT hr;

        wcout << L"checking and installing the private storage..." << endl;
        hr = PrivatePool::install_private_storage(unit_schemas, 0);
        if (FAILED(hr))
                return hr;

        wcout << L"adding the unit to private storage..." << endl;
        hr = PrivatePool::add_unit_to_private_storage(unit_schemas, 0);
        return hr;



}

HRESULT delete_private(PWINBIO_UNIT_SCHEMA unit_schemas) {

        HRESULT hr;

        wcout << L"removing unit from private storage..." << endl;
        hr = PrivatePool::remove_unit_from_private_storage(unit_schemas, 0);
        if (FAILED(hr))
                print_error(hr);

        wcout << L"checking and uninstalling private storage..." << endl;
        hr = PrivatePool::uninstall_private_storage(unit_schemas, 0);
        return hr;
}

void write_image_to_file(PBYTE image_buffer, int image_w, 
        int image_h, UCHAR pixel_depth, 
        const ULONG image_size, int sequence) {
        using namespace std;
        string ct =
                "C:\\Users\\Ahmet USER\\source\\repos\\ziezix360-hmb\\WFApp3\\Debug\\captured_template_";
        ct += ((char)(sequence + '0'));
        ct += ".bmp";

        cout << "image width = " << image_w
                << ", heigth = " << image_h
                << ", size = " << image_size 
                << ", bit count = " << (int)pixel_depth << endl;
        
        BITMAPFILEHEADER bmp_file_header;
        BITMAPINFOHEADER bmp_info_header;
        RGBQUAD rgbq[256];
        int bfh_size = sizeof(BITMAPFILEHEADER);
        int bih_size = sizeof(BITMAPINFOHEADER);
        int rgbq_size = sizeof(RGBQUAD) * 256;

        memset(&bmp_file_header, 0, bfh_size);
        memset(&bmp_info_header, 0, bih_size);
        memset(rgbq, 0, rgbq_size);

        for (int i = 0; i < 256; i++)
                rgbq[i].rgbBlue = rgbq[i].rgbGreen = rgbq[i].rgbRed = i;

        bmp_info_header.biSize = bih_size;
        bmp_info_header.biWidth = image_w;
        bmp_info_header.biHeight = image_h;
        bmp_info_header.biPlanes = 1;
        bmp_info_header.biBitCount = (WORD)pixel_depth;
        bmp_info_header.biSizeImage = image_size;
        bmp_info_header.biXPelsPerMeter = 14173;
        bmp_info_header.biYPelsPerMeter = 14173;
        bmp_info_header.biCompression = BI_RGB;
        //bmp_info_header.biClrUsed = 256;
        
        bmp_file_header.bfType = 0x4d42;
        bmp_file_header.bfSize = bfh_size + bih_size + rgbq_size + image_size;
        bmp_file_header.bfOffBits = bfh_size + bih_size + rgbq_size;

        filebuf bitmap_file_buffer;
        bitmap_file_buffer.open(ct, ios::out|ios::binary);
        cout << "image file is opened...\n";

        ostream bitmap_output_stream(&bitmap_file_buffer);
        
        bitmap_output_stream.write(
                (const char*)&bmp_file_header, bfh_size);
        bitmap_output_stream.write(
                (const char*)&bmp_info_header, bih_size);
        bitmap_output_stream.write(
                (const char*)rgbq, rgbq_size);

        bitmap_output_stream.write(
                (const char *)image_buffer, image_size);
        
        bitmap_file_buffer.close();
               
        cout << "image file is closed...\n";


}

void bio_callback_fn(PWINBIO_ASYNC_RESULT async_result) {


	wcout << "got this one: " << async_result->Operation << endl;

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
        wcout << endl << "RESULT: " << buffer << endl;
	
}

void print_guid(GUID guid) {
        wcout << "{";
        wcout.fill('0');
        wcout.width(8);
        wcout << hex << uppercase << guid.Data1;
        wcout.width(1);
        wcout << "-";
        wcout.width(4);
        wcout << hex << uppercase << guid.Data2;
        wcout.width(1);
        wcout << "-";
        wcout.width(4);
        wcout << hex << uppercase << guid.Data3;
        wcout.width(1);
        wcout << "-";
        wcout.width(2);
        wcout << hex << uppercase << guid.Data4[0];
        wcout.width(2);
        wcout << hex << uppercase << guid.Data4[1];
        wcout.width(1);
        wcout << "-";
        wcout.width(2);
        wcout << hex << uppercase << guid.Data4[2];
        wcout.width(2);
        wcout << hex << uppercase << guid.Data4[3];
        wcout.width(2);
        wcout << hex << uppercase << guid.Data4[4];
        wcout.width(2);
        wcout << hex << uppercase << guid.Data4[5];
        wcout.width(2);
        wcout << hex << uppercase << guid.Data4[6];
        wcout.width(2);
        wcout << hex << uppercase << guid.Data4[7];
        wcout.width(1);
        wcout << "}" << endl;
}

void free_bio_address(PVOID address) {
	if (address == NULL)
                return;

	WinBioFree(address);
}

void print_bio_service_providers() {
        HRESULT hr;
        PWINBIO_BSP_SCHEMA bsp_schemas = NULL;

        wcout << L"checking providers..." << endl;
        hr = enum_bsp(&bsp_schemas);
        if (FAILED(hr))
                print_error(hr);
        else
                free_bio_address(bsp_schemas);

}

bool verify_arg(int argc, char** argv, const char* arg, enum_arg_verify method) {
        // argv => -key1=value1 -key2=value2 ...

        if (argc <= 1)
                return false;

        for (int i = 1; i < argc; i++) {
                string _arg = argv[i];

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

