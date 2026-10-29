/* SPDX-License-Identifier: MIT */
#include "cm_wake_complete_cases.h"
void cm_wake_complete_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){unsigned i,slot=n/4096u,c=n%4096u;agc_word wake=c%16u==0u?0:c%16u==1u?AGC_NEG_ZERO:(agc_word)((c*113u)&AGC_MASK);
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)((n*313u+i*217u+012345u)&AGC_MASK);
 for(i=0;i<13u;++i)((agc_word *)io)[i]=(agc_word)((n*17u+i*113u)&AGC_MASK);
 io->superbank=(agc_word)((c%8u)*020u);io->interrupt_enabled=(agc_word)((c/8u)%2u);io->executive_alarm=0;
 s->words[0]=agc_extend(wake);s->words[1]=(agc_word)((c*191u)&0177777u);s->words[2]=(agc_word)(0100u+c%07700u);
 s->words[3]=(agc_word)((c%8u)*0400u);s->words[4]=(agc_word)(((c/8u)%32u)*02000u);s->words[5]=05201;s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));s->words[7]=0;
 s->words[0067]=c%3u==0u?AGC_NEG_ZERO:c%3u==1u?0:(agc_word)((c%6u+1u)*12u);
 for(i=0;i<7u;++i){agc_word p=(agc_word)(((c*13u+i*7u)%31u+1u)*01000u+0110u);s->words[0167u+i*12u]=c%4u==0u?AGC_NEG_ZERO:c%4u==1u?agc_negate(p):p;s->words[0164u+i*12u]=(agc_word)(wake==AGC_NEG_ZERO?1u:(wake+1u)&AGC_MASK);s->words[0165u+i*12u]=(agc_word)(((c+i)%8u)|(((c+i)%8u)*020u));}
 if(slot<7u){s->words[0167u+slot*12u]=agc_negate((agc_word)((c%31u+1u)*01000u+0110u));s->words[0164u+slot*12u]=c%16u<2u?(wake==0?AGC_NEG_ZERO:0):wake;}
 if(c%5u==0u)s->words[0167u+(c%7u)*12u]=0;
}
