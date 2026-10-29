/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_VELOCITY_PHASED_H
#define GUIDANCE_VELOCITY_PHASED_H
#include "guidance_velocity.h"
/* Three explicit native groups, each ending at a genuine original EDOP=0
 * DANZIG boundary. Execute in order; caller owns all objects and excludes
 * producers. ADDRWD is the sole ordinary physical-word owner at0116.
 * VSU/VXSC pops the DP quotient, retains six saved velocity words and reaches
 * LOC03366. VSL2/VAD V reaches03370. The final VAD/STADR consumes STOVL24/R;
 * LOC03371 is not a safe handoff, because STADR remains pending in EDOP.
 * Source scheduling/control state belongs to the caller, not these kernels.
 * Invalid MODE/VAC/PUSHLOC rejects without mutation. */
enum p66_velocity_status p66_velocity_correct_gravity(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_pushdown *,agc_word *addrwd,const agc_word bias[6]);
enum p66_velocity_status p66_velocity_add_current(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_pushdown *,agc_word *addrwd,const agc_word velocity[6]);
enum p66_velocity_status p66_velocity_store_position(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_pushdown *,agc_word *addrwd,const agc_word position[6]);
#endif
