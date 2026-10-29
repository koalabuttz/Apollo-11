/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "gravity_cases.h"
#include "gravity_vectors.h"
static gravity_case c;
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
    static const uint16_t samples[5]={0,2,3,6,144};
    unsigned failures=0; uint8_t i,token;
    mail[0]=0; puts("APOLLO NATIVE P66 PDVL GDT/2");
    failures+=gv_check(&c);
    for(i=0;i<5;++i) {
        uint8_t p; uint16_t words[GV_WORD_COUNT]; unsigned j; frozen_words reader;
        gv_prepare(&c,samples[i]); gv_next(&c,samples[i],0);
        token=enter(); mail[8]=(uint8_t)(0x10u+i);
        if(i==4) gv_prefix(&c,samples[i]);
        if(gv_apply(&c)!=P66_PUSH_OK) ++failures;
        mail[8]=(uint8_t)(0x20u+i); leave(token);
        __asm__ volatile("php\npla" : "=a"(p) : : "memory");
        if((p & 4u)!=(token & 4u) || (i==0 && (p & 4u)!=0)) ++failures;
        gv_words(&c,words); gravity_reference_begin(&reader,samples[i]);
        for(j=0;j<GV_WORD_COUNT;++j) {
            uint16_t expected;
            if(!frozen_words_next(&reader,&expected) || words[j]!=expected) ++failures;
        }
        if(!frozen_words_finished(&reader)) ++failures;
    }
    gv_prepare(&c,0); token=enter();
    {
        uint8_t nested=enter(),p;
        (void)gv_apply(&c); leave(nested);
        __asm__ volatile("php\npla" : "=a"(p) : : "memory");
        if((p & 4u)==0) ++failures;
    }
    leave(token);
    mail[1]=(uint8_t)failures; mail[2]=(uint8_t)(failures>>8);
    mail[3]=(uint8_t)GV_TARGET_ROWS; mail[4]=(uint8_t)(GV_TARGET_ROWS>>8);
    mail[5]=3; mail[6]=(uint8_t)sizeof(gravity_case); mail[7]=GV_WORD_COUNT;
    puts(failures ? "GRAVITY TESTS FAILED" : "156 COMPLETE ASSEMBLY STATES PASS");
    mail[0]=0xa5;
    return failures;
}
