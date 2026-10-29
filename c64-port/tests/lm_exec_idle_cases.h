/* SPDX-License-Identifier: MIT */
#ifndef LM_EXEC_IDLE_CASES_H
#define LM_EXEC_IDLE_CASES_H
#include "lm_exec_routines.h"
/* lm-exec-idle cases: each Luminary099 idle-loop, SUPDXCHZ or SVCT3 entry
 * entered at its original address and run through the trampoline for one
 * routine; tools/lm_exec_idle_reference.c runs the same entry in the
 * original program until it first reaches the continuation the native run
 * left in Z and FB. Inputs come from the qualified image milestones'
 * domains (idle dispatch, super dispatch) and the SVCT3 states. */
enum lm_exi_cohort {LM_EXI_ADVAN,LM_EXI_NUCHANG2,LM_EXI_DUMMYJOB,LM_EXI_SUPDXCHZ,LM_EXI_SVCT3,LM_EXI_COHORTS};
#define LM_EXI_CASES 4864u
/* Outcome kinds, shared with the oracle. */
#define LM_EXI_RETURNED 0u   /* at the continuation */
#define LM_EXI_ALARM 1u      /* at BAILOUT1 or POODOO1 */
#define LM_EXI_REFUSED 2u    /* native EXEC_OUT_OF_SCOPE */
enum lm_exi_cohort lm_exi_cohort(uint16_t n);
uint16_t lm_exi_first(enum lm_exi_cohort);
/* Fills the image, io and hardware (channel 11, in_interrupt) for case n. */
void lm_exi_input(apollo_overlay_state *,p66_bias_io *,agc_hardware *,uint16_t n);
/* Runs case n natively (one routine); returns the outcome kind (3: anything
 * else). The continuation is the image's Z and FB. */
unsigned lm_exi_run(apollo_overlay_state *,p66_bias_io *,agc_hardware *,uint16_t n);
uint32_t lm_exi_digest(apollo_overlay_state *,uint16_t n);
#endif
