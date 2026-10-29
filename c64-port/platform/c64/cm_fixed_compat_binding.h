/* SPDX-License-Identifier: MIT */
#ifndef C64_CM_FIXED_COMPAT_BINDING_H
#define C64_CM_FIXED_COMPAT_BINDING_H
#include "cm_fixed_callers.h"
#include <stdint.h>
#define CM_FIXED_IO ((cm_native_io *)(uintptr_t)0xc400)
#define CM_FIXED_FETCH ((agc_word *)(uintptr_t)0xc430)
typedef struct {uint32_t source_case;uint16_t run,phase,status,step,failures;} cm_fixed_fixture;
#define CM_FIXED_FIXTURE ((cm_fixed_fixture *)(uintptr_t)0xc000)
typedef char cm_fixed_io_lease[(sizeof(cm_native_io)<=0x20u)?1:-1];
typedef char cm_fixed_fixture_lease[(sizeof(cm_fixed_fixture)<=0x40u)?1:-1];
#endif
