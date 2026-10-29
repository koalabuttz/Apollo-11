/* SPDX-License-Identifier: MIT */
#include "cm_normal_display_cases.h"
#include "cm_nv85_cases.h"
void cm_normal_display_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){static const uint32_t time[16]={0,1,49,50,51,5999,6000,6001,359849,359850,359851,359949,359950,359951,268435454,268435455};cm_nv85_input(s,io,n);if(n<512u)s->words[01145]=01450;if(n<64u){uint32_t value=time[n%16u];s->words[03660]=(agc_word)(value>>14);s->words[03661]=(agc_word)(value&037777u);if(n&16u){s->words[03660]=agc_negate(s->words[03660]);s->words[03661]=agc_negate(s->words[03661]);}if(n&32u)s->words[03661]=agc_negate(s->words[03661]);}}
