/* SPDX-License-Identifier: MIT */
#ifndef LM_P_AXIS_H
#define LM_P_AXIS_H
#include "agc_transfer.h"
#include "lm_flight_io.h"
/* Luminary099 P-AXIS_RCS_AUTOPILOT.agc, all of it (bank 16, 02213-03640),
 * as native routines of the control-transfer ABI; io is p66_bias_io (the
 * interrupt-enable bit); channels 12, 13 and 31 are the program's
 * agc_hardware (include/lm_flight_io.h).
 *
 *   PAXIS     16,2213  T5 lead-in: TIME5, BANKRUPT/QRUPT, BAILOUT on a busy
 *                      DAPZRUPT, else TC CHEKBITS (twice for DAPZRUPT +0)
 *   16,2223            (CHEKBITS's return after the OCT 02000 word) TC CHEKBITS
 *   16,2224            (CHEKBITS's return) CDU samples, CDU desired update,
 *                      TCP/TCQR; PAXFILT: TC ACDT+C12 when CALLGMBL is set
 *   16,2653            (ACDT+C12's return) the KALMAN filter as an
 *                      interrupted job: TCF NOQBRSM +1
 *   SUPERJOB  16,2666  (RESUME at ZRUPT-1) TCF RATELOOP
 *   BACKP     16,2316  rate derivation, trap filters, AOS terms, gimbal
 *                      timers, translation (CHKVISFZ), manual and rate
 *                      command modes, P-axis RCS law and jet selection, to
 *                      the first call that leaves the file: TC ALARM,
 *                      IBNKCALL (ZATTEROR, SPSRCS, TJETLAW, JTLST),
 *                      TC WRITEP / WRITEP -1, or QRAXIS's DTCB to CALLQERR
 *   16,2773 16,3154 16,3403 16,3424 16,3443 16,3500 16,3512 16,3550
 *                      the returns from those calls, continuing the same
 *   CALCPERR  16,3446  P-axis attitude error; TC Q
 *
 * Refused (EXEC_OUT_OF_SCOPE, image unchanged): an entry under an FB other
 * than bank 16 or an EB other than EBANK6 (EBANK= PERROR), a missing io
 * record, a CALCPERR return in Q that is not a TC into fixed memory, a
 * 16,3403 entry with ROTINDEX other than 0 or 1 (it indexes MINTIMES; the
 * TC WRITEP it returns from follows PJETSLEC's LXCH ROTINDEX of 0 or 1),
 * and OLDPMIN -0 (CCS OLDPMIN would enter FIREP's RAND without its EXTEND;
 * the file only stores +0, 1 and OCT14 bits there). */
extern const agc_routine_table lm_p_axis_routines;
/* The inline operands read through Q by the file's TC ALARM calls (OCT 02001
 * after ABORTYZ, OCT 02003 in SELECTP), as compiled site operands. */
#define LM_PA_SITES 2u
extern const agc_site_operand lm_p_axis_sites[LM_PA_SITES];
/* This file's data words that no code in it reads (-100MS is KALMAN_FILTER's;
 * KAOS, 1/10SEC, NORMSCL and PSKIPADR have no reference in Luminary099), for
 * a program's passive fixed_data reads: the word at a physical address, or
 * false. */
bool lm_p_axis_data(uint16_t physical,agc_word *word);
#define LM_PA_FB16 034000u
#define LM_PA_EB 03000u
#define LM_PA_PAXIS 02213u
#define LM_PA_CHEKBITS2 02223u
#define LM_PA_AFTERCHEK 02224u
#define LM_PA_AFTERACDT 02653u
#define LM_PA_SUPERJOB 02666u
#define LM_PA_PRETIMCK 02667u
#define LM_PA_CHKVISFZ 02740u
#define LM_PA_BACKP 02316u
#define LM_PA_TRAPS 02451u    /* CA DAPBOOLS before LMONLY */
#define LM_PA_QRATE 02544u    /* Q-RATE */
#define LM_PA_RRATE 02602u    /* R-RATE */
#define LM_PA_ABORTYZ2 02773u
#define LM_PA_TSNEXTP1 03013u   /* TSNEXTP -1 */
#define LM_PA_TSNEXTP 03014u
#define LM_PA_DETENTCK 03071u
#define LM_PA_ZATTRET 03154u
#define LM_PA_RATERROR 03227u
#define LM_PA_PJETSLEC6 03350u  /* PJETSLEC -6 */
#define LM_PA_PJETSLEC1 03355u  /* PJETSLEC -1 */
#define LM_PA_WRITEPRET 03403u
#define LM_PA_JTLSTRET 03424u
#define LM_PA_JETSOFFRET 03443u
#define LM_PA_CALCPERR 03446u
#define LM_PA_PURGENCY 03463u
#define LM_PA_PURGENCY4 03467u  /* PURGENCY +4 */
#define LM_PA_SPSRET 03500u
#define LM_PA_TJETRET 03512u
#define LM_PA_SELALARM 03550u
#define LM_PA_FIRST 02213u
#define LM_PA_LAST 03640u
/* Where the file's calls leave it. */
#define LM_PA_BAILOUT 05634u
#define LM_PA_CHEKBITS 02000u   /* bank 16 */
#define LM_PA_ACDTC12 03736u    /* bank 16 */
#define LM_PA_NOQBRSM1 05275u
#define LM_PA_RATELOOP 03641u   /* bank 16 */
#define LM_PA_ALARM 05567u
#define LM_PA_IBNKCALL 04674u
#define LM_PA_WRITEP 05754u
#define LM_PA_WRITEP1 05753u   /* WRITEP -1 */
#define LM_PA_CALLQERR 02072u   /* bank 17 */
#endif
