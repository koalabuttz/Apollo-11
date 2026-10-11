/* SPDX-License-Identifier: MIT */
#include "native_source_finish_check.h"
static apollo_overlay_state image,expected;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;uint16_t failures=native_source_finish_guards();uint8_t n,irq;mail[0]=0;for(n=0;n<64u;++n)for(irq=0;irq<2u;++irq){uint8_t p;if(irq)__asm__ volatile("sei":::"memory");else __asm__ volatile("cli":::"memory");failures=(uint16_t)(failures+native_source_finish_check(n,irq,&image,&expected));__asm__ volatile("php\npla":"=a"(p)::"memory");if(((p>>2)&1u)!=irq)++failures;}
 __asm__ volatile("cli":::"memory");mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=128;mail[4]=7;mail[0]=0xa5;return failures!=0;}
