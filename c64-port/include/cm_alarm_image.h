/* SPDX-License-Identifier: MIT */
#ifndef CM_ALARM_IMAGE_H
#define CM_ALARM_IMAGE_H
#include <stdbool.h>
#include "cm_alarm_record.h"
/* Resolve an actual original ALARM inline fixed data read before any loader
 * swap; false means an erasable inline word (read by cm_alarm_image) or invalid
 * context. Fixed words are passive data and never decoded or dispatched. */
bool cm_alarm_fixed_address(const apollo_overlay_state *,const cm_native_io *,uint16_t *physical_word);
/* Ordinary compiled TC caller Q0100..7775, normal A for VAR/BORT, coherent
 * banks/ZERO; fixed_word is source-qualified passive data at resolved address.
 * ALARM erasable reads occur after its original ALMCADR store, including aliases.
 * Banked program words must occupy ordinary physical storage>=0100, excluding
 * CPU/editing/counter registers as inline code. BORTENT itself does not INHINT;
 * an overflow handoff preserves its incoming IRQ latch.
 * Other entries do not use fixed_word. Existing frozen record helper unchanged. */
enum exec_status cm_alarm_image(apollo_overlay_state *,cm_native_io *,enum cm_alarm_record_entry,agc_word fixed_word);
#endif
