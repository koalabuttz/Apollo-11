/* SPDX-License-Identifier: MIT */
#include "throttle_feedback_cases.h"
static agc_word next(uint32_t *seed){*seed=*seed*UINT32_C(1664525)+UINT32_C(1013904223);return (agc_word)((*seed>>8)&AGC_MASK);}
void tf_input(throttle_feedback_case *c,uint16_t n){
    static const agc_word edge[8]={0,077777,1,077776,020000,057777,037777,040000};
    agc_dp *inputs[7]={&c->input.mass,&c->input.lag_tau,&c->input.maxforce,&c->input.minforce,&c->feedback.afc,&c->feedback.fweight,&c->feedback.lasttpip};
    p66_preparation *s=&c->previous.base.previous.previous.previous.preparation;p66_pushdown *p=&c->previous.base.previous.previous.push;
    uint32_t seed=UINT32_C(0x66f3433)+(uint32_t)n;uint8_t i;
    unit_input(&c->previous,(uint32_t)n*3u);s->mpac[7]=077776;p->pushloc=p->base;c->previous.norm.addrwd=01766;
    for(i=0;i<7u;++i){inputs[i]->high=next(&seed);inputs[i]->low=next(&seed);}
    for(i=0;i<6u;++i)c->bias[i]=next(&seed);
    if(n<4096u){
        for(i=0;i<7u;++i){inputs[i]->high=edge[((uint32_t)n>>(2u*(i%4u)))&7u];inputs[i]->low=edge[(((uint32_t)n>>(3u*(i%3u)))+i)&7u];}
        p->words[20]=edge[(n>>3)&7u];p->words[21]=edge[(n>>6)&7u];p->words[22]=edge[(n>>9)&7u];p->words[23]=edge[n&7u];
    }else if(n<8192u){
        c->input.mass.high=(agc_word)(01000u+(n%0400u));c->input.mass.low=0;c->input.lag_tau.high=01000;c->input.lag_tau.low=0;
        c->input.maxforce.high=01000;c->input.maxforce.low=0;c->input.minforce.high=0100;c->input.minforce.low=0;
        c->feedback.lasttpip.high=s->this_piptime.high;c->feedback.lasttpip.low=(agc_word)(((uint32_t)s->this_piptime.low+077777u-0100u)&AGC_MASK);
        c->feedback.afc.high=0100;c->feedback.afc.low=0;c->feedback.fweight.high=0;c->feedback.fweight.low=0;
        p->words[20]=02000;p->words[21]=0;p->words[22]=020000;p->words[23]=0;
        for(i=0;i<6u;++i)c->bias[i]=0;
    }
}
void tf_words(const throttle_feedback_case *c,uint16_t w[TF_WORDS]){uint8_t i;unit_words(&c->previous,w);w[104]=c->input.mass.high;w[105]=c->input.mass.low;w[106]=c->input.lag_tau.high;w[107]=c->input.lag_tau.low;w[108]=c->input.maxforce.high;w[109]=c->input.maxforce.low;w[110]=c->input.minforce.high;w[111]=c->input.minforce.low;w[112]=c->feedback.afc.high;w[113]=c->feedback.afc.low;w[114]=c->feedback.fweight.high;w[115]=c->feedback.fweight.low;w[116]=c->feedback.lasttpip.high;w[117]=c->feedback.lasttpip.low;for(i=0;i<6u;++i)w[118u+i]=c->bias[i];}
#ifndef THROTTLE_FEEDBACK_INPUT_ONLY
unsigned tf_apply(throttle_feedback_case *c){return (unsigned)p66_throttle_feedback_compute(&c->previous.base.previous.previous.previous.preparation,&c->previous.base.previous.previous.previous.scratch,&c->previous.base.previous.elapsed,&c->previous.base.scratch,&c->previous.norm,&c->previous.base.previous.previous.push,&c->feedback,&c->input,c->bias);}
#endif
