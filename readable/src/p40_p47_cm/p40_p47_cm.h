/* SPDX-License-Identifier: MIT */
/* P40-P47.agc, the CM's (Comanche055 pp. 684-736): the burn programs. P40
 * for a burn of the service propulsion system (SPS), P41 for one of the
 * RCS jets; P47, the thrust monitor. The faithful C is
 * c64-port/src/cm_p40_p47.c.
 *
 * Written part by part. Part 1 is the programs' start (programs.c, banks
 * 24 and 27, pp. 684-690): P40CSM and P41CSM, the burn attitude and the
 * gimbal test, the countdown's displays (P40S/SV, P41/DSP, P40CNV85,
 * CALCN85), the end of the burn (POSTBURN, TIGNOW, POST41) and the
 * autopilot's deadbands (SETMINDB, SETMAXDB).
 *
 * The interpretive code is in the listing's notation, run by the readable
 * interpreter. The entries below are the basic code's: each program's
 * start, the words a routine outside the file comes back to, and the
 * words after each EXIT. The binding is src/binding/cm_p40_routines.c. */
#ifndef READABLE_P40_P47_CM_H
#define READABLE_P40_P47_CM_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if COMANCHE055

/* ---- Part 1: the programs' start (programs.c) ---- */

extern const struct interpretive_code P40CSM_CODE, P40PVA_CODE, P40TTOG_CODE, P40S_SV_CODE,
    P41_DSP_CODE, P40CNV85_CODE, CALCN85_CODE, P41CSM_CODE;

void p40csm(void);                      /* P40CSM */
void p40csm_after_downflag(void);       /* P40CSM +2: TC INTPRET */
void p40csm_after_exit(void);           /* after P40CSM_CODE's EXIT: R02BOTH */
void p40pva(void);                      /* P40PVA */
void p40sxty(void);                     /* P40SXTY, after P40PVA_CODE's EXIT */
void p40sxty_after_setmindb(void);      /* P40SXTY +1: RELINT */
void p40sxty_after_r60csm(void);        /* P40SXTY +4: NBRCYCLS, TIMRFLAG */
void p40sxty_after_upflag(void);        /* P40SXTY +010: TC P41/P40 */
void p40sxty_p41(void);                 /* P40SXTY +011: TC P41/DSP */
void p40ttog(void);                     /* P40TTOG */
void p40ttog_after_exit(void);          /* after P40TTOG_CODE's EXIT: P40GMB */
void p40gmb_terminate(void);            /* P40GMB +3: V34 */
void p40gmb_proceed(void);              /* P40GMB +4: V33 */
void trimonly(void);                    /* TRIMONLY */
void trimonly_after_waitlist(void);     /* TRIMONLY +010: the delay */
void trimonly_after_delayjob(void);     /* TRIMONLY +016: 2PHSCHNG */
void p40s_rs(void);                     /* P40S/RS */
void p40s_rs_after_waitlist(void);      /* P40S/RS +4: RELINT */
void p40s_sv(void);                     /* P40S/SV */
void p40s_sv_after_e7setter(void);      /* P40S/SV +1: TC INTPRET */
void p40s_sv_after_midtoav1(void);      /* P40S/SV +010: CALRB's return */
void p40s_sv_after_midtoav1_fail(void); /* P40S/SV +011 */
void p40set(void);                      /* P40SET */
void p40set_after_longcall(void);       /* P40SET +013: PHASCHNG */
void p40set_after_phaschng(void);       /* P40SET +015: ENDOFJOB */
void p40blnkr(void);                    /* P40BLNKR */
void p40blnkr_after_cleandsp(void);     /* P40BLNKR +2 */
void postburn(void);                    /* POSTBURN */
void postburn_terminate(void);          /* POSTBURN +3: V34 */
void postburn_proceed(void);            /* POSTBURN +4 */
void postburn_recycle(void);            /* POSTBURN +5 */
void p40mindb(void);                    /* P40MINDB: DELAYJOB's return */
void p40mindb_after_setmindb(void);     /* P40MINDB +1: RELINT */
void tignow(void);                      /* TIGNOW */
void tignow_after_phaschng(void);       /* TIGNOW +3 */
void tignow_terminate(void);            /* TIGNOW +6: V34 */
void tignow_proceed(void);              /* TIGNOW +7: V33 */
void tignow_recycle(void);              /* TIGNOW +010: V32 */
void setmindb(void);                    /* SETMINDB */
void setmaxdb_inhint(void);             /* SETMAXDB -1 */
void setmaxdb(void);                    /* SETMAXDB */
void ttg_0(void);                       /* TTG/0 */
void p40clk(void);                      /* P40CLK */
void p40clk_after_downflag(void);       /* P40CLK +2: TASKOVER */
void p41_dsp_after_exit(void);          /* after P41_DSP_CODE's EXIT */
void p41_dsp_after_2phschng(void);      /* P41/DSP +014: TCF P40S/RS */
void p41redsp_after_waitlist(void);     /* P41REDSP +6: TCF TTGPHS */
void calcn85(void);                     /* CALCN85 */
void calcn85_after_exit(void);          /* after CALCN85_CODE's EXIT: TC SERVXT */
void tigblnk(void);                     /* TIGBLNK */
void tigblnk_after_waitlist(void);      /* TIGBLNK +4: P40BLNKR's job */
void tigblnk_after_novac(void);         /* TIGBLNK +012: CS OCT37 */
void p40tsk_after_newphase(void);       /* P40TSK +2: TASKOVER */
void p41csm(void);                      /* P41CSM */
void p41csm_after_upflag(void);         /* P41CSM +2: TC INTPRET */

/* Words of later parts the ones above run on into. */
void ttgphs(void); /* TTGPHS, the countdown's restart group 4 */
void servxt(void); /* SERVXT, back to SERVICER */

#endif

#endif
