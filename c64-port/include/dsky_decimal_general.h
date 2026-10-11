/* SPDX-License-Identifier: MIT */
#ifndef DSKY_DECIMAL_GENERAL_H
#define DSKY_DECIMAL_GENERAL_H
#include "dsky_nv_admission.h"
enum dsky_decimal_general_route {DSKY_DECIMAL_FINISHED,DSKY_DECIMAL_CCSHOLE};
/* Original DSPDECWD/NR, actual LM3226/3262 orCM3133/3167 entries, through
 * ordinary callerQ or genuine CCSHOLE. Sign-agreed DP MPAC0/1; ordinary
 * third limb and relays including clean+0; DSPCOUNT14/9/4; fixed bank40,
 * coherent anyEB, SUPER100/120/140/160, legal IRQ, callerQ0100..7777.
 * Canonical image alone owns RAM; raw incoming A/L accepted. Caller excludes
 * producers and supplies a distinct route output. Unsupported image/IO/route
 * unchanged. No instruction decoder, invented poll or CCSHOLE body execution. */
enum p66_bias_status dsky_decimal_general(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission,uint8_t rounded,enum dsky_decimal_general_route *);
#endif
