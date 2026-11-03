/* SPDX-License-Identifier: MIT */
/* P32-P35_P72-P75.agc (Luminary099 pp. 618-650): the LM's rendezvous
 * targeting for the concentric flight plan. CSI, "COELLIPTIC SEQUENCE
 * INITIATION" (P32 when the LM is the active vehicle, P72 when it is the
 * passive one), and CDH, "CONSTANT DELTA HEIGHT" (P33 and P73). The
 * faithful C is c64-port/src/lm_p32_p35.c, which the listing compiles to:
 * its routine tables have 421 keys.
 *
 * The code, by its banks:
 *   programs.c: P32/P72, P33/P73, AVFLAGA, AVFLAGP, P20FLGON, DISDVLVC
 *   and COMPTGO (bank 35, pp. 621-631 and 650);
 *   csi.c: CSI/A from its start to SCNDSOL (bank 34, pp. 632-641);
 *   the rest of the CDH computations, ADVANCE to CDHMVR (bank 34,
 *   pp. 642-650), are still the faithful port's.
 * The binding is src/binding/p32_routines.c.
 *
 * Basic code round stretches of interpretive code, which are in the
 * listing's notation (src/interpreter/notation.h). The entries below are
 * the basic code's: each routine's start, the words VNPOOH, BANKCALL,
 * ALARM, VARALARM, WAITLIST, 2PHSCHNG, UPFLAG and DOWNFLAG return to, and
 * the words after each EXIT. */
#ifndef READABLE_P32_P35_H
#define READABLE_P32_P35_H

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

/* ---- programs.c: P32 and P72 (bank 35) ---- */

extern const struct interpretive_code ALMXITA_CODE, VN0611_CODE, VN0655_CODE, VN0637_CODE,
    P32_P72_N75_CODE;

void p32(void);
void p32_after_avflaga(void);
void p72(void);
void p32strt(void);
void almxit_after_exit(void);
void almxit_after_varalarm(void);
void almxit_terminate(void);
void almxit_proceed(void);
void p32_p72a(void);
void p32_p72a_after_p20flgon(void);
void vn0611_after_vnpooh(void);
void vn0611_again(void);
void vn0611_after_exit(void);
void vn0655_after_exit(void);
void vn0655_terminate(void);
void vn0655_proceed(void);
void vn0655_recycle(void);
void vn0637(void);
void vn0637_after_vnpooh(void);
void p32_p72_n75(void);
void p32_p72_n75_after_vnpooh(void);

/* ---- programs.c: P33 and P73 (bank 35) ---- */

extern const struct interpretive_code P33_P73A_CODE, P33_P73_ALARM_CODE, P33_P73_N75_CODE;

void p33(void);
void p33_after_avflaga(void);
void p73(void);
void p33_p73a(void);
void p33_p73a_after_p20flgon(void);
void p33_p73a_after_vnpooh(void);
void p33_p73_alarm(void);
void p33_p73_after_alarm(void);
void p33_p73_terminate(void);
void p33_p73_proceed(void);
void p33_p73_recycle(void);
void p33_p73_after_proceed(void);
void p33_p73_n75(void);
void p33_p73_n75_after_vnpooh(void);

/* ---- csi.c: CSI/A to SCNDSOL (bank 34) ---- */

extern const struct interpretive_code CSI_A_CODE, NTP_2_CODE;

/* ---- programs.c: AVFLAGA, AVFLAGP, P20FLGON, DISDVLVC and COMPTGO
 * (bank 35) ---- */

extern const struct interpretive_code DISDVLVC_CODE, DISDVLVC_EXIT_CODE;

void avflaga(void);
void avflaga_after_upflag(void);
void avflagp(void);
void avflagp_after_downflag(void);
void p20flgon(void);
void p20flgon_after_updatflg(void);
void p20flgon_after_trackflg(void);
void disdvlvc_after_exit(void);
void disdvlvc_after_vnpooh(void);
void comptgo(void);
void comptgo_after_waitlist(void);
void comptgo_after_2phschng(void);

#endif

#endif
