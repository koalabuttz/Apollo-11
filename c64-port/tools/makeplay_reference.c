/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#include "reference_runner.c"
#undef main
#include "makeplay_cases.h"
static apollo_overlay_state s;static agc_t pristine;
int main(int argc,char **argv){unsigned n,i,cycles,pc,status;if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;for(n=0;n<MAKEPLAY_CASES;++n){p66_bias_io io;agc_word channel;makeplay_input(&s,&io,&channel,(uint16_t)n);machine=pristine;for(i=0;i<APOLLO_SHARED_WORDS;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.InputChannel[011]=channel;machine.AllowInterrupt=io.interrupt_enabled;
for(cycles=0;cycles<2000u;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&((n<4096u&&pc==05146u)||(n>=4096u&&n<8192u&&(pc==05137u||pc==02754u||pc==05155u))||(n>=8192u&&pc==03070u)))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
if(cycles==2000u){fprintf(stderr,"case%u timeout%05o\n",n,pc);return 2;}status=pc==05146u?0u:pc==05137u?1u:pc==02754u?2u:pc==05155u?3u:4u;printf("%u %u",n,status);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,(unsigned)machine.InputChannel[011]);}return 0;}
