/* SPDX-License-Identifier: MIT */
#include "cm_nbsm_cases.h"
#include "cm_powered_interface_cases.h"
void cm_nbsm_input(apollo_overlay_state *m,agc_word *irq,uint16_t n){static const agc_word pointers[16]={00766,00700,01000,01370,01400,01770,02000,02370,02400,02770,03000,03370,03400,03504,03676,03770};agc_word base;uint8_t i;cm_powered_interface_input(m,irq,n,5);base=m->words[0120];m->words[base+052u]=(agc_word)((((uint32_t)n%30u)+2u)*02000u+(n&01777u));if(n==0u)m->words[base+052u]=036052;if(n==1u)m->words[base+052u]=047126;m->words[0164]=03606;m->words[067]=0;m->words[base+050u]=pointers[n%16u];for(i=0;i<6u;++i)m->words[m->words[base+050u]+i]=(agc_word)(((uint32_t)n*(2u*i+1u)+43u*i)&077777u);}
