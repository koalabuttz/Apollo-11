/* SPDX-License-Identifier: MIT */
#include "acceleration_phased_cases.h"
unsigned acc_phase(acceleration_case *c,uint8_t phase){
 p66_preparation *s=&c->previous.base.previous.previous.previous.preparation;p66_scale_scratch *sc=&c->previous.base.previous.previous.previous.scratch;p66_elapsed_state *el=&c->previous.base.previous.elapsed;p66_divide_scratch *d=&c->previous.base.scratch;p66_norm_scratch *n=&c->previous.norm;p66_pushdown *p=&c->previous.base.previous.previous.push;
 (void)d;switch(phase){
#if !defined(ACCELERATION_PHASE_SELECTED) || ACCELERATION_PHASE_SELECTED==0
case 0:return (unsigned)p66_acceleration_rate_phase(s,sc,el,d,n,p,&c->taurod);
#endif
#if !defined(ACCELERATION_PHASE_SELECTED) || ACCELERATION_PHASE_SELECTED==1
case 1:return (unsigned)p66_acceleration_gravity_phase(s,sc,el,d,n,p,c->gravity);
#endif
#if !defined(ACCELERATION_PHASE_SELECTED) || ACCELERATION_PHASE_SELECTED==2
case 2:return (unsigned)p66_acceleration_scale_phase(s,sc,el,d,n,p);
#endif
#if !defined(ACCELERATION_PHASE_SELECTED) || ACCELERATION_PHASE_SELECTED==3
case 3:return (unsigned)p66_acceleration_sum_phase(s,sc,el,n,p);
#endif
default:return P66_ACCELERATION_OUT_OF_SCOPE;}
}
