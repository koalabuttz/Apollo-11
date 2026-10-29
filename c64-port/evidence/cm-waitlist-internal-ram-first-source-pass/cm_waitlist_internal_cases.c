/* SPDX-License-Identifier: MIT */
#include "cm_waitlist_internal_cases.h"
#include "cm_waitlist_image_cases.h"
agc_word cm_waitlist_internal_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){static const agc_word dt[8]={1,2,199,0200,0201,16250,16251,037777};static const agc_word clocks[8]={0,1,0177,0200,0201,037700,037776,037777};static const agc_word steps[8]={077715,077777,1,0,037777,040000,077776,077700};uint32_t local=n%8192u;unsigned kind=n/8192u,i,profile=(local/64u)%8u;agc_word delay=dt[(local/8u)%8u],caller=(agc_word)(0100u+local%01277u);cm_waitlist_image_input(s,io,(uint16_t)(n&1u));s->words[3]=(agc_word)((local&7u)*0400u);s->words[4]=(agc_word)(((local>>3)&31u)*02000u);s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));io->superbank=(agc_word)(((local>>8)&7u)*020u);io->interrupt_enabled=0;s->words[026]=clocks[local&7u];for(i=0;i<8u;++i)s->words[01400u+i]=steps[(profile+i/4u)%8u];s->words[01430]=profile&1u?05306:02210;s->words[01431]=profile&1u?040063:040060;s->words[2]=caller;s->words[0]=delay;s->words[5]=kind==0u?05255:kind==1u?05272:05267;
 if(kind==0u){s->words[2]=delay;s->words[0]=03761;s->words[1]=(agc_word)(local%8u);s->words[061]=03753;}
 else{s->words[caller]=delay;}
 return delay;}
