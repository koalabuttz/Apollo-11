/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "elapsed_cases.h"
static void emit(const elapsed_case *c,uint16_t n,uint8_t step,uint8_t stage,uint8_t path) {
    uint16_t w[EL_WORD_COUNT]; unsigned i;
    el_words(c,w); printf("%u %u %u %u",n,step,stage,path);
    for(i=0;i<EL_WORD_COUNT;++i) printf(" %u",w[i]);
    puts("");
}
int main(void) {
    uint16_t n;
    for(n=0;n<EL_CASE_COUNT;++n) {
        elapsed_case c; uint8_t step;
        el_prepare(&c,n);
        for(step=0;step<el_steps(n);++step) {
            uint8_t path;
            el_next(&c,n,step);
            if(el_integrated(n)) { el_prefix(&c,n); emit(&c,n,step,0,0); }
            path=el_apply(&c); emit(&c,n,step,1,path);
        }
    }
    return 0;
}
