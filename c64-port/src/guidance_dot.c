/* SPDX-License-Identifier: MIT */
#include "guidance_dot.h"
#include "guidance_product.h"
static agc_register indicator(int overflow) { return overflow>0?1:agc_extend(077776); }
static void correct_overflow(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el) {
    agc_register_pair *r=&s->rate.registers;agc_word limit=r->a==1?037777:040000;
    agc_register a;int overflow;
    el->buf=limit;
    a=agc_add_register(agc_extend(limit),indicator(r->a==1?1:-1));
    a=agc_add_register(a,agc_extend(s->mpac[1]));s->mpac[1]=agc_correct(a);
    overflow=agc_overflow(a);a=overflow?indicator(overflow):0;
    a=agc_add_register(a,agc_extend(limit));a=agc_add_register(a,agc_extend(s->mpac[0]));
    s->mpac[0]=agc_correct(a);overflow=agc_overflow(a);
    if(overflow) {a=indicator(overflow);sc->ovfind=agc_correct(a);}r->a=a;
}
uint8_t p66_dot(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,
               p66_divide_scratch *d,agc_word *buf_third,const agc_word operand[6]) {
    agc_register_pair *r=&s->rate.registers;
    agc_dp y,z,factor,low;int overflow;
    if(s->mpac[7]!=077776) return 0;
    y.high=s->mpac[3];y.low=s->mpac[4];z.high=s->mpac[5];z.low=s->mpac[6];
    d->dvsign=2;d->dvnormct=07332;
    /* DMPSUB itself is independent of MODE. The exclusively owned native
     * calculation uses its scalar entry while constructing DOT's final DP
     * mode, with no intermediate observation or dispatch. */
    s->mpac[7]=0;factor.high=operand[0];factor.low=operand[1];
    (void)p66_scalar_product(s,sc,&factor);
    el->buf=s->mpac[0];d->buf_low=s->mpac[1];*buf_third=s->mpac[2];
    s->mpac[3]=0;s->mpac[4]=0;s->mpac[0]=y.high;s->mpac[1]=y.low;
    factor.high=operand[2];factor.low=operand[3];(void)p66_scalar_product(s,sc,&factor);
    r->a=agc_extend(s->mpac[1]);r->l=agc_extend(s->mpac[2]);s->mpac[1]=0;s->mpac[2]=0;
    low.high=d->buf_low;low.low=*buf_third;agc_double_add(r,&low);
    d->buf_low=low.high;*buf_third=low.low;
    r->a=agc_add_register(r->a,agc_extend(s->mpac[0]));r->a=agc_add_register(r->a,agc_extend(el->buf));
    el->buf=agc_correct(r->a);overflow=agc_overflow(r->a);
    if(overflow) {r->a=indicator(overflow);sc->ovfind=agc_correct(r->a);}
    s->mpac[5]=agc_correct(r->a);s->mpac[6]=0;s->mpac[0]=z.high;s->mpac[1]=z.low;
    factor.high=operand[4];factor.low=operand[5];(void)p66_scalar_product(s,sc,&factor);
    r->a=agc_extend(d->buf_low);r->l=agc_extend(*buf_third);d->buf_low=0;*buf_third=0;
    low.high=s->mpac[1];low.low=s->mpac[2];agc_double_add(r,&low);
    s->mpac[1]=low.high;s->mpac[2]=low.low;
    r->a=agc_add_register(r->a,agc_extend(s->mpac[0]));r->a=agc_add_register(r->a,agc_extend(el->buf));
    s->mpac[0]=agc_correct(r->a);overflow=agc_overflow(r->a);
    if(overflow) {r->a=indicator(overflow);correct_overflow(s,sc,el);}
    r->a=0;return 1;
}
