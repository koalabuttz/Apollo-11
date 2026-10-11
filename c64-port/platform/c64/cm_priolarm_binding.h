/* SPDX-License-Identifier: MIT */
#ifndef CM_PRIOLARM_BINDING_H
#define CM_PRIOLARM_BINDING_H
#include "cm_priolarm.h"
typedef struct cm_priolarm_fixture {uint32_t source_case;uint16_t run,status,step,failures;} cm_priolarm_fixture;
#define CM_PRIOLARM_FIXTURE ((cm_priolarm_fixture *)(uintptr_t)0xc000)
#define CM_PRIOLARM_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_priolarm_fixture_fits[(sizeof(cm_priolarm_fixture)<=0x40)?1:-1];
typedef char cm_priolarm_io_fits[(sizeof(cm_native_io)<=0x30)?1:-1];
#endif
