/* SPDX-License-Identifier: GPL-2.0-or-later; original CM services oracle, input-only constructors. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_restart_cases.h"
#include "cm_restart_addresses.h"
static void load_case(const cm_restart_case *c) {
    unsigned i,j;
    for(i=0;i<7;++i) for(j=0;j<12;++j) set(0154+i*12+j,c->exec.core[i][j]);
    for(i=0;i<5;++i) for(j=0;j<44;++j) set(0400+i*44+j,c->exec.vac[i][j]);
    set(0067,c->exec.newjob); set(0063,c->exec.newprio); set(0065,c->exec.newloc[0]); set(0066,c->exec.newloc[1]);
    set(0064,c->exec.locctr); set(0120,c->exec.fixloc); set(0121,c->exec.ovfind);
    set(026,c->wait.time3); set(0734,c->wait.ruptagn);
    for(i=0;i<8;++i) set(01400+i,c->wait.interval[i]);
    for(i=0;i<9;++i) { set(01410+i*2,c->wait.task[i].location); set(01411+i*2,c->wait.task[i].bank); }
    for(i=0;i<3;++i) set(0375+i,c->record.failreg[i]);
    for(i=0;i<2;++i) set(01363+i,c->record.almcadr[i]);
    for(i=0;i<12;++i) set(01023+i,c->record.dsptab[i]);
    for(i=0;i<CM_RS_WORD_COUNT;++i) set(cm_rs_addresses[i],c->record.word[i]);
    machine.InputChannel[011]=c->record.channel11; machine.InputChannel[012]=c->record.channel12;
    machine.Trap31A=c->record.trap31a; machine.Trap31B=c->record.trap31b; machine.Trap32=c->record.trap32;
    machine.InputChannel[013]=c->record.channel13; machine.InputChannel[014]=c->record.channel14;
    for(i=0;i<6;++i) {
        set(0752+i*2,c->phases.complement[i]); set(0753+i*2,c->phases.phase[i]);
        set(01436+i*2,c->phases.address[i].location); set(01437+i*2,c->phases.address[i].bank);
        set(01054+i*2,c->phases.priority[i]);
    }
}
static void read_case(cm_restart_case *c) {
    unsigned i,j;
    for(i=0;i<7;++i) for(j=0;j<12;++j) c->exec.core[i][j]=(agc_word)get(0154+i*12+j);
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
    for(i=0;i<CM_RS_WORD_COUNT;++i) c->record.word[i]=(agc_word)get(cm_rs_addresses[i]);
    c->record.channel11=(agc_word)machine.InputChannel[011]; c->record.channel12=(agc_word)machine.InputChannel[012];
    c->record.channel13=(agc_word)machine.InputChannel[013]; c->record.channel14=(agc_word)machine.InputChannel[014];
    c->record.trap31a=(agc_word)machine.Trap31A; c->record.trap31b=(agc_word)machine.Trap31B; c->record.trap32=(agc_word)machine.Trap32;
    for(i=0;i<6;++i) {
        c->phases.complement[i]=(agc_word)get(0752+i*2); c->phases.phase[i]=(agc_word)get(0753+i*2);
        c->phases.address[i].location=(agc_word)get(01436+i*2); c->phases.address[i].bank=(agc_word)get(01437+i*2);
        c->phases.priority[i]=(agc_word)get(01054+i*2);
    }
}
static void emit(void *u,agc_word w) { (void)u;printf(" %u",w); }
int main(int argc,char **argv) {
 unsigned n,i,cycles,pc=0;if(argc!=2) return 2;
 for(n=0;n<1280;++n) {
  cm_restart_case c;int status=0;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1])) return 2;
  cm_rs_prepare(&c,(uint16_t)n);load_case(&c);InhibitAlarms=1;
  for(i=0;i<16;++i) set(0300+i,(agc_word)(01000+i));
  set(RegBB,012003);set(RegEB,01400);set(RegFB,012000);machine.InputChannel[7]=machine.OutputChannel7=0100;machine.AllowInterrupt=0;
  set(RegA,0);set(RegL,0);set(RegQ,01010);
  set(RegZ,n<256 ? 03006 : n<512 ? 02776 : n<768 ? 02667 : 03520);
  if(n>=768) { set(RegFB,02000);set(RegBB,02003);set(0154,c.phases.phase[c.group]);set(0161,c.group); }
  for(cycles=0;cycles<8000;++cycles) {
   pc=get(RegZ);
   if(!machine.PendFlag && !machine.ExtraDelay) {
    if(n<512 && pc==01010) { c.exec.alarm=0;c.wait.alarm=0;break; }
    if(n>=512 && n<768 && pc==05360) break;
    if(n>=512 && n<768 && pc==02746 && get(RegFB)==012000) { status=CM_RS_PHASE_BAD;break; }
    if(n>=768 && pc==05222) break;
    if(n>=768 && pc==07755) { status=machine.Fixed[1][get(RegQ)&01777]&077777;c.exec.alarm=(agc_word)status;break; }
   }
   machine.ScalerCounter=0;if(agc_engine(&machine)) return 2;
  }
  if(cycles==8000) { fprintf(stderr,"CM restart%u timeout PC%05o\n",n,pc);return 2; }
  for(i=0;i<16;++i) if(get(0300+i)!=01000+i) { fprintf(stderr,"CM restart guard%u/%u\n",n,i);return 2; }
  read_case(&c);printf("%u %d",n,status);cm_rs_words(&c,emit,0);putchar('\n');
  fprintf(stderr,"CM restart %u: %u cycles PC%05o BB%05o SB%03o status%d\n",n,cycles,pc,get(RegBB),machine.OutputChannel7,status);
 }return 0;
}
