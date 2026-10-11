/* SPDX-License-Identifier: MIT */
#ifndef DSKY_FRACTION_PAIR_H
#define DSKY_FRACTION_PAIR_H
#include "dsky_decimal_general.h"
/* Original DSPDC2NR signed unrounded two-character fractional display,
 * LM40,3266/CM40,3173. Arbitrary ordinary TP limb signs, data positions
 * DSPCOUNT14/11/9/6/4/1, ordinarycallerQ0100..7777, coherentbank40/allEB,
 * selectingSUPER100/120/140/160, legalIRQ/rawA/L/ordinaryRAM. Actual caller
 * or source clean-relay CCSHOLE frontier; producer exclusion/distinctobjects.
 * Unsupported leaves image/IO/route unchanged. */
enum p66_bias_status dsky_fraction_pair(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission,enum dsky_decimal_general_route *);
#endif
