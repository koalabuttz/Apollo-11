/* SPDX-License-Identifier: MIT */
#include "p66_vertical_start_cases.h"
#include "p66_bias_cases.h"
void p66_vertical_start_prepare(apollo_overlay_state *s,uint16_t n) {
 static const uint16_t timers[]={037777,030000,0,1,0177,0200,037634,037635};
 uint8_t i,style=(uint8_t)((n/8u)%8u);p66_bias_prepare(s,n);
 s->words[5]=02536;s->words[026]=timers[n%8u];s->words[0734]=n%2u ? 1 : 077777;
 for(i=0;i<8;++i)s->words[01400+i]=style==0 ? WL_DEFAULT_INTERVAL : style==1 ? 1 : style==2 ? 0 : style==3 ? 077777 : (uint16_t)(077777u-(10u+i*(style-3u)));
 for(i=0;i<9;++i){s->words[01410+2u*i]=i==8 && style%2u==0 ? WL_END_LOCATION : (uint16_t)(02200+i);s->words[01411+2u*i]=(uint16_t)(012000+(i%8u));}
}
