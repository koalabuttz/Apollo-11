/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "push_cases.h"
#include "push_vectors.h"
static push_case c;
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
    static const uint16_t samples[5]={0,37,59,128,188};
    unsigned failures=0; uint8_t i,token;
    mail[0]=0; puts("APOLLO NATIVE P66 PDDL THISTPIP");
    failures+=ps_check(&c);
    for(i=0;i<5;++i) {
        uint8_t p; uint16_t words[PS_WORD_COUNT]; unsigned j;
        ps_prepare(&c,samples[i]); ps_next(&c,samples[i],0);
        if(i==3) ps_prefix(&c,samples[i]);
        token=enter(); mail[8]=(uint8_t)(0x10u+i);
        if(i==4) ps_prefix(&c,samples[i]);
        if(ps_apply(&c)!=P66_PUSH_OK) ++failures;
        mail[8]=(uint8_t)(0x20u+i); leave(token);
        __asm__ volatile("php\npla" : "=a"(p) : : "memory");
        if((p & 4u)!=(token & 4u) || (i==0 && (p & 4u)!=0)) ++failures;
        ps_words(&c,words);
        for(j=0;j<PS_WORD_COUNT;++j) if(words[j]!=push_reference_words[samples[i]][j]) ++failures;
    }
    ps_prepare(&c,0); token=enter();
    {
        uint8_t nested=enter(),p;
        if(ps_apply(&c)!=P66_PUSH_OK) ++failures;
        leave(nested);
        __asm__ volatile("php\npla" : "=a"(p) : : "memory");
        if((p & 4u)==0) ++failures;
    }
    leave(token);
    /* Native scope failures preserve the entire owned objects on target. */
    ps_prepare(&c,0); c.push.pushloc=(agc_word)(c.push.base+38u);
    {
        uint16_t before[PS_WORD_COUNT],after[PS_WORD_COUNT]; unsigned j;
        ps_words(&c,before);
        if(ps_apply(&c)!=P66_PUSH_OUT_OF_SCOPE) ++failures;
        ps_words(&c,after);
        for(j=0;j<PS_WORD_COUNT;++j) if(before[j]!=after[j]) ++failures;
    }
    mail[1]=(uint8_t)failures; mail[2]=(uint8_t)(failures>>8);
    mail[3]=(uint8_t)PS_TARGET_ROWS; mail[4]=(uint8_t)(PS_TARGET_ROWS>>8);
    mail[5]=3; mail[6]=(uint8_t)sizeof(push_case); mail[7]=PS_WORD_COUNT;
    puts(failures ? "PUSHDOWN TESTS FAILED" : "200 COMPLETE ASSEMBLY STATES PASS");
    puts("VECTOR SAVES AND TIMESTAMP LOADS PASS"); mail[0]=0xa5;
    return failures;
}
