/* SPDX-License-Identifier: MIT */
#ifndef LM_RESTART_ROUTINES_CASES_H
#define LM_RESTART_ROUTINES_CASES_H
#include "lm_restart_routines.h"
#include "lm_restart_select_cases.h"
/* Composition from RESTARTS to SWRETURN through the trampoline. Each service
 * the restart selects (FINDVAC, NOVAC, WAITLIST, LONGCALL) is a test stub
 * that records its entry registers and the 2CADR at Q, then returns to Q+2,
 * as the original services do; tools/lm_restart_routines_reference.c does
 * the same in the original program. Cases are the RESTARTS-entry cases of
 * lm_restart_select (variable, table, display, wrapped-clock table). */
#define LM_RSR_CASES 4096u
#define LM_RSR_MAX_CALLS 4u
typedef struct { agc_word service,a,l,q,cadr,bbcon; } lm_rsr_call;
typedef struct { lm_rsr_call call[LM_RSR_MAX_CALLS]; uint8_t calls; } lm_rsr_log;
static inline uint16_t lm_rsr_select_case(uint16_t n){return n<3072u?n:(uint16_t)(5120u+(n-3072u));}
/* Runs case n from RESTARTS until SWRETURN; returns the trampoline status. */
enum exec_status lm_rsr_run(apollo_overlay_state *,uint16_t n,lm_rsr_log *,uint16_t *steps);
/* Digests in the oracle's order (tools/lm_restart_freeze.py). */
uint32_t lm_rs_select_digest(apollo_overlay_state *,uint16_t n);
uint32_t lm_rs_compose_digest(apollo_overlay_state *,uint16_t n,enum exec_status *);
#endif
