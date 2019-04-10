#pragma once

#include "PrivatePool.h"

#include <iostream>
#include <fstream>

HRESULT enum_bsp(PWINBIO_BSP_SCHEMA* bsp_schemas);
HRESULT enum_units(PWINBIO_UNIT_SCHEMA* unit_schema);
HRESULT enum_storages(PWINBIO_STORAGE_SCHEMA* storage_schemas);
HRESULT open_session(WINBIO_UNIT_ID unit_id, bool is_private, 
        PWINBIO_SESSION_HANDLE session_handle);
HRESULT create_and_configure_private(PWINBIO_UNIT_SCHEMA unit_schemas);
HRESULT delete_private(PWINBIO_UNIT_SCHEMA unit_schemas);

void capture_sample(WINBIO_SESSION_HANDLE session_handle, 
        WINBIO_UNIT_ID unit_id, int sequence);
void write_image_to_file(PBYTE img_buffer, int img_width,
        int img_height, UCHAR pixel_depth, 
        const ULONG img_size, int sequence);
void print_bio_service_providers();
void bio_callback_fn(PWINBIO_ASYNC_RESULT async_result);
void free_bio_address(PVOID address);
void print_error(HRESULT result_handle);
void print_guid(GUID guid);

typedef enum _arg_verify {
        ARG_VERIFY_EQUAL,
        ARG_VERIFY_START_WITH,
        ARG_VERIFY_INCLUDES
} enum_arg_verify;

bool verify_arg(int argc, char** argv, const char* arg, enum_arg_verify method = ARG_VERIFY_EQUAL);
