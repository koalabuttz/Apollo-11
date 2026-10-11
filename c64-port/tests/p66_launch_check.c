/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "p66_launch_check.h"
#include "p66_launch_vectors.h"
uint16_t p66_launch_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before) {
 uint16_t f,patch,end,failures=0,number; p66_bias_io io,old;
 if(n>=P66_LAUNCH_TARGET_COUNT)return 1;
 number=p66_launch_numbers[n];p66_launch_prepare(s,number);memcpy(before,s,sizeof *s);
 io.superbank=(uint16_t)((number/5u)%4u*020u);io.interrupt_enabled=(uint16_t)(number%2u);old=io;
 if(p66_launch(s,&io)!=p66_launch_results[n])++failures;
 patch=p66_launch_offsets[n];end=p66_launch_offsets[n+1u];
 for(f=0;f<APOLLO_SHARED_WORDS+2u;++f){
  uint16_t expected=f<APOLLO_SHARED_WORDS ? before->words[f] : f==APOLLO_SHARED_WORDS ? old.superbank : old.interrupt_enabled;
  uint16_t actual=f<APOLLO_SHARED_WORDS ? s->words[f] : f==APOLLO_SHARED_WORDS ? io.superbank : io.interrupt_enabled;
  if(patch<end && p66_launch_patch[patch][0]==f)expected=p66_launch_patch[patch++][1];
  if(actual!=expected)++failures;
 }
 if(patch!=end)++failures;
 return failures;
}
