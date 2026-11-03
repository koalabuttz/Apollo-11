/* SPDX-License-Identifier: MIT */
/* P30-P37.agc (Comanche055 pp. 635-648): the CM's P30, the external
 * delta-V targeting program, and P31, general Lambert aimpoint guidance,
 * with the routines they share, S30.1 and S31.1 (their computations),
 * DELRSPL (the splash error V82 shows) and AVFLAGA, AVFLAGP and P20FLGON
 * (which set the flags for the active vehicle and for P20). The faithful
 * C is c64-port/src/cm_p30_p37.c, which the listing compiles to: its
 * routine tables have 143 keys.
 *
 * The code, by its banks:
 *   p30.c: P30, P31 and their displays (bank 35, pp. 635-638), and
 *   AVFLAGA, AVFLAGP and P20FLGON (fixed-fixed, p. 648);
 *   s30_s31.c: S30.1 and S31.1 (bank 31, pp. 639-642);
 *   delrspl.c: DELRSPL and AUGEKUGL (bank 32, pp. 643-647).
 * The binding is src/binding/cm_p30_routines.c.
 *
 * Basic code round stretches of interpretive code, which are in the
 * listing's notation (src/interpreter/notation.h). The entries below are
 * the basic code's: each routine's start, the words BANKCALL, PHASCHNG,
 * 2PHSCHNG, WAITLIST, UPFLAG and DOWNFLAG return to, and the words after
 * each EXIT. */
#ifndef READABLE_P30_CM_H
#define READABLE_P30_CM_H

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

/* ---- p30.c: P30 and P31 (bank 35) ---- */

extern const struct interpretive_code DISPMGA_CODE, END45_CODE, P30_CODE, P31_CODE, REFTEST_CODE,
    FLASHMGA_CODE;

void dispmga_after_exit(void);
void disp45(void);
void disp45_terminate(void);
void disp45_proceed(void);
void disp45_recycle(void);
void p30phsi(void);
void p30phsi_after_phaschng(void);
void comptgo(void);
void comptgo_after_timrflag(void);
void comptgo_after_waitlist(void);
void comptgo_after_2phschng(void);
void p30(void);
void p30_after_p30_p31(void);
void p30_after_cntnup30(void);
void p30_after_updatflg(void);
void p30_after_exit(void);
void p30_after_param30(void);
void p30_after_xdelvflg(void);
void p31(void);
void p31_after_p30_p31(void);
void p31_after_updatflg(void);
void p31_after_normsw(void);
void p31_after_exit(void);
void p31_after_cntnup30(void);
void p31_after_param30(void);
void p31_after_xdelvflg(void);
void p30_p31_after_updatflg(void);
void p30_p31_after_trackflg(void);
void p30_p31_terminate(void);
void p30_p31_proceed(void);
void p30_p31_recycle(void);
void p30_p31_immediate_return(void);
void p30_p31_after_phaschng(void);
void cntnup30_terminate(void);
void cntnup30_proceed(void);
void cntnup30_recycle(void);
void param30_terminate(void);
void param30_proceed(void);
void param30_recycle(void);
void flashmga_after_exit(void);

/* ---- p30.c: AVFLAGA, AVFLAGP and P20FLGON (fixed-fixed) ---- */

void avflaga(void);
void avflaga_after_avflag(void);
void avflagp(void);
void avflagp_after_avflag(void);
void p20flgon(void);
void p20flgon_after_trackflg(void);
void p20flgon_after_updatflg(void);

/* ---- s30_s31.c: S30.1 and S31.1 (bank 31) ---- */

extern const struct interpretive_code S30_1_CODE, S31_1_CODE;

/* ---- delrspl.c: DELRSPL and AUGEKUGL (bank 32) ---- */

extern const struct interpretive_code DELRSPL_CODE;

#endif

#endif
