/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#include "reference_runner.c"
#undef main
#include "display_job_lifecycle_cases.h"
static apollo_overlay_state s;static agc_t pristine;
int main(int argc,char **argv){unsigned n,i,cycles,pc,phase;if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;for(n=0;n<DISPLAY_JOB_LIFECYCLE_CASES;++n){p66_bias_io io;agc_word channel;display_job_lifecycle_input(&s,&io,&channel,(uint16_t)n);phase=0;machine=pristine;for(i=0;i<APOLLO_SHARED_WORDS;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.InputChannel[011]=channel;machine.AllowInterrupt=io.interrupt_enabled;for(cycles=0;cycles<30000u;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&pc==display_job_lifecycle_boundary((uint16_t)n,(uint16_t)phase)){printf("%u %u 0",n,phase);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,(unsigned)machine.InputChannel[011]);if(++phase==display_job_lifecycle_phases((uint16_t)n))break;}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==30000u){fprintf(stderr,"case%u phase%u timeout%05o\n",n,phase,pc);return 2;}}return 0;}
