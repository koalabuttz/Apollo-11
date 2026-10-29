/* SPDX-License-Identifier: MIT */
#ifndef CM_IDLE_WAKE_CASES_H
#define CM_IDLE_WAKE_CASES_H
#include "cm_idle_wake.h"
#define CM_IDLE_WAKE_CASES 512u
#define CM_IDLE_WAKE_PHASES 9u
void cm_idle_wake_input(apollo_overlay_state *,cm_native_io *,cm_idle_interrupt_state *,uint32_t);
#endif
