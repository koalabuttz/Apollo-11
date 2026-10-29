/* SPDX-License-Identifier: MIT */
#include "cm_danzig_cases.h"
#include "cm_basic_finish_cases.h"
void cm_danzig_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){unsigned slot=n%6u+1u;cm_basic_finish_input(s,io,n);s->words[5]=06030;s->words[023]=0;s->words[0164]=(agc_word)(03000u+n%512u);s->words[0165]=(agc_word)(((n>>3)&31u)*02000u+(n&7u));s->words[0067]=(agc_word)(n%4u==0?0:n%4u==1?0:slot*12u);s->words[0167u+slot*12u]=020110;s->words[0164u+slot*12u]=(agc_word)(n<2048u?02000u+slot:agc_negate((agc_word)(03400u+slot)));}
