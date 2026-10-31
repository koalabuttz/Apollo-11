/* SPDX-License-Identifier: MIT */
/* IMU_COMPENSATION_PACKAGE.agc, both missions (Luminary099 pages 326-337,
 * Comanche055 pages 297-306): the compensation of the inertial platform's
 * known errors, all of it basic code in bank 06. The faithful C is
 * c64-port/src/imu_compensation.c.
 *
 * Two kinds of error are compensated, both measured on the ground and
 * loaded as coefficients in E-bank 3:
 *   - the accelerometers' (the PIPAs'): a scale factor error (PIPASCF) and
 *     a bias (PIPABIAS), corrected in the counts SERVICER has just read
 *     (DELVX, DELVY, DELVZ), by 1/PIPA;
 *   - the gyros' drift: a bias drift (NBD), and drifts in proportion to
 *     the acceleration along the gyro's input axis (ADIA) and spin axis
 *     (ADSRA). These add up in GCOMP, a double word per gyro, in
 *     fractions of a gyro pulse. When a gyro's sum grows past two pulses
 *     the job 1/GYRO sends the whole pulses to the gyros (IMUPULSE).
 *
 * 1/PIPA runs after each PIPA reading while SERVICER runs (powered
 * flight). In free fall NBDONLY does the bias drift alone, from the time
 * since the last compensation (1/PIPADT): a job WAITLIST's SVCT3 starts
 * while the drift flag is set, "ENABLE EVERY 81.93 SECONDS"; on the lunar
 * surface (the LM's SURFFLAG) it does the acceleration terms too.
 * LASTBIAS, a job PREREAD starts as SERVICER begins, does the "LAST GYRO
 * COMPENSATION IN FREE FALL".
 *
 * The entries are below: the routines' starts and each word a call out of
 * the file returns to (NOVAC's, BANKCALL's two, the LM's IBNKCALL's).
 * The file's subroutines (IRIGX, IRIGY, IRIGZ, GCOMPSUB, DRIFTSUB,
 * DRFTSUB2, FBIASSUB) are reached only from inside it. The binding is
 * src/binding/imu_compensation_routines.c. */
#ifndef READABLE_IMU_COMPENSATION_H
#define READABLE_IMU_COMPENSATION_H

#include "../agc/agc.h"

void one_pipa(void);                /* 1/PIPA */
void irigcomp_after_novac(void);    /* the RELINT before IRIG1: NOVAC's return */
void one_gyro(void);                /* 1/GYRO */
void one_gyro_after_imupulse(void); /* LGCOMP +4: BANKCALL IMUPULSE's return */
void one_gyro_after_imustall(void); /* LGCOMP +6: BANKCALL IMUSTALL's return */
void gcomp1(void);
void nbdonly(void);
void lastbias(void);
void lastbias_after_pipuse(void); /* LASTBIAS +2: BANKCALL PIPUSE1's (CM PIPUSE's) return */
#if LUMINARY099
void nbdonly_after_pipasr(void); /* NBDONLY +020: IBNKCALL PIPASR +3's return */
void gcompzer(void);
#endif

#endif
