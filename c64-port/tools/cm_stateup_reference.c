/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Comanche055 STATEUP/THISVINT (FRESH_START_AND_RESTART bank 13, R00) oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cert_trace.h"
#include "cm_stateup_cases.h"
static int quiet(void){return !machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode&&!machine.SubstituteInstruction;}
/* From each case's state, as tests/cm_stateup_cases.c sets it up, until
 * cm_stateup_leaves(). Prints the interrupt-enable state, SUPERBNK and all
 * 2048 erasable words. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0;agc_t baseline;
 if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_STATEUP_CASES;++n){apollo_overlay_state s;p66_bias_io io;unsigned stopped=0;
  cm_stateup_input(&s,&io,(uint16_t)n);
  machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  for(cycles=0;cycles<200000u;++cycles){
   if(cycles&&quiet()){pc=get(RegZ);
    if(cm_stateup_leaves((agc_word)pc,(agc_word)(get(4)&076000u),(agc_word)machine.InputChannel[7],(agc_word)get(0164),(agc_word)get(0165))){stopped=1;break;}}
   if(quiet())CERT_MARK();
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(!stopped){fprintf(stderr,"CM STATEUP %u timeout %05o\n",n,pc);return 2;}
  printf("%u %u %u",n,(unsigned)machine.AllowInterrupt,(unsigned)machine.OutputChannel7&0160u);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));putchar('\n');
 }return 0;
}
