/* SPDX-License-Identifier: MIT */
#ifndef STANDARD_LEAD_ROUTINES_CASES_H
#define STANDARD_LEAD_ROUTINES_CASES_H
#include "standard_lead_routines.h"
#include "lm_interbank_routines.h"
/* Cases 0..3071: each standard_lead stage input (phases E/BKCALL..resume),
 * entered through the registry. Fixed inline CADRs belong to a test caller
 * whose compiled operands sit at 04000..04077 (the flight program has no
 * E/BKCALL or E/CALL caller).
 * Cases 3072..3583: E/BKCALL round trip, SWCALL, native callee, SWRETURN,
 * landing point 2607, back to the caller.
 * Cases 3584..4095: E/CALL round trip, INTPRET CALL, native interpretive
 * callee ending in RVQ, EXIT, resume, INTPRET+2, back to the caller's LOC. */
#define LM_SLR_CASES 4096u
enum lm_slr_kind {LM_SLR_STAGE,LM_SLR_BKCALL,LM_SLR_ECALL};
enum lm_slr_kind lm_slr_input(apollo_overlay_state *,p66_bias_io *,uint16_t n,const agc_native_program **program,agc_word *fixed);
/* Trampoline result for case n: status, steps, image and io left in the arguments. */
enum exec_status lm_slr_run(apollo_overlay_state *,p66_bias_io *,uint16_t n,uint16_t stop,uint16_t max_steps,uint16_t *steps);
uint32_t lm_slr_digest(enum exec_status,uint16_t steps,const apollo_overlay_state *,const p66_bias_io *);
#define LM_SLR_CALLEE_BANK 027u
#endif
