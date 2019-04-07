#pragma once

#include "BioHelper.h"

#include <windows.h>
#include <winbio.h>
#include <winbio_adapter.h>
#include <winbio_err.h>
#include <winbio_ioctl.h>
#include <winbio_types.h>

#include <iostream>

#ifndef __PRIVATEPOOL_H__
#define __PRIVATEPOOL_H__

namespace PrivatePool {


static const GUID PRIVATE_POOL_DATABASE_ID =
{ 0x0ea2500c, 0x230f, 0x4b80, { 0x99, 0xed, 0x14, 0x76, 0xee, 0x24, 0x04, 0x4b } };

HRESULT install_private_storage(PWINBIO_UNIT_SCHEMA unit_array, SIZE_T selected_unit);
HRESULT uninstall_private_storage(PWINBIO_UNIT_SCHEMA unit_array, SIZE_T selected_unit);
HRESULT add_unit_to_private_storage(PWINBIO_UNIT_SCHEMA unit_array, SIZE_T selected_unit);
HRESULT remove_unit_from_private_storage(PWINBIO_UNIT_SCHEMA unit_array, SIZE_T selected_unit);

bool is_private_storage_installed(__in WINBIO_UUID *DatabaseId);

}

#endif //__PRIVATEPOOL_H__