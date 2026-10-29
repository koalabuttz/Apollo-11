/* SPDX-License-Identifier: MIT */
#ifndef CM_MINUTES_SECONDS_H
#define CM_MINUTES_SECONDS_H
#include "cm_native_image.h"
/* Original M/SOUT42,3216 numerical and caller groups: phase0 fresh read,
 * normalize, source clamp/round/SEPSEC;1 seconds scale;2 seconds display;
 *3 middle blank;4 minutes scale;5 minutes display/source row reentry.
 * Mixed/normal ordinary DP pair, DECOUNT0..2, RXD1 DSPCOUNT14/9/4;
 * coherent bank42/SUPER100/any EB and nonzero relays. Caller owns exclusion. */
uint8_t cm_minutes_seconds_phase(apollo_overlay_state *,cm_native_io *,uint8_t);
#endif
