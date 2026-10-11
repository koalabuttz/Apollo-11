/* SPDX-License-Identifier: MIT */
#ifndef LM_IDLE_START_H
#define LM_IDLE_START_H
#include "p66_bias.h"
#include "executive.h"
/* Literal ADVAN3214 with NEWJOB+0 and an already admitted positive BASIC core0.
 * Executes CAF TWO/NUDIRECT/SUPDXCHZ through its actual source destination.
 * Positive pending NEWJOB (NUCHANG2), SELFCHK and interpreter continuations
 * require their separate source bodies. No native queue is polled here. */
enum exec_status lm_idle_start(apollo_overlay_state *,p66_bias_io *,agc_word *,exec_address *);
#endif
