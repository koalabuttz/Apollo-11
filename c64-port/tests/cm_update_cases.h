/* SPDX-License-Identifier: MIT */
#ifndef CM_UPDATE_CASES_H
#define CM_UPDATE_CASES_H
#include "cm_update.h"
/* Each case enters one Comanche055 UPDATE_PROGRAM entry (case n: entry n % 36;
 * UPACTOFF through TC BANKCALL from FRESH_START or P20-P25) and runs
 * through the trampoline (with cm_interbank_routines for POSTJUMP and
 * BANKCALL) until a point neither owns: TESTXACT, PHASCHNG, NEWMODEX,
 * GOXDSPF, FALTON, FINDVAC, ENDOFJOB, INTPRET, UPFLAG, TPAGREE, INTWAKEU,
 * NEWMODEX+3, ENDEXT, or SWRETURN after UPACTOFF's TC Q. The image is pseudo-random within the program's
 * own domain: UPVERB 0-3, COMPNUMB as OHWELL1 accepts it for that verb (2
 * for V70/V73, 3-20 for V71 and V72), UPCOUNT 1-COMPNUMB, UPTEMP a small component number (or, where TIMEDIDR returns
 * through it, one of the two return addresses TIMEDIDL stores), UPBUFF
 * words with update addresses in any erasable bank (EB0 from 061 up) where
 * V71/V72 store through them, including one-word loads at a bank's first word
 * and loads running past a bank's end, MODREG +0, -0, 2 or a mode, FLAGWRD5 bit 8 often set; FB/EB, SUPERBNK, TIME1/TIME2,
 * channel 11 and the interrupt-enable state pseudo-random.
 * tools/cm_update_reference.c does the same in the original program. */
#define CM_UP_CASES 4096u
#define CM_UP_BANKCALL 04662u
#define CM_UP_SWRETURN 04675u
#define CM_UP_SWRETURN_POINT (uint16_t)(2048u+(CM_UP_SWRETURN-04000u))
void cm_up_input(apollo_overlay_state *,cm_native_io *,agc_hardware *,uint16_t n);
enum exec_status cm_up_run(apollo_overlay_state *,cm_native_io *,agc_hardware *,uint16_t n,uint16_t *steps);
/* Digest in the oracle's order (tools/cm_update_freeze.py). */
uint32_t cm_up_digest(apollo_overlay_state *,uint16_t n,enum exec_status *);
#endif
