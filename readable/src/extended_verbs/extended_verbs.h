/* SPDX-License-Identifier: MIT */
/* EXTENDED_VERBS.agc: the extended verbs, V40 to V99 (Luminary099 pages
 * 262-300, Comanche055 pages 236-...). The faithful C is the port's
 * c64-port/src/lanec_ev_*.c, one file for each part of the listing's.
 *
 * PINBALL hands a verb of 40 or more to GOEXTVB with the verb less 40 in
 * MPAC; GOEXTVB jumps through the fan LST2FAN to the verb's routine. Most
 * of those that use the DSKY first claim the extended verb display system
 * (TESTXACT: the operator error light if another extended verb or a
 * priority display has it) and give it back when done (ENDEXT). ALM/END
 * lights OPERATOR ERROR and goes back to PINBALL (GOPIN).
 *
 * This header grows with the parts written. Part 1, the front end (pages
 * 262-264 of the LM's listing, 236-238 of the CM's): the fan, TESTXACT,
 * SETXTACT, XACTALM, XACT0, ALM/END, GOPIN and the LM's CHKPOOH. Part 2,
 * the short verbs (LM pp. 266-299, CM pp. 243-267): the flag verbs, the
 * job starters, the hand-overs to other banks, CKMODCAD, the CM's CHKPOOH
 * and EXDAPOFF, and SHOWSUM. Part 3, alignment and radar designate (LM
 * pp. 265-281, CM pp. 239-250): V40-V43, the LM's V44 and V60, OP/INERT,
 * the LM's RDRUSECK and the CM's CKLFTBTS. Part 4, the LM's radar test
 * verbs (pp. 276-279): R04 (V63), R77 (V78) and R77END (V79). The entries
 * are the routines' labels and the words a call out of the routine returns to. All of it is
 * basic code. */
#ifndef READABLE_EXTENDED_VERBS_H
#define READABLE_EXTENDED_VERBS_H

#include "../agc/agc.h"

/* ---- Part 1: the front end (front.c) ---- */
void goextvb(void);
void testxact(void);
void setxtact(void);
void setxtact_after_nvsub_busy(void); /* SETXTACT +5: NVSUB found the DSKY busy */
void setxtact_after_nvsub(void);      /* SETXTACT +6 */
void xactalm(void);
void xactalm_after_falton(void); /* XACTALM +1 */
void xact0(void);
void alm_end(void); /* ALM/END */
void gopin(void);   /* also where FALTON returns from ALM/END */
void chkpooh(void); /* the LM's in part 1, the CM's in part 2 */

/* ---- Part 2: the short verbs (short_verbs.c) ---- */
/* The flag verbs. */
void lemvec(void);
void lemvec_after_downflag(void);
void csmvec(void);
void wmatrxng(void);
void verb96(void);
void verb96_after_upflag(void);
#if LUMINARY099
void dapatter(void);
void dapatter_after_downflag(void);
void totatter(void);
void totatter_after_upflag(void);
void lron(void);
void lron_after_upflag(void);
void lroff(void);
void lroff_after_downflag(void);
void noupdown(void);
void noupdown_after_downflag(void);
void updatoff(void);
void updatoff_after_upflag(void);
void snuffout(void);
void snuffout_after_upflag(void);
void outsnuff(void);
void outsnuff_after_downflag(void);
void minimp(void);
void nominimp(void);
void nominimp_after_ibnkcall(void);
#else
void v61(void);
void v61_after_downflag(void);
void v62(void);
void v62_after_upflag(void);
void v62_after_second_upflag(void);
void v63(void);
void v63_after_upflag(void);
void v63_after_downflag(void);
void enatma(void);
void enatma_after_downflag(void);
void csmvec_after_upflag(void);
void lftflgon(void);
void lftflgon_after_upflag(void);
void wmatrxng_after_downflag(void);
void wmatrxng_after_second_downflag(void);
void setprflg(void);
void setprflg_after_upflag(void);
void resetprf(void);
void resetprf_after_downflag(void);
void verb96_after_second_upflag(void);
void setsurf(void);
void setsurf_after_upflag(void);
void restsrf(void);
void restsrf_after_downflag(void);
#endif
/* The job starters. */
void vb64(void);
void vb64_after_chkpooh(void);
void vb64_after_testxact(void);
void vb64_after_findvac(void);
void v83perf(void);
void v83perf_after_testxact(void);
void v89perf(void);
void v89perf_after_chkpooh(void);
void v89perf_after_testxact(void);
void v89perf_after_findvac(void);
void v90perf(void);
void v90perf_after_testxact(void);
void v90perf_after_findvac(void);
void crewmanu(void);
void crewmanu_after_chkpooh(void);
void crewmanu_after_testxact(void);
void crewmanu_after_findvac(void);
void systest(void);
void systest_after_chkpooh(void);
void attached(void);
void attached_after_findvac(void);
void v67(void);
void v67_after_testxact(void);
void v67_after_findvac(void);
#if LUMINARY099
void v47txact(void);
void v47txact_after_testxact(void);
void v47txact_after_findvac(void);
void v83perf_after_waitlist(void);
void systest_after_testxact(void);
void systest_after_findvac(void);
#else
void v83perf_plus5(void); /* V83PERF +5, where V85PERF goes on */
void v83perf_after_novac(void);
void v85perf(void);
void v85perf_after_testxact(void);
void v92cont(void);
void v92cont_after_exdapoff(void);
void v92cont_after_findvac(void);
void ckoptvb(void);
void ckoptvb_after_checkmm02_no(void);
void ckoptvb_after_checkmm02_yes(void);
void ckoptvb_after_findvac(void);
void chazfogc(void);
void chazfogc_after_checkmm02_no(void);
void chazfogc_after_checkmm02_yes(void);
void chazfogc_after_findvac(void);
void chazfogc_after_phaschng(void);
void strokon(void);
void strokon_after_novac(void);
#endif
/* The hand-overs. */
void goloadlv(void);
void goloadlv_after_flashoff(void);
void v82perf(void);
void v82perf_after_testxact(void);
void v82perf_after_priochng(void);
#if LUMINARY099
void alintime(void);
void alintime_after_testxact(void);
void trmtrack(void);
void trmtrack_after_downflag(void);
void trmtrack_after_second_downflag(void);
void trmtrack_after_third_downflag(void);
void dapdisp(void);
void dapdisp_after_testxact(void);
void dapdisp_after_priochng(void);
void verb85(void);
void verb85_after_testxact(void);
#else
void stablish(void);
void stablish_plus12(void); /* the listing's +3 */
void stablish_plus13(void); /* the listing's +8 */
void gotor21(void);
void gotor21_after_downflag(void);
void gotor23(void);
void gotor23_after_upflag(void);
void gotor23_after_testxact(void);
void gotor23_after_novac(void);
void regr21(void);
void regr21_after_novac(void);
void r22alarm(void);
void r22alarm_after_alarm(void);
void verb94(void);
void verb94_after_downflag(void);
void verb94_after_checkmm23_no(void);
void verb94_after_checkmm23_yes(void);
void verb94_after_phaschng(void);
#endif
/* The checks, and the small settings. */
void ckmodcad(void);
void ckmodcad_plus1(void); /* the CM's OPTCOARK enters here */
void dnedump(void);
#if COMANCHE055
void exdapoff(void);
void v60(void);
void v86perf(void);
void v52(void);
void v52_after_checkmm22_no(void);
void v52_after_checkmm22_yes(void);
#endif
/* SHOWSUM. */
void showsum(void);
void showsum_after_chkpooh(void);
void showsum_after_testxact(void);
#if LUMINARY099
void showsum_after_priochng(void);
#endif
void sdisplay(void);
void nokill(void);
void nokill_after_terminate(void); /* NOKILL +5, +6 and +7: GOXDSPF's returns */
void nokill_after_proceed(void);
void nokill_after_enter(void);
void nokill_plus8(void); /* where NOKILL +5's TC +3 goes */
void endsums(void);

/* ---- Part 3: alignment and radar designate (align.c) ---- */
void vbzero(void);
void vbzero_after_op_inert_imu(void);
void vbcoark(void);
void vbcoark_after_op_inert_imu(void);
void op_inert(void); /* OP/INERT */
void imuzerok(void);
void imuzerok_after_ckmodcad(void);
void imuzerok_after_imuzero(void);
void imuzerok_after_imustall_bad(void);
void imuzerok_after_imustall_good(void);
void imucoark(void);
void imucoark_after_ckmodcad(void);
void imucoark_after_testxact(void);
void imucoark_after_terminate(void); /* GOXDSPF's three returns */
void imucoark_after_proceed(void);
void icork2(void); /* and the enter */
void icork2_after_exdspret(void);
void icork2_after_imucoars(void);
void icork2_after_imustall_bad(void);
void icork2_after_imustall_good(void);
void imufinek(void);
void imufinek_after_ckmodcad(void);
void imufinek_after_testxact(void);
void imufinek_after_terminate(void);
void imufinek_after_proceed(void);
void imufinek_after_enter(void);
void imufinek_after_exdspret(void);
void imufinek_after_imufine(void);
void imufinek_after_imustall_bad(void);
void finek2(void); /* IMUSTALL's good return */
void finek2_after_imupulse(void);
void finek2_after_imustall_bad(void);
void finek2_after_imustall_good(void);
void imuattck(void);
void imuattck_after_chkpooh(void);
void imuattck_after_testxact(void);
void imuattck_after_terminate(void);
void imuattck_after_proceed(void);
void imuattck_after_enter(void);
#if LUMINARY099
void vbzero_after_op_inert_rr(void);
void vbcoark_after_op_inert_rr(void);
void rrzerok(void);
void rrzerok_after_rdruseck(void);
void rwaitk(void); /* where RRZERO returns */
void rwaitk_after_radstall_bad(void);
void rwaitk_after_radstall_good(void);
void lrpos2k(void);
void lrpos2k_after_rdruseck(void);
void lrpos2k_after_lrpos2(void);
void lrpos2k_after_radstall_bad(void);
void lrpos2k_after_radstall_good(void);
void lrp2alm(void);
void lrp2alm_after_alarm(void);
void rrdesnbk(void);
void rrdesnbk_after_rdruseck(void);
void rrdesnbk_after_testxact(void);
void rrdesnbk_plus10(void); /* where proceed asks again */
void rrdesnbk_after_terminate(void);
void rrdesnbk_after_proceed(void);
void rrdesnbk_after_enter(void);
void rrdesnbk_after_aurlokon(void);
void rrdesnbk_after_exdspret(void);
void rrdesnbk_after_findvac(void);
void rrdesk2(void);
void rrdesk2_after_rrdesnb(void);
void rrdesk2_plus3(void); /* where DESRETRN's INCR returns */
void rrdesk2_after_radstall_bad(void);
void rrdesk2_after_radstall_good(void);
void rrdesk2_plus14(void);
void rrdesk2_after_alarm(void);
void rrdesend(void);
void rrdesend_after_clradmod(void);
void rrdesend_after_delayjob(void);
void rrdesend_after_downflag(void);
void aurlokon(void); /* AURLOKON, NOLOKON and AURLKON1 are in bank 23 */
void aurlokon_after_makecadr(void);
void aurlokon_plus6(void);           /* the listing's -5 */
void aurlokon_after_terminate(void); /* GOMARKFR's three crew returns */
void aurlokon_after_proceed(void);
void aurlokon_after_recycle(void);
void aurlokon_immediate_return(void); /* and its immediate one */
void aurlokon_after_blanket(void);
void aurlokon_plus15(void); /* the listing's +5 */
void aurlokon_after_upflag(void);
void nolokon(void);
void nolokon_after_downflag(void);
void nolokon_after_upflag(void);
void aurlkon1(void);
void rdruseck(void);
void checkrr(void);
void checkrr_plus1(void); /* where RDRUSECK's last CCS skips to */
void ckrndbit(void);
void checkp22(void);
void imuattck_plus13(void);
void imuattck_after_exdspret(void);
void imuattck_after_waitlist(void);
#else
void vbzero_after_op_inert_optics(void);
void vbcoark_after_op_inert_optics(void);
void xact0q(void);
void optcoark(void);
void optcoark_after_ckmodcad(void);
void optcoark_after_testxact(void);
void optcoark_after_falton(void);
void optcoark_plus11(void);
void optcoark_after_alarm(void);
void optc1(void);
void optc1_after_terminate(void);
void optc1_after_proceed(void);
void optc1_after_enter(void);
void optc1_after_exdspret(void);
void optc1_plus15(void);
void imuattck_after_cklftbts_yes(void);
void imuattck_after_needle11(void);
void imuattck_after_needler2(void);
void trmattck(void);
void trmattck_after_cklftbts_yes(void);
void trmattck_after_cklftbts_no(void);
void attck1(void);
void attck1_after_needles(void);
void cklftbts(void);
#endif

/* ---- Part 4: the LM's radar test verbs (radar.c) ---- */
#if LUMINARY099
void r77(void);
void r77_after_rdruseck(void);
void r77_after_upflag(void);
void r04(void);
void r04_after_rdruseck(void);
void r04_after_testxact(void);
void r04z(void); /* where R04's UPFLAG returns */
void r04a(void);
void r04a_plus2(void);
void r04a_after_terminate(void); /* GOMARKFR's three crew returns */
void r04a_after_proceed(void);
void r04a_after_recycle(void);
void r04a_immediate_return(void); /* and its immediate one */
void r04a_after_blanket(void);
void r04a_plus11(void);
void r04x_minus1(void);
void r04x(void);
void r04b(void);
void r04b_plus5(void);
void r04b_after_terminate(void); /* GOMARK4's three returns */
void r04b_after_proceed(void);
void r04b_after_enter(void);
void r04c(void);
void r04c_plus4(void);
void r04c_after_waitlist(void);
void r04rr(void);
void r04rr_after_terminate(void); /* GOMARKF's three returns */
void r04rr_after_proceed(void);
void r04rr_after_recycle(void);
void r04rr_plus6(void);
void r04rr_plus6_after_terminate(void);
void r04rr_plus6_after_proceed(void);
void r04rr_plus6_after_recycle(void);
void r04lr(void);
void r04lr_after_terminate(void);
void r04lr_after_proceed(void);
void r04lr_after_recycle(void);
void r04lr_plus6(void);
void r04lr_plus6_after_terminate(void);
void r04lr_plus6_after_proceed(void);
void r04lr_plus6_after_recycle(void);
void r04y(void);
void r04y_after_delayjob(void);
void r04k(void);
void r04l(void);
void r04end(void);
void r04end_after_delayjob(void);
void r04end_after_downflag(void);
void r77end(void);
void r77end_after_delayjob(void);
void r77end_after_downflag(void);
#endif

#endif
