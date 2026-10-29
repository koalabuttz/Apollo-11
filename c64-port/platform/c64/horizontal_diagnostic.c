/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "horizontal_cases.h"
#include "horizontal_vectors.h"
unsigned hh_check(horizontal_case *);unsigned hh_seed(horizontal_case *,uint16_t);unsigned hh_compare(horizontal_case *,uint16_t,unsigned);
static horizontal_case c;
static uint8_t enter(void){uint8_t p;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}
static void leave(uint8_t p){if(!(p&4u))__asm__ volatile("cli":::"memory");else __asm__ volatile("sei":::"memory");}
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;unsigned failures;uint8_t phase,token,p;mail[0]=0;puts("APOLLO NATIVE P66 HORIZONTAL");failures=hh_check(&c);failures+=hh_seed(&c,0);
for(phase=0;phase<8u;++phase){unsigned status;token=enter();mail[8]=(uint8_t)(0x10u+phase);status=hh_apply(&c,phase);mail[8]=(uint8_t)(0x20u+phase);leave(token);__asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)!=(token&4u)||(!phase&&(p&4u)))++failures;failures+=hh_compare(&c,phase,status);}
failures+=hh_seed(&c,HH_REFERENCE_GROUPS-1u);token=enter();mail[8]=0x18;for(phase=0;phase<8u;++phase){unsigned status=hh_apply(&c,phase);if(status==P66_HORIZONTAL_OUT_OF_SCOPE||status==P66_HORIZONTAL_ALARM_1302)++failures;}mail[8]=0x28;leave(token);failures+=hh_compare(&c,HORIZONTAL_FROZEN_ROWS-1u,0);
failures+=hh_seed(&c,0);token=enter();{uint8_t nested=enter();for(phase=0;phase<8u;++phase)(void)hh_apply(&c,phase);leave(nested);__asm__ volatile("php\npla":"=a"(p)::"memory");if(!(p&4u))++failures;}leave(token);
mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=HORIZONTAL_FROZEN_ROWS;mail[4]=0;mail[5]=3;mail[6]=(uint8_t)sizeof(horizontal_case);mail[7]=HH_WORDS;puts(failures?"HORIZONTAL TESTS FAILED":"64 COMPLETE ORIGINAL STATES PASS");mail[0]=0xa5;return failures;}
