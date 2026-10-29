/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "dsky_two_digit_check.h"
#include "dsky_two_digit_vectors.h"
uint16_t dsky_two_digit_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before) {
 uint16_t f,patch,end,failures=0,number; p66_bias_io io,old;
 if(n>=DSKY_TWO_DIGIT_TARGET_COUNT)return 1;
 number=dsky_two_digit_numbers[n];dsky_two_digit_prepare(s,number);memcpy(before,s,sizeof *s);
 io.superbank=0100;io.interrupt_enabled=(uint16_t)(number%2u);old=io;
 if(dsky_two_digit(s,&io)!=dsky_two_digit_results[n])++failures;
 patch=dsky_two_digit_offsets[n];end=dsky_two_digit_offsets[n+1u];
 for(f=0;f<APOLLO_SHARED_WORDS+2u;++f){
  uint16_t expected=f<APOLLO_SHARED_WORDS ? before->words[f] : f==APOLLO_SHARED_WORDS ? old.superbank : old.interrupt_enabled;
  uint16_t actual=f<APOLLO_SHARED_WORDS ? s->words[f] : f==APOLLO_SHARED_WORDS ? io.superbank : io.interrupt_enabled;
  if(patch<end && dsky_two_digit_patch[patch][0]==f)expected=dsky_two_digit_patch[patch++][1];
  if(actual!=expected)++failures;
 }
 if(patch!=end)++failures;
 return failures;
}
