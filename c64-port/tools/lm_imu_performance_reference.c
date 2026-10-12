/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original IMU performance test oracle.
 * Links build/agc_engine_no_imu_device.o (tools/prepare_no_imu_device_engine.py). */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_imu_performance_cases.h"
/* Runs the unmodified Luminary099 rope (argv[1]) from each
 * lm-imu-performance case's image, interrupt state and scaler words until
 * ipt_leaves() or an idle point (ipt_idle()) and prints the case, kind 0,
 * the 2048 erasable words and the interrupt-enable state. The scalers stay
 * frozen (ScalerCounter 0). At an idle point with no job waiting the run
 * goes on until it is back there and must find every word unchanged (the
 * spin is a fixed point), else the kind is 1, as for a timeout.
 * With argv[2] = C64 part p (tests/lm_imu_performance_cases.h) the run also
 * stops at a registered key part p leaves out, as the part's chained step
 * does. */
static int at_boundary(unsigned pc){uint16_t ph;
 if(!ipt_part)return 0;
 if(pc==06070u)ph=agc_fixed_physical((agc_word)get(0164),(agc_word)get(0165),(agc_word)machine.InputChannel[7]);
 else if(pc>=04000u)ph=(uint16_t)(2048u+pc-04000u);
 else if(pc>=02000u)ph=agc_fixed_physical((agc_word)pc,(agc_word)(get(4)&076000u),(agc_word)machine.InputChannel[7]);
 else return 0;
 return ipt_is_key(ph)&&!ipt_part_has(ph);
}
static int boundary(void){return !machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode;}
int main(int argc,char **argv){unsigned n,i;unsigned long cycles;agc_t baseline;if(argc!=2&&argc!=3)return 2;InhibitAlarms=1;
 if(argc==3)ipt_part=(unsigned)atoi(argv[2]);
 memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;
 for(n=0;n<IPT_CASES;++n){apollo_overlay_state s;ipt_case c;unsigned pc=0,prev=0,stopped=0;static int16_t held[2048];int ie=0;
  ipt_input(&s,&c,(uint16_t)n);
  machine=baseline;
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)c.superbank;machine.AllowInterrupt=c.interrupt_enabled;
  machine.InputChannel[3]=(int16_t)c.hiscalar;machine.InputChannel[4]=(int16_t)c.loscalar;
  for(cycles=0;cycles<4000000ul;++cycles){
   if(boundary()){
    pc=get(RegZ);
    if(cycles&&(ipt_leaves((agc_word)pc,(agc_word)(get(4)&076000u),(agc_word)machine.InputChannel[7],(agc_word)get(0164),(agc_word)get(0165))||
     ipt_idle((agc_word)pc,(agc_word)prev)||at_boundary(pc))){stopped=1;break;}
    prev=pc;
   }
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(!stopped)fprintf(stderr,"lm-imu-performance case %u timeout %05o\n",n,pc);
  for(i=0;i<2048u;++i)held[i]=*memory(i);
  ie=machine.AllowInterrupt;
  /* A spin with no job waiting must come back to the same state. */
  if(stopped&&ipt_idle((agc_word)pc,(agc_word)prev)&&(get(067)==0||get(067)>037777)){unsigned again=0;prev=pc;
   for(cycles=0;cycles<2000ul&&!again;++cycles){
    machine.ScalerCounter=0;if(agc_engine(&machine))return 2;
    if(boundary()){unsigned z=get(RegZ);if(ipt_idle((agc_word)z,(agc_word)prev))again=1;prev=z;}
   }
   for(i=0;i<2048u&&again;++i)if(*memory(i)!=held[i])again=0;
   if(!again||machine.AllowInterrupt!=ie){stopped=0;fprintf(stderr,"lm-imu-performance case %u: idle point %05o is not a fixed point\n",n,pc);}
  }
  printf("%u %u",n,stopped?0u:1u);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)held[i]&(i==0||i==1?0177777u:077777u));
  printf(" %u\n",(unsigned)ie);
 }return 0;
}
