/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_clockjob_cases.h"
static agc_t baseline;
int main(int argc,char **argv){unsigned n,i,cycles,pc=0;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;for(n=0;n<CM_CLOCKJOB_CASES;++n){apollo_overlay_state s;cm_native_io io;unsigned status;machine=baseline;cm_clockjob_input(&s,&io,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;for(cycles=0;cycles<1000u;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&((n<8192u&&pc==03555u&&get(RegFB)==046000u)||(n>=8192u&&n<16384u&&pc==03342u&&get(RegFB)==050000u)||(n>=16384u&&(pc==05217u||(get(RegFB)==020000u&&(pc==02707u||pc==02715u))))))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==1000u){fprintf(stderr,"CM CLOCKJOB case%u timeout%05o\n",n,pc);return 2;}status=pc==03555u?1u:pc==03342u?2u:pc==05217u?0u:pc==02707u?3u:4u;printf("%u %u",n,status);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,machine.AllowInterrupt);}return 0;}
