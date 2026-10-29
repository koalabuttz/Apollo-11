/* SPDX-License-Identifier: MIT */
#include "radial_cases.h"
static uint16_t velocity_number(uint16_t n){return n<1443u?n:(uint16_t)(2451u+n-1443u);}
uint8_t ra_integrated(uint16_t n){return (uint8_t)((n>=16u&&n<147u)||n>=1443u);}
uint8_t ra_steps(uint16_t n){return n>=144u&&n<147u?4:1;}
void ra_prepare(radial_case *c,uint16_t n){
    static const uint8_t offsets[3]={0,3,5};
    p66_preparation *s=&c->previous.previous.previous.previous.previous.previous.preparation;
    p66_pushdown *p=&c->previous.previous.previous.previous.previous.push;
    uint32_t seed=UINT32_C(0x34113373)^((uint32_t)n*UINT32_C(2654435761));unsigned i;
    uint16_t scratch[11];vl_prepare(&c->previous,velocity_number(n));
    for(i=0;i<6u;++i){seed=seed*UINT32_C(1664525)+UINT32_C(1013904223);c->previous.position[i]=(agc_word)((seed>>16)&(i%2u?AGC_MASK:01777u));if(i%2u==0 && (seed&1u))c->previous.position[i]=(agc_word)(c->previous.position[i]^AGC_MASK);}
    for(i=0;i<11u;++i){seed=seed*UINT32_C(1664525)+UINT32_C(1013904223);scratch[i]=(uint16_t)((seed>>16)&AGC_MASK);}
    c->norm.sr=scratch[0];c->norm.cyl=scratch[1];c->norm.addrwd=01520;c->norm.buf_third=scratch[3];c->norm.tem1=scratch[4];for(i=0;i<6u;++i)c->norm.vbuf[i]=scratch[5u+i];
    c->output.hdotdisp.high=scratch[0];c->output.hdotdisp.low=scratch[1];c->output.hcalc1.high=scratch[3];c->output.hcalc1.low=scratch[4];
    /* Seeded lunar-radius test input at B24; the kernel accepts raw LAND. */
    c->land.high=03240;c->land.low=025600;
    if(n<16u || (n>=147u&&n<419u)){
        static const agc_word corner[8]={0,077777,1,077776,02000,075777,037777,040000};
        unsigned k=n<16u?n:n-147u;
        for(i=0;i<6u;++i)c->previous.position[i]=corner[((k>>(2u*(i%4u)))+i)&7u];
        if(n<8u){for(i=0;i<6u;++i)c->previous.position[i]=0;c->previous.position[2u*(n%3u)]=corner[n];}
    }
    if(!ra_integrated(n)){
        p->pushloc=p->base;s->mpac[7]=077776;
        for(i=0;i<3u;++i){s->mpac[offsets[i]]=c->previous.position[2u*i];s->mpac[offsets[i]+1u]=c->previous.position[2u*i+1u];}
        for(i=0;i<6u;++i)p->words[24u+i]=c->previous.velocity[i];
    }
}
void ra_next(radial_case *c,uint16_t n,uint8_t step){if(ra_integrated(n))vl_next(&c->previous,velocity_number(n),step);}
void ra_words(const radial_case *c,uint16_t w[RA_WORDS]){unsigned i;vl_words(&c->previous,w);w[117]=c->norm.sr;w[118]=c->norm.cyl;w[119]=c->norm.addrwd;w[120]=c->norm.buf_third;w[121]=c->norm.tem1;for(i=0;i<6u;++i)w[122+i]=c->norm.vbuf[i];w[128]=c->output.hdotdisp.high;w[129]=c->output.hdotdisp.low;w[130]=c->output.hcalc1.high;w[131]=c->output.hcalc1.low;w[132]=c->land.high;w[133]=c->land.low;}
void ra_from_words(radial_case *c,const uint16_t w[RA_WORDS]){unsigned i;vl_from_words(&c->previous,w);c->norm.sr=w[117];c->norm.cyl=w[118];c->norm.addrwd=w[119];c->norm.buf_third=w[120];c->norm.tem1=w[121];for(i=0;i<6u;++i)c->norm.vbuf[i]=w[122+i];c->output.hdotdisp.high=w[128];c->output.hdotdisp.low=w[129];c->output.hcalc1.high=w[130];c->output.hcalc1.low=w[131];c->land.high=w[132];c->land.low=w[133];}
#ifndef RADIAL_INPUT_ONLY
#ifndef RADIAL_C64_FROZEN_ENTRY
void ra_prefix(radial_case *c,uint16_t n){vl_prefix(&c->previous,velocity_number(n));(void)vl_apply(&c->previous);c->norm.addrwd=01520;}
#endif
enum p66_radial_status ra_apply(radial_case *c){return p66_radial_position(&c->previous.previous.previous.previous.previous.previous.preparation,&c->previous.previous.previous.previous.previous.previous.scratch,&c->previous.previous.previous.previous.elapsed,&c->previous.previous.previous.scratch,&c->norm,&c->previous.previous.previous.previous.previous.push,&c->output,&c->land);}
#endif
