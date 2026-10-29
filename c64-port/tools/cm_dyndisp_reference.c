/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_dyndisp_cases.h"
static apollo_overlay_state s;static agc_t pristine;
static const uint16_t pcs[27]={06037,06037,06030,06037,06030,06030,06037,06037,06037,06030,07303,07313,07330,07341,06030,06037,06037,07303,07313,07330,07341,06030,06030,06030,06037,06037,03342};
static const uint16_t locs[27]={03326,02370,02371,02373,02374,02374,03431,03477,03434,03434,03436,03436,03436,03436,03436,03644,03440,03441,03441,03441,03441,03441,03442,03443,02377,03330,03330};
int main(int argc,char **argv){unsigned n,i,phase,cycles,pc;if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;for(n=0;n<CM_DYNDISP_CASES;++n){cm_native_io io;cm_dyndisp_input(&s,&io,(uint16_t)n);machine=pristine;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;for(phase=0;phase<27u;++phase){for(cycles=0;cycles<16000u;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&pc==pcs[phase]&&get(0164)==locs[phase]&&(phase!=26u||get(4)==050000u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==16000u){fprintf(stderr,"source case%u phase%u timeoutPC%o LOC%o BB%o EDOP%o\n",n,phase,pc,get(0164),get(6),get(023));return 2;}printf("%u 1",n*27u+phase);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt);if(agc_engine(&machine))return 2;}}return 0;}
