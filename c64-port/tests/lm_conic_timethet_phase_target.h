/* SPDX-License-Identifier: MIT */
#ifndef LM_CONIC_TIMETHET_PHASE_TARGET_H
#define LM_CONIC_TIMETHET_PHASE_TARGET_H
#include "overlay_runtime.h"
/* Frozen native phase steps (tools/freeze_lm_conic_timethet_phases.py) of
 * the C64 diagnostic group LM_CONIC_TIMETHET_GROUP (1 TIMETHET, PARAM, GEOM
 * and its UNIT paths, 2 GETX, its WLOOP iteration, the POLY series and the
 * ABTCONIC paths, 3 XCOMMON through INFINITY, 4 COMMNOUT, DELTIME, NEWSTATE
 * and the final GOTO), or of every group when it is 0 or
 * undefined. Each step starts from its case's orbit-free debris
 * image (lm_conic_timethet_debris) plus the frozen entry delta and must produce exactly the frozen next phase,
 * IRQ/SUPER and image. 0 on success, otherwise a failure code, with
 * *completed the steps passed. */
int lm_conic_timethet_phase_check(apollo_overlay_state *state,apollo_overlay_state *expected,unsigned *completed);
#endif
