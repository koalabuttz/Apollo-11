/* SPDX-License-Identifier: MIT */
#ifndef CM_IDLE_DISPATCH_BINDING_H
#define CM_IDLE_DISPATCH_BINDING_H
#include "cm_idle_interrupt.h"
typedef struct cm_idle_fixture {uint32_t source_case;uint16_t run,status,step,failures;cm_idle_interrupt_state irq;} cm_idle_fixture;
#define CM_IDLE_FIXTURE ((cm_idle_fixture *)(uintptr_t)0xc000)
#define CM_IDLE_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_idle_fixture_fits[(sizeof(cm_idle_fixture)<=0x40)?1:-1];
typedef char cm_idle_io_fits[(sizeof(cm_native_io)<=0x30)?1:-1];
#endif
