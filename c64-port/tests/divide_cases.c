/* SPDX-License-Identifier: MIT */
#include "divide_cases.h"
static uint16_t store_number(uint16_t n) {
    if(n<16u) return n;
    if(n<147u) return (uint16_t)(n+16u);
    if(n<403u) return (uint16_t)(n-147u+353u);
    if(n<593u) return (uint16_t)(n-403u+163u);
    if(n<2641u) return (uint16_t)(n-593u+609u);
    if(n<3665u) return (uint16_t)(n-2641u+2657u);
    return (uint16_t)(609u+(n-3665u)%2048u);
}
uint8_t dv_integrated(uint16_t n) { return (uint8_t)((n>=16u && n<147u) || (n>=403u && n<593u) || (n>=2641u && n<3665u)); }
uint8_t dv_steps(uint16_t n) { return n>=144u && n<147u ? 4 : 1; }
void dv_prepare(divide_case *c,uint16_t n) {
    static const agc_word inputs[16][2]={
        {0,0},{077777,077777},{0,1},{077777,077776},
        {0,399},{077777,077160},{0,400},{077777,077157},
        {0,401},{077777,077156},{1,040000},{077776,037777},
        {1,0},{077776,077777},{0,077777},{077777,0}};
    st_prepare(&c->previous,store_number(n));
    if(n<16u) {
        c->previous.previous.previous.preparation.mpac[0]=inputs[n][0];
        c->previous.previous.previous.preparation.mpac[1]=inputs[n][1];
    }
    if(n>=3665u && n<5713u) {
        int16_t value=(int16_t)((int)(n-3665u)-1024);
        c->previous.previous.previous.preparation.mpac[0]=value<0 ? 077777 : 0;
        c->previous.previous.previous.preparation.mpac[1]=(agc_word)(value<0 ? AGC_MASK ^ (agc_word)(-value) : (agc_word)value);
    }
    if(n>=5713u) {
        uint16_t k=(uint16_t)(n-5713u),value=(uint16_t)(k%511u+1u);
        c->previous.previous.previous.preparation.mpac[0]=k<511u ? 1 : 077776;
        c->previous.previous.previous.preparation.mpac[1]=k<511u ? (agc_word)(AGC_MASK ^ (16384u-value)) : (agc_word)(16384u-value);
    }
    c->scratch.buf_low=(agc_word)((n*17u+012345u)&AGC_MASK);
    c->scratch.dvsign=(agc_word)((n*19u+023456u)&AGC_MASK);
    c->scratch.dvnormct=(agc_word)((n*23u+034567u)&AGC_MASK);
    c->scratch.maxdvsw=(agc_word)((n*29u+045670u)&AGC_MASK);
}
void dv_next(divide_case *c,uint16_t n,uint8_t step) { st_next(&c->previous,store_number(n),step); }
void dv_words(const divide_case *c,uint16_t w[DV_WORD_COUNT]) {
    el_words(&c->previous,w); w[89]=c->scratch.buf_low; w[90]=c->scratch.dvsign;
    w[91]=c->scratch.dvnormct; w[92]=c->scratch.maxdvsw;
}
void dv_from_words(divide_case *c,const uint16_t w[DV_WORD_COUNT]) {
    el_from_words(&c->previous,w); c->scratch.buf_low=w[89]; c->scratch.dvsign=w[90];
    c->scratch.dvnormct=w[91]; c->scratch.maxdvsw=w[92];
}
#ifndef DIVIDE_INPUT_ONLY
void dv_prefix(divide_case *c,uint16_t n) {
    (void)st_prefix(&c->previous,store_number(n)); (void)st_apply(&c->previous);
}
enum p66_divide_result dv_apply(divide_case *c) {
    return p66_divide_four_seconds(&c->previous.previous.previous.preparation,
        &c->previous.previous.previous.scratch,&c->previous.elapsed,&c->scratch);
}
#endif
