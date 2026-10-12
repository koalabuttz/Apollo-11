/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original BURN_BABY_BURN oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_burn_baby_cases.h"
/* Runs the unmodified Luminary099 rope (argv[1]) from each lm-burn-baby
 * case's image, channels and interrupt state until lmbb_leaves() and
 * prints the case, kind 0, the 2048 erasable words, channels 11, 14 and 30
 * and the interrupt-enable state. The scalers stay frozen (ScalerCounter
 * 0). */
int main(int argc,char **argv){unsigned n,i;unsigned long cycles;agc_t baseline;if(argc!=2)return 2;InhibitAlarms=1;
 memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;
 for(n=0;n<LMBB_CASES;++n){apollo_overlay_state s;lmbb_case c;unsigned pc=0,stopped=0;
  lmbb_input(&s,&c,(uint16_t)n);
  machine=baseline;
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)c.superbank;machine.AllowInterrupt=c.interrupt_enabled;
  machine.InputChannel[011]=(int16_t)c.chan11;machine.InputChannel[014]=(int16_t)c.chan14;machine.InputChannel[030]=(int16_t)c.chan30;
  for(cycles=0;cycles<4000000ul;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode){
    pc=get(RegZ);
    if(lmbb_leaves((agc_word)pc,(agc_word)(get(4)&076000u),(agc_word)machine.InputChannel[7],(agc_word)get(0164),(agc_word)get(0165))){stopped=1;break;}
   }
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(!stopped)fprintf(stderr,"lm-burn-baby case %u timeout %05o\n",n,pc);
  printf("%u %u",n,stopped?0u:1u);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));
  printf(" %u %u %u %u\n",(unsigned)machine.InputChannel[011]&077777u,(unsigned)machine.InputChannel[014]&077777u,
         (unsigned)machine.InputChannel[030]&077777u,(unsigned)machine.AllowInterrupt);
 }return 0;
}
