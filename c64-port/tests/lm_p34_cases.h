/* SPDX-License-Identifier: MIT */
#ifndef LM_P34_CASES_H
#define LM_P34_CASES_H
#include "lm_p34_routines.h"
/* Deterministic sampled cases for the P34-35_P74-75 transfer routines.
 * Case n:
 *  - n < LM_P34_BASIC_CASES: basic routine n%20 at its entry, over
 *    poisoned erasable debris (FLAGWRD2 BIT6, MODREG, the stored NORMEX
 *    and RTRN returns, an overflowing A and Q chosen per state);
 *  - then LM_P34_CLUPDATE_CASES at CLUPDATE's CLEAR GOTO (NEWOPS 35,3671)
 *    with QSAVED one of VN1645's interpretive returns or another CADR;
 *  - then LM_P34_VN1645_CASES lm-vn1645 entries and LM_P34_MIDGIM_CASES
 *    lm-midgim entries (GET+MGA, GET.LVC, SELECTMU) at their NEWOPS.
 * Every case runs through the trampoline with this file's routines and the
 * native INTER-BANK routines (BANKCALL) to the first point outside them.
 * The oracle stops at the same points: the external entries this file
 * transfers to, the alarm frontiers, a NEWOPS outside this file's
 * interpretive code, and the case's own basic returns (P3XORP7X's Q and Q+1,
 * the stored NORMEX or RTRN return). */
#define LM_P34_PER_BASIC 256u
#define LM_P34_BASIC_CASES (LM_P34_BASIC_ROUTINES*LM_P34_PER_BASIC)
#define LM_P34_CLUPDATE_CASES 1024u
#define LM_P34_VN1645_CASES 4096u
#define LM_P34_MIDGIM_CASES 4096u
#define LM_P34_CASES (LM_P34_BASIC_CASES+LM_P34_CLUPDATE_CASES+LM_P34_VN1645_CASES+LM_P34_MIDGIM_CASES)
/* Stops: z (logical), fb (or LM_P34_ANY_FB), q (or LM_P34_ANY_Q); a z of
 * 06070 means NEWOPS outside this file's interpretive code. */
#define LM_P34_ANY_FB 0177777u
#define LM_P34_ANY_Q 0177777u
#define LM_P34_STOPS 16u
#define LM_P34_COMMON_STOPS 10u /* the case's own returns follow these */
typedef struct {agc_word z,fb,q;} lm_p34_stop;
typedef struct {
 lm_p34_stop stop[LM_P34_STOPS]; /* terminated by z 0 */
 uint8_t refused;                /* native routine must refuse */
} lm_p34_case;
void lm_p34_input(apollo_overlay_state *,p66_bias_io *,lm_p34_case *,uint32_t n);
/* 1 if (FB, LOC) at NEWOPS lies in this file's interpretive code (the steps
 * pass through inner NEWOPS frontiers). */
int lm_p34_owned_newops(agc_word fb,agc_word loc);
/* Native run of case n: 1 stopped at an unmapped point, 2 refused, 3 any
 * other outcome. io returns the device record. */
#define LM_P34_STOPPED 1u
#define LM_P34_REFUSED 2u
unsigned lm_p34_run(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* The host program: this file's basic and interpretive routines, the native
 * INTER-BANK routines and their site operands merged with this file's. */
#define LM_P34_FETCH ((uint16_t)(2048u+(06070u-04000u)))
const agc_native_program *lm_p34_program(void);
/* Case n through another program (the C64 groups register only the
 * routines their cases reach). */
unsigned lm_p34_run_program(const agc_native_program *,apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* Digest of case n's run as tools/lm_p34_differential.py freezes it: two
 * 16-bit running sums over the tag (1 stopped, 0 refused, 3 other), the
 * 2048 erasable words (A and L 16-bit) and the interrupt-enable state. */
uint32_t lm_p34_digest(const agc_native_program *,apollo_overlay_state *,uint32_t n);
#endif
