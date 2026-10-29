/* SPDX-License-Identifier: MIT */
#ifndef P66_NATIVE_CASES_H
#define P66_NATIVE_CASES_H
#include "velocity_cases.h"
#include "p66_native.h"
#define PN_CASES 1155u
#define PN_WORDS 117u
uint16_t pn_number(uint16_t);
void pn_prepare(apollo_overlay_state *,executive *,velocity_case *,uint16_t);
void pn_words(const apollo_overlay_state *,agc_word base,uint16_t [PN_WORDS]);
#endif
