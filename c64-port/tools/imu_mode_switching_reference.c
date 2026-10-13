/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original IMU_MODE_SWITCHING_ROUTINES oracle.
 * Links build/agc_engine_no_imu_device.o (tools/prepare_no_imu_device_engine.py):
 * the pinned engine without its gyro, IMU CDU drive and optics peripherals. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "imu_mode_switching_cases.h"
static agc_t baseline[2];
/* Runs the unmodified rope (argv[1] Luminary099, argv[2] Comanche055) from
 * each imu-mode-switching case's image, channels and interrupt state to its
 * stop and prints the case, kind 0, the 2048 erasable words, the
 * interrupt-enable state and channels 11-14. */
int main(int argc,char **argv){unsigned n,i,cycles,pc=0,fb,stopped;if(argc!=3)return 2;InhibitAlarms=1;
 for(i=0;i<2u;++i){memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1+i]))return 2;baseline[i]=machine;}
 for(n=0;n<IMS_CASES;++n){apollo_overlay_state s;ims_case c;const ims_stop *e;
  ims_input(&s,&c,(uint16_t)n);
  if(c.cm>1u){fprintf(stderr,"imu-mode-switching case %u has no site\n",n);return 2;}
  machine=baseline[c.cm];
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)c.superbank;machine.AllowInterrupt=c.interrupt_enabled;
  for(i=0;i<4u;++i)machine.InputChannel[011+i]=(int16_t)c.channel[i];
  stopped=0;
  for(cycles=0;cycles<20000u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode){pc=get(RegZ);fb=(unsigned)get(4)&076000u;
    if(c.stop_z&&(pc==c.stop_z||pc==c.stop_z+1u)&&fb==c.stop_fb){stopped=1;break;}
    if((pc<02000u&&!ims_executed_erasable((agc_word)pc))){stopped=1;break;}
    for(e=ims_externals(&c);e->z;++e)if(pc==e->z&&(e->fb==IMS_ANY_FB||e->fb==fb)){stopped=1;break;}
    if(stopped)break;
   }
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(!stopped){fprintf(stderr,"imu-mode-switching case %u timeout %05o\n",n,pc);return 2;}
  printf("%u 0",n);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));
  printf(" %u",(unsigned)machine.AllowInterrupt);
  for(i=0;i<4u;++i)printf(" %u",(unsigned)machine.InputChannel[011+i]&077777u);
  printf("\n");
 }return 0;
}
