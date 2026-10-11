/* SPDX-License-Identifier: MIT */
#ifndef DSKY_TEST_NOUN_H
#define DSKY_TEST_NOUN_H
#include "dsky_enter_zero.h"
#include "dsky_nv_admission.h"
/* Direct original TESTNN41,2046; fresh original noun-table read and actual
 * normal/mixed selected continuation. NOUNREG0..99/eitherzero, ordinary RAM,
 * rawA/L, bank41/allEB/selectingSUPER/IRQ. DECBRNCH/VERBSAVE are retained.
 * Caller owns selected VERBFAN/MIXNOUN/GODSPALM/REQADD/INTMCTBS body.
 * Distinct objects/restoring exclusion; unsupported preserves all objects.
 * Native immutable tables are data only; no instruction decoder/poll. */
enum p66_bias_status dsky_test_noun(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission,enum dsky_enter_zero_route *);
#endif
