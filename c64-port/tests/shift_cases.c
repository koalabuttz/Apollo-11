/* SPDX-License-Identifier: MIT */
#include "shift_cases.h"
static uint16_t random_word(uint32_t *seed) {
    *seed=*seed*UINT32_C(1664525)+UINT32_C(1013904223);
    return (uint16_t)((*seed>>16)&AGC_MASK);
}
void sh_prepare(shift_case *c,uint16_t n) {
    uint16_t group=(uint16_t)(n/SH_GROUP_SIZE),local=(uint16_t)(n%SH_GROUP_SIZE);
    uint32_t seed=UINT32_C(0x5b177c64)^((uint32_t)n*UINT32_C(2654435761));
    p66_preparation *s=&c->previous.previous.previous.preparation;
    unsigned i;
    el_prepare(&c->previous,(uint16_t)(419u+n%2048u));
    if(group<60u) { c->short_form=0; c->operation=(uint8_t)(group/15u); c->count=(uint8_t)(group%15u); }
    else {
        group=(uint16_t)(group-60u); c->short_form=1;
        if(group<8u) { c->operation=(uint8_t)(group/4u); c->count=(uint8_t)(group%4u+1u); }
        else { group=(uint16_t)(group-8u); c->operation=(uint8_t)(2u+group/8u); c->count=(uint8_t)(group%8u+1u); }
    }
    for(i=0;i<7u;++i) s->mpac[i]=random_word(&seed);
    s->mpac[7]=c->operation<2u ? 0 : 077776;
    if(local<64u) {
        static const uint16_t corner[4]={0,077777,037777,040000};
        for(i=0;i<7u;++i) s->mpac[i]=corner[(local>>(2u*(i%3u)))&3u];
    }
}
void sh_words(const shift_case *c,uint16_t w[SH_WORD_COUNT]) { el_words(&c->previous,w); }
uint16_t sh_target_number(uint16_t row) {
    return (uint16_t)((row/2u)*SH_GROUP_SIZE+((row&1u)?137u:37u));
}
#ifndef SHIFT_INPUT_ONLY
uint8_t sh_apply(shift_case *c) {
    p66_preparation *s=&c->previous.previous.previous.preparation;
    p66_scale_scratch *sc=&c->previous.previous.previous.scratch;
    switch(c->operation) {
    case 0:return p66_scalar_shift_left(s,sc,c->count,c->short_form);
    case 1:return p66_scalar_shift_right(s,sc,c->count,c->short_form);
    case 2:return p66_vector_shift_left(s,sc,&c->previous.elapsed,c->count,c->short_form);
    default:return p66_vector_shift_right(s,sc,c->count,c->short_form);
    }
}
#endif
