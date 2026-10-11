/* SPDX-License-Identifier: MIT */
#ifndef DSKY_PINBALL_H
#define DSKY_PINBALL_H
#include "p66_bias.h"
enum dsky_pinball_entry {DSKY_DSPIN,DSKY_11DSPIN,DSKY_2BLANK};
/* Native original DSPIN/11DSPIN/2BLANK return to actual ordinary callerQ.
 * Physical bank40, any retained EB0..7, SUPER100. CharacterCOUNT0..21 optionallyBIT14,
 * CODE0..31; signentry A0..10 CODE0/BIT11; blankDSPCOUNT0..21.
 * Original clean+0 CCS-hole domain excluded for DSPIN paths. Canonical
 * image, caller excludes producers. Unsupported inputs leave all unchanged. */
enum p66_bias_status dsky_pinball_write(apollo_overlay_state *,p66_bias_io *,enum dsky_pinball_entry);
#endif
