/* SPDX-License-Identifier: MIT */
#ifndef KEYRUPT_UPRUPT_CASES_H
#define KEYRUPT_UPRUPT_CASES_H
#include "keyrupt_uprupt.h"
#include "p66_bias.h"
#include "cm_native_image.h"
/* keyrupt-uprupt cases: 256 states at each of the six entries of
 * lm_keyrupt_routines (mission 0) and cm_keyrupt_routines (mission 1),
 * entered at the original address and run for one routine;
 * tools/keyrupt_uprupt_reference.c runs the same entry in the original
 * program until it first reaches the continuation the native run left in Z.
 * Uplink words are a third triple-redundant codes, a sixth of them ERROR
 * RESET, the rest arbitrary. */
#define KU_PER_ENTRY 256u
#define KU_ENTRIES 6u
#define KU_CASES 1536u
#define KU_RETURNED 0u
#define KU_REFUSED 2u
typedef struct {p66_bias_io lm;cm_native_io cm;agc_hardware hw;} ku_io;
agc_word ku_entry(unsigned mission,uint16_t n);
void ku_input(unsigned mission,apollo_overlay_state *,ku_io *,uint16_t n);
/* Runs case n natively (one routine); returns the outcome kind (3: not
 * entered). */
unsigned ku_run(unsigned mission,apollo_overlay_state *,ku_io *,uint16_t n);
uint32_t ku_digest(unsigned mission,apollo_overlay_state *,uint16_t n);
#endif
