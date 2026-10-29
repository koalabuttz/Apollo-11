/* SPDX-License-Identifier: MIT */
#ifndef RTB_UNIT_H
#define RTB_UNIT_H
#include "rtb_vector.h"
enum rtb_unit_status {RTB_UNIT_OK,RTB_UNIT_ABORT_1302,RTB_UNIT_OVERFLOW,RTB_UNIT_OUT_OF_SCOPE};
/* Original UNIT+1 continuation used by the NORM frontiers, in three source
 * basic subroutine phases. No Executive handoff is admitted between phases.
 * Ordinary canonical image/raw16 A/L, source bank0, vector MODE-1 and five
 * valid VAC bases required. Stops after VSQSUB, normalized SQRTSUB, then at
 * actual mission DANZIG (or source 1302/overflow endpoint). All state lives
 * in canonical scratch/VAC; no private pointer survives an ABI return. */
/* Original UNIT TC VECAGREE prologue, ending at UNIT+1. */
enum rtb_unit_status rtb_unit_begin(apollo_overlay_state *,p66_bias_io *,enum rtb_mission);
enum rtb_unit_status rtb_unit_phase(apollo_overlay_state *,p66_bias_io *,enum rtb_mission,uint8_t phase);
#endif
