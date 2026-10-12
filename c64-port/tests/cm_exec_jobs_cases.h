/* SPDX-License-Identifier: MIT */
#ifndef CM_EXEC_JOBS_CASES_H
#define CM_EXEC_JOBS_CASES_H
#include "cm_exec_routines.h"
/* cm-exec-jobs cases: each Comanche055 Executive job service entered at its
 * original address and run through the trampoline for one routine;
 * tools/cm_exec_jobs_reference.c runs the same entry in the original program
 * until it first reaches the continuation the native run left in Z and FB.
 * Inputs come from the qualified image milestones' domains (admission,
 * context change, sleep, wake, priority and finish). */
enum cm_exj_cohort {CM_EXJ_SPVAC,CM_EXJ_CHANGE,CM_EXJ_JOBSLEEP,CM_EXJ_JOBWAKE,CM_EXJ_PRIOCHNG,CM_EXJ_ENDOFJOB,CM_EXJ_COHORTS};
#define CM_EXJ_CASES 6144u
/* Outcome kinds, shared with the oracle. */
#define CM_EXJ_RETURNED 0u   /* at the continuation */
#define CM_EXJ_ALARM 1u      /* at BAILOUT or POODOO */
#define CM_EXJ_REFUSED 2u    /* native EXEC_OUT_OF_SCOPE */
enum cm_exj_cohort cm_exj_cohort(uint16_t n);
uint16_t cm_exj_first(enum cm_exj_cohort);
/* Fills the image and io for case n. */
void cm_exj_input(apollo_overlay_state *,cm_native_io *,uint16_t n);
/* Runs case n natively (one routine); returns the outcome kind (3: anything
 * else). The continuation is the image's Z and FB. */
unsigned cm_exj_run(apollo_overlay_state *,cm_native_io *,uint16_t n);
uint32_t cm_exj_digest(apollo_overlay_state *,uint16_t n);
#endif
