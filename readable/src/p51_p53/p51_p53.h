/* SPDX-License-Identifier: MIT */
/* P51-P53.agc (Luminary099 pp. 926-983): the LM's IMU alignment programs.
 * P52, the realignment in flight to an orientation the crew picks (the
 * preferred, the nominal, REFSMMAT's, or the landing site's); P51, the
 * orientation determination; P57, the alignment on the lunar surface; and
 * the routines they share, among them S50 (LOCSAM), R51, R52, R55, R56
 * and R59. The faithful C is c64-port/src/lm_p51_p53.c, which the listing
 * compiles to: its routine tables have 696 keys.
 *
 * The file is written part by part, in the listing's order (README.md):
 *   part 1, p52.c: P52, PROG52 to P52OUT, the gyro torque coarse
 *     alignment option COARSTYP, the landing site's orientation (P52LS)
 *     and N89DISP, which displays the landing site (pp. 926-933).
 * The binding (src/binding/p51_routines.c) runs the keys these parts
 * write, and the faithful port's routines at the others.
 *
 * Basic code round stretches of interpretive code, which are in the
 * listing's notation (src/interpreter/notation.h). The entries below are
 * the basic code's: each program's start, the words BANKCALL and PHASCHNG
 * return to, the displays' returns (terminate, proceed and enter), and
 * the words after each EXIT. */
#ifndef READABLE_P51_P53_H
#define READABLE_P51_P53_H

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

/* Part 1, P52 -- p52.c */
void prog52(void);                /* PROG52 */
void prog52_after_r02both(void);  /* PROG52 +2 (BANKCALL's return) */
void prog52_no_preferred(void);   /* PROG52 +6 (P52A's return) */
void p52a(void);                  /* P52A */
void p52b_terminate(void);        /* P52B +3 (BANKCALL's return) */
void p52b_proceed(void);          /* P52B +4 */
void p52b_enter(void);            /* P52B +5 */
void p52b_immediate_return(void); /* P52B +6 */
void p52b_after_phaschng(void);   /* P52B +010 */
void p52_option4(void);           /* the option table's first word */
void p52t_terminate(void);        /* P52T +6 (BANKCALL's return) */
void p52t_proceed(void);          /* P52T +7 */
void p52t_enter(void);            /* P52T +010 */
void p52t_time(void);             /* P52T +011 (the TC -5's return) */
void p52v_option4(void);          /* P52V +4 (P52W's return) */
void p52d_after_exit(void);       /* P52D +3 */
void p52d_terminate(void);        /* P52D +6 (BANKCALL's return) */
void p52d_proceed(void);          /* P52D +7 */
void regcoars(void);              /* REGCOARS */
void p52out_after_exit(void);     /* P52OUT +1 */
void coarstyp_terminate(void);    /* COARSTYP +3 (BANKCALL's return) */
void coarstyp_normal(void);       /* COARSTYP +4 */
void coarstyp_gyro_torque(void);  /* COARSTYP +5 */
void lsdisp(void);                /* LSDISP */
void lsdisp_terminate(void);      /* LSDISP +3 (BANKCALL's return) */
void lsdisp_proceed(void);        /* LSDISP +4 */
void lsdisp_enter(void);          /* LSDISP +5 */
extern const struct interpretive_code P52E_CODE, P52V_CODE, P52W_CODE, P52H_CODE, REGCOARS_CODE,
    COARSTYP_CODE, P52LS_CODE, N89DISP_CODE, LSDISP_CODE;
#endif

#endif
