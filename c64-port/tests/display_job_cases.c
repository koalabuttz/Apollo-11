/* SPDX-License-Identifier: MIT */
#include "display_job_cases.h"
#include "makeplay_cases.h"
void display_job_input(apollo_overlay_state *s,p66_bias_io *io,agc_word *channel,uint16_t n){uint16_t i;makeplay_input(s,io,channel,n);s->words[5]=02546;s->words[0167]=(agc_word)(022000u+(n&0777u));s->words[0100]=0;s->words[01012]=0;s->words[01042]=s->words[01043]=0;s->words[01021]&=057777u;s->words[067]=0;io->superbank=060;for(i=1;i<8u;++i)s->words[0167u+12u*i]=AGC_NEG_ZERO;}
