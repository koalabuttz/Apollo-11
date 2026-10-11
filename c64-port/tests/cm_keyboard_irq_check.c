/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_keyboard_irq_check.h"
#include "cm_keyboard_irq_vectors.h"
uint16_t cm_keyboard_irq_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before){uint16_t f,patch,end,failures=0,number,isr,old_isr;cm_native_io io,old;
 if(n>=CM_KEYBOARD_IRQ_TARGET_COUNT)return 1;
 number=cm_keyboard_irq_numbers[n];cm_keyboard_irq_input(s,&io,&isr,number);memcpy(before,s,sizeof *s);old=io;old_isr=isr;
 if(cm_keyboard_irq_run(s,&io,&isr,number)!=cm_keyboard_irq_results[n])++failures;
 patch=cm_keyboard_irq_offsets[n];end=cm_keyboard_irq_offsets[n+1u];
 for(f=0;f<APOLLO_SHARED_WORDS+3u;++f){uint16_t expected=f<APOLLO_SHARED_WORDS ? before->words[f] : f==APOLLO_SHARED_WORDS ? old.superbank : f==APOLLO_SHARED_WORDS+1u ? old.interrupt_enabled : old_isr;
  uint16_t actual=f<APOLLO_SHARED_WORDS ? s->words[f] : f==APOLLO_SHARED_WORDS ? io.superbank : f==APOLLO_SHARED_WORDS+1u ? io.interrupt_enabled : isr;
  if(patch<end && cm_keyboard_irq_patch[patch][0]==f)expected=cm_keyboard_irq_patch[patch++][1];
  if(actual!=expected)++failures;
 }
 if(patch!=end)++failures;
 return failures;}
