/* SPDX-License-Identifier: MIT */
/* DAPIDLER_PROGRAM.agc, the LM's (Luminary099 pages 1410-1420, banks 16
 * and 20): "THE DAPIDLER PROGRAM IS STARTED BY FRESH START AND RESTART." It
 * is the T5RUPT task while the DAP is off, every 100 milliseconds: it sets
 * up 1/ACCS once, watches the mode select switch, the IMU and PGNCS
 * control, starts the DAP (STARTDAP) when all are ready, and meanwhile
 * keeps the jets off and drives the FDAI attitude error needles (ALTDSPLY,
 * NEEDLER). The faithful C is c64-port/src/lm_dapidler.c.
 *
 * Each function below is an entry: a label, or the word after a TC where
 * that call returns (the name says which). A run ends at a TC out of the
 * file (NOVAC, IBNKCALL), a DTCB to the other bank's part, TCF RESUME, or
 * CHEKMORE's RETURN to its caller: the program's trampoline calls the
 * routine at the new Z. */
#ifndef READABLE_DAPIDLER_H
#define READABLE_DAPIDLER_H

#include "../agc/names.h"

#if LUMINARY099
void chekbits(void);                /* CHEKBITS (TC CHEKBITS from the DAP) */
void chekmore(void);                /* CHEKMORE (RETNMORE's DTCB) */
void dapidler(void);                /* DAPIDLER: the T5RUPT task */
void checkup(void);                 /* CHECKUP: where NOVAC returns */
void checkup_after_chekbits(void);  /* CHECKUP +1 */
void startdap_after_zatteror(void); /* STARTDAP +2 */
void moreidle_after_qerrcalc(void); /* MOREIDLE +2 */
void shutdown_after_calcperr(void); /* SHUTDOWN, where CALCPERR returns */
void altdsply(void);                /* ALTDSPLY, bank 20 (JUMPDSP's DTCB) */
#endif

#endif
