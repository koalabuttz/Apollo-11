/* SPDX-License-Identifier: MIT */
#ifndef DSKY_MIXED_NOUN_H
#define DSKY_MIXED_NOUN_H
#include "dsky_enter_zero.h"
enum dsky_mixed_route {DSKY_MIXED_VERB,DSKY_MIXED_ALARM};
/* Actual MIXNOUN41,2221 in both independently qualified programs. Source
 * VERBREG0..99/-0; selected display0..6 requires packed RUTMX routines0..13.
 * All ordinary IDAD words allowed: LOW11 selects real canonical erasable
 * memory, including source CPU and editing-register reads. Includes actual
 * SFRUTMIX, RIGHT5/LEFT5, DPTEST, SETEBANK, all three component exchanges.
 * Stops VERBFAN2133 or GODSPALM2351. Caller excludes producers; unsupported
 * image/metadata/route unchanged. No instruction words are read or decoded. */
enum p66_bias_status dsky_mixed_noun(apollo_overlay_state *,p66_bias_io *,enum dsky_mixed_route *);
enum exec_status cm_mixed_noun(apollo_overlay_state *,cm_native_io *,enum dsky_mixed_route *);
#endif
