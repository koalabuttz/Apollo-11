/* SPDX-License-Identifier: MIT */
#include "guidance_norm.h"
static agc_register add(agc_register a,agc_word b) {return agc_add_register(a,agc_extend(b));}
static agc_word rotate(agc_word a) {return (agc_word)(((a<<1)&AGC_MASK)|(a>>14));}
static agc_word shifted(agc_word a) {return (agc_word)((a>>1)|(a&AGC_SIGN));}
static agc_register_pair divide(agc_word hi,agc_word lo,agc_word div) {
    int32_t n=(int32_t)agc_to_int(hi)*INT32_C(16384)+agc_to_int(lo),c=agc_to_int(div);
    int32_t q=n/c,rem=n%c;agc_register_pair r;
    r.a=agc_extend(agc_from_int((int16_t)q));r.l=agc_extend(agc_from_int((int16_t)rem));
    if(n<0) {if(!q) r.a=0177777;if(!rem) r.l=0177777;}
    if(!n && ((agc_to_int(hi)==0?lo:hi)&AGC_SIGN)) r.a=r.l=0177777;
    return r;
}
static agc_register triple_left(p66_preparation *s) {
    agc_dp low={s->mpac[1],s->mpac[2]};agc_register_pair r={agc_extend(low.high),agc_extend(low.low)};
    agc_double_add(&r,&low);s->mpac[1]=low.high;s->mpac[2]=low.low;
    r.a=add(add(r.a,s->mpac[0]),s->mpac[0]);s->mpac[0]=agc_correct(r.a);
    return r.a;
}
static void half_argument(p66_preparation *s,p66_norm_scratch *n,agc_register a) {
    agc_register_pair r=agc_multiply(agc_extend(s->mpac[0]),020000);
    agc_word minor=shifted(s->mpac[1]);n->sr=shifted(agc_correct(a));
    s->mpac[0]=agc_correct(r.a);s->mpac[1]=agc_correct(add(r.l,minor));
}
static enum p66_norm_result root(p66_preparation *s,p66_scale_scratch *sc,
    p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n) {
    agc_register a;agc_register_pair r;agc_word slope,bias;int hi;agc_dp p;
    sc->mptemp=0;hi=agc_to_int(s->mpac[0]);
    if(hi<0) {
        a=(agc_register)(-hi-1);
        if(a || agc_to_int(s->mpac[1])<0) {s->rate.registers.a=a ? (agc_register)(a-1u) : agc_diminished_abs(s->mpac[1]);return P66_NORM_ALARM_1302;}
        goto zero;
    }
    if(!hi) {
        s->mpac[0]=s->mpac[1];s->mpac[1]=s->mpac[2];s->mpac[2]=0;sc->mptemp=7;
        hi=agc_to_int(s->mpac[0]);if(hi<0) goto zero;
        if(!hi) {
            s->mpac[0]=s->mpac[1];s->mpac[1]=0;sc->mptemp=14;
            hi=agc_to_int(s->mpac[0]);if(hi<=0) goto zero;
        }
    }
    a=add((agc_register)(hi-1),060001);
    if(agc_to_int(agc_correct(a))>0) {
        half_argument(s,n,a);slope=022650;bias=06552;
    } else {
        a=add(a,010000);
        if(agc_to_int(agc_correct(a))>0) {half_argument(s,n,a);slope=032506;bias=04604;}
        else {
            uint8_t repeat=0;
            for(;;) {
                if(repeat) (void)triple_left(s);
                sc->mptemp=agc_add_word(sc->mptemp,1);a=triple_left(s);
                n->cyl=rotate(agc_correct(agc_add_register(a,a)));
                {
                    enum agc_ccs_class cl=agc_classify(n->cyl);n->cyl=rotate(n->cyl);
                    if(cl==AGC_NEGATIVE) {slope=022650;bias=06552;break;}
                    if(cl==AGC_MINUS_ZERO) {slope=032506;bias=04604;break;}
                    if(cl==AGC_POSITIVE) {
                        cl=agc_classify(n->cyl);n->cyl=rotate(n->cyl);
                        if(cl==AGC_PLUS_ZERO) {slope=022650;bias=06552;break;}
                        if(cl==AGC_NEGATIVE) {slope=032506;bias=04604;break;}
                    }
                }
                repeat=1;
            }
        }
    }
    r=agc_multiply(agc_extend(slope),s->mpac[0]);a=add(r.a,bias);el->buf=agc_correct(a);
    r=divide(s->mpac[0],0,el->buf);r=agc_multiply(r.a,020000);
    a=add(r.a,el->buf);el->buf=agc_correct(a);r=agc_multiply(a,020000);
    p.high=s->mpac[0];p.low=s->mpac[1];s->mpac[0]=agc_correct(r.a);s->mpac[1]=agc_correct(r.l);
    r=divide(p.high,p.low,el->buf);d->buf_low=agc_correct(r.a);
    r=divide(agc_correct(r.l),0,el->buf);r.l=agc_extend(agc_correct(r.a));r.a=agc_extend(d->buf_low);
    p.high=s->mpac[0];p.low=s->mpac[1];agc_double_add(&r,&p);s->mpac[0]=p.high;s->mpac[1]=p.low;
    if(r.a) {s->mpac[0]=s->mpac[1]=037777;r.a=037777;}
    s->rate.registers=r;return P66_NORM_OK;
zero:
    s->mpac[0]=s->mpac[1]=0;s->rate.registers.a=0;return P66_NORM_OK;
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
enum p66_norm_result p66_square_root(p66_preparation *s,p66_scale_scratch *sc,
    p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n) {
    enum p66_norm_result result;
    if(s->mpac[7]!=0) return P66_NORM_OUT_OF_SCOPE;
    result=root(s,sc,el,d,n);if(result==P66_NORM_OK) unnormalize(s,sc,n);return result;
}

enum p66_norm_result p66_normalized_square_root(p66_preparation *s,p66_scale_scratch *sc,
    p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n) {
    if(s->mpac[7]!=0 && s->mpac[7]!=077776) return P66_NORM_OUT_OF_SCOPE;
    return root(s,sc,el,d,n);
}
