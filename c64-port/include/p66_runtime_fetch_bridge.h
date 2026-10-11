/* SPDX-License-Identifier: MIT */
#ifndef P66_RUNTIME_FETCH_BRIDGE_H
#define P66_RUNTIME_FETCH_BRIDGE_H
#include "p66_runtime_bridge.h"
/* Original fixed EXIT fetch after the last DANZIG job poll. Actual entry
 * PC06067/LOC03507/CYR020006, or NEWOPS06070/LOC03510 with ordinary CYR.
 * MODE/EDOP0, BB/BANKSET064007; ordinary NEWJOB preserved without a poll.
 * Ends at THROTTLE+3 entry02221, with exact fixed fetch/register effects.
 * Unsupported image/io unchanged. The caller owns producer exclusion. */
enum p66_bridge_status p66_runtime_throttle_fetch(apollo_overlay_state *,p66_bias_io *);
#endif
