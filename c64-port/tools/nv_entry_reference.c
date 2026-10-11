/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#include "reference_runner.c"
#undef main
#include "nv_entry_cases.h"
static apollo_overlay_state s;
static agc_t pristine;
int main(int argc,char **argv){unsigned n,i,cycles,pc;if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;for(n=0;n<NV_ENTRY_CASES;++n){p66_bias_io io;nv_entry_input(&s,&io,(uint16_t)n);machine=pristine;for(i=0;i<APOLLO_SHARED_WORDS;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
 for(cycles=0;cycles<2000;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&(pc==03121u||pc==02337u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 if(cycles==2000){fprintf(stderr,"case%u timeout PC%05o\n",n,pc);return 2;}printf("%u %u",n,pc==03121u?1u:0u);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt);
}return 0;}
