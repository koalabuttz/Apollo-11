/* SPDX-License-Identifier: MIT */
#include "prepare_cases.h"
#include "prepare_vectors.h"
static unsigned check_words(const prepare_case *c,uint16_t row,uint8_t mask) {
    uint16_t words[PP_WORD_COUNT]; unsigned i,failures=0;
    pp_words(c,words);
    if(mask!=prepare_reference_mask[row]) ++failures;
    for(i=0;i<PP_WORD_COUNT;++i) if(words[i]!=prepare_reference_words[row][i]) ++failures;
    return failures;
}
unsigned pp_check(prepare_case *c) {
    uint16_t n,row=0; unsigned failures=0;
    prepare_case composite;
    for(n=0;n<PP_TARGET_COUNT;++n) {
        uint8_t step;
        pp_prepare(c,n);
        if(n>=144u) pp_prepare(&composite,n);
        for(step=0;step<pp_steps(n);++step) {
            uint8_t stage;
            pp_next(c,n,step);
            for(stage=n<144u ? 1 : 0;stage<3;++stage) {
                uint8_t mask=pp_apply(c,stage);
                failures+=check_words(c,row,mask);
                ++row;
            }
            if(n>=144u) {
                uint8_t mask;
                pp_next(&composite,n,step); mask=p66_prepare(&composite);
                failures+=check_words(&composite,(uint16_t)(row-1u),mask);
            }
        }
    }
    if(row!=PP_TARGET_ROWS) ++failures;
    return failures;
}
