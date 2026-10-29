/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_keyboard_runtime_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<CM_KEYBOARD_RUNTIME_CASES;++n){cm_native_io io;uint16_t isr,key;cm_keyboard_runtime_input(&s,&io,&isr,&key,n);printf("%u",n);for(i=0;i<2048;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u 0 0 0 0\n",io.superbank,io.interrupt_enabled,io.channel11,isr);}return 0;}
