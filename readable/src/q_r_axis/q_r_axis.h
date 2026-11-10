/* SPDX-License-Identifier: MIT */
/* Q_R-AXIS_RCS_AUTOPILOT.agc, the LM's (Luminary099 pages 1442-1459, bank
 * 17): the RCS autopilot's Q and R axes, which P-AXIS's T5RUPT reaches by
 * DTCB to CALLQERR. It finds the Q,R attitude errors (QERRCALC), chooses
 * the control for this pass (the trim gimbal, GTS, or the jets), the X
 * translation the hand controller asks for, and for each of the U and V
 * axes (the jets' own axes, Q and R turned by ROT-TOUV) the minimum
 * impulse, rate command or automatic mode's jet time (TJETLAW, or SPSRCS
 * docked) and the jets to fire for it. Then CLOSEOUT: the interrupted job
 * resumes. The faithful C is c64-port/src/lm_q_r_axis.c.
 *
 * Each function below is an entry: a label, the word an offset from one
 * names, or the word after a TC where that call returns (the name says
 * which). A run ends at a TC out of the file (ALARM, IBNKCALL, TJETLAW,
 * WRITEP, WRITEU -1, WRITEV -1), GOTOGTS's DTCB, CLOSEOUT's TCF NOQRSM, or
 * at the word another entry starts at, as the faithful port's runs do: the
 * program's trampoline then calls the routine at the new Z. QERRCALC,
 * JTLST and ROT-TOUV are subroutines: they return by TC Q. */
#ifndef READABLE_Q_R_AXIS_H
#define READABLE_Q_R_AXIS_H

#include "../agc/names.h"

#if LUMINARY099
void callqerr(void);                /* CALLQERR: from P-AXIS */
void rcs(void);                     /* RCS */
void rcs_plus1(void);               /* RCS +1 */
void senseget_after_alarm(void);    /* 17,2172: X-translation's ALARM returns */
void qrcontrl(void);                /* QRCONTRL */
void qrcontrl_after_zatteror(void); /* 17,2214: the minimum impulse mode */
void minrtn(void);                  /* MINRTN */
void chekstik(void);                /* CHEKSTIK: the rate command mode */
void enteruv_after_zatteror(void);  /* 17,2454 */
void backhand(void);                /* BACKHAND */
void trygts(void);                  /* TRYGTS */
void chkingts_after_timegmbl(void); /* 17,2620 */
void qerrcalc(void);                /* QERRCALC, a subroutine */
void stillrcs(void);                /* STILLRCS (ATTSTEER) */
void tjlaw_plus3(void);             /* TJLAW +3 */
void tjlaw_after_tjetlaw(void);     /* 17,2714 */
void tjlaw_after_spsrcs(void);      /* 17,2724 */
void dorotat_after_writep(void);    /* 17,2775 */
void norotat_after_writep(void);    /* 17,3004 */
void looper(void);                  /* LOOPER */
void doskip(void);                  /* DOSKIP */
void doskip_plus1(void);            /* DOSKIP +1 */
void notmin_after_writep(void);     /* 17,3042 */
void xtrans(void);                  /* XTRANS */
void xtrans_after_writeu(void);     /* 17,3074 */
void xtrans_after_writev(void);     /* 17,3103 */
void jtlst(void);                   /* JTLST, a subroutine */
void rot_touv(void);                /* ROT-TOUV, a subroutine */
void selctsub_after_alarm(void);    /* 17,3224 */
void closeout(void);                /* CLOSEOUT: from GTS */
#endif

#endif
