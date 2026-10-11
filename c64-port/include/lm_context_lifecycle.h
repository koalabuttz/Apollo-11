/* SPDX-License-Identifier: MIT */
#ifndef LM_CONTEXT_LIFECYCLE_H
#define LM_CONTEXT_LIFECYCLE_H
#include "lm_basic_change.h"
enum lm_context_lifecycle_entry {LM_CONTEXT_PRIORITY,LM_CONTEXT_SLEEP};
/* Literal basic PRIOCHNG05146 or JOBSLEEP05133; selected basic DTCB,
 * interpretive INTRSM/NEWOPS06070, or sleeping-to-idle ADVAN03214.
 * Ordinary canonical bank/priority/CADR domain; explicit channel11 and
 * SUPER/IRQ metadata. No callback, decoder or second scheduling poll.
 * Unsupported inputs preserve every supplied object. */
enum exec_status lm_context_lifecycle(apollo_overlay_state *,p66_bias_io *,
 agc_word *channel11,enum lm_context_lifecycle_entry,exec_transfer *);
#endif
