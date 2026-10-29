/* SPDX-License-Identifier: MIT */
#ifndef DSKY_PINBRANCH_H
#define DSKY_PINBRANCH_H
#include "dsky_blank.h"
enum dsky_pinbranch_route {DSKY_PINBRANCH_ERASE,DSKY_PINBRANCH_MARK,DSKY_PINBRANCH_PRIORITY,DSKY_PINBRANCH_NORMAL};
/* Actual PINBRNCH LM10,3050 / CM10,3272, any aligned EB and legal
 * SUPER/IRQ. Saves MARK2PAC to MPAC2 and performs actual source RELINT,
 * CCS condition and UPFLAG normal branch, including flag helper debris.
 * Stops at ERASER/MARKPLAY/KEEPPRIO/PLAYJUM1, never completes their bodies.
 * Unsupported input preserves image, IO and route. Caller excludes producers. */
enum p66_bias_status dsky_pinbranch(apollo_overlay_state *,p66_bias_io *,enum dsky_pinbranch_route *);
/* Executes those selected continuations through actual GOPLAY or NVSUB.
 * MARKPLAY includes source flag changes and conditional MARKNV complement;
 * normal/priority store their real COPINDEX. ERASER sets -3 and calls NVSUB. */
enum p66_bias_status dsky_pinbranch_resume(apollo_overlay_state *,p66_bias_io *,enum dsky_pinbranch_route *);
enum exec_status cm_pinbranch_resume(apollo_overlay_state *,cm_native_io *,enum dsky_pinbranch_route *);
enum exec_status cm_pinbranch(apollo_overlay_state *,cm_native_io *,enum dsky_pinbranch_route *);
#endif
