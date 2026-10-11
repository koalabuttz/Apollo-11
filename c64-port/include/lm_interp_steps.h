/* SPDX-License-Identifier: MIT */
#ifndef LM_INTERP_STEPS_H
#define LM_INTERP_STEPS_H
#include "agc_multiword.h"
#include "overlay_runtime.h"
/* Compiled Luminary099 interpretive dispatch paths and simple order bodies.
 *
 * Each function is a native translation of one fixed straight-line source
 * path of Luminary099/INTERPRETER.agc (DANZIG, NEWOPS, OPJUMP, address
 * formation and the named bodies). A native caller chooses every path at
 * compile time and passes the original order and operand words of its own
 * routine as literal constants; those words only reproduce the original
 * bookkeeping (EDOP, CYR, ADDRWD, POLISH, LOC, banks, A/L/Q). Nothing is
 * fetched from, or decoded out of, code memory, and no function selects a
 * path from an order word. Runtime data decisions (BZE, BPL, switch tests)
 * return their outcome so the compiled caller follows the original branch.
 *
 * Contract: canonical 2048-word image, ordinary erasable words, raw 16-bit
 * A/L, coherent EB/FB/BB, NEWJOB +0 and producers excluded by the caller.
 * Each body ends at the entry of DANZIG (PC 06061); dispatch helpers end at
 * the PC of the body they dispatch to. */

/* NEWOPS 06070 through OPJUMP's CCS CYR for a negative order-pair word. */
void lmi_newops(apollo_overlay_state *, agc_word pair);
/* DANZIG 06061 with EDOP +0 and NEWJOB +0 through INCR LOC to NEWOPS. */
void lmi_danzig_next(apollo_overlay_state *);
/* DANZIG 06061 with a pending right-hand order through OPJUMP's CCS CYR. */
void lmi_danzig_right(apollo_overlay_state *);

/* Operand formation after OPJUMP for orders with an address. `target` is
 * the INDJUMP body PC the source's INDEX CYR transfer reaches. */
void lmi_direct(apollo_overlay_state *, agc_word operand, agc_word target);
void lmi_indexed(apollo_overlay_state *, agc_word operand, agc_word target);
/* OPJUMP2/15BITADR for index, branch and switch orders (MISCJUMP). */
void lmi_misc(apollo_overlay_state *, agc_word operand, agc_word target);
/* OPJUMP2/OPJUMP3 for unary orders (UNAJUMP); target is the body PC. */
void lmi_unary(apollo_overlay_state *, agc_word target);

/* NEWOPS with a positive store code: DOSTORE, STORJUMP's TC and the
 * STORE, STORE,1 or STORE,2 address formation and DP/TP/vector store,
 * returning through TC Q to TCF DANZIG (DANZIG entry). */
void lmi_store(apollo_overlay_state *, agc_word code);

/* Bodies, each from its INDJUMP/MISCJUMP/UNAJUMP target to DANZIG. */
void lmi_dload(apollo_overlay_state *);
void lmi_sload(apollo_overlay_state *);
void lmi_pddl(apollo_overlay_state *);
void lmi_setpd(apollo_overlay_state *);
void lmi_vdef(apollo_overlay_state *);
void lmi_axt(apollo_overlay_state *);
void lmi_incr(apollo_overlay_state *);
/* SW/ for the switch-bit orders; returns 1 when the original branches
 * (GOTO through SWBRANCH), 0 when it skips (SWSKIP/NOOP to DANZIG). */
uint8_t lmi_switch(apollo_overlay_state *);
/* BZE/GOTO and BPL/BMN with their BRANCH outcome; 1 means GOTO taken.
 * A taken branch ends at NEWOPS of the target (GOTO, INTPRET +3). */
uint8_t lmi_bze(apollo_overlay_state *);
uint8_t lmi_bpl(apollo_overlay_state *);
void lmi_goto(apollo_overlay_state *);
/* RVQ through GOTO +4 and INTPRET +3 to the caller's NEWOPS. */
void lmi_rvq(apollo_overlay_state *);

/* Prologues from an INDJUMP/UNAJUMP target to the entry contract of an
 * existing native body (see the named headers). */
/* DDV: dividend fetch, BUF exchange and bank-0 switch, ending at DDV/BDDV
 * 02353; `divisor` is the fixed DP operand ADDRWD names. */
void lmi_ddv_prologue(apollo_overlay_state *, const agc_word divisor[2]);
/* GSHIFT: FBANK 0 and GENSHIFT 02214 with A = LOW7. */
void lmi_gshift_prologue(apollo_overlay_state *);
/* COSINE: BRANCH on MPAC and the DCS/QUARTER reduction to SINE. Returns
 * nothing; leaves the native SINE body entry contract (see sband_trig). */
#endif
