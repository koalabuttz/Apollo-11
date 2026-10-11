/* SPDX-License-Identifier: MIT */
#include "guidance_sum.h"
static agc_register indicator(int overflow) {return overflow>0?1:agc_extend(077776);}
static uint8_t component(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,
                         const agc_dp *operand,uint8_t offset,uint8_t operation) {
    agc_register_pair *r=&s->rate.registers;agc_dp result;uint8_t path=0;
    if(operation==2u) {
        /* BDSU/BVSU exchange the operand into MPAC and complement the
         * displaced accumulator in A/L before adding. Raw zero signs are
         * preserved across the exchange; this is not integer subtraction. */
        result=*operand;r->a=agc_extend(agc_negate(s->mpac[offset]));
        r->l=agc_extend(agc_negate(s->mpac[offset+1u]));
    } else {
        result.high=s->mpac[offset];result.low=s->mpac[offset+1u];
        r->a=agc_extend(operation?agc_negate(operand->high):operand->high);
        r->l=agc_extend(operation?agc_negate(operand->low):operand->low);
    }
    agc_double_add(r,&result);
    if(r->a!=0) {
        agc_word limit=r->a==1?037777:040000;agc_register a;int overflow;
        path=1;el->buf=limit;r->l=offset;
        a=agc_add_register(agc_extend(limit),indicator(r->a==1?1:-1));
        a=agc_add_register(a,agc_extend(result.low));result.low=agc_correct(a);
        overflow=agc_overflow(a);
        if(overflow) {path|=2u;a=indicator(overflow);}else a=0;
        a=agc_add_register(a,agc_extend(limit));a=agc_add_register(a,agc_extend(result.high));
        result.high=agc_correct(a);overflow=agc_overflow(a);
        if(overflow) {path|=4u;a=indicator(overflow);sc->ovfind=agc_correct(a);}r->a=a;
    }
    s->mpac[offset]=result.high;s->mpac[offset+1u]=result.low;return path;
}
static uint16_t scalar(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,const agc_dp *operand,uint8_t operation) {
    if(s->mpac[7]!=0)return P66_SUM_OUT_OF_SCOPE;
    return component(s,sc,el,operand,0,operation);
}
uint16_t p66_scalar_add(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,const agc_dp *operand) {
    return scalar(s,sc,el,operand,0);
}
uint16_t p66_scalar_subtract(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,const agc_dp *operand) {
    return scalar(s,sc,el,operand,1);
}
uint16_t p66_scalar_reverse_subtract(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,const agc_dp *operand) {
    return scalar(s,sc,el,operand,2);
}
uint16_t p66_vector_reverse_subtract(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,const agc_word operand[6]) {
    uint16_t path;agc_dp pair;
    if(s->mpac[7]!=077776)return P66_SUM_OUT_OF_SCOPE;
    pair.high=operand[2];pair.low=operand[3];path=(uint16_t)((uint16_t)component(s,sc,el,&pair,3,2)<<3);
    pair.high=operand[4];pair.low=operand[5];path=(uint16_t)(path|((uint16_t)component(s,sc,el,&pair,5,2)<<6));
    pair.high=operand[0];pair.low=operand[1];return (uint16_t)(path|component(s,sc,el,&pair,0,2));
}
