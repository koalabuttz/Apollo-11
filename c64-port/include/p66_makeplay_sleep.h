/* SPDX-License-Identifier: MIT */
#ifndef P66_MAKEPLAY_SLEEP_H
#define P66_MAKEPLAY_SLEEP_H
#include "p66_makeplay.h"
enum p66_sleep_boundary {P66_SLEEP_WAKE=0,P66_SLEEP_EXECUTIVE=1,P66_SLEEP_PLAY=2,P66_SLEEP_END=3,P66_SLEEP_PINBALL=4,P66_SLEEP_OUT_OF_SCOPE=5};
/* Fixed normal P66 MAKEPLAY sleep continuations, canonical bank10/EB7 and
 * original V06N60 parameters. GOSLEEPS2754->JOBWAKE5137; wake caller2770
 * ->JOBSLEEP5133 via actual XCHTOEND; real PLAYJUM1 wake2615->GOPLAY3070.
 * Busy NVSUB caller3121->NVSUBUSY4442 or ENDOFJOB5155 via source REST.
 * The actual wake/sleep/admission services remain separate scheduling edges.
 * Caller owns exclusion; unsupported image/io unchanged. */
enum p66_sleep_boundary p66_makeplay_sleep_enter(apollo_overlay_state *,p66_bias_io *);
enum p66_sleep_boundary p66_makeplay_sleep_return(apollo_overlay_state *,p66_bias_io *);
enum p66_sleep_boundary p66_makeplay_sleep_resume(apollo_overlay_state *,p66_bias_io *);
enum p66_sleep_boundary p66_makeplay_busy(apollo_overlay_state *,p66_bias_io *);
#endif
