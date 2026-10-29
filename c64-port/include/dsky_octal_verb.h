/* SPDX-License-Identifier: MIT */
#ifndef DSKY_OCTAL_VERB_H
#define DSKY_OCTAL_VERB_H
#include "dsky_decimal_general.h"
enum dsky_octal_verb_route {DSKY_VERB_FORMAT,DSKY_VERB_ENTRET,DSKY_VERB_ALARM,DSKY_VERB_CHANNEL};
/* Actual DSPA/B/C/AB/ABC component/decimal tests and data staging through
 * first DSPOCTWD, genuine GODSPALM or CHANDSP. Coherent bank41/anyEB,
 * selectingSUPER/legalIRQ, ordinary RAM, matching VERBREG1..5/source PC,
 * MIXBR1/2 and stable logical noun input0100..1775 resolving>=0100.
 * Channel-only DSPA may instead hand off CHANDSP without noun reads.
 * Caller excludes producers; unsupported image/IO/route remains unchanged.
 * Row-return resumes actual2422, stops next format or saved ENTRET0136;
 * no saved instruction, recovery or scheduler body runs here. */
enum p66_bias_status dsky_octal_verb_begin(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission,enum dsky_octal_verb_route *);
enum p66_bias_status dsky_octal_verb_return(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission,enum dsky_octal_verb_route *);
/* Actual CHANDSP resolved channel read and DSPCOM1 staging. External stable
 * ordinary channel_value is consumed only for channels other than L/Q(1/2),
 * which read canonical registers; channel7 reads retained SUPER. No channel output or device service. */
enum p66_bias_status dsky_octal_verb_channel(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission,agc_word channel_value,enum dsky_octal_verb_route *);
#endif
