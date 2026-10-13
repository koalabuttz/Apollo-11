/* SPDX-License-Identifier: MIT */
#ifndef CM_EXEC_IDLE_CASES_H
#define CM_EXEC_IDLE_CASES_H
#include "cm_exec_routines.h"
/* cm-exec-idle cases: each Comanche055 idle-loop, SUPDXCHZ or SVCT3 entry
 * entered at its original address and run through the trampoline for one
 * routine; tools/cm_exec_idle_reference.c runs the same entry in the
 * original program until it first reaches the continuation the native run
 * left in Z and FB. Inputs come from the qualified image milestones'
 * domains (idle dispatch, idle recheck, super dispatch, SVCT3). */
enum cm_exi_cohort {CM_EXI_ADVAN,CM_EXI_NUCHANG2,CM_EXI_DUMMYJOB,CM_EXI_SUPDXCHZ,CM_EXI_SVCT3,CM_EXI_COHORTS};
#define CM_EXI_CASES 5120u
/* Outcome kinds, shared with the oracle. */
#define CM_EXI_RETURNED 0u   /* at the continuation */
#define CM_EXI_ALARM 1u      /* at BAILOUT or POODOO */
#define CM_EXI_REFUSED 2u    /* native EXEC_OUT_OF_SCOPE */
enum cm_exi_cohort cm_exi_cohort(uint16_t n);
uint16_t cm_exi_first(enum cm_exi_cohort);
/* Fills the image and io for case n. */
void cm_exi_input(apollo_overlay_state *,cm_native_io *,uint16_t n);
/* Runs case n natively (one routine); returns the outcome kind (3: anything
 * else). The continuation is the image's Z and FB. */
unsigned cm_exi_run(apollo_overlay_state *,cm_native_io *,uint16_t n);
uint32_t cm_exi_digest(apollo_overlay_state *,uint16_t n);
#endif
