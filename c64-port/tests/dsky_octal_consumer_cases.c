/* SPDX-License-Identifier: MIT */
#include "dsky_octal_consumer_cases.h"
#include "dsky_octal_verb_cases.h"
void dsky_octal_consumer_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n,enum dsky_nv_mission *mission)
{
 uint8_t entry;uint16_t t=(uint16_t)(n%2048u),i;static const agc_word pc[5]={02365,02373,02400,02360,02353};
 dsky_octal_verb_input(s,io,(uint16_t)((n<2048u?0u:8192u)+t*3u),mission,&entry);
 s->words[5]=pc[s->words[01001]-1u];
 for(i=0;i<11u;++i)s->words[01023u+i]=t%31u==13u?0u:073777u;
 s->words[0115]=s->words[0122]=0;
 if(t%37u==0u){s->words[01001]=1;s->words[5]=02365;s->words[0140]=1;s->words[0146]=077776;s->words[0147]&=057777u;}
}
