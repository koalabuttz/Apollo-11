/* SPDX-License-Identifier: MIT */
#include "guidance_horizontal_phased.h"
enum p66_horizontal_status p66_horizontal_radial_product(p66_preparation *s,p66_scale_scratch *sc,p66_norm_scratch *n,p66_pushdown *p,const agc_dp *hdot){if(s->mpac[7]!=077776||p->base<0401u||p->base>0661u||(p->base-0401u)%054u||p->pushloc!=p->base+6u)return P66_HORIZONTAL_OUT_OF_SCOPE;(void)p66_vector_multiply(s,sc,hdot);n->addrwd=01473;return P66_HORIZONTAL_OK;}
