/* SPDX-License-Identifier: MIT */
#ifndef LM_T4RUPT_IMU_CASES_H
#define LM_T4RUPT_IMU_CASES_H
#include "lm_t4rupt_imu.h"
#include "p66_bias.h"
/* lm-t4rupt-imu cases: 2,048 states at IMUMON and 128 at each other entry of
 * lm_t4rupt_imu_routines (4,736), entered at the original address and run
 * for one routine; tools/lm_t4rupt_imu_reference.c runs the original
 * program from the same entry until it first reaches the continuation the
 * native run left in Z (and FB in banked memory). */
#define LTI_IMUMON_CASES 2048u
#define LTI_PER_ENTRY 128u
#define LTI_CASES (LTI_IMUMON_CASES+(LM_T4RUPT_IMU_ENTRIES-1u)*LTI_PER_ENTRY)
#define LTI_RETURNED 0u
#define LTI_REFUSED 2u
typedef struct {p66_bias_io lm;agc_hardware hw;} lti_io;
agc_word lti_entry(uint16_t n);
void lti_input(apollo_overlay_state *,lti_io *,uint16_t n);
/* Runs case n natively (one routine); returns the outcome kind (3: not entered). */
unsigned lti_run(apollo_overlay_state *,lti_io *,uint16_t n);
uint32_t lti_digest(apollo_overlay_state *,uint16_t n);
#endif
