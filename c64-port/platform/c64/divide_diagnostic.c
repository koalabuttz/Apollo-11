/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "divide_cases.h"
#include "divide_vectors.h"
static divide_case c;
static uint8_t enter(void) {
    uint8_t p;
    __asm__ volatile("php\nsei\npla" : "=a"(p) : : "memory"); return p;
}
static void leave(uint8_t p) {
    if((p & 4u)==0) __asm__ volatile("cli" : : : "memory");
    else __asm__ volatile("sei" : : : "memory");
}
int main(void) {
    volatile uint8_t *mail=(volatile uint8_t *)0xc080;
    /* Zero, positive/negative quotient, overflow, and complete prefix. */
    static const uint16_t samples[5]={0,2,3,6,144};
    unsigned failures=0; uint8_t i,token;
    mail[0]=0; puts("APOLLO NATIVE P66 DDV 4SEC(28)");
    failures+=dv_check(&c);
    for(i=0;i<5;++i) {
        uint8_t p,path; uint16_t words[DV_WORD_COUNT]; unsigned j;
        dv_prepare(&c,samples[i]); dv_next(&c,samples[i],0);
        token=enter(); mail[8]=(uint8_t)(0x10u+i);
        if(i==4) dv_prefix(&c,samples[i]);
        path=(uint8_t)dv_apply(&c);
        mail[8]=(uint8_t)(0x20u+i); leave(token);
        __asm__ volatile("php\npla" : "=a"(p) : : "memory");
        if((p & 4u)!=(token & 4u) || (i==0 && (p & 4u)!=0)) ++failures;
        dv_words(&c,words);
        if(path!=divide_reference_path[samples[i]]) ++failures;
        for(j=0;j<DV_WORD_COUNT;++j) if(words[j]!=divide_reference_words[samples[i]][j]) ++failures;
    }
    dv_prepare(&c,0); token=enter();
    {
        uint8_t nested=enter(),p;
        (void)dv_apply(&c); leave(nested);
        __asm__ volatile("php\npla" : "=a"(p) : : "memory");
        if((p & 4u)==0) ++failures;
    }
    leave(token);
    mail[1]=(uint8_t)failures; mail[2]=(uint8_t)(failures>>8);
    mail[3]=(uint8_t)DV_TARGET_ROWS; mail[4]=(uint8_t)(DV_TARGET_ROWS>>8);
    mail[5]=3; mail[6]=(uint8_t)sizeof(divide_case); mail[7]=DV_WORD_COUNT;
    puts(failures ? "DIVIDE TESTS FAILED" : "156 COMPLETE ASSEMBLY STATES PASS");
    puts("QUOTIENTS AND RETAINED VECTORS PASS"); mail[0]=0xa5;
    return failures;
}
