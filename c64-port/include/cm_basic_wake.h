/* SPDX-License-Identifier: MIT */
#ifndef CM_BASIC_WAKE_H
#define CM_BASIC_WAKE_H
#include "cm_native_image.h"
#include "cm_dispatch.h"
/* Actual JOBWAKE5201 through ordinary basic caller return. Sign-extended
 * A sleeping CADR; Q0100..07777; active positive core0 and all candidates
 * positive/dormant/-0 (no+0). NEWJOB-0 ordisplacements0..72 by12. A matching
 * dormant context has saved bank<=0177. Source full callerL debris retained.
 * IRQ0 on return; NOT_FOUND is a completed source return. Caller owns exclusion.
 * Unsupported inputs preserve image and complete io metadata. */
enum exec_status cm_basic_wake(apollo_overlay_state *,cm_native_io *);
#endif
