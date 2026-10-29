/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "sum_cases.h"
static vector_case c;
static uint8_t enter(void) {uint8_t p;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}
static void leave(uint8_t p) {
    if((p&4u)==0)__asm__ volatile("cli":::"memory");else __asm__ volatile("sei":::"memory");
}
int main(void) {
    volatile uint8_t *mail=(volatile uint8_t *)0xc080;
    static const uint16_t samples[5]={258,256,260,260,4497};
    static const uint8_t operations[5]={0,1,2,3,3};
    unsigned failures;uint8_t i,token;
    mail[0]=0;puts("APOLLO NATIVE SCALAR SUMS / VECTOR REVERSE");failures=su_check(&c);
    for(i=0;i<5u;++i) {
        uint8_t p;su_prepare(&c,samples[i],operations[i]);token=enter();mail[8]=(uint8_t)(0x10u+i);
        if(su_apply(&c,operations[i])==P66_SUM_OUT_OF_SCOPE)++failures;
        mail[8]=(uint8_t)(0x20u+i);leave(token);
        __asm__ volatile("php\npla":"=a"(p)::"memory");
        if((p&4u)!=(token&4u)||(i==0&&(p&4u)!=0))++failures;
    }
    su_prepare(&c,4497,3);token=enter();{
        uint8_t nested=enter(),p;(void)su_apply(&c,3);leave(nested);
        __asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)==0)++failures;
    }leave(token);
    mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=0;mail[4]=1;
    mail[5]=3;mail[6]=(uint8_t)sizeof(vector_case);mail[7]=SU_WORD_COUNT;
    puts(failures?"SUM TESTS FAILED":"256 COMPLETE ASSEMBLY STATES PASS");mail[0]=0xa5;return failures;
}
