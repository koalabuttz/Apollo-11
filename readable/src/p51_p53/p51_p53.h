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
 *   part 2, s50_r56.c: S50 (LOCSAM), the sun, earth and moon as the LM
 *     sees them, and R56 (PICAPAR), which picks the stars to sight (pp.
 *     934-940).
 *   part 3, r51.c: R51, the fine alignment, and GYCOARS, the gyro torque
 *     coarse alignment (pp. 941-944).
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

/* Part 2, S50 and R56 -- s50_r56.c */
void picapar(void);                /* PICAPAR */
void picapar_after_makecadr(void); /* PICAPAR +1 (MAKECADR's return) */
void picend_after_exit(void);      /* PICEND +3 */
void picnswp_after_exit(void);     /* PICNSWP +1 */
extern const struct interpretive_code LOCSAM_CODE, PICAPAR_CODE, PICEND_CODE, PICGXT_CODE;

/* Part 3, R51 and GYCOARS -- r51.c */
void r51_after_exit(void);         /* R51.1 +1 */
void r51c(void);                   /* R51C (PHASCHNG's return) */
void r51c_terminate(void);         /* R51C +3 (BANKCALL's return) */
void r51c_proceed(void);           /* R51C +4 */
void r51c_enter(void);             /* R51C +5 */
void r51c_pick_stars(void);        /* R51C +6 */
void r51c_after_exit(void);        /* R51C +015 */
void r51c_no_stars(void);          /* R51F -1 (R56's +2) */
void r51f(void);                   /* R51F (R56's +3) */
void r51i(void);                   /* R51I */
void r51i_after_alarm(void);       /* R51I +2 */
void r51i_terminate(void);         /* R51I +5 (BANKCALL's return) */
void r51i_proceed(void);           /* R51I +6 */
void r51i_enter(void);             /* R51I +7 */
void r51e(void);                   /* R51E */
void r51_3_after_exit(void);       /* R51.3 +1 */
void r51_3_restart(void);          /* R51.3 +3 (PHASCHNG's return) */
void r51_mark_after_exit(void);    /* R51.3 +7 */
void r51_mark_after_aotmark(void); /* R51.3 +011 */
void r51_mark_failed(void);        /* R51.3 +013 */
void r51_marked(void);             /* R51.3 +014 (CURTAINS's return) */
void r51_both_marked(void);        /* R51.3 +017 */
void r51_second_after_exit(void);  /* R51.3 +024 */
void r51_second_restart(void);     /* R51.3 +026 */
void r51p63(void);                 /* R51P63 */
void r51p63_terminate(void);       /* R51P63 +3 */
void r51p63_enter(void);           /* R51P63 +4 */
void r51p63_proceed(void);         /* R51P63 +5 */
void gycoars_after_exit(void);     /* GYCOARS +7 */
void gycoars_torque(void);         /* GYCOARS +012 (GODSPR's return) */
void gycoars_after_imupulse(void); /* GYCOARS +015 */
void gycoars_failed(void);         /* GYCOARS +017 */
void gycoars_torqued(void);        /* GYCOARS +020 (CURTAINS's return) */
void gycoars_restart(void);        /* GYCOARS +022 */
extern const struct interpretive_code R51_CODE, R51C_CODE, R51_3_CODE, R51_MARK_CODE,
    R51_SECOND_CODE, R51_TORQUE_CODE, R51P63_CODE, R51_4_CODE, GYCOARS_CODE, GYCOARS_DONE_CODE;
#endif

#endif
