/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original source oracle.
 * Erasable two-instruction callback bodies are explicit test fixtures, not
 * translated flight programs. Executive/restart source is original rope. */
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
#include "cm_restart_native_cases.h"
static void physical_bank(unsigned b) {
 set(RegBB,(agc_word)(b&076007));set(RegFB,(agc_word)(b&076000));
 set(RegEB,(agc_word)((b&7u)<<8));machine.InputChannel[7]=machine.OutputChannel7=(int)(b&0160);
}
static void run_stop(unsigned location,unsigned bank) {
 unsigned cycles;
 for(cycles=0;cycles<10000;++cycles) {
  if(!machine.PendFlag && !machine.ExtraDelay && get(RegZ)==location &&
     (bank==077777 || get(RegBB)==(bank&076007))) return;
  machine.ScalerCounter=0;if(agc_engine(&machine)) exit(2);
 }
 fprintf(stderr,"CM native restart timeout PC%05o FB%05o stop%05o\n",get(RegZ),get(RegFB),location);exit(2);
}
static exec_address candidate_address(const cm_restart_case *c) {
 unsigned i,slot=0,highest=0;exec_address a={0,0};
 for(i=1;i<7;++i) if(c->exec.core[i][11]<040000 && c->exec.core[i][11]>highest) {
  highest=c->exec.core[i][11];slot=i;
 }
 if(slot) { a.location=c->exec.core[slot][8];a.bank=c->exec.core[slot][9]; }
 return a;
}
int main(int argc,char **argv) {
 unsigned n,step,i;if(argc!=2) return 2;
 for(n=0;n<64;++n) {
  cm_restart_case c;exec_address current={0,0};unsigned callbacks=0;int status=EXEC_OK;
  memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1])) return 2;
  cm_rn_prepare(&c,(uint8_t)n);load_case(&c);InhibitAlarms=1;machine.AllowInterrupt=0;
  for(i=0;i<16;++i) set(0300+i,(agc_word)(012345+i));
  for(i=0;i<6;++i) { exec_address a=cm_rn_entry((uint8_t)i);set(a.location,024000+0156);set(a.location+1,03124); }
  for(step=0;step<14;++step) {
   if(step==0) {
    physical_bank(012003);set(RegQ,01010);set(RegA,0);set(RegL,0);set(RegZ,03006);run_stop(01010,077777);
    c.exec.alarm=c.wait.alarm=0;set(RegZ,02667);run_stop(05360,077777);
    for(i=6;i!=0;) {
     --i;if(c.phases.phase[i]==0) continue;
     physical_bank(02003);set(RegA,0);set(RegL,0);set(0154,c.phases.phase[i]);set(0161,i);set(RegZ,03520);run_stop(05222,077777);
    }
   } else if(step==1) {
    unsigned pending=get(0067),slot=pending/12;
    if(pending==077777) status=EXEC_IDLE;
    else {
     current.location=(agc_word)get(0154+slot*12+8);current.bank=(agc_word)get(0154+slot*12+9);
     physical_bank(02000);set(RegL,0);
     if(pending==0) { set(RegZ,03242);machine.Erasable[0][RegA]=2; }
     else set(RegZ,03032);
     run_stop(current.location,current.bank);
    }
   } else {
    exec_address next;read_case(&c);next=candidate_address(&c);++callbacks;
    /* Run actual erasable INCR MPAC+2 / TC ENDJOB fixture, not a native touch. */
    if(next.location==0) { status=EXEC_IDLE;run_stop(03225,077777);current.location=current.bank=0; }
    else { run_stop(next.location,next.bank);current=next; }
   }
   read_case(&c);c.exec.alarm=c.wait.alarm=0;
   for(i=0;i<16;++i) if(get(0300+i)!=012345+i) return 2;
   printf("%u %u",n,step);cm_rn_words(&c,(enum exec_status)status,current,(uint8_t)callbacks,emit,0);putchar('\n');
   fprintf(stderr,"CM restart native%u/%u: PC%05o BB%05o callbacks%u\n",n,step,get(RegZ),get(RegBB),callbacks);
   if(status==EXEC_IDLE) break;
  }
 }
 return 0;
}
