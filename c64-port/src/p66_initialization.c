/* SPDX-License-Identifier: MIT
 * Exact basic services Luminary099/DAP_INTERFACE_SUBROUTINES.agc124..139
 * and LUNAR_LANDING_GUIDANCE_EQUATIONS.agc1423..1429. */
#include "p66_initialization.h"
static int valid(const p66_basic_initialization *s,agc_word fb) {
 return s->q>=0100 && s->q<=07777 && s->bbank<=AGC_MASK && (s->bbank&076000)==fb &&
  (s->bbank&01770)==0 && s->superbank<=0160 && (s->superbank&017)==0 &&
  (fb!=062000 || (s->superbank&0100)==0) && s->interrupt_enabled<=1;
}
enum p66_init_status p66_stoprate(p66_basic_initialization *s) {
 uint8_t i;if(!valid(s,040000))return P66_INIT_OUT_OF_SCOPE;
 for(i=0;i<P66_CONTROL_WORDS;++i)s->control[i]=0;
 s->a=0;s->l=03000;s->q=3;
 return P66_INIT_OK;
}
enum p66_init_status p66_fastchng(p66_basic_initialization *s) {
 if(!valid(s,062000))return P66_INIT_OUT_OF_SCOPE;
 /* DXCH L is overlapping: the original leaves old Q in A and old EBANK
  * in Q. TS stores the caller return in PHSNAME3; LXCH restores EBANK.
  * TC A executes A as a basic TC, hence final Q=1 at caller return. */
 s->phase3_address[0]=s->q;s->a=s->q;s->l=01400;s->q=1;
 return P66_INIT_OK;
}
