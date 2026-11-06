/* SPDX-License-Identifier: MIT */
/* P51-P53.agc, the CM's (Comanche055 pp. 737-784): the IMU alignment
 * programs. P52 (and P54, its other name) realigns the IMU; P51 finds its
 * orientation from scratch; P53 is P51 with another sighting device;
 * R51 to R56 and S50 to S52 serve them. The faithful C is
 * c64-port/src/cm_p51_p53.c.
 *
 * Written part by part:
 *   part 1, p52.c: P52, the IMU realignment, through COARSTYP and GYCRS,
 *     the gyro torque coarse alignment (pp. 737-742); p51.c starts with
 *     SET1/PDT, which GYCRS runs.
 *   part 2, p52.c: P52LS and LSDISP, the landing site's orientation (pp.
 *     742-743, bank 16); r52.c: R52, the automatic optics positioning,
 *     and ADVORB (pp. 743-747, bank 14).
 *   part 3, s50_r56.c: S50 (LOCSAM) and PICAPAR (R56), the stars to sight
 *     (pp. 748-755, bank 14).
 * The binding (src/binding/cm_p51_routines.c) runs the keys these parts
 * write, and the faithful port's routines at the others.
 *
 * Basic code round stretches of interpretive code, which are in the
 * listing's notation (src/interpreter/notation.h). The entries below are
 * the faithful port's keys, by the listing's labels. */
#ifndef READABLE_P51_P53_CM_H
#define READABLE_P51_P53_CM_H

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

/* Part 1, P52 -- p52.c */
void prog52(void);                /* PROG52 */
void prog52_after_phaschng(void); /* PROG52 +2 */
void prog52_after_updatflg(void); /* PROG52 +4 (DOWNFLAG's return) */
void prog52_after_trackflg(void); /* PROG52 +6 (DOWNFLAG's return) */
void prog52_after_r02both(void);  /* PROG52 +010 (BANKCALL's return) */
void prog52_no_preferred(void);   /* PROG52 +014 (P52A's return) */
void p52a(void);                  /* P52A */
void p52b_terminate(void);        /* P52B +3 (GOPERF4R's return) */
void p52b_proceed(void);          /* P52B +4 */
void p52b_enter(void);            /* P52B +5 */
void p52b_immediate_return(void); /* P52B +6 */
void p52b_after_phaschng(void);   /* P52B +010 */
void p52_landing_site(void);      /* P52B +015 (the option table's return) */
void p52_preferred(void);         /* P52B +016 (P52T's return) */
void p52_nominal(void);           /* P52B +017 (P52J's return) */
void p52_reference(void);         /* P52B +020 (P52T's return) */
void p52t_terminate(void);        /* P52T +6 (GOFLASH's return) */
void p52t_proceed(void);          /* P52T +7 */
void p52t_enter(void);            /* P52T +010 */
void p52t_time(void);             /* P52T +011 */
void p52d_after_exit(void);       /* P52D +3 */
void p52d_terminate(void);        /* P52D +6 (GOFLASH's return) */
void p52d_proceed(void);          /* P52D +7 */
void p52j(void);                  /* P52J (COARSTYP's return) */
void p52j_coarse(void);           /* P52J +3 */
void p52c(void);                  /* P52C (after CAL53RET's EXIT) */
void p52c_after_phaschng(void);   /* P52C +2 */
void p52c_terminate(void);        /* P52C +5 (GOPERF1's return) */
void p52c_proceed(void);          /* P52C +6 */
void p52c_enter(void);            /* P52C +7 */
void p52c_stars(void);            /* P52C +010 (P52F's return) */
void p52e(void);                  /* P52E (after EXIT) */
void p52e_no_stars(void);         /* P52E +2 (PICAPAR's return) */
void p52f(void);                  /* P52F (P52I's return) */
void p52f_after_exit(void);       /* ENDP50S +1 */
void p52i_after_alarm(void);      /* P52I +2 */
void p52i_terminate(void);        /* P52I +5 (GOFLASH's return) */
void p52i_proceed(void);          /* P52I +6 */
void p52i_recycle(void);          /* P52I +7 (P52F's return) */
void coarstyp_terminate(void);    /* COARSTYP +3 (GOPERF1's return) */
void coarstyp_normal(void);       /* COARSTYP +4 */
void coarstyp_gyro(void);         /* COARSTYP +5 */
void gycrs_after_exit(void);      /* GYCRS +022 */
void gycrs_after_godspr(void);    /* GYCRS +025 (BANKCALL's return) */
void gycrs_after_imupulse(void);  /* GYCRS +030 (BANKCALL's return) */
void gycrs_torque_failed(void);   /* GYCRS +032 (BANKCALL's return) */
void gycrs_torqued(void);         /* GYCRS +033 (CURTAINS's return) */
void gycrs_after_phaschng(void);  /* GYCRS +035 */
extern const struct interpretive_code P52T_LANDING_SITE_CODE, P52T_NOMINAL_CODE, P52J_CODE,
    P52J_COARSE_CODE, P52C_STARS_CODE, P52F_CODE, GYCRS_CODE, GYCRS_DONE_CODE;

/* Part 2, P52LS -- p52.c (bank 16) */
void lsdisp(void);           /* LSDISP (after EXIT) */
void lsdisp_terminate(void); /* LSDISP +3 (GOFLASH's return) */
void lsdisp_proceed(void);   /* LSDISP +4 */
void lsdisp_enter(void);     /* LSDISP +5 */
void lsdisp_accept(void);    /* LSDISP +6 */
extern const struct interpretive_code P52LS_CODE, LSDISP_CODE;

/* Part 2, R52 -- r52.c (bank 14) */
void r52vrb_after_exit(void);     /* R52VRB +1 */
void r52a(void);                  /* R52A */
void r52c(void);                  /* R52C */
void r52d_over_90(void);          /* R52D +2 (SR52.1's return) */
void r52d_over_50(void);          /* R52D +3 */
void r52d_under_50(void);         /* R52D +4 */
void r52ja(void);                 /* R52JA (UPFLAG's return) */
void r52ja_not_lem(void);         /* R52JA +4 (R52E's return) */
void r52e(void);                  /* R52E (GODSPR's return) */
void r52e_trunnion(void);         /* R52E +7 (TC +3's return) */
void r52f_after_delayjob(void);   /* R52F +3 */
void r52ha(void);                 /* R52HA (after EXIT) */
void r52ha_after_r61csm(void);    /* R52HA +2 */
void r52sync(void);               /* R52SYNC (R52D's return) */
void r52j(void);                  /* R52J */
void r52j_after_downflag(void);   /* R52J +2 */
void r52j_after_alarm(void);      /* R52J +4 */
void r52m(void);                  /* R52M (R52JA's return) */
void r52m_no_r53(void);           /* R52M +4 (R52F's return) */
void r52m_after_findvac(void);    /* R52M +011 */
void r53job(void);                /* R53JOB */
void endplac_after_exit(void);    /* ENDPLAC +1 */
void r52l_not_lem(void);          /* R52L +4 (R52J's return) */
void r52l_terminate(void);        /* R52L +7 (PRIOLARM's return) */
void r52l_proceed(void);          /* R52L +010 */
void r52l_enter(void);            /* R52L +011 */
void r52l_immediate_return(void); /* R52L +012 */
void term52(void);                /* TERM52 */
void term52_after_clearmrk(void); /* TERM52 +1 */
void term52_after_mkreleas(void); /* TERM52 +3 (BANKCALL's return) */
void term52_after_kleenex(void);  /* TERM52 +7 (BANKCALL's return) */
extern const struct interpretive_code R52_CODE, R52VRB_CODE, R52A_CODE, R52Q_CODE, R53JOB_CODE,
    ADVORB_CODE;

/* Part 3, S50 and R56 -- s50_r56.c (bank 14) */
void picapar(void);                /* PICAPAR */
void picapar_after_makecadr(void); /* PICAPAR +1 */
void picend_after_exit(void);      /* PICEND +3 */
void picgxt_after_exit(void);      /* PICGXT +1 */
extern const struct interpretive_code S50_CODE, PICAPAR_CODE, PICEND_CODE, PICGXT_CODE;

/* P51 -- p51.c (so far SET1/PDT) */
void set1_pdt(void); /* SET1/PDT (by RTB) */

#endif

#endif
