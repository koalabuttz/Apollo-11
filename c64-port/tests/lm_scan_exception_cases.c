/* SPDX-License-Identifier: MIT */
#include "lm_scan_exception_cases.h"
#include "lm_context_lifecycle_cases.h"
void lm_scan_exception_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){
 static const uint8_t slots[8]={1,2,4,5,6,7,1,0};
 uint8_t entry=(uint8_t)(n%4u),slot=slots[(n/4u)%8u],i;agc_word channel;
 lm_context_lifecycle_input(s,io,&channel,(uint16_t)((n/4u)*2u+(entry==1u)));
 if(entry==0)s->words[5]=05146;
 else if(entry==1)s->words[5]=05133;
 else if(entry==2){s->words[5]=05155;s->words[0167]=(agc_word)(020000u+(n&4u?0110u:(0401u+(n/32u)%5u*054u)));}
 else{s->words[5]=03120;s->words[4]=02000;s->words[6]=(agc_word)(02000u|(s->words[3]>>8));io->interrupt_enabled=0;
  s->words[0131]=slot?AGC_NEG_ZERO:(agc_word)((n/32u)&1u);}
 if(!slot&&entry!=3u)slot=1;
 for(i=1;i<8u;++i){agc_word p=(agc_word)(01000u*((n+i)%31u+1u)+i);
  s->words[0167u+12u*i]=slot&&i<slot?(n&040u?agc_negate(p):p):AGC_NEG_ZERO;}
 if(slot)s->words[0167u+12u*slot]=0;
 if(slot>=4u&&n&0200u)s->words[0233]=0; /* third +0 literal data route */
}
