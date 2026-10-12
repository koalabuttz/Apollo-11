/* SPDX-License-Identifier: MIT */
#ifndef LM_INITVEL_PHASE_TARGET_H
#define LM_INITVEL_PHASE_TARGET_H
#include "overlay_runtime.h"
/* Frozen native phase steps (tools/freeze_lm_initvel_phases.py) of the C64
 * diagnostic group LM_INITVEL_GROUP (1 INITVEL through INITVEL1, 2
 * LAMBERT's entry and its two returns to INITVEL, 3 INITVEL2 and INITVEL3, 4
 * LAMBERT's return through INITVELX), or of every group when it is 0 or
 * undefined. Each step starts from its case's orbit-free
 * debris image (lm_initvel_debris) plus the frozen entry delta and must
 * produce exactly the frozen next phase, IRQ/SUPER and image. 0 on
 * success, otherwise a failure code, with *completed the steps passed. */
int lm_initvel_phase_check(apollo_overlay_state *state,apollo_overlay_state *expected,unsigned *completed);
#endif
