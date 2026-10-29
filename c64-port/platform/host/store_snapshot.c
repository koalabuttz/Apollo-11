/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "store_cases.h"
static void emit(const store_case *c,uint16_t n,uint8_t step,uint8_t stage,uint8_t path) {
    uint16_t w[ST_WORD_COUNT]; unsigned i;
    el_words(c,w); printf("%u %u %u %u",n,step,stage,path);
    for(i=0;i<ST_WORD_COUNT;++i) printf(" %u",w[i]);
    puts("");
}
int main(void) {
    uint16_t n;
    for(n=0;n<ST_CASE_COUNT;++n) {
        store_case c; uint8_t step;
        st_prepare(&c,n);
        for(step=0;step<st_steps(n);++step) {
            uint8_t path=0;
            st_next(&c,n,step);
            if(st_integrated(n)) { path=st_prefix(&c,n); emit(&c,n,step,0,path); }
            if(st_apply(&c)!=P66_STORE_OK) return 2;
            emit(&c,n,step,1,path);
        }
    }
    return 0;
}
