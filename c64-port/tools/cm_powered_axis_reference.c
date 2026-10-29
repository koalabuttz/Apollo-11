/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_powered_axis_cases.h"
static agc_t baseline;
static void row(uint32_t n,unsigned phase){uint16_t w[2052];unsigned i;w[0]=(uint16_t)n;w[1]=(uint16_t)(n>>16);w[2]=(uint16_t)phase;w[3]=(uint16_t)machine.AllowInterrupt;for(i=0;i<2048u;++i)w[i+4u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));if(fwrite(w,sizeof w,1,stdout)!=1)exit(2);}
int main(int argc,char **argv){uint32_t n;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;for(n=0;n<CM_POWERED_AXIS_CASES;++n){apollo_overlay_state m;agc_word irq;unsigned i,cycles,pc=0,phase=0;cm_powered_axis_input(&m,&irq,n);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=irq;for(cycles=0;cycles<10000u;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay){if(pc==0145u&&phase==0u){if(n==0u)fprintf(stderr,"pre-returnQ%o\n",get(2));row(n,0);phase=1;}else if(pc==m.words[2]&&phase==1u){row(n,1);break;}}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==10000u){fprintf(stderr,"case%u timeoutPC%o Q%o phase%u\n",(unsigned)n,pc,get(2),phase);return 2;}}return 0;}
