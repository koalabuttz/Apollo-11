/* SPDX-License-Identifier: MIT */
#include "dot_cases.h"
static uint16_t random_word(uint32_t *seed) {
    *seed=*seed*UINT32_C(1664525)+UINT32_C(1013904223);
    return (uint16_t)((*seed>>16)&AGC_MASK);
}
void dt_prepare(dot_case *c,uint16_t n) {
    p66_preparation *s=&c->previous.previous.previous.preparation;
    uint32_t seed=UINT32_C(0xd074c64)^((uint32_t)n*UINT32_C(2654435761));unsigned i;
    static const uint8_t offsets[6]={0,1,3,4,5,6};
    el_prepare(&c->previous,(uint16_t)(419u+n%2048u));
    for(i=0;i<7u;++i)s->mpac[i]=random_word(&seed);
    s->mpac[7]=077776;
    c->divide.buf_low=random_word(&seed);c->divide.dvsign=random_word(&seed);
    c->divide.dvnormct=random_word(&seed);c->divide.maxdvsw=random_word(&seed);c->buf_third=random_word(&seed);
    for(i=0;i<6u;++i)c->operand[i]=random_word(&seed);
    if(n<32u) {
        static const uint16_t edge[8][2]={{0,0},{077777,077777},{020000,0},{057777,077777},
            {037777,037777},{040000,040000},{1,040000},{077776,037777}};
        for(i=0;i<3u;++i) {
            uint16_t first=(uint16_t)((n+i)%8u),second=(uint16_t)((n/4u+2u*i)%8u);
            s->mpac[offsets[2u*i]]=edge[first][0];s->mpac[offsets[2u*i+1u]]=edge[first][1];
            c->operand[2u*i]=edge[second][0];c->operand[2u*i+1u]=edge[second][1];
        }
    } else if(n<8224u) {
        static const uint16_t corner[4]={0,077777,037777,040000};
        uint16_t index=(uint16_t)((n-32u)%4096u);
        for(i=0;i<6u;++i) {
            uint16_t value=corner[(index>>(2u*i))&3u];
            s->mpac[offsets[i]]=value;c->operand[i]=n<4128u?value:(uint16_t)(value^AGC_MASK);
        }
    }
}
void dt_words(const dot_case *c,uint16_t w[DT_WORD_COUNT]) {
    unsigned i;el_words(&c->previous,w);w[89]=c->divide.buf_low;w[90]=c->divide.dvsign;
    w[91]=c->divide.dvnormct;w[92]=c->divide.maxdvsw;w[93]=c->buf_third;
    for(i=0;i<6u;++i)w[94+i]=c->operand[i];
}
uint16_t dt_target_number(uint16_t row) {
    return row<32u?row:row<64u?(uint16_t)(32u+((row-32u)/2u)*251u+((row&1u)?4096u:0u)):
        (uint16_t)(8224u+(row-64u)*61u);
}
#ifndef DOT_INPUT_ONLY
uint8_t dt_apply(dot_case *c) {
    return p66_dot(&c->previous.previous.previous.preparation,&c->previous.previous.previous.scratch,
                   &c->previous.elapsed,&c->divide,&c->buf_third,c->operand);
}
#endif
