/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "nv_words_check.h"
#include "nv_words_vectors.h"
uint16_t nv_words_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before) {
 uint16_t f,patch,end,failures=0,number; p66_bias_io io,old;
 if(n>=64u)return 1;
 number=nv_words_numbers[n];nv_words_input(s,&io,(uint16_t)(number/2u));if(number%2u)(void)p66_nv_verb(s,&io);memcpy(before,s,sizeof *s);
 old=io;
 if((number%2u?p66_nv_noun(s,&io):p66_nv_verb(s,&io))!=nv_words_results[n])++failures;
 patch=nv_words_offsets[n];end=nv_words_offsets[n+1u];
 for(f=0;f<APOLLO_SHARED_WORDS+2u;++f){
  uint16_t expected=f<APOLLO_SHARED_WORDS ? before->words[f] : f==APOLLO_SHARED_WORDS ? old.superbank : old.interrupt_enabled;
  uint16_t actual=f<APOLLO_SHARED_WORDS ? s->words[f] : f==APOLLO_SHARED_WORDS ? io.superbank : io.interrupt_enabled;
  if(patch<end && nv_words_patch[patch][0]==f)expected=nv_words_patch[patch++][1];
  if(actual!=expected)++failures;
 }
 if(patch!=end)++failures;
 return failures;
}
