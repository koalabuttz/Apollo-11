/* SPDX-License-Identifier: MIT */
#ifndef DSKY_OPERATOR_RESPONSE_H
#define DSKY_OPERATOR_RESPONSE_H
#include "dsky_response_recall.h"
/* Actual VBPROC/+1, VBTERM, VBRESEQ, PROCKEY through KILMONON,
 * RELDSP, FLASHOFF, RECALTST and genuine ENDOFJOB. Both missions;
 * any EB/selecting SUPER/IRQ/raw registers and CADRSTOR/list classes.
 * Branch-sensitive sequential wake domain; rejection preserves all
 * state. One canonical source image under restoring producer exclusion. */
enum p66_bias_status dsky_operator_response(apollo_overlay_state *,dsky_lamp_io *);
enum exec_status cm_operator_response(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channels,uint16_t *restart_light);
#endif
