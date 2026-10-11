/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "dsky_key_irq_check.h"
#include "dsky_key_irq_vectors.h"
uint16_t dsky_key_irq_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before) {
 uint16_t f,patch,end,failures=0,number; dsky_key_irq_io io,old;
 if(n>=DSKY_KEY_IRQ_TARGET_COUNT)return 1;
 number=dsky_key_irq_numbers[n];dsky_key_irq_prepare(s,number);memcpy(before,s,sizeof *s);
 io.control.superbank=(uint16_t)((number/512u)%8u*020u);io.control.interrupt_enabled=0;io.in_interrupt=1;old=io;
 if(dsky_key_irq(s,&io,dsky_key_irq_channel15(number))!=dsky_key_irq_results[n])++failures;
 patch=dsky_key_irq_offsets[n];end=dsky_key_irq_offsets[n+1u];
 for(f=0;f<APOLLO_SHARED_WORDS+3u;++f){
  uint16_t expected=f<APOLLO_SHARED_WORDS ? before->words[f] : f==APOLLO_SHARED_WORDS ? old.control.superbank : f==APOLLO_SHARED_WORDS+1u ? old.control.interrupt_enabled : old.in_interrupt;
  uint16_t actual=f<APOLLO_SHARED_WORDS ? s->words[f] : f==APOLLO_SHARED_WORDS ? io.control.superbank : f==APOLLO_SHARED_WORDS+1u ? io.control.interrupt_enabled : io.in_interrupt;
  if(patch<end && dsky_key_irq_patch[patch][0]==f)expected=dsky_key_irq_patch[patch++][1];
  if(actual!=expected)++failures;
 }
 if(patch!=end)++failures;
 return failures;
}
