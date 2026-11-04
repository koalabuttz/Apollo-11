/* SPDX-License-Identifier: MIT */
/* R31.agc, both missions' (Luminary099 pp. 703-708, Comanche055 pp.
 * 505-510): R31, the rendezvous parameter display, verb 83. The faithful
 * C is c64-port/src/lm_r31_file.c (and the older lm_r31_routines.c, with
 * the same keys) and cm_r31.c.
 *
 * R31CALL, which verb 83 starts, starts a job at V83CALL, then waits
 * (DSPDELAY) until that job has its first values (bit 12 of EXTVBACT)
 * and starts the display, DISPN5X: verb 16 noun 54, the range, range rate
 * and theta, kept up to date until the astronaut answers it. The V83CALL
 * job extrapolates both vehicles to now, works out the range, range rate
 * and theta, and goes round again (REV83, the CM's V83) until the display
 * is answered.
 *
 * The interpretive code is in the listing's notation, run by the readable
 * interpreter; the entries below are the basic code's. Each one ends where
 * its code leaves it: at a TC out of the file (Z the routine, Q the
 * return), at a TC INTPRET (Z INTPRET), or at a jump to another of these
 * (Z its label), as the faithful port's older LM module steps them. The
 * binding, src/binding/r31_routines.c, runs on from a jump or into the
 * interpreter where the faithful files' routines do. */
#ifndef READABLE_R31_H
#define READABLE_R31_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

/* Both missions'. */
void r31call(void);       /* R31CALL */
void dspdelay(void);      /* DSPDELAY */
void dspdelay_wake(void); /* the word after DSPDELAY's wait */
void dispn5x(void);       /* DISPN5X */
void dispn5x_terminate(void);
void dispn5x_proceed(void);
void dispn5x_recycle(void);
void v83call(void); /* V83CALL */
void r31_after_exit(void);

#if LUMINARY099
extern const struct interpretive_code V83CALL_CODE, REV83_CODE, GETRVN_CODE, GETRVN_EXIT_CODE,
    R31SURF_CODE, ONEBASE_CODE;

void r31_taskover(void); /* where NOVAC returns, in the task */
void rev83(void);        /* REV83 */
void getrvn(void);       /* GETRVN */
void getrvn_intpret(void);
void getrvn_after_exit(void);
void getrvn_after_exit_intpret(void);
void r31surf(void); /* R31SURF */
void mung(void);    /* MUNG? */
void onebase(void); /* ONEBASE */
#else
extern const struct interpretive_code V83_CODE, V83CALL_CODE, COMPDISP_CODE, STATEXTP_CODE,
    R34LOS_CODE;

void dispn5x_r34(void);       /* DISPN5X +6: R34's noun 53 */
void v83(void);               /* V83 */
void r34los_after_exit(void); /* R34LOS +1 */
#endif

#endif
