/* SPDX-License-Identifier: MIT */
#ifndef IMU_GYRO_DEVICE_H
#define IMU_GYRO_DEVICE_H
#include "imu_pulse.h"
#define IMU_GYRO_OUTPUT_EVENTS 18u
enum imu_gyro_clock_mode {IMU_GYRO_CLOCK_BURST=0,IMU_GYRO_CLOCK_TIMED=1};
typedef struct {uint32_t phase;agc_word pending,selection;} imu_gyro_device;
/* Native gyro peripheral binding, one eligible source-peripheral clock step
 * (6/160 nominal3200pps), not one C64 CPU cycle. Explicit external device
 * state and output-event buffer; no private persistent state. Burst transport
 * mirrors the source reference backend; timed transport consumes one pulse
 * at each source divider crossing and clears activity on the final pulse.
 * Positive GYROCTR0..037777; ordinary channel14 and valid mode-specific state;
 * nonnull distinct image/io/device/event/count objects; producers excluded.
 * Events are original channel0177 payloads for simulated IMU transport.
 * No instructions, scheduler, wall-clock source or physical hardware run here.
 * Unsupported input returns0 with all output objects unchanged. */
uint8_t imu_gyro_device_step(apollo_overlay_state *,imu_pulse_io *,imu_gyro_device *,enum imu_gyro_clock_mode,agc_word events[IMU_GYRO_OUTPUT_EVENTS],uint8_t *count);
#endif
