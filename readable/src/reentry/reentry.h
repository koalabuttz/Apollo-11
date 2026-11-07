/* SPDX-License-Identifier: MIT */
/* REENTRY_CONTROL.agc (Comanche055 pp. 844-882, banks 25 and 26): the
 * entry guidance, P63 to P67. Every two seconds, through SERVICER's
 * CM/POSE, SCALEPOP gets the state (TARGETNG: the velocity, drag D, the
 * range to the target THETAH) and goes on at GOTOADDR, the roll command
 * equations of the present phase of entry: INITROLL, HUNTEST, UPCONTRL,
 * KEP2, PREDICT3. In the order of the listing:
 *
 *   STARTENT, STARTEN1, SCALEPOP, REFAZE10 (startent.c, pp. 844-847):
 *     the start, the target vector, and the two-second pass.
 *   TARGETNG, GETETA (targetng.c, pp. 848-852): the state and the range.
 *
 * The faithful C is c64-port/src/cm_reentry.c.
 *
 * Each basic function runs from a word the program can resume at (an
 * entry, a return from a routine out of the file, or the word after an
 * EXIT) until its code leaves it: at a TC out of the file (Z the routine,
 * Q the return) or at a TC INTPRET (run on to NEWOPS). The interpretive
 * code is the listing's words, run by the readable interpreter. */
#ifndef READABLE_REENTRY_H
#define READABLE_REENTRY_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if COMANCHE055

/* TC at, to a routine out of the file: Q gets the return, and the
 * routine runs next. */
static inline void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* STARTENT to REFAZE10 -- startent.c */
void startent_after_exit(void);     /* STARTENT +1 */
void refaze10(void);                /* REFAZE10 (SCALEPOP's EXIT) */
void refaze10_after_phaschng(void); /* REFAZE10 +2 */
extern const struct interpretive_code STARTENT_CODE, STARTENT_INTERPRETIVE_CODE, SCALEPOP_CODE,
    REFAZE10_CODE;

/* TARGETNG and GETETA -- targetng.c */
extern const struct interpretive_code TARGETNG_CODE, GETETA_CODE;

#endif

#endif
