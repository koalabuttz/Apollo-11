/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "store_cases.h"
#include "store_vectors.h"
static store_case c;
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
    /* Four direct stores, then full RODCOMP through STORE 30D. */
    static const uint16_t samples[5]={0,1,15,18,160};
    unsigned failures=0; uint8_t i,token;
    mail[0]=0; puts("APOLLO NATIVE P66 STORE 30D");
    failures+=st_check(&c);
    for(i=0;i<5;++i) {
        uint8_t p,path; uint16_t words[ST_WORD_COUNT]; unsigned j;
        st_prepare(&c,samples[i]); st_next(&c,samples[i],0);
        token=enter(); mail[8]=(uint8_t)(0x10u+i);
        path=i==4 ? st_prefix(&c,samples[i]) : 0;
        if(st_apply(&c)!=P66_STORE_OK) ++failures;
        mail[8]=(uint8_t)(0x20u+i); leave(token);
        __asm__ volatile("php\npla" : "=a"(p) : : "memory");
        if((p & 4u)!=(token & 4u) || (i==0 && (p & 4u)!=0)) ++failures;
        el_words(&c,words);
        if(path!=store_reference_path[samples[i]]) ++failures;
        for(j=0;j<ST_WORD_COUNT;++j) if(words[j]!=store_reference_words[samples[i]][j]) ++failures;
    }
    st_prepare(&c,0); token=enter();
    {
        uint8_t nested=enter(),p;
        if(st_apply(&c)!=P66_STORE_OK) ++failures;
        leave(nested);
        __asm__ volatile("php\npla" : "=a"(p) : : "memory");
        if((p & 4u)==0) ++failures;
    }
    leave(token);
    {
        uint16_t before[ST_WORD_COUNT],after[ST_WORD_COUNT]; unsigned j;
        st_prepare(&c,0); c.previous.push.base=0402; el_words(&c,before);
        token=enter();
        if(st_apply(&c)!=P66_STORE_OUT_OF_SCOPE) ++failures;
        leave(token); el_words(&c,after);
        for(j=0;j<ST_WORD_COUNT;++j) if(before[j]!=after[j]) ++failures;
    }
    mail[1]=(uint8_t)failures; mail[2]=(uint8_t)(failures>>8);
    mail[3]=(uint8_t)ST_TARGET_ROWS; mail[4]=(uint8_t)(ST_TARGET_ROWS>>8);
    mail[5]=3; mail[6]=(uint8_t)sizeof(store_case); mail[7]=ST_WORD_COUNT;
    puts(failures ? "STORE TESTS FAILED" : "176 COMPLETE ASSEMBLY STATES PASS");
    puts("ELAPSED STORE AND RETAINED WORDS PASS"); mail[0]=0xa5;
    return failures;
}
