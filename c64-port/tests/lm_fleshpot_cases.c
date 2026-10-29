/* SPDX-License-Identifier: MIT */
#include "lm_fleshpot_cases.h"
#include "lm_powered_axis_cases.h"
void lm_fleshpot_input(apollo_overlay_state *m,agc_word *irq,uint16_t n){static const agc_word offset[4]={0,040,0100,0200};lm_powered_axis_input(m,irq,n);m->words[4]=020000;m->words[6]=(agc_word)(020000u|(m->words[3]>>8));m->words[5]=02037;m->words[2]=04631;m->words[0]=(agc_word)(0400u*(1u+(n%7u))+offset[(n/7u)%4u]);if(n<16u)m->words[0]=02664;m->words[067]=0;}
