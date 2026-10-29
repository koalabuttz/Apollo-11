/* SPDX-License-Identifier: MIT */
#ifndef DSKY_DECIMAL_RAW_H
#define DSKY_DECIMAL_RAW_H
#include "dsky_decimal_general.h"
/* Original formatter for arbitrary ordinary DP/TP limb signs, including
 * source SF bypass/scalar results. Original bounded TRACE1 table index reads
 * compiled mission-specific RELTAB neighbour DATA; no opcode execution.
 * Caller/relay/IO domains and routes otherwise match the frozen general
 * formatter. Unsupported preserves complete image/IO/route. */
enum p66_bias_status dsky_decimal_raw(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission,uint8_t rounded,enum dsky_decimal_general_route *);
#endif
