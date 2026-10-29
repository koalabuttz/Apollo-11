/* SPDX-License-Identifier: MIT */
#ifndef DSKY_DECIMAL_SELECT_H
#define DSKY_DECIMAL_SELECT_H
#include "dsky_nv_admission.h"
/* Original DSPDCPUT41,2537 through actual SFOUTABR-selected scale entry.
 * Canonical ordinary RAM, DECOUNT0..2, MIXBR1/2, routine0..13, source
 * factor index within the mission SFOUTAB. Both immutable tables must be
 * linked. Coherent bank41/anyEB, selecting SUPER100/120/140/160 and legal IRQ.
 * Returns source routine index; selected scale body remains a continuation.
 * Caller excludes producers. Unsupported contexts preserve image/IO/route.
 * No fixed instruction decoder, invented scheduler poll or scale execution. */
enum p66_bias_status dsky_decimal_select(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission,agc_word *route);
#endif
