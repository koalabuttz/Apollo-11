/* SPDX-License-Identifier: MIT */
#include "cm_downtelemetry_cases.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static const agc_word dumpcnt[CM_DNTM_DUMP_CASES]={004000u,010000u,020000u};
uint16_t cm_dntm_steps(uint16_t n){
 if(n<CM_DNTM_CASES-CM_DNTM_DUMP_CASES)return 240u;
 return (uint16_t)(1100u<<(n-(CM_DNTM_CASES-CM_DNTM_DUMP_CASES)));
}
void cm_dntm_begin(apollo_overlay_state *s,cm_dntm_io *io,uint16_t n){
 unsigned i;uint32_t seed=UINT32_C(0x3521)+(uint32_t)n*UINT32_C(977);
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 s->words[7]=0;s->words[0]=agc_extend(s->words[0]);s->words[1]=agc_extend(s->words[1]);
 {static const cm_dntm_io z;*io=z;}
 if(n<CM_DNTM_CASES-CM_DNTM_DUMP_CASES){s->words[CM_DNTM_DNTMGOTO]=CM_DNTM_DNPHASE1;s->words[CM_DNTM_DNLSTCOD]=(agc_word)(n%5u);}
 else{s->words[CM_DNTM_DNTMGOTO]=CM_DNTM_DNDUMPI;s->words[CM_DNTM_DNLSTCOD]=(agc_word)(n%5u);
  s->words[CM_DNTM_DUMPCNT]=dumpcnt[n-(CM_DNTM_CASES-CM_DNTM_DUMP_CASES)];}
}
void cm_dntm_entry(apollo_overlay_state *s,cm_dntm_io *io,uint16_t n,uint16_t step){
 uint32_t seed=UINT32_C(0x4040)+(uint32_t)n*UINT32_C(131071)+(uint32_t)step*UINT32_C(8191);agc_word bb;
 (void)rnd(&seed);
 bb=(agc_word)((((rnd(&seed)>>16)%040u)<<10)|((rnd(&seed)>>20)&7u));
 s->words[0]=agc_extend(bb);                  /* XCH BBANK: the interrupted BB */
 s->words[2]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 s->words[4]=(agc_word)(05u<<10);s->words[3]=0;s->words[6]=s->words[4];
 s->words[5]=CM_DNTM_DODOWNTM;
 io->io.channel11=(agc_word)((rnd(&seed)>>12)&AGC_MASK);io->io.channel12=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 io->io.channel13=(agc_word)((rnd(&seed)>>12)&043777u);io->io.channel14=(agc_word)((rnd(&seed)>>12)&037u); /* no CDU, optics or gyro drive bits */
 io->channel30=(agc_word)((rnd(&seed)>>12)&AGC_MASK);io->channel31=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 io->channel32=(agc_word)((rnd(&seed)>>12)&AGC_MASK);io->channel33=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
}
enum exec_status cm_dntm_downrupt(apollo_overlay_state *s,cm_dntm_io *io){
 agc_native_program program={&cm_downtelemetry_routines,1,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};uint16_t steps;
 return agc_transfer_run(&program,s,io,&io->io.superbank,CM_DNTM_RESUME,8,&steps);
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t cm_dntm_digest(apollo_overlay_state *s,uint16_t n,enum exec_status *status){
 cm_dntm_io io;uint16_t k,steps=cm_dntm_steps(n);unsigned i;
 sum_a=sum_b=0;*status=EXEC_OK;cm_dntm_begin(s,&io,n);
 for(k=0;k<steps;++k){cm_dntm_entry(s,&io,n,k);
  if((*status=cm_dntm_downrupt(s,&io))!=EXEC_OK)return 0;
  mix(io.io.channel13);mix(io.channel34);mix(io.channel35);}
 mix(io.io.trap31a);mix(io.io.trap31b);mix(io.io.trap32);
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
