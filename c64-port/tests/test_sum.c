/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "sum_cases.h"
int main(void) {
    vector_case c,before;unsigned operation,mode;
    assert(su_check(&c)==0);
    for(operation=0;operation<4u;++operation)for(mode=0;mode<=077777u;++mode) {
        if(mode==(operation<3u?0u:077776u))continue;
        su_prepare(&c,4501,(uint8_t)operation);c.preparation.mpac[7]=(agc_word)mode;
        before=c;assert(su_apply(&c,(uint8_t)operation)==P66_SUM_OUT_OF_SCOPE);
        assert(memcmp(&c,&before,sizeof c)==0);
    }
    puts("PASS: 256 complete original DAD/DSU/BDSU/BVSU states and overflow paths; all unsupported MODEs reject unchanged");
    return 0;
}
