/* SPDX-License-Identifier: MIT */
#ifndef DSKY_INPUT_ARITHMETIC_H
#define DSKY_INPUT_ARITHMETIC_H
#include "dsky_lamp_release.h"
enum dsky_input_arithmetic_route {DSKY_INPUT_ARITHMETIC_CALLER,DSKY_INPUT_ARITHMETIC_RECYCLE,DSKY_INPUT_ARITHMETIC_FALLTHROUGH};
/* Original TPSL1, TPLEFTN/LEFTNCOM, 2ROUND/+2/2RNDEND, TESTOFUF.
 * Canonical ordinary TP limbs, raw A/L, ordinary selected saved caller;
 * bounded native shift count N-1=0..14. TPSL1 fixed-fixed any retained bank
 * and legal SUPER; other entries actual bank40/selecting SUPER. TESTOFUF
 * preserves actual -zero fallthrough as source frontier, not a return.
 * Exclusion belongs to caller; unsupported image/IO/route unchanged. */
enum p66_bias_status dsky_input_arithmetic(apollo_overlay_state *,dsky_lamp_io *,enum dsky_input_arithmetic_route *);
enum exec_status cm_input_arithmetic(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channels,uint16_t *restart_light,enum dsky_input_arithmetic_route *);
#endif
