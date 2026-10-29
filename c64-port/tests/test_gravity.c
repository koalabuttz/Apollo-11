/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "gravity_cases.h"
int main(void) {
    gravity_case c,before; unsigned n;
    assert(gv_check(&c)==0);
    gv_prepare(&c,0); before=c;
    for(n=1;n<=077777u;++n) {
        c=before; c.previous.previous.previous.previous.preparation.mpac[7]=(agc_word)n;
        {
            gravity_case bad=c;
            assert(gv_apply(&c)==P66_PUSH_OUT_OF_SCOPE);
            assert(memcmp(&c,&bad,sizeof c)==0);
        }
    }
    for(n=0;n<=077777u;++n) {
        p66_pushdown *p;
        c=before; p=&c.previous.previous.previous.push;
        p->base=(agc_word)n; p->pushloc=(agc_word)n;
        if(n>=0401u && n<=0661u && (n-0401u)%054u==0) continue;
        {
            gravity_case bad=c;
            assert(gv_apply(&c)==P66_PUSH_OUT_OF_SCOPE);
            assert(memcmp(&c,&bad,sizeof c)==0);
        }
    }
    for(n=0;n<=65535u;++n) {
        p66_pushdown *p;
        c=before; p=&c.previous.previous.previous.push; p->pushloc=(agc_word)n;
        if(n>=p->base && n-p->base<=P66_VAC_WORDS-2u) continue;
        {
            gravity_case bad=c;
            assert(gv_apply(&c)==P66_PUSH_OUT_OF_SCOPE);
            assert(memcmp(&c,&bad,sizeof c)==0);
        }
    }
    puts("PASS: 156 complete PDVL states; all unsupported modes, VAC bases and push pointers reject unchanged");
    return 0;
}
