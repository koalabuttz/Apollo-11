/* SPDX-License-Identifier: MIT */
#ifndef LM_TIME_FREE_FALL_PHASE_TARGET_H
#define LM_TIME_FREE_FALL_PHASE_TARGET_H
#include "overlay_runtime.h"
/* Frozen native phase steps (tools/freeze_lm_time_free_fall_phases.py) of
 * the C64 diagnostic group LM_TIME_FREE_FALL_GROUP (1 TFFCONIC/TFFCONMU and
 * TFFRP/RA, 2 CALCTPER/CALCTFF through TFFXTEST, 3 the series, polynomial and
 * TFF exits, 4 the ellipse path), or of every group when it is 0 or
 * undefined; LM_TIME_FREE_FALL_PART (1..) links one part of the group's
 * rows (one PRG each), 0 or undefined every part. Each step starts from its case's orbit-free debris
 * image (lm_time_free_fall_debris) plus the frozen entry delta and must produce exactly the frozen next phase,
 * IRQ/SUPER and image. 0 on success, otherwise a failure code, with
 * *completed the steps passed. */
int lm_time_free_fall_phase_check(apollo_overlay_state *state,apollo_overlay_state *expected,unsigned *completed);
#endif
