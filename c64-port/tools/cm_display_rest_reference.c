/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_display_rest_cases.h"
static apollo_overlay_state s;static agc_t baseline;
int main(int argc,char **argv){unsigned n,i,cycles,pc;cm_native_io io;if(argc!=2||agc_load_binfile(&baseline,argv[1]))return 2;InhibitAlarms=1;for(n=0;n<CM_DISPLAY_REST_CASES;++n){cm_display_rest_input(&s,&io,(uint16_t)n);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;for(cycles=0;cycles<200u;++cycles){pc=get(RegZ);if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&(pc==05217u||pc==04456u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==200u){fprintf(stderr,"RESTsource timeout %u PC%oFB%o\n",n,pc,get(4));return 2;}printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt);}return 0;}
