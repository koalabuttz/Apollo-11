/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "lm_finish_check.h"
#include "lm_finish_vectors.h"
uint16_t lm_finish_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before) {
 uint16_t f,patch,end,failures=0,number; p66_bias_io io,old;exec_transfer next;enum exec_status status;agc_word channel,oldchannel;
 if(n>=64u)return 1;
 number=lm_finish_numbers[n];lm_finish_input(s,&io,&channel,number);memcpy(before,s,sizeof *s);
 old=io;oldchannel=channel;
 status=lm_basic_finish(s,&io,&channel,&next);if(status!=lm_finish_results[n])++failures;
 patch=lm_finish_offsets[n];end=lm_finish_offsets[n+1u];
 for(f=0;f<APOLLO_SHARED_WORDS+3u;++f){
  uint16_t expected=f<APOLLO_SHARED_WORDS ? before->words[f] : f==APOLLO_SHARED_WORDS ? old.superbank : f==APOLLO_SHARED_WORDS+1u?old.interrupt_enabled:oldchannel;
  uint16_t actual=f<APOLLO_SHARED_WORDS ? s->words[f] : f==APOLLO_SHARED_WORDS ? io.superbank : f==APOLLO_SHARED_WORDS+1u?io.interrupt_enabled:channel;
  if(patch<end && lm_finish_patch[patch][0]==f)expected=lm_finish_patch[patch++][1];
  if(actual!=expected)++failures;
 }
 if(patch!=end)++failures;
 if(s->words[5]==03214u){if(next.address.location!=0u||next.address.bank!=0u||next.interpretive!=0u)++failures;}else if(next.address.location!=(s->words[5]==06070u?s->words[0164]:s->words[5])||next.address.bank!=(agc_word)(s->words[6]|io.superbank)||next.interpretive!=(uint8_t)(s->words[5]==06070u))++failures;
 return failures;
}
