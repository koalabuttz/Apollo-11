/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "lm_priority_check.h"
#include "lm_priority_vectors.h"
uint16_t lm_priority_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before) {
 uint16_t f,patch,end,failures=0,number; p66_bias_io io,old;exec_address next;
 if(n>=64u)return 1;
 number=lm_priority_numbers[n];lm_priority_input(s,&io,number);memcpy(before,s,sizeof *s);
 old=io;
 if(lm_basic_priority_change(s,&io,&next)!=lm_priority_results[n])++failures;
 patch=lm_priority_offsets[n];end=lm_priority_offsets[n+1u];
 for(f=0;f<APOLLO_SHARED_WORDS+2u;++f){
  uint16_t expected=f<APOLLO_SHARED_WORDS ? before->words[f] : f==APOLLO_SHARED_WORDS ? old.superbank : old.interrupt_enabled;
  uint16_t actual=f<APOLLO_SHARED_WORDS ? s->words[f] : f==APOLLO_SHARED_WORDS ? io.superbank : io.interrupt_enabled;
  if(patch<end && lm_priority_patch[patch][0]==f)expected=lm_priority_patch[patch++][1];
  if(actual!=expected)++failures;
 }
 if(patch!=end)++failures;
 if(next.location!=s->words[5]||next.bank!=(uint16_t)(s->words[6]|io.superbank))++failures;
 return failures;
}
