/* SPDX-License-Identifier: MIT */
#ifndef CM_PRIORITY_DISPLAY_BINDING_H
#define CM_PRIORITY_DISPLAY_BINDING_H
#include "cm_priority_display.h"
typedef struct cm_priority_display_fixture {uint32_t source_case;uint16_t run,status,step,failures;} cm_priority_display_fixture;
#define CM_PRIORITY_DISPLAY_FIXTURE ((cm_priority_display_fixture *)(uintptr_t)0xc000)
#define CM_PRIORITY_DISPLAY_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_priority_display_fixture_fits[(sizeof(cm_priority_display_fixture)<=0x40)?1:-1];
typedef char cm_priority_display_io_fits[(sizeof(cm_native_io)<=0x30)?1:-1];
#endif
