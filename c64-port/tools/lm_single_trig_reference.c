/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main base_reference_main
#include "reference_runner.c"
#undef main
#include "lm_single_trig_cases.h"
static agc_t baseline;
static void row(uint16_t n,uint8_t selector,uint8_t phase){uint16_t w[2052];unsigned i;w[0]=n;w[1]=selector;w[2]=phase;w[3]=(uint16_t)machine.AllowInterrupt;for(i=0;i<2048u;++i)w[i+4u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));if(fwrite(w,sizeof w,1,stdout)!=1)exit(2);}
int main(int argc,char **argv){uint8_t selector;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;for(selector=0;selector<2u;++selector){uint32_t n;for(n=0;n<LM_SINGLE_TRIG_CASES;++n){apollo_overlay_state m;agc_word irq;unsigned i,cycles,pc=0,phase=0;lm_single_trig_input(&m,&irq,(uint16_t)n,selector);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=irq;for(cycles=0;cycles<300u;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay){if(!phase&&(pc==05066u||pc==05071u)){row((uint16_t)n,selector,0);phase=1;}else if(phase&&pc==m.words[2]&&get(2)==3){row((uint16_t)n,selector,1);break;}}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==300u){fprintf(stderr,"selector%u case%u timeoutPC%o Q%o phase%u\n",selector,(unsigned)n,pc,get(2),phase);return 2;}}}return 0;}
