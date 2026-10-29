/* SPDX-License-Identifier: MIT */
#ifndef LM_EXEC_SERVICES_CASES_H
#define LM_EXEC_SERVICES_CASES_H
#include "lm_exec_routines.h"
#include "lm_restart_routines.h"
/* lm-exec-services cases, each run through the trampoline from its entry to
 * a stop the case names; tools/lm_exec_services_reference.c runs the same
 * entry in the original program.
 *   0..4095     RESTARTS (the lm_restart_routines cases) through the native
 *               FINDVAC/NOVAC/WAITLIST/LONGCALL it selects, to SWRETURN
 *   4096..8183  every genuine NOVAC/FINDVAC/WAITLIST/LONGCALL/TWIDDLE call
 *               site (146), 28 states each, to the caller's return
 *   8184..9207  the LONGCYCL and GETCADR Waitlist tasks, to TASKOVER or the
 *               task 2CADR
 * Executive and Waitlist erasable is canonical, with occupancy chosen so the
 * no-core (1202), no-VAC (1201), full-Waitlist (1203), bad-delay (1204) and
 * LONGCALL delta (1204) alarms are reached too. */
#define LM_EXS_RESTART_CASES 4096u
#define LM_EXS_SITES 146u
#define LM_EXS_SITE_STATES 28u
#define LM_EXS_SITE_FIRST LM_EXS_RESTART_CASES
#define LM_EXS_TASK_FIRST (LM_EXS_SITE_FIRST+LM_EXS_SITES*LM_EXS_SITE_STATES)
#define LM_EXS_TASK_CASES 1024u
#define LM_EXS_CASES (LM_EXS_TASK_FIRST+LM_EXS_TASK_CASES)
/* Outcome kinds, shared with the oracle. */
#define LM_EXS_RETURNED 0u   /* reached the case's stop */
#define LM_EXS_ALARM 1u      /* at BAILOUT1 or POODOO1 */
#define LM_EXS_REFUSED 2u    /* native EXEC_OUT_OF_SCOPE; the original executes DPBIT14 */
typedef struct {
 agc_word stop_z,stop_fb; /* stop_fb 0177777: any FB (fixed-fixed) */
 agc_word superbank;
} lm_exs_case;
/* Fills the image, io and stop for case n. */
void lm_exs_input(apollo_overlay_state *,p66_bias_io *,lm_exs_case *,uint16_t n);
/* Runs case n natively; returns the outcome kind (3: anything else). */
unsigned lm_exs_run(apollo_overlay_state *,uint16_t n,uint16_t *steps);
uint32_t lm_exs_digest(apollo_overlay_state *,uint16_t n);
#endif
