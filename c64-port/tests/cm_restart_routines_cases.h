/* SPDX-License-Identifier: MIT */
#ifndef CM_RESTART_ROUTINES_CASES_H
#define CM_RESTART_ROUTINES_CASES_H
#include "cm_restart_routines.h"
#include "agc_word.h"
/* Selection: seven cohorts of 1024 entering cm_restart_routines_select:
 * variable phases, every table phase from RESTARTS, the X.1 display phase,
 * PHSPART2 second entries, GETPART2 table parts, table phases again with
 * wrapped clocks, and the signed-zero and overflow branches of CHKNOVAC,
 * FINDTIME and ITSLGCL2. */
#define CM_RR_SELECT_CASES 7168u
enum cm_rr_entry cm_rr_select_input(apollo_overlay_state *,uint16_t n);
uint16_t cm_rr_pairs(void);
void cm_rr_pair(uint16_t k,uint8_t *group,agc_word *phase);
/* Composition from RESTARTS to SWRETURN through the trampoline. Each service
 * the restart selects (FINDVAC, NOVAC, WAITLIST, LONGCALL) is a test stub
 * that records its entry registers and the 2CADR at Q, then returns to Q+2,
 * as the original services do; tools/cm_restart_routines_reference.c does
 * the same in the original program. Cases are the RESTARTS-entry cases of
 * the selection (variable, table, display, wrapped-clock table, targeted). */
#define CM_RR_COMPOSE_CASES 5120u
#define CM_RR_MAX_CALLS 4u
typedef struct { agc_word service,a,l,q,cadr,bbcon; } cm_rr_call;
typedef struct { cm_rr_call call[CM_RR_MAX_CALLS]; uint8_t calls; } cm_rr_log;
static inline uint16_t cm_rr_compose_case(uint16_t n){return n<3072u?n:(uint16_t)(5120u+(n-3072u));}
enum exec_status cm_rr_compose(apollo_overlay_state *,uint16_t n,cm_rr_log *,uint16_t *steps);
/* NEWPHASE from each of its nine callers' `TC NEWPHASE` (Z 4114, Q at the
 * group operand) through the trampoline to the caller's L+2. Phases
 * positive, +0, negative and -0, with TIME1 and the old phase words random. */
#define CM_RR_NEWPHASE_CASES 1024u
void cm_rr_newphase_input(apollo_overlay_state *,uint16_t n);
uint16_t cm_rr_newphase_return(const apollo_overlay_state *);
enum exec_status cm_rr_newphase(apollo_overlay_state *,uint16_t n,uint16_t *steps);
/* Digests in the oracle's order (tools/cm_restart_routines_freeze.py). */
uint32_t cm_rr_select_digest(apollo_overlay_state *,uint16_t n);
uint32_t cm_rr_compose_digest(apollo_overlay_state *,uint16_t n,enum exec_status *);
uint32_t cm_rr_newphase_digest(apollo_overlay_state *,uint16_t n,enum exec_status *);
#endif
