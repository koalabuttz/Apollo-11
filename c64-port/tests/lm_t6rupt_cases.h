/* SPDX-License-Identifier: MIT */
#ifndef LM_T6RUPT_CASES_H
#define LM_T6RUPT_CASES_H
#include "lm_t6rupt_routines.h"
/* lm-t6rupt cases: each entry of lm_t6rupt_routines entered at its original
 * address and run through the trampoline for one routine;
 * tools/lm_t6rupt_reference.c runs the same entry in the original program
 * until it first reaches the continuation the native run left in Z and FB,
 * or, after RESUME, the end of the interrupt. */
enum lm_t6_cohort {LM_T6_VECTOR,LM_T6_DOT6RUPT,LM_T6_T6JOBCHK,LM_T6_WRITE,LM_T6_RESUME,LM_T6_COHORTS};
#define LM_T6_CASES 3840u
#define LM_T6_RETURNED 0u   /* at the continuation, or the interrupt ended */
#define LM_T6_REFUSED 2u    /* native EXEC_OUT_OF_SCOPE */
enum lm_t6_cohort lm_t6_cohort(uint16_t n);
uint16_t lm_t6_first(enum lm_t6_cohort);
void lm_t6_input(apollo_overlay_state *,p66_bias_io *,agc_hardware *,uint16_t n);
/* Runs case n natively (one routine); returns the outcome kind (3: not
 * entered). */
unsigned lm_t6_run(apollo_overlay_state *,p66_bias_io *,agc_hardware *,uint16_t n);
uint32_t lm_t6_digest(apollo_overlay_state *,uint16_t n);
#endif
