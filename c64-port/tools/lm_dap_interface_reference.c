/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Luminary099 DAP_INTERFACE_SUBROUTINES oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_dap_interface_cases.h"
static agc_t baseline;
/* Runs the unmodified rope from each lm-dap-interface case entry to the
 * caller's return (kind 0) or to BAILOUT1/POODOO1 (kind 1) and prints all
 * 2048 erasable words. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0,fb,kind;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<LM_DAP_CASES;++n){apollo_overlay_state s;p66_bias_io io;lm_dap_case c;
  machine=baseline;lm_dap_input(&s,&io,&c,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)c.superbank;machine.AllowInterrupt=0;kind=3;
  for(cycles=0;cycles<20000u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode){pc=get(RegZ);fb=(unsigned)get(4)&076000u;
    if(pc==c.stop_z&&(c.stop_fb==0177777u||fb==c.stop_fb)){kind=LM_DAP_RETURNED;break;}
    if(pc==LM_EXEC_BAILOUT1||pc==LM_EXEC_POODOO1){kind=LM_DAP_ALARM;break;}
   }
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(kind==3u){fprintf(stderr,"lm-dap-interface case %u timeout %05o\n",n,pc);return 2;}
  printf("%u %u",n,kind);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));
  putchar('\n');
 }return 0;
}
