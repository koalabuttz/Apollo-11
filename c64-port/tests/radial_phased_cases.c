/* SPDX-License-Identifier: MIT */
#include "radial_phased_cases.h"
#ifndef RADIAL_PHASED_INPUT_ONLY
unsigned rp_apply(radial_case *c,uint8_t phase){p66_preparation *s=&c->previous.previous.previous.previous.previous.previous.preparation;p66_scale_scratch *sc=&c->previous.previous.previous.previous.previous.previous.scratch;p66_elapsed_state *el=&c->previous.previous.previous.previous.elapsed;p66_divide_scratch *d=&c->previous.previous.previous.scratch;p66_norm_scratch *n=&c->norm;p66_pushdown *p=&c->previous.previous.previous.previous.previous.push;switch(phase){case 0:return (unsigned)p66_unit_square_phase(s,sc,el,d,n,p);case 1:return (unsigned)p66_unit_root_phase(s,sc,el,d,n,p);case 2:return (unsigned)p66_unit_finish_phase(s,sc,el,d,n,p);case 3:return (unsigned)p66_radial_store_unit(s,n,p);case 4:return (unsigned)p66_radial_dot_velocity(s,sc,el,d,n,p);case 5:return (unsigned)p66_radial_store_rate(s,n,p,&c->output);case 6:return (unsigned)p66_radial_predict_distance(s,sc,n,p,&c->output);case 7:return (unsigned)p66_radial_subtract_radius(s,sc,el,n,p,&c->land);default:return (unsigned)p66_radial_store_altitude(s,n,p,&c->output);}}

#endif
