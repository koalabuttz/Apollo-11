/* SPDX-License-Identifier: MIT */
#include "guidance_vector.h"
static agc_register indicator(int overflow) {
    return overflow>0 ? 1 : agc_extend(077776);
}
static uint8_t component(p66_preparation *s,p66_scale_scratch *scale,
                         p66_elapsed_state *elapsed,const agc_word *operand,
                         uint8_t offset,uint8_t subtract) {
    agc_register_pair *r=&s->rate.registers;
    agc_dp result;
    uint8_t path=0;
    result.high=s->mpac[offset]; result.low=s->mpac[offset+1u];
    r->a=agc_extend(subtract ? agc_negate(operand[0]) : operand[0]);
    r->l=agc_extend(subtract ? agc_negate(operand[1]) : operand[1]);
    agc_double_add(r,&result);
    if(r->a!=0) {
        agc_word limit=r->a==1 ? 037777 : 040000;
        agc_register a;
        int overflow;
        path=1; elapsed->buf=limit;
        /* OVERFLWY/Z select component index in L; OVERFLOW then performs
         * the same low/high correction as scalar DSU. The next DAS resets L. */
        r->l=offset;
        a=agc_add_register(agc_extend(limit),indicator(r->a==1 ? 1 : -1));
        a=agc_add_register(a,agc_extend(result.low)); result.low=agc_correct(a);
        overflow=agc_overflow(a);
        if(overflow!=0) { path|=2u; a=indicator(overflow); } else a=0;
        a=agc_add_register(a,agc_extend(limit));
        a=agc_add_register(a,agc_extend(result.high)); result.high=agc_correct(a);
        overflow=agc_overflow(a);
        if(overflow!=0) { path|=4u; a=indicator(overflow); scale->ovfind=agc_correct(a); }
        r->a=a;
    }
    s->mpac[offset]=result.high; s->mpac[offset+1u]=result.low;
    return path;
}
static uint16_t vector(p66_preparation *s,p66_scale_scratch *scale,
                        p66_elapsed_state *elapsed,const agc_word operand[6],
                        uint8_t subtract) {
    uint16_t path;
    if(s->mpac[7]!=077776) return P66_VECTOR_OUT_OF_SCOPE;
    path=(uint16_t)((uint16_t)component(s,scale,elapsed,operand+2,3,subtract)<<3);
    path=(uint16_t)(path | ((uint16_t)component(s,scale,elapsed,operand+4,5,subtract)<<6));
    return (uint16_t)(path | component(s,scale,elapsed,operand,0,subtract));
}
uint16_t p66_vector_add(p66_preparation *s,p66_scale_scratch *scale,
                        p66_elapsed_state *elapsed,const agc_word operand[6]) {
    return vector(s,scale,elapsed,operand,0);
}
uint16_t p66_vector_subtract(p66_preparation *s,p66_scale_scratch *scale,
                             p66_elapsed_state *elapsed,const agc_word operand[6]) {
    return vector(s,scale,elapsed,operand,1);
}
