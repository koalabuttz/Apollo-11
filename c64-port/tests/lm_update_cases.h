/* SPDX-License-Identifier: MIT */
#ifndef LM_UPDATE_CASES_H
#define LM_UPDATE_CASES_H
#include "lm_update.h"
/* Each case enters one UPDATE_PROGRAM entry (case n: entry n % 35) and runs
 * through the trampoline (with lm_interbank_routines for POSTJUMP and
 * BANKCALL) until a point neither owns: TESTXACT, PHASCHNG, NEWMODEX,
 * GOXDSPF, FALTON, FINDVAC, ENDOFJOB, INTPRET, UPFLAG, TPAGREE, INTWAKEU,
 * NEWMODEX+3 or ENDEXT. The image is pseudo-random within the program's
 * own domain: UPVERB 0-3, COMPNUMB as OHWELL1 accepts it for that verb (2
 * for V70/V73, 3-20 for V71 and V72), UPCOUNT 1-COMPNUMB, UPTEMP a small component number (or, where TIMEDIDR returns
 * through it, one of the two return addresses TIMEDIDL stores), UPBUFF
 * words with update addresses in any erasable bank (EB0 from 061 up) where
 * V71/V72 store through them, including one-word loads at a bank's first word
 * and loads running past a bank's end, MODREG +0, -0 or a mode; FB/EB, SUPERBNK, TIME1/TIME2,
 * channel 11 and the interrupt-enable state pseudo-random.
 * tools/lm_update_reference.c does the same in the original program. */
#define LM_UP_CASES 4096u
void lm_up_input(apollo_overlay_state *,p66_bias_io *,agc_hardware *,uint16_t n);
enum exec_status lm_up_run(apollo_overlay_state *,p66_bias_io *,agc_hardware *,uint16_t n,uint16_t *steps);
/* Digest in the oracle's order (tools/lm_update_freeze.py). */
uint32_t lm_up_digest(apollo_overlay_state *,uint16_t n,enum exec_status *);
#endif
