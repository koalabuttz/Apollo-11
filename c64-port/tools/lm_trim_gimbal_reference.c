/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Luminary099 TRIM_GIMBAL_CONTROL_SYSTEM oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_trim_gimbal_cases.h"
static agc_t baseline;
/* Runs the unmodified rope from each lm-trim-gimbal case entry to CLOSEOUT
 * (GTS), the caller's return (ACDT+C12, 21,3435) or ACDT+C12 reached
 * through IBNKCALL (TIMEGMBL), and prints the outcome (LM_TG_NONE after
 * 40000 cycles), channel 12 and all 2048 erasable words. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0,fb,kind;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<LM_TG_CASES;++n){apollo_overlay_state s;lm_flight_io io;lm_tg_case c;
  machine=baseline;lm_tg_input(&s,&io,&c,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)c.superbank;machine.AllowInterrupt=0;kind=LM_TG_NONE;
  machine.InputChannel[012]=(int16_t)io.chan12;
  for(cycles=0;cycles<40000u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode){pc=get(RegZ);fb=(unsigned)get(4)&076000u;
    if(n%LM_TG_ENTRIES!=0u&&pc==c.stop_z&&(c.stop_fb==0177777u||fb==c.stop_fb)){kind=LM_TG_RETURNED;break;}
    if(n%LM_TG_ENTRIES==0u&&pc==LM_TG_CLOSEOUT&&fb==036000u){kind=LM_TG_CLOSED;break;}
   }
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  /* A GTS case the native routine refuses may run anywhere here (its
   * INDEX SHFTFLAG forms an arbitrary instruction); it prints kind
   * LM_TG_NONE on a timeout and the freeze leaves it out. */
  printf("%u %u %u",n,kind,(unsigned)machine.InputChannel[012]&077777u);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<2u?0177777u:077777u));
  putchar('\n');
 }return 0;
}
