/* SPDX-License-Identifier: MIT */
#include "cm_mission_loop_cases.h"
#include "cm_service_loop_cases.h"
void cm_mission_loop_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){agc_word bb;cm_service_loop_input(s,io,n);bb=(agc_word)((((n>>3)&31u)<<10)|(n&7u));s->words[6]=bb;s->words[3]=(agc_word)((bb&7u)*0400u);s->words[4]=(agc_word)(bb&076000u);s->words[5]=06030;s->words[0164]=(agc_word)(03000u+n);s->words[0165]=bb;s->words[0067]=0;s->words[023]=0;s->words[0115]=(agc_word)(bb&040000u);io->interrupt_enabled=1;}
