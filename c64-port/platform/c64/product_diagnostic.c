/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "product_cases.h"
#include "product_vectors.h"
static product_case c;
static uint8_t enter(void) {
    uint8_t p; __asm__ volatile("php\nsei\npla" : "=a"(p) : : "memory"); return p;
}
static void leave(uint8_t p) {
    if((p&4u)==0) __asm__ volatile("cli" : : : "memory");
    else __asm__ volatile("sei" : : : "memory");
}
int main(void) {
    volatile uint8_t *mail=(volatile uint8_t *)0xc080;
    static const uint16_t samples[5]={0,1,15,31,63};
    unsigned failures=0; uint8_t i,token;
    mail[0]=0; puts("APOLLO NATIVE P66 DMP1 / VXSC"); failures+=pr_check(&c);
    for(i=0;i<5u;++i) {
        uint8_t p,sub=(uint8_t)(i&1u); uint16_t words[PR_WORD_COUNT],row;
        unsigned j;
        pr_prepare(&c,pr_target_number(samples[i]),sub); row=(uint16_t)(2u*samples[i]+sub);
        token=enter(); mail[8]=(uint8_t)(0x10u+i); (void)pr_apply(&c,sub);
        mail[8]=(uint8_t)(0x20u+i); leave(token);
        __asm__ volatile("php\npla" : "=a"(p) : : "memory");
        if((p&4u)!=(token&4u) || (i==0 && (p&4u)!=0)) ++failures;
        ve_words(&c,words);
        for(j=0;j<PR_WORD_COUNT;++j) if(words[j]!=product_reference_words[row][j]) ++failures;
    }
    pr_prepare(&c,0,0); token=enter();
    {
        uint8_t nested=enter(),p; (void)pr_apply(&c,0); leave(nested);
        __asm__ volatile("php\npla" : "=a"(p) : : "memory");
        if((p&4u)==0) ++failures;
    }
    leave(token);
    mail[1]=(uint8_t)failures; mail[2]=(uint8_t)(failures>>8);
    mail[3]=(uint8_t)PR_TARGET_ROWS; mail[4]=(uint8_t)(PR_TARGET_ROWS>>8);
    mail[5]=3; mail[6]=(uint8_t)sizeof(product_case); mail[7]=PR_WORD_COUNT;
    puts(failures ? "VECTOR TESTS FAILED" : "128 COMPLETE ASSEMBLY STATES PASS");
    mail[0]=0xa5; return failures;
}
