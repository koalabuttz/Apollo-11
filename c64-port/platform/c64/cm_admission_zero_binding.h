/* SPDX-License-Identifier: MIT */
#ifndef CM_ADMISSION_ZERO_BINDING_H
#define CM_ADMISSION_ZERO_BINDING_H
#include "cm_admission_zero.h"
typedef struct {uint32_t source_case;uint16_t run,status,step,failures;exec_address job;} cm_admission_zero_fixture;
#define CM_ADMISSION_ZERO_FIXTURE ((cm_admission_zero_fixture *)(uintptr_t)0xc000)
#define CM_ADMISSION_ZERO_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_admission_zero_fixture_bound[(sizeof(cm_admission_zero_fixture)<=0x40)?1:-1];
typedef char cm_admission_zero_io_bound[(sizeof(cm_native_io)<=0x30)?1:-1];
#endif
