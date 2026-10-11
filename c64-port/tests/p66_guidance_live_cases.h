/* SPDX-License-Identifier: MIT */
#ifndef P66_GUIDANCE_LIVE_CASES_H
#define P66_GUIDANCE_LIVE_CASES_H
#include "p66_native_guidance.h"
#define PGL_WORDS 166u
#define PGL_FULL_WORDS 493u
#define PGL_CASES 128u
#define PGL_STAGES 23u
#define PGL_MIXED_STAGES 25u
extern const agc_word pgl_boundaries[PGL_STAGES];
extern const agc_word pgl_mixed_boundaries[PGL_MIXED_STAGES];
void pgl_addresses(uint16_t [PGL_WORDS],uint16_t base);
void pgl_input(uint16_t [PGL_WORDS],uint16_t number);
void pgl_prepare(apollo_overlay_state *,executive *,uint16_t number);
void pgl_prepare_high(apollo_overlay_state *,uint16_t number);
void pgl_words(const apollo_overlay_state *,uint16_t [PGL_FULL_WORDS],unsigned stage);
#endif
