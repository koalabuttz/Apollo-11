/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "divide_cases.h"
static void emit(const divide_case *c,uint16_t n,uint8_t step,uint8_t stage,uint8_t path) {
    uint16_t w[DV_WORD_COUNT]; unsigned i;
    dv_words(c,w); printf("%u %u %u %u",n,step,stage,path);
    for(i=0;i<DV_WORD_COUNT;++i) printf(" %u",w[i]);
    puts("");
}
int main(void) {
    uint16_t n;
    for(n=0;n<DV_CASE_COUNT;++n) {
        divide_case c; uint8_t step;
        dv_prepare(&c,n);
        for(step=0;step<dv_steps(n);++step) {
            uint8_t path=0;
            dv_next(&c,n,step);
            if(dv_integrated(n)) { dv_prefix(&c,n); emit(&c,n,step,0,path); }
            path=(uint8_t)dv_apply(&c);
            emit(&c,n,step,1,path);
        }
    }
    return 0;
}
