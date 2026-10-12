/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Comanche055 UPDATE_PROGRAM oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_update_cases.h"
static agc_t baseline;
static int quiet(void){return !machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode&&!machine.SubstituteInstruction;}
/* Points the native run owns: UPDATE_PROGRAM (bank 43 3726-3765, bank 27
 * 3373-3762), INTER-BANK_COMMUNICATION (4662-4775), Z = 2 (TC Q executes
 * the word in Q) and UPTEMP (TC UPTEMP executes the word stored there). */
static int owned(unsigned z,unsigned fb){unsigned bank=fb>>10;
 if(z==2u||z==CM_UP_UPTEMP)return 1;
 if(z>=04662u&&z<=04775u)return 1;
 if(bank>=030u&&(machine.OutputChannel7&0100))bank+=010u;
 if(bank==043u&&z>=03726u&&z<=03765u)return 1;
 if(bank==027u&&z>=03373u&&z<=03762u)return 1;
 return 0;
}
/* From each case's entry, as tests/cm_update_cases.c sets it up, to the first
 * point the native run does not own. Prints the interrupt-enable state,
 * SUPERBNK, channel 11 and all 2048 erasable words. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0;
 if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_UP_CASES;++n){apollo_overlay_state s;cm_native_io io;agc_hardware hw;
  cm_up_input(&s,&io,&hw,(uint16_t)n);
  machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;machine.InputChannel[011]=(int16_t)hw.channel11;
  for(cycles=0;cycles<6000u;++cycles){
   if(cycles&&quiet()){pc=get(RegZ);if(!owned(pc,get(4)))break;}
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==6000u){fprintf(stderr,"CM update %u timeout %05o\n",n,pc);return 2;}
  printf("%u %u %u %u",n,(unsigned)machine.AllowInterrupt,(unsigned)machine.OutputChannel7&0160u,(unsigned)machine.InputChannel[011]&077777u);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));putchar('\n');
 }return 0;
}
