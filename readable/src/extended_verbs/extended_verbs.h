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
 * and EXDAPOFF, and SHOWSUM. The entries are the routines' labels and the
 * words a call out of the routine returns to. All of it is basic code. */
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

#endif
