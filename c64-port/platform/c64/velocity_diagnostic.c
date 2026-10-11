/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "velocity_cases.h"
#include "velocity_vectors.h"
static velocity_case c;
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
#if VELOCITY_DIAGNOSTIC_HALF==0
    static const uint16_t samples[5]={0,2,3,6,16};
#else
    static const uint16_t samples[5]={80,96,112,128,144};
#endif
    unsigned failures=0; uint8_t i,token;
    mail[0]=0; puts("APOLLO NATIVE P66 STOVL24D / R");
    failures+=vl_check(&c);
    for(i=0;i<5;++i) {
        uint8_t p; uint16_t words[VL_WORD_COUNT]; unsigned j; frozen_words reader;
        vl_prepare(&c,samples[i]); vl_next(&c,samples[i],0);
        if(i<4u && vl_integrated(samples[i])) vl_prefix(&c,samples[i]);
        token=enter(); mail[8]=(uint8_t)(0x10u+i);
        if(i==4) vl_prefix(&c,samples[i]);
        if(vl_apply(&c)!=P66_VELOCITY_OK) ++failures;
        mail[8]=(uint8_t)(0x20u+i); leave(token);
        __asm__ volatile("php\npla" : "=a"(p) : : "memory");
        if((p & 4u)!=(token & 4u) || (i==0 && (p & 4u)!=0)) ++failures;
        vl_words(&c,words); velocity_reference_begin(&reader,samples[i]-VL_FIRST_CASE);
        for(j=0;j<VL_WORD_COUNT;++j) {
            uint16_t expected;
            if(!frozen_words_next(&reader,&expected) || words[j]!=expected) ++failures;
        }
        if(!frozen_words_finished(&reader)) ++failures;
    }
    vl_prepare(&c,0); token=enter();
    {
        uint8_t nested=enter(),p;
        (void)vl_apply(&c); leave(nested);
        __asm__ volatile("php\npla" : "=a"(p) : : "memory");
        if((p & 4u)==0) ++failures;
    }
    leave(token);
    mail[1]=(uint8_t)failures; mail[2]=(uint8_t)(failures>>8);
    mail[3]=(uint8_t)VELOCITY_FROZEN_ROWS; mail[4]=(uint8_t)(VELOCITY_FROZEN_ROWS>>8);
    mail[5]=3; mail[6]=(uint8_t)sizeof(velocity_case); mail[7]=VL_WORD_COUNT;
    puts(failures ? "VELOCITY TESTS FAILED" : "COMPLETE ASSEMBLY STATE HALF PASS");
    mail[0]=0xa5;
    return failures;
}
