/* SPDX-License-Identifier: MIT */
#include "rotation_cases.h"
static agc_word next(uint32_t *seed){*seed=*seed*UINT32_C(1664525)+UINT32_C(1013904223);return (agc_word)((*seed>>8)&AGC_MASK);}
void rot_input(rotation_case *c,uint16_t n){
    static const agc_word edge[8]={0,077777,1,077776,020000,057777,037777,040000};
    p66_preparation *s=&c->previous.base.previous.previous.previous.preparation;
    uint32_t seed=UINT32_C(0x23a3675)+(uint32_t)n;uint8_t i;
    unit_input(&c->previous,(uint32_t)(n/2u)*3u);
    s->mpac[7]=(agc_word)((n>>1)%3u==0u?0u:(n>>1)%3u==1u?077776u:next(&seed));
    c->trig.buf2.high=next(&seed);c->trig.buf2.low=next(&seed);c->trig.polish=next(&seed);
    c->trig.tem2=next(&seed);c->trig.tem3=next(&seed);
    for(i=0;i<6u;++i){c->trig.cduspot[i]=next(&seed);c->trig.sin[i]=next(&seed);c->trig.cos[i]=next(&seed);}
    c->rotation.dex2=next(&seed);c->rotation.rtnsaver=next(&seed);
    c->direction=(agc_word)((n&1u)?077774:3);c->return_word=(agc_word)(next(&seed)&07777u);
    for(i=0;i<6u;++i)c->previous.norm.vbuf[i]=next(&seed);
    if(n<4096u){
        for(i=0;i<6u;++i){c->previous.norm.vbuf[i]=edge[((uint32_t)n>>(2u*(i%5u)))&7u];c->trig.sin[i]=edge[((uint32_t)n>>(3u*(i%4u)))&7u];c->trig.cos[i]=edge[((uint32_t)n>>(2u*(i%3u)))&7u];}
    }else if(n<8192u){
        for(i=0;i<3u;++i){c->trig.sin[2u*i]=0;c->trig.sin[2u*i+1u]=0;c->trig.cos[2u*i]=020000;c->trig.cos[2u*i+1u]=0;c->previous.norm.vbuf[2u*i]&=03777u;}
    }else if(n<12288u){
        for(i=0;i<3u;++i){c->trig.sin[2u*i]=020000;c->trig.sin[2u*i+1u]=0;c->trig.cos[2u*i]=0;c->trig.cos[2u*i+1u]=0;c->previous.norm.vbuf[2u*i]&=03777u;}
    }
}
void rot_words(const rotation_case *c,uint16_t w[ROT_WORDS]){
    uint8_t i;unit_words(&c->previous,w);w[104]=c->trig.buf2.high;w[105]=c->trig.buf2.low;w[106]=c->trig.polish;w[107]=c->trig.tem2;w[108]=c->trig.tem3;
    for(i=0;i<6u;++i){w[109u+i]=c->trig.cduspot[i];w[115u+i]=c->trig.sin[i];w[121u+i]=c->trig.cos[i];}
    w[127]=c->rotation.dex2;w[128]=c->rotation.rtnsaver;w[129]=c->direction;w[130]=c->return_word;
}
#ifndef ROTATION_INPUT_ONLY
unsigned rot_apply(rotation_case *c){return p66_axis_rotate(&c->previous.base.previous.previous.previous.preparation,&c->previous.base.previous.previous.previous.scratch,&c->previous.base.previous.elapsed,&c->previous.base.scratch,&c->previous.norm,&c->trig,&c->rotation,c->direction,c->return_word);}
#endif
