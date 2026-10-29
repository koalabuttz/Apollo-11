/* SPDX-License-Identifier: MIT */
#ifndef LM_CONIC_KEPLER_PHASE_TARGET_H
#define LM_CONIC_KEPLER_PHASE_TARGET_H
#include "overlay_runtime.h"
/* Frozen native phase steps (tools/freeze_lm_conic_kepler_phases.py) of
 * the C64 diagnostic group LM_CONIC_KEPLER_GROUP (1 KEPLERN through
 * STORBNDS, 2 KEPLOOP and DELTIME, 3 STOREMIN/DXCOMP and the Newton/bound
 * update through BRNCHCTR, 4 BADX, the time overflow paths and KEPCONVG), or of every group when it is 0 or
 * undefined. Each step starts from its case's orbit-free debris
 * image (lm_conic_kepler_debris) plus the frozen entry delta and must produce exactly the frozen next phase,
 * IRQ/SUPER and image. 0 on success, otherwise a failure code, with
 * *completed the steps passed. */
int lm_conic_kepler_phase_check(apollo_overlay_state *state,apollo_overlay_state *expected,unsigned *completed);
#endif
