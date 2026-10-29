/* SPDX-License-Identifier: MIT */
#ifndef CM_EPHEMERIS_VELOCITY_H
#define CM_EPHEMERIS_VELOCITY_H
#include "cm_ephemeris_time.h"
/* Compiled LUNVEL entry through original RVQ, ordinary source VAC and
 * documented caller return. No instruction decoder or asynchronous producer. */
bool cm_ephemeris_velocity(apollo_overlay_state *,const p66_bias_io *);
#endif
