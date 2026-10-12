/* SPDX-License-Identifier: MIT */
#ifndef LM_EXEC_JOBS_CASES_H
#define LM_EXEC_JOBS_CASES_H
#include "lm_exec_routines.h"
/* lm-exec-jobs cases: each Executive job service and Waitlist task service
 * entered at its original address and run through the trampoline;
 * tools/lm_exec_jobs_reference.c runs the same entry in the original program.
 * Inputs come from the qualified image milestones' domains (sleep, wake,
 * priority, finish, basic and interpreter change, admission, delay,
 * TASKOVER resume and T3 dispatch), with interpretive successors added for
 * JOBSLEEP and PRIOCHNG and FIXDELAY run from all 16 genuine call sites. */
enum lm_exj_cohort {LM_EXJ_SPVAC,LM_EXJ_CHANG1,LM_EXJ_CHANG2,LM_EXJ_JOBSLEEP,LM_EXJ_JOBWAKE,
 LM_EXJ_PRIOCHNG,LM_EXJ_ENDOFJOB,LM_EXJ_DELAY,LM_EXJ_FIXDELAY_SITES,LM_EXJ_RESUME,LM_EXJ_DISPATCH,LM_EXJ_COHORTS};
#define LM_EXJ_CASES 9216u
/* Outcome kinds, shared with the oracle. */
#define LM_EXJ_RETURNED 0u   /* reached one of the case's stops */
#define LM_EXJ_ALARM 1u      /* at BAILOUT1 or POODOO1 */
#define LM_EXJ_REFUSED 2u    /* native EXEC_OUT_OF_SCOPE */
#define LM_EXJ_MAX_STOPS 12u
typedef struct {
 agc_word stop[LM_EXJ_MAX_STOPS]; /* Z values where the run ends (any FB) */
 uint8_t stops;
 uint8_t resume;      /* also ends when the interrupt ends (RESUME) */
 agc_word until;      /* nonzero: the trampoline stops here (TASKOVER) */
 agc_hardware hw;
} lm_exj_case;
enum lm_exj_cohort lm_exj_cohort(uint16_t n);
uint16_t lm_exj_first(enum lm_exj_cohort);
/* Fills the image, io and case for case n. */
void lm_exj_input(apollo_overlay_state *,p66_bias_io *,lm_exj_case *,uint16_t n);
/* Runs case n natively; returns the outcome kind (3: anything else). */
unsigned lm_exj_run(apollo_overlay_state *,p66_bias_io *,lm_exj_case *,uint16_t n,uint16_t *steps);
uint32_t lm_exj_digest(apollo_overlay_state *,uint16_t n);
#endif
