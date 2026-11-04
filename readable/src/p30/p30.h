/* SPDX-License-Identifier: MIT */
/* P30_P37.agc, the LM's (Luminary099 pp. 614-617): P30, the external
 * delta-V targeting program, and S30.1, its computation. The faithful C
 * is c64-port/src/lm_p30_file.c (and the older c64-port/src/
 * lm_p30_routines.c, with the same keys).
 *
 * P30's basic code (bank 35) sets the update and track flags and asks the
 * astronaut, through VNPOOH, for the time of ignition (verb 06 noun 33)
 * and the delta V in local vertical coordinates (verb 06 noun 81). Then
 * its interpretive code CALLs S30.1 (bank 34), shows apogee, perigee and
 * delta V (verb 06 noun 42, PARAM30) and goes on to the display
 * REVN1645 (GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc's, src/lambert_aimpoint/)
 * with XDELVFLG set, for P40's external delta-V guidance.
 *
 * The interpretive code is in the listing's notation (P30_CODE,
 * PARAM30_CODE, S30_1_CODE), run by the readable interpreter. The entries
 * below are the basic code's: P30, each word a call out of the file
 * returns to (UPFLAG's, VNPOOH's, DOWNFLAG's) and PARAM30, where the
 * EXIT after the CALL S30.1 goes on. The binding is
 * src/binding/p30_routines.c. */
#ifndef READABLE_P30_H
#define READABLE_P30_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if LUMINARY099
extern const struct interpretive_code P30_CODE, PARAM30_CODE, S30_1_CODE;

void p30(void);
void p30_after_updatflg(void);   /* P30 +2: the word the first UPFLAG returns to */
void p30n33(void);               /* P30N33: the word the second UPFLAG returns to */
void p30_after_v06n33(void);     /* P30N33 +2: the word VNPOOH returns to */
void p30_after_v06n81(void);     /* P30N33 +4 */
void p30_after_downflag(void);   /* P30N33 +6: the word DOWNFLAG returns to */
void param30(void);              /* PARAM30: after the EXIT */
void param30_after_v06n42(void); /* PARAM30 +2: the word VNPOOH returns to */
#endif

#endif
