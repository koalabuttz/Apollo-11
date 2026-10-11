/* SPDX-License-Identifier: MIT; source input construction only. */
#include "standard_lead_wake_cases.h"
void standard_lead_wake_input(apollo_overlay_state *s,enum standard_lead_mission mission,uint16_t n){
 uint16_t i,count=mission==STANDARD_LEAD_LM?8u:7u,slot=(uint16_t)(n%count);agc_word wake=(agc_word)((n*13u)&077777u);
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)(((uint32_t)i*29u+n*251u+0x1234u)&077777u);
 s->words[0]=agc_extend(wake);s->words[1]=(agc_word)((n*977u)&0177777u);s->words[2]=04753;s->words[3]=(agc_word)((n%8u)*0400u);s->words[4]=010000;s->words[5]=mission==STANDARD_LEAD_LM?02631:02606;s->words[6]=(agc_word)(010000u|(n%8u));s->words[7]=0;
 s->words[072]=(agc_word)(04000u+n%256u);s->words[0167]=(agc_word)(020000u+(n&0777u));s->words[067]=n%2u?AGC_NEG_ZERO:0;
 for(i=1;i<count;++i){s->words[0164u+i*12u]=(agc_word)((wake+i+17u)&077777u);s->words[0165u+i*12u]=(agc_word)((n+i)&0167u);s->words[0167u+i*12u]=i%3u==0u?AGC_NEG_ZERO:agc_negate((agc_word)(01000u+i*17u));}
 if(slot){s->words[0164u+slot*12u]=wake;s->words[0167u+slot*12u]=agc_negate(033110);}
 if(n<128u){s->words[0]=n&1u?0177777u:0;for(i=1;i<count;++i)s->words[0167u+i*12u]=AGC_NEG_ZERO;if(slot){s->words[0164u+slot*12u]=n&2u?AGC_NEG_ZERO:0;s->words[0167u+slot*12u]=agc_negate(033110);}}
}
