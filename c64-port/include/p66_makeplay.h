/* SPDX-License-Identifier: MIT */
#ifndef P66_MAKEPLAY_H
#define P66_MAKEPLAY_H
#include "p66_bias.h"
#include "agc_word.h"
enum p66_makeplay_boundary {P66_MAKEPLAY_PRIORITY=0,P66_MAKEPLAY_WAKE=1,P66_MAKEPLAY_SLEEP=2,P66_MAKEPLAY_END=3,P66_MAKEPLAY_PLAY=4,P66_MAKEPLAY_OUT_OF_SCOPE=5};
/* Actual normal nonflashing P66 V06N60 MAKEPLAY, with PLAYTEM1=01474,
 * PLAYTEM3=063476 and PLAYTEM4=023400, bank10/EB7. Producer exclusion.
 * Priority entry2546 -> PRIOCHNG5146. Selection2553 -> JOBWAKE5137,
 * GOSLEEPS2754 or actual ENDOFJOB5155. Wake return2614 -> GOPLAY3070,
 * replacing a found core LOC with ENDINST5155 via original XCHTOEND.
 * Channel11 is explicit ordinary input. Source services decide lifecycle;
 * unsupported input leaves image/io/channel unchanged. No decoder or polling. */
enum p66_makeplay_boundary p66_makeplay_priority(apollo_overlay_state *,p66_bias_io *);
enum p66_makeplay_boundary p66_makeplay_select(apollo_overlay_state *,p66_bias_io *,agc_word *channel11);
enum p66_makeplay_boundary p66_makeplay_wake_return(apollo_overlay_state *,p66_bias_io *);
#endif
