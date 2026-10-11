/* SPDX-License-Identifier: MIT */
#include "cm_powered_interface_cases.h"
#include "cm_powered_axis_cases.h"
void cm_powered_interface_input(apollo_overlay_state *m,agc_word *irq,uint16_t n,uint8_t selector){static const agc_word entry[8]={03477,03503,03621,03624,03634,03637,03644,03646};uint16_t base=(agc_word)(0401u+054u*(n%5u));cm_powered_axis_input(m,irq,(uint32_t)n*7u);m->words[0]=n;m->words[1]=(agc_word)((uint32_t)n*37u);m->words[4]=046000;m->words[5]=06037;m->words[6]=(agc_word)(046000u|(m->words[3]>>8));m->words[0164]=entry[selector];m->words[0165]=m->words[6];m->words[0115]=040000;m->words[023]=0;m->words[067]=077777;m->words[0120]=base;m->words[0166]=base;m->words[base+052u]=(agc_word)(042000u+n%3072u);m->words[0163]=(agc_word)((uint32_t)n*17u&077777u);}
