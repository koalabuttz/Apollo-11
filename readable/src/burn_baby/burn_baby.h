/* SPDX-License-Identifier: MIT */
/* BURN,_BABY,_BURN_--_MASTER_IGNITION_ROUTINE.agc (Luminary099 pp. 731-751):
 * the LM's master ignition routine, which counts down to a burn's
 * ignition for P12, P40, P41, P42, P63 and the aborts, and KILLTASK. The
 * faithful C is c64-port/src/lm_burn_baby.c.
 *
 * Words of the listing no instruction names, so no C name either: ? (=
 * GOTOPOOH) and OCT20 (= BIT5), p. 749.
 *
 * The code is in bank 36, with P40AUTO in bank 35, TIGTASK1 and the zoom
 * tasks in bank 17, KILLTSK2 in bank 27 and KILLTASK in fixed-fixed. Each
 * entry below ends where its code leaves it: at a TC or TCF out of the file
 * (Z the routine, Q the return), at a TC INTPRET (Z INTPRET), or at a
 * transfer to an address the code computes (Z that address): INDEX WHICH;
 * TCF n into an ignition table, a TC Q, NULLCLOK's TC P40/RET and
 * KILLTSK2's DTCB. The binding, src/binding/burn_baby_routines.c, runs on
 * from there while the address is the file's, as the faithful port does. */
#ifndef READABLE_BURN_BABY_H
#define READABLE_BURN_BABY_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if LUMINARY099
/* The interpretive code: P41SPOT's (36,2152-2201) and the ABVEL display's
 * after CALLT-35 (36,2233-2236). */
extern const struct interpretive_code P41SPOT_CODE, ABVEL_CODE;

/* ---- burn_baby.c: pp. 731-743 ---- */
void p41table(void);                /* P41TABLE (5): TCF P41SPOT */
void burnbaby(void);                /* BURNBABY */
void burnbaby_after_phaschng(void); /* its OCT 04024's return */
void b_star_rnb_star_b_star(void);  /* B*RNB*B* */
void b_star_rnb_star_b_star_after_enginof3(void);
void p40spot(void);                        /* P40SPOT (P42SPOT, P12SPOT) */
void p41spot(void);                        /* P41SPOT (P63SPOT): TC INTPRET */
void gomidav_after_midtoav1(void);         /* MIDTOAV1's return */
void gomidav_after_midtoav1_slipped(void); /* its error return */
void callt_35(void);                       /* CALLT-35 */
void callt_35_after_longcall(void);
void callt_35_after_phaschng(void);
void callt_35_not_p63(void);    /* CHECKMM's returns */
void callt_35_p63(void);        /* (TC INTPRET) */
void callt_35_after_exit(void); /* the ABVEL display's EXIT */
void tig_35(void);              /* TIG-35 */
void tig_35_after_twiddle(void);
void tig_35_after_phaschng(void);
void tig_35_after_second_twiddle(void);
void tig_35_after_novac(void);
void p41blank(void); /* P41BLANK */
void p41blank_after_cleandsp(void);
void tig_30_1(void); /* TIG-30.1 */
void tig_30_1_after_novac(void);
void tig_30a(void); /* TIG-30A */
void tig_30(void);  /* TIG-30 */
void tig_30_after_twiddle(void);
void tig_30_after_second_twiddle(void);
void wantaps(void);  /* WANTAPS */
void ullgnot(void);  /* ULLGNOT */
void redo2_17(void); /* REDO2.17 */
void redo4_2(void);  /* REDO4.2 */
void ullgtask(void); /* ULLGTASK */
void ullgtask_after_onullage(void);
void ullgtask_after_phaschng(void);
void tig_5(void); /* TIG-5 */
void tig_5_after_twiddle(void);
void tig_5_after_downflag(void);
void tig_5_after_second_downflag(void);
void p40sjunk(void); /* P40SJUNK */
void p40sjunk_after_findvac(void);
void dispchng(void); /* DISPCHNG */
void common(void);   /* COMMON */
void common_after_phaschng(void);
void tig_0(void);         /* TIG-0 */
void tig_0_not_p63(void); /* CHECKMM's returns */
void tig_0_p63(void);
void tig_0_after_waitlist(void);
void ignyet_q(void); /* IGNYET? */
void ignition(void); /* IGNITION */
void p63ign(void);   /* P63IGN */
void p40ign(void);   /* P40IGN */
void p63ign1(void);  /* P63IGN1 */
void p63ign1_after_2phschng(void);
void p12ign(void);   /* P12IGN */
void abrtign(void);  /* ABRTIGN */
void p42ign(void);   /* P42IGN */
void dvmoncon(void); /* DVMONCON */
void dvmoncon_after_downflag(void);
void dvmoncon_after_second_downflag(void);
void dvmoncon_after_third_downflag(void);
void dvmoncon_after_phaschng(void);
void ullagoff(void); /* ULLAGOFF */
void waitabit(void); /* WAITABIT */
void tigtask(void);  /* TIGTASK */
void tigtask1(void); /* TIGTASK1 (bank 17) */
void tigtask1_after_novac(void);
void tigtask1_after_phaschng(void);
void p63zoom(void); /* P63ZOOM (bank 17) */
void p63zoom_after_flatout(void);
void p40zoom(void);  /* P40ZOOM (bank 17) */
void p40zooma(void); /* P40ZOOMA (ZOOM, bank 17) */
void p40zooma_after_phaschng(void);
void comfail(void); /* COMFAIL */
void comfail_after_upflag(void);
void comfail_after_second_upflag(void);
void comfail_after_stclok1(void); /* (and its TCF +3) */
void comfail_after_phaschng(void);
void comfail1(void); /* COMFAIL1 */
void comfail3(void); /* COMFAIL3 */
void comfail4(void); /* COMFAIL4 */
void comfail4_after_downflag(void);
void comfail4_after_second_downflag(void);
void comfail2(void); /* COMFAIL2 */
void comfail2_after_phaschng(void);
void comfail2_after_killtask(void);
void comfail2_after_enginof4(void);
void comfail2_after_upflag(void);
void comfail2_after_invflag(void);
void comfail2_after_second_upflag(void);
void comfail2_after_twiddle(void);
void invflag(void); /* INVFLAG */
void invflag_after_debit(void);
void noullage(void); /* NOULLAGE: TC Q */
void onullage(void); /* ONULLAGE: TC Q */

/* ---- clock.c: pp. 743-748 ---- */
void stclok1(void); /* STCLOK1 */
void stclok2(void); /* STCLOK2 */
void stclok3(void); /* STCLOK3 */
void stclok3_after_makecadr(void);
void stclok3_after_tpagree(void);
void stclok3_after_twiddle(void);
void stclok3_after_2phschng(void);
void cloktask(void); /* CLOKTASK */
void cloktask_after_novac(void);
void cloktask_after_fixdelay(void);
void killclok(void);                  /* KILLCLOK */
void clokjob(void);                   /* CLOKJOB */
void dispnot_minus35(void);           /* DISPNOT -35 (VB97DEX) */
void dispnot_minus35_terminate(void); /* CLOCPLAY's returns */
void dispnot_minus35_proceed(void);
void dispnot_minus35_enter(void);
void dispnot_minus25(void);           /* DISPNOT -25 (V06N61) */
void dispnot_minus25_terminate(void); /* REFLASH's returns */
void dispnot_minus25_proceed(void);
void dispnot_minus25_enter(void);
void dispnot_minus17(void);           /* DISPNOT -17 (CNTDNDEX) */
void dispnot_minus13(void);           /* DISPNOT -13 (VB99DEX, V99RECYC) */
void dispnot_minus13_terminate(void); /* CLOCPLAY's returns */
void dispnot_minus13_proceed(void);
void dispnot_minus13_enter(void);
void dispnot_minus2(void); /* DISPNOT -2 (BLANKDEX) */
void dispnot(void);        /* DISPNOT */
void stopclok(void);       /* STOPCLOK */
void stopclok_after_nullclok(void);
void nullclok(void); /* NULLCLOK */
void nullclok_after_noullage(void);
void nullclok_after_killtask(void);
void nullclok_after_phaschng(void); /* TC P40/RET */
void astnretn(void);                /* ASTNRETN */
void astnretn_after_phaschng(void);
void astnretn_after_findvac(void);
void star_proceed(void); /* *PROCEED */
void star_proceed_after_upflag(void);
void star_enter(void); /* *ENTER */
void gopost(void);     /* GOPOST */
void gopost_after_findvac(void);
void gopost_after_allcoast(void);
void gopost_after_nullclok(void);
void gopost_after_phaschng(void);
void gocutoff(void); /* GOCUTOFF */
void gocutoff_after_findvac(void);
void gocutoff_after_downflag(void);
void gocutoff_after_allcoast(void);
void gocutoff_after_nullclok(void);
void gocutoff_after_phaschng(void);
void ignite(void); /* IGNITE */
void ignite_after_twiddle(void);
void ignite1(void);            /* IGNITE1 */
void p40alm(void);             /* P40ALM */
void rep40alm(void);           /* REP40ALM */
void rep40alm_terminate(void); /* GOFLASH's returns */
void rep40alm_proceed(void);
void rep40alm_recycle(void);
void p40auto(void); /* P40AUTO (bank 35) */
void p40auto_after_makecadr(void);
void p40a_p(void); /* P40A/P */
void p40a_p_after_gpn_auto(void);
void turniton(void);           /* TURNITON */
void turniton_terminate(void); /* GOPERF1's returns */
void turniton_recycle(void);
void goback(void); /* GOBACK */

/* ---- killtask.c: pp. 750-751 ---- */
void killtask(void); /* KILLTASK (fixed-fixed) */
void killtsk2(void); /* KILLTSK2 (bank 27) */
void adrscan(void);  /* ADRSCAN */
void letitliv(void); /* LETITLIV */
void dead(void);     /* DEAD: DTCB */
void tstfbank(void); /* TSTFBANK */
void killdead(void); /* KILLDEAD */
#endif

#endif
