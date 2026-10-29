/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "nv_entry_check.h"
#include "nv_entry_vectors.h"
uint16_t nv_entry_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before) {
 uint16_t f,patch,end,failures=0,number; p66_bias_io io,old;
 if(n>=64u)return 1;
 number=nv_entry_numbers[n];nv_entry_input(s,&io,number);memcpy(before,s,sizeof *s);
 old=io;
 if(p66_nv_enter(s,&io)!=nv_entry_results[n])++failures;
 patch=nv_entry_offsets[n];end=nv_entry_offsets[n+1u];
 for(f=0;f<APOLLO_SHARED_WORDS+2u;++f){
  uint16_t expected=f<APOLLO_SHARED_WORDS ? before->words[f] : f==APOLLO_SHARED_WORDS ? old.superbank : old.interrupt_enabled;
  uint16_t actual=f<APOLLO_SHARED_WORDS ? s->words[f] : f==APOLLO_SHARED_WORDS ? io.superbank : io.interrupt_enabled;
  if(patch<end && nv_entry_patch[patch][0]==f)expected=nv_entry_patch[patch++][1];
  if(actual!=expected)++failures;
 }
 if(patch!=end)++failures;
 return failures;
}
