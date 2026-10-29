/* SPDX-License-Identifier: MIT */
#ifndef LM_FLESHPOT_H
#define LM_FLESHPOT_H
#include "overlay_runtime.h"
#include "agc_word.h"
enum lm_fleshpot_status { LM_FLESHPOT_OK=0,LM_FLESHPOT_OUT_OF_SCOPE=1 };
enum lm_fleshpot_phase {LM_FLESHPOT_FIRST_ROW=0,LM_FLESHPOT_OTHER_ROW,LM_FLESHPOT_CROSS,LM_FLESHPOT_FINISH};
/* Original FLESHPOT basic source, canonical scratch/destination ownership.
 * Internal phase returns retain sourceowner; CROSS ends pendingEXIT DANZIG. */
enum lm_fleshpot_status lm_fleshpot(apollo_overlay_state *,enum lm_fleshpot_phase);
#endif
