/* SPDX-License-Identifier: MIT */
#include "cm_idle_dispatch_cases.h"
#include "cm_basic_finish_cases.h"
void cm_idle_dispatch_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){unsigned kind=n/1024u,slot=n%6u+1u;agc_word bank=(agc_word)(((n>>3)&31u)*02000u+(n&7u)+((n&7u)*020u));cm_basic_finish_input(s,io,n);s->words[5]=03231;s->words[4]=02000;s->words[6]=(agc_word)(02000u|(s->words[3]>>8));s->words[0067]=(agc_word)(kind==0u?0:kind==3u?AGC_NEG_ZERO:slot*12u);s->words[0164]=02001;s->words[0165]=bank;s->words[0167]=010110;s->words[0164u+slot*12u]=(agc_word)(kind==2u?agc_negate((agc_word)(03400u+slot)):02000u+slot);s->words[0165u+slot*12u]=bank;s->words[0167u+slot*12u]=020110;s->words[01361]=03340;}
