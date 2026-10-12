/* SPDX-License-Identifier: MIT */
#ifndef LM_MIDGIM_PHASE_TARGET_H
#define LM_MIDGIM_PHASE_TARGET_H
#include "overlay_runtime.h"
/* Frozen native phase steps (tools/freeze_lm_midgim_phases.py) of the C64
 * diagnostic group LM_MIDGIM_GROUP (1 GET+MGA, 2 GET.LVC, 3 SELECTMU), or of every group when it is 0 or
 * undefined. Each step starts from its case's orbit-free
 * debris image (lm_midgim_debris) plus the frozen entry delta and must
 * produce exactly the frozen next phase, IRQ/SUPER and image. 0 on
 * success, otherwise a failure code, with *completed the steps passed. */
int lm_midgim_phase_check(apollo_overlay_state *state,apollo_overlay_state *expected,unsigned *completed);
#endif
