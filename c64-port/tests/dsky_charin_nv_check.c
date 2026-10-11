/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "dsky_charin_nv_check.h"
#include "dsky_charin_nv_vectors.h"
uint16_t dsky_charin_nv_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before) {
 uint16_t f,patch,end,failures=0,number; dsky_charin_io io,old;
 if(n>=DSKY_CHARIN_NV_TARGET_COUNT)return 1;
 number=dsky_charin_nv_numbers[n];dsky_charin_nv_prepare(s,number);memcpy(before,s,sizeof *s);
 io.control.superbank=0100;io.control.interrupt_enabled=(uint16_t)(number%2u);io.channel11=dsky_charin_nv_channel11(number);io.wrote_channel11=0;old=io;
 if(dsky_charin_nv(s,&io)!=dsky_charin_nv_results[n])++failures;
 patch=dsky_charin_nv_offsets[n];end=dsky_charin_nv_offsets[n+1u];
 for(f=0;f<APOLLO_SHARED_WORDS+4u;++f){
  uint16_t expected=f<APOLLO_SHARED_WORDS ? before->words[f] : f==APOLLO_SHARED_WORDS ? old.control.superbank : f==APOLLO_SHARED_WORDS+1u ? old.control.interrupt_enabled : f==APOLLO_SHARED_WORDS+2u ? old.channel11 : old.wrote_channel11;
  uint16_t actual=f<APOLLO_SHARED_WORDS ? s->words[f] : f==APOLLO_SHARED_WORDS ? io.control.superbank : f==APOLLO_SHARED_WORDS+1u ? io.control.interrupt_enabled : f==APOLLO_SHARED_WORDS+2u ? io.channel11 : io.wrote_channel11;
  if(patch<end && dsky_charin_nv_patch[patch][0]==f)expected=dsky_charin_nv_patch[patch++][1];
  if(actual!=expected)++failures;
 }
 if(patch!=end)++failures;
 return failures;
}
