/* SPDX-License-Identifier: MIT */
#ifndef CM_IDLE_WAKE_H
#define CM_IDLE_WAKE_H
#include <stdbool.h>
#include "cm_idle_interrupt.h"
#include "cm_dispatch.h"
/* Nine genuine source groups: JOBSLEEP, idle T3 interrupt, T3RUPT2,
 * WAKER, JOBWAKE, WAKER return, TASKOVER, RESUME, idle adoption.
 * Each C invocation returns before the next group/overlay is selected.
 * No task/job payload is invented; final transfer names the actual job entry.
 * Image/io are authoritative, irq is the original external interrupt metadata.
 * Producer exclusion spans the composed source groups. */
bool cm_idle_wake_step(apollo_overlay_state *,cm_native_io *,cm_idle_interrupt_state *,exec_transfer *,uint16_t);
#endif
