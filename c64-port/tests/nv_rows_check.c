/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "nv_rows_check.h"
#include "nv_rows_vectors.h"
uint16_t nv_rows_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before) {
 uint16_t f,patch,end,failures=0,number; p66_bias_io io,old;
 if(n>=64u)return 1;
 number=nv_rows_numbers[n];nv_rows_input(s,&io,number);memcpy(before,s,sizeof *s);
 old=io;
 if(p66_nv_data_prepare(s,&io)!=nv_rows_results[n])++failures;
 patch=nv_rows_offsets[n];end=nv_rows_offsets[n+1u];
 for(f=0;f<APOLLO_SHARED_WORDS+2u;++f){
  uint16_t expected=f<APOLLO_SHARED_WORDS ? before->words[f] : f==APOLLO_SHARED_WORDS ? old.superbank : old.interrupt_enabled;
  uint16_t actual=f<APOLLO_SHARED_WORDS ? s->words[f] : f==APOLLO_SHARED_WORDS ? io.superbank : io.interrupt_enabled;
  if(patch<end && nv_rows_patch[patch][0]==f)expected=nv_rows_patch[patch++][1];
  if(actual!=expected)++failures;
 }
 if(patch!=end)++failures;
 return failures;
}
