/* SPDX-License-Identifier: MIT */
#ifndef DSKY_FIVE_BLANK_H
#define DSKY_FIVE_BLANK_H
#include "dsky_octal_display.h"
enum dsky_five_blank_route {DSKY_FIVE_BLANK_CALLER,DSKY_FIVE_BLANK_CCS_HOLE};
/* Actual LM40,2535/2536/2540/CLR52525 or CM40,2436/2437/2441/CLR52426.
 * Numeric R1/R2/R3: DSPCOUNT0..14, or ordinary A0..14 for entry-1;
 * entry+2 uses INREL2..4 and saved ordinary L0100..7777. Other entries
 * ordinary Q0100..7777. Canonical words; any EB/four selectingSUPER,IRQ.
 * Selected clean+0 runs actual CCSHOLE prelude, unused zeros accepted.
 * Caller holds restoring exclusion; unsupported state/IO/route unchanged. */
enum p66_bias_status dsky_five_blank(apollo_overlay_state *,p66_bias_io *,enum dsky_five_blank_route *);
enum exec_status cm_five_blank(apollo_overlay_state *,cm_native_io *,enum dsky_five_blank_route *);
#endif
