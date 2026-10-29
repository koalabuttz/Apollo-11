/* SPDX-License-Identifier: MIT */
#ifndef SBAND_LM_PITCH_H
#define SBAND_LM_PITCH_H
#include "sband_plane.h"
enum sband_lm_pitch_status {SBAND_LM_PITCH_UNSUPPORTED,SBAND_LM_PITCH_OK,SBAND_LM_PITCH_ALARM1302,SBAND_LM_PITCH_ALARM1301};
/* Actual normal LM antenna-plane return through full pitch calculation.
 * Five valid VAC/exact stack, ordinary data/coherent original banks,
 * NEWJOB+0 and producer exclusion across all paired operators required.
 * Stops before NOADJUST yaw operations or before true alarm service. */
enum sband_lm_pitch_status sband_lm_pitch(apollo_overlay_state *,p66_bias_io *);
#endif
