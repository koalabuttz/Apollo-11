/* SPDX-License-Identifier: MIT */
#ifndef CM_T4RUPT_IMU_H
#define CM_T4RUPT_IMU_H
#include "agc_transfer.h"
/* Comanche055 T4RUPT_PROGRAM.agc, the IMU monitor (bank 06,2174-2764, with
 * ZOPFIN3 06,3065-3067 that LAMPTEST shares): IMUMON, TNONTEST, PROCTNON,
 * the ENDTNON/UNZ2/ISSUP tasks, OPONLY/ISSZERO, C33TEST, GLOCKMON, TLIM,
 * ITURNON, IMUCAGE, IMUOP, PIPFAIL, DNTMFAST, UPTMFAST, SETISSW, CAGESUB and
 * LAMPTEST as routines of the control-transfer ABI (milestone cm-t4rupt-imu,
 * docs/CM-T4RUPT-IMU.md).
 *
 * Entries: IMUMON (T4JUMP's target), the tasks ENDTNON, UNZ2 and ISSUP,
 * SETISSW (from IMU_MODE_SWITCHING_ROUTINES), and the continuation after
 * each call that leaves the file (ALARM, WAITLIST, VARDELAY, IBNKCALL,
 * ZEROICDU, VARALARM). Each runs to the next of those calls, TASKOVER,
 * POSTJUMP IMUBAD or RESUME (GLOCKOK). Channels 11 (DSALMOUT), 12, 14, 30
 * and 33 are program->hardware's. */
#define CM_T4RUPT_IMU_ENTRIES 22u
extern const agc_routine_table cm_t4rupt_imu_routines;
#endif
