/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "projection_cases.h"
#include "projection_vectors.h"
unsigned pj_check(projection_case *);unsigned pj_seed(projection_case *,uint16_t);unsigned pj_compare(projection_case *,uint16_t,unsigned);
static projection_case c;
static uint8_t enter(void){uint8_t p;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}
static void leave(uint8_t p){if(!(p&4u))__asm__ volatile("cli":::"memory");else __asm__ volatile("sei":::"memory");}
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;unsigned failures;uint8_t phase,token,p;mail[0]=0;puts("APOLLO NATIVE P66 PROJECTION");failures=pj_check(&c);failures+=pj_seed(&c,0);
for(phase=0;phase<3u;++phase){unsigned status;token=enter();mail[8]=(uint8_t)(0x10u+phase);status=pj_apply(&c,phase);mail[8]=(uint8_t)(0x20u+phase);leave(token);__asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)!=(token&4u)||(!phase&&(p&4u)))++failures;failures+=pj_compare(&c,phase,status);}
failures+=pj_seed(&c,PJ_REFERENCE_GROUPS-1u);token=enter();mail[8]=0x13;for(phase=0;phase<3u;++phase)if(pj_apply(&c,phase))++failures;mail[8]=0x23;leave(token);failures+=pj_compare(&c,PROJECTION_FROZEN_ROWS-1u,0);
failures+=pj_seed(&c,0);token=enter();{uint8_t nested=enter();for(phase=0;phase<3u;++phase)(void)pj_apply(&c,phase);leave(nested);__asm__ volatile("php\npla":"=a"(p)::"memory");if(!(p&4u))++failures;}leave(token);
mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=PROJECTION_FROZEN_ROWS;mail[4]=0;mail[5]=3;mail[6]=(uint8_t)sizeof(projection_case);mail[7]=PJ_WORDS;puts(failures?"PROJECTION TESTS FAILED":"36 COMPLETE ORIGINAL STATES PASS");mail[0]=0xa5;return failures;}
