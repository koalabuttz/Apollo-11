/* SPDX-License-Identifier: MIT */
#include "unit_phased_cases.h"
void uph_input(unit_case *c,uint32_t n){unit_input(c,3u*(n%UPH_NORMAL_CASES)+2u);if(n>=UPH_NORMAL_CASES){p66_preparation *s=&c->base.previous.previous.previous.preparation;s->mpac[0]=(agc_word)(AGC_MASK^(1u+n-UPH_NORMAL_CASES));c->base.previous.previous.previous.scratch.ovfind=0;}}
void uph_from_words(unit_case *c,const uint16_t w[UNIT_WORDS]){unsigned i;dv_from_words(&c->base,w);c->norm.sr=w[93];c->norm.cyl=w[94];c->norm.addrwd=w[95];c->norm.buf_third=w[96];c->norm.tem1=w[97];for(i=0;i<6u;++i)c->norm.vbuf[i]=w[98+i];}
#ifndef UNIT_PHASED_INPUT_ONLY
unsigned uph_apply(unit_case *c,uint8_t phase){p66_preparation *s=&c->base.previous.previous.previous.preparation;p66_scale_scratch *sc=&c->base.previous.previous.previous.scratch;p66_elapsed_state *el=&c->base.previous.elapsed;p66_divide_scratch *d=&c->base.scratch;p66_pushdown *p=&c->base.previous.previous.push;if(!phase)return (unsigned)p66_unit_square_phase(s,sc,el,d,&c->norm,p);if(phase==1u)return (unsigned)p66_unit_root_phase(s,sc,el,d,&c->norm,p);return (unsigned)p66_unit_finish_phase(s,sc,el,d,&c->norm,p);}
#endif
