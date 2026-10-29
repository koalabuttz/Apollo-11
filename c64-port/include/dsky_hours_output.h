/* SPDX-License-Identifier: MIT */
#ifndef DSKY_HOURS_OUTPUT_H
#define DSKY_HOURS_OUTPUT_H
#include "dsky_decimal_raw.h"
enum dsky_hours_stage {DSKY_HOURS_FRESH,DSKY_HOURS_SECONDS,DSKY_HOURS_SECONDS_DISPLAY,DSKY_HOURS_MINUTES,DSKY_HOURS_MINUTES_DISPLAY,DSKY_HOURS_HOURS,DSKY_HOURS_HOURS_DISPLAY,DSKY_HOURS_RETURN};
/* Actual HMSOUT and helper arithmetic/callers. Explicit stages retain the
 * original bank42 source boundaries. Fresh ordinary pair1000..3776/no bank
 * crossing, normal/mixed noun row0..2; coherent bank42/allEB/four selecting
 * SUPER/legalIRQ/rawA/L/Q/ordinaryRAM. Display stages require nonzero relay
 * words. Caller holds exclusion; unsupported complete image/IO unchanged.
 * Final stage stops before the saved ENTRET instruction, not its execution. */
enum p66_bias_status dsky_hours_output(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission,enum dsky_hours_stage);
#endif
