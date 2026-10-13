/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "keyrupt_uprupt_cases.h"
static const agc_word lm_z[KU_ENTRIES]={03274,03300,03303,03313,03317,03323};
static const agc_word cm_z[KU_ENTRIES]={03673,03677,03702,03712,03716,03722};
agc_word ku_entry(unsigned mission,uint16_t n){return mission?cm_z[n/KU_PER_ENTRY]:lm_z[n/KU_PER_ENTRY];}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s>>9;}
/* An uplink word: high and low five bits the code, the middle five its
 * complement, when it passes the redundancy test. */
static agc_word uplink(uint32_t *r){uint32_t x=rnd(r)%6u;agc_word c=(agc_word)(x==0u?022u:rnd(r)&037u);
 return x<2u?(agc_word)(c<<10|((~c&037u)<<5)|c):(agc_word)(rnd(r)&AGC_MASK);}
void ku_input(unsigned mission,apollo_overlay_state *m,ku_io *io,uint16_t n){uint32_t r=0x0e0e1cc5u+mission*0x9e3779b9u+n*2654435761u;unsigned i;agc_word fb=mission?016000u:010000u,eb,z=ku_entry(mission,n);
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)(rnd(&r)&AGC_MASK);
 m->words[7]=0;m->words[035]=0;m->words[036]=0;m->words[047]=0;for(i=050;i<=054;++i)m->words[i]=0;
 /* A as the lead-in or caller leaves it: a 15-bit value, no overflow. */
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
 eb=(agc_word)(rnd(&r)%8u);m->words[3]=(agc_word)(eb<<8);m->words[4]=fb;m->words[6]=(agc_word)(fb|eb);m->words[5]=z;
 m->words[045]=uplink(&r);
 m->words[064]=(agc_word)(12u*(rnd(&r)%7u));
 m->words[0103]=(agc_word)(rnd(&r)&1u?m->words[0103]|010u:m->words[0103]&~010u);
 memset(io,0,sizeof *io);
 if(mission){io->cm.superbank=(agc_word)((rnd(&r)%8u)*020u);io->cm.interrupt_enabled=0;}
 else{io->lm.superbank=(agc_word)((rnd(&r)%8u)*020u);io->lm.interrupt_enabled=0;}
 io->hw.in_interrupt=1;io->hw.channel11=(agc_word)(rnd(&r)&AGC_MASK);io->hw.channel15=(agc_word)(rnd(&r)&AGC_MASK);
}
unsigned ku_run(unsigned mission,apollo_overlay_state *s,ku_io *io,uint16_t n){
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 program.basic=mission?&cm_keyrupt_routines:&lm_keyrupt_routines;program.basic_count=1;program.hardware=&io->hw;
 ku_input(mission,s,io,n);
 call=agc_transfer_resolve(&program,s,mission?io->cm.superbank:io->lm.superbank);
 if(!call)return 3u;
 return call(&program,s,mission?(void *)&io->cm:(void *)&io->lm)==EXEC_OK?KU_RETURNED:KU_REFUSED;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t ku_digest(unsigned mission,apollo_overlay_state *s,uint16_t n){static ku_io io;unsigned i,kind=ku_run(mission,s,&io,n);
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=KU_REFUSED){for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<3u?0177777u:077777u)));
  mix(mission?io.cm.superbank:io.lm.superbank);mix(mission?io.cm.interrupt_enabled:io.lm.interrupt_enabled);mix(io.hw.in_interrupt);mix(io.hw.channel11);}
 return (uint32_t)sum_b<<16|sum_a;
}
