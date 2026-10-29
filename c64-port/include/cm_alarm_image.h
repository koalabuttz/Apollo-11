/* SPDX-License-Identifier: MIT */
#ifndef CM_ALARM_IMAGE_H
#define CM_ALARM_IMAGE_H
#include <stdbool.h>
#include "cm_alarm_record.h"
/* A fixed inline alarm code is the caller's compiled operand; the flight entry
 * cm_alarm_site_call takes it from cm_alarm_sites (docs/NATIVE_TRANSFER_ABI.md). */
/* Ordinary compiled TC caller Q0100..7775, normal A for VAR/BORT, coherent
 * banks/ZERO; fixed_word is source-qualified passive data at resolved address.
 * ALARM erasable reads occur after its original ALMCADR store, including aliases.
 * Banked program words must occupy ordinary physical storage>=0100, excluding
 * CPU/editing/counter registers as inline code. BORTENT itself does not INHINT;
 * an overflow handoff preserves its incoming IRQ latch.
 * Other entries do not use fixed_word. Existing frozen record helper unchanged. */
enum exec_status cm_alarm_image(apollo_overlay_state *,cm_native_io *,enum cm_alarm_record_entry,agc_word fixed_word);
#endif
