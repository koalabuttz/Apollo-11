/* SPDX-License-Identifier: MIT */
#ifndef LM_CONIC_TIMETHET_TARGET_H
#define LM_CONIC_TIMETHET_TARGET_H
#include "overlay_runtime.h"
/* Runs the frozen oracle entries and the admission guards in the two
 * caller-supplied images; 0 on success, otherwise a failure code, with
 * *completed the oracle entries passed. */
int lm_conic_timethet_target_check(apollo_overlay_state *state,apollo_overlay_state *expected,unsigned *completed);
/* Guard cases checked by a passing run, beside the oracle entries. */
#define LM_CONIC_TIMETHET_GUARDS 22u
#endif
