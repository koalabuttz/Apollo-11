/* SPDX-License-Identifier: MIT */
#ifndef P66_DESCENT_LIVE_CASES_H
#define P66_DESCENT_LIVE_CASES_H
#include "p66_guidance_live_cases.h"
#include "p66_bias.h"
#define PDL_EXTRA_WORDS 39u
#define PDL_WORDS (PGL_FULL_WORDS+PDL_EXTRA_WORDS+3u)
extern const uint16_t pdl_extra_addresses[PDL_EXTRA_WORDS];
void pdl_prepare(apollo_overlay_state *,executive *,p66_bias_io *,agc_word *,uint16_t);
void pdl_words(const apollo_overlay_state *,p66_bias_io,agc_word,uint16_t [PDL_WORDS],unsigned);
#endif
