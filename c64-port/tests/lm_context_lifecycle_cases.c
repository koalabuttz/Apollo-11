/* SPDX-License-Identifier: MIT */
#include "lm_context_lifecycle_cases.h"
#include "lm_priority_cases.h"
#include "lm_sleep_cases.h"
void lm_context_lifecycle_input(apollo_overlay_state*s,p66_bias_io*io,agc_word*channel,uint16_t n){
 uint8_t i;uint16_t k=(uint16_t)(n/2u);
 if(n&1u)lm_sleep_input(s,io,channel,k);
 else{lm_priority_input(s,io,k);*channel=(agc_word)(((uint32_t)n*977u)&AGC_MASK);
  /* Literal source arithmetic also accepts ordinary priority low bits. */
  if(n>=16384u){s->words[0]=(uint16_t)((s->words[0]&037000u)|((n*17u)&0777u));
   if(n%16u==0u)s->words[0]=0;
   if(n%64u==0u)for(i=1;i<8u;++i)s->words[0167u+12u*i]=AGC_NEG_ZERO;
   if(s->words[0]+(s->words[0167]&0777u)>=AGC_SIGN)s->words[0]=037000u;}}
 for(i=1;i<8u;++i)if((n+i)&1u)s->words[0164u+12u*i]=agc_negate((agc_word)(03300u+i));
 /* Explicit zero/tie candidates coexist with interpreted and basic slots. */
 if(n>=24576u){for(i=1;i<8u;++i)s->words[0167u+12u*i]=AGC_NEG_ZERO;
  i=(uint8_t)(n%7u+1u);s->words[0167u+12u*i]=(uint16_t)(n%8u+1u);
  if(!(n&1u)){s->words[0167]=010001;s->words[0]=0;}
 }
}
