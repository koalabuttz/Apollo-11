/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main reference_main
#include "cm_executive_reference.c"
#undef main
#include "dsky_fixed_display_cases.h"
static apollo_overlay_state s;static agc_t lm,cm;
int main(int argc,char **argv){uint32_t n;unsigned stage,i,c,pc,cmode,route;enum dsky_nv_mission mission;p66_bias_io io;if(argc!=3||agc_load_binfile(&lm,argv[1])||agc_load_binfile(&cm,argv[2]))return 2;InhibitAlarms=1;
 for(n=0;n<DSKY_FIXED_DISPLAY_CASES;++n){dsky_fixed_display_input(&s,&io,n,&mission);cmode=mission==DSKY_NV_CM;machine=cmode?cm:lm;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  for(stage=0;stage<3u;++stage){route=99;for(c=0;c<3000u;++c){pc=get(RegZ);if(c&&!machine.PendFlag&&!machine.ExtraDelay){if(stage==0u&&pc==(cmode?04715u:04651u))route=DSKY_FIXED_DATA_ENTRY;else if(stage==1u&&pc==03361u&&get(RegFB)==062000u)route=DSKY_FIXED_READ_READY;else if(stage==2u){if(pc==(cmode?05217u:05155u))route=DSKY_FIXED_END;else if(pc==(cmode?05747u:05705u))route=DSKY_FIXED_CCS_HOLE;}if(route!=99u)break;}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
   if(c==3000u){fprintf(stderr,"fixed display timeout n%lu stage%u PC%o\n",(unsigned long)n,stage,pc);return 2;}
   printf("%lu",(unsigned long)(n*3u+stage));for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,route);
  }
 }return 0;}
