/* SPDX-License-Identifier: MIT */
#ifndef CM_VAC5_STORE_BINDING_H
#define CM_VAC5_STORE_BINDING_H
#include "cm_vac5_store.h"
typedef struct cm_vac5_fixture {uint32_t source_case;uint16_t run,status,step,failures;} cm_vac5_fixture;
#define CM_VAC5_FIXTURE ((cm_vac5_fixture *)(uintptr_t)0xc000)
#define CM_VAC5_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_vac5_fixture_fits[(sizeof(cm_vac5_fixture)<=0x40)?1:-1];
typedef char cm_vac5_io_fits[(sizeof(cm_native_io)<=0x30)?1:-1];
#endif
