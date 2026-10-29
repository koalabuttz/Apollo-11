/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_powered_quick_cases.h"
static agc_t baseline;
static void row(uint16_t n,uint8_t phase){uint16_t w[2051];unsigned i;w[0]=n;w[1]=phase;w[2]=(uint16_t)machine.AllowInterrupt;for(i=0;i<2048u;++i)w[i+3u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));if(fwrite(w,sizeof w,1,stdout)!=1)exit(2);}
int main(int argc,char **argv){uint32_t n;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;for(n=0;n<LM_POWERED_QUICK_CASES;++n){apollo_overlay_state m;agc_word irq;unsigned i,cycles,pc=0,phase=0;lm_powered_quick_input(&m,&irq,(uint16_t)n);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=irq;for(cycles=0;cycles<10000u;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay){if(pc==03645u&&phase==0u){row((uint16_t)n,0);phase=1;}else if(pc==m.words[2]&&phase==1u){row((uint16_t)n,1);break;}}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==10000u){fprintf(stderr,"case%u timeoutPC%o Q%o phase%u\n",(unsigned)n,pc,get(2),phase);return 2;}}return 0;}
