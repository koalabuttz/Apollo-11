/* SPDX-License-Identifier: MIT */
#include "guidance_length_phased.h"
static agc_register add(agc_register a,agc_word b){return agc_add_register(a,agc_extend(b));}
static agc_register indicator(int sign){return sign>0?1:0177776;}
static uint8_t valid_vac(const p66_pushdown *p){return (uint8_t)(p->base==0401||p->base==0455||p->base==0531||p->base==0605||p->base==0661);}
static void double_registers(agc_register_pair *r){
    agc_register lo=agc_add_register(r->l,r->l),hi=agc_add_register(r->a,r->a);int ov=agc_overflow(lo);
    if(ov)hi=agc_add_register(hi,indicator(ov));
    r->a=hi;r->l=agc_extend(agc_correct(lo));
}
static void square(p66_preparation *s){
    agc_register_pair r;agc_dp p;agc_word major=s->mpac[0],minor=s->mpac[1];
    r=agc_multiply(agc_extend(minor),minor);s->mpac[2]=agc_correct(r.a);s->mpac[1]=0;
    r=agc_multiply(agc_extend(minor),major);double_registers(&r);
    p.high=0;p.low=s->mpac[2];agc_double_add(&r,&p);s->mpac[1]=p.high;s->mpac[2]=p.low;
    s->mpac[0]=agc_correct(r.a);r=agc_multiply(agc_extend(major),major);
    p.high=s->mpac[0];p.low=s->mpac[1];agc_double_add(&r,&p);s->mpac[0]=p.high;s->mpac[1]=p.low;s->rate.registers=r;
}
static void correct_overflow(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el){
    agc_register_pair *r=&s->rate.registers;agc_word limit=r->a==1?037777:040000;agc_register a;int ov;
    el->buf=limit;a=agc_add_register(agc_extend(limit),indicator(r->a==1?1:-1));a=add(a,s->mpac[1]);s->mpac[1]=agc_correct(a);ov=agc_overflow(a);a=ov?indicator(ov):0;
    a=add(add(a,limit),s->mpac[0]);s->mpac[0]=agc_correct(a);ov=agc_overflow(a);if(ov){a=indicator(ov);sc->ovfind=agc_correct(a);}r->a=a;
}
static void vector_square(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n,agc_word return_word){
    agc_dp y={s->mpac[3],s->mpac[4]},z={s->mpac[5],s->mpac[6]},low;agc_register_pair *r=&s->rate.registers;int ov;
    d->dvnormct=return_word;square(s);el->buf=s->mpac[0];d->buf_low=s->mpac[1];n->buf_third=s->mpac[2];
    s->mpac[3]=agc_correct(r->a);s->mpac[4]=agc_correct(r->l);s->mpac[0]=y.high;s->mpac[1]=y.low;square(s);
    r->a=agc_extend(s->mpac[1]);r->l=agc_extend(s->mpac[2]);s->mpac[1]=0;s->mpac[2]=0;low.high=d->buf_low;low.low=n->buf_third;agc_double_add(r,&low);d->buf_low=low.high;n->buf_third=low.low;
    r->a=add(add(r->a,s->mpac[0]),el->buf);el->buf=agc_correct(r->a);ov=agc_overflow(r->a);if(ov){r->a=indicator(ov);sc->ovfind=agc_correct(r->a);}
    s->mpac[5]=agc_correct(r->a);s->mpac[6]=0;s->mpac[0]=z.high;s->mpac[1]=z.low;square(s);
    r->a=agc_extend(d->buf_low);r->l=agc_extend(n->buf_third);d->buf_low=0;n->buf_third=0;low.high=s->mpac[1];low.low=s->mpac[2];agc_double_add(r,&low);s->mpac[1]=low.high;s->mpac[2]=low.low;
    r->a=add(add(r->a,s->mpac[0]),el->buf);s->mpac[0]=agc_correct(r->a);ov=agc_overflow(r->a);if(ov){r->a=indicator(ov);correct_overflow(s,sc,el);}
}
static void unnormalize(p66_preparation *s,p66_scale_scratch *sc,p66_norm_scratch *n) {
    uint8_t count=(uint8_t)sc->mptemp;agc_word old;agc_register_pair r;agc_dp low;
    s->rate.registers.a=(agc_register)(count?count-1u:0);
    if(!count) return;
    if(count>=14u) {
        s->rate.registers.l=agc_extend(n->addrwd);n->addrwd=0;
        old=s->mpac[1];s->mpac[1]=s->mpac[0];s->mpac[0]=0;s->mpac[2]=old;
        count=(uint8_t)(count-14u);sc->mptemp=count?(agc_word)(count-1u):077777;
        if(!count) {s->rate.registers.a=0;return;}
    } else s->mpac[2]=0;
    sc->mptemp=(agc_word)(040000u>>count);
    r=agc_multiply(agc_extend(sc->mptemp),s->mpac[2]);s->mpac[2]=agc_correct(r.a);
    old=s->mpac[1];r=agc_multiply(agc_extend(sc->mptemp),s->mpac[0]);s->mpac[0]=agc_correct(r.a);s->mpac[1]=agc_correct(r.l);
    r=agc_multiply(agc_extend(sc->mptemp),old);low.high=s->mpac[1];low.low=s->mpac[2];agc_double_add(&r,&low);
    s->mpac[1]=low.high;s->mpac[2]=low.low;s->rate.registers=r;
}

enum p66_norm_result p66_length_square_phase(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n,p66_pushdown *p){if(s->mpac[7]!=077776||!valid_vac(p))return P66_NORM_OUT_OF_SCOPE;vector_square(s,sc,el,d,n,03202);return P66_NORM_OK;}
enum p66_norm_result p66_length_root_phase(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n,p66_pushdown *p){agc_word old;if(s->mpac[7]!=077776||!valid_vac(p))return P66_NORM_OUT_OF_SCOPE;s->mpac[7]=agc_correct(s->rate.registers.l);old=p->words[35];p->words[34]=s->mpac[0];p->words[35]=s->mpac[1];s->rate.registers.a=agc_extend(p->words[34]);s->rate.registers.l=agc_extend(old);return p66_normalized_square_root(s,sc,el,d,n);}
enum p66_norm_result p66_length_finish_phase(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n,p66_pushdown *p){(void)el;(void)d;if(s->mpac[7]!=0||!valid_vac(p))return P66_NORM_OUT_OF_SCOPE;unnormalize(s,sc,n);return P66_NORM_OK;}
