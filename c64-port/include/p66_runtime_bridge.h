/* SPDX-License-Identifier: MIT */
#ifndef P66_RUNTIME_BRIDGE_H
#define P66_RUNTIME_BRIDGE_H
#include "p66_throttle_bridge.h"
#include "p66_display_control.h"
/* Fixed source continuations under producer exclusion. Enter accepts actual
 * NEWOPS06070 or literal EXIT06743 with LOC03510, MODE0, CYR/EDOP0,
 * BB/BANKSET064007. Return accepts THDUMP02367 and the original saved return.
 * Display accepts the actual basic PC03531/LOC03530 after interpreted EXIT.
 * NEWJOB is any ordinary15-bit word, preserved without a basic-code poll.
 * Ends at THROTTLE+3, NEWOPS/LOC03515, or REGODSPR/ENDOFJOB respectively.
 * Unsupported boundary leaves canonical image and SUPER/IRQ unchanged. */
enum p66_bridge_status p66_runtime_throttle_enter(apollo_overlay_state *,p66_bias_io *);
enum p66_bridge_status p66_runtime_throttle_return(apollo_overlay_state *,p66_bias_io *);
enum p66_display_boundary p66_runtime_display_exit(apollo_overlay_state *,p66_bias_io *);
#endif
