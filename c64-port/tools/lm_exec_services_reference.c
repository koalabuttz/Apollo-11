/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Luminary099 Executive/Waitlist service oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_exec_services_cases.h"
static agc_t baseline;
/* Runs the unmodified rope from each lm-exec-services case entry to its stop
 * (kind 0), to BAILOUT1 or POODOO1 (kind 1), or to the constant words DPBIT14
 * in LONGCYCL (01,3477 or 01,3500; kind 2), and prints the erasable image
 * (none for kind 2). */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0,fb,kind;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<LM_EXS_CASES;++n){apollo_overlay_state s;p66_bias_io io;lm_exs_case c;
  machine=baseline;lm_exs_input(&s,&io,&c,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)c.superbank;machine.AllowInterrupt=0;kind=3;
  for(cycles=0;cycles<20000u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode){pc=get(RegZ);fb=(unsigned)get(4)&076000u;
    if(pc==c.stop_z&&(c.stop_fb==0177777u||fb==c.stop_fb)){kind=LM_EXS_RETURNED;break;}
    if(pc==LM_EXEC_BAILOUT1||pc==LM_EXEC_POODOO1){kind=LM_EXS_ALARM;break;}
    if((pc==03477u||pc==03500u)&&fb==02000u){kind=LM_EXS_REFUSED;break;}
   }
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(kind==3u){fprintf(stderr,"lm-exec-services case %u timeout %05o\n",n,pc);return 2;}
  printf("%u %u",n,kind);
  if(kind!=LM_EXS_REFUSED)for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));
  putchar('\n');
 }return 0;
}
