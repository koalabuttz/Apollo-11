/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only original STOPRATE/FASTCHNG assembly oracle; input data only. */
#define main original_reference_main
#include "reference_runner.c"
#undef main
#include "p66_initialization_cases.h"
static const unsigned controls[]={03243,03244,03245,03240,03241,03242,01277,01300,01301};
static void emit(void *u,agc_word w){(void)u;printf(" %u",w);}
int main(int argc,char **argv){
 static const unsigned guards[]={03237,03246,01440,01441,01444,01445,01302,0163,0130};
 unsigned n,i,cycles,pc;static agc_t pristine;
 if(argc!=2 || agc_load_binfile(&pristine,argv[1]))return 2;
 for(n=0;n<P66_INIT_CASE_COUNT;++n){
  p66_init_case c;unsigned ret;memcpy(&machine,&pristine,sizeof machine);p66_init_prepare(&c,(uint16_t)n);InhibitAlarms=1;
  for(i=0;i<9;++i)set(controls[i],c.state.control[i]);set(01442,c.state.phase3_address[0]);set(01443,c.state.phase3_address[1]);
  for(i=0;i<sizeof guards/sizeof guards[0];++i)set(guards[i],(agc_word)(012345+i));
  machine.Erasable[0][RegA]=(int16_t)c.state.a;machine.Erasable[0][RegL]=(int16_t)SignExtend(c.state.l);
  ret=c.state.q;set(RegQ,c.state.q);set(RegBB,c.state.bbank);set(RegFB,(agc_word)(c.state.bbank&076000));set(RegEB,(agc_word)((c.state.bbank&7u)<<8));
  machine.OutputChannel7=machine.InputChannel[7]=c.state.superbank;machine.AllowInterrupt=c.state.interrupt_enabled;set(RegZ,c.fast ? 03740 : 02165);
  for(cycles=0;cycles<400;++cycles){pc=get(RegZ);if(!machine.PendFlag && !machine.ExtraDelay && pc==ret)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==400){fprintf(stderr,"initialization%u timeout PC%05o BB%05o\n",n,pc,get(RegBB));return 2;}
  for(i=0;i<sizeof guards/sizeof guards[0];++i)if(get(guards[i])!=012345+i)return 2;
  for(i=0;i<9;++i)c.state.control[i]=(agc_word)get(controls[i]);c.state.phase3_address[0]=(agc_word)get(01442);c.state.phase3_address[1]=(agc_word)get(01443);
  c.state.a=(agc_register)((unsigned)machine.Erasable[0][RegA]&0177777u);c.state.l=(agc_register)((unsigned)machine.Erasable[0][RegL]&0177777u);c.state.q=(agc_word)get(RegQ);
  c.state.bbank=(agc_word)get(RegBB);c.state.superbank=(agc_word)machine.OutputChannel7;c.state.interrupt_enabled=(agc_word)machine.AllowInterrupt;
  printf("%u 0",n);p66_init_words(&c,emit,0);putchar('\n');fprintf(stderr,"initialization%u %ucycles PC%05o BB%05o\n",n,cycles,pc,get(RegBB));
 }
 return 0;
}
