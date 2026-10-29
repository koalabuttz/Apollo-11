/* SPDX-License-Identifier: MIT */
#ifndef CM_INIT_MIXED_BINDING_H
#define CM_INIT_MIXED_BINDING_H
#include "cm_init_display_mixed.h"
#include <stdint.h>
#define CM_INIT_MIXED_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef struct {uint16_t run,source_case,phase,status,failures;exec_transfer next;} cm_init_mixed_fixture;
#define CM_INIT_MIXED_FIXTURE ((cm_init_mixed_fixture *)(uintptr_t)0xc000)
typedef char cm_init_mixed_fixture_bound[(sizeof(cm_init_mixed_fixture)<=0x40u)?1:-1];
typedef char cm_init_mixed_io_bound[(sizeof(cm_native_io)<=0x20u)?1:-1];
#endif
