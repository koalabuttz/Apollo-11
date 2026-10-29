/* SPDX-License-Identifier: MIT */
#include "cm_powered_fixed_return_cases.h"
#include "cm_powered_interface_cases.h"
void cm_powered_fixed_return_input(apollo_overlay_state *m,agc_word *irq,uint16_t n,uint8_t origin){static const agc_word loc[4]={03502,03506,03633,03770};cm_powered_interface_input(m,irq,n,0);m->words[067]=0;m->words[0164]=loc[origin];m->words[5]=origin<2u?06037u:06030u;m->words[023]=origin==2u?0161u:0u;m->words[m->words[0120]+052u]=n;}
