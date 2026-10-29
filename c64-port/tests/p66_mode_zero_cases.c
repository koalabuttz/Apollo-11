/* SPDX-License-Identifier: MIT
 * New +0/-0 relay blank domain, preserving prior mode-display corpus. */
#include "p66_mode_zero_cases.h"
#include "p66_mode_display_cases.h"
void p66_mode_zero_prepare(apollo_overlay_state *s,uint16_t n){static const uint16_t relays[]={0,077777,1,040000,037777,073777};static const uint16_t nouts[]={0,077777,1,037777,040000};p66_mode_display_prepare(s,n);s->words[01011]=077777;s->words[01035]=relays[n%6u];s->words[01016]=nouts[(n/6u)%5u];if(n%3u==2){s->words[01011]=(uint16_t)(n%100u);s->words[01035]=04000;}}
