/* SPDX-License-Identifier: MIT */
#include "prepare_cases.h"
static uint16_t extend_word(uint16_t word) {
    return (uint16_t)(word | (uint16_t)((word & 040000u) << 1));
}
void pp_words(const prepare_case *c, uint16_t w[PP_WORD_COUNT]) {
    unsigned i;
    w[0]=c->rate.count; w[1]=c->rate.scale;
    w[2]=c->rate.desired.high; w[3]=c->rate.desired.low;
    w[4]=c->rate.registers.a; w[5]=c->rate.registers.l;
    for(i=0;i<3;++i) w[6+i]=c->pipa[i];
    w[9]=c->time.high; w[10]=c->time.low;
    for(i=0;i<3;++i) { w[11+i]=c->old_pipa[i]; w[14+i]=c->ruptreg[i]; }
    w[17]=c->this_piptime.high; w[18]=c->this_piptime.low;
    for(i=0;i<3;++i) { w[19+i]=c->pipatmp[i]; w[22+i]=c->tem[i]; }
    for(i=0;i<8;++i) w[25+i]=c->mpac[i];
    for(i=0;i<6;++i) w[33+i]=c->delvrod[i];
}
void pp_from_words(prepare_case *c, const uint16_t w[PP_WORD_COUNT]) {
    unsigned i;
    c->rate.count=w[0]; c->rate.scale=w[1];
    c->rate.desired.high=w[2]; c->rate.desired.low=w[3];
    c->rate.registers.a=w[4]; c->rate.registers.l=w[5];
    for(i=0;i<3;++i) c->pipa[i]=w[6+i];
    c->time.high=w[9]; c->time.low=w[10];
    for(i=0;i<3;++i) { c->old_pipa[i]=w[11+i]; c->ruptreg[i]=w[14+i]; }
    c->this_piptime.high=w[17]; c->this_piptime.low=w[18];
    for(i=0;i<3;++i) { c->pipatmp[i]=w[19+i]; c->tem[i]=w[22+i]; }
    for(i=0;i<8;++i) c->mpac[i]=w[25+i];
    for(i=0;i<6;++i) c->delvrod[i]=w[33+i];
}
void pp_prepare(prepare_case *c, uint16_t n) {
    static const uint16_t edges[8]={0,077777,1,077776,037777,040000,020000,057777};
    uint16_t words[PP_WORD_COUNT]; unsigned i;
    uint32_t seed=UINT32_C(0x32753353) ^ ((uint32_t)n * UINT32_C(2654435761));
    for(i=0;i<PP_WORD_COUNT;++i) {
        seed=seed * UINT32_C(1664525)+UINT32_C(1013904223);
        words[i]=(uint16_t)((seed >> 16) & AGC_MASK);
    }
    words[4]=extend_word(words[4]); words[5]=extend_word(words[5]);
    pp_from_words(c,words);
    if(n<128u) {
        /* All 64 TS skip masks in each sign family, forced by input only.
         * Two same-sign delta terms overflow with either a loaded CS value
         * or the previous TS's +/-1; register end-around carry is exercised. */
        uint16_t polarity=n<64u ? 037777 : 040000;
        uint16_t opposite=n<64u ? 040000 : 037777;
        for(i=0;i<3;++i) {
            c->pipa[i]=polarity;
            c->pipatmp[i]=(n & (uint16_t)(1u << i)) ? polarity : 0;
            c->tem[i]=c->old_pipa[i]=(n & (uint16_t)(1u << (i+3u))) ? opposite : 0;
        }
    } else if(n<144u) {
        for(i=0;i<3;++i) {
            c->pipa[i]=edges[(n+i)%8u]; c->old_pipa[i]=edges[(n+i+3u)%8u];
            c->pipatmp[i]=edges[(n+i+1u)%8u]; c->tem[i]=edges[(n+i+5u)%8u];
        }
        c->time.high=edges[n%8u]; c->time.low=edges[(n+1u)%8u];
        c->this_piptime.high=edges[(n+2u)%8u]; c->this_piptime.low=edges[(n+3u)%8u];
    }
}
uint8_t pp_steps(uint16_t n) { return n>=144u && n<PP_TARGET_COUNT ? 4 : 1; }
void pp_next(prepare_case *c, uint16_t n, uint8_t step) {
    static const uint16_t clicks[4]={1,077776,077777,3};
    unsigned i;
    if(n>=144u && n<PP_TARGET_COUNT) {
        c->rate.count=clicks[step];
        c->time.high=(uint16_t)(step==3u ? 1 : 0);
        c->time.low=(uint16_t)(step==3u ? 0 : 037775u+step);
        for(i=0;i<3;++i) {
            uint16_t raw=(uint16_t)((step+1u)*(i+2u)+(n-144u)*7u);
            c->pipa[i]=(n==145u) ? (agc_word)(raw ^ AGC_MASK) : raw;
            c->pipatmp[i]=(uint16_t)(i+step);
            /* Simulate READACCS updates between calls; prior OLD/time,
             * DELV low words and MPAC scratch survive naturally. */
            c->tem[i]=(uint16_t)(step+i+1u);
        }
    }
}
#ifndef PREPARE_INPUT_ONLY
uint8_t pp_apply(prepare_case *c, uint8_t stage) {
    if(stage==0) p66_rate_update(&c->rate);
    else if(stage==1) p66_snapshot(c);
    else return p66_vector_inputs(c);
    return 0;
}
#endif
