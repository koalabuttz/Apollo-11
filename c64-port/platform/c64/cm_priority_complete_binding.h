/* SPDX-License-Identifier: MIT */
#ifndef CM_PRIORITY_COMPLETE_BINDING_H
#define CM_PRIORITY_COMPLETE_BINDING_H
#include "cm_priority_complete.h"
typedef struct cm_prio_fixture {uint32_t source_case;uint16_t run,status,step,failures;exec_transfer next;} cm_prio_fixture;
#define CM_PRIO_FIXTURE ((cm_prio_fixture *)(uintptr_t)0xc000)
#define CM_PRIO_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_prio_fixture_fits[(sizeof(cm_prio_fixture)<=0x40)?1:-1];
typedef char cm_prio_io_fits[(sizeof(cm_native_io)<=0x30)?1:-1];
#endif
