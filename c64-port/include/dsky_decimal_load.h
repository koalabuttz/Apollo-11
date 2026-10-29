/* SPDX-License-Identifier: MIT */
#ifndef DSKY_DECIMAL_LOAD_H
#define DSKY_DECIMAL_LOAD_H
#include "dsky_nv_admission.h"
/* Original DECDSP41,2523 through actual DSPDCPUT entry2537. MIXBR1/2,
 * source component field0..2, logical NOUNADD0100..1775, ordinary memory,
 * coherent retained EB/BB, selecting SUPER100/120/140/160 and legal IRQ.
 * Reads actual canonical inputs in descending source order, preserving alias
 * effects; fixed-memory/channel inputs outside this declared domain.
 * Caller excludes producers. Unsupported image/IO unchanged. No decoder,
 * new scheduler poll or selected scale/formatter body execution. */
enum p66_bias_status dsky_decimal_load(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission);
#endif
