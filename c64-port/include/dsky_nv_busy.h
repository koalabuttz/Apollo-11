/* SPDX-License-Identifier: MIT */
#ifndef DSKY_NV_BUSY_H
#define DSKY_NV_BUSY_H
#include "dsky_nv_admission.h"
enum dsky_nv_busy_entry {DSKY_NV_ENDIDLE,DSKY_NV_BUSY,DSKY_NV_PREBUSY,DSKY_NV_WAIT,DSKY_NV_BUSY_INNER};
enum dsky_nv_busy_route {DSKY_NV_SLEEP_ENTRY,DSKY_NV_ABORT_ENTRY,DSKY_NV_AVAILABLE_ENTRY};
/* Original ENDIDLE/NVSUBUSY/PRENVBSY/NVSBWAIT/NVSUBSY1 through actual
 * JOBSLEEP, POODOO or NVSBCOM entry. IRQ/channel11 explicit and canonical
 * image authoritative. Source source-list -zero falls into inline data and
 * is excluded; CADRSTOR -zero retains the actual ISLIST fallthrough.
 * No selected service body execution/NEWJOB poll. Caller excludes producers.
 * Unsupported contexts preserve image/IO/channel/route. */
enum p66_bias_status dsky_nv_busy(apollo_overlay_state *,p66_bias_io *,agc_word *,enum dsky_nv_mission,enum dsky_nv_busy_entry,enum dsky_nv_busy_route *);
#endif
