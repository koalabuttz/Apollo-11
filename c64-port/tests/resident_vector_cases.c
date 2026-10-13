/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "resident_vector_cases.h"
#include "resident_rupt.h"
#include "rupt_lead_ins_cases.h"
#include "t4rupt_core_cases.h"
#include "keyrupt_uprupt_cases.h"
#include "lm_t6rupt_cases.h"
/* One rnd call per full expression (resident_rupt_cases.c). */
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s>>9;}
static const uint8_t vectors[RR_VECTORS]={RK_RUPT_T6,RK_RUPT_T5,RK_RUPT_T4,RK_RUPT_KEY1,RK_RUPT_KEY2,RK_RUPT_UP,
 RK_RUPT_DOWN,RK_RUPT_RADAR,RK_RUPT_HAND};
uint8_t rr_vector(uint16_t n){return vectors[n/RR_VECTOR_PER%RR_VECTORS];}
/* KEYRUPT1 (LM 04,3274, CM 07,3673) and UPRUPT (KEYRUPT1 +023), where the
 * KEYRUPT1 and UPRUPT lead-ins go. */
#define KEYRUPT1(m) ((m)?03673u:03274u)
void rr_vector_input(unsigned mission,uint16_t n,rr_vcase *c){uint32_t r=0x56454354u+mission*0x9e3779b9u+n*2654435761u;
 uint8_t v=rr_vector(n);uint16_t j=(uint16_t)(n%RR_VECTOR_PER),k;agc_word z,w;apollo_overlay_state *s=c->s;
 memset(&c->lm,0,sizeof c->lm);memset(&c->cm,0,sizeof c->cm);memset(&c->hw,0,sizeof c->hw);
 if(v==RK_RUPT_T4){static t4_io io;t4_input(mission,s,&io,j);c->lm=io.lm;c->cm=io.cm;c->hw=io.hw;}
 else if(v==RK_RUPT_KEY1||v==RK_RUPT_UP){static ku_io io;agc_word e=(agc_word)(KEYRUPT1(mission)+(v==RK_RUPT_UP?023u:0u));
  for(k=0;k<KU_ENTRIES-1u&&ku_entry(mission,(uint16_t)(k*KU_PER_ENTRY))!=e;++k){}
  ku_input(mission,s,&io,(uint16_t)(k*KU_PER_ENTRY+j));c->lm=io.lm;c->cm=io.cm;c->hw=io.hw;}
 else if(v==RK_RUPT_T6&&!mission)lm_t6_input(s,&c->lm,&c->hw,(uint16_t)(lm_t6_first(LM_T6_VECTOR)+j));
 else{static rli_io io;unsigned cases=rli_cases(mission);
  for(k=0;(unsigned)(k+1u)*RLI_PER_ENTRY<cases&&rli_entry(mission,(uint16_t)(k*RLI_PER_ENTRY))!=04000u+4u*v;++k){}
  rli_input(mission,s,&io,(uint16_t)(k*RLI_PER_ENTRY+j));c->lm=io.lm;c->cm=io.cm;c->hw=io.hw;
  /* A 2CADR names fixed-fixed code by its fixed-fixed address (yaYUL), never
   * by its banked alias: run through the alias, RESUME's XCH BBANK switches
   * the bank under Z, which the canonical resolver keys do not model. */
  for(k=01274u;k<=01312u;k=(uint16_t)(k==01274u?01310u:k+2u)){agc_word a=s->words[k],b=(agc_word)(s->words[k+1u]&076000u);
   if(a<04000u&&(b==04000u||b==06000u))s->words[k]=(agc_word)(b+(a&01777u));}}
 /* Before the interrupt: Z on an unswitched erasable word that is not
  * RELINT, INHINT or EXTEND, A without overflow, interrupts enabled. */
 z=(agc_word)(060u+rnd(&r)%(01400u-060u));
 for(;;){w=(agc_word)(s->words[z]&AGC_MASK);if(w!=3u&&w!=4u&&w!=6u)break;z=(agc_word)(z==01377u?060u:z+1u);}
 s->words[5]=z;
 s->words[0]&=0137777u;s->words[0]=agc_extend(agc_correct(s->words[0]));
 c->lm.interrupt_enabled=1;c->cm.interrupt_enabled=1;c->hw.in_interrupt=0;
 c->pending=(uint16_t)(1u<<v);
}
uint32_t rr_vector_digest(unsigned mission,const rr_vcase *c,uint8_t kind){uint32_t h=rr_mix(RR_BASIS,kind);unsigned i;
 if(kind>RR_V_STOPPED)return h;
 for(i=0;i<2048u;++i)h=rr_mix(h,(uint16_t)(c->s->words[i]&(i<3u?0177777u:077777u)));
 h=rr_mix(h,(uint16_t)((mission?c->cm.superbank:c->lm.superbank)&0160u));
 h=rr_mix(h,(uint16_t)(mission?c->cm.interrupt_enabled:c->lm.interrupt_enabled));h=rr_mix(h,c->hw.in_interrupt);
 h=rr_mix(h,(uint16_t)(c->hw.channel5&077777u));h=rr_mix(h,(uint16_t)(c->hw.channel6&077777u));
 h=rr_mix(h,(uint16_t)(c->hw.channel10&077777u));h=rr_mix(h,(uint16_t)(c->hw.channel11&077777u));
 return rr_mix(h,(uint16_t)(c->hw.channel13&077777u));}
