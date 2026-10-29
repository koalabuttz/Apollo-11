/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main reference_main
#include "cm_executive_reference.c"
#undef main
#include "dsky_fraction_pair_cases.h"
static apollo_overlay_state s;static agc_t lm,cm;
int main(int argc,char **argv){unsigned n,i,c,pc,caller,returned;enum dsky_nv_mission mission;p66_bias_io io;if(argc!=3||agc_load_binfile(&lm,argv[1])||agc_load_binfile(&cm,argv[2]))return 2;InhibitAlarms=1;for(n=0;n<DSKY_FRACTION_PAIR_CASES;++n){dsky_fraction_pair_input(&s,&io,(uint16_t)n,&mission);machine=mission==DSKY_NV_LM?lm:cm;caller=s.words[2];returned=0;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;for(c=0;c<1500u;++c){pc=get(RegZ);if(c&&!machine.PendFlag&&!machine.ExtraDelay&&((returned&&pc==caller&&get(2)==0116u)||pc==(mission==DSKY_NV_LM?05705u:05747u))&&get(4)==060000u)break;if(!machine.PendFlag&&!machine.ExtraDelay&&pc==(mission==DSKY_NV_LM?03257u:03164u))returned=1;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(c==1500u){fprintf(stderr,"fraction_pair timeout %u PC%o\n",n,pc);return 2;}printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,get(2)==0116u?0u:1u);}return 0;}
