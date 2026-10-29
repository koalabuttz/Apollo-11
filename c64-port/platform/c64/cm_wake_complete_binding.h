/* SPDX-License-Identifier: MIT */
#ifndef CM_WAKE_DISPATCH_BINDING_H
#define CM_WAKE_DISPATCH_BINDING_H
#include "cm_wake_complete.h"
typedef struct cm_wake_fixture {uint32_t source_case;uint16_t run,status,step,failures;exec_transfer next;} cm_wake_fixture;
#define CM_WAKE_FIXTURE ((cm_wake_fixture *)(uintptr_t)0xc000)
#define CM_WAKE_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_wake_fixture_fits[(sizeof(cm_wake_fixture)<=0x40)?1:-1];
typedef char cm_wake_io_fits[(sizeof(cm_native_io)<=0x30)?1:-1];
#endif
