/* SPDX-License-Identifier: MIT */
#ifndef IMU_PULSE_END_H
#define IMU_PULSE_END_H
#include "imu_pulse.h"
enum imu_gyro_end_entry {IMU_GYRO_WAKE_END=0,IMU_GYRO_BAD_END=1};
enum imu_gyro_end_status {IMU_GYRO_END_TASKOVER=0,IMU_GYRO_END_UNSUPPORTED=1};
/* Actual source JOBWAKE/NORESET/IMUFINED/ENDIMU or IMUBAD through literal
 * TASKOVER entry. IMU device0 only, complete canonical Executive arena and
 * valid basic wake helper domain, nonnull owned image/device objects, excluded
 * producers. Ordinary banks/RAM, rawA/L, source bank07 and actual entry PC/Q.
 * Positive MODECADR or source sentinels +0/-0/-1; actual source differences
 * between LM/CM negative sentinel routes are retained. No scheduler RESUME,
 * interpreted job body or hardware pulse consumer runs here. */
enum imu_gyro_end_status imu_gyro_end(apollo_overlay_state *,imu_pulse_io *,enum rtb_mission,enum imu_gyro_end_entry);
#endif
