/* SPDX-License-Identifier: MIT */
#ifndef DSKY_OCTAL_DISPLAY_H
#define DSKY_OCTAL_DISPLAY_H
#include "dsky_pinball.h"
#include "cm_native_image.h"
enum dsky_octal_route {DSKY_OCTAL_CALLER,DSKY_OCTAL_CCS_HOLE};
/* Actual DSPOCTWD/DSPOCTWO41,3363 or DSP2BIT41,3416 through ordinary
 * Q caller or genuine selected-clean+0 CCSHOLE handoff. A sign-extended
 * ordinary word, Q0100..7777, DSPCOUNT4..21 (five chars) or1..21(two).
 * Any EB, SUPER100/120/140/160, IRQ0/1; canonical ordinary words.
 * Caller restoring exclusion. Unsupported image/IO/route unchanged. */
enum p66_bias_status dsky_octal_display(apollo_overlay_state *,p66_bias_io *,enum dsky_octal_route *);
enum exec_status cm_octal_display(apollo_overlay_state *,cm_native_io *,enum dsky_octal_route *);
#endif
