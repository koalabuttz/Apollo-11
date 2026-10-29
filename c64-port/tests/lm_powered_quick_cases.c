/* SPDX-License-Identifier: MIT */
#include "lm_powered_quick_cases.h"
#include "lm_powered_trig_cases.h"
void lm_powered_quick_input(apollo_overlay_state *m,agc_word *irq,uint16_t n){lm_powered_trig_input(m,irq,n,0);m->words[5]=03615;}
