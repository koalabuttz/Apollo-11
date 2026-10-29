/* SPDX-License-Identifier: MIT */
#include "guidance_velocity.h"
enum p66_velocity_status p66_update_velocity(p66_preparation *s,
    p66_scale_scratch *scale,p66_elapsed_state *elapsed,p66_pushdown *p,
    const agc_word bias[6],const agc_word velocity[6],const agc_word position[6]) {
    agc_dp factor; agc_word delta[6],old_z_low; uint8_t i;
    static const uint8_t offsets[3]={0,3,5};
    if(s->mpac[7]!=077776 || p->base<0401 || p->base>0661 ||
       (uint16_t)(p->base-0401u)%054u!=0 || p->pushloc!=p->base+8u)
        return P66_VELOCITY_OUT_OF_SCOPE;
    (void)p66_vector_subtract(s,scale,elapsed,bias);
    p->pushloc=(agc_word)(p->pushloc-2u);
    factor.high=p->words[6]; factor.low=p->words[7];
    (void)p66_vector_multiply(s,scale,&factor);
    (void)p66_vector_shift_left(s,scale,elapsed,2,1);
    (void)p66_vector_add(s,scale,elapsed,velocity);
    p->pushloc=(agc_word)(p->pushloc-6u);
    for(i=0;i<6u;++i) delta[i]=p->words[i];
    (void)p66_vector_add(s,scale,elapsed,delta);
    for(i=0;i<3u;++i) {
        p->words[24u+2u*i]=s->mpac[offsets[i]];
        p->words[25u+2u*i]=s->mpac[offsets[i]+1u];
    }
    /* VSTORE's old destination A/L is consumed by VLOAD. The final DXCH
     * retains the updated velocity's Z low word in L; VMODE replaces A. */
    old_z_low=s->mpac[6];
    for(i=0;i<3u;++i) {
        s->mpac[offsets[i]]=position[2u*i];
        s->mpac[offsets[i]+1u]=position[2u*i+1u];
    }
    s->rate.registers.a=agc_extend(077776);
    s->rate.registers.l=agc_extend(old_z_low);
    return P66_VELOCITY_OK;
}
