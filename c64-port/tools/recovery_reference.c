/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only unchanged source adapter, no native recovery implementation. */
#define main original_reference_main
#include "reference_runner.c"
#undef main
#include "recovery_cases.h"
static void emit(void *u,agc_word w) { (void)u; printf(" %u",w); }
static void transfer(recovery_case *c,int read) {
 unsigned i,j,a; agc_word *v;
 for(i=0;i<6;++i) for(j=0;j<6;++j) {
  a=j==0 ? 0752+2*i : j==1 ? 0753+2*i : j==2 ? 01436+2*i : j==3 ? 01437+2*i : j==4 ? 01054+2*i : 01053+2*i;
  v=j==0 ? &c->state.records.complement[i] : j==1 ? &c->state.records.phase[i] : j==2 ? &c->state.records.address[i].location : j==3 ? &c->state.records.address[i].bank : j==4 ? &c->state.records.priority[i] : &c->state.tbase[i];
  if(read) *v=(agc_word)get(a); else set(a,*v);
 }
 for(i=0;i<2;++i) { if(read) c->state.longbase[i]=(agc_word)get(01151+i); else set(01151+i,c->state.longbase[i]); }
 if(read) {
  c->state.flagword2=(agc_word)get(076); c->state.flagword3=(agc_word)get(077); c->state.flagword10=(agc_word)get(0106);
  c->state.a=(agc_register)((unsigned)machine.Erasable[0][RegA]&0177777u);
  c->state.l=(agc_register)((unsigned)machine.Erasable[0][RegL]&0177777u);
  c->state.q=(agc_word)get(RegQ); c->state.bbank=(agc_word)get(RegBB);
  c->state.superbank=(agc_word)machine.OutputChannel7; c->state.interrupt_enabled=(agc_word)machine.AllowInterrupt;
 } else {
  set(076,c->state.flagword2); set(077,c->state.flagword3); set(0106,c->state.flagword10);
  machine.Erasable[0][RegA]=(int16_t)c->state.a; machine.Erasable[0][RegL]=(int16_t)c->state.l;
  set(RegQ,c->state.q); set(RegBB,c->state.bbank); set(RegEB,(agc_word)((c->state.bbank&7)<<8)); set(RegFB,(agc_word)(c->state.bbank&076000));
  machine.InputChannel[7]=machine.OutputChannel7=c->state.superbank; machine.AllowInterrupt=c->state.interrupt_enabled;
 }
}
int main(int argc,char **argv) {
 static const unsigned entries[]={03755,02643,02647,02652,05661};
 static const unsigned guards[]={061,062,063,064,065,066,070,071,072,073,0130,0154,01434,01452};
 unsigned n,i,pc,cycles,ret; if(argc!=2) return 2;
 for(n=0;n<RECOVERY_CASE_COUNT;++n) {
  recovery_case c; memset(&machine,0,sizeof machine); if(agc_load_binfile(&machine,argv[1])) return 2;
  recovery_prepare(&c,(uint16_t)n); ret=c.state.q; transfer(&c,0); InhibitAlarms=1;
  for(i=0;i<sizeof guards/sizeof guards[0];++i) set(guards[i],(agc_word)(013421+i));
  set(RegZ,(agc_word)entries[c.entry]);
  for(cycles=0;cycles<2000;++cycles) {
   pc=get(RegZ);
   if(!machine.PendFlag && !machine.ExtraDelay && pc==(c.entry==RECOVERY_POODOO_PHASE4 ? 05665 : ret)) break;
   machine.ScalerCounter=0; if(agc_engine(&machine)) return 2;
  }
  if(cycles==2000) { fprintf(stderr,"recovery %u timeout PC%05o BB%05o\n",n,pc,get(RegBB)); return 2; }
  for(i=0;i<sizeof guards/sizeof guards[0];++i) if(get(guards[i])!=013421+i) { fprintf(stderr,"recovery %u guard %05o\n",n,guards[i]); return 2; }
  transfer(&c,1); printf("%u 0",n); recovery_words(&c,emit,0); putchar('\n');
  fprintf(stderr,"recovery %u: %u cycles PC=%05o BB=%05o\n",n,cycles,pc,get(RegBB));
 }
 return 0;
}
