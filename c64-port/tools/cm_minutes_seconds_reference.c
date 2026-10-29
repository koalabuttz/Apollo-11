/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_minutes_seconds_cases.h"
static apollo_overlay_state s;static agc_t pristine;
static const unsigned pc[6]={03256,03262,03264,03274,03304,02570};
static const unsigned fb[6]={064000,064000,064000,064000,064000,062000};
int main(int argc,char **argv){unsigned n,i,p,cycles,z;if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;for(n=0;n<CM_MINUTES_SECONDS_CASES;++n){cm_native_io io;cm_minutes_seconds_input(&s,&io,(uint16_t)n);machine=pristine;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.InputChannel[011]=io.channel11;machine.AllowInterrupt=io.interrupt_enabled;for(p=0;p<6u;++p){for(cycles=0;cycles<12000u;++cycles){z=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&z==pc[p]&&get(4)==fb[p])break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==12000u){fprintf(stderr,"sourceNV case%u p%u timeoutPC%oFB%o\n",n,p,z,get(4));return 2;}printf("%u 1",n*6u+p);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt);if(agc_engine(&machine))return 2;}}return 0;}
