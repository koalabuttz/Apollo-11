/* SPDX-License-Identifier: MIT */
#ifndef CM_ABORT_BINDING_H
#define CM_ABORT_BINDING_H
#include "cm_abort_snapshot.h"
#include <stdint.h>
#define CM_ABORT_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef struct {uint16_t run,source_case,phase,status,failures;agc_word literal;} cm_abort_fixture;
#define CM_ABORT_FIXTURE ((cm_abort_fixture *)(uintptr_t)0xc000)
typedef char cm_abort_fixture_bound[(sizeof(cm_abort_fixture)<=0x40u)?1:-1];
typedef char cm_abort_io_bound[(sizeof(cm_native_io)<=0x20u)?1:-1];
#endif
