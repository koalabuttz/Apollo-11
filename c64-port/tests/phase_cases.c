/* SPDX-License-Identifier: MIT */
#include "phase_cases.h"
static uint32_t random_word(uint32_t *p) { *p=*p*1664525u+1013904223u; return (*p>>16)&AGC_MASK; }
void phase_prepare(phase_case *c,uint16_t n) {
 uint32_t p=(uint32_t)n+124; uint8_t i;
 for(i=0;i<6;++i) { c->state.records.phase[i]=(agc_word)random_word(&p);
  c->state.records.complement[i]=(agc_word)random_word(&p);
  c->state.records.priority[i]=(agc_word)random_word(&p);
  c->state.records.address[i].location=(agc_word)random_word(&p);
  c->state.records.address[i].bank=(agc_word)random_word(&p);
  c->state.tbase[i]=(agc_word)random_word(&p);
 }
 for(i=0;i<2;++i) { c->state.longbase[i]=(agc_word)random_word(&p); c->state.time[i]=(agc_word)random_word(&p); }
 for(i=0;i<PH_SCRATCH_COUNT;++i) c->state.scratch[i]=(agc_word)random_word(&p);
 c->state.a=0; c->state.l=(agc_register)random_word(&p);
 c->state.q=01000; c->state.bbank=(agc_word)(((n/8)%32)*02000+(n%8));
 c->state.superbank=(agc_word)(n%8*020); c->state.interrupt_enabled=(agc_word)(n%2);
 c->two=(uint8_t)(n%2); c->count=5;
 for(i=0;i<5;++i) c->words[i]=(agc_word)random_word(&p);
 c->words[c->two]=(agc_word)((n/12)*8+1+(n/2)%6);
 if(n%64<3) { c->state.time[0]=n%64==0 ? 0 : n%64==1 ? AGC_NEG_ZERO : 037777; c->state.time[1]=c->state.time[0]; }
 if(n%32<3) c->words[(uint8_t)(c->two+1)]=n%32==0 ? 0 : n%32==1 ? AGC_NEG_ZERO : 040000;
 if(c->two) c->words[0]=(agc_word)((c->words[0]&061770)+1+((n+2)%6));
}
void phase_words(const phase_case *c,void (*emit)(void *,agc_word),void *user) {
 uint8_t i;
 for(i=0;i<6;++i) { emit(user,c->state.records.complement[i]); emit(user,c->state.records.phase[i]);
  emit(user,c->state.records.address[i].location); emit(user,c->state.records.address[i].bank);
  emit(user,c->state.records.priority[i]); emit(user,c->state.tbase[i]); }
 for(i=0;i<2;++i) emit(user,c->state.longbase[i]);
 for(i=0;i<2;++i) emit(user,c->state.time[i]);
 for(i=0;i<PH_SCRATCH_COUNT;++i) emit(user,c->state.scratch[i]);
 emit(user,c->state.a); emit(user,c->state.l); emit(user,c->state.q); emit(user,c->state.bbank);
 emit(user,c->state.superbank); emit(user,c->state.interrupt_enabled);
}

uint16_t phase_target(uint8_t n) { return (uint16_t)((uint16_t)(n/2)*768+(n%2)+2*((n/2)%6)); }
