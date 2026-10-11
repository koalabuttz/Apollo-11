/* SPDX-License-Identifier: MIT */
#ifndef CM_ABORT_LIFECYCLE_H
#define CM_ABORT_LIFECYCLE_H
#include <stdbool.h>
#include "cm_abort_snapshot.h"
#include "cm_abort_recovery.h"
#include "cm_abort_whimper.h"
#include "cm_enema_image.h"
/* Explicit original group selectors0..4 snapshot,5..11 POODOO recovery,
 *12 WHIMPER and13 ENEMA. Source-selected branches skip inapplicable groups;
 * the returned canonical PC is the actual next source entry. No selected
 * application is fabricated and no C continuation survives an overlay swap. */
bool cm_abort_lifecycle_step(apollo_overlay_state *,cm_native_io *,uint8_t group,agc_word source_alarm,uint8_t *in_isr,uint16_t *source_result);
#endif
