/* SPDX-License-Identifier: MIT */
#include "guidance_trig.h"
#include "guidance_product.h"
static agc_register add(agc_register a,agc_word b){return agc_add_register(a,agc_extend(b));}
static agc_register indicator(int overflow){return overflow>0?1:0177776;}
static uint8_t supported(const p66_preparation *s){return (uint8_t)(s->mpac[7]==0||s->mpac[7]==077776);}
static void doubled(agc_register_pair *r){agc_register hi=agc_add_register(r->a,r->a),lo=agc_add_register(r->l,r->l);int ov=agc_overflow(lo);if(ov)hi=agc_add_register(hi,indicator(ov));r->a=hi;r->l=agc_extend(agc_correct(lo));}
static void square(p66_preparation *s){agc_register_pair r;agc_dp p;agc_word hi=s->mpac[0],lo=s->mpac[1];r=agc_multiply(agc_extend(lo),lo);s->mpac[2]=agc_correct(r.a);s->mpac[1]=0;r=agc_multiply(agc_extend(lo),hi);doubled(&r);p.high=0;p.low=s->mpac[2];agc_double_add(&r,&p);s->mpac[1]=p.high;s->mpac[2]=p.low;s->mpac[0]=agc_correct(r.a);r=agc_multiply(agc_extend(hi),hi);p.high=s->mpac[0];p.low=s->mpac[1];agc_double_add(&r,&p);s->mpac[0]=p.high;s->mpac[1]=p.low;s->rate.registers=r;}
static void multiply(p66_preparation *s,p66_scale_scratch *sc,const agc_dp *p){agc_word mode=s->mpac[7];s->mpac[7]=0;(void)p66_scalar_product(s,sc,p);s->mpac[7]=mode;}
static void triple_double(p66_preparation *s){agc_dp low={s->mpac[1],s->mpac[2]};agc_register_pair *r=&s->rate.registers;r->a=agc_extend(low.high);r->l=agc_extend(low.low);agc_double_add(r,&low);s->mpac[1]=low.high;s->mpac[2]=low.low;r->a=add(add(r->a,s->mpac[0]),s->mpac[0]);s->mpac[0]=agc_correct(r->a);}
uint8_t p66_sine(p66_preparation *s,p66_scale_scratch *sc,p66_divide_scratch *d,p66_norm_scratch *n,p66_trig_scratch *t){
    static const agc_dp coefficient[5]={{014441,037325},{053250,060764},{012146,021276},{075466,071471},{00236,032757}};
    agc_register_pair r;agc_dp x,result;agc_register twice;int ov;uint8_t i;
    if(!supported(s))return 0;
    r.a=agc_extend(s->mpac[0]);r.l=agc_extend(s->mpac[1]);doubled(&r);
    if(agc_overflow(r.a)){r.a=(agc_register)(r.a^0177777u);r.l=(agc_register)(r.l^0177777u);}
    s->mpac[0]=agc_correct(r.a);s->mpac[1]=agc_correct(r.l);
    twice=agc_add_register(agc_extend(s->mpac[0]),agc_extend(s->mpac[0]));ov=agc_overflow(twice);
    if(ov){agc_register bound=ov>0?040000:0137777;s->mpac[0]=agc_correct(add(bound,agc_negate(s->mpac[0])));s->mpac[1]=agc_negate(s->mpac[1]);}
    t->buf2.high=s->mpac[0];t->buf2.low=s->mpac[1];square(s);x.high=s->mpac[0];x.low=s->mpac[1];n->vbuf[0]=x.high;n->vbuf[1]=x.low;
    s->mpac[0]=coefficient[4].high;s->mpac[1]=coefficient[4].low;n->addrwd=0122;n->tem1=03573;t->polish=03566;d->maxdvsw=3;
    for(i=4;i>0;--i){multiply(s,sc,&x);r.a=agc_extend(coefficient[i-1u].high);r.l=agc_extend(coefficient[i-1u].low);result.high=s->mpac[0];result.low=s->mpac[1];agc_double_add(&r,&result);s->mpac[0]=result.high;s->mpac[1]=result.low;s->rate.registers=r;if(i>1u){--d->maxdvsw;t->polish=(agc_word)(t->polish-2u);}}
    n->addrwd=0133;multiply(s,sc,&t->buf2);triple_double(s);triple_double(s);return 1;
}
uint8_t p66_cosine(p66_preparation *s,p66_scale_scratch *sc,p66_divide_scratch *d,p66_norm_scratch *n,p66_trig_scratch *t){uint8_t i;int sign=0;if(!supported(s))return 0;for(i=0;i<3u;++i)if((sign=agc_to_int(s->mpac[i]))!=0)break;if(sign>0){s->mpac[0]=agc_negate(s->mpac[0]);s->mpac[1]=agc_negate(s->mpac[1]);}s->mpac[0]=agc_correct(add(010000,s->mpac[0]));return p66_sine(s,sc,d,n,t);}
uint8_t p66_cdu_logic(p66_preparation *s){agc_word original=s->mpac[0];agc_dp value;agc_register_pair r;if(!supported(s))return 0;value.high=0;value.low=(original&AGC_SIGN)?057777:0;r=agc_multiply(agc_extend(original),020000);agc_double_add(&r,&value);s->mpac[0]=value.high;s->mpac[1]=value.low;s->rate.registers=r;return 1;}
uint8_t p66_cdu_trig(p66_preparation *s,p66_scale_scratch *sc,p66_divide_scratch *d,p66_norm_scratch *n,p66_trig_scratch *t,const agc_word cdu[3],agc_word basic_return,agc_word basic_bankset){
    static const uint8_t order[3]={4,2,0},axis[3]={0,2,1};uint8_t k;
    if(s->mpac[7]!=077776 || basic_return>07777u || (basic_bankset&077770u)!=046000u)return 0;
    t->tem2=basic_return;
    for(k=0;k<3u;++k){uint8_t pos=order[k];agc_word saved_hi=s->mpac[0],saved_lo=s->mpac[1],angle_hi,angle_lo,old_sin_lo;
        t->tem3=pos;n->vbuf[4]=saved_hi;n->vbuf[5]=saved_lo;s->mpac[0]=cdu[axis[k]];(void)p66_cdu_logic(s);
        angle_hi=s->mpac[0];angle_lo=s->mpac[1];t->cduspot[pos]=angle_hi;t->cduspot[pos+1u]=angle_lo;
        (void)p66_cosine(s,sc,d,n,t);t->cos[pos]=s->mpac[0];t->cos[pos+1u]=s->mpac[1];
        s->mpac[0]=angle_hi;s->mpac[1]=angle_lo;(void)p66_sine(s,sc,d,n,t);
        n->vbuf[4]=basic_bankset;n->vbuf[5]=agc_correct(s->rate.registers.l);
        old_sin_lo=t->sin[pos+1u];t->sin[pos]=s->mpac[0];t->sin[pos+1u]=s->mpac[1];s->mpac[0]=saved_hi;s->mpac[1]=saved_lo;
        s->rate.registers.a=agc_diminished_abs(pos);s->rate.registers.l=agc_extend(old_sin_lo);
    }
    return 1;
}
