/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main reference_main
#include "cm_executive_reference.c"
#undef main
#include "dsky_minutes_output_cases.h"
static apollo_overlay_state s;static agc_t lm,cm;
int main(int argc,char **argv){unsigned n,i,k,c,pc;static const unsigned lp[]={03343,03347,03351,03361,03371,02570},cp[]={03256,03262,03264,03274,03304,02570};enum dsky_nv_mission mission;p66_bias_io io;if(argc!=3||agc_load_binfile(&lm,argv[1])||agc_load_binfile(&cm,argv[2]))return 2;InhibitAlarms=1;for(n=0;n<DSKY_MINUTES_OUTPUT_CASES;++n){dsky_minutes_output_input(&s,&io,(uint16_t)n,&mission);machine=mission==DSKY_NV_LM?lm:cm;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;for(k=0;k<6u;++k){for(c=0;c<1500u;++c){pc=get(RegZ);if(c&&!machine.PendFlag&&!machine.ExtraDelay&&pc==(mission==DSKY_NV_LM?lp[k]:cp[k])&&get(4)==(k==5u?062000u:064000u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(c==1500u){fprintf(stderr,"two_integer timeout %u/%u PC%o\n",n,k,pc);return 2;}printf("%u",n*16u+k);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt);}}return 0;}
