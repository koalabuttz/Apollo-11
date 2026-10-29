/* SPDX-License-Identifier: MIT */
#ifndef APOLLO_LAYOUT_H
#define APOLLO_LAYOUT_H
#include "fixed_data.h"
#include "fixed_data_symbols.h"
#include "overlay_runtime.h"
#define APOLLO_LAYOUT_SOURCE_FIXED_BANKS 36u
#define APOLLO_CM_LAYOUT_UNAVAILABLE_PSEUDO_FIRST 0120000u
#define APOLLO_CM_LAYOUT_UNAVAILABLE_PSEUDO_LAST 0167777u
#define APOLLO_CM_LAYOUT_UNAVAILABLE_BANK_FIRST 36u
#define APOLLO_CM_LAYOUT_UNAVAILABLE_BANK_LAST 55u

/* Raw canonical storage views, not AGC register/instruction semantics.
 * Compile-time original aliases are in fixed_data_symbols.h. A/L/Q can
 * retain full sixteen-bit representations. No separate alias RAM exists. */
uint16_t *apollo_layout_cell(apollo_overlay_state *, uint16_t physical_word);
/* Resolve an original logical erasable address under a retained EB register.
 * Direct addresses0000..1377 ignore valid EB; banked1400..1777 use EB0..7.
 * Invalid inputs preserve the destination. */
bool apollo_layout_erasable_index(uint16_t logical_word, uint16_t eb,
                                  uint16_t *physical_word);
typedef struct apollo_layout_fixed_word {
    uint16_t physical_word, value;
} apollo_layout_fixed_word;
#define APOLLO_LAYOUT_LM_FIXED_WORDS 36u
#define APOLLO_LAYOUT_CM_FIXED_WORDS 43u
extern const apollo_layout_fixed_word apollo_layout_lm_fixed[APOLLO_LAYOUT_LM_FIXED_WORDS];
extern const apollo_layout_fixed_word apollo_layout_cm_fixed[APOLLO_LAYOUT_CM_FIXED_WORDS];
extern const apollo_layout_fixed_word apollo_layout_cm_erasable_emission;
/* Original BNKSUM bugger metadata and seven CM EB-switch ECADR words only.
 * Returned original checksums describe the independent assembled rope,
 * never native overlay integrity or any translated instruction path. */
bool apollo_layout_fixed_read(enum apollo_fixed_mission, uint16_t physical_word,
                              uint16_t *value);
/* Materialize original declaration emissions explicitly: LM has none;
 * CM source line2094 has one at01356. This is declaration-data handling,
 * not evidence that original hardware/startup initializes this RAM word.
 * Allocation and alias effects exist independently in compile-time bindings.
 * Invalid mission/image preserves the whole image. */
bool apollo_layout_materialize_emissions(enum apollo_fixed_mission,
                                        apollo_overlay_state *);
#endif
