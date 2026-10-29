/* SPDX-License-Identifier: MIT */
#include "store_cases.h"
uint16_t st_target_extra(uint8_t index) {
    static const uint16_t cases[ST_TARGET_EXTRA]={353,438,523,608};
    return cases[index];
}
static uint16_t elapsed_number(uint16_t n) {
    if(n<163u) return n;
    if(n<353u) return (uint16_t)(32u+(n-163u)%128u);
    if(n<609u) return (uint16_t)(419u+(n-353u));
    if(n<2657u) return (uint16_t)(419u+(n-609u));
    return (uint16_t)(2467u+(n-2657u));
}
uint8_t st_integrated(uint16_t n) {
    return (uint8_t)((n>=32u && n<353u) || n>=2657u);
}
void st_prepare(store_case *c,uint16_t n) {
    el_prepare(c,elapsed_number(n));
    if(n>=163u && n<353u) {
        uint16_t k=(uint16_t)(n-163u);
        c->previous.push.base=(agc_word)(0401u+(k/38u)*054u);
        c->previous.push.pushloc=(agc_word)(c->previous.push.base+k%38u);
    }
    if(n>=353u && n<609u) {
        static const agc_word corners[4]={0,077777,037777,040000};
        uint16_t k=(uint16_t)(n-353u);
        c->previous.previous.preparation.mpac[0]=corners[k&3u];
        c->previous.previous.preparation.mpac[1]=corners[(k>>2)&3u];
        c->previous.push.words[30]=corners[(k>>4)&3u];
        c->previous.push.words[31]=corners[(k>>6)&3u];
    }
}
uint8_t st_steps(uint16_t n) { return n>=160u && n<163u ? 4 : 1; }
void st_next(store_case *c,uint16_t n,uint8_t step) {
    el_next(c,elapsed_number(n),step);
}
#ifndef STORE_INPUT_ONLY
uint8_t st_prefix(store_case *c,uint16_t n) {
    el_prefix(c,elapsed_number(n)); return el_apply(c);
}
enum p66_store_status st_apply(store_case *c) {
    return p66_store_elapsed(&c->previous.previous.preparation,&c->previous.push);
}
#endif
