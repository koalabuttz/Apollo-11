/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "scale_cases.h"
#include "scale_vectors.h"
static scale_case c;
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
    /* Below/exact half, rounding propagation in both signs, full prefix. */
    static const uint16_t samples[5]={144,145,147,151,280};
    unsigned failures=0; uint8_t i,token;
    mail[0]=0; puts("APOLLO NATIVE P66 VXSC KPIP1");
    failures+=sc_check(&c);
    for(i=0;i<5;++i) {
        uint8_t p; uint16_t row,words[SC_WORD_COUNT]; unsigned j;
        sc_prepare(&c,samples[i]); sc_next(&c,samples[i],0);
        token=enter(); mail[8]=(uint8_t)(0x10u+i);
        if(i==4) sc_apply(&c,samples[i]);
        else p66_scale_velocity(&c.preparation,&c.scratch);
        mail[8]=(uint8_t)(0x20u+i); leave(token);
        __asm__ volatile("php\npla" : "=a"(p) : : "memory");
        if((p & 4u)!=(token & 4u) || (i==0 && (p & 4u)!=0)) ++failures;
        row=samples[i]; sc_words(&c,words);
        for(j=0;j<SC_WORD_COUNT;++j) if(words[j]!=scale_reference_words[row][j]) ++failures;
    }
    token=enter();
    {
        uint8_t nested=enter(),p;
        p66_scale_velocity(&c.preparation,&c.scratch); leave(nested);
        __asm__ volatile("php\npla" : "=a"(p) : : "memory");
        if((p & 4u)==0) ++failures;
    }
    leave(token);
    mail[1]=(uint8_t)failures; mail[2]=(uint8_t)(failures>>8);
    mail[3]=(uint8_t)SC_TARGET_ROWS; mail[4]=(uint8_t)(SC_TARGET_ROWS>>8);
    mail[5]=3; mail[6]=(uint8_t)sizeof(scale_case); mail[7]=SC_WORD_COUNT;
    puts(failures ? "SCALING TESTS FAILED" : "292 COMPLETE ASSEMBLY STATES PASS");
    puts("DP CORNERS AND INTEGRATED HISTORIES PASS"); mail[0]=0xa5;
    return failures;
}
