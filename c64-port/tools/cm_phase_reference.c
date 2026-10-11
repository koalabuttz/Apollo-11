/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only unchanged assembly adapter; no native phase implementation linked. */
#define main original_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_phase_cases.h"
static void emit(void *u,agc_word w) { (void)u; printf(" %u",w); }
static void transfer(cm_phase_case *c,int read) {
 unsigned i,j; agc_word *v; unsigned a;
 for(i=0;i<6;++i) for(j=0;j<6;++j) {
  a=j==0 ? 0752+2*i : j==1 ? 0753+2*i : j==2 ? 01436+2*i : j==3 ? 01437+2*i : j==4 ? 01054+2*i : 01053+2*i;
  v=j==0 ? &c->state.records.complement[i] : j==1 ? &c->state.records.phase[i] : j==2 ? &c->state.records.address[i].location : j==3 ? &c->state.records.address[i].bank : j==4 ? &c->state.records.priority[i] : &c->state.tbase[i];
  if(read) *v=(agc_word)get(a); else set(a,*v);
 }
 for(i=0;i<2;++i) { if(read) { c->state.longbase[i]=(agc_word)get(01135+i); c->state.time[i]=(agc_word)get(024+i); }
  else { set(01135+i,c->state.longbase[i]); set(024+i,c->state.time[i]); } }
 for(i=0;i<CM_PH_SCRATCH_COUNT;++i) { a=i<6 ? 061+i : 070+i-6;
  if(read) c->state.scratch[i]=(agc_word)get(a); else set(a,c->state.scratch[i]); }
 if(read) {
  c->state.a=(agc_register)((unsigned)machine.Erasable[0][RegA]&0177777u);
  c->state.l=(agc_register)((unsigned)machine.Erasable[0][RegL]&0177777u);
  c->state.q=(agc_word)get(RegQ); c->state.bbank=(agc_word)get(RegBB);
  c->state.superbank=(agc_word)machine.OutputChannel7; c->state.interrupt_enabled=(agc_word)machine.AllowInterrupt;
 } else {
  machine.Erasable[0][RegA]=(int16_t)c->state.a; machine.Erasable[0][RegL]=(int16_t)SignExtend(c->state.l);
  set(RegQ,c->state.q); set(RegBB,c->state.bbank); set(RegEB,(agc_word)((c->state.bbank&7)<<8)); set(RegFB,(agc_word)(c->state.bbank&076000));
  machine.OutputChannel7=machine.InputChannel[7]=c->state.superbank; machine.AllowInterrupt=c->state.interrupt_enabled;
 }
}
int main(int argc,char **argv) {
 static const unsigned guards[]={0130,0131,0135,0140,0141,0154,0377,01434,01452};
 unsigned n,i,cycles,pc; if(argc!=2) return 2;
 for(n=0;n<CM_PH_CASE_COUNT;++n) {
  cm_phase_case c; unsigned ret,k; memset(&machine,0,sizeof machine);
  if(agc_load_binfile(&machine,argv[1])) return 2;
  cm_phase_prepare(&c,(uint16_t)n); transfer(&c,0); InhibitAlarms=1;
  for(i=0;i<5;++i) set(01000+i,c.words[i]);
  for(i=0;i<sizeof guards/sizeof guards[0];++i) set(guards[i],(agc_word)(012345+i));
  k=c.two; ret=01000+1+k;
  if(c.words[k]&014000) {
   if(c.words[k] & ((c.words[k]&010000) ? 04000 : 01000)) ++ret;
   if(c.words[k]&02000) ret+=2;
  }
  set(RegZ,c.two ? 05372 : 05412);
  for(cycles=0;cycles<5000;++cycles) {
   pc=get(RegZ);
   if(!machine.PendFlag && !machine.ExtraDelay && pc==ret) break;
   machine.ScalerCounter=0;
   if(agc_engine(&machine)) return 2;
  }
  if(cycles==5000) { fprintf(stderr,"phase %u timeout PC %05o BB %05o Q %05o\n",n,pc,get(RegBB),get(RegQ)); return 2; }
  for(i=0;i<sizeof guards/sizeof guards[0];++i) if(get(guards[i])!=012345+i) { fprintf(stderr,"phase %u guard %05o modified\n",n,guards[i]); return 2; }
  for(i=0;i<5;++i) if(get(01000+i)!=c.words[i]) return 2;
  transfer(&c,1); printf("%u 0",n); cm_phase_words(&c,emit,0); putchar('\n');
  fprintf(stderr,"phase %u: %u cycles PC=%05o BB=%05o\n",n,cycles,pc,get(RegBB));
 }
 return 0;
}
