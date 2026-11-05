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
 *   part 4, r55.c: R55, the gyro torque; R54 (CHKSDATA), the star data
 *     test; CAL53A and MATMOVE (pp. 945-948).
 *   part 5, p51.c: P51, the orientation determination, with COARSE,
 *     NCOARSE, S52.2 and S52.3 (pp. 949-954).
 *   part 6, r52_r59.c: R52, the automatic optics positioning, and R59, the
 *     lunar surface star acquisition (pp. 955-960).
 *   part 7, planet_gvdeter.c: PLANET, the sighted body's direction, and
 *     GVDETER, the gravity vector determination (pp. 961-966).
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

/* Part 4, R55, R54 (CHKSDATA), CAL53A, MATMOVE and RDCDUS -- r55.c */
void r55_1(void);               /* R55.1 (after EXIT) */
void r55_terminate(void);       /* R55.1 +3 (GOFLASH's return) */
void r55_proceed(void);         /* R55.1 +4 */
void r55_enter(void);           /* R55.1 +5 */
void r55_2(void);               /* R55.2 */
void r55_torque(void);          /* R55.2 +2 (PHASCHNG's return) */
void r55_after_imupulse(void);  /* R55.2 +5 */
void r55_failed(void);          /* R55.2 +7 */
void r55_torqued(void);         /* R55.2 +010 (CURTAINS's return) */
void r55ret(void);              /* R55RET */
void chksdata_after_exit(void); /* CHKSDATA +034 */
void chksdata_terminate(void);  /* CHKSDATA +037 (GOFLASH's return) */
void chksdata_proceed(void);    /* CHKSDATA +040 */
void chksdata_enter(void);      /* CHKSDATA +041 */
void chksda(void);              /* CHKSDA */
void cal53a_after_exit(void);   /* CAL53A +026 */
void coarfine(void);            /* CAL53A +030 (PHASCHNG's return) */
void rdcdus(void);              /* RDCDUS, an RTB's routine */
extern const struct interpretive_code R55_CODE, R55RET_CODE, CHKSDATA_CODE, CHKSDATA_ENTER_CODE,
    CHKSDA_CODE, CAL53A_CODE, COARFINE_CODE, MATMOVE_CODE;

/* Part 5, P51, COARSE, NCOARSE, S52.2 and S52.3 -- p51.c */
void p51(void);                   /* P51 */
void p51_plus2(void);             /* P51 +2 (IMUCHK's return) */
void p51_terminate(void);         /* P51 +5 (GOPERF1's return) */
void p51_proceed(void);           /* P51 +6 */
void p51_enter(void);             /* P51 +7 */
void p51_zero_gimbals(void);      /* P51 +011 (PHASCHNG's return) */
void p51_display_v41(void);       /* P51 +020 */
void p51_coarse(void);            /* P51 +023 */
void p51_after_coarse(void);      /* P51 +027 (after EXIT) */
void p51_again(void);             /* P51 +031 (PHASCHNG's return) */
void p51b_ncoarse(void);          /* P51B +2 (PHASCHNG's return) */
void p51c_plus1(void);            /* P51C +1 (after EXIT) */
void p51c_aotmark(void);          /* P51C +3 (PHASCHNG's return) */
void p51c_after_aotmark(void);    /* P51C +5 */
void p51c_failed(void);           /* P51C +7 */
void p51c_star(void);             /* P51C +010 (CURTAINS's return) */
void p51d_plus1(void);            /* P51D +1 (after EXIT) */
void p51d_star(void);             /* P51D +3 (PHASCHNG's return) */
void p51d_planet(void);           /* P51D +7 (PHASCHNG's return) */
void p51d_second_star(void);      /* P51D +015 (after EXIT) */
void p51e_planet(void);           /* P51E +2 (PHASCHNG's return) */
void p51e_again(void);            /* P51E +021 (after EXIT) */
void p51g_finis(void);            /* P51G +011 (after EXIT) */
void coarse_imucoars(void);       /* COARSE +1 (after EXIT) */
void coarse_after_imucoars(void); /* COARSE +3 */
void coarse_failed(void);         /* COARSE +5 */
void coarse_imufine(void);        /* COARSE +6 (CURTAINS's return) */
void coarse_after_imufine(void);  /* COARSE +010 */
void coarse_fine_failed(void);    /* COARSE +012 */
void coarse_return(void);         /* COARSE +013 (CURTAINS's return) */
void ncoarse_plus1(void);         /* NCOARSE +1 (after EXIT) */
extern const struct interpretive_code P51_CODE, P51B_CODE, P51C_CODE, P51D_CODE, P51E_CODE,
    P51G_CODE, COARSE_CODE, COARSE_RETURN_CODE, NCOARSE_CODE, NCOARSE_GCOMP_CODE, S52_2_CODE,
    S52_3_CODE;

/* Part 6, R52 and R59 -- r52_r59.c */
void r52_starcode(void);                /* R52 +2 (after EXIT) */
void r52_terminate(void);               /* R52A +3 (GOFLASH's return) */
void r52_proceed(void);                 /* R52A +4 */
void r52_enter(void);                   /* R52A +5 */
void r52b_detent(void);                 /* R52B +2 (DOWNFLAG's return) */
void getazel_terminate(void);           /* GETAZEL +3 (GOFLASH's return) */
void getazel_proceed(void);             /* GETAZEL +4 */
void getazel_enter(void);               /* GETAZEL +5 */
void azel_r60lem(void);                 /* AZEL +014 (after EXIT) */
void azel_after_r60lem(void);           /* AZEL +016 (BANKCALL's return) */
void r59(void);                         /* R59 */
void r59_terminate(void);               /* R59 +7 (GOFLASH's return) */
void r59_proceed(void);                 /* R59 +010 */
void r59_recycle(void);                 /* R59 +011 */
void r59a_after_exit(void);             /* R59A +027 */
void r59d_display(void);                /* R59D +015 (after EXIT) */
void zspcr_plus1(void);                 /* ZSPCR +1 (after EXIT) */
void nxax_plus1(void);                  /* NXAX +1 (after EXIT) */
void r59alm_after_alarm(void);          /* R59ALM +2 (ALARM's return) */
void r59alm_terminate(void);            /* R59ALM +5 (GOFLASH's return) */
void r59alm_proceed(void);              /* R59ALM +6 */
void r59alm_recycle(void);              /* R59ALM +7 */
void seventy_nine_disp_terminate(void); /* 79DISP +3 (GOFLASH's return) */
void seventy_nine_disp_proceed(void);   /* 79DISP +4 */
void seventy_nine_disp_recycle(void);   /* 79DISP +5 */
void r59out_after_aotmark(void);        /* R59OUT +2 (BANKCALL's return) */
void r59out_badend(void);               /* R59OUT +4 (BANKCALL's return) */
extern const struct interpretive_code R52_CODE, AZEL_CODE, R52_RETURN_CODE, R59A_CODE, INCAZ_CODE,
    ZSPCR_CODE, NXAX_CODE;

/* Part 7, PLANET and GVDETER -- planet_gvdeter.c */
void planet_after_exit(void);       /* PLANET +5 */
void planet_terminate(void);        /* PLANET +021 (GOFLASH's return) */
void planet_proceed(void);          /* PLANET +022 */
void planet_enter(void);            /* PLANET +023 */
void planet_vector(void);           /* PLANET +024 (the TC +2's return) */
void gvdeter(void);                 /* GVDETER */
void gravel_after_exit(void);       /* GVDETER +060 */
void gravel_display(void);          /* GVDETER +062 (DOWNFLAG's return) */
void gravel_terminate(void);        /* GVDETER +065 (GOFLASH's return) */
void gravel_proceed(void);          /* GVDETER +066 */
void gravel_recycle(void);          /* GVDETER +067 */
void prograv(void);                 /* PROGRAV (UPFLAG's return) */
void prograv_after_phaschng(void);  /* PROGRAV +2 */
void lung_after_exit(void);         /* LUNG +5 */
void lung_coarse(void);             /* LUNG +7 (PHASCHNG's return) */
void lung_after_imucoars(void);     /* LUNG +011 (BANKCALL's return) */
void lung_coarse_failed(void);      /* LUNG +013 */
void lung_fine(void);               /* LUNG +014 (CURTAINS's return) */
void lung_after_imufine(void);      /* LUNG +016 (BANKCALL's return) */
void lung_fine_failed(void);        /* LUNG +020 */
void lung_aligned(void);            /* LUNG +021 (CURTAINS's return) */
void lung_after_gcompzer(void);     /* LUNG +027 (BANKCALL's return) */
void lung_pipas(void);              /* LUNG +031 (PHASCHNG's return) */
void lung_after_pipsrine(void);     /* LUNG +033 (BANKCALL's return) */
void greed_after_exit(void);        /* GREED +1 */
void greed_after_twiddle(void);     /* GREED +4 (TWIDDLE's return) */
void grabgrav(void);                /* GRABGRAV, the task */
void grabgrav_after_pipsrine(void); /* GRABGRAV +2 (IBNKCALL's return) */
void grabgrav_after_findvac(void);  /* GRABGRAV +6 */
void addgrav(void);                 /* ADDGRAV, the job */
void addgrav_after_pipa(void);      /* ADDGRAV +2 (BANKCALL's return) */
void addgrav_after_exit(void);      /* ADDGRAV +023 */
void qminexit(void);                /* QMINEXIT (PHASCHNG's return) */
extern const struct interpretive_code PLANET_CODE, PLANET_VECTOR_CODE, NOTPLAN_CODE, CALSAM_CODE,
    GVDETER_CODE, PROGRAV_CODE, LUNG_CODE, GREED_CODE, ADDGRAV_CODE, QMINEXIT_CODE;

/* INDEX before CA, TS and DXCH (R52, R59 and PLANET) -- indexed.c: the
 * address the indexed instruction works on. */
unsigned indexed_ca(unsigned address, word index);
unsigned indexed_ts(unsigned address, word index);
unsigned indexed_dxch(unsigned address, word index);
#endif

#endif
