/* SPDX-License-Identifier: MIT */
#include "push_cases.h"
static uint16_t scale_number(uint16_t n) {
    return n<188u ? (uint16_t)(n-60u+152u) : n<191u ? (uint16_t)(n-188u+280u)
           : (uint16_t)(n-1299u+2331u);
}
uint8_t ps_integrated(uint16_t n) {
    return (uint8_t)((n>=60u && n<191u) || n>=1299u);
}
void ps_prepare(push_case *c,uint16_t n) {
    static const uint16_t edges[12]={0,077777,1,077776,2,077775,
                                  017777,060000,020000,057777,037777,040000};
    unsigned i;
    uint32_t seed=UINT32_C(0x65236540) ^ ((uint32_t)n*UINT32_C(2654435761));
    if(ps_integrated(n)) sc_prepare(&c->previous,scale_number(n));
    else {
        uint16_t edge=n<60u ? n : (uint16_t)(n-131u);
        sc_prepare(&c->previous,(uint16_t)(283u+n%1024u));
        if(n<275u) {
            p66_preparation *s=&c->previous.preparation;
            static const uint8_t positions[3]={0,3,5};
            s->this_piptime.high=edges[edge/12u]; s->this_piptime.low=edges[edge%12u];
            for(i=0;i<3;++i) {
                s->mpac[positions[i]]=edges[(edge+i*3u)%12u];
                s->mpac[positions[i]+1u]=edges[(edge/12u+i*5u)%12u];
            }
        }
    }
    c->push.base=(uint16_t)(0401u+(n%5u)*054u);
    c->push.pushloc=(uint16_t)(c->push.base+(n%38u));
    if(n>=188u && n<191u) c->push.pushloc=(uint16_t)(c->push.base+(n-188u)*5u);
    for(i=0;i<P66_VAC_WORDS;++i) {
        seed=seed*UINT32_C(1664525)+UINT32_C(1013904223);
        c->push.words[i]=n<275u ? edges[(n+i)%12u] : (uint16_t)((seed>>16)&AGC_MASK);
    }
}
uint8_t ps_steps(uint16_t n) { return n>=188u && n<191u ? 4 : 1; }
void ps_next(push_case *c,uint16_t n,uint8_t step) {
    if(ps_integrated(n)) sc_next(&c->previous,scale_number(n),step);
}
void ps_words(const push_case *c,uint16_t w[PS_WORD_COUNT]) {
    unsigned i;
    sc_words(&c->previous,w); w[41]=c->push.base; w[42]=c->push.pushloc;
    for(i=0;i<P66_VAC_WORDS;++i) w[43+i]=c->push.words[i];
}
void ps_from_words(push_case *c,const uint16_t w[PS_WORD_COUNT]) {
    unsigned i;
    sc_from_words(&c->previous,w); c->push.base=w[41]; c->push.pushloc=w[42];
    for(i=0;i<P66_VAC_WORDS;++i) c->push.words[i]=w[43+i];
}
#ifndef PUSH_INPUT_ONLY
void ps_prefix(push_case *c,uint16_t n) { sc_apply(&c->previous,scale_number(n)); }
enum p66_push_status ps_apply(push_case *c) {
    return p66_push_timestamp(&c->previous.preparation,&c->push);
}
#endif
