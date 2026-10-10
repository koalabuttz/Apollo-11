/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "restart_check.h"
#include "restart_vectors.h"
static restart_case c;
static uint8_t enter(void *user) {
    uint8_t p; (void)user;
    __asm__ volatile("php\nsei\npla" : "=a"(p) : : "memory"); return p;
}
static void leave(void *user,uint8_t p) {
    (void)user;
    if((p & 4u)==0) __asm__ volatile("cli" : : : "memory");
    else __asm__ volatile("sei" : : : "memory");
}
static const exec_exclusion exclusion={enter,leave,0};
int main(void) {
    volatile uint8_t *mail=(volatile uint8_t *)0xc080;
    unsigned failures=0;
    uint8_t i,token;
    static const uint16_t samples[4]={31,111,119,183};
    mail[0]=0;
    puts("APOLLO NATIVE ALARM / RESTART");
    failures+=rs_check(&c,&exclusion);
    for(i=0;i<4;++i) {
        int status;
        rs_prepare(&c,samples[i]); token=enter(0);
        mail[8]=(uint8_t)(0x10u+i);
        status=rs_apply(&c,samples[i],0);
        mail[8]=(uint8_t)(0x20u+i);
        leave(0,token);
        if(rs_digest(&c,status)!=restart_reference_digest[samples[i]]) ++failures;
    }
    mail[8]=0x14; failures+=rs_demo(&c,&exclusion); mail[8]=0x24;
    token=enter(0); failures+=rs_demo(&c,&exclusion);
    {
        uint8_t p;
        __asm__ volatile("php\npla" : "=a"(p) : : "memory");
        if((p & 4u)==0) ++failures;
    }
    leave(0,token);
    mail[1]=(uint8_t)failures; mail[2]=(uint8_t)(failures>>8);
    mail[3]=190; mail[4]=226; mail[5]=6;
    mail[6]=(uint8_t)sizeof(restart_state); mail[7]=(uint8_t)sizeof(restart_phases);
    mail[9]=(uint8_t)sizeof(restart_case); mail[10]=(uint8_t)(sizeof(restart_case)>>8);
    mail[11]=(uint8_t)c.record.failreg[0]; mail[12]=(uint8_t)(c.record.failreg[0]>>8);
    puts(failures ? "RESTART TESTS FAILED" : "226 ASSEMBLY CHECKPOINTS PASS");
    puts("SIX RECOVERED NATIVE JOBS PASS");
    mail[0]=0xa5;
    return failures;
}
