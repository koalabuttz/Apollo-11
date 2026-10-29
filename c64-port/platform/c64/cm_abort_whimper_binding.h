/* SPDX-License-Identifier: MIT */
#ifndef CM_ABORT_WHIMPER_BINDING_H
#define CM_ABORT_WHIMPER_BINDING_H
#include "cm_abort_whimper.h"
typedef struct cm_whimper_fixture {uint32_t source_case;uint16_t run,status,step,failures;uint8_t in_isr;} cm_whimper_fixture;
#define CM_WHIMPER_FIXTURE ((cm_whimper_fixture *)(uintptr_t)0xc000)
#define CM_WHIMPER_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_whimper_fixture_fits[(sizeof(cm_whimper_fixture)<=0x40)?1:-1];
typedef char cm_whimper_io_fits[(sizeof(cm_native_io)<=0x30)?1:-1];
#endif
