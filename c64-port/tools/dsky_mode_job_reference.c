/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#include "cm_executive_reference.c"
#undef main
#include "dsky_mode_job_cases.h"
static agc_t ropes[2];
int main(int argc,char **argv){unsigned n,i,cycles,pc,cm,route;if(argc!=3||agc_load_binfile(&ropes[0],argv[1])||agc_load_binfile(&ropes[1],argv[2]))return 2;InhibitAlarms=1;
 for(n=0;n<DSKY_MODE_JOB_CASES;++n){apollo_overlay_state s;p66_bias_io io;dsky_mode_job_input(&s,&io,(uint16_t)n);cm=n>=4096u;machine=ropes[cm];for(i=0;i<2048;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
 for(cycles=0;cycles<4000;++cycles){pc=get(RegZ);if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&(pc==(cm?05217u:05155u)||pc==(cm?05747u:05705u)))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 route=pc==(cm?05747u:05705u)?1u:0u;
 if(cycles==4000){fprintf(stderr,"MODEADMIT%u timeout%05o\n",n,pc);return 2;}printf("%u 0",n);for(i=0;i<2048;++i)printf(" %u",(unsigned)*memory(i)&(i<=2?0177777u:077777u));printf(" %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,route);
 }return 0;}
