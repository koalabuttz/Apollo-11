/* SPDX-License-Identifier: MIT */
#ifndef LM_PERIAPO_PHASE_TARGET_H
#define LM_PERIAPO_PHASE_TARGET_H
#include "overlay_runtime.h"
/* Frozen native phase steps (tools/freeze_lm_periapo_phases.py) of the C64
 * diagnostic group LM_PERIAPO_GROUP (1 the PERIAPO family phases, 2 the
 * APSIDES entry and PARAM, 3 GETECC and INFINAPO, 4 GEOM through PARAM's
 * return), or of every group
 * when it is 0 or undefined. Each step starts from its case's orbit-free
 * debris image (lm_periapo_debris) plus the frozen entry delta and must
 * produce exactly the frozen next phase, IRQ/SUPER and image. 0 on
 * success, otherwise a failure code, with *completed the steps passed. */
int lm_periapo_phase_check(apollo_overlay_state *state,apollo_overlay_state *expected,unsigned *completed);
#endif
