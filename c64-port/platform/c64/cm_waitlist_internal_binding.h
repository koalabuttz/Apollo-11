/* SPDX-License-Identifier: MIT */
#ifndef CM_WL_INTERNAL_DISPATCH_BINDING_H
#define CM_WL_INTERNAL_DISPATCH_BINDING_H
#include "cm_waitlist_internal.h"
typedef struct cm_wl_internal_fixture {uint32_t source_case;uint16_t run,status,step,failures;agc_word literal;exec_transfer next;} cm_wl_internal_fixture;
#define CM_WL_INTERNAL_FIXTURE ((cm_wl_internal_fixture *)(uintptr_t)0xc000)
#define CM_WL_INTERNAL_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_wl_internal_fixture_fits[(sizeof(cm_wl_internal_fixture)<=0x40)?1:-1];
typedef char cm_wl_internal_io_fits[(sizeof(cm_native_io)<=0x30)?1:-1];
#endif
