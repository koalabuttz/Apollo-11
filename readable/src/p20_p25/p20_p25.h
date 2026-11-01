/* SPDX-License-Identifier: MIT */
/* P20-P25.agc (Luminary099 pp. 492-613): the LM's rendezvous navigation
 * program P20 (P22's, on the lunar surface), its preferred tracking
 * attitude program P25, the rendezvous radar routines they run (R21
 * designate, R22 data read, R23 manual acquisition, R24 search, R25
 * gimbal monitor), the landing radar's read routines, the radar's own
 * tests and the LOS and landing site routines. The faithful C is
 * c64-port/src/lm_p20_p25.c, which the listing compiles to: its routine
 * tables have 1,028 keys.
 *
 * The file is written part by part, in the listing's order (README.md):
 *   part 1, programs.c: P20 and P25 themselves (pp. 492-503), with the
 *     state vector update UPPSV and ORBCHG3, P22's CSM orbit change;
 *   r22_r21.c: so far only LUNSFCHK (p. 507), which part 1 calls;
 *   r23_r24.c: so far only R23LEM's first step (p. 513), which part 1
 *     runs.
 * The binding (src/binding/p20_p25_routines.c) runs the keys these parts
 * write, and the faithful port's routines at the others.
 *
 * Basic code round stretches of interpretive code, which are in the
 * listing's notation (src/interpreter/notation.h). The entries below are
 * the basic code's: each program's start, the words BANKCALL, PHASCHNG,
 * 2PHSCHNG, PRIOCHNG, UPFLAG, DOWNFLAG, FALTON, TWIDDLE, FIXDELAY and
 * FINDVAC return to, the displays' returns (terminate, proceed, enter and
 * PRIOLARM's immediate return), the tasks and jobs TWIDDLE, FIXDELAY and
 * FINDVAC start, and the words after each EXIT. */
#ifndef READABLE_P20_P25_H
#define READABLE_P20_P25_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if LUMINARY099

/* ---- Part 1: P20 and P25 (programs.c) ---- */

extern const struct interpretive_code ORBCHG2_CODE;
extern const struct interpretive_code P20LEM1_CODE;
extern const struct interpretive_code P20LEMC3_CODE;
extern const struct interpretive_code UPPSV_CODE;
extern const struct interpretive_code ORBCHG3_CODE;

/* P20 (P22) */
void prog20(void);
void prog20_after_2phschng(void);
void prog20_on_surface(void);
void prog20_in_orbit(void);
void orbchgo(void);
void orbchgo_after_upflag(void);
void orbchgo_terminate(void);
void orbchgo_proceed(void);
void orbchgo_enter(void);
void orbchg1(void);
void orbchg1_terminate(void);
void orbchg1_proceed(void);
void orbchg1_enter(void);
void orbchg2(void);
void orbchg3_after_exit(void);
void prog20a(void);
void prog20a_after_r02both(void);
void prog20a_after_updatflg(void);
void prog20a_after_trackflg(void);
void prog20a_after_rndvzflg(void);
void prog20a_after_srchoptn(void);
void prog20a_after_acmodflg(void);
void prog20a_after_r04flag(void);
void prog20a_after_norrmon(void);
void prog20a_after_loscmflg(void);
void p20lem1(void);
void p20lem1_after_phaschng(void);
void p20lem1_after_exit(void);
void p20lem1_in_range(void);
void five_twenty_six_alarm(void);
void five_twenty_six_alarm_terminate(void);
void five_twenty_six_alarm_proceed(void);
void five_twenty_six_alarm_enter(void);
void five_twenty_six_alarm_immediate_return(void);
void p20lema_after_phaschng(void);
void p20lema_on_surface(void);
void p20lema_in_orbit(void);
void p20lemb(void);
void p20lemb_after_phaschng(void);
void p20lemb_after_priochng(void);
void p20lemb5(void); /* R22 goes on here too */
void p20lemb5_terminate(void);
void p20lemb5_proceed(void);
void p20lemb5_enter(void);
void p20lemb5_immediate_return(void);
void p20lemb6_terminate(void);
void p20lemb6_proceed(void);
void p20lemb6_enter(void);
void p20lemb6_on_surface(void);
void p20lemb6_in_orbit(void);
void p20lemb1(void);
void p20lemb1_after_upflag(void);
void p20lemb2(void);
void p20lemb2_after_falton(void);
void p20lemb3(void);
void p20lemb3_after_srchoptn(void);
void p20lemb3_after_acmodflg(void);
void p20lemb4(void);
void p20lemb4_after_delayjob(void);
void p20lemc3(void);
void p20lemc(void);
void p20lemc_after_phaschng(void);
void p20lemwt(void);
void p20lemwt_plus1(void); /* R22WAIT goes on here */
void p20lemwt_after_twiddle(void);
void p20lmwt1_after_phaschng(void);
void p20lemc1(void);
void p20lemc1_after_findvac(void);
void p20lemc2_after_fixdelay(void);
void p20lemd(void);
void p20lemd_after_twiddle(void);
void p20lemd1(void);
void p20lemd1_after_fixdelay(void);
void p20lemd2(void);
void p20lemd2_after_findvac(void);

/* P25 */
void prog25(void);
void prog25_after_2phschng(void);
void prog25_after_r02both(void);
void prog25_after_trackflg(void);
void p25lem1(void);
void p25lem1_after_phaschng(void);
void p25lem1_after_r65lem(void);
void p25lemwt(void);
void p25lmwt1(void);
void p25lmwt1_after_twiddle(void);
void p25lem2(void);
void p25lem2_after_findvac(void);

/* ---- R22 and R21 (r22_r21.c) ---- */

void lunsfchk(void);

/* ---- R23 and R24 (r23_r24.c) ---- */

void r23lem(void);

#endif

#endif
