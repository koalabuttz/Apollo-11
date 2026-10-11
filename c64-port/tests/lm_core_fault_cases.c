/* SPDX-License-Identifier: MIT */
#include "lm_core_fault_cases.h"
#include "lm_admission_raw_cases.h"
exec_address lm_core_fault_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){
 uint8_t entry=(uint8_t)(n%5u),slot=(uint8_t)((n/5u)%8u),i;
 exec_address job=lm_admission_raw_input(s,io,(uint16_t)((n/5u)*4u+(entry%4u)));
 if(entry==4u){s->words[5]=05137;s->words[0]=agc_extend((agc_word)(060000u+(n&01777u)));io->interrupt_enabled=(uint16_t)((n/40u)&1u);}
 for(i=0;i<8u;++i){agc_word p=(agc_word)(01000u*((n+i)%31u+1u)+0110u);
  s->words[0167u+12u*i]=i<slot?(n&0100u?agc_negate(p):p):AGC_NEG_ZERO;
  s->words[0164u+12u*i]=(agc_word)(050000u+(n+i)%01777u);}
 s->words[0167u+12u*slot]=0;
 for(i=0;i<5u;++i)s->words[0400u+44u*i]=i<n%5u?0:(agc_word)(0400u+44u*i);
 return job;
}
