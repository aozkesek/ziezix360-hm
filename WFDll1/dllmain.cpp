// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

/*
*/
HRESULT open_session(HWND hwnd, 
        PWINBIO_SESSION_HANDLE session_handle);
HRESULT capture_sample(WCHAR* sequence, HWND hwnd,
        WINBIO_SESSION_HANDLE session_handle);
void write_image_to_bitmap_file(PBYTE image_buffer, int image_w,
        int image_h, UCHAR pixel_depth,
        const ULONG image_size, WCHAR* sequence);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, 
        LPVOID lpReserved)
{
        
        switch (ul_reason_for_call)
        {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
        break;
        }
        return TRUE;
}

void LogSendMessage(HWND hwnd, const WCHAR* message) {
        //SendMessage(hwnd, LB_ADDSTRING, NULL, (LPARAM) message);
        MessageBox(hwnd, (LPCWSTR)message, L"Fingerprint Scanner", 
                MB_ICONWARNING|MB_OK);
}

HRESULT open_session(HWND hwnd, PWINBIO_SESSION_HANDLE session_handle) {

        HRESULT hr = 0;
       
        if (session_handle == NULL)
                return E_INVALIDARG;

        hr = WinBioOpenSession(
                WINBIO_TYPE_FINGERPRINT,
                WINBIO_POOL_SYSTEM,
                WINBIO_FLAG_DEFAULT | WINBIO_FLAG_RAW,
                NULL,
                0,
                WINBIO_DB_DEFAULT,
                session_handle);
        if (FAILED(hr)) {
                LogSendMessage(hwnd, L"could not open a session.");
                session_handle = NULL;
        }
        return hr;
}

HRESULT capture_sample(WCHAR *sequence, HWND hwnd,
        WINBIO_SESSION_HANDLE session_handle) {

        HRESULT hr;
        WINBIO_UNIT_ID unit_id = NULL;

        PWINBIO_BIR bir_sample = NULL;
        SIZE_T bir_sample_size = 0;
        WINBIO_REJECT_DETAIL reject_detail;

        PWINBIO_BIR_HEADER bir_header = NULL;
        USHORT valid_fields_mask = 0;

        PWINBIO_BDB_ANSI_381_HEADER ansi_header = NULL;
        PWINBIO_BDB_ANSI_381_RECORD ansi_record = NULL;
        DWORD image_w = 0L;
        DWORD image_h = 0L;
        ULONG image_size = 0L;
        UCHAR pixel_depth = NULL;
        PBYTE image_buffer = NULL;

        if (session_handle == NULL) {
                LogSendMessage(hwnd, L"session not found.");
                return E_INVALIDARG;
        }
               
        hr = WinBioLocateSensor(session_handle, &unit_id);
        if (FAILED(hr)) {
                LogSendMessage(hwnd, L"could not locate a unit.");
                return hr;
        }

        hr = WinBioCaptureSample(
                session_handle,
                WINBIO_NO_PURPOSE_AVAILABLE,
                WINBIO_DATA_FLAG_RAW,
                &unit_id,
                &bir_sample,
                &bir_sample_size,
                &reject_detail);
        if (FAILED(hr)) {
                LogSendMessage(hwnd, L"could not capture a sample.");
                goto clean_and_return;
        }

        bir_header =
                (PWINBIO_BIR_HEADER)(((PBYTE)bir_sample) + bir_sample->HeaderBlock.Offset);

        valid_fields_mask = bir_header->ValidFields;
        if (valid_fields_mask & WINBIO_BIR_FIELD_NEVER_VALID) {
                LogSendMessage(hwnd, L"sample is malformatted.");
                hr = E_INVALIDARG;
                goto clean_and_return;
        }

        ansi_header = (PWINBIO_BDB_ANSI_381_HEADER)
                (((PBYTE)bir_sample) + bir_sample->StandardDataBlock.Offset);
        ansi_record = (PWINBIO_BDB_ANSI_381_RECORD)
                (((PBYTE)ansi_header) + sizeof(WINBIO_BDB_ANSI_381_HEADER));

        image_w = ansi_record->HorizontalLineLength;
        image_h = ansi_record->VerticalLineLength;
        image_size = ansi_record->BlockLength - sizeof(WINBIO_BDB_ANSI_381_RECORD);
        pixel_depth = ansi_header->PixelDepth;

        image_buffer = ((PBYTE)ansi_record + sizeof(WINBIO_BDB_ANSI_381_RECORD));

        write_image_to_bitmap_file(image_buffer, image_w, image_h, pixel_depth, 
                image_size, sequence);

clean_and_return:
        if (bir_sample != NULL)
                WinBioFree(bir_sample);

        return hr;
}

void write_image_to_bitmap_file(PBYTE image_buffer, int image_w,
        int image_h, UCHAR pixel_depth, const ULONG image_size, 
        WCHAR *sequence) {
        
        using namespace std;
        
        BITMAPFILEHEADER bmp_file_header;
        BITMAPINFOHEADER bmp_info_header;
        RGBQUAD rgbq[256];
        int bfh_size = sizeof(BITMAPFILEHEADER);
        int bih_size = sizeof(BITMAPINFOHEADER);
        int rgbq_size = sizeof(RGBQUAD) * 256;
        
        //colorref table
        memset(rgbq, 0, rgbq_size);
        for (int i = 0; i < 256; i++)
                rgbq[i].rgbBlue = rgbq[i].rgbGreen = rgbq[i].rgbRed = i;

        memset(&bmp_info_header, 0, bih_size);
        bmp_info_header.biSize = bih_size;
        bmp_info_header.biWidth = image_w;
        bmp_info_header.biHeight = image_h;
        bmp_info_header.biPlanes = 1;
        bmp_info_header.biBitCount = (WORD)pixel_depth;
        bmp_info_header.biSizeImage = image_size;
        bmp_info_header.biXPelsPerMeter = 14173;
        bmp_info_header.biYPelsPerMeter = 14173;
        bmp_info_header.biCompression = BI_RGB;
        
        memset(&bmp_file_header, 0, bfh_size);
        bmp_file_header.bfType = 0x4d42;
        bmp_file_header.bfSize = bfh_size + bih_size + rgbq_size + image_size;
        bmp_file_header.bfOffBits = bfh_size + bih_size + rgbq_size;

        filebuf bitmap_file_buffer;
        bitmap_file_buffer.open(sequence, ios::out | ios::binary);
        ostream bitmap_output_stream(&bitmap_file_buffer);

        bitmap_output_stream.write(
                (const char*)& bmp_file_header, bfh_size);
        
        bitmap_output_stream.write(
                (const char*)& bmp_info_header, bih_size);
        
        bitmap_output_stream.write(
                (const char*)rgbq, rgbq_size);

        bitmap_output_stream.write(
                (const char*)image_buffer, image_size);

        bitmap_file_buffer.close();

}

extern "C" {

        __declspec(dllexport) int __stdcall
                BiometricCapture(WCHAR* sequence, HWND hwnd) {

                HRESULT hr = S_OK;
                WINBIO_SESSION_HANDLE session_handle = NULL;

                SendMessage(hwnd, WM_ACTIVATEAPP, TRUE, NULL);

                hr = open_session(hwnd, &session_handle);
                if (FAILED(hr))
                        return hr;

                WinBioAcquireFocus();
                hr = capture_sample(sequence, hwnd, session_handle);
                WinBioReleaseFocus();

                WinBioCloseSession(session_handle);

                return hr;
        }

}