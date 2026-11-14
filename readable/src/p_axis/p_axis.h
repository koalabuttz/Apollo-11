/* SPDX-License-Identifier: MIT */
/* P-AXIS_RCS_AUTOPILOT.agc, the LM's (Luminary099 pages 1421-1441, bank
 * 16): the RCS autopilot's T5RUPT, every 100 ms while the DAP is on. It
 * reads the CDUs, derives the body rates (a state estimator: the rates
 * and the attitude errors filtered against the jets' expected effect),
 * counts down the trim gimbal drives, selects the jets for a translation
 * the hand controller asks for (Y and Z), and runs the P axis: the minimum
 * impulse mode, the manual rate command mode (direct rate, or pseudo-auto)
 * or the automatic one (TJETLAW, or SPSRCS docked), and the jet selection
 * for its rotation. Then the Q and R axes (QRAXIS, by DTCB to bank 17).
 * The faithful C is c64-port/src/lm_p_axis.c.
 *
 * Each function below is an entry: a label, the word an offset from one
 * names, or the word after a TC where that call returns (the name says
 * which). A run ends at a TC out of the file (CHEKBITS, ACDT+C12, ALARM,
 * WRITEP, IBNKCALL), a TCF out of it (NOQBRSM +1, RATELOOP), QRAXIS's DTCB,
 * or, in the rate loop and the P axis, at the word another entry starts
 * at, as the faithful port's runs do: the program's trampoline then calls
 * the routine at the new Z. */
#ifndef READABLE_P_AXIS_H
#define READABLE_P_AXIS_H

#include "../agc/names.h"

#if LUMINARY099
void paxis(void);                   /* PAXIS: the T5RUPT task */
void paxis_plus10(void);            /* PAXIS +10: TC CHEKBITS */
void paxis_after_chekbits(void);    /* PAXIS +11 */
void paxfilt_after_acdt_c12(void);  /* PAXFILT +4 */
void superjob(void);                /* SUPERJOB: the DAP's job */
void backp(void);                   /* BACKP: the rate derivation */
void backp_plus0133(void);          /* BACKP +133 */
void q_rate(void);                  /* Q-RATE */
void r_rate(void);                  /* R-RATE */
void pretimck(void);                /* PRETIMCK */
void chkvisfz(void);                /* CHKVISFZ */
void abortyz_plus2(void);           /* ABORTYZ +2 */
void tsnextp_minus1(void);          /* TSNEXTP -1 */
void tsnextp(void);                 /* TSNEXTP */
void detentck(void);                /* DETENTCK */
void ratedone_after_zatteror(void); /* RATEDONE +16 */
void raterror(void);                /* RATERROR */
void pjetslec_minus6(void);         /* PJETSLEC -6 */
void pjetslec_minus1(void);         /* PJETSLEC -1 */
void pjetslec_after_writep(void);   /* PJETSLEC +25 */
void pjetslec_after_jtlst(void);    /* PJETSLEC +46 */
void jetsoff_after_writep(void);    /* JETSOFF +1 */
void calcperr(void);                /* CALCPERR, a subroutine */
void purgency(void);                /* PURGENCY */
void purgency_plus4(void);          /* PURGENCY +4 */
void purgency_after_spsrcs(void);   /* PURGENCY +15 */
void headtjet_after_tjetlaw(void);  /* HEADTJET +5 */
void selectp_after_alarm(void);     /* SELECTP +17 */
#endif

#endif
