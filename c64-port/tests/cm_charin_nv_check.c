/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_charin_nv_check.h"
#include "cm_charin_nv_vectors.h"
uint16_t cm_charin_nv_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before) {
 uint16_t f,patch,end,failures=0,number; cm_native_io io,old;uint16_t wrote;
 if(n>=CM_CHARIN_NV_TARGET_COUNT)return 1;
 number=cm_charin_nv_numbers[n];cm_charin_nv_input(s,&io,&wrote,number);memcpy(before,s,sizeof *s);
 old=io;
 if(cm_charin_nv(s,&io,&wrote)!=cm_charin_nv_results[n])++failures;
 patch=cm_charin_nv_offsets[n];end=cm_charin_nv_offsets[n+1u];
 for(f=0;f<APOLLO_SHARED_WORDS+4u;++f){
  uint16_t expected=f<APOLLO_SHARED_WORDS ? before->words[f] : f==APOLLO_SHARED_WORDS ? old.superbank : f==APOLLO_SHARED_WORDS+1u ? old.interrupt_enabled : f==APOLLO_SHARED_WORDS+2u ? old.channel11 : 0;
  uint16_t actual=f<APOLLO_SHARED_WORDS ? s->words[f] : f==APOLLO_SHARED_WORDS ? io.superbank : f==APOLLO_SHARED_WORDS+1u ? io.interrupt_enabled : f==APOLLO_SHARED_WORDS+2u ? io.channel11 : wrote;
  if(patch<end && cm_charin_nv_patch[patch][0]==f)expected=cm_charin_nv_patch[patch++][1];
  if(actual!=expected)++failures;
 }
 if(patch!=end)++failures;
 return failures;
}
