/* SPDX-License-Identifier: MIT */
#ifndef CM_IDLE_INTERRUPT_CASES_H
#define CM_IDLE_INTERRUPT_CASES_H
#include "cm_idle_interrupt.h"
#define CM_IDLE_INTERRUPT_CASES 4096u
void cm_idle_interrupt_input(apollo_overlay_state *,cm_native_io *,cm_idle_interrupt_state *,uint32_t);
#endif
