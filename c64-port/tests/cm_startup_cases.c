/* SPDX-License-Identifier: MIT; independent source input constructors. */
#include "cm_startup_cases.h"
void cm_startup_prepare(apollo_overlay_state *s,cm_native_io *io,cm_startup_ports *p,uint16_t n) {
 uint16_t i;agc_word k=(agc_word)((n*97u+05271u)&AGC_MASK);
 for(i=0;i<2048;++i) s->words[i]=(agc_word)((k+i*313u)&AGC_MASK);
 s->words[0]=s->words[1]=s->words[7]=0;
 s->words[035]=s->words[036]=s->words[047]=0;
 for(i=050;i<=054;++i) s->words[i]=0;
 s->words[2]=01010;
 s->words[3]=01400;s->words[4]=012000;s->words[6]=012003;
 s->words[01036]=(agc_word)((n&3u)*020u+((n&4u)!=0 ? 010u : 0));
 io->channel11=k;io->channel12=(agc_word)(k^AGC_MASK);
 io->channel13=(agc_word)((n&7u)*010000u+((n&8u)!=0 ? 03777u : 0));io->channel14=k;
 io->main_keys=(agc_word)(k&037);io->navigation_keys=(agc_word)(k&0177);
 io->superbank=(agc_word)((n&7u)*020u);io->interrupt_enabled=(agc_word)(n&1u);
 io->trap31a=(agc_word)(n&1u);io->trap31b=(agc_word)((n>>1)&1u);io->trap32=(agc_word)((n>>2)&1u);
 io->executive_alarm=io->waitlist_alarm=n<256 ? 01202 : 0;
 p->channel5=k;p->channel6=(agc_word)(k^AGC_MASK);
}
void cm_startup_words(const apollo_overlay_state *s,const cm_native_io *io,const cm_startup_ports *p,
 enum cm_startup_status status,void (*emit)(void *,agc_word),void *u) {
 uint16_t i;emit(u,(agc_word)status);
 for(i=0;i<2048;++i) if(i!=5) emit(u,s->words[i]);
 emit(u,io->channel11);emit(u,io->channel12);emit(u,io->channel13);emit(u,io->channel14);
 emit(u,io->main_keys);emit(u,io->navigation_keys);emit(u,io->superbank);emit(u,io->interrupt_enabled);
 emit(u,io->trap31a);emit(u,io->trap31b);emit(u,io->trap32);emit(u,io->executive_alarm);emit(u,io->waitlist_alarm);
 emit(u,p->channel5);emit(u,p->channel6);
}
