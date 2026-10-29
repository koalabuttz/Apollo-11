/* SPDX-License-Identifier: GPL-2.0-or-later; original CM source whole-image oracle. */
#define main cm_admission_main
#include "cm_executive_reference.c"
#undef main
#include "cm_startup_cases.h"
static void emit(void *u,agc_word w) { (void)u;printf(" %u",w); }
int main(int argc,char **argv) {
 unsigned n,i,cycles,pc;static const unsigned entries[]={02365,02342,02501,02505,02513};
 if(argc!=2) return 2;
 for(n=0;n<640;++n) {
  apollo_overlay_state s;cm_native_io io;cm_startup_ports p;
  memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1])) return 2;
  cm_startup_prepare(&s,&io,&p,(uint16_t)n);
  for(i=0;i<2048;++i) set(i,s.words[i]);
  machine.InputChannel[011]=io.channel11;machine.InputChannel[012]=io.channel12;machine.InputChannel[013]=io.channel13;machine.InputChannel[014]=io.channel14;
  machine.InputChannel[015]=io.main_keys;machine.InputChannel[016]=io.navigation_keys;
  machine.InputChannel[7]=machine.OutputChannel7=io.superbank;
  machine.InputChannel[5]=p.channel5;machine.InputChannel[6]=p.channel6;
  machine.AllowInterrupt=io.interrupt_enabled;machine.Trap31A=io.trap31a;machine.Trap31B=io.trap31b;machine.Trap32=io.trap32;
  InhibitAlarms=1;set(RegZ,entries[n/128]);
  for(cycles=0;cycles<12000;++cycles) {
   pc=get(RegZ);
   if(!machine.PendFlag && !machine.ExtraDelay && pc==(n<256 ? 02477 : 01010)) break;
   machine.ScalerCounter=0;if(agc_engine(&machine)) return 2;
  }
  if(cycles==12000) { fprintf(stderr,"CM startup%u timeout PC%05o\n",n,pc);return 2; }
  for(i=0;i<2048;++i) s.words[i]=(agc_word)get(i);
  s.words[0]=(agc_register)((unsigned)machine.Erasable[0][RegA]&0177777u);s.words[1]=(agc_register)((unsigned)machine.Erasable[0][RegL]&0177777u);
  io.channel11=(agc_word)machine.InputChannel[011];io.channel12=(agc_word)machine.InputChannel[012];io.channel13=(agc_word)machine.InputChannel[013];io.channel14=(agc_word)machine.InputChannel[014];
  io.trap31a=(agc_word)machine.Trap31A;io.trap31b=(agc_word)machine.Trap31B;io.trap32=(agc_word)machine.Trap32;
  io.interrupt_enabled=(agc_word)machine.AllowInterrupt;io.superbank=(agc_word)machine.OutputChannel7;
  if(n>=128 && n<256) io.executive_alarm=io.waitlist_alarm=0;
  p.channel5=(agc_word)machine.InputChannel[5];p.channel6=(agc_word)machine.InputChannel[6];
  printf("%u",n);cm_startup_words(&s,&io,&p,CM_STARTUP_OK,emit,0);putchar('\n');
  fprintf(stderr,"CM startup%u: %u cycles PC%05o BB%05o SB%03o IRQ%u\n",n,cycles,pc,get(RegBB),machine.OutputChannel7,machine.AllowInterrupt);
 }
 return 0;
}
