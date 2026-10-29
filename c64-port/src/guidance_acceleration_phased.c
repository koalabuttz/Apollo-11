/* SPDX-License-Identifier: MIT */
#include "guidance_acceleration_phased.h"
#include "guidance_dp_divide.h"
#include "guidance_gravity.h"
#include "guidance_shift.h"
#include "guidance_sum.h"
static uint8_t valid(const p66_preparation *s,const p66_pushdown *p,uint8_t offset){return (uint8_t)(s->mpac[7]==0&&p->base>=0401u&&p->base<=0661u&&(p->base-0401u)%054u==0&&p->pushloc==p->base+offset);}
enum p66_acceleration_status p66_acceleration_rate_phase(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n,p66_pushdown *p,const agc_dp *taurod){
 if(!valid(s,p,0))return P66_ACCELERATION_OUT_OF_SCOPE;
 (void)p66_scalar_reverse_subtract(s,sc,el,&s->rate.desired);(void)p66_dp_divide(s,sc,el,d,*taurod,0);n->addrwd=01540;return P66_ACCELERATION_OK;
}
enum p66_acceleration_status p66_acceleration_gravity_phase(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n,p66_pushdown *p,const agc_word gravity[6]){
 if(!valid(s,p,0))return P66_ACCELERATION_OUT_OF_SCOPE;
 n->addrwd=01636;(void)p66_push_gravity(s,p,gravity);if(p66_vector_length(s,sc,el,d,n,p)!=P66_NORM_OK)return P66_ACCELERATION_ALARM_1302;return P66_ACCELERATION_OK;
}
enum p66_acceleration_status p66_acceleration_scale_phase(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n,p66_pushdown *p){
 static const agc_dp gscale={01440,0};if(!valid(s,p,2))return P66_ACCELERATION_OUT_OF_SCOPE;
 (void)p66_dp_divide(s,sc,el,d,gscale,0);(void)p66_scalar_shift_right(s,sc,2,1);n->sr=1;n->addrwd=03651;return P66_ACCELERATION_OK;
}
enum p66_acceleration_status p66_acceleration_sum_phase(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,p66_norm_scratch *n,p66_pushdown *p){
 agc_dp saved;if(!valid(s,p,2))return P66_ACCELERATION_OUT_OF_SCOPE;
 p->words[20]=s->mpac[0];p->words[21]=s->mpac[1];p->pushloc=p->base;saved.high=p->words[0];saved.low=p->words[1];(void)p66_scalar_add(s,sc,el,&saved);n->addrwd=p->base;return P66_ACCELERATION_OK;
}
