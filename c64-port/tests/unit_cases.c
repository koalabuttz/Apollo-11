/* SPDX-License-Identifier: MIT */
#include "unit_cases.h"
static uint16_t random_word(uint32_t *r){*r=*r*1664525u+1013904223u;return (uint16_t)((*r>>8)&077777u);}
void unit_input(unit_case *c,uint32_t index){
    static const uint16_t signs[4]={0,077776,077777,1},flags[4]={0,1,077776,077777};
    static const uint16_t base[5]={0401,0455,0531,0605,0661},edge[8]={0,077777,1,077776,010000,020000,037777,040000},positions[3]={25,28,30};
    uint16_t w[DV_WORD_COUNT];uint32_t k=index/3u,r=k+0xc01231u;unsigned i;
    for(i=0;i<DV_WORD_COUNT;++i)w[i]=random_word(&r);
    w[4]=(uint16_t)(w[4]|((w[4]&040000u)<<1));w[5]=(uint16_t)(w[5]|((w[5]&040000u)<<1));
    w[90]=signs[k&3u];w[40]=flags[(k>>2)&3u];
    w[32]=index%3u?077776:0;w[41]=base[k%5u];w[42]=(uint16_t)(w[41]+k%44u);
    if(k<512u){for(i=0;i<3u;++i){unsigned pos=positions[i];w[pos]=edge[(k>>(3u*i))&7u];w[pos+1u]=(w[pos]&040000u)?077777:0;}}
    else if(k<1024u){for(i=0;i<3u;++i){unsigned pos=positions[i];w[pos]=0;w[pos+1u]=edge[((k-512u)>>(3u*i))&7u];}}
    else if(k<3072u){for(i=0;i<3u;++i){unsigned pos=positions[i];w[pos]&=07777u;if(k&1u){w[pos]^=077777u;w[pos+1u]^=077777u;}}}
    if(k>=8192u){
        for(i=0;i<3u;++i){unsigned pos=positions[i];uint16_t mag;
            if(k<10240u)mag=(uint16_t)(i?0:k-8192u+1u);
            else if(k<12288u)mag=(uint16_t)(random_word(&r)&07777u);
            else if(k<14336u)mag=(uint16_t)(random_word(&r)&017777u);
            else mag=(uint16_t)(random_word(&r)&037777u);
            w[pos]=0;w[pos+1u]=mag;
            if((k>>(i+1u))&1u)w[pos+1u]^=077777u;
            if(k>=12288u&&k<14336u){w[pos]=(uint16_t)((k>>3)&1u);if((k>>(i+1u))&1u)w[pos]^=077777u;}
        }
    }
    if(k>=16384u){
        for(i=0;i<3u;++i){unsigned pos=positions[i];w[pos]=(uint16_t)(i?0:k-16384u+1u);w[pos+1u]=0;if(k&1u){w[pos]^=077777u;w[pos+1u]^=077777u;}}
    }
    if(k>=16896u){
        static const uint16_t regression[16][6]={
            {040000,1,1,077776,077776,037777},
            {1,040000,077776,077776,037777,077776},
            {040000,1,0,02000,077776,037777},
            {1,040000,077777,02000,037777,077776},
            {040000,1,1,075777,077776,037777},
            {1,040000,077776,075777,037777,077776},
            {040000,1,0,037777,077776,037777},
            {1,040000,077777,037777,037777,077776},
            {1,077777,1,040000,037777,075777},
            {077776,077777,1,040000,040000,075777},
            {037777,1,1,040000,1,037777},
            {040000,1,1,040000,077776,037777},
            {1,077776,1,040000,037777,040000},
            {077776,077776,1,040000,040000,040000},
            {037777,02000,1,040000,1,0},
            {040000,02000,1,040000,077776,0}
        };
        for(i=0;i<3u;++i){w[positions[i]]=regression[k-16896u][2u*i];w[positions[i]+1u]=regression[k-16896u][2u*i+1u];}
    }
    dv_from_words(&c->base,w);c->norm.sr=random_word(&r);c->norm.cyl=random_word(&r);c->norm.addrwd=random_word(&r);c->norm.buf_third=random_word(&r);c->norm.tem1=random_word(&r);for(i=0;i<6u;++i)c->norm.vbuf[i]=random_word(&r);
}
void unit_words(const unit_case *c,uint16_t w[UNIT_WORDS]){
    unsigned i;dv_words(&c->base,w);w[93]=c->norm.sr;w[94]=c->norm.cyl;w[95]=c->norm.addrwd;w[96]=c->norm.buf_third;w[97]=c->norm.tem1;for(i=0;i<6u;++i)w[98+i]=c->norm.vbuf[i];
}
#ifndef UNIT_INPUT_ONLY
unsigned unit_apply(unit_case *c,uint8_t op){
    p66_preparation *s=&c->base.previous.previous.previous.preparation;p66_scale_scratch *sc=&c->base.previous.previous.previous.scratch;
    if(!op)return (unsigned)p66_absolute_value(s);
    if(op==1u)return (unsigned)p66_vector_length(s,sc,&c->base.previous.elapsed,&c->base.scratch,&c->norm,&c->base.previous.previous.push);
    return (unsigned)p66_unit_vector(s,sc,&c->base.previous.elapsed,&c->base.scratch,&c->norm,&c->base.previous.previous.push);
}
#endif
