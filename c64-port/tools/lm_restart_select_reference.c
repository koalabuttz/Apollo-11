/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Luminary099 RESTARTS oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_restart_select_cases.h"
static agc_t baseline;
/* Runs the unmodified Luminary099 rope from each case's entry to the TC
 * GOLOC-1 (Z 0704) or the X.1 display TC FINDVAC, and prints the status and
 * all 2048 erasable words. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<LM_RSSELECT_CASES;++n){apollo_overlay_state s;unsigned status;
  machine=baseline;(void)lm_rsselect_input(&s,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=0;machine.AllowInterrupt=0;
  for(cycles=0;cycles<800;++cycles){pc=get(RegZ);if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&(pc==0704||(pc==LM_RS_FINDVAC&&get(RegQ)==03546)))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==800){fprintf(stderr,"LM restart select %u timeout %05o\n",n,pc);return 2;}
  status=pc==LM_RS_FINDVAC?3u:get(0704)==LM_RS_LONGCALL?2u:get(0704)==LM_RS_WAITLIST?1u:0u;
  printf("%u %u",n,status);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));putchar('\n');
 }return 0;
}
