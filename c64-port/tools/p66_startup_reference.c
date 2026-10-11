/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only unchanged Luminary099 assembly oracle. No native startup,
 * phase, Waitlist, Executive or arithmetic implementation linked. */
#define main original_reference_main
#include "reference_runner.c"
#undef main
#include "p66_startup_cases.h"
static void word(agc_word *v,unsigned a,int read) { if(read) *v=(agc_word)get(a); else set(a,*v); }
static void registers(agc_register *a,agc_register *l,agc_word *q,agc_word *bb,agc_word *super,agc_word *irq,int read) {
 if(read) { *a=(agc_register)((unsigned)machine.Erasable[0][RegA]&0177777u); *l=(agc_register)((unsigned)machine.Erasable[0][RegL]&0177777u); *q=(agc_word)get(RegQ); *bb=(agc_word)get(RegBB); *super=(agc_word)machine.OutputChannel7; *irq=(agc_word)machine.AllowInterrupt; }
 else { machine.Erasable[0][RegA]=(int16_t)*a; machine.Erasable[0][RegL]=(int16_t)SignExtend(*l); set(RegQ,*q); set(RegBB,*bb); set(RegEB,(agc_word)((*bb&7u)<<8)); set(RegFB,(agc_word)(*bb&076000)); machine.OutputChannel7=machine.InputChannel[7]=*super; machine.AllowInterrupt=*irq; }
}
static void transfer(p66_startup_case *c,int read) {
 unsigned i,j;
 if(c->mode==0) {
  if(read) c->value.op.a=(agc_register)((unsigned)machine.Erasable[0][RegA]&0177777u);
  else { machine.Erasable[0][RegA]=(int16_t)c->value.op.a; set(RegBB,040007); set(RegFB,040000); set(RegEB,03400); }
  word(&c->value.op.rodcount,03746,read);
 } else if(c->mode==1) {
  executive *e=&c->value.task.exec; p66_task_io *io=&c->value.task.io;
  for(i=0;i<EXEC_CORE_COUNT;++i) for(j=0;j<EXEC_CORE_WORDS;++j) word(&e->core[i][j],0154+i*12+j,read);
  for(i=0;i<EXEC_VAC_COUNT;++i) for(j=0;j<EXEC_VAC_WORDS;++j) word(&e->vac[i][j],0400+i*44+j,read);
  word(&e->newjob,067,read); word(&e->newprio,063,read); word(&e->newloc[0],065,read); word(&e->newloc[1],066,read);
  word(&e->locctr,064,read); word(&e->fixloc,0120,read); word(&e->ovfind,0121,read);
  word(&io->exectem1,061,read); word(&io->exectem2,062,read);
  registers(&io->a,&io->l,&io->q,&io->bbank,&io->superbank,&io->interrupt_enabled,read);
 } else {
  phase_state *p=&c->value.vert.phase; waitlist *w=&c->value.vert.wait;
  for(i=0;i<6;++i) {
   word(&p->records.complement[i],0752+2*i,read); word(&p->records.phase[i],0753+2*i,read);
   word(&p->records.address[i].location,01436+2*i,read); word(&p->records.address[i].bank,01437+2*i,read);
   word(&p->records.priority[i],01054+2*i,read); word(&p->tbase[i],01053+2*i,read);
  }
  for(i=0;i<2;++i) { word(&p->longbase[i],01151+i,read); word(&p->time[i],024+i,read); }
  for(i=0;i<PH_SCRATCH_COUNT;++i) word(&p->scratch[i],i<6 ? 061+i : 070+i-6,read);
  registers(&p->a,&p->l,&p->q,&p->bbank,&p->superbank,&p->interrupt_enabled,read);
  word(&w->time3,026,read);
  for(i=0;i<WL_INTERVAL_COUNT;++i) word(&w->interval[i],01400+i,read);
  for(i=0;i<WL_TASK_COUNT;++i) { word(&w->task[i].location,01410+2*i,read); word(&w->task[i].bank,01411+2*i,read); }
  word(&w->ruptagn,0734,read);
 }
}
static void emit(void *u,agc_word w) { (void)u; printf(" %u",w); }
int main(int argc,char **argv) {
 static const unsigned guards[]={0130,0131,0135,0140,0141,0377,01434,01452};
 uint32_t n; unsigned i,cycles,pc,result; static agc_t pristine;
 if(argc!=2) return 2;
 memset(&pristine,0,sizeof pristine);
 if(agc_load_binfile(&pristine,argv[1])) return 2;
 for(n=0;n<P66_STARTUP_CASE_COUNT;++n) {
  p66_startup_case c; memcpy(&machine,&pristine,sizeof machine);
  p66_startup_prepare(&c,n); transfer(&c,0); InhibitAlarms=1;
  for(i=0;i<sizeof guards/sizeof guards[0];++i) set(guards[i],(agc_word)(012345+i));
  set(RegZ,c.mode==0 ? 02115 : c.mode==1 ? 03263 : 03270); result=0;
  for(cycles=0;cycles<5000;++cycles) {
   pc=get(RegZ);
   if(!machine.PendFlag && !machine.ExtraDelay) {
    if((c.mode==0 && pc==05270) || (c.mode==1 && pc==05261) || (c.mode==2 && pc==03275 && get(RegFB)==064000)) break;
    if(pc==05716) {
     unsigned q=get(RegQ); result=(unsigned)machine.Fixed[1][q&01777]&077777u; break;
    }
   }
   machine.ScalerCounter=0; if(agc_engine(&machine)) return 2;
  }
  if(cycles==5000) { fprintf(stderr,"startup %lu timeout PC%05o BB%05o\n",(unsigned long)n,pc,get(RegBB)); return 2; }
  for(i=0;i<sizeof guards/sizeof guards[0];++i) if(get(guards[i])!=012345+i) { fprintf(stderr,"guard%05o case%lu\n",guards[i],(unsigned long)n); return 2; }
  transfer(&c,1);
  if(c.mode==1) c.value.task.exec.alarm=(agc_word)result;
  if(c.mode==2) c.value.vert.wait.alarm=(agc_word)result;
  printf("%lu %u",(unsigned long)n,result); p66_startup_words(&c,emit,0); putchar('\n');
  if(n>=65536) fprintf(stderr,"startup%lu: %u cycles PC%05o BB%05o\n",(unsigned long)n,cycles,pc,get(RegBB));
 }
 return 0;
}
