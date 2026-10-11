/* SPDX-License-Identifier: MIT */
#ifndef CM_PHASE_CLEANUP_BINDING_H
#define CM_PHASE_CLEANUP_BINDING_H
#include "cm_phase_cleanup.h"
typedef struct cm_cleanup_fixture {uint32_t source_case;uint16_t run,status,step,failures;} cm_cleanup_fixture;
#define CM_CLEANUP_FIXTURE ((cm_cleanup_fixture *)(uintptr_t)0xc000)
#define CM_CLEANUP_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_cleanup_fixture_fits[(sizeof(cm_cleanup_fixture)<=0x40)?1:-1];
typedef char cm_cleanup_io_fits[(sizeof(cm_native_io)<=0x30)?1:-1];
#endif
