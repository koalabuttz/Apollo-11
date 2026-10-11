/* SPDX-License-Identifier: MIT */
#include "guidance_unit.h"
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
enum p66_norm_result p66_absolute_value(p66_preparation *s){
    uint8_t i;int sign=0;agc_register a=0;if(s->mpac[7]!=0)return P66_NORM_OUT_OF_SCOPE;
    for(i=0;i<3u;++i)if((sign=agc_to_int(s->mpac[i]))!=0){a=agc_diminished_abs(s->mpac[i]);break;}
    if(sign<0){s->rate.registers.l=agc_extend(s->mpac[1]);for(i=0;i<3u;++i)s->mpac[i]=agc_negate(s->mpac[i]);a=agc_extend(s->mpac[2]);}
    s->rate.registers.a=a;return P66_NORM_OK;
}
enum p66_norm_result p66_vector_length(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n,p66_pushdown *p){
    agc_word old_minor;if(s->mpac[7]!=077776||!valid_vac(p))return P66_NORM_OUT_OF_SCOPE;
    vector_square(s,sc,el,d,n,03202);s->mpac[7]=agc_correct(s->rate.registers.l);old_minor=p->words[35];p->words[34]=s->mpac[0];p->words[35]=s->mpac[1];s->rate.registers.a=agc_extend(p->words[34]);s->rate.registers.l=agc_extend(old_minor);
    return p66_square_root(s,sc,el,d,n);
}
static void agree_pair(agc_word *hi,agc_word *lo){
    int sign=agc_to_int(*hi),ov;agc_register a;agc_word h=*hi,l=*lo;
    if(!sign){*hi=0;return;}if(sign<0){h=agc_negate(h);l=agc_negate(l);}
    a=add(040000,l);l=agc_correct(a);ov=agc_overflow(a);a=add(ov?indicator(ov):0,037777);h=agc_correct(add(a,h));
    if(sign<0){h=agc_negate(h);l=agc_negate(l);}*hi=h;*lo=l;
}
static enum p66_unit_result divide_overflow(p66_preparation *s,p66_scale_scratch *sc,p66_divide_scratch *d){
    s->mpac[0]=s->mpac[1]=d->dvsign==077777?040000:037777;if(d->dvsign==077777)s->rate.registers.l=037777;
    s->rate.registers.a=1;sc->ovfind=1;return P66_UNIT_OVERFLOW;
}
/* UNIT enters GENDDV after normalization even when shifting discarded the
 * last nonzero dividend bit. DDV's earlier zero fast exit is inappropriate
 * here: multiplying -0 by BUF's low word decides the retained low zero. */
static agc_register_pair normalized_ratio(agc_word hi,agc_word lo,agc_word div){
    int32_t num=(int32_t)agc_to_int(hi)*INT32_C(16384)+agc_to_int(lo),den=agc_to_int(div);
    int32_t q=num/den,rem=num%den;agc_register_pair r;
    r.a=agc_extend(agc_from_int((int16_t)q));r.l=agc_extend(agc_from_int((int16_t)rem));
    if(num<0){if(!q)r.a=0177777;if(!rem)r.l=0177777;}
    if(!num&&((agc_to_int(hi)==0?lo:hi)&AGC_SIGN))r.a=r.l=0177777;
    return r;
}
static void normalized_quotient(p66_preparation *s,p66_elapsed_state *el,p66_divide_scratch *d){
    agc_register a;agc_register_pair r=normalized_ratio(s->mpac[0],s->mpac[1],el->buf);
    s->mpac[0]=agc_correct(r.a);s->mpac[1]=agc_correct(r.l);
    r=agc_multiply(agc_extend(agc_negate(s->mpac[0])),d->buf_low);a=add(r.a,s->mpac[1]);s->rate.registers.l=r.l;
    if(agc_overflow(a)){a=agc_add_register(a,agc_extend(agc_negate(el->buf)));s->mpac[0]=agc_add_word(s->mpac[0],1);}
    if(agc_to_int(agc_correct(a))>0){
        agc_register reduced=agc_add_register(a,agc_extend(agc_negate(el->buf)));
        if(agc_to_int(agc_correct(reduced))>=0){a=reduced;s->mpac[0]=agc_add_word(s->mpac[0],1);}
    }else if(agc_to_int(agc_correct(a))<0){
        s->mpac[0]=agc_from_int((int16_t)(agc_to_int(s->mpac[0])-1));a=add(a,el->buf);
    }else goto store_minor;
    r=normalized_ratio(agc_correct(a),0,el->buf);a=r.a;s->rate.registers.l=r.l;
store_minor:
    s->mpac[1]=agc_correct(a);
}
static void unit_divide(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n){
    int high=agc_to_int(s->mpac[0]),minor=agc_to_int(s->mpac[1]),negative;agc_word sign;agc_register_pair r;uint8_t count,i;
    if(!high&&!minor){s->rate.registers.a=0;return;}negative=high<0||(!high&&minor<0);sign=negative?077777:agc_diminished_abs(high?s->mpac[0]:s->mpac[1]);
    if(negative){s->mpac[0]=agc_negate(s->mpac[0]);s->mpac[1]=agc_negate(s->mpac[1]);}
    r.a=agc_extend(s->mpac[0]);r.l=agc_extend(s->mpac[1]);
    if(sc->mptemp==077753){
        n->sr=(agc_word)((s->mpac[1]>>1)|(s->mpac[1]&AGC_SIGN));r=agc_multiply(r.a,020000);r.l=add(r.l,n->sr);n->sr=(agc_word)((n->sr>>1)|(n->sr&AGC_SIGN));
    }else{count=(uint8_t)-agc_to_int(sc->mptemp);for(i=0;i<count;++i)double_registers(&r);}
    s->mpac[0]=agc_correct(r.a);s->mpac[1]=agc_correct(r.l);
    normalized_quotient(s,el,d);d->dvsign=sign;
    if(negative){s->mpac[0]=agc_negate(s->mpac[0]);s->mpac[1]=agc_negate(s->mpac[1]);s->rate.registers.l=agc_extend(agc_negate(s->mpac[1]));s->rate.registers.a=0;}
    else s->rate.registers.a=agc_diminished_abs(sign);
}
enum p66_unit_result p66_unit_vector(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n,p66_pushdown *p){
    static const uint8_t positions[3]={0,3,5};uint8_t i,count;agc_word old_ovf,ovf,old_lvlo,old_square_low,old_buf_low;agc_register_pair r;agc_dp component[3],root_pair;enum p66_norm_result status;
    if(s->mpac[7]!=077776||!valid_vac(p))return P66_UNIT_OUT_OF_SCOPE;
    for(i=0;i<3u;++i){uint8_t at=positions[i];agree_pair(&s->mpac[at],&s->mpac[at+1u]);n->vbuf[2u*i]=s->mpac[at];n->vbuf[2u*i+1u]=s->mpac[at+1u];}
    old_ovf=sc->ovfind;n->tem1=old_ovf;sc->ovfind=0;vector_square(s,sc,el,d,n,03031);ovf=sc->ovfind;sc->ovfind=old_ovf;
    if(ovf!=0&&ovf!=077777)return divide_overflow(s,sc,d);
    old_square_low=p->words[35];p->words[34]=s->mpac[0];p->words[35]=s->mpac[1];s->rate.registers.a=0;s->rate.registers.l=agc_extend(old_square_low);
    status=p66_normalized_square_root(s,sc,el,d,n);if(status==P66_NORM_ALARM_1302)return P66_UNIT_ALARM_1302;
    if(agc_to_int(s->mpac[0])<=0){s->rate.registers.a=agc_extend(p->words[36]);s->rate.registers.l=agc_extend(p->words[37]);p->words[36]=p->words[37]=0;return divide_overflow(s,sc,d);}
    count=(uint8_t)sc->mptemp;root_pair.high=s->mpac[0];root_pair.low=s->mpac[1];old_lvlo=p->words[37];
    if(count>14u){
        sc->mptemp=agc_negate((agc_word)(count-15u));for(i=0;i<3u;++i){n->vbuf[2u*i]=n->vbuf[2u*i+1u];n->vbuf[2u*i+1u]=0;}
        r=agc_multiply(agc_extend((agc_word)(020000u>>(count-15u))),s->mpac[0]);p->words[36]=0;p->words[37]=agc_correct(r.a);
    }else if(count==14u){sc->mptemp=077762;p->words[36]=0;p->words[37]=s->mpac[0];}
    else if(!count){sc->mptemp=077753;p->words[36]=root_pair.high;p->words[37]=root_pair.low;}
    else{
        agc_word bit=(agc_word)(040000u>>count),minor_high;sc->mptemp=agc_negate((agc_word)(count-1u));
        r=agc_multiply(agc_extend(bit),root_pair.low);minor_high=agc_correct(r.a);el->buf=minor_high;r=agc_multiply(agc_extend(bit),root_pair.high);r.l=add(r.l,minor_high);p->words[36]=agc_correct(r.a);p->words[37]=agc_correct(r.l);
    }
    for(i=0;i<3u;++i){component[i].high=n->vbuf[2u*i];component[i].low=n->vbuf[2u*i+1u];}
    old_buf_low=d->buf_low;n->vbuf[0]=077776;n->vbuf[1]=old_lvlo;d->maxdvsw=077776;el->buf=root_pair.high;d->buf_low=root_pair.low;
    for(i=0;i<3u;++i){
        s->rate.registers.l=i?0:agc_extend(old_buf_low);s->mpac[0]=component[i].high;s->mpac[1]=component[i].low;unit_divide(s,sc,el,d,n);component[i].high=s->mpac[0];component[i].low=s->mpac[1];
        if(i<2u){n->vbuf[2u*i+2u]=agc_correct(s->rate.registers.a);n->vbuf[2u*i+3u]=agc_correct(s->rate.registers.l);}
    }
    for(i=0;i<3u;++i){uint8_t at=positions[i];s->mpac[at]=component[i].high;s->mpac[at+1u]=component[i].low;}
    s->rate.registers.a=agc_extend(agc_correct(s->rate.registers.a));s->rate.registers.l=agc_extend(agc_correct(s->rate.registers.l));return P66_UNIT_OK;
}
