/* SPDX-License-Identifier: MIT */
#include "matrix_native.h"
#include "guidance_norm.h"
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
static void dot_subroutine(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,
               p66_divide_scratch *d,agc_word *buf_third,const agc_word operand[6],agc_word ret) {
    agc_register_pair *r=&s->rate.registers;
    agc_dp y,z,factor,low;int overflow;
    y.high=s->mpac[3];y.low=s->mpac[4];z.high=s->mpac[5];z.low=s->mpac[6];
    d->dvnormct=ret;
    /* DMPSUB itself is independent of MODE. The exclusively owned native
     * calculation uses its scalar entry temporarily; source vector MODE is
     * retained at the return. No intermediate dispatch is introduced. */
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
    s->mpac[7]=077776;
}

static void dca(p66_preparation *p,const agc_word *v){p->rate.registers.a=agc_extend(v[0]);p->rate.registers.l=agc_extend(v[1]);}
static void dxch(p66_preparation *p,agc_word *v){agc_register a=p->rate.registers.a,l=p->rate.registers.l;dca(p,v);v[0]=agc_correct(a);v[1]=agc_correct(l);}
uint8_t matrix_native_phase(apollo_overlay_state *s,enum apollo_fixed_mission mission,uint8_t column,const agc_word matrix[18],uint8_t phase){
 p66_preparation p={0};p66_scale_scratch sc;p66_elapsed_state el={0};p66_divide_scratch d;agc_word vbuf[6],buf2[2],operand[6],third,addr;uint16_t *w=s->words,first,expected,i,row,offset;static const uint8_t map[6]={0,1,3,4,5,6};
 if((mission!=APOLLO_FIXED_CM&&mission!=APOLLO_FIXED_LM)||column>1u||phase>3u||matrix==0)return 0;
 first=mission==APOLLO_FIXED_CM?07313:07344;
 expected=phase==0?(uint16_t)(first-(column?5u:8u)):phase==1?first:phase==2?(uint16_t)(first+015u):(uint16_t)(first+026u);
 if(w[5]!=expected||w[0163]!=077776u||w[7]!=0||w[3]>03400u||(w[3]&0377u)||w[4]>076000u||(w[4]&01777u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||w[0116]<01000u||w[0116]>07777u)return 0;
 offset=(uint16_t)((phase?phase-1u:0u)*(column?2u:6u));
 if(w[0116]<01000u+offset||w[0116]>07757u+offset)return 0;
 if(phase!=0&&(w[0136]!=(column?6u:2u)||w[0140]!=(column?077765u:2u)))return 0;
 for(i=0;i<18u;++i)if(matrix[i]>AGC_MASK)return 0;
 for(i=0;i<8u;++i){p.mpac[i]=w[0154u+i];}p.rate.registers.a=w[0];p.rate.registers.l=w[1];sc.mptemp=w[0135];sc.ovfind=w[0121];el.buf=w[0130];d.buf_low=w[0131];third=w[0132];d.dvsign=w[0136];d.dvnormct=w[0137];d.maxdvsw=w[0140];addr=w[0116];
 for(i=0;i<6u;++i){vbuf[i]=w[0122u+i];}buf2[0]=w[0133];buf2[1]=w[0134];
 if(phase==0){d.maxdvsw=column?077765:2;d.dvsign=column?6:2;p.rate.registers.a=agc_extend(vbuf[4]);p.rate.registers.l=agc_extend(vbuf[5]);for(i=0;i<6u;++i)vbuf[i]=p.mpac[map[i]];w[2]=3;w[5]=first;}
 else{row=(uint16_t)(phase-1u);for(i=0;i<3u;++i){uint16_t at=column?(uint16_t)(2u*row+6u*i):(uint16_t)(6u*row+2u*i);operand[2u*i]=matrix[at];operand[2u*i+1u]=matrix[at+1u];}
 dot_subroutine(&p,&sc,&el,&d,&third,operand,(agc_word)(expected+1u));addr=agc_add_word(addr,(agc_word)(2u*d.dvsign));w[2]=0140;
 if(phase==1){dca(&p,vbuf);dxch(&p,p.mpac);dxch(&p,buf2);dca(&p,vbuf+2);dxch(&p,p.mpac+3);dca(&p,vbuf+4);dxch(&p,p.mpac+5);}
 else if(phase==2){dxch(&p,vbuf);dxch(&p,p.mpac);dxch(&p,vbuf+2);dxch(&p,p.mpac+3);dxch(&p,vbuf+4);dxch(&p,p.mpac+5);}
 else{dxch(&p,buf2);dxch(&p,p.mpac);dxch(&p,p.mpac+5);dxch(&p,vbuf+2);dxch(&p,p.mpac+3);}
 if(phase<3u){p.rate.registers.a=agc_add_register(agc_extend(d.maxdvsw),agc_extend(addr));addr=agc_correct(p.rate.registers.a);w[5]=(uint16_t)(first+(phase==1u?015u:026u));}else w[5]=mission==APOLLO_FIXED_CM?06030:06061;}
 for(i=0;i<8u;++i){w[0154u+i]=p.mpac[i];}w[0]=p.rate.registers.a;w[1]=p.rate.registers.l;w[0135]=sc.mptemp;w[0121]=sc.ovfind;w[0130]=el.buf;w[0131]=d.buf_low;w[0132]=third;w[0136]=d.dvsign;w[0137]=d.dvnormct;w[0140]=d.maxdvsw;w[0116]=addr;for(i=0;i<6u;++i){w[0122u+i]=vbuf[i];}w[0133]=buf2[0];w[0134]=buf2[1];return 1;}
