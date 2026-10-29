/* SPDX-License-Identifier: MIT */
#include "enema_cases.h"
static agc_word random_word(uint32_t *p) { *p=*p*1664525u+1013904223u; return (agc_word)((*p>>16)&AGC_MASK); }
static agc_register extend_input(agc_word w) { return (agc_register)(w|((w&AGC_SIGN)<<1)); }
void enema_prepare(enema_case *c,uint16_t n) {
 uint32_t p=(uint32_t)n+959; uint8_t i;
 rs_prepare(&c->previous,(uint16_t)(96+n%16));
 c->previous.exec.alarm=0; c->previous.wait.alarm=0;
 for(i=0;i<6;++i) {
  c->previous.phases.phase[i]=random_word(&p);
  c->previous.phases.complement[i]=(agc_word)(AGC_MASK^c->previous.phases.phase[i]);
 }
 if(n%8<6) {
  agc_word disagreement;
  static const agc_word corners[]={0,1,037777,040000,077776,077777};
  disagreement=n>=2048 ? random_word(&p) : (agc_word)(AGC_MASK^(1u<<((n/8)%15)));
  if(n%64>=16 && n%64<22) disagreement=corners[n%64-16];
  c->previous.phases.complement[n%8]=(agc_word)(c->previous.phases.phase[n%8]^disagreement);
 }
 c->io.flagword10=random_word(&p); c->io.main_keys=random_word(&p);
 c->io.navigation_keys=random_word(&p);
 if(n%32<4) { c->io.main_keys=n%32==0 ? 022 : n%32==1 ? 0 : n%32==2 ? AGC_NEG_ZERO : 021;
  c->io.navigation_keys=020; }
 if(n%32==4) { c->io.main_keys=022; c->io.navigation_keys=0; }
 c->io.a=extend_input(random_word(&p)); c->io.l=extend_input(random_word(&p));
 c->io.q=01010; c->io.bbank=(agc_word)(012000+n%8);
 c->io.superbank=(agc_word)(n%8*020); c->io.interrupt_enabled=(agc_word)(n%2);
}
uint16_t enema_target(uint8_t n) { return (uint16_t)(n*31); }
void enema_words(const enema_case *c,rs_word_sink emit,void *u) {
 rs_words(&c->previous,emit,u);
 emit(u,c->io.flagword10); emit(u,c->io.main_keys); emit(u,c->io.navigation_keys);
 emit(u,c->io.a); emit(u,c->io.l); emit(u,c->io.q); emit(u,c->io.bbank);
 emit(u,c->io.superbank); emit(u,c->io.interrupt_enabled);
}
