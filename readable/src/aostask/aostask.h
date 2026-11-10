/* SPDX-License-Identifier: MIT */
/* AOSTASK_AND_AOSJOB.agc, the LM's (Luminary099 pages 1485-1506, bank 20,
 * EBANK= AOSQ): 1/ACCS, "THE INTERFACE BETWEEN THE GUIDANCE PROGRAMS AND
 * THE DIGITAL AUTOPILOT". From the mass, the configuration (ascent,
 * descent, docked), the deadband and the offset accelerations it gives
 * the DAP its jet accelerations and their reciprocals, the gimbal's
 * ACCDOTQ and ACCDOTR, and the deadbands of each axis.
 *
 *   1/ACCSET  null the offset estimates, then 1/ACCJOB
 *   1/ACCJOB  1/ACCS by BANKCALL; at +2 (its return) TC ENDOFJOB
 *   1/ACCS    EBANK 6, then at +2 (BANKCALL's entry) TC MAKECADR, and at
 *             +3 the work: the mass, the jet accelerations, ACCDOTQ and
 *             ACCDOTR, the gimbal switches; then TIMEGMBL by IBNKCALL, or
 *             DOCKTEST
 *   DOCKTEST  docked: 1/ACCRET; else 1/ACCONT, which leaves for ROT-TOUV
 *             by IBNKCALL; at GETAOSUV +6 (its return) the P axis and the
 *             U axis, to GENTRAN; at the U axis's return the V axis, to
 *             GENTRAN; at the V axis's return the deadbands, and 1/ACCRET:
 *             ACCSOKAY set and BANKJUMP to the caller
 *   INVERT    1 over -A, scaled at 2(7)/PI; TC Q
 *
 * DVOVSUB, 1/ANET-, 1/NETMIN and DO1/NET+, entered only by TC from the
 * file, are functions of it. The faithful C is c64-port/src/lm_aostask.c. */
#ifndef READABLE_AOSTASK_H
#define READABLE_AOSTASK_H

#include "../agc/names.h"

#if LUMINARY099
void one_accset(void);        /* 1/ACCSET: from fresh start and restarts */
void one_accjob(void);        /* 1/ACCJOB: a job's entry */
void one_accjob_return(void); /* 1/ACCJOB +2, where 1/ACCS returns */
void one_accs(void);          /* 1/ACCS: by BANKCALL */
void one_accs_plus2(void);    /* 1/ACCS +2: BANKCALL's entry */
void one_accs_plus3(void);    /* 1/ACCS +3, where MAKECADR returns */
void docktest(void);          /* DOCKTEST, where TIMEGMBL returns */
void getaosuv_plus6(void);    /* GETAOSUV +6, where ROT-TOUV returns */
void u_axis_stored(void);     /* the U axis's GENTRAN return */
void v_axis_stored(void);     /* the V axis's GENTRAN return */
void invert(void);            /* INVERT: by TC, Q the return */
#endif

#endif
