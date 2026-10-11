/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main reference_main
#include "cm_executive_reference.c"
#undef main
#include "dsky_monitor_paste_cases.h"
static apollo_overlay_state s;static agc_t lm,cm;
int main(int argc,char **argv){uint32_t n;unsigned i,c,pc,target;enum dsky_nv_mission mission;enum dsky_monitor_paste_stage stage;p66_bias_io io;if(argc!=3||agc_load_binfile(&lm,argv[1])||agc_load_binfile(&cm,argv[2]))return 2;InhibitAlarms=1;
 for(n=0;n<DSKY_MONITOR_PASTE_CASES;++n){dsky_monitor_paste_input(&s,&io,(uint16_t)n,&mission,&stage);machine=mission==DSKY_NV_CM?cm:lm;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;target=stage==DSKY_MONITOR_PASTE_BEGIN?02337u:stage==DSKY_MONITOR_PASTE_UPDATED?(mission==DSKY_NV_CM?04271u:04255u):(mission==DSKY_NV_CM?05217u:05155u);
  for(c=0;c<3000u;++c){pc=get(RegZ);if(c&&pc==target&&!machine.PendFlag&&!machine.ExtraDelay)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(c==3000u){fprintf(stderr,"monitor paste timeout n%lu PC%o\n",(unsigned long)n,pc);return 2;}printf("%lu",(unsigned long)n);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt);
 }return 0;}
