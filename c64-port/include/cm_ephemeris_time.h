/* SPDX-License-Identifier: MIT */
#ifndef CM_EPHEMERIS_TIME_H
#define CM_EPHEMERIS_TIME_H
#include "ephemeris_shift.h"
#include "p66_bias.h"
/* Four original ephemerides entries through LSTIME, ending at actual
 * RES/REM/VEM NEWOPS before position or velocity body. Exclusive canonical
 * ownership, scalar MODE+0, coherent declared caller context required. */
bool cm_ephemeris_time(apollo_overlay_state *,const p66_bias_io *);
#endif
