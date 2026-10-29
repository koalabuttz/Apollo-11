/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Luminary099 P-AXIS_RCS_AUTOPILOT oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_p_axis_cases.h"
static agc_t baseline;
/* Runs the unmodified rope from each lm-p-axis case entry until Z leaves
 * 16,2213-3640 (an instruction boundary with Z outside the range or FB not
 * bank 16; Z in the central registers is the TC Q on its way to the word
 * in Q), and prints the outcome (LM_PA_NONE after 40000 cycles),
 * channels 12 and 13, the interrupt-enable bit and all 2048 erasable
 * words. */
int main(int argc,char **argv){unsigned n,i,cycles,pc,fb,kind;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<LM_PA_CASES;++n){apollo_overlay_state s;lm_flight_io io;
  machine=baseline;lm_pa_input(&s,&io,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=0;machine.AllowInterrupt=(int)io.bias.interrupt_enabled;kind=LM_PA_NONE;
  machine.InputChannel[012]=(int16_t)io.hw.channel12;machine.InputChannel[013]=(int16_t)io.hw.channel13;machine.InputChannel[031]=(int16_t)io.hw.channel31;
  for(cycles=0;cycles<40000u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode){pc=get(RegZ);fb=(unsigned)get(4)&076000u;
    if(pc>=010u&&(pc<LM_PA_FIRST||pc>LM_PA_LAST||fb!=LM_PA_FB16)){kind=LM_PA_LEFT;break;}
   }
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  printf("%u %u %u %u %u",n,kind,(unsigned)machine.InputChannel[012]&077777u,(unsigned)machine.InputChannel[013]&077777u,
   (unsigned)(machine.AllowInterrupt!=0));
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<2u?0177777u:077777u));
  putchar('\n');
 }return 0;
}
