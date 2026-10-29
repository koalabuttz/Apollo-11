/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_keyboard_irq_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<CM_KEYBOARD_IRQ_CASES;++n){cm_native_io io;uint16_t isr;cm_keyboard_irq_input(&s,&io,&isr,n);printf("%u 0",n);for(i=0;i<2048;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,isr);}return 0;}
