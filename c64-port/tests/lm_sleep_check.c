/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "lm_sleep_check.h"
#include "lm_sleep_vectors.h"
uint16_t lm_sleep_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before) {
 uint16_t f,patch,end,failures=0,number; p66_bias_io io,old;exec_address next;enum exec_status status;agc_word channel,oldchannel;
 if(n>=64u)return 1;
 number=lm_sleep_numbers[n];lm_sleep_input(s,&io,&channel,number);memcpy(before,s,sizeof *s);
 old=io;oldchannel=channel;
 status=lm_basic_sleep(s,&io,&channel,&next);if(status!=lm_sleep_results[n])++failures;if(status==EXEC_IDLE){if(next.location!=0||next.bank!=0)++failures;}else if(next.location!=s->words[5]||next.bank!=(uint16_t)(s->words[6]|io.superbank))++failures;
 patch=lm_sleep_offsets[n];end=lm_sleep_offsets[n+1u];
 for(f=0;f<APOLLO_SHARED_WORDS+3u;++f){
  uint16_t expected=f<APOLLO_SHARED_WORDS ? before->words[f] : f==APOLLO_SHARED_WORDS ? old.superbank : f==APOLLO_SHARED_WORDS+1u?old.interrupt_enabled:oldchannel;
  uint16_t actual=f<APOLLO_SHARED_WORDS ? s->words[f] : f==APOLLO_SHARED_WORDS ? io.superbank : f==APOLLO_SHARED_WORDS+1u?io.interrupt_enabled:channel;
  if(patch<end && lm_sleep_patch[patch][0]==f)expected=lm_sleep_patch[patch++][1];
  if(actual!=expected)++failures;
 }
 if(patch!=end)++failures;
 return failures;
}
