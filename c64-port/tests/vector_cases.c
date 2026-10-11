/* SPDX-License-Identifier: MIT */
#include "vector_cases.h"
static uint16_t random_word(uint32_t *seed) {
    *seed=*seed*UINT32_C(1664525)+UINT32_C(1013904223);
    return (uint16_t)((*seed>>16)&AGC_MASK);
}
uint16_t ve_target_number(uint16_t index) {
    return index<32u ? (uint16_t)(index*8u) : index<48u ? (uint16_t)(256u+index-32u)
                   : (uint16_t)(4496u+index-48u);
}
void ve_prepare(vector_case *c,uint16_t n,uint8_t subtract) {
    uint16_t w[VE_WORD_COUNT]; unsigned i;
    uint32_t seed=UINT32_C(0x67517000)^((uint32_t)n*UINT32_C(2654435761));
    static const uint8_t offset[3]={0,3,5};
    for(i=0;i<VE_WORD_COUNT;++i) w[i]=random_word(&seed);
    w[4]=(uint16_t)(w[4] | ((w[4]&040000u)<<1));
    w[5]=(uint16_t)(w[5] | ((w[5]&040000u)<<1)); w[32]=077776;
    ve_from_words(c,w);
    if(n<256u) {
        static const uint16_t corner[4]={0,077777,037777,040000};
        for(i=0;i<3;++i) {
            c->preparation.mpac[offset[i]]=corner[(n+i)&3u];
            c->preparation.mpac[offset[i]+1u]=corner[((n>>2)+i)&3u];
            c->operand[2u*i]=corner[((n>>4)+i)&3u];
            c->operand[2u*i+1u]=corner[((n>>6)+i)&3u];
        }
    } else if(n<400u) {
        static const uint16_t edge[12]={0,077777,1,077776,2,077775,017777,060000,020000,057777,037777,040000};
        for(i=0;i<3;++i) {
            c->preparation.mpac[offset[i]]=edge[((n-256u)/12u+i)%12u];
            c->preparation.mpac[offset[i]+1u]=edge[((n-256u)%12u+i)%12u];
            c->operand[2u*i]=edge[((n-256u)/12u+i+5u)%12u];
            c->operand[2u*i+1u]=edge[((n-256u)%12u+i+7u)%12u];
        }
    } else if(n<4496u) {
        /* All 4096 combinations of four corner words across three components. */
        static const uint16_t corner[4]={0,077777,037777,040000};
        for(i=0;i<3;++i) {
            unsigned pair=((unsigned)n-400u)>>(4u*i);
            c->preparation.mpac[offset[i]]=corner[pair&3u];
            c->preparation.mpac[offset[i]+1u]=corner[(pair>>2)&3u];
            c->operand[2u*i]=corner[(pair+1u)&3u];
            c->operand[2u*i+1u]=corner[((pair>>2)+2u)&3u];
        }
    }
    /* The same raw operands exercise both operations; no expected results here. */
    (void)subtract;
}
void ve_words(const vector_case *c,uint16_t w[VE_WORD_COUNT]) {
    unsigned i; pp_words(&c->preparation,w);
    w[39]=c->scale.mptemp; w[40]=c->scale.ovfind;
    w[41]=c->elapsed.piptime.high; w[42]=c->elapsed.piptime.low; w[43]=c->elapsed.buf;
    for(i=0;i<6;++i) w[44+i]=c->operand[i];
}
void ve_from_words(vector_case *c,const uint16_t w[VE_WORD_COUNT]) {
    unsigned i; pp_from_words(&c->preparation,w);
    c->scale.mptemp=w[39]; c->scale.ovfind=w[40];
    c->elapsed.piptime.high=w[41]; c->elapsed.piptime.low=w[42]; c->elapsed.buf=w[43];
    for(i=0;i<6;++i) c->operand[i]=w[44+i];
}
#ifndef VECTOR_INPUT_ONLY
uint16_t ve_apply(vector_case *c,uint8_t subtract) {
    return subtract ? p66_vector_subtract(&c->preparation,&c->scale,&c->elapsed,c->operand)
                    : p66_vector_add(&c->preparation,&c->scale,&c->elapsed,c->operand);
}
#endif
