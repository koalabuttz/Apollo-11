/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#include "reference_runner.c"
#undef main
#include "lm_interpreter_change_cases.h"
static agc_t pristine;static apollo_overlay_state state;
int main(int argc,char **argv){unsigned n,i,cycles,target,pc;if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;
 for(n=0;n<LM_INTERPRETER_CHANGE_CASES;++n){p66_bias_io io;lm_interpreter_change_input(&state,&io,(uint16_t)n);target=(n&1u)?06070u:state.words[0164u+state.words[067]];machine=pristine;for(i=0;i<APOLLO_SHARED_WORDS;++i)*memory(i)=(int16_t)state.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;machine.InputChannel[011]=lm_interpreter_change_channel11((uint16_t)n);machine.InputChannel[014]=lm_interpreter_change_channel14((uint16_t)n);
 for(cycles=0;cycles<1000u;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&pc==target)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==1000u){fprintf(stderr,"case%u timeoutPC%05o\n",n,pc);return 3;}
 printf("%u %u",n,EXEC_OK);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,(unsigned)machine.InputChannel[011],(unsigned)machine.InputChannel[014]);
 }return 0;}
