/* SPDX-License-Identifier: MIT */
#ifndef DSKY_MINUTES_OUTPUT_H
#define DSKY_MINUTES_OUTPUT_H
#include "dsky_decimal_general.h"
enum dsky_minutes_stage {DSKY_MINUTES_FRESH,DSKY_MINUTES_SECONDS,DSKY_MINUTES_SECONDS_DISPLAY,DSKY_MINUTES_BLANK,DSKY_MINUTES_MINUTES,DSKY_MINUTES_MINUTES_DISPLAY};
/* Actual M/SOUT, clamp/round/SEPSEC, seconds/minutes scaling and signed
 * two-character displays, middle blank, source DSPDCEND+2 reentry. Normal
 * or mixed fresh ordinary pair1000..3776/no bankcross; row0..2, RXD1 count
 *14/9/4; coherentbank42/allEB/four selectingSUPER/legalIRQ/rawA/L/Q and
 * ordinaryRAM/nonzero relays. Caller holds exclusion, distinctimage/IO.
 * Unsupported unchanged. No scheduler check, deviceclock or decoder. */
enum p66_bias_status dsky_minutes_output(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission,enum dsky_minutes_stage);
#endif
