/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Luminary099 LANDING_ANALOG_DISPLAYS oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_landing_displays_cases.h"
static agc_t baseline;
/* Whether Z (with FB) is in this file: 21,2154-3076, the central registers
 * (a TC Q on its way to the word in Q), and LADQSAVE when TC LADQSAVE runs
 * ALTROUT +1 or ALTOUT +1 there. */
static int in_file(unsigned pc,unsigned fb){
 if(pc<010u)return 1;
 if(pc==LM_LAD_LADQSAVE){unsigned w=(unsigned)*memory(03714u) /* LADQSAVE under EBANK7 */&077777u;
  return w==LM_LAD_ALTROUT+1u||w==LM_LAD_ALTOUT+1u;}
 return pc>=LM_LAD_FIRST&&pc<=LM_LAD_LAST&&fb==LM_LAD_FB21;
}
/* Runs the unmodified rope from each lm-landing-displays case entry until Z
 * leaves the file at an instruction boundary, and prints the outcome
 * (LM_LAD_NONE after 40000 cycles), channels 12 and 14 and all 2048
 * erasable words. */
int main(int argc,char **argv){unsigned n,i,cycles,kind;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<LM_LAD_CASES;++n){apollo_overlay_state s;lm_flight_io io;
  machine=baseline;lm_lad_input(&s,&io,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=0;machine.AllowInterrupt=0;machine.InIsr=0;kind=LM_LAD_NONE;
  machine.InputChannel[012]=(int16_t)io.hw.channel12;machine.InputChannel[014]=(int16_t)io.hw.channel14;machine.InputChannel[030]=(int16_t)io.hw.channel30;
  for(cycles=0;cycles<40000u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode
      &&!in_file((unsigned)get(RegZ),(unsigned)get(4)&076000u)){kind=LM_LAD_LEFT;break;}
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  printf("%u %u %u %u",n,kind,(unsigned)machine.InputChannel[012]&077777u,(unsigned)machine.InputChannel[014]&077777u);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<2u?0177777u:077777u));
  putchar('\n');
 }return 0;
}
