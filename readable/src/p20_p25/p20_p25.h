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
 *   part 2, r22_r21.c: R22, the radar data read, and R21, the designate
 *     (pp. 504-512), with LUNSFCHK, which every part calls;
 *   part 3, r23_r24.c: R23, manual acquisition, and R24, the search
 *     (pp. 513-516), with CLRADMOD;
 *   part 4, r61.c: R61 and R65, the preferred tracking attitude
 *     (pp. 517-520), with READCDUD;
 *   part 5, rr_routines.c: the rendezvous radar's subroutines, MAGSUB to
 *     RRZ2 (pp. 521-537), with ENDRADAR, RDBADEND and RADNOOP;
 *   part 6, designate.c: the designate, RRDESSM to DODESEND (pp. 538-552).
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

/* TC at, to a routine out of the file (or to a keyed label): Q gets the
 * return, and the routine runs next. */
static inline void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* TC Q: the routine returns to the address in Q (bits 1-12), and Q
 * becomes 3. Not modelled: a Q that is not such an address (bits 13-15
 * set, or below 010). */
static inline void tc_q(void)
{
    word at = (word)(Q & 07777u);

    if ((Q & 070000u) || Q < 010u) not_modelled();
    Q = 3u;
    Z = at;
}

/* TC at, to an erasable word holding a return address (TC L, TC RRRET):
 * the word runs as the instruction TC <address>, so Z gets the address and
 * Q the word's address + 1. Not modelled: a word that is not such an
 * address (bits 13-15 set, or below 010). */
static inline void tc_return_in(word at, unsigned address)
{
    word to;

    tc_at(at); /* (which leaves L sign-corrected: a 15-bit word) */
    to = fetch(address);
    if ((to & 070000u) || to < 010u) not_modelled();
    Q = (word)(address + 1u);
    Z = to;
}

/* CCS A after a MASK of positive bits: true when a bit was set (A then
 * holds the masked word less one), false for +0. A MASK leaves neither a
 * negative word nor -0, so the CCS's other two ways are not modelled. */
static inline bool bit_set_in_A(void)
{
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: return true;
    case IS_PLUS_ZERO: return false;
    default: not_modelled(); return false;
    }
}

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
void p20lema(void); /* R22 and R21 go on here too */
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

/* ---- Part 2: R22 and R21 (r22_r21.c) ---- */

extern const struct interpretive_code R22LEM3_CODE;
extern const struct interpretive_code R22LEM96_CODE;
extern const struct interpretive_code R22LEM96_WAIT_CODE;
extern const struct interpretive_code R22LEM7_CODE;
extern const struct interpretive_code R21LEM1_CODE;
extern const struct interpretive_code R21LEM3_CODE;
extern const struct interpretive_code R21LEM12_CODE;
extern const struct interpretive_code R21LEM12_FIRST_PASS_CODE;
extern const struct interpretive_code R21LEM7_CODE;
extern const struct interpretive_code R21LEM8_CODE;

/* R22 */
void r22lem(void);
void r22lem_after_phaschng(void);
void r22lem_after_p20lemb5(void);
void r22lem_read(void);
void r22lem_after_read(void);
void r22lem_good_data(void);
void r22lem_no_data(void);
void r22lem_los_alarm(void);
void r22lem_los_alarm_terminate(void);
void r22lem_los_alarm_proceed(void);
void r22lem_los_alarm_enter(void);
void r22lem_los_alarm_immediate_return(void);
void r22lem1_after_phaschng(void);
void r22lem1_terminate(void);
void r22lem1_proceed(void);
void r22lem1_enter(void);
void r22lem2(void);
void r22lem2_after_phaschng(void);
void r22lem2_on_surface(void);
void r22lem2_in_orbit(void);
void r22lem2_after_boresight(void);
void r22lem2_boresight_in(void);
void r22lem2_boresight_out(void);
void r22lem2_after_r61lem(void);
void r22lem3(void);
void r22lem93_after_exit(void);
void r22lem93_after_phaschng(void);
void r22lem96_after_exit(void);
void r22lem96_after_phaschng(void);
void r22lem96_after_novac(void);
void r22lem96_recycle(void);
void r22lem96_on_surface(void);
void r22lem96_in_orbit(void);
void r22lem44_on_surface(void);
void r22lem44_in_orbit(void);
void r22lem42(void);
void r22lem42_on_surface(void);
void r22lem42_in_orbit(void);
void r22lem45_after_r65lem(void);
void r22wait(void);
void r22lem46(void);
void r22lem46_after_delayjob(void);
void n49dsp(void);
void n49dsp_terminate(void);
void n49dsp_proceed(void);
void r22rstrt(void);
void r22rstrt_after_phaschng(void);
void r22rstrt_after_rrrdot(void);
void r22rstrt_bad_read(void);
void r22rstrt_good_read(void);
void lunsfchk(void);

/* R21 */
void r21lem(void);
void r21lem_on_surface(void);
void r21lem_in_orbit(void);
void r21lem5(void);
void r21lem5_mode2_center(void);
void r21lem6_after_downflag(void);
void r21lem6_after_rrdesnb(void);
void r21lem6_stall(void);
void r21lem6_bad_return(void);
void r21lem10(void);
void r21lem10_after_upflag(void);
void r21lem2(void);
void r21lem3(void);
void r21lem3_after_upflag(void);
void r21lem3_after_downflag(void);
void r21lem3_not_in_coverage(void);
void r21lem3_maneuver(void);
void r21lem3_designating(void);
void r21lem3_bad_end(void);
void r21lem3_lockon(void);
void r21_503(void);
void r21_503_terminate(void);
void r21_503_proceed(void);
void r21_503_enter(void);
void r21_503_immediate_return(void);
void r21end_after_downflag(void);
void r21srch(void);
void r21srch_after_phaschng(void);
void r21lem4_after_upflag(void);
void r21lem4_after_downflag(void);
void r21lem12_not_in_coverage(void);
void r21lem12_maneuver(void);
void r21lem12_in_coverage(void);
void r21lem12_after_killtask(void);
void r21lem13_tried_enough(void);
void r21lem7(void);
void r21lem8_after_exit(void);
void r21lem8_after_cdesflag(void);
void r21lem8_after_lokonsw(void);
void r21lem8_after_norrmon(void);
void r21lem8_after_rrdesnb(void);
void r21lem8_delta_time(void);
void r21lem8_after_delta_time(void);
void r21lem8_after_longcall(void);
void r21lem9(void);
void r21lem9_after_killtask(void);
void r21lem9_after_clradmod(void);
void r21lem9_after_findvac(void);
void r21lem11_terminate(void);
void r21lem11_proceed(void);
void r21lem11_enter(void);
void r21lem11_immediate_return(void);
void r21disp_after_phaschng(void);
void r21disp_terminate(void);
void r21disp_proceed(void);
void r21disp_enter(void);
void r21disp_immediate_return(void);
void r21disp_after_linus(void);

/* ---- Part 3: R23 and R24 (r23_r24.c) ---- */

/* R23 */
void r23lem(void);
void r23lem_after_upflag(void);
void r23lem_after_setmindb(void);
void r23lem1_terminate(void);
void r23lem1_proceed(void);
void r23lem1_enter(void);
void r23lem11(void);
void r23lem11_out_of_limits(void);
void r23lem11_within_limits(void);
void r23lem11_after_restordb(void);
void r23lem11_after_downflag(void);
void outoflim(void);
void outoflim_terminate(void);
void outoflim_proceed(void);
void outoflim_enter(void);
void outoflim_immediate_return(void);
void r23lem2_after_downflag(void);
void r23lem3(void);
void r23lem3_after_r61lem(void);

/* R24 */
void r24lem(void);
void r24lem_after_upflag(void);
void r24lem1(void);
void r24lem2_after_phaschng(void);
void r24lem2_terminate(void);
void r24lem2_proceed(void);
void r24lem2_enter(void);
void r24lem2_immediate_return(void);
void r24end(void);
void r24end_after_killtask(void);
void r24end_after_clradmod(void);
void r24lem3_after_phaschng(void);
void r24lem3_after_killtask(void);
void r24lem3_after_clradmod(void);
void r24lem3_after_delayjob(void);
void r24lem3_on_surface(void);
void r24lem3_in_orbit(void);
void r24lem4(void);

/* CLRADMOD (fixed-fixed) */
void clradmod(void);

/* ---- Part 4: R61 and R65 (r61.c) ---- */

extern const struct interpretive_code R61C_L03_CODE;
extern const struct interpretive_code R61LEM1_AUTO_CODE;
extern const struct interpretive_code R61C_L05_CODE;

void r61lem(void);
void r61lem_after_makecadr(void);
void r61lem_after_upflag(void);
void r65lem(void);
void r65lem_after_makecadr(void);
void r61c_l01(void);
void r61lem1_after_exit(void);
void r61lem1_after_phaschng(void);
void r61lem1_after_auto_check(void);
void r61lem1_in_auto(void);
void r61lem2_after_exit(void);
void r61c_l05_after_exit(void);
void r61c_l05_after_zatteror(void);
void r61c_l05_after_setmindb(void);
void r61c_l05_after_downflag(void);
void r61c_l05_after_upflag(void);
void r61c_l05_after_r60lem(void);
void r61c_l05_after_restordb(void);
void r61c_l05_after_phaschng(void);
void r61c_l06(void);
void r61c_l06_r65(void);
void r61c_l06_count_out(void);
void r61c_l06_again(void);
void r61c_l06_after_twiddle(void);
void r61c_l2(void);
void r61c_l2_after_findvac(void);
void r61c_l04_after_ballangs(void);
void r61c_l4(void);
void r65wait(void);
void readcdud(void);

/* ---- Part 5: the rendezvous radar's subroutines (rr_routines.c) ---- */

/* MAGSUB to FLIP (fixed-fixed) */
void magsub(void);
void rrlimchk(void);
void rrlimchk_trunnion_out(void);
void rrlimchk_trunnion_in(void);
void rrlimchk_shaft_out(void);
void rrlimchk_shaft_in(void);
void mode2chk(void);
void mode2chk_shaft_out(void);
void mode2chk_shaft_in(void);
void rrlimok(void);
void mode2chk_trunnion_out(void);
void settrkf(void);
void flip(void);

/* RRTURNON to RRZ2 (bank 25) */
void rrturnon(void);
void rrturnon_after_rrzerosb(void);
void rrturnon_after_fixdelay(void);
void rrzerosb(void);
void rrzerosb_after_zeroing(void);
void rrzerosb_after_settling(void);
void rrzerosb_mode2(void);
void rrzerosb_mode1(void);
void rrzerosb_after_settrkf(void);
void dorrepos(void);
void dorrepos_after_setrrecr(void);
void dorrepos_trunnion(void);
void dorrepos_shaft(void);
void reposrpt(void);
void setrrecr(void);
void remode(void);
void remode_shaft(void);
void remode_trunnion_out(void);
void remode_trunnion_back(void);
void remode_shaft_back(void);
void remode_invert(void);
void remode_end(void);
void rmodinv(void);
void rrtonly(void);
void rrsonly(void);
void rr1ax2(void);
void rr1ax2_done(void);
void rr1ax2_command(void);
void rr1ax2_after_rrout(void);
void rrout(void);
void rrout2(void);
void setrrctr(void);
void rroutlim(void);
void rrzero(void);
void rrzero_after_waitlist(void);
void roadback(void);
void rrz2(void);
void rrz2_after_rrzerosb(void);
void rdbadend(void); /* (no keys: they run inside the routines that end at them) */
void endradar(void);

/* ---- Part 6: the designate (designate.c) ---- */

extern const struct interpretive_code RRDESSM_CODE;
extern const struct interpretive_code OKDESNB_CODE;
extern const struct interpretive_code DODES_CODE;
extern const struct interpretive_code DODES_URR_CODE;
extern const struct interpretive_code DORROUT_CODE;

/* RRDESSM, STARTDES and TRYSWS */
void rrdessm_after_exit(void);
void rrdessm_not_in_mode(void);
void okdessm(void);
void okdessm_after_startdes(void);
void lundesch(void);
void lundesch_not_rendezvous(void);
void startdes_after_setrrecr(void);
void desretrn(void);
void trysws_after_rmodinv(void);
void trysws_out_of_limits(void);
void trysws_within_limits(void);
void trysws_after_rmodinv2(void);
void nodessm_after_rmodinv(void);

/* RRDESNB and RRLIMNB */
void rrdesnb(void);
void rrdesnb_after_makecadr(void);
void rrdesnb_after_downflag(void);
void rrdesnb_not_in_mode(void);
void okdesnb(void);
void okdesnb_after_exit(void);
void tryswn_after_rmodinv(void);
void tryswn_out_of_limits(void);
void tryswn_within_limits(void);
void tryswn_after_rmodinv2(void);
void nodesnb_after_rmodinv(void);
void nodesnb_after_alarm(void);
void nodesnb_after_clradmod(void);
void rrlimnb_trunnion_out(void);
void rrlimnb_trunnion_in(void);
void rrlimnb_shaft_out(void);
void rrlimnb_shaft_in(void);
void negshaft(void);

/* BEGDES, STDESIG and DODES */
void begdes(void);
void begdes_remode(void);
void desloop(void);
void stdesig(void);
void moredes_after_findvac(void);
void dodes(void);
void dodes_after_exit(void);
void dodes_after_dot(void);
void rrdesdun_after_downflag(void);
void notp20(void);
void notp20_after_rrout(void);
void notp20_new_los(void);
void notp20_after_killtask(void);
void notp20_after_chang1(void);
void dodesend(void);

#endif

#endif
