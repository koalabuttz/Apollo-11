/* SPDX-License-Identifier: MIT */
#include "lm_powered_axis_cases.h"
#include "lm_powered_trig_cases.h"
static agc_word next(uint32_t *seed){*seed=*seed*UINT32_C(1664525)+UINT32_C(1013904223);return (agc_word)((*seed>>8)&077777u);}
void lm_powered_axis_input(apollo_overlay_state *m,agc_word *irq,uint32_t n){static const agc_word edge[8]={0,077777,1,077776,020000,057777,037777,040000};uint32_t seed=UINT32_C(0x6547c12)+n;uint8_t i;lm_powered_trig_input(m,irq,(uint16_t)(n&077777u),1);m->words[5]=03675;m->words[0]=(n&1u)?0177774u:3u;m->words[0163]=(agc_word)(n>>1);for(i=0;i<6u;++i){m->words[0122u+i]=next(&seed);m->words[0736u+i]=next(&seed);m->words[0744u+i]=next(&seed);if(n<8192u){m->words[0122u+i]=edge[(n>>(2u*(i%5u)))&7u];m->words[0736u+i]=edge[(n>>(3u*(i%4u)))&7u];m->words[0744u+i]=edge[(n>>(2u*(i%3u)))&7u];}else if(n<16384u){m->words[0736u+i]=0;m->words[0744u+i]=(i&1u)?0u:020000u;}else if(n<24576u){m->words[0736u+i]=(i&1u)?0u:020000u;m->words[0744u+i]=0;}}}
