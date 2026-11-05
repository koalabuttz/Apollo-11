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
 * DOSPSOFF, SPSOFF, DOTVCRCS) and TVCZAP. Part 3 adds the steering
 * during the burn (steering.c, pp. 699-702: UPDATEVG, STEERING) and the
 * countdown's display with the crew's answers to it (clocktask.c, pp.
 * 703-709: CLOKTASK, CLOCKJOB, verbs 99 and 97, E7SETTER, E6SETTER).
 * Part 4 adds the burn's targeting, all interpretive (s40.c, pp.
 * 709-718): S40.1 with AGAIN and CALCUT, S40.2,3 and S41.1. Part 5
 * adds S40.8, the cross-product steering (s40.c, pp. 719-722). Part 6 adds
 * S40.9, the velocity to be gained of a Lambert burn, and S40.13, the
 * burn's time (timeburn.c, pp. 723-728). Part 7, the last, adds S40.6,
 * the gimbal drive test and trim, the autopilot's data (S41.2, S40.14,
 * DAPFIG) and the RCS autopilot's start (RCSUP, RCSDAPON) (dap_data.c,
 * pp. 729-736).
 *
 * Words of the listing no instruction names, so no C name either: the
 * constants 40CST5 (OCT 37730, "40 CS FOR THE T5 CLOCK", p. 689) and OCT53
 * (p. 690), SPBIT1 (00D, p. 719), POS-2.5 (OCT 37405) and RCSCADR (2CADR
 * RCSUP) (p. 735), and the label ENDS40.6 (p. 731), whose TCF TASKOVER the
 * code before it runs on into.
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
    P41_DSP_CODE, P40CNV85_CODE, CALCN85_CODE, P41CSM_CODE, P47CSM_CODE, CALCN83_CODE, P47BODY_CODE,
    UPDATEVG_CODE, CALL40_8_CODE, SURELY_9_CODE, STEERING_CODE, DYNDISP_CODE, S40_1_CODE,
    AGAIN_CODE, S40_2_3_CODE, S41_1_CODE, S40_8_CODE, LOTHRUST_TEST_CODE, TGOCALC_CODE, S40_81_CODE,
    S40_81_END_CODE, INCRSVG_CODE, INCRSVG_END_CODE, LOTHRUST_CODE, LOTHRUST_END_CODE, S40_9_CODE,
    ENDLAMB_CODE, COPY40_9_CODE, REDO40_9_CODE, S40_13_CODE, ULLAGE_CODE, S40_133_CODE,
    NOTADDUL_CODE;

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
void post41(void);                      /* POST41 */
void p40rcs(void);                      /* P40RCS */
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

/* ---- Part 3: the steering (steering.c) ---- */

void updatevg_after_exit(void);     /* after UPDATEVG_CODE's EXIT */
void updatevg_after_phaschng(void); /* UPDATEVG +021: TC INTPRET */
void surely_9_after_exit(void);     /* SURELY.9 +1: CAF PRIO10 */
void surely_9_after_findvac(void);  /* SURELY.9 +5: 2PHSCHNG */
void surely_9_after_2phschng(void); /* SURELY.9 +010: TC INTPRET */
void steering(void);                /* STEERING */
void steering_after_exit(void);     /* after STEERING_CODE's EXIT */
void steering_after_e7setter(void); /* STEERING +020: INHINT */
void steering_after_dpagree(void);  /* STEERING +030: CAE MPAC +1 */
void steering_after_waitlist(void); /* STEERING +045: 2PHSCHNG */
void steering_after_2phschng(void); /* STEERING +050: DOWNFLAG */
void steering_after_downflag(void); /* STEERING +052: TCF SERVXT */

/* ---- Part 3: the countdown's display, verbs 99 and 97 (clocktask.c) ---- */

void cloktask(void);                /* CLOKTASK */
void cloktask_after_phaschng(void); /* CLOKTASK +6: TASKOVER */
void setclock_after_waitlist(void); /* SETCLOCK +4: CCS NVWORD1 */
void settb6(void);                  /* SETTB6 */
void setupdyn_after_findvac(void);  /* SETUPDYN +4: TCF SETTB6 */
void dyndisp(void);                 /* DYNDISP */
void dyndisp_after_exit(void);      /* DYNDISP +4: TCF CKNVWRD1 */
void clockjob(void);                /* CLOCKJOB */
void cknvwrd1(void);                /* CKNVWRD1 */
void faildsp_terminate(void);       /* FAILDSP +3: TCF V97T */
void faildsp_proceed(void);         /* FAILDSP +4: TCF V97P */
void faildsp_enter(void);           /* FAILDSP +5: TCF V97E */
void engreqst_terminate(void);      /* ENGREQST +3: TCF V99T */
void engreqst_proceed(void);        /* ENGREQST +4: TCF V99P */
void engreqst_enter(void);          /* ENGREQST +5: TCF V99E */
void e7setter(void);                /* E7SETTER */
void e6setter(void);                /* E6SETTER */
void v99e(void);                    /* V99E */
void v99ejob(void);                 /* V99EJOB */
void v99ejob_after_tvczap(void);    /* V99EJOB +1: TCF P40RCS */
void v99t(void);                    /* V99T */
void v99tjob(void);                 /* V99TJOB */
void v99tjob_after_tvczap(void);    /* V99TJOB +1: TCF POST41 */
void v99p(void);                    /* V99P */
void v99p_tig(void);                /* V99P/TIG */
void v97t(void);                    /* V97T */
void v97t_after_2phschng(void);     /* V97T +3: CAF ONE */
void v97t_after_twiddle(void);      /* V97T +6: ENDOFJOB */
void v97ttask(void);                /* V97TTASK */
void v97ttask_after_phaschng(void); /* V97TTASK +010: TCR SPSOFF */
void v97ttask_after_spsoff(void);   /* V97TTASK +011: PHASCHNG */
void v97ttask_tailoff(void);        /* V97TTASK +013: FIXDELAY */
void v97trcs(void);                 /* V97TRCS */
void v97trcs_after_rcsdapon(void);  /* V97TRCS +2: CAF PRIO27 */
void endv97t(void);                 /* ENDV97T */
void v97p(void);                    /* V97P */
void v97p_after_phaschng(void);     /* V97P +2: CAF ONE */
void v97p_after_twiddle(void);      /* V97P +5: ENDOFJOB */
void v97ptask(void);                /* V97PTASK */
void v97ptask_after_idlefail(void); /* V97PTASK +6: UPFLAG */
void v97ptask_after_steersw(void);  /* V97PTASK +010: PHASCHNG */
void v97ptask_after_phaschng(void); /* V97PTASK +012: FIXDELAY */
void r40enabl(void);                /* R40ENABL */
void r40enabl_after_downflag(void); /* R40ENABL +2: PHASCHNG */
void endv97p(void);                 /* ENDV97P */
void v97e(void);                    /* V97E */
void v97e_after_phaschng(void);     /* V97E +2: CAF ONE */
void v97e_after_waitlist(void);     /* V97E +6: ENDOFJOB */
void v97etask(void);                /* V97ETASK */
void v97etask_after_e6setter(void); /* V97ETASK +5: CAF 3MDOT */
void spsoff97(void);                /* SPSOFF97 */
void spsoff97_after_spsoff(void);   /* SPSOFF97 +1: PHASCHNG */
void spsoff97_after_phaschng(void); /* SPSOFF97 +3: FIXDELAY */
void v97e40_6(void);                /* V97E40.6 */
void v97e40_6_after_waitlist(void); /* V97E40.6 +4: IBNKCALL */
void v97e40_6_after_rcsdapon(void); /* V97E40.6 +6: 2PHSCHNG */
void quickign(void);                /* QUICKIGN */
void v99flash(void);                /* V99FLASH */
void v99flash_after_2phschng(void); /* V99FLASH +5: CAF PRIO20 */
void endv97e(void);                 /* ENDV97E */

/* ---- Part 5: S40.8 (s40.c) ---- */

void s40_8_after_exit(void);         /* S40.8 +014: PHASCHNG */
void s40_8_after_phaschng(void);     /* S40.8 +016: TC INTPRET */
void lothrust_test_after_exit(void); /* TGOCALC -010: CAE DAPDATR1 */
void s40_81_after_exit(void);        /* S40.81 +5: CAF POSMAX */
void incrsvg_after_exit(void);       /* INCRSVG +1: TC ALARM */
void incrsvg_after_alarm(void);      /* INCRSVG +3: TC INTPRET */
void lothrust_after_exit(void);      /* LOTHRUST +6: CS ZERO */

/* ---- Part 6: S40.9 and S40.13 (timeburn.c) ---- */

void s40_9(void);                   /* S40.9 */
void s40_9_after_exit(void);        /* S40.9 +017: PHASCHNG */
void s40_9_after_phaschng(void);    /* S40.9 +022: TC INTPRET */
void mooncase_after_exit(void);     /* MOONCASE +1: PHASCHNG */
void copy40_9(void);                /* COPY40.9 */
void copy40_9_after_exit(void);     /* ENDS40.9 -2: CS ONE */
void ends40_9_after_phaschng(void); /* ENDS40.9 +2: TCF ENDOFJOB */
void redo40_9(void);                /* REDO40.9 */
void redo40_9_after_exit(void);     /* REDO40.9 +011: TCF ENDS40.9 -2 */
void s40_13(void);                  /* S40.13 */
void s40_13_after_exit(void);       /* S40.13 +7: CAF BIT7 */
void s40_132_after_exit(void);      /* S40.132 +1: TC TPAGREE */
void s40_132_after_tpagree(void);   /* S40.132 +2: CA MPAC */
void s40_133_after_exit(void);      /* S40.134: PHASCHNG */
void s40_134_after_phaschng(void);  /* S40.134 +2: TCF ENDOFJOB */

/* ---- Part 7: S40.6, S41.2, DAPFIG, S40.14, RCSUP (dap_data.c) ---- */

void pre40_6(void);                 /* PRE40.6 */
void s40_6(void);                   /* S40.6 */
void s40_6_after_fixdelay(void);    /* S40.6 +012: CAF BIT2 */
void rstrtst(void);                 /* RSTRTST */
void gimdtest_second(void);         /* GIMDTEST +2: CAF -4ACTDEG */
void gimdtest_third(void);          /* GIMDTEST +4: CAF +2ACTDEG */
void gimdtest_check(void);          /* GIMDTEST +6: CS CNTR */
void output_after_fixdelay(void);   /* OUTPUT +012: TC TEMPR60 */
void gimtrim_after_fixdelay(void);  /* GIMTRIM +2: CS ZERO */
void s41_2(void);                   /* S41.2 */
void dapfig(void);                  /* DAPFIG */
void rcsdapup_after_ibnkcall(void); /* RCSDAPUP +3: RELINT */
void nodapup_after_dapbit1(void);   /* NODAPUP +5: TC DOWNFLAG */
void nodapup_after_dapbit2(void);   /* NODAPUP +7: INHINT */
void nodapup_after_zerojet(void);   /* NODAPUP +012: RELINT */
void s40_14(void);                  /* S40.14 */
void rcsup(void);                   /* RCSUP */
void rcsup_after_rcsdapon(void);    /* RCSUP +4: TCF RESUME */
void rcsdapon(void);                /* RCSDAPON */
void rcsdapon_plus_1(void);         /* RCSDAPON +1 */

/* Words of programs.c the parts run on into. */
void ttgphs(void); /* TTGPHS, the countdown's restart group 4 */
void servxt(void); /* SERVXT, back to SERVICER */

#endif

#endif
