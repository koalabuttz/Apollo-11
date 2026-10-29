/* SPDX-License-Identifier: GPL-2.0-or-later
 * ORIGINAL assembly, input-only fixtures. No native Executive is linked. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_interpreter_cases.h"
static void load_state(const cm_executive *s) {
    unsigned i,j;
    for(i=0;i<7;++i) for(j=0;j<12;++j) set(0154+i*12+j,s->core[i][j]);
    for(i=0;i<5;++i) for(j=0;j<44;++j) set(0400+i*44+j,s->vac[i][j]);
    set(0067,s->newjob); set(0063,s->newprio); set(0065,s->newloc[0]);
    set(0066,s->newloc[1]); set(0064,s->locctr); set(0120,s->fixloc); set(0121,s->ovfind);
}
static void read_state(cm_executive *s) {
    unsigned i,j;
    for(i=0;i<7;++i) for(j=0;j<12;++j) s->core[i][j]=(agc_word)get(0154+i*12+j);
    for(i=0;i<5;++i) for(j=0;j<44;++j) s->vac[i][j]=(agc_word)get(0400+i*44+j);
    s->newjob=(agc_word)get(0067); s->newprio=(agc_word)get(0063);
    s->newloc[0]=(agc_word)get(0065); s->newloc[1]=(agc_word)get(0066);
    s->locctr=(agc_word)get(0064); s->fixloc=(agc_word)get(0120); s->ovfind=(agc_word)get(0121); s->alarm=0;
}
static void bank(agc_word b) {
    set(RegBB,(agc_word)(b & 076007)); set(RegFB,(agc_word)(b & 076000));
    set(RegEB,(agc_word)((b & 7u)<<8)); machine.InputChannel[7]=machine.OutputChannel7=b & 0160;
}
int main(int argc,char **argv) {
 unsigned n;
 if(argc!=2) return 2;
 for(n=0;n<CM_INTERPRETER_COUNT;++n) {
  cm_interpreter_case c; exec_transfer next={{0,0},0};
  unsigned cycles,pc=0,i,slot;
  memset(&machine,0,sizeof machine);
  if(agc_load_binfile(&machine,argv[1])!=0) return 2;
  cm_interpreter_prepare(&c,n); load_state(&c.state);
  for(i=0;i<16;++i) set(0300+i,(agc_word)(012345+i));
  set(0115,c.control.intbit15); set(RegEDOP,c.control.edop);
  bank((agc_word)(c.state.core[0][CM_EXEC_BANK]|c.superbank));
  slot=c.state.newjob/12;
  next.address.location=c.state.core[slot][CM_EXEC_LOC];
  next.address.bank=c.state.core[slot][CM_EXEC_BANK];
  next.interpretive=(uint8_t)((next.address.location&040000)!=0);
  if(next.interpretive) next.address.location=(agc_word)((next.address.location^077777)+1u);
  set(RegZ,05170); machine.AllowInterrupt=0; InhibitAlarms=1;
  for(cycles=0;cycles<4000;++cycles) {
   pc=get(RegZ);
   if(!machine.PendFlag && !machine.ExtraDelay && pc==(next.interpretive?06037u:next.address.location) && get(RegBB)==(next.address.bank&076007u) && (unsigned)machine.OutputChannel7==(next.address.bank&0160u)) break;
   if(agc_engine(&machine)!=0) return 2;
  }
  if(cycles==4000) { fprintf(stderr,"CM CHANG2 %u timeout at %05o\n",n,pc); return 2; }
  for(i=0;i<16;++i) if(get(0300+i)!=012345+i) return 2;
  read_state(&c.state); c.control.intbit15=(agc_word)get(0115); c.control.edop=(agc_word)get(RegEDOP);
  cm_interpreter_row(n,&c,EXEC_OK,next);
  fprintf(stderr,"CM CHANG2 %u %u cycles PC=%05o BB=%05o SB=%03o\n",n,cycles,pc,get(RegBB),machine.OutputChannel7);
 }
 return 0;
}
