/* SPDX-License-Identifier: MIT */
#ifndef DSKY_RESPONSE_RECALL_H
#define DSKY_RESPONSE_RECALL_H
#include "dsky_lamp_release.h"
/* Original RECALTST/RECAL1/2/3/DOPROC/DOTERM, both missions. All ordinary
 * CADRSTOR/LOADSTAT classes, direct RECAL2 raw A, source aligned LOCCTR or
 * -1 (unmatched wake), all EB/selecting SUPER/IRQ/raw registers. Selected
 * wakes retain the frozen canonical wake domain; unused scheduler words
 * remain opaque. Completes at actual ENDOFJOB, no extra scheduler poll.
 * Canonical image under caller producer exclusion; rejection is atomic. */
enum p66_bias_status dsky_response_recall(apollo_overlay_state *,dsky_lamp_io *);
enum exec_status cm_response_recall(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channels,uint16_t *restart_light);
#endif
