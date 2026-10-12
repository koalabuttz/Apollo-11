/* SPDX-License-Identifier: MIT */
#ifndef LM_CONIC_LAMBERT_PHASE_TARGET_H
#define LM_CONIC_LAMBERT_PHASE_TARGET_H
#include "overlay_runtime.h"
/* Frozen native phase steps (tools/freeze_lm_conic_lambert_phases.py) of
 * the C64 diagnostic group LM_CONIC_LAMBERT_GROUP (1 the LAMBERT entry with
 * GEOM, 2 GEOM's return through the COGA bounds, 3 MINCOGA, NOGUESS,
 * LAMBLOOP and GETX's return, 4 GETX through POLYCOEF, 5 XCOMMON through
 * INFINITY, 6 DELTIME, 7 DELTIME's and ITERATOR's returns through 360LAMB,
 * 8 INITV and TARGETV, 9 ITERATOR), or of every group when it is 0 or
 * undefined. Each step starts from its case's orbit-free debris image
 * (lm_conic_lambert_debris) plus the frozen entry delta and must produce
 * exactly the frozen next phase, IRQ/SUPER and image. 0 on success,
 * otherwise a failure code, with *completed the steps passed. */
int lm_conic_lambert_phase_check(apollo_overlay_state *state,apollo_overlay_state *expected,unsigned *completed);
#endif
