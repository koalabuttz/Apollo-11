/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only original ENEMA adapter; native cleanup/validation not linked. */
#define main original_reference_main
#include "reference_runner.c"
#undef main
#include "enema_cases.h"
#include "restart_addresses.h"
static void load_case(const restart_case *c) {
    unsigned i,j;
    for(i=0;i<8;++i) for(j=0;j<12;++j) set(0154+i*12+j,c->exec.core[i][j]);
    for(i=0;i<5;++i) for(j=0;j<44;++j) set(0400+i*44+j,c->exec.vac[i][j]);
    set(0067,c->exec.newjob); set(0063,c->exec.newprio); set(0065,c->exec.newloc[0]); set(0066,c->exec.newloc[1]);
    set(0064,c->exec.locctr); set(0120,c->exec.fixloc); set(0121,c->exec.ovfind);
    set(026,c->wait.time3); set(0734,c->wait.ruptagn);
    for(i=0;i<8;++i) set(01400+i,c->wait.interval[i]);
    for(i=0;i<9;++i) { set(01410+i*2,c->wait.task[i].location); set(01411+i*2,c->wait.task[i].bank); }
    for(i=0;i<3;++i) set(0375+i,c->record.failreg[i]);
    for(i=0;i<2;++i) set(01363+i,c->record.almcadr[i]);
    for(i=0;i<12;++i) set(01023+i,c->record.dsptab[i]);
    for(i=0;i<RS_WORD_COUNT;++i) set(rs_addresses[i],c->record.word[i]);
    machine.InputChannel[011]=c->record.channel11; machine.InputChannel[012]=c->record.channel12;
    machine.Trap31A=c->record.trap31a; machine.Trap31B=c->record.trap31b; machine.Trap32=c->record.trap32;
    machine.InputChannel[013]=c->record.channel13; machine.InputChannel[014]=c->record.channel14;
    for(i=0;i<6;++i) {
        set(0752+i*2,c->phases.complement[i]); set(0753+i*2,c->phases.phase[i]);
        set(01436+i*2,c->phases.address[i].location); set(01437+i*2,c->phases.address[i].bank);
        set(01054+i*2,c->phases.priority[i]);
    }
}
static void read_case(restart_case *c) {
    unsigned i,j;
    for(i=0;i<8;++i) for(j=0;j<12;++j) c->exec.core[i][j]=(agc_word)get(0154+i*12+j);
    for(i=0;i<5;++i) for(j=0;j<44;++j) c->exec.vac[i][j]=(agc_word)get(0400+i*44+j);
    c->exec.newjob=(agc_word)get(0067); c->exec.newprio=(agc_word)get(0063);
    c->exec.newloc[0]=(agc_word)get(0065); c->exec.newloc[1]=(agc_word)get(0066);
    c->exec.locctr=(agc_word)get(0064); c->exec.fixloc=(agc_word)get(0120); c->exec.ovfind=(agc_word)get(0121);
    c->wait.time3=(agc_word)get(026); c->wait.ruptagn=(agc_word)get(0734);
    for(i=0;i<8;++i) c->wait.interval[i]=(agc_word)get(01400+i);
    for(i=0;i<9;++i) { c->wait.task[i].location=(agc_word)get(01410+i*2); c->wait.task[i].bank=(agc_word)get(01411+i*2); }
    for(i=0;i<3;++i) c->record.failreg[i]=(agc_word)get(0375+i);
    for(i=0;i<2;++i) c->record.almcadr[i]=(agc_word)get(01363+i);
    for(i=0;i<12;++i) c->record.dsptab[i]=(agc_word)get(01023+i);
    for(i=0;i<RS_WORD_COUNT;++i) c->record.word[i]=(agc_word)get(rs_addresses[i]);
    c->record.channel11=(agc_word)machine.InputChannel[011]; c->record.channel12=(agc_word)machine.InputChannel[012];
    c->record.channel13=(agc_word)machine.InputChannel[013]; c->record.channel14=(agc_word)machine.InputChannel[014];
    c->record.trap31a=(agc_word)machine.Trap31A; c->record.trap31b=(agc_word)machine.Trap31B; c->record.trap32=(agc_word)machine.Trap32;
    for(i=0;i<6;++i) {
        c->phases.complement[i]=(agc_word)get(0752+i*2); c->phases.phase[i]=(agc_word)get(0753+i*2);
        c->phases.address[i].location=(agc_word)get(01436+i*2); c->phases.address[i].bank=(agc_word)get(01437+i*2);
        c->phases.priority[i]=(agc_word)get(01054+i*2);
    }
}

static void emit(void *u,agc_word w) { (void)u; printf(" %u",w); }
int main(int argc,char **argv) {
 static const unsigned guards[]={01053,01055,01057,01061,01063,01065,01151,01152,0130,0131,0132,01434,01452};
 unsigned n,i,cycles,pc; if(argc!=2) return 2;
 for(n=0;n<ENEMA_CASE_COUNT;++n) {
  enema_case c; int result; memset(&machine,0,sizeof machine);
  if(agc_load_binfile(&machine,argv[1])) return 2;
  enema_prepare(&c,(uint16_t)n); load_case(&c.previous); InhibitAlarms=1;
  for(i=0;i<sizeof guards/sizeof guards[0];++i) set(guards[i],(agc_word)(014731+i));
  set(0106,c.io.flagword10); machine.InputChannel[015]=c.io.main_keys;
  machine.InputChannel[016]=c.io.navigation_keys;
  machine.Erasable[0][RegA]=(int16_t)c.io.a; machine.Erasable[0][RegL]=(int16_t)c.io.l;
  set(RegQ,c.io.q); set(RegBB,c.io.bbank); set(RegFB,012000); set(RegEB,(agc_word)((c.io.bbank&7)<<8));
  machine.InputChannel[7]=machine.OutputChannel7=c.io.superbank;
  machine.AllowInterrupt=c.io.interrupt_enabled; set(RegZ,02765);
  for(cycles=0;cycles<10000;++cycles) {
   pc=get(RegZ);
   if(!machine.PendFlag && !machine.ExtraDelay && (pc==05315 || pc==03054 || pc==03107)) break;
   machine.ScalerCounter=0; if(agc_engine(&machine)) return 2;
  }
  if(cycles==10000) { fprintf(stderr,"enema %u timeout PC%05o BB%05o\n",n,pc,get(RegBB)); return 2; }
  result=pc==05315 ? ENEMA_DISPLAY : pc==03054 ? ENEMA_BAD_PHASE : ENEMA_FRESH_START;
  for(i=0;i<sizeof guards/sizeof guards[0];++i) if(get(guards[i])!=014731+i) { fprintf(stderr,"enema %u guard %05o changed\n",n,guards[i]); return 2; }
  read_case(&c.previous); c.io.flagword10=(agc_word)get(0106);
  c.io.a=(agc_register)((unsigned)machine.Erasable[0][RegA]&0177777u);
  c.io.l=(agc_register)((unsigned)machine.Erasable[0][RegL]&0177777u);
  c.io.q=(agc_word)get(RegQ); c.io.bbank=(agc_word)get(RegBB);
  c.io.superbank=(agc_word)machine.OutputChannel7; c.io.interrupt_enabled=(agc_word)machine.AllowInterrupt;
  c.io.main_keys=(agc_word)machine.InputChannel[015]; c.io.navigation_keys=(agc_word)machine.InputChannel[016];
  printf("%u %d",n,result); enema_words(&c,emit,0); putchar('\n');
  fprintf(stderr,"enema %u: %u cycles PC=%05o BB=%05o result=%d\n",n,cycles,pc,get(RegBB),result);
 }
 return 0;
}
