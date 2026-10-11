/* SPDX-License-Identifier: MIT */
#ifndef LM_TASKOVER_RESUME_H
#define LM_TASKOVER_RESUME_H
#include "p66_bias.h"
#include "executive.h"
/* TASKOVER5261, RUPTAGN-0: fixed original register/bank restoration through
 * architectural RESUME. The interrupted source instruction in BRUPT remains
 * ordinary saved data; the native caller resumes an explicit compiled entry.
 * The +1 next-task route is handled by the distinct WAITLIST source service. */
enum exec_status lm_taskover_resume(apollo_overlay_state *,p66_bias_io *,uint16_t *in_interrupt);
#endif
