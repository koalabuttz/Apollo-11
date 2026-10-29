/* SPDX-License-Identifier: MIT */
#ifndef CM_SCALE_H
#define CM_SCALE_H
#include "cm_native_image.h"
#include "dsky_scale.h"
/* Actual Comanche DP1OUTSF40,2614 / DP2OUTSF40,2621 / DP3OUTSF40,2623,
 * fresh READLO/TPAGREE/DMP/rescale through DSPDCEND41,2566.
 * Canonical source data domains as dsky_scale_output, retained EB0..7 and
 * SUPER100. Actual CM return-address stores are preserved; no native opcode
 * lookup or callback. Caller owns source-image exclusion. */
enum exec_status cm_scale_output(apollo_overlay_state *,cm_native_io *,enum dsky_scale_entry);
#endif
