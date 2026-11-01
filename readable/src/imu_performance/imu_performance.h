/* SPDX-License-Identifier: MIT */
/* IMU_PERFORMANCE_TEST_2.agc and IMU_PERFORMANCE_TESTS_4.agc, the LM's
 * (Luminary099 pages 373-389): P07, the IMU performance test, run on the
 * ground. The faithful C is c64-port/src/lm_imu_performance.c.
 *
 * Basic code in bank 37 (and FINETIME, in fixed-fixed), with interpretive
 * code among it in the listing's notation, one table for each run of it
 * (IMU_PERFORMANCE_CODES), which the readable interpreter runs.
 *
 * Each function below starts at its word, with the machine as control
 * leaves it there (Z is the word), and returns with Z where the listing
 * goes next: another of these words, another file's routine, or NEWOPS
 * (after a TC INTPRET). pipachk_landing and pipjobb_landing take any of
 * the words an INDEX NDXCTR; TC +1 can land on, from Z. The binding is
 * src/binding/imu_performance_routines.c. */
#ifndef READABLE_IMU_PERFORMANCE_H
#define READABLE_IMU_PERFORMANCE_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if LUMINARY099

extern const struct interpretive_code *const IMU_PERFORMANCE_CODES[];
extern const unsigned IMU_PERFORMANCE_CODE_COUNT;

/* IMU_PERFORMANCE_TEST_2: the test's start, the latitude and azimuth. */
void redo(void);
void geoimutt(void);
void imuback(void);               /* IMUZERR's return */
void nbpospl_after_zeroing(void); /* NBPOSPL +4 */
void latazchk_after_exit(void);   /* LATAZCHK +6 */
void latazchk_display(void);      /* LATAZCHK +010 */
void latazchk_after_terminate(void);
void latazchk_after_proceed(void);
void latazchk_after_enter(void);
void latazchk_accepted(void); /* LATAZCHK +016 */
/* Coarse align, gimbal lock, and on to the PIPA test or the drift test. */
void posgmbl_after_exit(void);     /* POSGMBL +3 */
void posgmbl_after_imucoars(void); /* POSGMBL +5 */
void posgmbl_after_downflag(void); /* POSGMBL +014 */
void posgmbl_after_imuslllg(void); /* POSGMBL +015 */
void posgmbl_after_finimudd(void); /* POSGMBL +020 */
void posgmbl_after_longcall(void); /* POSGMBL +025 */
void goestims(void);
void goestims_after_jobwake(void);
void torque(void);
/* The PIPA test. */
void pipachk(void);
void pipachk_landing(void); /* PIPACHK +2 to +015 */
void pipachk_after_checkg(void);
void pipachk_after_twiddle(void);
void pipatask(void);
void startpip(void);
void startpip_after_findvac(void);
void pipjobb(void);
void pipjobb_landing(void); /* PIPJOBB +2 to +011 */
void pipjobb_after_checkg(void);
void aingotn_after_exit(void); /* AINGOTN +7 */
void aingotn_show(void);       /* AINGOTN +011 */
/* The vertical drift test, results and the end. */
void vertdrft(void);
void vertdrft_plus1(void);
void pon_after_earthr_star(void); /* PON +1 */
void valmis(void);
void endtest1(void);
void endtest1_after_downflag(void);
void endtest1_after_newmodea(void);
/* The IMU's modes: coarse and fine align, zero, wait. */
void coaalign(void);
void aligncoa(void);
void aligncoa_after_bad_stall(void);
void aligncoa_after_stall(void);
void imuslllg(void);
void finimudd(void);
void finimudd_after_imufine(void);
void imuzerr(void);
void imuzerr_after_imuzero(void);
/* Catching a PIPA pulse, and the time. */
void checkg(void);
void checkg1(void);
void checkg_after_chang1(void); /* CHECKG1 +5 */
void checkg_look(void);         /* CHECKG1 +011 */
void checkg_caught(void);       /* CHECKG1 +017 */
void checkg_after_finetime(void);
void zeroing(void);
void earthr_star(void);
void earthr_star_after_exit(void);     /* EARTHR* +6 */
void earthr_star_after_imuslllg(void); /* EARTHR* +7 */
void show(void);
void show1(void);
void show_after_terminate(void);
void show_after_proceed(void);
void show_after_enter(void);
void finetime(void);
void finetime_plus1(void);
void finetime_reread(void); /* FINETIME +010 */

/* IMU_PERFORMANCE_TESTS_4: the drift test's filter. */
void estims(void);
void estims_after_twiddle(void);
void estims_after_zeroing(void);
void vertskip_after_exit(void);
void alloop(void);
void alloop_after_twiddle(void);
void alloop_read_pipas(void);
void specsts_after_findvac(void);
void alflt(void);
void normlop(void);
void perferas_after_exit(void);
void perferas_after_atiginc(void);
void setuper1_after_exit(void);
void setuper1_after_imupulse(void);
void geostrt4(void);
void geostrt4_after_exit(void);
void sleepie(void);
void sleepie_plus1(void);
void sleepie_after_earthr_star(void); /* SLEEPIE +3 */
void someerrr(void);
void someerrr_plus1(void);
void someerrr_after_alarm(void);
void somerr2(void);
void somerr2_after_varalarm(void);
void somerr2_after_downflag(void);

#endif

#endif
