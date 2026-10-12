/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Luminary099 FRESH_START_AND_RESTART (bank 05) oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cert_trace.h"
#include "lm_fresh_start_cases.h"
static agc_t baseline;
static int quiet(void){return !machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode&&!machine.SubstituteInstruction;}
/* Points the native run owns: FRESH_START_AND_RESTART bank 05 (2447-3401)
 * and fixed-fixed (GOTOPOOH 6001-6006), INTER-BANK_COMMUNICATION (4616-4731) but for
 * SWRETURN (4631-4634, where the native run stops), and Z = 2 (TC Q
 * executes the word in Q). */
static int owned(unsigned z,unsigned fb){unsigned bank=fb>>10;
 if(z==2u)return 1;
 if(z>=04631u&&z<=04634u)return 0;
 if(z>=04616u&&z<=04731u)return 1;
 if(z>=06001u&&z<=06006u)return 1;
 if(bank>=030u&&(machine.OutputChannel7&0100))bank+=010u;
 if(bank==05u&&z>=02447u&&z<=03401u)return 1;
 return 0;
}
/* From each case's entry, as tests/lm_fresh_start_cases.c sets it up, to the
 * first point the native run does not own. Prints the interrupt-enable
 * state, SUPERBNK, channels 11, 5, 6, 12, 13 and 14, the three trap latches
 * and all 2048 erasable words. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0;
 if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<LM_FS_CASES;++n){apollo_overlay_state s;lm_fresh_io io;agc_hardware hw;
  lm_fs_input(&s,&io,&hw,(uint16_t)n);
  machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=io.control.superbank;machine.AllowInterrupt=io.control.interrupt_enabled;
  machine.InputChannel[011]=(int16_t)hw.channel11;machine.InputChannel[05]=(int16_t)io.channel5;machine.InputChannel[06]=(int16_t)io.channel6;
  machine.InputChannel[012]=(int16_t)io.channel12;machine.InputChannel[013]=(int16_t)io.channel13;machine.InputChannel[014]=(int16_t)io.channel14;
  machine.InputChannel[015]=(int16_t)io.channel15;machine.InputChannel[016]=(int16_t)io.channel16;machine.InputChannel[033]=(int16_t)io.channel33;
  machine.Trap31A=io.trap31a;machine.Trap31B=io.trap31b;machine.Trap32=io.trap32;
  for(cycles=0;cycles<20000u;++cycles){
   if(cycles&&quiet()){pc=get(RegZ);if(!owned(pc,get(4)))break;}
   if(quiet())CERT_MARK();
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==20000u){fprintf(stderr,"LM fresh start %u timeout %05o\n",n,pc);return 2;}
  printf("%u %u %u %u %u %u %u %u %u %u %u %u",n,(unsigned)machine.AllowInterrupt,(unsigned)machine.OutputChannel7&0160u,
         (unsigned)machine.InputChannel[011]&077777u,(unsigned)machine.InputChannel[05]&077777u,(unsigned)machine.InputChannel[06]&077777u,
         (unsigned)machine.InputChannel[012]&077777u,(unsigned)machine.InputChannel[013]&077777u,(unsigned)machine.InputChannel[014]&077777u,
         (unsigned)machine.Trap31A,(unsigned)machine.Trap31B,(unsigned)machine.Trap32);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));putchar('\n');
 }return 0;
}
