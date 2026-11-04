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
 * autopilot's deadbands (SETMINDB, SETMAXDB). Part 2 adds P47CSM, the
 * thrust monitor, and the countdown's TIG-30 (programs.c, p. 693), and
 * the burn itself (ignition.c, pp. 694-699): TIG-5, TIG-0, IGNITION,
 * the TVC's start (DOTVCON, DOSTRULL), IMPLBURN, the shutdown (ENGINOFF,
 * DOSPSOFF, SPSOFF, DOTVCRCS) and TVCZAP.
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
    P41_DSP_CODE, P40CNV85_CODE, CALCN85_CODE, P41CSM_CODE, P47CSM_CODE, CALCN83_CODE, P47BODY_CODE;

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

/* ---- Part 2: P47CSM and TIG-30 (programs.c) ---- */

void p47csm(void);                 /* P47CSM */
void p47csm_after_r02both(void);   /* P47CSM +2: TC INTPRET */
void p47csm_after_midtoav2(void);  /* P47CSM +5: CALRB's return */
void p47csm_after_waitlist(void);  /* P47CSM +012: PHASCHNG */
void p47csm_after_phaschng(void);  /* P47CSM +014: ENDOFJOB */
void tigon(void);                  /* TIGON */
void tigon_after_novac(void);      /* TIGON +7: CS BIT2 */
void calcn83(void);                /* CALCN83 */
void calcn83_after_exit(void);     /* after CALCN83_CODE's EXIT: PHASCHNG */
void calcn83_after_phaschng(void); /* CALCN83 +016: GENTRAN */
void calcn83_after_gentran(void);  /* CALCN83 +022: TC SERVXT */
void p47body(void);                /* P47BODY */
void p47bod(void);                 /* P47BOD, after P47BODY_CODE's EXIT */
void p47bod_after_priochng(void);  /* P47BOD +2: PHASCHNG */
void p47_dsp(void);                /* P47/DSP */
void p47_dsp_terminate(void);      /* P47/DSP +3: GOTOPOOH */
void p47_dsp_proceed(void);        /* P47/DSP +4: GOTOPOOH */
void p47_dsp_recycle(void);        /* P47/DSP +5: TCF P47BODY */
void tigaveg(void);                /* TIGAVEG */
void tigaveg_p41(void);            /* TIGAVEG +1: TCF P41REDSP */
void tigaveg_p40(void);            /* TIGAVEG +2 */
void tigaveg_after_waitlist(void); /* TIGAVEG +010: CS SIX */
void precheck(void);               /* PRECHECK */

/* ---- Part 2: the burn (ignition.c) ---- */

void tig_5(void);                   /* TIG-5 */
void tig_5_after_waitlist(void);    /* TIG-5 +4 */
void tig_5_after_2phschng(void);    /* TIG-5 +011: CAF PRIO20 */
void tig_5_after_findvac(void);     /* TIG-5 +015: TASKOVER */
void tig_0(void);                   /* TIG-0 */
void ignition(void);                /* IGNITION */
void preptvc_after_newphase(void);  /* PREPTVC +011: FIXDELAY */
void dotvcon(void);                 /* DOTVCON */
void dotvcon_after_newphase(void);  /* DOTVCON +022: CAF POSMAX */
void dostrull(void);                /* DOSTRULL */
void dostrull_after_steerull(void); /* DOSTRULL +4: TCR ULAGEOFF */
void dostrull_after_ulageoff(void); /* DOSTRULL +5: kill group 4 */
void implburn_after_e7setter(void); /* IMPLBURN +4 */
void implburn_after_2phschng(void); /* IMPLBURN +020: CAE TGO +1 */
void implcont(void);                /* IMPLCONT */
void implcont_after_e6setter(void); /* IMPLCONT +4: CAF ZERO */
void enginoff(void);                /* ENGINOFF */
void enginoff_after_e6setter(void); /* ENGINOFF +1: CAE CSMMASS */
void dospsoff(void);                /* DOSPSOFF */
void dospsoff_after_spsoff(void);   /* DOSPSOFF +1: CS OCT27/24 */
void dospsoff_after_newphase(void); /* DOSPSOFF +4: FIXDELAY */
void dotvcrcs(void);                /* DOTVCRCS */
void dotvcrcs_after_setmaxdb(void); /* DOTVCRCS +1: IBNKCALL RCSDAPON */
void dotvcrcs_after_rcsdapon(void); /* DOTVCRCS +3: IBNKCALL MASSPROP */
void dotvcrcs_after_massprop(void); /* DOTVCRCS +5: TCR TVCZAP */
void dotvcrcs_after_tvczap(void);   /* DOTVCRCS +6: PHASCHNG */
void dotvcrcs_after_phaschng(void); /* DOTVCRCS +010: CAF PRIO12 */
void dotvcrcs_after_novac(void);    /* DOTVCRCS +014: TASKOVER */
void spsoff(void);                  /* SPSOFF */
void tvczap(void);                  /* TVCZAP */

/* Words of later parts the ones above run on into. */
void ttgphs(void); /* TTGPHS, the countdown's restart group 4 */
void servxt(void); /* SERVXT, back to SERVICER */

#endif

#endif
