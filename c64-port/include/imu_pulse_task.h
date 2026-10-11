/* SPDX-License-Identifier: MIT */
#ifndef IMU_PULSE_TASK_H
#define IMU_PULSE_TASK_H
#include "imu_pulse.h"
enum imu_gyro_task_status {IMU_GYRO_WAITLIST=0,IMU_GYRO_JOBWAKE=1,IMU_GYRO_BAD=2,IMU_GYRO_UNSUPPORTED=3,IMU_GYRO_TASKOVER=4};
/* Deferred original STRTGYRO or8192AUG body through the actual WAITLIST,
 * JOBWAKE or IMUBAD handoff. Exclusive image/device ownership, ordinary RAM,
 * coherent bank07, idle output counters and initialized LGYRO phase0..3 with
 * E4..E7 six-word command disjoint from scratch/queue are caller obligations.
 * Augment input activity bit01000 must be clear during this isolated
 * numerical qualification. IRQ remains inherited. No external scheduler/alarm/hardware consumer runs. */
enum imu_gyro_task_status imu_gyro_task(apollo_overlay_state *,imu_pulse_io *,enum rtb_mission,uint8_t augment);
/* Successful queued next segment, through power-supply activation to the
 * literal TCF TASKOVER instruction. GYROCMD and activity channel bit now
 * describe a pending physical pulse train; its asynchronous consumption and
 * TASKOVER/interrupt continuation are external caller-owned services. */
enum imu_gyro_task_status imu_gyro_register(apollo_overlay_state *,imu_pulse_io *,enum rtb_mission);
#endif
