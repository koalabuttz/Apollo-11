/* SPDX-License-Identifier: MIT */
#ifndef DSKY_TWO_DIGIT_H
#define DSKY_TWO_DIGIT_H
#include "dsky_pinball.h"
/* Actual DSPDECVN through its ordinary caller. A=0..99; DSPCOUNT odd1..21
 * identifies two adjacent display characters. Original MPAC+2 is retained
 * on entry; source SHORTMP consumes it. Bank40, retained EB0..7, SUPER100.
 * Canonical image authoritative and producers excluded; unsupported no-op. */
enum p66_bias_status dsky_two_digit(apollo_overlay_state *,p66_bias_io *);
#endif
