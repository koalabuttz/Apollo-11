/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "runtime_bridge_check.h"
#include "runtime_bridge_vectors.h"
uint16_t runtime_bridge_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before) {
 uint16_t f,patch,end,failures=0,number; p66_bias_io io,old;
 if(n>=64u)return 1;
 number=runtime_bridge_numbers[n];runtime_bridge_input(s,&io,number);memcpy(before,s,sizeof *s);
 old=io;
 if((runtime_bridge_apply(s,&io,number))!=runtime_bridge_results[n])++failures;
 patch=runtime_bridge_offsets[n];end=runtime_bridge_offsets[n+1u];
 for(f=0;f<APOLLO_SHARED_WORDS+2u;++f){
  uint16_t expected=f<APOLLO_SHARED_WORDS ? before->words[f] : f==APOLLO_SHARED_WORDS ? old.superbank : old.interrupt_enabled;
  uint16_t actual=f<APOLLO_SHARED_WORDS ? s->words[f] : f==APOLLO_SHARED_WORDS ? io.superbank : io.interrupt_enabled;
  if(patch<end && runtime_bridge_patch[patch][0]==f)expected=runtime_bridge_patch[patch++][1];
  if(actual!=expected)++failures;
 }
 if(patch!=end)++failures;
 return failures;
}
