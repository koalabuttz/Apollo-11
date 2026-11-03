/* SPDX-License-Identifier: MIT */
/* P32-P33_P72-P73.agc (Comanche055 pp. 649-683): the CM's rendezvous
 * targeting for the concentric flight plan, the twin of the LM's
 * P32-P35_P72-P75 (src/p32_p35/). CSI, "COELLIPTIC SEQUENCE INITIATION"
 * (P32 when the CM is the active vehicle, P72 when it is the passive
 * one), and CDH, "CONSTANT DELTA HEIGHT" (P33 and P73). The faithful C is
 * c64-port/src/cm_p32_p33.c, which the listing compiles to: its routine
 * tables have 407 keys.
 *
 * The CM's code is the LM's, cut by SETLOC into stretches in many banks
 * that GOTOs and STCALLs join. The code, by its banks:
 *   programs.c: P32/P72 and P33/P73 (bank 4, and their stretches in banks
 *   14 and 12, pp. 652-659) and DISDVLVC (bank 37, p. 662);
 *   csi.c: CSI/A to CIRCL, the start of the CSI computation (banks 6, 12,
 *   22 and 23, pp. 664-668);
 *   the rest of the CSI and CDH computations, NTP/2 to CDHMVR (pp.
 *   668-683), are still the faithful port's.
 * AVFLAGA, AVFLAGP and P20FLGON are the CM's P30-P37.agc's
 * (src/p30_cm/). The binding is src/binding/cm_p32_routines.c.
 *
 * Basic code round stretches of interpretive code, which are in the
 * listing's notation (src/interpreter/notation.h). The entries below are
 * the basic code's: each routine's start, the words AVFLAGA, AVFLAGP,
 * P20FLGON, VNPOOH, BANKCALL, ALARM and VARALARM return to, and the words
 * after each EXIT. */
#ifndef READABLE_P32_P33_CM_H
#define READABLE_P32_P33_CM_H

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

/* ---- programs.c: P32 and P72 (banks 4 and 14) ---- */

extern const struct interpretive_code P32STRT_CODE, ALMXITA_CODE, P32_P72A_CODE, VN0637_CODE,
    P32_P72E_CODE, P32_P72_N75_CODE;

void p32(void);
void p32_after_avflaga(void);
void p72(void);
void p32strt(void);
void p32strt_after_exit(void);
void almxit_after_exit(void);
void almxit_after_varalarm(void);
void almxit_terminate(void);
void almxit_proceed(void);
void p32_p72a(void);
void p32_p72a_after_p20flgon(void);
void vn0611(void);
void vn0655(void);
void vn0655_terminate(void);
void vn0655_proceed(void);
void vn0655_recycle(void);
void vn0637(void);
void vn0637_after_vnpooh(void);
void p32_p72_n75(void);
void p32_p72_n75_after_vnpooh(void);

/* ---- programs.c: P33 and P73 (banks 4 and 12) ---- */

extern const struct interpretive_code P33_P73A_CODE, P33_P73_ALARM_CODE, P33_P73C_CODE,
    P33_P73E_CODE, P33_P73_N75_CODE;

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

/* ---- programs.c: DISDVLVC (bank 37) ---- */

extern const struct interpretive_code DISDVLVC_CODE, DISDVLVC_EXIT_CODE;

void disdvlvc_after_exit(void);
void disdvlvc_after_vnpooh(void);

/* ---- csi.c: CSI/A to CIRCL (banks 6, 12, 22 and 23) ---- */

extern const struct interpretive_code CSI_A_CODE, CSI_B2_CODE, CSI_B23_CODE, CSINEXT_CODE;

#endif

#endif
