/* SPDX-License-Identifier: MIT */
#include "p66_recurrence_live_cases.h"
void prl_prepare(apollo_overlay_state *m,executive *e,p66_bias_io *io,agc_word *c14,agc_word *c11,uint16_t n){pal_prepare(m,e,io,c14,c11,n);m->words[026]=020000;m->words[024]=(n&1u)?037700:020000;m->words[025]=0;m->words[01234]=0;m->words[01235]=(n&1u)?037600:017700;p66_native_import_executive(e,m);}
