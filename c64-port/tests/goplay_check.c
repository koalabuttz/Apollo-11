/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "goplay_check.h"
#include "goplay_vectors.h"
uint16_t goplay_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before) {
 uint16_t f,patch,end,failures=0,number; p66_bias_io io,old;agc_word channel,oldchannel;
 if(n>=64u)return 1;
 number=goplay_numbers[n];goplay_input(s,&io,&channel,(uint16_t)(number/2u),(uint8_t)(number%2u));memcpy(before,s,sizeof *s);
 old=io;oldchannel=channel;
 if((number%2u?p66_goplay_finish(s,&io,&channel):p66_goplay_enter(s,&io))!=goplay_results[n])++failures;
 patch=goplay_offsets[n];end=goplay_offsets[n+1u];
 for(f=0;f<APOLLO_SHARED_WORDS+3u;++f){
  uint16_t expected=f<APOLLO_SHARED_WORDS ? before->words[f] : f==APOLLO_SHARED_WORDS ? old.superbank : f==APOLLO_SHARED_WORDS+1u?old.interrupt_enabled:oldchannel;
  uint16_t actual=f<APOLLO_SHARED_WORDS ? s->words[f] : f==APOLLO_SHARED_WORDS ? io.superbank : f==APOLLO_SHARED_WORDS+1u?io.interrupt_enabled:channel;
  if(patch<end && goplay_patch[patch][0]==f)expected=goplay_patch[patch++][1];
  if(actual!=expected)++failures;
 }
 if(patch!=end)++failures;
 return failures;
}
