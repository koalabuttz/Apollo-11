/* SPDX-License-Identifier: MIT */
#ifndef CM_RECOVERY_BINDING_H
#define CM_RECOVERY_BINDING_H
#include "cm_abort_recovery.h"
#include <stdint.h>
#define CM_RECOVERY_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef struct {uint16_t run,source_case,phase,status,failures;agc_word literal;} cm_recovery_fixture;
#define CM_RECOVERY_FIXTURE ((cm_recovery_fixture *)(uintptr_t)0xc000)
typedef char cm_recovery_fixture_bound[(sizeof(cm_recovery_fixture)<=0x40u)?1:-1];
typedef char cm_recovery_io_bound[(sizeof(cm_native_io)<=0x20u)?1:-1];
#endif
