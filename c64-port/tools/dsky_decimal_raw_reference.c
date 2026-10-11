/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main reference_main
#include "cm_executive_reference.c"
#undef main
#include "dsky_decimal_raw_cases.h"
static apollo_overlay_state s;static agc_t lm,cm;
int main(int argc,char **argv){unsigned n,i,cycles,pc,caller,route;enum dsky_nv_mission mission;uint8_t rounded;p66_bias_io io;if(argc!=3||agc_load_binfile(&lm,argv[1])||agc_load_binfile(&cm,argv[2]))return 2;InhibitAlarms=1;for(n=0;n<DSKY_DECIMAL_RAW_CASES;++n){dsky_decimal_raw_input(&s,&io,n,&mission,&rounded);caller=s.words[2];route=0;machine=mission==DSKY_NV_LM?lm:cm;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;for(cycles=0;cycles<1000u;++cycles){pc=get(RegZ);if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&get(4)==060000u){if(pc==caller&&get(2)==0116u)break;if(pc==(mission==DSKY_NV_LM?05705u:05747u)&&get(2)==(mission==DSKY_NV_LM?03354u:03261u)){route=1;break;}}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==1000u){fprintf(stderr,"decimal general timeout %u PC%oFB%oQ%o\n",n,pc,get(4),get(2));return 2;}printf("%lu",(unsigned long)n);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,route);}return 0;}
