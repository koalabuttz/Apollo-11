/* SPDX-License-Identifier: MIT */
#ifndef LM_FIXED_COMPAT_H
#define LM_FIXED_COMPAT_H
#include <stdbool.h>
#include "agc_word.h"
/* Passive original Luminary099-layout words for actual checksum/fixed-memory display.
 * No instruction decode, execution, dispatch or executable coverage claim.
 * Physical banks0..35 contain original words; banks36..39 are unpopulated0.
 * Define LM_FIXED_COMPAT_GROUP0..5 to link one seven-bank native data payload.
 * Caller never retains a pointer to an unloaded data overlay. */
bool lm_fixed_compat_read(uint16_t physical_word,agc_word *out);
#endif
