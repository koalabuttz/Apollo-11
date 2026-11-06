/* SPDX-License-Identifier: MIT */
/* P51-P53.agc, the CM's (Comanche055 pp. 737-784): the IMU alignment
 * programs. P52 (and P54, its other name) realigns the IMU; P51 finds its
 * orientation from scratch; P53 is P51 with another sighting device;
 * R51 to R56 and S50 to S52 serve them. The faithful C is
 * c64-port/src/cm_p51_p53.c.
 *
 * Written part by part, in the listing's order (README.md):
 *   part 1, p52.c: P52, the IMU realignment, through COARSTYP and GYCRS,
 *     the gyro torque coarse alignment (pp. 737-742); p51.c starts with
 *     SET1/PDT, which GYCRS runs.
 *   part 2, p52.c: P52LS and LSDISP, the landing site's orientation (pp.
 *     742-743, bank 16); r52.c: R52, the automatic optics positioning,
 *     and ADVORB (pp. 743-747, bank 14).
 *   part 3, s50_r56.c: S50 (LOCSAM) and PICAPAR (R56), the stars to sight
 *     (pp. 748-755, bank 14).
 *   part 4, r51_r55.c: R51, the fine alignment (pp. 756-758, bank 14),
 *     with CHKSCODE (p. 773), and R55, the gyro torque (p. 759, bank 15).
 *   part 5, r54_cal53a.c: R54 (CHKSDATA), the star data test, CAL53A,
 *     the coarse alignment if one is needed, and MATMOVE (pp. 759-762,
 *     bank 14); RDCDUS (bank 15); CALCSMSC (p. 763, bank 16).
 *   part 6, p51.c: P51, the IMU orientation determination, and P53, its
 *     backup (pp. 766-769, bank 14); SXTSM, a mark's line of sight in
 *     stable member axes (p. 771, bank 17).
 *   part 7, r53_sr52_1.c: R53, the sighting mark routine (pp. 772-773,
 *     bank 14); S52.2, the gimbal angles (p. 774, bank 11); SR52.1, the
 *     optics angles to a target (pp. 775-777, bank 13).
 *   part 8, advtrack_r56_planet.c: ADVTRACK, the line of sight to a
 *     point ahead on the ground track (pp. 778-779, bank 26); S52.3, the
 *     preferred orientation (p. 780, bank 16); R56, the alternate line of
 *     sight mark (pp. 781-782, bank 17); PLANET, a star's or a planet's
 *     direction (pp. 783-784, bank 15). indexed.c: INDEX before CA, TS
 *     and DXCH (parts 4, 7 and 8).
 * The binding (src/binding/cm_p51_routines.c) runs every key on these
 * parts' routines.
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

/* INDEX before CA, TS and DXCH (R51, R53, R56 and PLANET) -- indexed.c:
 * the address the indexed instruction works on. */
unsigned indexed_ca(unsigned address, word index);
unsigned indexed_ts(unsigned address, word index);
unsigned indexed_dxch(unsigned address, word index);

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

/* Part 4, R51 and R55 -- r51_r55.c (banks 14 and 15) */
void r51_after_exit(void);       /* R51 +1 */
void r51_3_after_exit(void);     /* R51.3 +4 */
void r51_3_restart(void);        /* R51.3 +7 (PHASCHNG's return) */
void r51dsp_terminate(void);     /* R51DSP +3 (GOFLASHR's return) */
void r51dsp_proceed(void);       /* R51DSP +4 */
void r51dsp_enter(void);         /* R51DSP +5 */
void r51dsp_displayed(void);     /* R51DSP +6 */
void r51dsp_after_blanket(void); /* R51DSP +010 */
void r51dsp_bad_code(void);      /* R51DSP +012 (CHKSCODE's return) */
void r51dsp_after_falton(void);  /* R51DSP +013 */
void r51dsp_code_good(void);     /* R51DSP +014 (R51DSP's return) */
void r51st_after_exit(void);     /* R51ST +2 */
void r51a_after_exit(void);      /* R51A +4 */
void r51a_after_mkreleas(void);  /* R51A +6 (BANKCALL's return) */
void r51a_sighted(void);         /* R51A +013 */
void r51a_both_sighted(void);    /* R51A +015 (R51.4's return) */
void r51k_after_exit(void);      /* R51K +1 */
void r51k_terminate(void);       /* R51K +4 (GOPERF1's return) */
void r51k_proceed(void);         /* R51K +5 */
void r51k_enter(void);           /* R51K +6 */
void r51k_again(void);           /* R51K +7 */
void r51k_done(void);            /* R51K +011 (BANKCALL's return) */
bool chkscode(void);             /* CHKSCODE: false at the caller's +0 */
void r55_1(void);                /* R55.1 (after PULSEM's EXIT) */
void r55_terminate(void);        /* R55.1 +3 (GOFLASH's return) */
void r55_proceed(void);          /* R55.1 +4 */
void r55_enter(void);            /* R55.1 +5 */
void r55_2(void);                /* R55.2 */
void r55_torque(void);           /* R55.2 +2 (PHASCHNG's return) */
void r55_after_imupulse(void);   /* R55.2 +5 (BANKCALL's return) */
void r55_failed(void);           /* R55.2 +7 (BANKCALL's return) */
void r55_torqued(void);          /* R55.2 +010 (CURTAINS's return) */
void r55ret(void);               /* R55RET */
extern const struct interpretive_code R51_CODE, R51_3_CODE, R51_STAR_CODE, R51_MARK_CODE,
    R51A_PLANET_CODE, R51_TORQUE_CODE, R51K_DONE_CODE, R51_4_CODE, R51B_CODE, R55_CODE, R55RET_CODE;

/* Part 5, R54, CAL53A, RDCDUS, CALCSMSC -- r54_cal53a.c (banks 14-16) */
void chksdata_after_exit(void);     /* CHKSDATA +034 */
void chksdata_after_cleandsp(void); /* CHKSDATA +037 (BANKCALL's return) */
void chksdata_terminate(void);      /* CHKSDATA +042 (GOFLASH's return) */
void chksdata_proceed(void);        /* CHKSDATA +043 */
void chksdata_enter(void);          /* CHKSDATA +044 */
void coarfine_after_exit(void);     /* COARFINE +1 */
void coarfine_after_phaschng(void); /* COARFINE +3 */
void coarfine_after_imucoars(void); /* COARFINE +5 (BANKCALL's return) */
void coarfine_coarse_failed(void);  /* COARFINE +7 (BANKCALL's return) */
void coarfine_coarse_aligned(void); /* COARFINE +010 (CURTAINS's return) */
void coarfine_after_imufin20(void); /* COARFINE +012 (BANKCALL's return) */
void coarfine_fine_failed(void);    /* COARFINE +014 (BANKCALL's return) */
void coarfine_fine(void);           /* COARFINE +015 (CURTAINS's return) */
void rdcdus(void);                  /* RDCDUS (by RTB) */
extern const struct interpretive_code CHKSDATA_CODE, CHKSDATA_ENTER_CODE, CHKSDA_CODE, CAL53A_CODE,
    COARFINE_CODE, CALCSMSC_CODE;

/* Part 6, P51, P53, SXTSM -- p51.c (banks 14, 15, 17) */
void p51(void);                  /* P51, alias P53 */
void p51_alarm(void);            /* P51 +4 (P51A's return) */
void p51_pooh(void);             /* P51 +6 (ALARM's return) */
void p51aa(void);                /* P51AA (BANKCALL's return) */
void p51aa_terminate(void);      /* P51AA +3 (GOPERF1's +0) */
void p51aa_proceed(void);        /* P51AA +4 (+1, V33) */
void p51aa_enter(void);          /* P51AA +5 (+2) */
void p51aa_zero_gimbals(void);   /* P51AA +010 (PHASCHNG's return) */
void p51aa_display_v41(void);    /* P51AA +017 (BANKCALL's return) */
void p51aa_imucoars(void);       /* P51AA +022 (BANKCALL's return) */
void p51aa_after_imucoars(void); /* P51AA +024 (BANKCALL's return) */
void p51aa_coarse_failed(void);  /* P51AA +026 (BANKCALL's return) */
void p51aa_imufin20(void);       /* P51AA +027 (CURTAINS's return) */
void p51aa_after_imufin20(void); /* P51AA +031 (BANKCALL's return) */
void p51aa_fine_failed(void);    /* P51AA +033 (BANKCALL's return) */
void p51aa_coarse_done(void);    /* P51AA +034 (CURTAINS's return) */
void p51aa_again(void);          /* P51AA +037 (PHASCHNG's return) */
void p51b_plus2(void);           /* P51B +2 (PHASCHNG's return) */
void p51b_one_mark(void);        /* P51B +017 (after EXIT) */
void p51c_checkmm(void);         /* P51C +3 (PHASCHNG's return) */
void p51c_not_p53(void);         /* P51C +5 (CHECKMM's return) */
void p51c_p53(void);             /* P51C +6 (CHECKMM's +1) */
void p51e_plus1(void);           /* P51E +1 (after EXIT) */
void p51e_mkreleas(void);        /* P51E +4 (PHASCHNG's return) */
void p51e_star(void);            /* P51E +6 (BANKCALL's return) */
void p51e_second_star(void);     /* P51E +013 (PHASCHNG's return) */
void p51f_plus3(void);           /* P51F +3 (PHASCHNG's return) */
void p51f_again(void);           /* P51F +022 (after EXIT) */
void set1_pdt(void);             /* SET1/PDT (by RTB) */
extern const struct interpretive_code P51B_CODE, P53_CODE, P51C_1_CODE, P51F_CODE, P51G_CODE,
    SXTSM_CODE;

/* Part 7, R53, S52.2, SR52.1 -- r53_sr52_1.c (banks 14, 11, 13) */
void r53_after_exit(void);        /* R53A (after EXIT) */
void r53a_after_sxtmark(void);    /* R53A +4 (BANKCALL's return) */
void r53a_failed(void);           /* R53A +6 (BANKCALL's return) */
void r53a_marked(void);           /* R53A +7 (CURTAINS's return) */
void r53b_not_p22(void);          /* R53B +2 (CHECKMM's return) */
void r53b_p22(void);              /* R53B +3 (CHECKMM's +1) */
void r53b_not_p23(void);          /* R53B +6 (CHECKMM's return) */
void r53b_p23(void);              /* R53B +7 (CHECKMM's +1) */
void r53c1(void);                 /* R53C1 */
void r53c_after_cleandsp(void);   /* R53C (BANKCALL's return) */
void r53c_terminate(void);        /* R53C +3 (GOFLASH's +0) */
void r53c_proceed(void);          /* R53C +4 (+1) */
void r53c_enter(void);            /* R53C +5 (+2) */
void r53z(void);                  /* R53Z (TC R53C's return) */
void r53z_bad_code(void);         /* R53Z +1 (CHKSCODE's return) */
void r53z_after_falton(void);     /* R53Z +2 (FALTON's return) */
void r53z_code_good(void);        /* R53Z +3 (CHKSCODE's +2) */
void r53d(void);                  /* R53D */
void sr52_1(void);                /* SR52.1 */
void sr52_1_after_makecadr(void); /* SR52.1 +1 (MAKECADR's return) */
void sr52_1_cult(void);           /* SR52.1 +075 (after EXIT) */
void sr52e22_after_exit(void);    /* SR52E22 +1 (after EXIT) */
void sr52e3_after_exit(void);     /* SR52E3 +1 (after EXIT) */
extern const struct interpretive_code R53_CODE, R53OUT_CODE, S52_2_CODE, SR52_1_CODE, TRUN38_CODE,
    SR52E3_CODE;

/* Part 8, ADVTRACK, S52.3, R56, PLANET -- advtrack_r56_planet.c (banks 26,
 * 16, 17, 15) */
void advtrack_periods(void);       /* ADVTRACK +026 (after EXIT) */
void r56_after_exit(void);         /* R56 +2 (after EXIT) */
void r56_terminate(void);          /* R56 +5 (GOFLASH's +0) */
void r56_proceed(void);            /* R56 +6 (+1) */
void r56_enter(void);              /* R56 +7 (+2) */
void r56a(void);                   /* R56A */
void r56a_after_sxtmark(void);     /* R56A +2 (BANKCALL's return) */
void r56a1(void);                  /* R56A1 (BANKCALL's return) */
void r56a1_terminate(void);        /* R56A1 +3 (GOMARK2's +0, V34) */
void r56a1_again(void);            /* R56A1 +4 (+1, V33) */
void r56a1_mark(void);             /* R56A1 +5 (+2, enter) */
void r56a1_store_mark(void);       /* R56A1 +014 (after EXIT) */
void r56a1_after_clearmrk(void);   /* R56A1 +042 (CLEARMRK's return) */
void r56a1_marked_terminate(void); /* R56A1 +045 (GOPERF1's +0) */
void r56a1_marked_proceed(void);   /* R56A1 +046 (+1) */
void r56a1_marked_enter(void);     /* R56A1 +047 (+2) */
void nosam_after_exit(void);       /* NOSAM +1 (after EXIT) */
void nosam_terminate(void);        /* NOSAM +015 (GOFLASH's +0) */
void nosam_proceed(void);          /* NOSAM +016 (+1) */
void nosam_enter(void);            /* NOSAM +017 (+2) */
void nosam_planet(void);           /* NOSAM +020 */
extern const struct interpretive_code ADVTRACK_CODE, ADVTRACK_ROTA_CODE, S52_3_CODE, R56_CODE,
    R56A1_CODE, PLANET_CODE, NOSAM_CODE, NOTPLAN_CODE, CALSAM1_CODE;

#endif

#endif
