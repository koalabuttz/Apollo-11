/* SPDX-License-Identifier: MIT */
#include "scale_cases.h"
static uint16_t pp_number(uint16_t n) {
    return n<280u ? (uint16_t)(n-152u) : n<283u ? (uint16_t)(n-136u)
           : (uint16_t)(n-2331u+147u);
}
uint8_t sc_integrated(uint16_t n) {
    return (uint8_t)((n>=152u && n<283u) || n>=2331u);
}
void sc_prepare(scale_case *c,uint16_t n) {
    static const uint16_t edges[12]={0,077777,1,077776,2,077775,
                                  017777,060000,020000,057777,037777,040000};
    unsigned i;
    if(sc_integrated(n)) pp_prepare(&c->preparation,pp_number(n));
    else {
        pp_prepare(&c->preparation,(uint16_t)(147u+n));
        c->preparation.mpac[7]=077776;
        if(n<144u) {
            static const uint8_t p[3]={0,3,5};
            for(i=0;i<3;++i) {
                c->preparation.mpac[p[i]]=edges[(n/12u+i*3u)%12u];
                c->preparation.mpac[p[i]+1u]=edges[(n%12u+i*5u)%12u];
            }
        } else if(n<152u) {
            /* Inputs selected with integer partial-product arithmetic;
             * expectations still come only from the original assembly.
             * Below/exactly/above half and low-word rounding propagation. */
            static const uint16_t high[4]={74,80,86,78};
            static const uint16_t low[4]={3248,5569,7265,2009};
            static const uint8_t p[3]={0,3,5};
            uint16_t index=(uint16_t)((n-144u)%4u);
            for(i=0;i<3;++i) {
                c->preparation.mpac[p[i]]=(uint16_t)(high[index] ^ (n>=148u ? AGC_MASK : 0));
                c->preparation.mpac[p[i]+1u]=(uint16_t)(low[index] ^ (n>=148u ? AGC_MASK : 0));
            }
        }
    }
    c->scratch.mptemp=(uint16_t)((012345u+n)&AGC_MASK);
    c->scratch.ovfind=(uint16_t)((054321u+n)&AGC_MASK);
}
uint8_t sc_steps(uint16_t n) { return n>=280u && n<283u ? 4 : 1; }
void sc_next(scale_case *c,uint16_t n,uint8_t step) {
    if(sc_integrated(n)) pp_next(&c->preparation,pp_number(n),step);
}
void sc_words(const scale_case *c,uint16_t w[SC_WORD_COUNT]) {
    pp_words(&c->preparation,w); w[39]=c->scratch.mptemp; w[40]=c->scratch.ovfind;
}
void sc_from_words(scale_case *c,const uint16_t w[SC_WORD_COUNT]) {
    pp_from_words(&c->preparation,w); c->scratch.mptemp=w[39]; c->scratch.ovfind=w[40];
}
#ifndef SCALE_INPUT_ONLY
void sc_apply(scale_case *c,uint16_t n) {
    if(sc_integrated(n)) (void)p66_prepare(&c->preparation);
    p66_scale_velocity(&c->preparation,&c->scratch);
}
#endif
