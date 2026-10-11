/* SPDX-License-Identifier: MIT */
#include "guidance_gravity.h"
enum p66_push_status p66_push_gravity(p66_preparation *s,
    p66_pushdown *p,const agc_word gravity[6]) {
    agc_word old_z_low;
    if(s->mpac[7]!=0 || p->base<0401 || p->base>0661 ||
       (uint16_t)(p->base-0401u)%054u!=0 || p->pushloc<p->base ||
       (uint16_t)(p->pushloc-p->base)>P66_VAC_WORDS-2u)
        return P66_PUSH_OUT_OF_SCOPE;
    old_z_low=s->mpac[6];
    p->words[p->pushloc-p->base]=s->mpac[0];
    p->words[p->pushloc-p->base+1u]=s->mpac[1];
    p->pushloc=(agc_word)(p->pushloc+2u);
    s->mpac[0]=gravity[0]; s->mpac[1]=gravity[1];
    s->mpac[3]=gravity[2]; s->mpac[4]=gravity[3];
    s->mpac[5]=gravity[4]; s->mpac[6]=gravity[5];
    s->mpac[7]=077776;
    s->rate.registers.a=agc_extend(077776);
    s->rate.registers.l=agc_extend(old_z_low);
    return P66_PUSH_OK;
}
