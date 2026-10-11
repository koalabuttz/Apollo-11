/* SPDX-License-Identifier: MIT */
#ifndef CM_IDLE_RECHECK_ZERO_BINDING_H
#define CM_IDLE_RECHECK_ZERO_BINDING_H
#include "cm_idle_recheck.h"
typedef struct cm_idle_recheck_fixture {uint32_t source_case;uint16_t run,status,step,failures;exec_transfer next;} cm_idle_recheck_fixture;
#define CM_IDLE_RECHECK_FIXTURE ((cm_idle_recheck_fixture *)(uintptr_t)0xc000)
#define CM_IDLE_RECHECK_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_idle_recheck_fixture_fits[(sizeof(cm_idle_recheck_fixture)<=0x40)?1:-1];
typedef char cm_idle_recheck_io_fits[(sizeof(cm_native_io)<=0x30)?1:-1];
#endif
