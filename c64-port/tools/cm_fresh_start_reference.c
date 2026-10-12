/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Comanche055 FRESH_START_AND_RESTART (bank 05) oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_fresh_start_cases.h"
static agc_t baseline;
static int quiet(void){return !machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode&&!machine.SubstituteInstruction;}
/* Points the native run owns: FRESH_START_AND_RESTART bank 05 (2342-3217)
 * and fixed-fixed (GOTOPOOH 4106-4111), INTER-BANK_COMMUNICATION
 * (4662-4775) but for SWRETURN (4675-4700, where the native run stops), and
 * Z = 2 (TC Q executes the word in Q). */
static int owned(unsigned z,unsigned fb){unsigned bank=fb>>10;
 if(z==2u)return 1;
 if(z>=04675u&&z<=04700u)return 0;
 if(z>=04662u&&z<=04775u)return 1;
 if(z>=04106u&&z<=04111u)return 1;
 if(bank>=030u&&(machine.OutputChannel7&0100))bank+=010u;
 if(bank==05u&&z>=02342u&&z<=03217u)return 1;
 return 0;
}
/* From each case's entry, as tests/cm_fresh_start_cases.c sets it up, to the
 * first point the native run does not own. Prints the interrupt-enable
 * state, SUPERBNK, channels 11, 5, 6, 12, 13, 14 and 33, the three trap
 * latches and all 2048 erasable words. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0;
 if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_FS_CASES;++n){apollo_overlay_state s;cm_fresh_io io;
  cm_fs_input(&s,&io,(uint16_t)n);
  machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=io.control.superbank;machine.AllowInterrupt=io.control.interrupt_enabled;
  machine.InputChannel[011]=(int16_t)io.control.channel11;machine.InputChannel[05]=(int16_t)io.channel5;machine.InputChannel[06]=(int16_t)io.channel6;
  machine.InputChannel[012]=(int16_t)io.control.channel12;machine.InputChannel[013]=(int16_t)io.control.channel13;machine.InputChannel[014]=(int16_t)io.control.channel14;
  machine.InputChannel[015]=(int16_t)io.control.main_keys;machine.InputChannel[016]=(int16_t)io.control.navigation_keys;machine.InputChannel[033]=(int16_t)io.channel33;
  machine.Trap31A=io.control.trap31a;machine.Trap31B=io.control.trap31b;machine.Trap32=io.control.trap32;
  for(cycles=0;cycles<20000u;++cycles){
   if(cycles&&quiet()){pc=get(RegZ);if(!owned(pc,get(4)))break;}
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==20000u){fprintf(stderr,"CM fresh start %u timeout %05o\n",n,pc);return 2;}
  printf("%u %u %u %u %u %u %u %u %u %u %u %u %u",n,(unsigned)machine.AllowInterrupt,(unsigned)machine.OutputChannel7&0160u,
         (unsigned)machine.InputChannel[011]&077777u,(unsigned)machine.InputChannel[05]&077777u,(unsigned)machine.InputChannel[06]&077777u,
         (unsigned)machine.InputChannel[012]&077777u,(unsigned)machine.InputChannel[013]&077777u,(unsigned)machine.InputChannel[014]&077777u,
         (unsigned)machine.InputChannel[033]&077777u,(unsigned)machine.Trap31A,(unsigned)machine.Trap31B,(unsigned)machine.Trap32);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));putchar('\n');
 }return 0;
}
