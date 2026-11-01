/* SPDX-License-Identifier: MIT */
/* SYSTEM_TEST_STANDARD_LEAD_INS.agc, the LM's (Luminary099 pages 370-372)
 * and the CM's (Comanche055 pages 420-422): "SPECIAL PROGRAMS TO EASE THE
 * PANGS OF ERASABLE MEMORY PROGRAMS", the ways a program loaded into
 * erasable memory (a test program keyed in on the ground) calls the
 * program in fixed memory and comes back. The faithful C is
 * c64-port/src/standard_lead.c, standard_lead_interp.c, standard_lead_wake.c
 * and standard_lead_routines.c.
 *
 *   E/BKCALL  a BANKCALL to fixed memory that returns to erasable;
 *   E/CALL    a CALL of an interpretive subroutine in fixed memory from
 *             interpretive code in erasable (its CALL and EXIT are
 *             STANDARD_LEAD_CODE, run by the readable interpreter);
 *   E/JOBWAK  JOBWAKE for a job asleep in erasable memory.
 *
 * The CM's file also has its system-test entry words, at fixed places in
 * bank 33: COMPVER, GTSCPSS1 and REDO.
 *
 * Each entry below starts with the machine as the TC (or the SWCALL return,
 * or the interpreter) leaves it at that word, and returns with Z where the
 * listing goes next. The binding is src/binding/standard_lead_routines.c. */
#ifndef READABLE_STANDARD_LEAD_H
#define READABLE_STANDARD_LEAD_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

/* E/CALL's interpretive words, CALL EBUF2 and EXIT. */
extern const struct interpretive_code STANDARD_LEAD_CODE;

void e_bkcall(void);
void e_bkcall_bad_return(void);     /* E/BKCALL +11: TC Q, V34, a bad stall */
void e_bkcall_proceed_return(void); /* E/BKCALL +12: proceed, a good stall */
void e_bkcall_enter_return(void);   /* E/BKCALL +13: enter or recycle */
void e_call(void);
void e_call_after_exit(void); /* E/CALL +12 */
void e_jobwak(void);
#if COMANCHE055
void compver(void);
void gtscpss1(void);
void redo(void);
#endif

#endif
