/* SPDX-License-Identifier: MIT */
#include "cm_powered_quick_cases.h"
#include "cm_powered_trig_cases.h"
void cm_powered_quick_input(apollo_overlay_state *m,agc_word *irq,uint16_t n){cm_powered_trig_input(m,irq,n,0);m->words[5]=03555;}
