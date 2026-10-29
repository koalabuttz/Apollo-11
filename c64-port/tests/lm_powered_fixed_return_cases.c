/* SPDX-License-Identifier: MIT */
#include "lm_powered_fixed_return_cases.h"
#include "lm_powered_interface_cases.h"
void lm_powered_fixed_return_input(apollo_overlay_state *m,agc_word *irq,uint16_t n,uint8_t origin){static const agc_word loc[4]={03542,03546,03660,02034};lm_powered_interface_input(m,irq,n,0);m->words[067]=0;m->words[0164]=loc[origin];m->words[5]=origin==2u?06061u:06070u;if(origin==3u){m->words[4]=020000;m->words[6]=(agc_word)(020000u|(m->words[3]>>8));m->words[0165]=m->words[6];m->words[0115]=0;}m->words[023]=origin==2u?0161u:0u;m->words[m->words[0120]+052u]=n;}
