/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "store_cases.h"
static void rejected(store_case *c) {
    store_case before=*c;
    assert(st_apply(c)==P66_STORE_OUT_OF_SCOPE);
    assert(memcmp(c,&before,sizeof before)==0);
}
int main(void) {
    store_case c; unsigned n;
    assert(st_check(&c)==0);
    /* STORE has no push-pointer dependency, including at push capacity. */
    st_prepare(&c,353); c.previous.push.pushloc=077777;
    assert(st_apply(&c)==P66_STORE_OK && c.previous.push.pushloc==077777);
    st_prepare(&c,353); c.previous.previous.preparation.mpac[7]=077777; rejected(&c);
    st_prepare(&c,353); c.previous.previous.preparation.mpac[7]=077776; rejected(&c);
    st_prepare(&c,353); c.previous.previous.preparation.mpac[7]=1; rejected(&c);
    for(n=0;n<=077777u;++n) {
        if(n>=0401u && n<=0661u && (n-0401u)%054u==0) continue;
        st_prepare(&c,353); c.previous.push.base=(agc_word)n; rejected(&c);
    }
    puts("PASS: 176 complete STORE states/paths; signed-zero source/destination corners, overlapping slots, retained histories; unchanged-on-scope-rejection and unused PUSHLOC");
    return 0;
}
