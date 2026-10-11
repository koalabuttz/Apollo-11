/* SPDX-License-Identifier: MIT */
#include "p66_startup_cases.h"
static agc_word randword(uint32_t *p) { *p=*p*1664525u+1013904223u; return (agc_word)((*p>>16)&AGC_MASK); }
void p66_startup_prepare(p66_startup_case *c,uint32_t n) {
 uint32_t seed=n%8u+831; uint8_t i,j;
 if(n<65536UL) {
  c->mode=0; c->value.op.a=(agc_register)((n>=32768UL ? 0100 : 040)|((n/4096)&AGC_MASK));
  c->value.op.rodcount=(agc_word)(n%32768UL);
 } else if(n<69632UL) {
  executive *e=&c->value.task.exec; p66_task_io *io=&c->value.task.io;
  uint8_t core=(uint8_t)(n%9u),vac=(uint8_t)((n/9)%6u);
  c->mode=1;
  for(i=0;i<EXEC_CORE_COUNT;++i) {
   for(j=0;j<EXEC_CORE_WORDS;++j) e->core[i][j]=randword(&seed);
   e->core[i][EXEC_PRIORITY]=i==core ? AGC_NEG_ZERO : (agc_word)(01000+((i*3+n)%31)*01000);
  }
  for(i=0;i<EXEC_VAC_COUNT;++i) {
   for(j=0;j<EXEC_VAC_WORDS;++j) e->vac[i][j]=randword(&seed);
   e->vac[i][0]=i==vac ? (agc_word)(0400+i*44u) : 0;
  }
  e->newjob=core==0 ? AGC_NEG_ZERO : 0;
  e->newprio=randword(&seed); e->newloc[0]=randword(&seed); e->newloc[1]=randword(&seed);
  e->locctr=randword(&seed); e->fixloc=randword(&seed); e->ovfind=randword(&seed); e->alarm=0;
  io->a=randword(&seed); io->l=randword(&seed); io->q=randword(&seed);
  io->bbank=(agc_word)(064000+(n%8u)); io->superbank=(agc_word)((n/8u)%4u*020u);
  io->interrupt_enabled=(agc_word)(n%2u); io->exectem1=randword(&seed); io->exectem2=randword(&seed);
 } else {
  phase_state *p=&c->value.vert.phase; waitlist *w=&c->value.vert.wait;
  static const agc_word timers[]={037777,030000,0,1,0177,0200,037634,037635};
  uint8_t style=(uint8_t)((n/8u)%8u);
  c->mode=2;
  for(i=0;i<6;++i) {
   p->records.complement[i]=randword(&seed); p->records.phase[i]=randword(&seed);
   p->records.priority[i]=randword(&seed); p->records.address[i].location=randword(&seed);
   p->records.address[i].bank=randword(&seed); p->tbase[i]=randword(&seed);
  }
  for(i=0;i<2;++i) { p->longbase[i]=randword(&seed); p->time[i]=randword(&seed); }
  for(i=0;i<PH_SCRATCH_COUNT;++i) p->scratch[i]=randword(&seed);
  p->a=randword(&seed); p->l=randword(&seed); p->q=randword(&seed);
  p->bbank=(agc_word)(064000+n%8u); p->superbank=(agc_word)((n/64u)%4u*020u); p->interrupt_enabled=(agc_word)(n%2u);
  w->time3=timers[n%8u]; w->ruptagn=n%2u ? 1 : AGC_NEG_ZERO; w->alarm=0;
  for(i=0;i<WL_INTERVAL_COUNT;++i) w->interval[i]=style==0 ? WL_DEFAULT_INTERVAL : style==1 ? 1 : style==2 ? 0 : style==3 ? AGC_NEG_ZERO : (agc_word)(AGC_MASK-(10u+i*(style-3u)));
  for(i=0;i<WL_TASK_COUNT;++i) {
   w->task[i].location=i==8 && style%2u==0 ? WL_END_LOCATION : (agc_word)(02200+i);
   w->task[i].bank=(agc_word)(012000+i*020u+(i%8u));
  }
 }
}
uint32_t p66_startup_target(uint8_t n) {
 return n<32 ? (uint32_t)(n/2u)*1024u+(n%2u)*UINT32_C(32768) : n<80 ? UINT32_C(65536)+(uint32_t)(n-32u)*83u : UINT32_C(69632)+(uint32_t)(n-80u)*83u;
}
void p66_startup_words(const p66_startup_case *c,void (*emit)(void *,agc_word),void *u) {
 uint8_t i,j;
 if(c->mode==0) { emit(u,c->value.op.a); emit(u,c->value.op.rodcount); }
 else if(c->mode==1) {
  const executive *e=&c->value.task.exec; const p66_task_io *io=&c->value.task.io;
  for(i=0;i<EXEC_CORE_COUNT;++i) for(j=0;j<EXEC_CORE_WORDS;++j) emit(u,e->core[i][j]);
  for(i=0;i<EXEC_VAC_COUNT;++i) for(j=0;j<EXEC_VAC_WORDS;++j) emit(u,e->vac[i][j]);
  emit(u,e->newjob); emit(u,e->newprio); emit(u,e->newloc[0]); emit(u,e->newloc[1]);
  emit(u,e->locctr); emit(u,e->fixloc); emit(u,e->ovfind); emit(u,e->alarm);
  emit(u,io->a); emit(u,io->l); emit(u,io->q); emit(u,io->bbank); emit(u,io->superbank); emit(u,io->interrupt_enabled); emit(u,io->exectem1); emit(u,io->exectem2);
 } else {
  const phase_state *p=&c->value.vert.phase; const waitlist *w=&c->value.vert.wait;
  for(i=0;i<6;++i) { emit(u,p->records.complement[i]); emit(u,p->records.phase[i]); emit(u,p->records.address[i].location); emit(u,p->records.address[i].bank); emit(u,p->records.priority[i]); emit(u,p->tbase[i]); }
  for(i=0;i<2;++i) emit(u,p->longbase[i]);
  for(i=0;i<2;++i) emit(u,p->time[i]);
  for(i=0;i<PH_SCRATCH_COUNT;++i) emit(u,p->scratch[i]);
  emit(u,p->a); emit(u,p->l); emit(u,p->q); emit(u,p->bbank); emit(u,p->superbank); emit(u,p->interrupt_enabled);
  emit(u,w->time3);
  for(i=0;i<WL_INTERVAL_COUNT;++i) emit(u,w->interval[i]);
  for(i=0;i<WL_TASK_COUNT;++i) { emit(u,w->task[i].location); emit(u,w->task[i].bank); }
  emit(u,w->ruptagn); emit(u,w->alarm);
 }
}
#ifndef P66_STARTUP_INPUT_ONLY
agc_word p66_startup_apply(p66_startup_case *c) {
 if(c->mode==0) { p66_descbits(&c->value.op); return 0; }
 if(c->mode==1) return (agc_word)p66_rodtask(&c->value.task.exec,&c->value.task.io);
 return (agc_word)p66_verta(&c->value.vert.phase,&c->value.vert.wait);
}
#endif
