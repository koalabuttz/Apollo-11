/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_keyboard_runtime_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i,j;for(n=0;n<CM_KEYBOARD_RUNTIME_CASES;++n){cm_native_io io;uint16_t isr,key,wrote=0;exec_transfer next={{0,0},0};enum exec_status status;
 cm_keyboard_runtime_input(&s,&io,&isr,&key,n);
 for(j=0;j<4;++j){uint16_t selector=j==0 ? CM_KEYBOARD_IRQ : j==1 ? CM_KEYBOARD_SELECT : j==3 ? CM_KEYBOARD_FINISH : n%4u<2u ? CM_KEYBOARD_NV : CM_KEYBOARD_NUMBER;
 status=cm_keyboard_runtime_entry(&s,&io,&isr,key,selector,&next,&wrote);printf("%u %u %u",n,j,(unsigned)status);for(i=0;i<2048;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u %u %u %u %u\n",io.superbank,io.interrupt_enabled,io.channel11,isr,wrote,next.address.location,next.address.bank,next.interpretive);
 }}return 0;}
