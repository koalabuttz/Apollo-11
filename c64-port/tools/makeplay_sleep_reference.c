/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#include "reference_runner.c"
#undef main
#include "makeplay_sleep_cases.h"
static apollo_overlay_state s;
static agc_t pristine;
int main(int argc,char **argv){unsigned n,i,cycles,pc;if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;for(n=0;n<MAKEPLAY_SLEEP_CASES;++n){p66_bias_io io;makeplay_sleep_input(&s,&io,(uint16_t)n);machine=pristine;for(i=0;i<APOLLO_SHARED_WORDS;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
 for(cycles=0;cycles<2000;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&((n<4096u&&pc==05137u)||(n>=4096u&&n<8192u&&pc==05133u)||(n>=8192u&&n<12288u&&pc==03070u)||(n>=12288u&&(pc==05155u||pc==04442u))))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 if(cycles==2000){fprintf(stderr,"case%u timeout PC%05o\n",n,pc);return 2;}printf("%u %u",n,(unsigned)(pc==05137u?0u:pc==05133u?1u:pc==03070u?2u:pc==05155u?3u:4u));for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt);
}return 0;}
