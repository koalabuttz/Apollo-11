/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "velocity_cases.h"
static void emit(velocity_case *c,uint16_t n,uint8_t step,uint8_t stage) {
    uint16_t words[VL_WORD_COUNT]; unsigned i; vl_words(c,words);
    printf("%u %u %u",n,step,stage);
    for(i=0;i<VL_WORD_COUNT;++i) printf(" %u",words[i]);
    puts("");
}
int main(void) {
    uint16_t n;
    for(n=0;n<VL_CASE_COUNT;++n) {
        velocity_case c; uint8_t step; vl_prepare(&c,n);
        for(step=0;step<vl_steps(n);++step) {
            vl_next(&c,n,step); if(vl_integrated(n)) { vl_prefix(&c,n); emit(&c,n,step,0); }
            if(vl_apply(&c)!=P66_VELOCITY_OK) return 2;
            emit(&c,n,step,1);
        }
    }
    return 0;
}
