/* SPDX-License-Identifier: MIT */
#ifndef CM_DISPLAY_REST_H
#define CM_DISPLAY_REST_H
#include "cm_native_image.h"
/* Actual bank10 NVDSP blocked return03343, REST03467 or RESTSLEP03473
 * through ENDOFJOB05217 or NVSUBUSY04456 entry. COPINDEX0..2; ordinary
 * GENMASK/flags/CADR; coherent banks, explicit legal IO. Includes actual
 * UPENT2 alias effects (ITEMP1, L, IRQ and selected flag word). No selected
 * lifecycle body/NEWJOB poll or source instruction decoding. Caller owns
 * exclusion; unsupported input leaves complete image/IO unchanged. */
enum exec_status cm_display_rest(apollo_overlay_state *,cm_native_io *);
#endif
