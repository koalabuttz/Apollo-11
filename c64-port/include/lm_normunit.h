/* SPDX-License-Identifier: MIT */
#ifndef LM_NORMUNIT_H
#define LM_NORMUNIT_H
#include "rtb_unit.h"
/* Luminary RTB NORMUNIT, from its basic code (FB 10, Z 03726, as an RTB
 * leaves it): the NORM front (rtb_vector) and UNIT's three phases
 * (rtb_unit). RTB_OK at DANZIG (UNIT's overflow ending too); 2 at
 * POODOO1 with alarm 1302 (a zero vector), where the run leaves the
 * caller's file; RTB_OUT_OF_SCOPE when an input is outside those bodies'
 * domain. */
int lm_normunit(apollo_overlay_state *,p66_bias_io *);
/* RTB NORMUNX1 (FB 10, Z 03724): CAF ONE, then NORMUNIT +1, so the
 * shift count goes to X1 (FIXLOC+1) instead of X2. */
int lm_normunx1(apollo_overlay_state *,p66_bias_io *);
#endif
