/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_PREPARE_H
#define GUIDANCE_PREPARE_H
#include "guidance.h"
/* MPAC[7] IS MODE; +2 and DELVROD's low words are retained scratch. */
typedef struct {
    p66_rate_state rate;
    agc_word pipa[3];
    agc_dp time;
    agc_word old_pipa[3], ruptreg[3];
    agc_dp this_piptime;
    agc_word pipatmp[3], tem[3], mpac[8], delvrod[6];
} p66_preparation;
/* Own all modeled state and exclude its sensor/count/time producers.
 * Snapshot stage: 32,3303..3314; preserves A/L exchange debris. */
void p66_snapshot(p66_preparation *state);
/* Vector-input stage: 32,3315..3352. Return bits 0..5 indicate the six
 * arithmetic TS skips, an observation added to the native ABI. No saturation,
 * alarm or short-circuit is introduced. Stop before TC INTPRET at 32,3353. */
uint8_t p66_vector_inputs(p66_preparation *state);
/* Compose the existing rate update, snapshot and vector-input stages. */
uint8_t p66_prepare(p66_preparation *state);
#endif
