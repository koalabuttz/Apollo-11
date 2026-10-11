/* SPDX-License-Identifier: MIT */
#ifndef LM_EPHEMERIS_TARGET_H
#define LM_EPHEMERIS_TARGET_H
#include "overlay_runtime.h"
/* Runs the frozen oracle states and the admission guards in the two
 * caller-supplied images (the C64 diagnostic places them under KERNAL ROM);
 * 0 on success, otherwise a failure code, with *completed the oracle states
 * passed. */
int lm_ephemeris_target_check(apollo_overlay_state *state,apollo_overlay_state *expected,unsigned *completed);
/* Guard cases checked by a passing run, beside the oracle states. */
#define LM_EPHEMERIS_GUARDS 21u
#endif
