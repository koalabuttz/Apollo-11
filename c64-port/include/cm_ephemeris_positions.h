/* SPDX-License-Identifier: MIT */
#ifndef CM_EPHEMERIS_POSITIONS_H
#define CM_EPHEMERIS_POSITIONS_H
#include "cm_ephemeris_solar.h"
/* Actual LSPOS callers; solar result remains in VAC+2, lunar result in MPAC. */
uint8_t cm_ephemeris_positions(apollo_overlay_state *,p66_bias_io *);
bool cm_ephemeris_positions_moon(apollo_overlay_state *,const p66_bias_io *);
#endif
