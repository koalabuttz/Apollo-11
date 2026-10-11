/* SPDX-License-Identifier: MIT; ordinary constructor inputs, no outputs. */
#include "standard_lead_cm_cases.h"
void standard_lead_cm_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){
 uint16_t i,k=(uint16_t)(n%512u),slot=(uint16_t)(k%8u);cm_native_io empty={0};*io=empty;
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)(((uint32_t)i*29u+k*251u+0x1234u)&077777u);
 io->superbank=(agc_word)((k%4u)*020u);io->interrupt_enabled=(agc_word)(k%2u);
 s->words[0]=(agc_word)((k*977u)&0177777u);s->words[1]=(agc_word)((k*8191u)&0177777u);s->words[2]=04000;s->words[3]=(agc_word)((k%8u)*0400u);s->words[4]=066000;s->words[5]=(agc_word)(02000u+n/512u);s->words[6]=(agc_word)(066000u|(k%8u));s->words[7]=0;s->words[067]=slot==0u?AGC_NEG_ZERO:(agc_word)((k%7u)*12u);
 for(i=0;i<7u;++i){s->words[0167u+i*12u]=i==slot?AGC_NEG_ZERO:(agc_word)(i%2u?agc_negate(021110):035110);s->words[0165u+i*12u]=(agc_word)((k+i)&0167u);}
}
