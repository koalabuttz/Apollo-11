/* SPDX-License-Identifier: MIT */
#ifndef SBAND_FRAME_H
#define SBAND_FRAME_H
#include "sband_orbit.h"
enum sband_frame_stage {SBAND_FRAME_UNIT,SBAND_FRAME_AFTER_UNIT,SBAND_FRAME_MATRIX,SBAND_FRAME_SHIFT};
enum sband_frame_status {SBAND_FRAME_UNSUPPORTED,SBAND_FRAME_OK,SBAND_FRAME_ABORT_1302};
/* Explicit original antenna source continuations. UNIT/AFTER_UNIT LM only;
 * MATRIX/SHIFT both missions. All word state is canonical; callers exclude
 * producers throughout delegated phases. External orbital/display bodies
 * and source DANZIG scheduling are not silently executed. */
enum sband_frame_status sband_frame(apollo_overlay_state *,p66_bias_io *,enum apollo_fixed_mission,enum sband_frame_stage);
#endif
