/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original IMU_COMPENSATION_PACKAGE oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "imu_compensation_cases.h"
static agc_t baseline[2];
/* Runs the unmodified rope (argv[1] Luminary099, argv[2] Comanche055) from
 * each imu-compensation case's image to its stop: the caller's continuation
 * of a round trip, or an external address of the case, and prints the case,
 * kind 0, the 2048 erasable words and the interrupt-enable state. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0,fb,stopped;if(argc!=3)return 2;InhibitAlarms=1;
 for(i=0;i<2u;++i){memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1+i]))return 2;baseline[i]=machine;}
 for(n=0;n<ICP_CASES;++n){apollo_overlay_state s;icp_case c;const agc_word *e;
  icp_input(&s,&c,(uint16_t)n);machine=baseline[c.cm];
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)c.superbank;machine.AllowInterrupt=c.interrupt_enabled;stopped=0;
  for(cycles=0;cycles<20000u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode){pc=get(RegZ);fb=(unsigned)get(4)&076000u;
    if(c.stop_z&&pc==c.stop_z&&fb==c.stop_fb){stopped=1;break;}
    for(e=icp_externals(&c);*e;++e)if(pc==*e){stopped=1;break;}
    if(stopped)break;
   }
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(!stopped){fprintf(stderr,"imu-compensation case %u timeout %05o\n",n,pc);return 2;}
  printf("%u 0",n);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));
  printf(" %u\n",(unsigned)machine.AllowInterrupt);
 }return 0;
}
