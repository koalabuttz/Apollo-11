/* SPDX-License-Identifier: MIT */
#ifndef CM_IDLE_WAKE_BINDING_H
#define CM_IDLE_WAKE_BINDING_H
#include "cm_idle_wake.h"
typedef struct {uint32_t source_case;uint16_t run,phase,failures;cm_idle_interrupt_state irq;} cm_idle_wake_fixture;
#define CM_IDLE_WAKE_FIXTURE ((cm_idle_wake_fixture *)(uintptr_t)0xc000)
#define CM_IDLE_WAKE_IO ((cm_native_io *)(uintptr_t)0xc400)
typedef char cm_idle_wake_fixture_fits[(sizeof(cm_idle_wake_fixture)<=0x40)?1:-1];
typedef char cm_idle_wake_io_fits[(sizeof(cm_native_io)<=0x30)?1:-1];
#endif
