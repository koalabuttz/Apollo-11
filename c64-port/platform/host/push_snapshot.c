/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "push_cases.h"
static void emit(const push_case *c,uint16_t n,uint8_t step,uint8_t stage) {
    uint16_t w[PS_WORD_COUNT]; unsigned i;
    ps_words(c,w); printf("%u %u %u",n,step,stage);
    for(i=0;i<PS_WORD_COUNT;++i) printf(" %u",w[i]);
    puts("");
}
int main(void) {
    uint16_t n;
    for(n=0;n<PS_CASE_COUNT;++n) {
        push_case c; uint8_t step;
        ps_prepare(&c,n);
        for(step=0;step<ps_steps(n);++step) {
            ps_next(&c,n,step);
            if(ps_integrated(n)) { ps_prefix(&c,n); emit(&c,n,step,0); }
            if(ps_apply(&c)!=P66_PUSH_OK) return 2;
            emit(&c,n,step,1);
        }
    }
    return 0;
}
