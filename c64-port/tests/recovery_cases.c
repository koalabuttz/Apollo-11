/* SPDX-License-Identifier: MIT */
#include "recovery_cases.h"
static agc_word random_word(uint32_t *p) { *p=*p*1664525u+1013904223u; return (agc_word)((*p>>16)&AGC_MASK); }
static agc_register extend_input(agc_word w) { return (agc_register)(w|((w&AGC_SIGN)<<1)); }
void recovery_prepare(recovery_case *c,uint16_t n) {
 uint32_t p=(uint32_t)n+671; uint8_t i;
 c->entry=n<32768 ? RECOVERY_FLAGS : (enum recovery_abort_entry)(1+(n-32768)/4096);
 for(i=0;i<6;++i) { c->state.records.phase[i]=random_word(&p);
  c->state.records.complement[i]=random_word(&p); c->state.records.priority[i]=random_word(&p);
  c->state.records.address[i].location=random_word(&p); c->state.records.address[i].bank=random_word(&p);
  c->state.tbase[i]=random_word(&p);
 }
 for(i=0;i<2;++i) c->state.longbase[i]=random_word(&p);
 c->state.flagword2=(agc_word)(n&AGC_MASK);
 c->state.flagword3=(agc_word)(AGC_MASK^(n&AGC_MASK));
 c->state.flagword10=(agc_word)(((n<<1)|(n>>14))&AGC_MASK);
 c->state.a=extend_input(random_word(&p)); c->state.l=extend_input(random_word(&p));
 c->state.q=(agc_word)(01000+(n%0400));
 c->state.bbank=(agc_word)((c->entry==RECOVERY_FLAGS ? 026000 : c->entry==RECOVERY_POODOO_PHASE4 ? ((n/8)%32)*02000 : 012000)+(n%8));
 c->state.superbank=(agc_word)(n%8*020); c->state.interrupt_enabled=(agc_word)(n%2);
 if(n%64<4) for(i=0;i<6;++i) { c->state.records.phase[i]=n%64==0 ? 0 : n%64==1 ? AGC_NEG_ZERO : n%64==2 ? 037777 : 040000;
  c->state.records.complement[i]=(agc_word)(AGC_MASK^c->state.records.phase[i]); }
}
uint16_t recovery_target(uint8_t n) { return n<64 ? (uint16_t)(n*511) : (uint16_t)(32768+(n%4)*4096+(n/4-16)*237); }
void recovery_words(const recovery_case *c,void (*emit)(void *,agc_word),void *u) {
 uint8_t i;
 for(i=0;i<6;++i) { emit(u,c->state.records.complement[i]); emit(u,c->state.records.phase[i]);
  emit(u,c->state.records.address[i].location); emit(u,c->state.records.address[i].bank);
  emit(u,c->state.records.priority[i]); emit(u,c->state.tbase[i]); }
 for(i=0;i<2;++i) emit(u,c->state.longbase[i]);
 emit(u,c->state.flagword2); emit(u,c->state.flagword3); emit(u,c->state.flagword10);
 emit(u,c->state.a); emit(u,c->state.l); emit(u,c->state.q); emit(u,c->state.bbank);
 emit(u,c->state.superbank); emit(u,c->state.interrupt_enabled);
}
