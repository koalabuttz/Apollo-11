/* SPDX-License-Identifier: MIT */
#ifndef LM_TIME_FREE_FALL_TARGET_H
#define LM_TIME_FREE_FALL_TARGET_H
#include "overlay_runtime.h"
/* Runs the frozen oracle call chains and the admission guards in the two
 * caller-supplied images; 0 on success, otherwise a failure code, with
 * *completed the oracle chains passed. */
int lm_time_free_fall_target_check(apollo_overlay_state *state,apollo_overlay_state *expected,unsigned *completed);
/* Guard cases checked by a passing run, beside the oracle chains. */
#define LM_TIME_FREE_FALL_GUARDS 22u
#endif
