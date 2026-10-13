/* SPDX-License-Identifier: MIT */
#ifndef CM_T4RUPT_IMU_CASES_H
#define CM_T4RUPT_IMU_CASES_H
#include "cm_t4rupt_imu.h"
#include "cm_native_image.h"
/* cm-t4rupt-imu cases: 2,048 states at IMUMON and 128 at each other entry of
 * cm_t4rupt_imu_routines (4,736), entered at the original address and run
 * for one routine; tools/cm_t4rupt_imu_reference.c runs the original
 * program from the same entry until it first reaches the continuation the
 * native run left in Z (and FB in banked memory). */
#define CTI_IMUMON_CASES 2048u
#define CTI_PER_ENTRY 128u
#define CTI_CASES (CTI_IMUMON_CASES+(CM_T4RUPT_IMU_ENTRIES-1u)*CTI_PER_ENTRY)
#define CTI_RETURNED 0u
#define CTI_REFUSED 2u
typedef struct {cm_native_io cm;agc_hardware hw;} cti_io;
agc_word cti_entry(uint16_t n);
void cti_input(apollo_overlay_state *,cti_io *,uint16_t n);
/* Runs case n natively (one routine); returns the outcome kind (3: not entered). */
unsigned cti_run(apollo_overlay_state *,cti_io *,uint16_t n);
uint32_t cti_digest(apollo_overlay_state *,uint16_t n);
#endif
