/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_normal_display_cases.h"
static apollo_overlay_state s;static agc_t pristine;
static const unsigned pc[26]={02707,02770,05210,02775,05201,03036,03312,04171,02337,03602,03604,02537,02623,02566,02570,02537,02623,02566,02570,02537,02623,02566,02570,03344,05217,03231};
static const unsigned fb[26]={020000,020000,020000,020000,020000,020000,020000,020000,062000,062000,062000,062000,060000,062000,062000,062000,060000,062000,062000,062000,060000,062000,062000,020000,020000,02000};
static const unsigned n40_pc[30]={02707,02770,05210,02775,05201,03036,03312,04171,02337,03602,03604,02537,02623,02566,02570,02537,02623,02566,02570,02537,03216,03256,03262,03264,03274,03304,02570,03344,05217,03231};
static const unsigned n40_fb[30]={020000,020000,020000,020000,020000,020000,020000,020000,062000,062000,062000,062000,060000,062000,062000,062000,060000,062000,062000,062000,064000,064000,064000,064000,064000,064000,062000,020000,020000,02000};
int main(int argc,char **argv){unsigned n,i,p,cycles,z;if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;for(n=0;n<CM_NORMAL_DISPLAY_CASES;++n){cm_native_io io;cm_normal_display_input(&s,&io,(uint16_t)n);machine=pristine;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.InputChannel[011]=io.channel11;machine.AllowInterrupt=io.interrupt_enabled;for(p=0;p<(n<512u?30u:26u);++p){for(cycles=0;cycles<12000u;++cycles){z=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&z==(n<512u?n40_pc[p]:pc[p])&&get(4)==(n<512u?n40_fb[p]:fb[p]))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==12000u){fprintf(stderr,"sourceNV case%u p%u timeoutPC%oFB%o\n",n,p,z,get(4));return 2;}printf("%u 1",n*30u+p);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,(unsigned)machine.InputChannel[011]);if(agc_engine(&machine))return 2;}}return 0;}
