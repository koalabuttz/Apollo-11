/* SPDX-License-Identifier: MIT */
/* P20-P25.agc (Comanche055 pp. 562-634): the CM's rendezvous navigation
 * program P20, its orbital navigation program P22 and its target
 * pointing, with the routines they run (R57, R21 and R23 for the CSM,
 * R61CSM's preferred tracking attitude, R22's VHF ranging and sighting
 * marks, the orbital navigation routine S22, V89, P23's cislunar
 * navigation and the W-matrix monitor V67). The faithful C is
 * c64-port/src/cm_p20_p25.c, which the listing compiles to: its routine
 * tables have 1,128 keys.
 *
 * The file is written part by part, in the listing's order (README.md):
 *   part 1, p20.c: P20 itself (pp. 562-564);
 *   part 2, p22.c: P22 up to S22.1 (pp. 565-569);
 *   part 3, r57.c and r21_r23.c: R57, and R21 and R23 for the CSM
 *   (pp. 570-573);
 *   part 4, r61csm.c and bvectors.c: R61CSM, BVECTORS and GETUM
 *   (pp. 574-578);
 *   part 5, r22.c: R22 up to REND12 (pp. 579-582);
 *   part 6, r22_update.c and gotov56.c: R22 from REND12 (pp. 582-585),
 *   and GOTOV56 (pp. 631-632);
 *   part 7, r22_subroutines.c: R22's subroutines in bank 27, SETINTG to
 *   INITIALW (pp. 586-589);
 *   part 8, crs61.c: CRS61.1 and R63 (pp. 590-597);
 *   parts 9 to 12, s22.c, s22_landmark.c, s22_incorporate.c and
 *   s22_display.c: S22.1, the orbital navigation routine (pp. 598-610);
 *   part 13, w_nine_to_six.c: the W matrix from nine dimensions to six
 *   (pp. 610-614);
 *   part 14, v89.c: V89 (pp. 616-618);
 *   parts 15 to 17, p23.c, p23_marks.c and p23_incorporate.c: P23 (pp.
 *   619-626);
 *   part 18, horiz.c: HORIZ, DIVIDE and RECT.1 (pp. 627-630);
 *   part 19, p23_targets.c and v67.c: LOWMEMRY, LDPLANET and V67 (pp.
 *   631-634).
 * The binding (src/binding/cm_p20_p25_routines.c) runs every key of the
 * file.
 *
 * Seven of the listing's constants are in no readable file, because
 * nothing in Comanche055 refers to them: OCT203 (p. 564), FIRST3 (p. 564,
 * EQUALS FURST3), OC40200 (p. 587), MAT1B1 (p. 597), and TRUN19A,
 * VARSUBL3 and OCT00077 (p. 631). No key reaches them.
 *
 * Basic code round stretches of interpretive code, which are in the
 * listing's notation (src/interpreter/notation.h). The entries below are
 * the basic code's: each program's start, the words BANKCALL, 2PHSCHNG,
 * PRIOCHNG, UPFLAG, DOWNFLAG and FINDVAC return to, and the words after
 * each EXIT. */
#ifndef READABLE_P20_P25_CM_H
#define READABLE_P20_P25_CM_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if COMANCHE055

/* TC at, to a routine out of the file (or to a keyed label): Q gets the
 * return, and the routine runs next. */
static inline void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* ---- Part 1: P20 (p20.c) ---- */

extern const struct interpretive_code PROG20_CODE;
extern const struct interpretive_code P20R52JB_CODE;

void prog20(void);
void prog20_after_r02both(void);
void prog20_after_prftrkat(void);
void prog20_after_vehupflg(void);
void prog20_after_trackflg(void);
void prog20_after_updatflg(void);
void prog20_after_rndvzflg(void);
void prog20_after_2phschng(void);
void prog20_after_exit(void);
void prog20_after_findvac(void);
void pikup20(void);
void pikup20_after_priochng(void);
void pikup20_after_r61csm(void);
void p20r52jb(void);
void p20r52jb_after_exit(void);
void p20r52jb_after_mkrlees(void);

/* ---- Part 2: P22 (p22.c) ---- */

extern const struct interpretive_code PROG22_CODE;
extern const struct interpretive_code PROG22A_CODE;
extern const struct interpretive_code CALLR52_CODE;
extern const struct interpretive_code S22LSITE_CODE;
extern const struct interpretive_code P22SUBRB_CHECK_CODE;
extern const struct interpretive_code N89ERRX_3_CODE;
extern const struct interpretive_code S22ABDE_CODE;
extern const struct interpretive_code DE_GR_50_CODE;

void prog22(void);
void prog22_after_rndvzflg(void);
void prog22_after_r02both(void);
void prog22_after_exit(void);
void prog22_terminate(void);
void prog22_proceed(void);
void prog22_enter(void);
void prog22_immediate_return(void);
void prog22_after_blanket(void);
void callr52_after_exit(void);
void callr52_after_2phschng(void);
void callr52_after_targ2flg(void);
void callr52_after_targ1flg(void);
void p22over_after_exit(void);
void p22over_after_phaschng(void);
void p22subrb_after_exit(void);
void p22subrb_terminate(void);
void p22subrb_proceed(void);
void p22subrb_enter(void);
void p22subrb_check(void);
void n89errx_after_exit(void);
void n89errx_after_falton(void);
void s22dsppa(void);
void s22dsppa_terminate(void);
void s22dsppa_proceed(void);
void s22dsppa_recycle(void);
void s22dsppa_immediate_return(void);
void s22dsppa_after_blanket(void);
void s22dsppa_known(void);
void s22dsppa_after_knownflg(void);
void s22dspp_after_falton(void);
void s22dspp_plus2(void);
void s22dspp_plus3(void);
void s22dspp_known_landmark(void);
void s22dspp_after_knownflg(void);
void s22abde(void);
void de_gr_50_after_2phschng(void);
void de_gr_50_after_targ2flg(void);
void de_gr_50_after_targ1flg(void);

/* ---- Part 3: R57 (r57.c), R21 and R23 (r21_r23.c) ---- */

extern const struct interpretive_code R57_CODE;
extern const struct interpretive_code ENDR57_CODE;

void r57_after_exit(void);
void r57_not_set(void);
void r57a_after_v59flag(void);
void r57a_terminate(void);
void r57a_proceed(void);
void r57a_enter(void);
void r57a_immediate_return(void);
void r57a_after_blanket(void);
void markdisp(void);
void markdisp_terminate(void);
void markdisp_proceed(void);
void markdisp_enter(void);
void markdisp_immediate_return(void);
void markdisp_after_blanket(void);
void endr57_after_v59flag(void);
void endr57_after_novac(void);
void r21csm(void);
void r23csm(void);
void r23csm_terminate(void);
void r23csm_proceed(void);
void r23csm_enter(void);
void r23csm1(void);
void r23csm1_terminate(void);
void r23csm1_proceed(void);
void r23csm1_mark(void);
void r23csm1_after_gentran(void);
void r21c1(void);
void r21c1_terminate(void);
void r21c1_proceed(void);
void r21c1_recycle(void);
void r21csm1_after_gentran(void);
void r21end_after_kleenex(void);
void r21end_after_r21mark(void);

/* ---- Part 4: R61CSM (r61csm.c), BVECTORS and GETUM (bvectors.c) ---- */

extern const struct interpretive_code R61CSM_CODE;
extern const struct interpretive_code BVECTORS_CODE;
extern const struct interpretive_code GETUM_CODE;

void r61csm(void);
void r61csm_after_makecadr(void);
void r61csm_do_r61(void);
void r61csm_leave(void);
void r61csm_interpret(void);
void r61csm_after_exit(void);
void r61csm_no_maneuver(void);
void r61c1(void);
void r61c1_after_3axisflg(void);
void r61c1_after_pdspflag(void);
void r61c1_after_r60csm(void);
void r61c1_after_pdspflag_reset(void);
void r61c1_after_phaschng(void);
void r61end(void);

/* ---- Part 5: R22 up to REND12 (r22.c) ---- */

extern const struct interpretive_code R22_CODE;
extern const struct interpretive_code REND2_CODE;
extern const struct interpretive_code REND3_CODE;
extern const struct interpretive_code REND4_CODE;
extern const struct interpretive_code REND6_CODE;
extern const struct interpretive_code REND9_CODE;
extern const struct interpretive_code REND10_CODE;
extern const struct interpretive_code REND10A_CODE;

void r22(void);
void r22_after_priochng(void);
void rend1a_after_exit(void);
void rend2_after_gentran(void);

/* ---- Part 6: R22 from REND12 (r22_update.c), GOTOV56 (gotov56.c) ---- */

extern const struct interpretive_code REND12_CODE;
extern const struct interpretive_code REND14_CODE;
extern const struct interpretive_code CMPOS_CODE;
extern const struct interpretive_code RENDISP_CODE;
extern const struct interpretive_code RENDISP7_CODE;
extern const struct interpretive_code RENDISP3_CODE;
extern const struct interpretive_code RENDISP4_CODE;

void rendisp_after_exit(void);
void rendisp_optics(void);
void rendisp_after_phaschng(void);
void rendisp7(void);
void rendisp2(void);
void rendisp2_terminate(void);
void rendisp2_proceed(void);
void rendisp3_after_exit(void);
void gotov56(void);

/* ---- Part 7: R22's subroutines in bank 27 (r22_subroutines.c) ---- */

extern const struct interpretive_code SETINTG_CODE;
extern const struct interpretive_code CNTCHK_CODE;
extern const struct interpretive_code WAITONE_CODE;
extern const struct interpretive_code WAITONE_AGAIN_CODE;
extern const struct interpretive_code RANGERD_CODE;
extern const struct interpretive_code RANGERD4_CODE;
extern const struct interpretive_code RANGERD3_CODE;
extern const struct interpretive_code LIGHTON_CODE;
extern const struct interpretive_code SHIFTNDX_CODE;
extern const struct interpretive_code INITIALW_CODE;

void redor22(void);
void redor22_after_phaschng(void);
void redor22_after_priochng(void);
void waitone_after_exit(void);
void waitone_after_delayjob(void);
void rangerd_after_exit(void);
void rangerd_after_skipvhf(void);
void rangerd_bad(void);
void rangerd_good(void);
void rangerd1(void);
void rangerd1_plus_zero(void);
void rangerd1_negative(void);
void rangerd1_minus_zero(void);
void rangerd4(void);
void lighton_after_trfailon(void);
void vhfread(void);
void vhfgood(void);

/* ---- Part 8: CRS61.1 and R63 (crs61.c) ---- */

extern const struct interpretive_code CRS61_1_CODE;
extern const struct interpretive_code ASET_CODE;
extern const struct interpretive_code CDULOOP_CODE;
extern const struct interpretive_code STKTEST_CODE;
extern const struct interpretive_code STEP3CK_CODE;
extern const struct interpretive_code MANUEXIT_CODE;
extern const struct interpretive_code R63_CODE;

void crs61_1_after_exit(void);
void autock(void);
void dapck(void);
void aset(void);
void step2ck_after_upactoff(void);
void cduloop_after_exit(void);
void cduloop_done(void);
void stktest_after_exit(void);
void stktest_upacty(void);
void step3ck(void);
void crs61_2a_after_exit(void);
void manuexis(void);

/* ---- Part 9: the start of S22.1, and its subroutines in bank 31 (s22.c) ---- */

extern const struct interpretive_code S22_1_CODE;
extern const struct interpretive_code S22SHIFT_CODE;
extern const struct interpretive_code S2231X13_CODE;

void s22_1_after_exit(void);
void s22_1_after_mkreleas(void);
void s22_1_after_2phschng(void);

/* ---- Part 10: S22.1's first pass for an unknown landmark (s22_landmark.c) ---- */

extern const struct interpretive_code S22_UNKNOWN_CODE;

/* ---- Part 11: S22.1's next mark, and the incorporation (s22_incorporate.c) ---- */

extern const struct interpretive_code S22SAVET_CODE;
extern const struct interpretive_code S22NEXT_CODE;

void s22i_eq_n_after_exit(void);
void s22i_eq_n_after_phaschng(void);
void s22i_eq_n_next_mark(void);
void s22_display_after_exit(void);
void s22_display_terminate(void);
void s22_display_proceed(void);
void s22_display_enter(void);
void s22_display_immediate_return(void);
void s22_display_after_blanket(void);

/* ---- Part 12: S22.1's end, the landmark's position shown (s22_display.c) ---- */

extern const struct interpretive_code S22BOX42_CODE;
extern const struct interpretive_code S22F244_CODE;
extern const struct interpretive_code S22RLS_CODE;
extern const struct interpretive_code S22_981X_CODE;
extern const struct interpretive_code S22EXEX_CODE;
extern const struct interpretive_code S22GTP_CODE;
extern const struct interpretive_code S22F2410_CODE;

void s22exex(void);
void s22f244_after_exit(void);
void s22f244_terminate(void);
void s22f244_proceed(void);
void s22f244_recycle(void);
void s22_landing_site(void);
void s22_981x(void);
void s22gtp_after_exit(void);

/* ---- Part 13: the W matrix from nine dimensions to six (w_nine_to_six.c) ---- */

extern const struct interpretive_code NINE_DWTO6DW_CODE;
extern const struct interpretive_code ROWDOT_CODE;

/* ---- Part 14: V89, the CSM pointed at the LM (v89.c) ---- */

extern const struct interpretive_code V89RECL_CODE;

void v89call(void);
void v89call_after_r02both(void);
void v89_choose_terminate(void);
void v89_choose_proceed(void);
void v89_choose_data_in(void);
void v89_choice(void);
void v89recl(void);
void v89recl_after_exit(void);
void v89recl_terminate(void);
void v89recl_proceed(void);
void v89recl_recycle(void);
void v89_maneuver(void);
void v89_after_downflag(void);
void v89_after_r60csm(void);
void setpaf_after_upflag(void);

/* ---- Part 15: P23's start (p23.c) ---- */

extern const struct interpretive_code P23_CODE;
extern const struct interpretive_code P23_00_CODE;
extern const struct interpretive_code P23_17_CODE;
extern const struct interpretive_code P23_20_CODE;
extern const struct interpretive_code P23_30_CODE;
extern const struct interpretive_code P23_55_CODE;
extern const struct interpretive_code V94ENTER_CODE;
extern const struct interpretive_code R60CALL_CODE;

void p23(void);
void p23_after_downflag(void);
void p23_after_2phschng(void);
void p23_00(void);
void p23_06(void);
void p23_06_terminate(void);
void p23_06_proceed(void);
void p23_06_redisplay(void);
void p23_15(void);
void p23_15_landmark(void);
void p23_151(void);
void p23_152_moon(void);
void p23_152_after_upflag(void);
void p23_starcode(void);
void p23_17(void);
void p23_176(void);
void p23_176_far(void);
void p23_176_after_upflag(void);
void p23_18(void);
void p23_18_after_downflag(void);
void p23_20(void);
void p23_30(void);
void p23_31_after_exit(void);
void p23_30_terminate(void);
void p23_30_proceed(void);
void p23_55(void);
void v94enter(void);
void v94enter_after_exit(void);
void v94enter_after_downflag(void);
void r60call(void);
void r60call_after_r60csm(void);
void r60call_after_phaschng(void);
void r23_10_after_falton(void);

/* ---- Part 16: P23's marks and POINTAXS (p23_marks.c) ---- */

extern const struct interpretive_code P23_60_CODE;
extern const struct interpretive_code P23_65_CODE;
extern const struct interpretive_code P23_85_CODE;
extern const struct interpretive_code R23_2_CODE;

void p23_60_after_exit(void);
void p23_60_display(void);
void p23_60_terminate(void);
void p23_60_proceed(void);
void p23_60_redisplay(void);
void p23_65(void);
void p23_65_after_exit(void);
void r23_2_after_exit(void);
void r23_2_after_phaschng(void);

/* ---- Part 17: R23.55 to R23.END, the incorporation (p23_incorporate.c) ---- */

extern const struct interpretive_code R23_55_CODE;
extern const struct interpretive_code R23_55_TRUNNION_CODE;
extern const struct interpretive_code R23_8_CODE;

void r23_55_after_exit(void);
void r23_6(void);
void r23_6_terminate(void);
void r23_6_proceed(void);
void r23_6_enter(void);
void r23_6_blank(void);
void r23_6_after_blanket(void);
void r23_6_after_phaschng(void);
void r23_end(void);

/* ---- Part 18: HORIZ, DIVIDE and RECT.1 (horiz.c) ---- */

extern const struct interpretive_code HORIZ_CODE;
extern const struct interpretive_code DIVIDE_CODE;
extern const struct interpretive_code RECT_1_CODE;

/* ---- Part 19: LOWMEMRY and LDPLANET (p23_targets.c), and V67 (v67.c) ---- */

extern const struct interpretive_code LOWMEMRY_CODE;
extern const struct interpretive_code LDPLANET_CODE;
extern const struct interpretive_code LDPLANET_LOAD_CODE;
extern const struct interpretive_code V67CALL_CODE;
extern const struct interpretive_code V06N9933_CODE;
extern const struct interpretive_code V6N99INP_CODE;
extern const struct interpretive_code V67WW_CODE;

void ldplanet_after_exit(void);
void ldplanet_terminate(void);
void ldplanet_intpret(void);
void ldplanet_proceed(void);
void ldplanet_enter(void);
void v67call(void);
void v06n99ds(void);
void v06n99ds_terminate(void);
void v06n9933(void);
void v06n99ds_proceed(void);
void v06n99ds_loaded(void);
void v06n9933_after_exit(void);
void v67exitx_after_exit(void);
void v67xxx_after_exit(void);

#endif

#endif
