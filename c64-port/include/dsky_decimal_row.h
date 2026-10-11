/* SPDX-License-Identifier: MIT */
#ifndef DSKY_DECIMAL_ROW_H
#define DSKY_DECIMAL_ROW_H
#include "dsky_nv_admission.h"
enum dsky_decimal_row_entry {DSKY_DECIMAL_ROW_CALL,DSKY_DECIMAL_ROW_RETURN};
/* CALL: actual DSPDCEND41,2566 BANKCALL through DSPDECWD40,3226 LM/3133CM.
 * RETURN: actual SWRETURN4631LM/4675CM with BUF2={2570,62000}, through next
 * DSPDCPUT2537 or actual ENTRET0136 entry (before execution of its
 * saved source return instruction). No formatter/indirect continuation is executed here.
 * DECOUNT0..2, ordinary image/raw16 A/L/Q, coherent anyEB/BB, selecting
 * SUPER100/120/140/160 and legal IRQ; caller excludes producers.
 * Unsupported image/IO unchanged. No source instruction decoder or poll. */
enum p66_bias_status dsky_decimal_row(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission,enum dsky_decimal_row_entry);
#endif
