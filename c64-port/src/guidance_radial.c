/* SPDX-License-Identifier: MIT */
#include "guidance_radial.h"
#include "guidance_dot.h"
#include "guidance_shift.h"
#include "guidance_product.h"
#include "guidance_sum.h"
static void store_load(p66_preparation *s,agc_dp *destination,const agc_dp *source) {
    agc_word low=s->mpac[1];
    destination->high=s->mpac[0];destination->low=low;
    s->mpac[0]=source->high;s->mpac[1]=source->low;s->mpac[2]=0;s->mpac[7]=0;
    s->rate.registers.a=0;s->rate.registers.l=agc_extend(low);
}
enum p66_radial_status p66_radial_position(p66_preparation *s,p66_scale_scratch *sc,
    p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n,p66_pushdown *p,
    p66_radial_state *out,const agc_dp *land) {
    static const uint8_t offsets[3]={0,3,5};agc_word velocity[6];agc_dp operand;
    uint8_t i;enum p66_unit_result status;
    if(s->mpac[7]!=077776 || p->base<0401 || p->base>0661 ||
       (uint16_t)(p->base-0401u)%054u!=0 || p->pushloc!=p->base)
        return P66_RADIAL_OUT_OF_SCOPE;
    status=p66_unit_vector(s,sc,el,d,n,p);
    if(status!=P66_UNIT_OK && status!=P66_UNIT_OVERFLOW) return (enum p66_radial_status)status;
    for(i=0;i<3u;++i) {
        p->words[14u+2u*i]=s->mpac[offsets[i]];
        p->words[15u+2u*i]=s->mpac[offsets[i]+1u];
        velocity[2u*i]=p->words[24u+2u*i];velocity[2u*i+1u]=p->words[25u+2u*i];
    }
    (void)p66_dot(s,sc,el,d,&n->buf_third,velocity);
    (void)p66_scalar_shift_left(s,sc,1,1);n->sr=0;
    operand.high=p->words[30];operand.low=p->words[31];
    store_load(s,&out->hdotdisp,&operand);
    (void)p66_scalar_shift_left(s,sc,11,0);
    (void)p66_scalar_product(s,sc,&out->hdotdisp);
    operand.high=p->words[36];operand.low=p->words[37];
    (void)p66_scalar_add(s,sc,el,&operand);
    (void)p66_scalar_subtract(s,sc,el,land);
    store_load(s,&out->hcalc1,&out->hdotdisp);n->addrwd=01473;
    return status==P66_UNIT_OVERFLOW ? P66_RADIAL_UNIT_OVERFLOW : P66_RADIAL_OK;
}
