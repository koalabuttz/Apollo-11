/* SPDX-License-Identifier: MIT */
#ifndef CM_EPHEMERIS_MOON_H
#define CM_EPHEMERIS_MOON_H
#include "cm_ephemeris_time.h"
/* Original LUNPOS through source lunar Horner and return to Sband caller. */
bool cm_ephemeris_moon(apollo_overlay_state *,const p66_bias_io *);
#endif
