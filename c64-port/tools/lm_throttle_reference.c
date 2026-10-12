/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Luminary099 THROTTLE_CONTROL_ROUTINES oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_throttle_cases.h"
static agc_t baseline;
/* Runs the unmodified rope from each lm-throttle case entry to the caller's
 * return and prints channel 14 and all 2048 erasable words. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0,fb;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<LM_THR_CASES;++n){apollo_overlay_state s;lm_flight_io io;lm_thr_case c;int done=0;
  machine=baseline;lm_thr_input(&s,&io,&c,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)c.superbank;machine.InputChannel[014]=(int16_t)io.chan14;machine.AllowInterrupt=0;
  for(cycles=0;cycles<20000u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode){pc=get(RegZ);fb=(unsigned)get(4)&076000u;
    if(pc==c.stop_z&&(c.stop_fb==0177777u||fb==c.stop_fb)){done=1;break;}}
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(!done){fprintf(stderr,"lm-throttle case %u timeout %05o\n",n,pc);return 2;}
  printf("%u 0 %u",n,(unsigned)machine.InputChannel[014]&077777u);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<3u?0177777u:077777u));
  putchar('\n');
 }return 0;
}
