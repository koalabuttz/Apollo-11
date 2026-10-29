/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main reference_main
#include "cm_executive_reference.c"
#undef main
#include "dsky_octal_verb_cases.h"
static apollo_overlay_state s;static agc_t lm,cm;
int main(int argc,char **argv){unsigned n,i,cycles,pc;enum dsky_nv_mission mission;uint8_t entry;p66_bias_io io;if(argc!=3||agc_load_binfile(&lm,argv[1])||agc_load_binfile(&cm,argv[2]))return 2;InhibitAlarms=1;for(n=0;n<DSKY_OCTAL_VERB_CASES;++n){dsky_octal_verb_input(&s,&io,(uint16_t)n,&mission,&entry);machine=mission==DSKY_NV_LM?lm:cm;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;if(entry==2u)machine.InputChannel[s.words[01017]&0777u]=s.words[01020];for(cycles=0;cycles<400u;++cycles){pc=get(RegZ);if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&(pc==03363u||pc==0136u||pc==02351u||pc==02504u)&&get(4)==062000u)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==400u){fprintf(stderr,"decimal row timeout %u PC%oFB%oQ%o\n",n,pc,get(4),get(2));return 2;}printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt);}return 0;}
