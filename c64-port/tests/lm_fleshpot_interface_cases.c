/* SPDX-License-Identifier: MIT */
#include "lm_fleshpot_interface_cases.h"
#include "lm_powered_interface_cases.h"
void lm_fleshpot_interface_input(apollo_overlay_state *m,agc_word *irq,uint16_t n){lm_powered_interface_input(m,irq,n,0);m->words[4]=020000;m->words[6]=(agc_word)(020000u|(m->words[3]>>8));m->words[0165]=m->words[6];m->words[0115]=0;m->words[0164]=02030;m->words[067]=0;}
