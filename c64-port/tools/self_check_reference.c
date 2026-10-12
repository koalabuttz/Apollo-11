/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original AGC_BLOCK_TWO_SELF-CHECK (bank 43) oracle, both missions. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include <stdlib.h>
#include "self_check_cases.h"
#include "case_trace.h"
static int quiet(void){return !machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode&&!machine.SubstituteInstruction;}
/* argv: mission (0 Luminary099, 1 Comanche055) and that mission's rope.
 * From each case's state, as tests/self_check_cases.c sets it up, until
 * self_check_leaves(). Prints the interrupt-enable state, SUPERBNK and all
 * 2048 erasable words. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0,mission;agc_t baseline;
 if(argc!=3)return 2;
 mission=(unsigned)atoi(argv[1])!=0u;
 memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[2]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<SELF_CHECK_CASES;++n){apollo_overlay_state s;p66_bias_io io;
  self_check_input(mission,&s,&io,(uint16_t)n);
  machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  for(cycles=0;cycles<200000u;++cycles){
   if(quiet()){pc=get(RegZ);
    if(self_check_leaves(mission,(agc_word)pc,(agc_word)(get(4)&076000u),(agc_word)machine.OutputChannel7)){if(cycles)break;}
    else CASE_MARK();}
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==200000u){fprintf(stderr,"SELF-CHECK %u/%u timeout %05o\n",mission,n,pc);return 2;}
  CASE_END(n);
  printf("%u %u %u",n,(unsigned)machine.AllowInterrupt,(unsigned)machine.OutputChannel7&0160u);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));putchar('\n');
 }return 0;
}
