/* SPDX-License-Identifier: MIT; input-only original source scenarios. */
#include "cm_enema_cases.h"
static agc_word draw(uint32_t *p) { *p=*p*UINT32_C(1664525)+UINT32_C(1013904223);return (agc_word)((*p>>16)&AGC_MASK); }
static agc_register extend(agc_word w) { return (agc_register)(w|((w&AGC_SIGN)<<1)); }
void cm_enema_prepare(cm_enema_case *c,uint16_t n) {
 uint32_t p=n+959u;uint8_t i;static const agc_word times[]={0,1,127,128,129,16383,8192,16300};
 cm_rs_prepare(&c->previous,(uint16_t)(n%256u));c->previous.exec.alarm=0;c->previous.wait.alarm=0;
 for(i=0;i<6;++i) { c->previous.phases.phase[i]=draw(&p);c->previous.phases.complement[i]=(agc_word)(c->previous.phases.phase[i]^AGC_MASK); }
 if(n%8u<6) c->previous.phases.complement[n%8u]^=(agc_word)(1u<<((n/8u)%15u));
 c->previous.record.word[CM_RS_FLAGWRD6]=(agc_word)(((n/8u)%4u)*020000u+(draw(&p)&017777));
 c->previous.record.word[CM_RS_RASFLAG]=draw(&p);c->previous.wait.time3=times[(n/32u)%8u];
 c->io.main_keys=draw(&p);c->io.navigation_keys=draw(&p);
 if(n%32u<6) {
  c->io.main_keys=n%32u==0 || n%32u==4 ? 022 : n%32u==1 ? 0 : n%32u==2 ? AGC_NEG_ZERO : 021;
  c->io.navigation_keys=n%32u==4 ? 0 : n%32u==5 ? 0122 : 0100;
 }
 c->io.a=extend(draw(&p));c->io.l=extend(draw(&p));c->io.q=01010;c->io.bbank=(agc_word)(012000+n%8u);
 c->io.superbank=(agc_word)(n%8u*020u);c->io.interrupt_enabled=(agc_word)(n%2u);c->io.waitexit=draw(&p);c->io.waitbank=draw(&p);
}
void cm_enema_words(const cm_enema_case *c,void (*emit)(void *,agc_word),void *u) {
 cm_rs_words(&c->previous,emit,u);emit(u,c->io.main_keys);emit(u,c->io.navigation_keys);emit(u,c->io.a);emit(u,c->io.l);emit(u,c->io.q);emit(u,c->io.bbank);emit(u,c->io.superbank);emit(u,c->io.interrupt_enabled);emit(u,c->io.waitexit);emit(u,c->io.waitbank);
}
