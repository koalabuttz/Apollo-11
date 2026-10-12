/* SPDX-License-Identifier: MIT */
#ifndef LM_IMU_PERFORMANCE_CASES_H
#define LM_IMU_PERFORMANCE_CASES_H
#include "lm_imu_performance.h"
/* lm-imu-performance cases: every routine of lm_imu_performance_routines
 * and lm_imu_performance_interpretive, IPT_STATES states each, run from its
 * key until control leaves the file (include/lm_imu_performance.h).
 * tools/lm_imu_performance_reference.c runs the same image, interrupt
 * state and scaler words in the unmodified rope from the same Z and stops
 * where ipt_leaves() says; the 2048 erasable words and the interrupt-enable
 * state are compared. States are drawn inside the routines' domain; the
 * refusals are exercised by tests/test_lm_imu_performance.c. */
#define IPT_STATES 512u
#define IPT_CASES (IPT_ROUTINES*IPT_STATES)
#define IPT_STOPPED 0u
#define IPT_REFUSED 2u
typedef struct {
 agc_word superbank;
 uint16_t interrupt_enabled;
 agc_word hiscalar,loscalar;
} ipt_case;
void ipt_input(apollo_overlay_state *, ipt_case *, uint16_t n);
/* Where both programs stop: 1 when the original leaves the file and
 * interpreter at this instruction boundary (Z under FB/superbank; LOC under
 * BANKSET at NEWOPS). */
int ipt_leaves(agc_word z, agc_word fb, agc_word superbank, agc_word loc, agc_word bankset);
/* Runs case n natively; returns the kind (3: anything else) and the final
 * interrupt-enable state. */
unsigned ipt_run(apollo_overlay_state *, uint16_t n, uint16_t *interrupt_enabled);
uint32_t ipt_digest(apollo_overlay_state *, uint16_t n);
#endif
