/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Luminary099 ALARM_AND_ABORT oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_alarm_cases.h"
static agc_t baseline;
static int quiet(void){return !machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode&&!machine.SubstituteInstruction;}
/* Points the native run owns: ALARM_AND_ABORT (fixed-fixed 5567-5752, bank 10
 * 3562-3572, bank 13 3755-3766, the erasable ALMCADR), INTER-BANK_COMMUNICATION
 * (4616-4731 except SWRETURN 4631-4634, where a BANKCALL'd FLAGS returns:
 * lm_interbank_routines takes SWRETURN only with Q 0100-7775, and FLAGS's
 * TC Q leaves Q = 3) and Z = 2, where TC Q executes the word in Q. */
static int owned(unsigned z,unsigned fb){unsigned bank=fb>>10;
 if(z==2u||z==LM_AL_ALMCADR)return 1;
 if(z>=04616u&&z<=04731u&&!(z>=04631u&&z<=04634u))return 1;
 if(z>=05567u&&z<=05752u)return 1;
 if(bank>=030u&&(machine.OutputChannel7&0100))return 0;
 if(bank==010u&&z>=03562u&&z<=03572u)return 1;
 if(bank==013u&&z>=03755u&&z<=03766u)return 1;
 return 0;
}
/* From each case's entry, as tests/lm_alarm_cases.c sets it up, to the first
 * point the native run does not own. Prints the interrupt-enable state,
 * SUPERBNK, the in-interrupt state and all 2048 erasable words. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0;
 if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<LM_AL_CASES;++n){apollo_overlay_state s;p66_bias_io io;agc_hardware hw;
  lm_al_input(&s,&io,&hw,(uint16_t)n);
  machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;machine.InIsr=hw.in_interrupt;
  for(cycles=0;cycles<4000u;++cycles){
   if(cycles&&quiet()){pc=get(RegZ);if(!owned(pc,get(4)))break;}
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==4000u){fprintf(stderr,"LM alarm %u timeout %05o\n",n,pc);return 2;}
  printf("%u %u %u %u",n,(unsigned)machine.AllowInterrupt,(unsigned)machine.OutputChannel7&0160u,(unsigned)machine.InIsr);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));putchar('\n');
 }return 0;
}
