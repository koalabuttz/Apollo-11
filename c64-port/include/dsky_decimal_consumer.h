/* SPDX-License-Identifier: MIT */
#ifndef DSKY_DECIMAL_CONSUMER_H
#define DSKY_DECIMAL_CONSUMER_H
#include "dsky_decimal_general.h"
enum dsky_decimal_consumer_stage { DSKY_DECIMAL_LOAD, DSKY_DECIMAL_SELECT, DSKY_DECIMAL_SCALAR, DSKY_DECIMAL_DP, DSKY_DECIMAL_CALL, DSKY_DECIMAL_FORMAT, DSKY_DECIMAL_RETURN };
enum dsky_decimal_consumer_route { DSKY_DECIMAL_NEXT, DSKY_DECIMAL_ENTRET, DSKY_DECIMAL_RECOVERY };
/* Explicit compiled stages of actual DECDSP through source row selection,
 * supported scalar/DP scaling, formatter and row continuation. Original
 * physical image remains authoritative. Ordinary stable RAM, source input
 * domains of selected kernels, coherent banks/anyEB, selecting SUPER and
 * legal IRQ are required. Caller excludes producers and supplies distinct
 * route storage. External scale routines remain source-selected entries;
 * no saved ENTRET instruction, recovery or scheduler body is executed. */
enum p66_bias_status dsky_decimal_consumer(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission,enum dsky_decimal_consumer_stage,agc_word channel33,enum dsky_decimal_consumer_route *);
#endif
