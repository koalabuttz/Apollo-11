/* SPDX-License-Identifier: MIT */
#ifndef DSKY_OCTAL_CONSUMER_H
#define DSKY_OCTAL_CONSUMER_H
#include "dsky_octal_verb.h"
#include "dsky_octal_display.h"
enum dsky_octal_consumer_stage {DSKY_OCTAL_BEGIN,DSKY_OCTAL_CHANNEL,DSKY_OCTAL_FORMAT,DSKY_OCTAL_RETURN};
enum dsky_octal_consumer_route {DSKY_OCTAL_NEXT,DSKY_OCTAL_ENTRET,DSKY_OCTAL_ALARM,DSKY_OCTAL_RECOVERY};
/* Explicit compiled stages for actual original DSPA/B/C/AB/ABC row loop.
 * Source domain inherited from verb/formatter entries; no source opcode
 * decoder, saved ENTRET instruction, alarm body or Executive execution. */
enum p66_bias_status dsky_octal_consumer(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission,enum dsky_octal_consumer_stage,agc_word channel_value,enum dsky_octal_consumer_route *);
#endif
