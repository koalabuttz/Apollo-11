/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "prepare_cases.h"
int main(void) {
    uint16_t n;
    for(n=0;n<PP_CASE_COUNT;++n) {
        prepare_case c; uint8_t step;
        pp_prepare(&c,n);
        for(step=0;step<pp_steps(n);++step) {
            uint8_t stage;
            pp_next(&c,n,step);
            for(stage=n<144u ? 1 : 0;stage<3;++stage) {
                uint16_t words[PP_WORD_COUNT]; unsigned i;
                uint8_t mask=pp_apply(&c,stage);
                pp_words(&c,words); printf("%u %u %u %u",n,step,stage,mask);
                for(i=0;i<PP_WORD_COUNT;++i) printf(" %u",words[i]);
                puts("");
            }
        }
    }
    return 0;
}
