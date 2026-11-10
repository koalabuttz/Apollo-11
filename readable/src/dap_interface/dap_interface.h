/* SPDX-License-Identifier: MIT */
/* DAP_INTERFACE_SUBROUTINES.agc, the LM's (Luminary099 pages 1406-1409,
 * bank 20): "A COLLECTION OF SUBROUTINES WHICH ALLOW MISSION CONTROL
 * PROGRAMS TO CONTROL THE MODE AND INTERFACE WITH THE DAP". The deadband
 * (SETMAXDB, SETMINDB, RESTORDB, PFLITEDB, each of which starts a job to
 * reposition the jet switch curves for it), the attitude errors and rate
 * commands zeroed (ZATTEROR, STOPRATE), and ALLCOAST, which also zeroes
 * the trim gimbal's angular accelerations. The faithful C is
 * c64-port/src/lm_dap_interface.c.
 *
 * Each function below is an entry: a label, or the word after a TC where
 * that call returns (the name says which). A run ends at a TC, to a
 * routine of this file or to NOVAC, or at a return to the caller: the
 * program's trampoline calls the routine at the new Z. */
#ifndef READABLE_DAP_INTERFACE_H
#define READABLE_DAP_INTERFACE_H

#include "../agc/names.h"

#if LUMINARY099
void restordb(void);                /* RESTORDB */
void restordb_plus1(void);          /* RESTORDB +1: ALLCOAST's TC, A given */
void setmaxdb(void);                /* SETMAXDB */
void setmindb(void);                /* SETMINDB */
void callaccs_after_novac(void);    /* CALLACCS +4: TC RUPTREG1 */
void pflitedb(void);                /* PFLITEDB */
void pflitedb_after_zatteror(void); /* PFLITEDB +3 */
void zatteror(void);                /* ZATTEROR: TC Q */
void stoprate(void);                /* STOPRATE: TC Q */
void allcoast(void);                /* ALLCOAST */
void allcoast_after_stoprate(void); /* ALLCOAST +3 */
void allcoast_after_restordb(void); /* ALLCOAST +021: TC RUPTREG2 */
#endif

#endif
