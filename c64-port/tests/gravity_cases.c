/* SPDX-License-Identifier: MIT */
#include "gravity_cases.h"
static uint16_t divide_number(uint16_t n) {
    if(n<147u) return n;
    if(n<357u) return (uint16_t)(147u+(n-147u)%256u);
    if(n<537u) return (uint16_t)(16u+(n-357u)%128u);
    if(n<793u) return (uint16_t)(147u+n-537u);
    if(n<2841u) return (uint16_t)(593u+n-793u);
    return (uint16_t)(2641u+n-2841u);
}
uint8_t gv_integrated(uint16_t n) {
    return (uint8_t)((n>=16u && n<147u) || (n>=357u && n<537u) || n>=2841u);
}
uint8_t gv_steps(uint16_t n) { return n>=144u && n<147u ? 4 : 1; }
void gv_prepare(gravity_case *c,uint16_t n) {
    unsigned i; uint32_t seed=UINT32_C(0x65571236)^((uint32_t)n*UINT32_C(2654435761));
    p66_pushdown *p=&c->previous.previous.previous.push;
    dv_prepare(&c->previous,divide_number(n));
    if(gv_integrated(n) && n<144u) p->pushloc=(agc_word)(p->base+n%36u);
    if(n>=147u && n<357u) {
        uint16_t k=(uint16_t)(n-147u);
        p->base=(agc_word)(0401u+(k/42u)*054u); p->pushloc=(agc_word)(p->base+k%42u);
    } else if(n>=357u && n<537u) {
        uint16_t k=(uint16_t)(n-357u);
        p->base=(agc_word)(0401u+(k/36u)*054u); p->pushloc=(agc_word)(p->base+k%36u);
    } else if(gv_integrated(n) && n>=147u) p->pushloc=p->base;
    for(i=0;i<6;++i) {
        seed=seed*UINT32_C(1664525)+UINT32_C(1013904223);
        c->gravity[i]=(agc_word)((seed>>16)&AGC_MASK);
    }
    if(n<16u || (n>=537u && n<793u)) {
        static const agc_word corner[4]={0,077777,037777,040000};
        uint16_t k=n<16u ? n : (uint16_t)(n-537u);
        for(i=0;i<6;++i) c->gravity[i]=corner[(k>>(2u*(i%4u)))&3u];
    }
}
void gv_next(gravity_case *c,uint16_t n,uint8_t step) { dv_next(&c->previous,divide_number(n),step); }
void gv_words(const gravity_case *c,uint16_t w[GV_WORD_COUNT]) {
    unsigned i; dv_words(&c->previous,w); for(i=0;i<6;++i) w[93u+i]=c->gravity[i];
}
void gv_from_words(gravity_case *c,const uint16_t w[GV_WORD_COUNT]) {
    unsigned i; dv_from_words(&c->previous,w); for(i=0;i<6;++i) c->gravity[i]=w[93u+i];
}
#ifndef GRAVITY_INPUT_ONLY
void gv_prefix(gravity_case *c,uint16_t n) { dv_prefix(&c->previous,divide_number(n)); (void)dv_apply(&c->previous); }
enum p66_push_status gv_apply(gravity_case *c) {
    return p66_push_gravity(&c->previous.previous.previous.previous.preparation,
        &c->previous.previous.previous.push,c->gravity);
}
#endif
