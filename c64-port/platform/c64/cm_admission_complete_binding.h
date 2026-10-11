/* SPDX-License-Identifier: MIT */
#ifndef CM_ADMISSION_COMPLETE_BINDING_H
#define CM_ADMISSION_COMPLETE_BINDING_H
#include "cm_admission_complete.h"
typedef struct cm_admit_fixture {uint32_t source_case;uint16_t run,status,step,failures;exec_address job;} cm_admit_fixture;
#define CM_ADMIT_FIXTURE ((cm_admit_fixture *)(uintptr_t)0xc000)
#define CM_ADMIT_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_admit_fixture_fits[(sizeof(cm_admit_fixture)<=0x40)?1:-1];
typedef char cm_admit_io_fits[(sizeof(cm_native_io)<=0x30)?1:-1];
#endif
