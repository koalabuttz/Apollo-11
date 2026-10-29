/* SPDX-License-Identifier: MIT */
#ifndef CM_SUPER_DISPATCH_BINDING_H
#define CM_SUPER_DISPATCH_BINDING_H
#include "cm_super_dispatch.h"
typedef struct cm_super_fixture {uint32_t source_case;uint16_t run,status,step,failures;exec_transfer next;} cm_super_fixture;
#define CM_SUPER_FIXTURE ((cm_super_fixture *)(uintptr_t)0xc000)
#define CM_SUPER_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_super_fixture_fits[(sizeof(cm_super_fixture)<=0x40)?1:-1];
typedef char cm_super_io_fits[(sizeof(cm_native_io)<=0x30)?1:-1];
#endif
