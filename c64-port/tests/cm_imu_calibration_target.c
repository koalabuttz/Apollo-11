/* SPDX-License-Identifier: MIT */
/* The program the cm-imu-calibration cases run: Comanche055
 * IMU_CALIBRATION_AND_ALIGNMENT's routines alone, entered at NEWOPS. */
#include "agc_transfer.h"
#include "cm_imu_calibration.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program cm={&cm_imu_calibration_basic,1,&cm_imu_calibration_interpretive,1,0,0,
 (uint16_t)(2048u+06037u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &cm;}
