/* SPDX-License-Identifier: MIT */
#include "guidance_dp_divide.h"
/* Independent fixed-point algebra for the DDV/BDDV source sequence.
 * Ordinary-word overflow corrections are retained at each source store. */
static agc_register add(agc_register a,agc_word b) { return agc_add_register(a,agc_extend(b)); }
static agc_register neg(agc_word w) { return agc_extend(agc_negate(w)); }
static agc_register carry(int n) { return n>0 ? 1 : 0177776; }
static void tp_agree(p66_preparation *s) {
    agc_register a; agc_word limit; int sign,ov; uint8_t i;
    s->mpac[2]=0; s->rate.registers.l=02365;
    sign=agc_to_int(s->mpac[0]); if(!sign) sign=agc_to_int(s->mpac[1]);
    if(!sign) { s->mpac[0]=s->mpac[1]=0; s->rate.registers.a=0; return; }
    limit=sign>0 ? 037777 : 040000;
    a=agc_add_register(agc_extend(limit),carry(sign));
    for(i=2;i>0;--i) {
        a=add(a,s->mpac[i]); s->mpac[i]=agc_correct(a); ov=agc_overflow(a);
        a=add(ov ? carry(ov) : 0,limit);
    }
    a=add(a,s->mpac[0]); s->mpac[0]=agc_correct(a); ov=agc_overflow(a);
    if(ov) a=carry(ov); else s->mpac[1]=agc_correct(a);
    s->rate.registers.a=a;
}
static void canonical_positive(agc_word *hi,agc_word *lo) {
    agc_register a=add(040000,*lo); int ov=agc_overflow(a);
    *lo=agc_correct(a); a=add(ov ? carry(ov) : 0,037777);
    a=add(a,*hi); *hi=agc_correct(a);
}
static void double_pair(agc_word *hi,agc_word *lo) {
    agc_register_pair r; agc_dp p;
    r.a=agc_extend(*hi); r.l=agc_extend(*lo); p.high=*hi; p.low=*lo;
    agc_double_add(&r,&p); *hi=p.high; *lo=p.low;
}
/* Valid hardware-DV domain reached by normalized DDV: no quotient overflow.
 * A/L are interpreted as separate signed radix-16384 limbs. Signed zeros
 * use the lower limb's sign when the high limb is zero. */
static agc_register_pair ratio(agc_word hi,agc_word lo,agc_word divisor) {
    int32_t n=(int32_t)agc_to_int(hi)*INT32_C(16384)+agc_to_int(lo);
    int32_t c=agc_to_int(divisor),q=n/c,r=n%c;
    agc_register_pair out;
    out.a=agc_extend(agc_from_int((int16_t)q));
    out.l=agc_extend(agc_from_int((int16_t)r));
    if(n<0) { if(!q) out.a=0177777; if(!r) out.l=0177777; }
    if(!n && ((agc_to_int(hi)==0 ? lo : hi)&AGC_SIGN)) out.a=out.l=0177777;
    return out;
}
enum p66_divide_result p66_dp_divide(p66_preparation *s,
    p66_scale_scratch *scale,p66_elapsed_state *elapsed,
    p66_divide_scratch *d,agc_dp operand,uint8_t backward) {
    agc_word bh,bl; agc_register a; agc_register_pair r;
    int high,minor,bhigh,bminor,shifts=0; uint8_t i;
    if(s->mpac[7]!=0 || operand.high>AGC_MASK || operand.low>AGC_MASK)
        return P66_DIVIDE_OUT_OF_SCOPE;
    bh=operand.high; bl=operand.low;
    if(backward) { bh=s->mpac[0]; bl=s->mpac[1]; s->mpac[0]=operand.high; s->mpac[1]=operand.low; }
    s->rate.registers.l=agc_extend(d->buf_low);
    d->dvsign=d->dvnormct=d->maxdvsw=077776;
    bhigh=agc_to_int(bh); bminor=agc_to_int(bl);
    if((bhigh==1 && bminor<0) || (bhigh==-1 && bminor>0)) {
        a=add(bhigh>0 ? 040000 : 0137777,bl); bl=agc_correct(a); bh=0; bhigh=0;
    }
    if(!bhigh) {
        tp_agree(s); high=agc_to_int(s->mpac[0]);
        if(high) {
            if(high<0) d->dvsign=agc_add_word(d->dvsign,1);
            a=neg(bl); if(agc_to_int(agc_correct(a))>0) d->dvsign=agc_add_word(d->dvsign,1);
            goto overflow;
        }
        s->mpac[0]=s->mpac[1]; s->mpac[1]=bh; bh=bl; bl=0;
        bhigh=agc_to_int(bh);
        if(!bhigh) {
            a=neg(s->mpac[0]); if(agc_to_int(agc_correct(a))>0) d->dvsign=agc_add_word(d->dvsign,1);
            goto overflow;
        }
    }
    if(bhigh<0) {
        s->rate.registers.l=agc_extend(bl); bh=agc_negate(bh); bl=agc_negate(bl);
        d->dvsign=agc_add_word(d->dvsign,1);
    }
    high=agc_to_int(s->mpac[0]); minor=agc_to_int(s->mpac[1]);
    if(!high && !minor) { elapsed->buf=bh; d->buf_low=bl; s->rate.registers.a=0; return P66_DIVIDE_ZERO; }
    if(high<0 || (!high && minor<0)) {
        s->rate.registers.l=agc_extend(s->mpac[1]);
        s->mpac[0]=agc_negate(s->mpac[0]); s->mpac[1]=agc_negate(s->mpac[1]);
        d->dvsign=agc_add_word(d->dvsign,1);
    }
    a=add(add(neg(s->mpac[0]),077776),bh);
    if(agc_to_int(agc_correct(a))<=0) {
        canonical_positive(&s->mpac[0],&s->mpac[1]); canonical_positive(&bh,&bl);
        a=add(neg(s->mpac[0]),bh);
        if(agc_to_int(agc_correct(a))<0) goto overflow;
        if(agc_to_int(agc_correct(a))==0) {
            d->maxdvsw=0; a=add(neg(s->mpac[1]),bl);
            if(agc_to_int(agc_correct(a))<=0) goto overflow;
        }
    }
    while(!agc_overflow(agc_add_register(agc_extend(bh),agc_extend(bh)))) {
        d->dvnormct=agc_add_word(d->dvnormct,077776);
        double_pair(&bh,&bl); ++shifts;
    }
    for(i=0;i<(uint8_t)shifts;++i) double_pair(&s->mpac[0],&s->mpac[1]);
    if(d->maxdvsw==0 && agc_to_int(bh)==agc_to_int(s->mpac[0])) {
        s->mpac[0]=037777; a=add(add(neg(bl),s->mpac[1]),bh);
    } else {
        r=ratio(s->mpac[0],s->mpac[1],bh); s->mpac[0]=agc_correct(r.a); s->mpac[1]=agc_correct(r.l);
        r=agc_multiply(neg(s->mpac[0]),bl); a=add(r.a,s->mpac[1]);
        s->rate.registers.l=r.l;
        if(agc_overflow(a)) {
            a=agc_add_register(a,neg(bh)); s->mpac[0]=agc_add_word(s->mpac[0],1);
        }
        if(agc_to_int(agc_correct(a))>0) {
            agc_register reduced=agc_add_register(a,neg(bh));
            if(agc_to_int(agc_correct(reduced))>=0) { a=reduced; s->mpac[0]=agc_add_word(s->mpac[0],1); }
        } else if(agc_to_int(agc_correct(a))<0) {
            s->mpac[0]=agc_from_int((int16_t)(agc_to_int(s->mpac[0])-1)); a=add(a,bh);
        } else goto store_minor;
    }
    r=ratio(agc_correct(a),0,bh); a=r.a; s->rate.registers.l=r.l;
store_minor:
    s->mpac[1]=agc_correct(a);
    if(d->dvsign==077777) {
        s->rate.registers.l=agc_extend(s->mpac[1]);
        s->mpac[0]=agc_negate(s->mpac[0]); s->mpac[1]=agc_negate(s->mpac[1]);
    }
    s->mpac[2]=0; elapsed->buf=bh; d->buf_low=bl; s->rate.registers.a=0;
    return P66_DIVIDE_QUOTIENT;
overflow:
    s->mpac[0]=s->mpac[1]=d->dvsign==077777 ? 040000 : 037777;
    if(d->dvsign==077777) s->rate.registers.l=037777;
    scale->ovfind=1; elapsed->buf=bh; d->buf_low=bl; s->rate.registers.a=1;
    return P66_DIVIDE_OVERFLOW;
}
