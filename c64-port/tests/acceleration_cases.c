/* SPDX-License-Identifier: MIT */
#include "acceleration_cases.h"
static agc_word next(uint32_t *seed){*seed=*seed*UINT32_C(1664525)+UINT32_C(1013904223);return (agc_word)((*seed>>8)&AGC_MASK);}
void acc_input(acceleration_case *c,uint16_t n){
    static const agc_word edge[8]={0,077777,1,077776,02000,075777,037777,040000};
    static const agc_dp times[12]={{0,0},{077777,077777},{0,1},{0,077776},{1,0},{077776,077777},{037777,037777},{040000,040000},{0,010000},{040000,1},{02000,0},{0100,012345}};
    p66_preparation *s=&c->previous.base.previous.previous.previous.preparation;
    p66_pushdown *p=&c->previous.base.previous.previous.push;
    uint32_t seed=UINT32_C(0xa66c3411)+(uint32_t)n;uint8_t i;
    unit_input(&c->previous,(uint32_t)n*3u);s->mpac[7]=0;p->pushloc=p->base;c->previous.norm.addrwd=01473;
    c->taurod.high=next(&seed);c->taurod.low=next(&seed);
    for(i=0;i<6u;++i)c->gravity[i]=next(&seed);
    if(n<4096u){
        c->taurod=times[n%12u];
        s->mpac[0]=edge[(n>>3)&7u];s->mpac[1]=edge[(n>>6)&7u];
        s->rate.desired.high=edge[(n>>9)&7u];s->rate.desired.low=edge[n&7u];
        for(i=0;i<6u;++i)c->gravity[i]=edge[(((uint32_t)n>>(3u*(i%4u)))+i)&7u];
    }else if(n<8192u){
        c->taurod.high=(agc_word)(1u+(n%0177u));c->taurod.low=next(&seed);
        s->mpac[0]&=077u;s->rate.desired.high&=077u;
        for(i=0;i<3u;++i)c->gravity[2u*i]&=017u;
    }
}
void acc_words(const acceleration_case *c,uint16_t w[ACC_WORDS]){
    uint8_t i;unit_words(&c->previous,w);w[104]=c->taurod.high;w[105]=c->taurod.low;
    for(i=0;i<6u;++i)w[106u+i]=c->gravity[i];
}
#ifndef ACCELERATION_INPUT_ONLY
unsigned acc_apply(acceleration_case *c){
    return (unsigned)p66_acceleration(&c->previous.base.previous.previous.previous.preparation,
        &c->previous.base.previous.previous.previous.scratch,&c->previous.base.previous.elapsed,
        &c->previous.base.scratch,&c->previous.norm,&c->previous.base.previous.previous.push,
        &c->taurod,c->gravity);
}
#endif
