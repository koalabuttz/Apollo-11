/* SPDX-License-Identifier: MIT */
#ifndef DSKY_TWO_INTEGER_H
#define DSKY_TWO_INTEGER_H
#include "dsky_sign_output.h"
enum dsky_two_integer_stage {DSKY_TWO_INTEGER_BLANK,DSKY_TWO_INTEGER_SIGN,DSKY_TWO_INTEGER_FIRST,DSKY_TWO_INTEGER_SECOND,DSKY_TWO_INTEGER_RETURN};
/* Original 2INTOUT, two positive SP integers 0..99 (or -0). Explicit stages retain
 * actual source call/return boundaries. Bank40/allEB/four selectingSUPER,
 * legalIRQ, row0..2, ordinaryRAM, nonzero relay words and restoring producer
 * exclusion. SECOND reads fresh ordinary pair0100..3776/no bank crossing;
 * its low word must be0..99. Unsupported stage leaves all objects unchanged.
 * RETURN stops at actual DSPDCEND+2, before row continuation execution. */
enum p66_bias_status dsky_two_integer(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission,enum dsky_two_integer_stage);
#endif
