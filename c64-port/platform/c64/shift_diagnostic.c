/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "shift_cases.h"
static shift_case c;
static uint8_t enter(void) {uint8_t p;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}
static void leave(uint8_t p) {
    if((p&4u)==0)__asm__ volatile("cli":::"memory");else __asm__ volatile("sei":::"memory");
}
int main(void) {
    volatile uint8_t *mail=(volatile uint8_t *)0xc080;
    static const uint16_t samples[5]={457,9417,10377,19017,26697};
    unsigned failures;uint8_t i,token;
    mail[0]=0;puts("APOLLO NATIVE P66 SCALAR / VECTOR SHIFTS");failures=sh_check(&c);
    for(i=0;i<5u;++i) {
        uint8_t p;sh_prepare(&c,samples[i]);token=enter();mail[8]=(uint8_t)(0x10u+i);
        if(!sh_apply(&c))++failures;mail[8]=(uint8_t)(0x20u+i);leave(token);
        __asm__ volatile("php\npla":"=a"(p)::"memory");
        if((p&4u)!=(token&4u)||(i==0&&(p&4u)!=0))++failures;
    }
    sh_prepare(&c,26697);token=enter();{
        uint8_t nested=enter(),p;(void)sh_apply(&c);leave(nested);
        __asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)==0)++failures;
    }leave(token);
    mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=SH_TARGET_ROWS;mail[4]=0;
    mail[5]=3;mail[6]=(uint8_t)sizeof(shift_case);mail[7]=SH_WORD_COUNT;
    puts(failures?"SHIFT TESTS FAILED":"168 COMPLETE ASSEMBLY STATES PASS");mail[0]=0xa5;return failures;
}
