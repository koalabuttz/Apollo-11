/* SPDX-License-Identifier: MIT */
#include "velocity_phased_cases.h"
void vlp_words(const velocity_phased_case *c,uint16_t w[VLP_WORDS]){vl_words(&c->previous,w);w[VL_WORD_COUNT]=c->addrwd;}
void vlp_from_words(velocity_phased_case *c,const uint16_t w[VLP_WORDS]){vl_from_words(&c->previous,w);c->addrwd=w[VL_WORD_COUNT];}
#ifndef VELOCITY_PHASED_INPUT_ONLY
unsigned vlp_apply(velocity_phased_case *c,uint8_t phase){p66_preparation *s=&c->previous.previous.previous.previous.previous.previous.preparation;p66_scale_scratch *sc=&c->previous.previous.previous.previous.previous.previous.scratch;p66_elapsed_state *el=&c->previous.previous.previous.previous.elapsed;p66_pushdown *p=&c->previous.previous.previous.previous.previous.push;if(!phase)return (unsigned)p66_velocity_correct_gravity(s,sc,el,p,&c->addrwd,c->previous.bias);if(phase==1u)return (unsigned)p66_velocity_add_current(s,sc,el,p,&c->addrwd,c->previous.velocity);return (unsigned)p66_velocity_store_position(s,sc,el,p,&c->addrwd,c->previous.position);}
#endif
