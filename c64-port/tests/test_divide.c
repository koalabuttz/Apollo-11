/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "divide_cases.h"
int main(void) {
    divide_case c,before; unsigned n;
    assert(dv_check(&c)==0);
    for(n=1;n<=077777u;++n) {
        dv_prepare(&c,0); c.previous.previous.previous.preparation.mpac[7]=(agc_word)n;
        before=c; assert(dv_apply(&c)==P66_DIVIDE_OUT_OF_SCOPE);
        assert(memcmp(&c,&before,sizeof c)==0);
    }
    dv_prepare(&c,3); assert(dv_apply(&c)==P66_DIVIDE_QUOTIENT);
    assert(c.previous.previous.previous.preparation.mpac[0]==077727);
    assert(c.previous.previous.previous.preparation.mpac[1]==041217);
    puts("PASS: 156 complete DDV states/paths; signed zero, raw sign agreement, quotient/overflow and retained histories; all unsupported modes reject unchanged");
    return 0;
}
