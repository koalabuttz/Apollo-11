/* SPDX-License-Identifier: MIT */
#include "dsky_display_release_cases.h"
#include "p66_bias_cases.h"
void dsky_display_release_input(apollo_overlay_state *s,dsky_charin_io *io,uint16_t n){
 static const uint16_t raw[]={0,077777,037777,040000,0100000,0177777,0140000,1},cadrs[]={0,077777,1,077776},lists[]={0,077777,02546,077776};uint16_t i,cores=n<4096u ? 8u : 7u,slot=(uint16_t)(1u+(n/32u)%(cores-1u));
 p66_bias_prepare(s,n);s->words[3]=(uint16_t)((n/2u)%8u*0400u);s->words[4]=060000;s->words[6]=(uint16_t)(060000u|((n/2u)%8u));s->words[5]=n<4096u ? 02077 : 02000;s->words[0154]=031;s->words[1]=raw[(n/64u)%8u];s->words[01042]=cadrs[(n/16u)%4u];s->words[01043]=lists[(n/64u)%4u];s->words[01012]=(uint16_t)((n/4u)%2u);s->words[01021]=(uint16_t)((s->words[01021]&057777u)|((n/8u)%2u)*020000u);
 s->words[0167]=030123;for(i=1;i<cores;++i)s->words[0167u+12u*i]=077777;
 s->words[067]=n%3u ? 0 : 077777;
 if(n%2u){s->words[0167u+12u*slot]=agc_negate((uint16_t)((n%7u ? 020000u : 035000u)+slot));s->words[0164u+12u*slot]=s->words[01043];s->words[0165u+12u*slot]=(uint16_t)((n/2u)&0167u);}
 if(n%5u==0){uint16_t candidate=slot==1u ? 2u : 1u;s->words[0167u+12u*candidate]=020321;s->words[067]=(uint16_t)(candidate*12u);}
 /* No-wake branches deliberately contain unusable unrelated contexts. */
 if(s->words[01043]==0){s->words[0167]=0;s->words[0167u+12u]=0;s->words[067]=037777;}
 s->words[4]=(uint16_t)(((n/17u)%32u)*02000u);s->words[6]=(uint16_t)(s->words[4]|(s->words[3]>>8));s->words[5]=n<4096u ? 04457 : 04473;s->words[2]=(uint16_t)(07100u+n%0100u);s->words[0]=raw[(n/256u)%8u];
 io->control.superbank=(uint16_t)(((n/32u)%8u)*020u);io->control.interrupt_enabled=(uint16_t)((n/2u)%2u);io->channel11=(uint16_t)((n*151u+04100u)&077777u);io->wrote_channel11=0;
}
