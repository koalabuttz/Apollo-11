/* SPDX-License-Identifier: MIT */
#include "p66_initialization_cases.h"
static agc_word word(uint32_t *s) {*s=*s*1664525u+1013904223u;return (agc_word)((*s>>16)&AGC_MASK);}
void p66_init_prepare(p66_init_case *c,uint16_t n) {
 uint8_t i;uint32_t seed=n+922; c->fast=(uint8_t)(n%2u);
 for(i=0;i<P66_CONTROL_WORDS;++i)c->state.control[i]=word(&seed);
 c->state.phase3_address[0]=word(&seed);c->state.phase3_address[1]=word(&seed);
 c->state.a=word(&seed);c->state.l=word(&seed);c->state.q=(agc_word)(01000+n/2u%256u);
 c->state.bbank=(agc_word)((c->fast ? 062000 : 040000)+(n/2u)%8u);
 c->state.superbank=(agc_word)((n/16u)% (c->fast ? 4u : 8u)*020u);c->state.interrupt_enabled=(agc_word)(n/2u%2u);
}
void p66_init_words(const p66_init_case *c,void (*emit)(void *,agc_word),void *u) {
 uint8_t i;for(i=0;i<P66_CONTROL_WORDS;++i)emit(u,c->state.control[i]);
 emit(u,c->state.phase3_address[0]);emit(u,c->state.phase3_address[1]);emit(u,c->state.a);emit(u,c->state.l);emit(u,c->state.q);emit(u,c->state.bbank);emit(u,c->state.superbank);emit(u,c->state.interrupt_enabled);
}
