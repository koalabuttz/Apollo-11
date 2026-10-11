/* SPDX-License-Identifier: MIT */
#include "cm_scan_zero_cases.h"
#include <string.h>
void cm_scan_zero_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){
 unsigned i,kind=n/4096u,c=n%4096u,zero=c%6u+1u;
 memset(io,0,sizeof *io);
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)((n*313u+i*217u+012345u)&AGC_MASK);
 for(i=0;i<13u;++i)((agc_word *)io)[i]=(agc_word)((n*17u+i*113u)&AGC_MASK);
 io->superbank=(agc_word)((c%8u)*020u);io->interrupt_enabled=(agc_word)((c/8u)%2u);io->executive_alarm=0;
 s->words[0]=kind==2u?(agc_word)((c%31u+1u)*01000u):agc_extend((agc_word)((c*43u)&AGC_MASK));
 s->words[1]=(agc_word)((c*191u)&0177777u);s->words[2]=(agc_word)(0100u+c%07700u);
 s->words[3]=(agc_word)((c%8u)*0400u);s->words[4]=(agc_word)(((c/8u)%32u)*02000u);s->words[5]=kind==0u?05217:kind==1u?05175:05210;s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));s->words[7]=0;
 s->words[0067]=0;s->words[0164]=02222;s->words[0165]=s->words[6];s->words[0166]=0401;s->words[0167]=(agc_word)((c%31u+1u)*01000u+0401u);s->words[0400]=0;
 for(i=1;i<7u;++i){unsigned a=0154u+12u*i;agc_word p=(agc_word)(((c*13u+i*7u)%31u+1u)*01000u+0110u);s->words[a+8u]=(agc_word)(02000u+c%01777u);s->words[a+9u]=(agc_word)((((c+i)%32u)*02000u)|((c+i)%8u)|(((c+i)%8u)*020u));s->words[a+10u]=0110;s->words[a+11u]=c%4u==0u?AGC_NEG_ZERO:c%4u==1u?agc_negate(p):p;}
 s->words[0167u+zero*12u]=0;
 if(c%7u==0u)s->words[0233]=0;
}
