/* SPDX-License-Identifier: MIT */
/* P40-P47.agc, the LM's (Luminary099 pp. 752-784): the burn programs. P40
 * for a burn of the descent engine (DPS), P42 for one of the ascent
 * engine (APS), P41 for one of the RCS jets, which the crew flies by hand
 * from the displays; P47, the thrust monitor. The faithful C is
 * c64-port/src/lm_p40_p47.c.
 *
 * Written part by part. Part 1 is the programs (programs.c, bank 36): P40,
 * P41, P42 and P47, their displays and their ends (POSTBURN, TIGNOW,
 * TERM40); then IMPLBURN and the engine off routines (engine_off.c), and
 * UPDATEVG, STEERING, NSTEER and GETDT (steering.c). Part 2 is bank 27's
 * subroutines S40.1, S40.2,3 and S40.8 (s40.c), S40.13 and S40.9
 * (timeburn.c); RASTEER1, TRIMGIMB and S41.1 are the part to come.
 *
 * The interpretive code is in the listing's notation, run by the readable
 * interpreter. The entries below are the basic code's: each program's
 * start, the words a routine outside the file comes back to, and the
 * words after each EXIT. The binding is src/binding/p40_routines.c. */
#ifndef READABLE_P40_P47_H
#define READABLE_P40_P47_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if LUMINARY099

/* ---- Part 1: the programs (programs.c) ---- */

extern const struct interpretive_code P40_CODE, P41_CODE, P41_VGBODY_CODE, DYNMDISP_CODE,
    CALCN85_CODE, P42_CODE, P47_CODE, CALCN83_CODE, CALCN83_DELVCTL_CODE, P47BODY_CODE;

void p40lm(void);                  /* P40LM */
void p40lm_after_phaschng(void);   /* P40LM +2 */
void p40lm_after_r02both(void);    /* P40LM +012 */
void p40lm_after_exit(void);       /* the INHINT after P40_CODE's EXIT */
void p40lm_after_pflitedb(void);   /* IBNKCALL's return: TC P40SXT4 */
void p40lm_burn(void);             /* P40SXT4's return: TCF BURNBABY */
void p40sxt4_after_downflag(void); /* P40SXT4 +5: R60LEM */
void p40sxt4_after_r60lem(void);   /* P40SXT4 +7: TC P40/RET */
void postburn(void);               /* POSTBURN */
void postburn_terminate(void);     /* GOFLASHR's returns */
void postburn_proceed(void);
void postburn_enter(void);
void p40phs1(void);                /* P40PHS1, POSTBURN's return */
void p40phs1_after_phaschng(void); /* P40PHS1 +2 */
void tignow(void);                 /* TIGNOW */
void tignow_after_zatteror(void);  /* TIGNOW +3 */
void tignow_after_setmindb(void);  /* TIGNOW +5 */
void tignow_terminate(void);       /* REFLASHR's returns */
void tignow_proceed(void);
void tignow_enter(void);
void tignow_immediate(void);      /* REFLASHR's immediate return */
void term40_after_restordb(void); /* TERM40 +012 */
void p41lm(void);                 /* P41LM */
void p41lm_after_r02both(void);   /* P41LM +4, the TC INTPRET */
void p41lm_after_exit(void);      /* the INHINT after P41_CODE's EXIT */
void p41lm_after_zatteror(void);  /* IBNKCALL's returns */
void p41lm_after_setmindb(void);
void p41lm_after_p40sxt4(void);     /* the TC INTPRET for P41_VGBODY_CODE */
void p41lm_after_vgbody(void);      /* verb 16 noun 85, after the EXIT */
void p41lm_after_godspret(void);    /* DYNMDISP as a job */
void p41lm_after_findvac(void);     /* the restart groups */
void p41lm_burn(void);              /* 2PHSCHNG's return: TCF B*RNB*B* */
void redo6_7(void);                 /* REDO6.7 */
void redo6_7_after_godspret(void);  /* REDO6.7 +6 */
void dynmdisp(void);                /* DYNMDISP, PRIOCHNG's return */
void dynmdisp_after_exit(void);     /* DYNMDISP +011 */
void dynmdisp_after_delayjob(void); /* DYNMDISP +014 */
void calcn85(void);                 /* CALCN85 */
void calcn85_after_exit(void);      /* CALCN85 +010 */
void p42lm(void);                   /* P42LM */
void p42lm_after_phaschng(void);    /* P42LM +2 */
void p42stage(void);                /* P42STAGE */
void p42stage_after_r02both(void);  /* P42STAGE +2 */
void p47lm(void);                   /* P47LM */
void p47lm_after_r02both(void);     /* P47LM +2, the TC INTPRET */
void p47lm_after_midtoav2(void);    /* P47LM +5, CALRB's return */
void p47lm_after_twiddle(void);     /* P47LM +010 */
void startp47(void);                /* STARTP47 */
void startp47_after_phaschng(void); /* STARTP47 +3 */
void startp47_after_findvac(void);  /* STARTP47 +012 */
void calcn83(void);                 /* CALCN83 */
void calcn83_after_exit(void);      /* CALCN83 +011 */
void calcn83_after_phaschng(void);  /* CALCN83 +013, the TC INTPRET */
void calcn83_after_delvctl(void);   /* CALCN83 +020, after the EXIT */
void p47bod_terminate(void);        /* GOFLASHR's returns */
void p47bod_proceed(void);
void p47bod_enter(void);
void p47bod_immediate(void);
void p47body(void);            /* P47BODY */
void p47body_after_exit(void); /* P47BODY +6 */

/* ---- Part 1: the engine off routines (engine_off.c) ---- */

void implburn(void);                /* IMPLBURN */
void implburn_after_getdt(void);    /* IMPLBURN +2 */
void implburn_after_twiddle(void);  /* IMPLBURN +4 */
void implburn_after_ignflag(void);  /* IMPLBURN +6 */
void implburn_after_astnflag(void); /* IMPLBURN +010 */
void implburn_after_impulsw(void);  /* IMPLBURN +012 */
void implburn_after_phaschng(void); /* IMPLBURN +014 */
void implburn_after_fixdelay(void); /* IMPLBURN +016 */
void implburn_after_noullage(void); /* IMPLBURN +017 */
void engoftsk(void);                /* ENGOFTSK */
void engoftsk_after_enginoff(void); /* ENGOFTSK +2 */
void enginoff(void);                /* ENGINOFF */
void enginof2(void);                /* ENGINOF2 */
void enginof1(void);                /* ENGINOF1 */
void enginof4(void);                /* ENGINOF4 */
void enginof3(void);                /* ENGINOF3 */
void coastset(void);                /* COASTSET */
void coastset_after_allcoast(void); /* COASTSET +2 */

/* ---- Part 1: steering (steering.c) ---- */

extern const struct interpretive_code UPDATEVG_CODE, ENDSTEER_CODE, NO_9_CODE, STEERING_CODE;

void updatevg_after_exit(void);    /* 180SETUP +1, after the EXIT */
void updatevg_after_findvac(void); /* 180SETUP +010 */
void endsteer(void);               /* ENDSTEER, 2PHSCHNG's return */
void steering(void);               /* STEERING */
void nsteer(void);                 /* NSTEER, STEERING's EXIT */
void nsteer_after_stoprate(void);  /* NSTEER +017 */
void nsteer_after_impulsw(void);   /* NSTEER +021 */
void nsteer_after_idleflag(void);  /* NSTEER +023 */
void nsteer_after_tpagree(void);   /* NSTEER +033 */
void nsteer_after_getdt(void);     /* NSTEER +035 */
void nsteer_after_twiddle(void);   /* NSTEER +037 */
void nsteer_after_2phschng(void);  /* NSTEER +042 */
void getdt(void);                  /* GETDT */

/* ---- Part 2: S40.1, S40.2,3 and S40.8 (s40.c) ---- */

extern const struct interpretive_code S40_1_CODE, S40_2_3_CODE, S40_8_CODE, ALARMIT_CODE;

void alarmit_after_exit(void);  /* ALARMIT +1, after the EXIT */
void alarmit_after_alarm(void); /* ALARMIT +3 */

/* ---- Part 2: S40.13 and S40.9 (timeburn.c) ---- */

extern const struct interpretive_code S40_13_CODE, S40_13D_CODE, S40_130V_CODE, S40_138_CODE,
    APSTGO_CODE, S40_9_CODE;

void s40_13(void);                     /* S40.13 */
void s40_132_star(void);               /* S40.132*, after the EXIT */
void s40_132_star_after_tpagree(void); /* S40.132* +1 */
void s40_134(void);                    /* S40.134, after S40.133's EXIT */
void s40_134_after_phaschng(void);     /* S40.134 +2 */
void s40_130v_after_exit(void);        /* S40.130V +5 */
void storetgo_after_exit(void);        /* STORETGO +2 */
void s40_9(void);                      /* S40.9 */
void ends40_9_after_exit(void);        /* ENDS40.9 +1 */
void ends40_9_after_phaschng(void);    /* ENDS40.9 +3 */

#endif

#endif
