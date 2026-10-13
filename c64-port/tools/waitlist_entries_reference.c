/* SPDX-License-Identifier: GPL-2.0-or-later; host-only yaAGC oracle for milestone waitlist-entries. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "waitlist_entries_cases.h"
/* argv: Luminary rope, Comanche rope. For every case the native run reached
 * (a refusal prints "n 0"), runs the original from the same image, SUPERBNK,
 * interrupt enable and InIsr until Z and FB (banked) first equal the native
 * continuation, or, when the native run ended the interrupt, until InIsr
 * clears; prints SUPERBNK, the enable bit, InIsr and the 2048 erasable words. */
int main(int argc,char **argv){static agc_t rope[2];static apollo_overlay_state s,native;static wle_io_storage io;agc_hardware hw,nhw;
 uint16_t n,i,sb,en;unsigned cycles,k,ended,pc,fb;uint8_t kind,cm;
 if(argc!=3)return 2;
 for(i=0;i<2u;++i){memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1+i]))return 2;rope[i]=machine;}
 InhibitAlarms=1;
 for(n=0;n<wle_cases();++n){
  k=wle_run(&native,&io,&nhw,n);if(!k){printf("%u 0\n",(unsigned)n);continue;}
  kind=wle_kind(n);cm=(uint8_t)(kind==1u||kind==3u);
  machine=rope[cm];wle_input(&s,&io,&hw,kind,wle_case_base_of(n));for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  wle_io(&io,kind,&sb,&en);
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)sb;machine.AllowInterrupt=en;machine.InIsr=hw.in_interrupt;
  ended=hw.in_interrupt&&!nhw.in_interrupt;
  for(cycles=0;cycles<20000u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode){pc=get(5);fb=get(4);
    if(ended?!machine.InIsr:(pc==native.words[5]&&(pc>=04000u||fb==native.words[4])))break;}
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==20000u){fprintf(stderr,"waitlist-entries case %u timeout\n",(unsigned)n);return 2;}
  printf("%u 1 %u %u %u",(unsigned)n,(unsigned)machine.OutputChannel7&0160u,(unsigned)machine.AllowInterrupt,(unsigned)machine.InIsr);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<3u?0177777u:077777u));printf("\n");
 }
 return 0;}
