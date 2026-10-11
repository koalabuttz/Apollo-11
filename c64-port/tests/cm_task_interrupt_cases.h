/* SPDX-License-Identifier: MIT */
#ifndef CM_TASK_INTERRUPT_CASES_H
#define CM_TASK_INTERRUPT_CASES_H
#include "cm_task_interrupt.h"
#define CM_TASK_INTERRUPT_CASES 1024u
void cm_task_interrupt_input(apollo_overlay_state *,cm_native_io *,cm_task_interrupt_state *,uint16_t);
#endif
