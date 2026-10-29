/* SPDX-License-Identifier: MIT */
#include "velocity_cases.h"
static uint16_t gravity_number(uint16_t n) {
    return n<147u ? n : n<403u ? (uint16_t)(537u+n-147u)
                    : n<2451u ? (uint16_t)(793u+n-403u) : (uint16_t)(2841u+n-2451u);
}
uint8_t vl_integrated(uint16_t n) { return (uint8_t)((n>=16u && n<147u) || n>=2451u); }
uint8_t vl_steps(uint16_t n) { return n>=144u && n<147u ? 4 : 1; }
void vl_prepare(velocity_case *c,uint16_t n) {
    unsigned i; uint32_t seed=UINT32_C(0x33723373)^((uint32_t)n*UINT32_C(2654435761));
    p66_preparation *s=&c->previous.previous.previous.previous.previous.preparation;
    p66_pushdown *p=&c->previous.previous.previous.previous.push;
    gv_prepare(&c->previous,gravity_number(n)); p->pushloc=p->base;
    for(i=0;i<6;++i) {
        seed=seed*UINT32_C(1664525)+UINT32_C(1013904223); c->bias[i]=(agc_word)((seed>>16)&AGC_MASK);
        seed=seed*UINT32_C(1664525)+UINT32_C(1013904223); c->velocity[i]=(agc_word)((seed>>16)&AGC_MASK);
        seed=seed*UINT32_C(1664525)+UINT32_C(1013904223); c->position[i]=(agc_word)((seed>>16)&AGC_MASK);
    }
    if(!vl_integrated(n)) {
        static const uint8_t offset[3]={0,3,5};
        p->pushloc=(agc_word)(p->base+8u); s->mpac[7]=077776;
        for(i=0;i<3u;++i) { s->mpac[offset[i]]=c->previous.gravity[2u*i]; s->mpac[offset[i]+1u]=c->previous.gravity[2u*i+1u]; }
    }
    if(n<16u || (n>=147u && n<403u)) {
        static const agc_word corner[4]={0,077777,037777,040000};
        unsigned k=n<16u ? n : n-147u;
        for(i=0;i<6u;++i) {
            c->bias[i]=corner[((k>>(2u*(i%4u)))+i)&3u];
            c->velocity[i]=corner[((k>>(2u*(i%4u)))+i+1u)&3u];
            c->position[i]=corner[((k>>(2u*(i%4u)))+i+2u)&3u];
            p->words[i]=corner[((k>>(2u*(i%4u)))+i+3u)&3u];
        }
        p->words[6]=corner[(k>>4)&3u]; p->words[7]=corner[(k>>6)&3u];
    }
}
void vl_next(velocity_case *c,uint16_t n,uint8_t step) { gv_next(&c->previous,gravity_number(n),step); }
void vl_words(const velocity_case *c,uint16_t w[VL_WORD_COUNT]) {
    unsigned i; gv_words(&c->previous,w);
    for(i=0;i<6;++i) { w[99+i]=c->bias[i]; w[105+i]=c->velocity[i]; w[111+i]=c->position[i]; }
}
void vl_from_words(velocity_case *c,const uint16_t w[VL_WORD_COUNT]) {
    unsigned i; gv_from_words(&c->previous,w);
    for(i=0;i<6;++i) { c->bias[i]=w[99+i]; c->velocity[i]=w[105+i]; c->position[i]=w[111+i]; }
}
#ifndef VELOCITY_INPUT_ONLY
void vl_prefix(velocity_case *c,uint16_t n) { gv_prefix(&c->previous,gravity_number(n)); (void)gv_apply(&c->previous); }
enum p66_velocity_status vl_apply(velocity_case *c) {
    return p66_update_velocity(&c->previous.previous.previous.previous.previous.preparation,
        &c->previous.previous.previous.previous.previous.scratch,&c->previous.previous.previous.elapsed,
        &c->previous.previous.previous.previous.push,c->bias,c->velocity,c->position);
}
#endif
