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
 *   and GOTOV56 (pp. 631-632).
 * The binding (src/binding/cm_p20_p25_routines.c) runs the keys these
 * parts write, and the faithful port's routines at the others.
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

#endif

#endif
