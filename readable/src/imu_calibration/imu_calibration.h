/* SPDX-License-Identifier: MIT */
/* IMU_CALIBRATION_AND_ALIGNMENT.agc of the CM (Comanche055 pp. 423-455):
 * the IMU performance tests, the prelaunch alignment (gyrocompassing) and
 * its optical verification, which the system test group ran on the pad.
 * The faithful C is c64-port/src/cm_imu_calibration.c.
 *
 * Written part by part:
 *   - imu_test.c (pp. 423-429): IMUTEST, the IMU performance test's
 *     positioning (GEOIMUTT), the PIPA test (PIPACHK, CHECKG), the
 *     vertical drift test (VERTDRFT) and the subroutines they share.
 *   - drift_loop.c (pp. 430-438, and the small routines of pp. 442 and
 *     446-447 it calls): EARTHR* and SHOW, FINETIME, and the drift test's
 *     loop: ESTIMS, SLEEPIE, the waitlist task ALLOOP every second and
 *     the job ALFLT it starts, up to the filter (NORMLOP).
 *
 * Every basic word is in bank 33 but FINETIME's (fixed-fixed). The
 * erasable words are EBANK 5's (POSITON, NDXCTR, QPLACE, ZERONDX1 ...).
 *
 * Each function runs from its word until control leaves it: for another
 * of this file's words that a run can come back to (continue_at; the
 * binding, src/binding/cm_imu_cal_routines.c, runs that word's function
 * next), another file's routine (BANKCALL, TWIDDLE, ...), or the
 * interpreter. A TC or TCF to a word of this file that nothing else comes
 * back to is a call of its function. */
#ifndef READABLE_IMU_CALIBRATION_H
#define READABLE_IMU_CALIBRATION_H

#include "../agc/agc.h"

#if COMANCHE055

#define E(name) ADDRESS_OF(name) /* an erasable word (EBANK 5's) */

/* A word's value as a signed number (ones' complement: -0 is 0). */
static inline long signed_word(word value)
{
    return (value & SIGN_BIT) ? -(long)(MINUS_ZERO & ~value) : (long)value;
}

/* INDEX k, then an instruction on erasable word base: the word C(k) on
 * (C(k) a signed count). The faithful port runs one in erasable memory
 * (0 to 01777) but Z; a store (TS) not into FB or BB either, and an LXCH
 * not into a central register (below 010). Any other is not modelled. */
enum indexed_use { INDEXED_READ, INDEXED_STORE, INDEXED_EXCHANGE };

static inline unsigned indexed_erasable(unsigned base, word index, enum indexed_use use)
{
    long at = (long)base + signed_word(index);

    if (at < 0 || at > 01777 || at == Z_ADDRESS) not_modelled();
    if (use == INDEXED_STORE && (at == FBANK_ADDRESS || at == BBANK_ADDRESS)) not_modelled();
    if (use == INDEXED_EXCHANGE && at < 010) not_modelled();
    return (unsigned)at;
}

/* TC Q: back through the return in Q (Q becomes 3). Q 3 is TC 3, the
 * EBANK register run as an instruction: the faithful port takes it as a
 * RELINT and goes on at the word EBANK names. Any other return below 010
 * or above 07777 is not modelled. */
static inline void tc_q(void)
{
    word at = (word)(overflow_corrected(Q) & MINUS_ZERO);

    if (at == Q_ADDRESS + 1u && EBANK >= 02000u) {
        relint();
        tc_at(Q_ADDRESS + 1u);
        Z = EBANK;
        return;
    }
    if (at < 010u || at > 07777u) not_modelled();
    tc_at(Q_ADDRESS);
    Z = at;
}

/* TC to an erasable word that holds a return (TC QPLACE, TC QPLACES, TC
 * MPAC): the AGC runs the word as TC <return>. A word below 010 or above
 * 07777 is not modelled. */
static inline void tc_return(unsigned address)
{
    word to = (word)(fetch(address) & MINUS_ZERO);

    if (to < 010u || to > 07777u) not_modelled();
    tc_through(address);
}

/* ---- imu_test.c: IMUTEST to ZEROING -- pp. 423-429 ---- */
void imutest(void);                 /* IMUTEST */
void imutest_after_coaalign(void);  /* IMUTEST +6 */
void geoimutt(void);                /* GEOIMUTT */
void geoimutt_after_imuzero(void);  /* GEOIMUTT +2 */
void imuback(void);                 /* IMUBACK */
void nbpospl(void);                 /* NBPOSPL */
void nbpospl_after_zeroing(void);   /* NBPOSPL +4 */
void nbpospl_after_exit(void);      /* NBPOSPL +021 */
void nbpospl_not_verifying(void);   /* NBPOSPL +023 */
void nbpospl_verifying(void);       /* NBPOSPL +024 */
void nbpospl_after_calcga(void);    /* NBPOSPL +031 */
void nbpospl_after_imucoars(void);  /* NBPOSPL +033 */
void nbpospl_after_downflag(void);  /* NBPOSPL +042 */
void nbpospl_after_imustllg(void);  /* NBPOSPL +043 */
void nbpospl_fine_align(void);      /* NBPOSPL +045 */
void nbpospl_after_imufine(void);   /* NBPOSPL +047 */
void nbpospl_settle(void);          /* NBPOSPL +050 */
void nbpospl_after_longcall(void);  /* NBPOSPL +055 */
void goestims(void);                /* GOESTIMS */
void torque(void);                  /* TORQUE */
void goestims_after_jobwake(void);  /* GOESTIMS +2 */
void pipachk(void);                 /* PIPACHK */
void pipachk_earthr(void);          /* PIPACHK +2 */
void pipachk_start(void);           /* PIPACHK +3 */
void pipachk_after_checkg(void);    /* PIPACHK +015 */
void pipachk_after_twiddle(void);   /* PIPACHK +021 */
void pipatask(void);                /* PIPATASK */
void startpip(void);                /* STARTPIP */
void startpip_after_findvac(void);  /* STARTPIP +4 */
void pipjobb(void);                 /* PIPJOBB */
void pipjobb_earthr(void);          /* PIPJOBB +2 */
void pipjobb_check_time(void);      /* PIPJOBB +3 */
void pipjobb_after_checkg(void);    /* PIPJOBB +012 */
void aingotn_after_exit(void);      /* AINGOTN +010 */
void aingotn_show(void);            /* AINGOTN +012 */
void vertdrft(void);                /* VERTDRFT */
void vertdrft_plus1(void);          /* VERTDRFT +1 */
void pong_after_earthr_star(void);  /* PONG +1 */
void valmis(void);                  /* VALMIS */
void endtest1(void);                /* ENDTEST1 */
void endtest1_after_downflag(void); /* ENDTEST1 +2 */
void endtest1_after_newmodex(void); /* ENDTEST1 +4 */
void endtest1_after_mkreleas(void); /* ENDTEST1 +6 */
void coaalign(void);                /* COAALIGN */
void coaalign_after_imucoars(void); /* COAALIGN +010 */
void coaalign_bad_stall(void);      /* COAALIGN +012 */
void coaalign_after_imustall(void); /* COAALIGN +013 */
void imustllg(void);                /* IMUSTLLG */
void checkg(void);                  /* CHECKG */
void checkg1(void);                 /* CHECKG1 */
void checkg_after_chang1(void);     /* CHECKG1 +5 */
void checkg_after_finetime(void);   /* CHECKG1 +024 */
void zeroing(void);                 /* ZEROING */

/* ---- drift_loop.c: EARTHR* to ALFLT, and the routines they call --
 * pp. 430-438, 442, 446-447 ---- */
void earthr_star(void);             /* EARTHR* */
void prout_after_exit(void);        /* PROUT +1 */
void prout_after_imustllg(void);    /* PROUT +2 */
void show(void);                    /* SHOW */
void show_after_terminate(void);    /* SHOW +7 */
void show_after_proceed(void);      /* SHOW +010 */
void show_after_enter(void);        /* SHOW +011 */
void p11out(void);                  /* P11OUT */
void finetime(void);                /* FINETIME */
void finetime_plus1(void);          /* FINETIME +1 */
void estims(void);                  /* ESTIMS */
void rstgts1(void);                 /* RSTGTS1 */
void rstgts1_after_zeroing(void);   /* RSTGTS1 +014 */
void estims_after_exit(void);       /* ESTIMS +031 */
void estims_drift_test(void);       /* ESTIMS +033 */
void estims_compass(void);          /* ESTIMS +034 */
void estims_after_erthrvse(void);   /* ESTIMS +040 */
void estims_after_newmodex(void);   /* ESTIMS +044 */
void estims_after_pipuse(void);     /* ESTIMS +046 */
void sleepie(void);                 /* SLEEPIE */
void sleepie_plus1(void);           /* SLEEPIE +1 */
void sleepie_after_phaschng(void);  /* SLEEPIE +3 */
void wtlistnt(void);                /* WTLISTNT */
void wtlistnt_after_chkcomed(void); /* WTLISTNT +1 */
void wtlistnt_after_setgwlst(void); /* WTLISTNT +2 */
void wtgtsmpl_after_twiddle(void);  /* WTGTSMPL +2 */
void storedta(void);                /* STOREDTA */
void alloop(void);                  /* ALLOOP */
void alloop1(void);                 /* ALLOOP1 */
void specsts(void);                 /* SPECSTS */
void specsts_after_findvac(void);   /* SPECSTS +4 */
void alflt(void);                   /* ALFLT */
void alflt_after_storedta(void);    /* ALFLT +1 */
void alflt_after_phaschng(void);    /* ALFLT +3 */
void alflt1(void);                  /* ALFLT1 */
void alflt1_after_loadstdt(void);   /* ALFLT1 +1 */
void alflt1_calibration(void);      /* ALFLT1 +3 */
void alflt1_compass(void);          /* ALFLT1 +4 */
void alflt1_after_chkcomed(void);   /* ALFLT1 +5 */
void normlop(void);                 /* NORMLOP */
void someerrr(void);                /* SOMEERRR */
void someerrr_after_alarm(void);    /* SOMEERRR +2 */
void somerr2(void);                 /* SOMERR2 */
void somerr2_after_alarm(void);     /* SOMERR2 +2 */
void somerr2_after_phaschng(void);  /* SOMERR2 +4 */
void annnnnn(void);                 /* ANNNNNN */
void chkcomed(void);                /* CHKCOMED */
void prelterm_after_priochng(void); /* PRELTERM +2 */

/* ---- gyrocompass.c: PERFERAS to NOCHORLD -- pp. 441-446 ---- */
void perferas_after_exit(void);     /* PERFERAS +1 */
void oncemore(void);                /* ONCEMORE */
void oncemore_torqndx(void);        /* ONCEMORE +2 */
void oncemore_cdux(void);           /* ONCEMORE +5 */
void torqinch(void);                /* TORQINCH */
void torqinch_after_phaschng(void); /* TORQINCH +2 */
void torqinch_after_imupulse(void); /* TORQINCH +5 */
void torqinch_after_imustllg(void); /* TORQINCH +6 */
void torqinch_erate(void);          /* TORQINCH +010 */
void torqinch_after_exit(void);     /* TORQINCH +014 */
void gtscpss(void);                 /* GTSCPSS */
void setupgc_after_zeroing(void);   /* SETUPGC +4 */
void setupgc_after_posn17c(void);   /* SETUPGC +5 */
void posn17c(void);                 /* POSN17C */
void posn17c_after_exit(void);      /* POSN17C +017 */
void endgtsal(void);                /* ENDGTSAL */
void endgtsal_chkcomed(void);       /* ENDGTSAL +2 */
void endgtsal_after_chkcomed(void); /* ENDGTSAL +3 */
void restaier(void);                /* RESTAIER */
void restaier_after_phaschng(void); /* RESTAIER +2 */
void restaier_after_exit(void);     /* RESTAIER +014 */
void restaier_after_earthr(void);   /* RESTAIER +015 */
void restest1(void);                /* RESTEST1 */
void restest1_after_exit(void);     /* RESTEST1 +5 */
void restest1_phaschng(void);       /* RESTEST1 +7 */
void restest3(void);                /* RESTEST3 */
void restest3_after_exit(void);     /* RESTEST3 +020 */
void restest3_after_posn17c(void);  /* RESTEST3 +021 */
void reschng(void);                 /* RESCHNG */
void spitgyro(void);                /* SPITGYRO */
void spitgyro_after_imupulse(void); /* SPITGYRO +3 */
void spitgyro_bad_stall(void);      /* SPITGYRO +5 */
void spitgyro_after_imustall(void); /* SPITGYRO +6 */
void noazchge_after_exit(void);     /* NOAZCHGE +1 */
void nochorld(void);                /* NOCHORLD */

/* ---- optical_verification.c: GCOMPVER to AZMTHCG1 -- pp. 449-454 ---- */
void data_word(void);               /* (a data word) */
void gcompver(void);                /* GCOMPVER */
void gcompver_after_phaschng(void); /* GCOMPVER +2 */
void setnbpos(void);                /* SETNBPOS */
void setnbpos_after_nbpospl(void);  /* SETNBPOS +1 */
void optdata(void);                 /* OPTDATA */
void optdata_display(void);         /* OPTDATA +010 */
void optdata_after_godspret(void);  /* OPTDATA +013 */
void optdata_terminate(void);       /* OPTDATA +016 */
void optdata_proceed(void);         /* OPTDATA +017 */
void optdata_enter(void);           /* OPTDATA +020 */
void tar_eref_minus1(void);         /* TAR/EREF -1 */
void contin33(void);                /* CONTIN33 */
void contin33_after_targdrve(void); /* CONTIN33 +4 */
void nextbnks_after_exit(void);     /* NEXTBNKS +014 */
void nexbnkss(void);                /* NEXBNKSS */
void nexbnkss_after_targdrve(void); /* NEXBNKSS +4 */
void gcomp4(void);                  /* GCOMP4 */
void gcomp4_terminate(void);        /* GCOMP4 +3 */
void gcomp4_proceed(void);          /* GCOMP4 +4 */
void gcomp4_enter(void);            /* GCOMP4 +5 */
void gcomp5(void);                  /* GCOMP5 */
void gcomp5_after_mkreleas(void);   /* GCOMP5 +2 */
void gcomp5_after_downflag(void);   /* GCOMP5 +4 */
void gcomp5_after_newmodex(void);   /* GCOMP5 +6 */
void gcomp5_after_phaschng(void);   /* GCOMP5 +010 */
void gtsoptcs_after_alarm(void);    /* GTSOPTCS +2 */
void latazchk_after_exit(void);     /* LATAZCHK +6 */
void latazchk_flash(void);          /* LATAZCHK +012 */
void latazchk_terminate(void);      /* LATAZCHK +015 */
void latazchk_proceed(void);        /* LATAZCHK +016 */
void latazchk_enter(void);          /* LATAZCHK +017 */
void latazchk_load(void);           /* LATAZCHK +020 */
void targdrve(void);                /* TARGDRVE */
void targdrve_after_exit(void);     /* TARGDRVE +014 */
void retarg_after_sxtmark(void);    /* RETARG +5 */
void retarg_bad_stall(void);        /* RETARG +7 */
void retarg_after_optstall(void);   /* RETARG +010 */
void retarg_mark(void);             /* RETARG +014 */
void azmthcg1(void);                /* AZMTHCG1 */
void azmthcg1_after_exit(void);     /* AZMTHCG1 +5 */
void azmthcg1_flash(void);          /* AZMTHCG1 +011 */
void azmthcg1_terminate(void);      /* AZMTHCG1 +014 */
void azmthcg1_proceed(void);        /* AZMTHCG1 +015 */
void azmthcg1_enter(void);          /* AZMTHCG1 +016 */
void azmthcg1_after_load(void);     /* AZMTHCG1 +025 */
void azmthcg1_after_phaschng(void); /* AZMTHCG1 +031 */

/* ---- The interpretive code, in the listing's notation ---- */

struct interpretive_code;
extern const struct interpretive_code GUESS_CODE;       /* imu_test.c, p. 423 */
extern const struct interpretive_code AXES_CODE;        /* NBPOSPL +7, p. 424 */
extern const struct interpretive_code CALCGA_CODE;      /* NBPOSPL +026, p. 424 */
extern const struct interpretive_code AINGOTN_CODE;     /* p. 426 */
extern const struct interpretive_code OVERFFIX_CODE;    /* p. 428 */
extern const struct interpretive_code ERTHRVSE_CODE;    /* drift_loop.c, p. 430 (bank 32) */
extern const struct interpretive_code EARTHR_CODE;      /* p. 430 */
extern const struct interpretive_code EARTHR_STAR_CODE; /* p. 430 */
extern const struct interpretive_code RSTGTS1_CODE;     /* p. 434 */
extern const struct interpretive_code ESTIMS_CODE;      /* ESTIMS +035, p. 434 */
extern const struct interpretive_code NORMLOP_CODE;     /* pp. 438-441 */

#endif
#endif
