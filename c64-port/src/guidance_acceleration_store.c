/* SPDX-License-Identifier: MIT */
#include "guidance_acceleration_store.h"
#include "guidance_sum.h"
static uint8_t valid(const p66_preparation *s,const p66_pushdown *p){return (uint8_t)(s->mpac[7]==0&&p->base>=0401u&&p->base<=0661u&&(p->base-0401u)%054u==0&&p->pushloc==p->base+2u);}
enum p66_acceleration_status p66_acceleration_store_phase(p66_preparation *s,p66_norm_scratch *n,p66_pushdown *p){
 agc_word old_low;if(!valid(s,p))return P66_ACCELERATION_OUT_OF_SCOPE;
 old_low=p->words[21];p->words[20]=s->mpac[0];p->words[21]=s->mpac[1];s->rate.registers.a=0;s->rate.registers.l=agc_extend(old_low);n->addrwd=(agc_word)(p->base+20u);return P66_ACCELERATION_OK;
}
enum p66_acceleration_status p66_acceleration_add_phase(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,p66_norm_scratch *n,p66_pushdown *p){
 agc_dp saved;if(!valid(s,p))return P66_ACCELERATION_OUT_OF_SCOPE;
 p->pushloc=p->base;saved.high=p->words[0];saved.low=p->words[1];(void)p66_scalar_add(s,sc,el,&saved);n->addrwd=p->base;return P66_ACCELERATION_OK;
}
