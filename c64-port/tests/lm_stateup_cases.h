/* SPDX-License-Identifier: MIT */
#ifndef LM_STATEUP_CASES_H
#define LM_STATEUP_CASES_H
#include "lm_stateup.h"
/* Each case enters one order pair (case n: pair n % 13) at NEWOPS with LOC
 * there and BANKSET = BBANK (FB 13, or 04 for R00), and runs through the
 * trampoline until a point the table does not own: NEWOPS with LOC outside
 * the file, R00's EXIT to basic code, or CHANG2. The image is pseudo-random within the program's domain: the flag
 * words (so each switch order goes both ways), EB, SUPERBNK, the
 * interrupt-enable state and MODE; EDOP +0; a work area of one of the five
 * VAC areas with PUSHLOC inside it; NEWJOB +0, or positive in a fifth of
 * the cases; for THISVINT, QPRET as CALL leaves it (its caller's return,
 * or another bank 13 word outside the file).
 * tools/lm_stateup_reference.c runs the same states in the original
 * program until lm_stateup_leaves(). */
#define LM_STATEUP_CASES 4096u
void lm_stateup_input(apollo_overlay_state *,p66_bias_io *,uint16_t n);
int lm_stateup_leaves(agc_word z,agc_word fb,agc_word superbank,agc_word loc,agc_word bankset);
enum exec_status lm_stateup_run(apollo_overlay_state *,p66_bias_io *,uint16_t n,uint16_t *steps);
/* Digest in the oracle's order (tools/lm_stateup_freeze.py). */
uint32_t lm_stateup_digest(apollo_overlay_state *,uint16_t n,enum exec_status *);
#endif
