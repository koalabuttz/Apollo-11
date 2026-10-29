/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "lm_busy_check.h"
#include "lm_busy_vectors.h"
uint16_t lm_busy_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before) {
 uint16_t f,patch,end,failures=0,number; p66_bias_io io,old;agc_word channel,oldchannel;
 if(n>=64u)return 1;
 number=lm_busy_numbers[n];lm_busy_input(s,&io,&channel,number);memcpy(before,s,sizeof *s);
 old=io;oldchannel=channel;
 if((lm_pinball_busy(s,&io,&channel))!=lm_busy_results[n])++failures;
 patch=lm_busy_offsets[n];end=lm_busy_offsets[n+1u];
 for(f=0;f<APOLLO_SHARED_WORDS+3u;++f){
  uint16_t expected=f<APOLLO_SHARED_WORDS ? before->words[f] : f==APOLLO_SHARED_WORDS ? old.superbank : f==APOLLO_SHARED_WORDS+1u?old.interrupt_enabled:oldchannel;
  uint16_t actual=f<APOLLO_SHARED_WORDS ? s->words[f] : f==APOLLO_SHARED_WORDS ? io.superbank : f==APOLLO_SHARED_WORDS+1u?io.interrupt_enabled:channel;
  if(patch<end && lm_busy_patch[patch][0]==f)expected=lm_busy_patch[patch++][1];
  if(actual!=expected)++failures;
 }
 if(patch!=end)++failures;
 return failures;
}
