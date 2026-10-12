/* SPDX-License-Identifier: MIT */
#ifndef LM_EPHEMERIS_PHASE_TARGET_H
#define LM_EPHEMERIS_PHASE_TARGET_H
#include "overlay_runtime.h"
/* Frozen native phase steps (tools/freeze_lm_ephemeris_phases.py) of the C64
 * diagnostic group LM_EPHEMERIS_GROUP (1 entry and POSITB loop, 2 POSITD to
 * the solar angles, 3 lunar MXV/UNIT/STORE, 4 solar MXV/UNIT/STORE and RVQ),
 * or of every group when it is 0 or undefined. Each step starts from its sampled entry state plus the
 * frozen entry delta and must produce exactly the frozen next phase,
 * IRQ/SUPER and image. 0 on success, otherwise a failure code, with
 * *completed the steps passed. */
int lm_ephemeris_phase_check(apollo_overlay_state *state,apollo_overlay_state *expected,unsigned *completed);
#endif
