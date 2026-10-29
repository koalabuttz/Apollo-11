/* SPDX-License-Identifier: MIT */
#ifndef CM_STANDARD_LEAD_ROUTINES_CASES_H
#define CM_STANDARD_LEAD_ROUTINES_CASES_H
#include "standard_lead_routines.h"
#include "cm_interbank_routines.h"
/* Cases 0..3071: each standard_lead stage input (phases E/BKCALL..resume),
 * entered through the registry. Fixed inline CADRs belong to a test caller
 * whose compiled operands sit at 04000..04077 (the flight program has no
 * E/BKCALL or E/CALL caller).
 * Cases 3072..3583: E/BKCALL round trip, SWCALL, native callee, SWRETURN,
 * landing point 2607, back to the caller.
 * Cases 3584..4095: E/CALL round trip, INTPRET CALL, native interpretive
 * callee ending in RVQ, EXIT, resume, INTPRET+2, back to the caller's LOC. */
#define CM_SLR_CASES 4096u
enum cm_slr_kind {CM_SLR_STAGE,CM_SLR_BKCALL,CM_SLR_ECALL};
enum cm_slr_kind cm_slr_input(apollo_overlay_state *,cm_native_io *,uint16_t n,const agc_native_program **program,agc_word *fixed);
/* Trampoline result for case n: status, steps, image and io left in the arguments. */
enum exec_status cm_slr_run(apollo_overlay_state *,cm_native_io *,uint16_t n,uint16_t stop,uint16_t max_steps,uint16_t *steps);
uint32_t cm_slr_digest(enum exec_status,uint16_t steps,const apollo_overlay_state *,const cm_native_io *);
#define CM_SLR_CALLEE_BANK 027u
#endif
