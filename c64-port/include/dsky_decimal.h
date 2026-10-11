/* SPDX-License-Identifier: MIT */
#ifndef DSKY_DECIMAL_H
#define DSKY_DECIMAL_H
#include "dsky_pinball.h"
/* Source DSPDECWD (rounded) or DSPDECNR (unrounded), complete sign and
 * five decimal characters through actual callerQ. DP MPAC0/1 sign-agreed;
 * MPAC+2 retained original TP low word, DSPCOUNT14/9/4 selects R1/R2/R3.
 * Physical bank40, any retained EB0..7,/SUPER100, nonzero DSPTAB entries, ordinary callerQ.
 * Canonical image authoritative, all producers excluded; bad domain no-op. */
enum p66_bias_status dsky_decimal_display(apollo_overlay_state *,p66_bias_io *,uint8_t rounded);
#endif
