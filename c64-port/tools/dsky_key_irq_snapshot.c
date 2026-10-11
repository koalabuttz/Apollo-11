/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_key_irq_cases.h"
int main(void){uint16_t n,i;static apollo_overlay_state s;for(n=0;n<DSKY_KEY_IRQ_CASE_COUNT;++n){dsky_key_irq_io io={{(uint16_t)((n/512u)%8u*020u),0},1};dsky_key_irq_prepare(&s,n);printf("%u %u",n,(unsigned)dsky_key_irq(&s,&io,dsky_key_irq_channel15(n)));for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.control.superbank,io.control.interrupt_enabled,io.in_interrupt);}return 0;}
