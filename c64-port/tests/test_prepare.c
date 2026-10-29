/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "prepare_cases.h"
int main(void) {
    prepare_case c; unsigned i;
    assert(pp_check(&c)==0);
    memset(&c,0,sizeof c);
    c.pipa[0]=10; c.pipa[1]=20; c.pipa[2]=30;
    c.old_pipa[0]=7; c.old_pipa[1]=18; c.old_pipa[2]=29;
    c.this_piptime.high=1; c.this_piptime.low=077777;
    c.time.high=2; c.time.low=3; c.mpac[2]=012345;
    for(i=0;i<3;++i) {
        c.pipatmp[i]=(uint16_t)(i+1u); c.tem[i]=(uint16_t)(i+4u);
        c.delvrod[i*2+1]=(uint16_t)(01642u+i);
    }
    p66_snapshot(&c);
    assert(c.rate.registers.a==1 && c.rate.registers.l==0177777);
    assert(c.ruptreg[0]==7 && c.ruptreg[1]==18 && c.ruptreg[2]==29);
    assert(c.this_piptime.high==2 && c.this_piptime.low==3);
    assert(p66_vector_inputs(&c)==0);
    assert(c.mpac[0]==11 && c.mpac[3]==22 && c.mpac[5]==33);
    assert(c.delvrod[0]==1 && c.delvrod[2]==3 && c.delvrod[4]==5);
    assert(c.mpac[2]==012345 && c.mpac[7]==077776);
    assert(c.mpac[1]==0 && c.mpac[4]==0 && c.mpac[6]==0);
    for(i=0;i<3;++i) {
        assert(c.old_pipa[i]==c.pipa[i] && c.pipa[i]==(i+1u)*10u);
        assert(c.pipatmp[i]==i+1u && c.tem[i]==0 && c.delvrod[i*2+1]==01642u+i);
    }
    assert(c.rate.registers.a==0177776 && c.rate.registers.l==0177777);
    /* In both sign families all six TS instructions skip; CAF ZERO is
     * skipped and all would-be zero stores receive the returned +/-1. */
    pp_prepare(&c,63); p66_snapshot(&c); assert(p66_vector_inputs(&c)==63);
    assert(c.mpac[1]==077776 && c.mpac[4]==077776 && c.mpac[6]==077776);
    for(i=0;i<3;++i) assert(c.tem[i]==077776);
    pp_prepare(&c,127); p66_snapshot(&c); assert(p66_vector_inputs(&c)==63);
    assert(c.mpac[1]==1 && c.mpac[4]==1 && c.mpac[6]==1);
    for(i=0;i<3;++i) assert(c.tem[i]==1);
    puts("PASS: 324 complete preparation states, all TS-skip masks, snapshot debris, MODE alias, retained scratch and composed histories");
    return 0;
}
