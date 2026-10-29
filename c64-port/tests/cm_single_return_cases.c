/* SPDX-License-Identifier: MIT */
#include "cm_single_return_cases.h"
void cm_single_return_input(apollo_overlay_state *m,agc_word *irq,uint16_t n){uint8_t selector=(uint8_t)(n/3072u);cm_trig_input(m,irq,(uint16_t)((uint32_t)n*37u),selector);m->words[2]=(agc_word)(02000u+n%3072u);m->words[4]=(agc_word)((n%32u)<<10);m->words[6]=(agc_word)(m->words[4]|(m->words[3]>>8));}
