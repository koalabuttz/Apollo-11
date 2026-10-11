/* SPDX-License-Identifier: MIT */
#include "guidance_shift.h"
static agc_register indicator(int overflow) { return overflow>0 ? 1 : agc_extend(077776); }
static uint8_t valid(const p66_preparation *s,uint8_t count,uint8_t short_form,uint8_t vector) {
    return (uint8_t)(s->mpac[7]==(vector?077776u:0u)&&short_form<=1u&&
      (short_form ? count>=1u&&count<=(vector?8u:4u) : count<=14u));
}
static void overflow(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,uint8_t offset) {
    agc_register_pair *r=&s->rate.registers;
    agc_word limit=r->a==1u?037777:040000;
    agc_register a;
    int ov;
    el->buf=limit;r->l=offset;
    a=agc_add_register(agc_extend(limit),indicator(r->a==1u?1:-1));
    a=agc_add_register(a,agc_extend(s->mpac[offset+1u]));
    s->mpac[offset+1u]=agc_correct(a);ov=agc_overflow(a);a=ov?indicator(ov):0;
    a=agc_add_register(a,agc_extend(limit));a=agc_add_register(a,agc_extend(s->mpac[offset]));
    s->mpac[offset]=agc_correct(a);ov=agc_overflow(a);
    if(ov) { a=indicator(ov);sc->ovfind=agc_correct(a); }r->a=a;
}
uint8_t p66_scalar_shift_left(p66_preparation *s,p66_scale_scratch *sc,uint8_t count,uint8_t short_form) {
    agc_register_pair *r=&s->rate.registers;uint8_t i;
    if(!valid(s,count,short_form,0)) return 0;
    if(!count) {r->a=0;return 1;}
    for(i=0;i<count;++i) {
        agc_dp low={s->mpac[1],s->mpac[2]};int ov;
        r->a=agc_extend(low.high);r->l=agc_extend(low.low);agc_double_add(r,&low);
        s->mpac[1]=low.high;s->mpac[2]=low.low;
        r->a=agc_add_register(r->a,agc_extend(s->mpac[0]));
        r->a=agc_add_register(r->a,agc_extend(s->mpac[0]));
        s->mpac[0]=agc_correct(r->a);ov=agc_overflow(r->a);
        if(ov) {r->a=indicator(ov);sc->ovfind=agc_correct(r->a);}
    }
    sc->mptemp=0;r->a=short_form?(agc_register)(027777u-count):7;return 1;
}
uint8_t p66_scalar_shift_right(p66_preparation *s,p66_scale_scratch *sc,uint8_t count,uint8_t short_form) {
    agc_register_pair *r=&s->rate.registers;agc_register_pair product;agc_dp low;
    agc_word old_low;
    if(!valid(s,count,short_form,0)) return 0;
    if(!count) {r->a=0;return 1;}
    if(count==14u) {
        s->mpac[2]=s->mpac[1];s->mpac[1]=s->mpac[0];s->mpac[0]=0;
        /* GENSHFT2's count/direction multiply leaves its low product in L;
         * the whole-word GENSCR path contains no later instruction writing L. */
        sc->mptemp=077777;r->a=0;r->l=03400;return 1;
    }
    sc->mptemp=(agc_word)(UINT16_C(040000)>>count);
    product=agc_multiply(agc_extend(sc->mptemp),s->mpac[2]);s->mpac[2]=agc_correct(product.a);
    old_low=s->mpac[1];product=agc_multiply(agc_extend(sc->mptemp),s->mpac[0]);
    s->mpac[0]=agc_correct(product.a);s->mpac[1]=agc_correct(product.l);
    *r=agc_multiply(agc_extend(sc->mptemp),old_low);
    low.high=s->mpac[1];low.low=s->mpac[2];agc_double_add(r,&low);
    s->mpac[1]=low.high;s->mpac[2]=low.low;return 1;
}
uint8_t p66_vector_shift_left(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,uint8_t count,uint8_t short_form) {
    static const uint8_t offsets[3]={0,3,5};agc_register_pair *r=&s->rate.registers;
    uint8_t i,j;
    if(!valid(s,count,short_form,1)) return 0;
    if(!count) {r->a=0;return 1;}
    for(i=0;i<count;++i) for(j=0;j<3u;++j) {
        uint8_t offset=offsets[j];agc_dp value={s->mpac[offset],s->mpac[offset+1u]};
        r->a=agc_extend(value.high);r->l=agc_extend(value.low);agc_double_add(r,&value);
        s->mpac[offset]=value.high;s->mpac[offset+1u]=value.low;
        if(r->a) overflow(s,sc,el,offset);
    }
    sc->mptemp=0;r->a=0;return 1;
}
static void rounded_component(p66_preparation *s,p66_scale_scratch *sc,uint8_t offset) {
    agc_register_pair *r=&s->rate.registers;agc_register twice;int ov;agc_dp result;
    *r=agc_multiply(agc_extend(sc->mptemp),s->mpac[offset+1u]);
    s->mpac[offset+1u]=agc_correct(r->a);
    twice=agc_add_register(r->l,r->l);s->mpac[2]=agc_correct(twice);ov=agc_overflow(twice);
    if(ov) s->mpac[offset+1u]=agc_correct(agc_add_register(indicator(ov),agc_extend(s->mpac[offset+1u])));
    s->mpac[2]=0;*r=agc_multiply(agc_extend(s->mpac[offset]),sc->mptemp);
    result.high=0;result.low=s->mpac[offset+1u];agc_double_add(r,&result);
    s->mpac[offset]=result.high;s->mpac[offset+1u]=result.low;
}
uint8_t p66_vector_shift_right(p66_preparation *s,p66_scale_scratch *sc,uint8_t count,uint8_t short_form) {
    static const uint8_t offsets[3]={0,3,5};agc_register_pair *r=&s->rate.registers;uint8_t j;
    if(!valid(s,count,short_form,1)) return 0;
    if(!count) {r->a=0;return 1;}
    if(count<14u) {
        sc->mptemp=(agc_word)(UINT16_C(040000)>>count);
        for(j=0;j<3u;++j) rounded_component(s,sc,offsets[j]);
    } else {
        for(j=0;j<3u;++j) {
            uint8_t offset=offsets[j];agc_register twice=agc_add_register(agc_extend(s->mpac[offset+1u]),agc_extend(s->mpac[offset+1u]));
            int ov=agc_overflow(twice);agc_dp value={0,s->mpac[offset]};
            s->mpac[2]=agc_correct(twice);r->a=0;r->l=ov?indicator(ov):0;agc_double_add(r,&value);
            s->mpac[offset]=value.high;s->mpac[offset+1u]=value.low;
        }
        sc->mptemp=077777;
    }
    return 1;
}
