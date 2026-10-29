/* SPDX-License-Identifier: MIT */
#include "trig_cases.h"
static uint16_t random_word(uint32_t *r){*r=*r*UINT32_C(1664525)+UINT32_C(1013904223);return (uint16_t)((*r>>16)&AGC_MASK);}
uint8_t tg_operation(uint32_t n){return (uint8_t)(n<32768u?n%4u:0u);}
void tg_input(trig_case *c,uint32_t n){uint32_t k=n/4u,r=k^UINT32_C(0x35303663);unsigned i;uint8_t op=tg_operation(n);uint16_t w[UNIT_WORDS];unit_input(&c->previous,(k%16896u)*3u);unit_words(&c->previous,w);w[32]=(k&1u)?077776:0;if(op==3u)w[32]=077776;
    if(k<4096u){static const uint16_t edge[16]={0,077777,1,077776,07777,010000,010001,067776,020000,057777,017777,020001,037777,040000,040001,037776};w[25]=edge[k&15u];w[26]=edge[(k>>4)&15u];w[27]=edge[(k>>8)&15u];}
    else{w[25]=random_word(&r);w[26]=random_word(&r);w[27]=random_word(&r);}if(n>=32768u)w[25]=(uint16_t)(n-32768u);dv_from_words(&c->previous.base,w);
    c->trig.buf2.high=random_word(&r);c->trig.buf2.low=random_word(&r);c->trig.polish=random_word(&r);c->trig.tem2=random_word(&r);c->trig.tem3=random_word(&r);for(i=0;i<6u;++i){c->trig.cduspot[i]=random_word(&r);c->trig.sin[i]=random_word(&r);c->trig.cos[i]=random_word(&r);}for(i=0;i<3u;++i)c->cdu[i]=random_word(&r);if(k<4096u){static const agc_word hardware_edge[16]={0,077777,1,077776,07777,010000,010001,067776,020000,057777,017777,020001,037777,040000,040001,037776};for(i=0;i<3u;++i)c->cdu[i]=hardware_edge[(k>>(4u*i))&15u];}
}
void tg_words(const trig_case *c,uint16_t w[TG_WORDS]){unsigned i;unit_words(&c->previous,w);w[104]=c->trig.buf2.high;w[105]=c->trig.buf2.low;w[106]=c->trig.polish;w[107]=c->trig.tem2;w[108]=c->trig.tem3;for(i=0;i<6u;++i){w[109+i]=c->trig.cduspot[i];w[115+i]=c->trig.sin[i];w[121+i]=c->trig.cos[i];}for(i=0;i<3u;++i)w[127+i]=c->cdu[i];}
#ifndef TRIG_INPUT_ONLY
unsigned tg_apply(trig_case *c,uint8_t op){p66_preparation *s=&c->previous.base.previous.previous.previous.preparation;p66_scale_scratch *sc=&c->previous.base.previous.previous.previous.scratch;p66_divide_scratch *d=&c->previous.base.scratch;p66_norm_scratch *n=&c->previous.norm;if(op==0u)return p66_cdu_logic(s);if(op==1u)return p66_sine(s,sc,d,n,&c->trig);if(op==2u)return p66_cosine(s,sc,d,n,&c->trig);return p66_cdu_trig(s,sc,d,n,&c->trig,c->cdu,03663,(agc_word)(046000u+((uint32_t)c->previous.base.previous.previous.previous.preparation.mpac[2]&7u)));}
#endif
