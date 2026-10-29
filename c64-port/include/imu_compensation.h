/* SPDX-License-Identifier: MIT */
#ifndef IMU_COMPENSATION_H
#define IMU_COMPENSATION_H
#include "agc_transfer.h"
/* IMU_COMPENSATION_PACKAGE (Luminary099 and Comanche055, bank 06) as native
 * routines of the control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md).
 *
 * Each routine is a compiled translation of the source from its entry to the
 * point where control leaves the file: the caller's SWRETURN, TC NOVAC with
 * the 2CADR 1/GYRO at Q, TC BANKCALL (IMUPULSE, IMUSTALL, PIPUSE1/PIPUSE),
 * LM TC IBNKCALL (PIPASR +3) or TCF ENDOFJOB. Where the original continues
 * after such a call (NOVAC's return to Q+2, BANKCALL/IBNKCALL returns to
 * Q+1), the continuation is a routine of its own. The file's local
 * subroutines (IRIGX/Y/Z, GCOMPSUB, DRIFTSUB, FBIASSUB, DRFTSUB2) are reached
 * only from inside the file and are compiled into their callers, with the
 * original Q effects: a return through TC MPAC +2 or TC BUF +1 executes the
 * stored TC and leaves Q at 0157 or 0132, TC Q leaves Q at 3.
 *
 * io is the mission's device record (p66_bias_io for Luminary, cm_native_io
 * for Comanche); INHINT and RELINT set its interrupt_enabled.
 *
 * Refused (EXEC_OUT_OF_SCOPE, image unchanged): the Luminary continuation
 * after IBNKCALL PIPASR +3 with TEM1 other than +0 or BIT8, which NBDONLY
 * stores just before the call. */
extern const agc_routine_table lm_imu_compensation_routines;
extern const agc_routine_table cm_imu_compensation_routines;

/* Luminary099 entries and continuations, bank 06. */
#define LM_ICP_1PIPA 03263u
#define LM_ICP_NOVAC_RETURN 03352u
#define LM_ICP_1GYRO 03507u
#define LM_ICP_IMUPULSE_RETURN 03535u
#define LM_ICP_IMUSTALL_RETURN 03537u
#define LM_ICP_GCOMP1 03540u
#define LM_ICP_NBDONLY 03555u
#define LM_ICP_PIPASR_RETURN 03575u
#define LM_ICP_LASTBIAS 03663u
#define LM_ICP_PIPUSE1_RETURN 03665u
#define LM_ICP_GCOMPZER 03701u
#define LM_ICP_SWRETURN 04631u
#define LM_ICP_BANKCALL 04616u
#define LM_ICP_IBNKCALL 04674u
#define LM_ICP_NOVAC 05072u
#define LM_ICP_ENDOFJOB 05155u

/* Comanche055 entries and continuations, bank 06. */
#define CM_ICP_1PIPA 03262u
#define CM_ICP_NOVAC_RETURN 03406u
#define CM_ICP_1GYRO 03474u
#define CM_ICP_IMUPULSE_RETURN 03522u
#define CM_ICP_IMUSTALL_RETURN 03524u
#define CM_ICP_GCOMP1 03525u
#define CM_ICP_NBDONLY 03542u
#define CM_ICP_LASTBIAS 03636u
#define CM_ICP_PIPUSE_RETURN 03640u
#define CM_ICP_SWRETURN 04675u
#define CM_ICP_BANKCALL 04662u
#define CM_ICP_NOVAC 05134u
#define CM_ICP_ENDOFJOB 05217u
#endif
