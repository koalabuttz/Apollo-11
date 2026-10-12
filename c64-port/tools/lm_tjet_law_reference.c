/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Luminary099 TJET_LAW oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_tjet_law_cases.h"
static agc_t baseline;
/* Runs the unmodified rope from each lm-tjet-law case entry to the caller's
 * return and prints all 2048 erasable words. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0,fb;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<LM_TJ_CASES;++n){apollo_overlay_state s;lm_tj_case c;int done=0;
  machine=baseline;lm_tj_input(&s,&c,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)c.superbank;machine.AllowInterrupt=0;
  for(cycles=0;cycles<20000u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode){pc=get(RegZ);fb=(unsigned)get(4)&076000u;
    if(pc==c.stop_z&&(c.stop_fb==0177777u||fb==c.stop_fb)){done=1;break;}}
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(!done){fprintf(stderr,"lm-tjet-law case %u timeout %05o\n",n,pc);return 2;}
  printf("%u 0",n);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<3u?0177777u:077777u));
  putchar('\n');
 }return 0;
}
