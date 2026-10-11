/* SPDX-License-Identifier: MIT */
#include "lm_idle_dispatch_cases.h"
#include "lm_context_lifecycle_cases.h"
void lm_idle_dispatch_input(apollo_overlay_state*s,p66_bias_io*io,agc_word*channel,uint16_t n){
 uint16_t kind=(uint16_t)(n/2048u),offset=(uint16_t)((n%7u+1u)*12u),bank;
 lm_context_lifecycle_input(s,io,channel,(uint16_t)(n&16383u));
 bank=(agc_word)(((n>>3)&31u)*02000u+(n&7u)+((n&7u)*020u));
 s->words[5]=(agc_word)(kind<4u?03214u:kind<7u?03011u:03225u);
 s->words[4]=02000;s->words[6]=(uint16_t)(02000u|(s->words[3]>>8));
 s->words[067]=(agc_word)(kind==0u||kind==4u?0:kind==3u?AGC_NEG_ZERO:offset);
 s->words[0164]=02001;s->words[0165]=bank;s->words[0167]=010110;
 if(kind==1u||kind==2u||kind==5u||kind==6u){
  if(n%4u==0u)s->words[0167]=AGC_NEG_ZERO;
  else if(n%4u==1u)s->words[0167]=agc_negate(010110);
  else if(n%4u==2u)s->words[0167]=0;
 }
 s->words[0164u+offset]=(agc_word)(kind==2u||kind==6u?agc_negate((agc_word)(03400u+offset/12u)):
  02000u+offset/12u);
 s->words[0165u+offset]=bank;s->words[0167u+offset]=020110;s->words[01361]=03340;
 if(kind==7u){s->words[0]=2;if(n%4u==0u)s->words[067]=0;
  else if(n%4u==1u)s->words[067]=AGC_NEG_ZERO;}
}
