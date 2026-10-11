/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "dot_cases.h"
int main(void) {
    dot_case c,before;unsigned mode;
    assert(dt_check(&c)==0);
    for(mode=0;mode<=077777u;++mode) {
        if(mode==077776u)continue;
        dt_prepare(&c,8321);c.previous.previous.previous.preparation.mpac[7]=(agc_word)mode;
        before=c;assert(!dt_apply(&c));assert(memcmp(&c,&before,sizeof c)==0);
    }
    puts("PASS: 96 complete unrounded original DOT states and exact scratch/overflow exchanges; all unsupported MODEs reject unchanged");
    return 0;
}
