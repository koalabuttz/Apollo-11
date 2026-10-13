/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Comanche055 ALARM_AND_ABORT oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_alarm_cases.h"
static agc_t baseline;
static int quiet(void){return !machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode&&!machine.SubstituteInstruction;}
/* Points the native run owns: ALARM_AND_ABORT (fixed-fixed 5650-5772 and
 * 7755-7774, bank 10 3766-3776, the erasable ALMCADR) and
 * INTER-BANK_COMMUNICATION (4662-4775). */
static int owned(unsigned z,unsigned fb){unsigned bank=fb>>10;
 if(z==CM_AL_ALMCADR)return 1;
 if(z>=04662u&&z<=04775u)return 1;
 if((z>=05650u&&z<=05772u)||(z>=07755u&&z<=07774u))return 1;
 if(bank>=030u&&(machine.OutputChannel7&0100))return 0;
 if(bank==010u&&z>=03766u&&z<=03776u)return 1;
 return 0;
}
/* From each case's entry, as tests/cm_alarm_cases.c sets it up, to the first
 * point the native run does not own. Prints the interrupt-enable state,
 * SUPERBNK, the in-interrupt state and all 2048 erasable words. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0;
 if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_AL_CASES;++n){apollo_overlay_state s;cm_native_io io;agc_hardware hw;
  cm_al_input(&s,&io,&hw,(uint16_t)n);
  machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;machine.InIsr=hw.in_interrupt;
  for(cycles=0;cycles<4000u;++cycles){
   if(cycles&&quiet()){pc=get(RegZ);if(!owned(pc,get(4)))break;}
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==4000u){fprintf(stderr,"CM alarm %u timeout %05o\n",n,pc);return 2;}
  printf("%u %u %u %u",n,(unsigned)machine.AllowInterrupt,(unsigned)machine.OutputChannel7&0160u,(unsigned)machine.InIsr);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));putchar('\n');
 }return 0;
}
