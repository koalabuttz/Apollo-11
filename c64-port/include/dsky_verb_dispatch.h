/* SPDX-License-Identifier: MIT */
#ifndef DSKY_VERB_DISPATCH_H
#define DSKY_VERB_DISPATCH_H
#include "dsky_enter_zero.h"
enum dsky_verb_route {DSKY_VERB_DIRECT,DSKY_VERB_EXTENDED_RELEASE};
/* Actual VERBFAN41,2133→VBFANDIR/VERBTAB/BANKJUMP for verb0..39
 * (including -0 as zero), or actual RELDSP entry for verb40..99.
 * Bank41, any EB, SUPER100/120/140/160, ordinary canonical words.
 * The compile-time mission-specific CADR data selects real source address;
 * no source instruction decode/fetch or native callback is performed here.
 * Caller excludes producers. Unsupported input preserves image/IO/route. */
enum p66_bias_status dsky_verb_dispatch(apollo_overlay_state *,p66_bias_io *,enum dsky_verb_route *);
enum exec_status cm_verb_dispatch(apollo_overlay_state *,cm_native_io *,enum dsky_verb_route *);
#endif
