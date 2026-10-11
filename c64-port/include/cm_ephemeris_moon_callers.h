/* SPDX-License-Identifier: MIT */
#ifndef CM_EPHEMERIS_MOON_CALLERS_H
#define CM_EPHEMERIS_MOON_CALLERS_H
#include "cm_ephemeris_time.h"
/* Original LUNPOS through source lunar Horner and return to all three actual source callers. */
bool cm_ephemeris_moon_callers(apollo_overlay_state *,const p66_bias_io *);
#endif
