/* SPDX-License-Identifier: MIT */
#ifndef CM_FLOOR_DISPATCH_BINDING_H
#define CM_FLOOR_DISPATCH_BINDING_H
#include "cm_admission_floor.h"
typedef struct cm_floor_fixture {uint32_t source_case;uint16_t run,status,step,failures;exec_transfer next;} cm_floor_fixture;
#define CM_FLOOR_FIXTURE ((cm_floor_fixture *)(uintptr_t)0xc000)
#define CM_FLOOR_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_floor_fixture_fits[(sizeof(cm_floor_fixture)<=0x40)?1:-1];
typedef char cm_floor_io_fits[(sizeof(cm_native_io)<=0x30)?1:-1];
#endif
