/* SPDX-License-Identifier: MIT */
#ifndef DSKY_NV_UPDATE_GENERAL_H
#define DSKY_NV_UPDATE_GENERAL_H
#include "dsky_nv_update.h"
enum dsky_nv_update_route {DSKY_NV_UPDATED,DSKY_NV_UPDATE_CCSHOLE};
/* Source update fronts through actual fixed caller or actual CCSHOLE entry.
 * A (STORE entries) is sign-agreed0..99 or -zero; UPDATE reads that ordinary
 * register value. Bank41, selecting SUPER100/120/140/160, coherent EB/BB,
 * ordinary RAM, fixed callerQ02000..7777; caller excludes producers.
 * Canonical image authoritative; both original noun tables available.
 * Clean +0 relay takes the real source hole, not a fabricated blank.
 * Unsupported contexts preserve image/IO/route; no new scheduler poll. */
enum p66_bias_status dsky_nv_update_general(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission,enum dsky_nv_update_entry,enum dsky_nv_update_route *);
#endif
