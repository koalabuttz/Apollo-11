/* SPDX-License-Identifier: MIT */
/* THE_LUNAR_LANDING.agc (Luminary099 pp. 785-792): P63's set-up, the
 * braking phase before the burn (bank 32), and P68, landing confirmation
 * (LANDJUNK, bank 31). The faithful C is c64-port/src/lm_lunar_landing.c.
 *
 * P63LM readies BURNBABY for P63 and runs the ignition algorithm, IGNALG:
 * it guesses the time of ignition from the landing time, integrates the
 * LM's state to it and runs the guidance (?GUIDSUB) there, and DDUMCALC
 * corrects the guess until it moves by less than DDUMCRIT. Then the
 * countdown clock, R51's fine alignment (ASTNRET), the burn attitude
 * (P63SPOT2, R60LEM) and the landing radar's position, and on to
 * BURNBABY. LANDJUNK, after touchdown, holds the attitude, works out where
 * the LM is (LAT-LONG, R-TO-RP), shows it, and readies P57.
 *
 * Each entry below ends where its code leaves it: at a TC or TCF out of
 * the file (Z the routine, Q the return), or at a TC INTPRET (Z INTPRET).
 * The binding, src/binding/lunar_landing_routines.c, goes on from there
 * through the interpretive code while it is the file's, as the faithful
 * port does. */
#ifndef READABLE_LUNAR_LANDING_H
#define READABLE_LUNAR_LANDING_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if LUMINARY099
/* The interpretive code: P63's flags and IGNALG to IGNALOOP's ?GUIDSUB
 * (32,3020-3101); DDUMCALC's correction, DDUMGOOD and R60INIT
 * (32,3104-3201); ASTNRET and P63SPOT2 (32,3211-3222); LANDJUNK's
 * (31,2154-2177) and the P57 set-up after its display (31,2207-2213). */
extern const struct interpretive_code IGNALG_CODE, DDUMCALC_CODE, P63SPOT2_CODE, LANDJUNK_CODE,
    GSAV_CODE;

/* ---- P63: pp. 785-789 ---- */
void p63lm(void);                   /* P63LM */
void p63lm_after_phaschng(void);    /* its OCT 04024's return */
void p63lm_after_r02both(void);     /* R02BOTH's return: FLAGORGY TC INTPRET */
void ddumcalc(void);                /* DDUMCALC: ?GUIDSUB's return */
void ignalgrt(void);                /* IGNALGRT: R60INIT's EXIT */
void astnclok(void);                /* ASTNCLOK: the PHASCHNG's return */
void astnclok_after_stclok2(void);  /* STCLOK2's return */
void astnret(void);                 /* ASTNRET: the job the clock starts */
void p63spot2_after_exit(void);     /* P63SPOT2's EXIT */
void p63spot2_after_pflitedb(void); /* PFLITEDB's return */
void p63spot2_after_r60lem(void);   /* R60LEM's return */
void p63spot3(void);                /* P63SPOT3: PHASCHNG's return */
void p63spot3_terminate(void);      /* GOPERF1's returns */
void p63spot3_proceed(void);
void p63spot4(void);               /* P63SPOT4: and its ENTER */
void p63spot4_after_setpos1(void); /* SETPOS1's return */

/* ---- P68: pp. 791-792 ---- */
void landjunk(void);                /* LANDJUNK */
void landjunk_after_phaschng(void); /* its OCT 04024's return */
void landjunk_after_zatteror(void); /* ZATTEROR's return */
void landjunk_after_setmaxdb(void); /* SETMAXDB's return: TC INTPRET */
void landjunk_after_exit(void);     /* the EXIT after RLS: V06N43* */
void landjunk_terminate(void);      /* GOFLASH's returns */
void landjunk_proceed(void);
void landjunk_recycle(void);
void landjunk_after_refmf_exit(void); /* the EXIT after REFMF */
#endif

#endif
