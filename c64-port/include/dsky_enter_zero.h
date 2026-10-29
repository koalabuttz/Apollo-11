/* SPDX-License-Identifier: MIT */
#ifndef DSKY_ENTER_ZERO_H
#define DSKY_ENTER_ZERO_H
#include "noun_reader.h"
#include "cm_native_image.h"
#include "p66_bias.h"
enum dsky_enter_zero_route {DSKY_ZERO_VERB,DSKY_ZERO_MIXED,DSKY_ZERO_ALARM,DSKY_ZERO_REQUEST_ADDRESS,DSKY_ZERO_MACHINE_ADDRESS};
/* Actual ENTPAS041,2035, any retained EB, SUPER100/120/140/160.
 * VERBREG0..99 or -0; NOUNREG0..99 or -0 only when actual verb<28
 * selects a noun lookup. Composes immutable original noun_reader then
 * source normal/mixed admission, including SETNCADR and -0 augmentation.
 * Stops actual VERBFAN/MIXNOUN/GODSPALM/REQADD/INTMCTBS+2.
 * Image sole authority, caller excludes producers; unsupported unchanged. */
enum p66_bias_status dsky_enter_zero(apollo_overlay_state *,p66_bias_io *,enum dsky_enter_zero_route *);
enum exec_status cm_enter_zero(apollo_overlay_state *,cm_native_io *,enum dsky_enter_zero_route *);
#endif
