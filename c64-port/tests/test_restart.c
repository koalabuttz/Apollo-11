/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "restart_check.h"
static restart_case c,before;
typedef struct { unsigned depth,enters,leaves; uint8_t permitted; } guard;
static uint8_t enter(void *user) {
    guard *g=user; uint8_t old=g->permitted;
    assert(g->depth==0); ++g->depth; ++g->enters; g->permitted=0; return old;
}
static void leave(void *user,uint8_t old) {
    guard *g=user; assert(g->depth==1); --g->depth; ++g->leaves; g->permitted=old;
}
int main(void) {
    guard g={0,0,0,1}; exec_exclusion x={enter,leave,&g};
    uint8_t i;
    assert(rs_check(&c,0)==0); assert(rs_check(&c,&x)==0);
    assert(rs_demo(&c,0)==0); assert(rs_demo(&c,&x)==0);
    assert(g.depth==0 && g.enters==g.leaves && g.permitted==1);
    g.permitted=0; assert(rs_demo(&c,&x)==0); assert(g.permitted==0);
    rs_prepare(&c,186); c.exec.alarm=01202; c.phases.phase[0]=3;
    before=c; assert(rs_recover(&c.record,&c.exec,&c.wait,&c.phases,c.caller)==RS_OUT_OF_SCOPE);
    assert(memcmp(&c,&before,sizeof c)==0);
    c=before; c.phases.phase[0]=00402; c.exec.alarm=01204;
    before=c; assert(rs_recover(&c.record,&c.exec,&c.wait,&c.phases,c.caller)==RS_OUT_OF_SCOPE);
    assert(memcmp(&c,&before,sizeof c)==0);
    rs_prepare(&c,186); c.exec.alarm=01202; c.record.failreg[0]=077777;
    before=c; assert(rs_recover(&c.record,&c.exec,&c.wait,&c.phases,c.caller)==RS_OUT_OF_SCOPE);
    assert(memcmp(&c,&before,sizeof c)==0);
    rs_prepare(&c,186); c.exec.alarm=01202; c.wait.alarm=01203;
    before=c; assert(rs_recover(&c.record,&c.exec,&c.wait,&c.phases,c.caller)==RS_OUT_OF_SCOPE);
    assert(memcmp(&c,&before,sizeof c)==0);
    rs_prepare(&c,186); c.wait.alarm=01203;
    assert(rs_recover(&c.record,&c.exec,&c.wait,&c.phases,c.caller)==RS_OK);
    assert(c.record.failreg[0]==01203 && c.wait.alarm==0);
    rs_prepare(&c,186); c.exec.alarm=01202; c.phases.complement[5]^=1;
    assert(rs_recover(&c.record,&c.exec,&c.wait,&c.phases,c.caller)==RS_PHASE_BAD);
    assert(c.record.failreg[0]==01202 && c.exec.core[0][5]==5 && c.exec.newjob==AGC_NEG_ZERO);
    rs_prepare(&c,186); c.exec.alarm=01202;
    for(i=0;i<6;++i) c.phases.priority[i]=03000;
    assert(rs_recover(&c.record,&c.exec,&c.wait,&c.phases,c.caller)==RS_RESOURCE_ALARM);
    assert(c.exec.alarm==01201 && c.record.failreg[0]==01202);
    for(i=0;i<5;++i) assert(c.exec.vac[i][0]==0);
    rs_prepare(&c,138); c.exec.newjob=0;
    before=c; assert(rs_job(&c.exec,&c.phases,0)==EXEC_OUT_OF_SCOPE); assert(memcmp(&c,&before,sizeof c)==0);
    puts("PASS: 226 assembly fingerprints, recovery/resumption, phase/resource failures and restoring exclusion");
    return 0;
}
