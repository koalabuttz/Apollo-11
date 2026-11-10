/* SPDX-License-Identifier: MIT */
/* IMU_MODE_SWITCHING_ROUTINES.agc: the cage tests the file's routines
 * share (LM page 1327, CM page 1438). Each returns by TC Q, or leaves for
 * IMUBAD or MODEEXIT; the caller's tc_at() sets Q first, as its TC does. */
#ifndef READABLE_IMU_MODE_SUBROUTINES_H
#define READABLE_IMU_MODE_SUBROUTINES_H

#include "imu_mode_switching.h"

/* CAGETEST: true if it returned (A +0); false if the IMU has been caged,
 * and it went on to IMUBAD. */
bool cagetest(void);
/* CAGETSTQ: true if it skipped (INCR Q: the IMU is not being caged). */
bool cagetstq(void);
/* CAGETSTJ: true if it returned; false if the IMU is being caged, and it
 * set IMUCADR to -0 and left by MODEEXIT. */
bool cagetstj(void);
/* CAGETSTJ +4: IMUCADR to -0 ("OPERATION COMPLETE BUT FAILED"), and
 * MODEEXIT. */
void cagetstj_failed(void);

#endif
