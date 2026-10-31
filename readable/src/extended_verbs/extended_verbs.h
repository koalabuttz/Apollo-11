/* SPDX-License-Identifier: MIT */
/* EXTENDED_VERBS.agc: the extended verbs, V40 to V99 (Luminary099 pages
 * 262-300, Comanche055 pages 236-...). The faithful C is the port's
 * c64-port/src/lanec_ev_*.c, one file for each part of the listing's.
 *
 * PINBALL hands a verb of 40 or more to GOEXTVB with the verb less 40 in
 * MPAC; GOEXTVB jumps through the fan LST2FAN to the verb's routine. Most
 * of those that use the DSKY first claim the extended verb display system
 * (TESTXACT: the operator error light if another extended verb or a
 * priority display has it) and give it back when done (ENDEXT). ALM/END
 * lights OPERATOR ERROR and goes back to PINBALL (GOPIN).
 *
 * This header grows with the parts written. Part 1, the front end (pages
 * 262-264 of the LM's listing, 236-238 of the CM's): the fan, TESTXACT,
 * SETXTACT, XACTALM, XACT0, ALM/END, GOPIN and the LM's CHKPOOH. The
 * entries are the routines' labels and the words a call out of the file
 * returns to. All of it is basic code. */
#ifndef READABLE_EXTENDED_VERBS_H
#define READABLE_EXTENDED_VERBS_H

#include "../agc/agc.h"

/* ---- Part 1: the front end (front.c) ---- */
void goextvb(void);
void testxact(void);
void setxtact(void);
void setxtact_after_nvsub_busy(void); /* SETXTACT +5: NVSUB found the DSKY busy */
void setxtact_after_nvsub(void);      /* SETXTACT +6 */
void xactalm(void);
void xactalm_after_falton(void); /* XACTALM +1 */
void xact0(void);
void alm_end(void); /* ALM/END */
void gopin(void);   /* also where FALTON returns from ALM/END */
#if LUMINARY099
void chkpooh(void);
#endif

#endif
