/* SPDX-License-Identifier: MIT */
#ifndef CM_SCANRITY_COMPLETE_BINDING_H
#define CM_SCANRITY_COMPLETE_BINDING_H
#include "cm_scan_zero.h"
typedef struct cm_scan_fixture {uint32_t source_case;uint16_t run,status,step,failures;exec_transfer next;} cm_scan_fixture;
#define CM_SCAN_FIXTURE ((cm_scan_fixture *)(uintptr_t)0xc000)
#define CM_SCAN_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_scan_fixture_fits[(sizeof(cm_scan_fixture)<=0x40)?1:-1];
typedef char cm_scan_io_fits[(sizeof(cm_native_io)<=0x30)?1:-1];
#endif
