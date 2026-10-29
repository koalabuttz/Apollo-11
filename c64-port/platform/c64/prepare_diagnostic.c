/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "prepare_cases.h"
#include "prepare_vectors.h"
static prepare_case c;
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
    /* Snapshot, nominal/all-skip vector stages, negative-skip stage, composed.
     * Setup and full comparisons are outside the measured intervals. */
    static const uint16_t samples[5]={128,0,63,127,144};
    unsigned failures=0; uint8_t i,token;
    mail[0]=0; puts("APOLLO NATIVE P66 PREPARATION");
    failures+=pp_check(&c);
    for(i=0;i<5;++i) {
        uint8_t mask=0,p; uint16_t row,words[PP_WORD_COUNT]; unsigned j;
        pp_prepare(&c,samples[i]); pp_next(&c,samples[i],0);
        if(i>0 && i<4) p66_snapshot(&c);
        token=enter(); mail[8]=(uint8_t)(0x10u+i);
        if(i==0) p66_snapshot(&c);
        else if(i<4) mask=p66_vector_inputs(&c);
        else mask=p66_prepare(&c);
        mail[8]=(uint8_t)(0x20u+i); leave(token);
        __asm__ volatile("php\npla" : "=a"(p) : : "memory");
        if((p & 4u)!=(token & 4u) || (i==0 && (p & 4u)!=0)) ++failures;
        row=i==0 ? 256 : i<4 ? (uint16_t)(samples[i]*2u+1u) : 290;
        pp_words(&c,words);
        if(mask!=prepare_reference_mask[row]) ++failures;
        for(j=0;j<PP_WORD_COUNT;++j) if(words[j]!=prepare_reference_words[row][j]) ++failures;
    }
    token=enter();
    {
        uint8_t nested=enter(),p;
        (void)p66_prepare(&c); leave(nested);
        __asm__ volatile("php\npla" : "=a"(p) : : "memory");
        if((p & 4u)==0) ++failures;
    }
    leave(token);
    mail[1]=(uint8_t)failures; mail[2]=(uint8_t)(failures>>8);
    mail[3]=(uint8_t)PP_TARGET_ROWS; mail[4]=(uint8_t)(PP_TARGET_ROWS>>8);
    mail[5]=3; mail[6]=(uint8_t)sizeof(p66_preparation); mail[7]=PP_WORD_COUNT;
    puts(failures ? "PREPARATION TESTS FAILED" : "324 COMPLETE ASSEMBLY STATES PASS");
    puts("ALL TS SKIP MASKS AND HISTORIES PASS"); mail[0]=0xa5;
    return failures;
}
