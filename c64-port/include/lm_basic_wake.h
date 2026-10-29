/* SPDX-License-Identifier: MIT */
#ifndef LM_BASIC_WAKE_H
#define LM_BASIC_WAKE_H
#include "executive.h"
#include "p66_bias.h"
/* Actual JOBWAKE05137 through ordinary basic caller return. A is a sign-extended
 * ordinary sleeping CADR, Q0100..7777; core0 active, asleep or -0 (a task's
 * JOBWAKE under the dummy job; a core0 match takes WAKETEST's SPECTEST path,
 * which needs NEWJOB -0 or a displacement), ordinary banks/SUPER,
 * candidate priorities positive/dormant/-0 (no+0). NEWJOB0/displacements12..84
 * or-0. Matching dormant contexts have BBCON<=0177. Wakes the first match,
 * retains raw caller L in source DXCH debris, and leaves IRQ inhibited.
 * EXEC_NOT_FOUND is a completed original return, not a rejection. Unsupported
 * image/io unchanged. Canonical image owned under caller exclusion. */
enum exec_status lm_basic_wake(apollo_overlay_state *,p66_bias_io *);
#endif
