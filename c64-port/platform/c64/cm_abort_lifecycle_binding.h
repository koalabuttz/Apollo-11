/* SPDX-License-Identifier: MIT */
#ifndef CM_ABORT_LIFECYCLE_BINDING_H
#define CM_ABORT_LIFECYCLE_BINDING_H
#include "cm_abort_lifecycle.h"
typedef struct {uint16_t run,source_case,phase,status,failures,literal;uint8_t in_isr;} cm_lifecycle_fixture;
#define CM_LIFECYCLE_FIXTURE ((cm_lifecycle_fixture *)(uintptr_t)0xc000)
#define CM_LIFECYCLE_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_lifecycle_fixture_bound[(sizeof(cm_lifecycle_fixture)<=0x40u)?1:-1];
typedef char cm_lifecycle_io_bound[(sizeof(cm_native_io)==26u)?1:-1];
#endif
