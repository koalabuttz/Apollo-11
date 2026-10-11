/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_single_return_cases.h"
static agc_t baseline;
static void row(uint16_t n,uint8_t phase){uint16_t w[2052];unsigned i;w[0]=n;w[1]=(uint16_t)(n/3072u);w[2]=phase;w[3]=(uint16_t)machine.AllowInterrupt;for(i=0;i<2048u;++i)w[i+4u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));if(fwrite(w,sizeof w,1,stdout)!=1)exit(2);}
int main(int argc,char **argv){uint16_t n;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;for(n=0;n<CM_SINGLE_RETURN_CASES;++n){apollo_overlay_state m;agc_word irq;unsigned i,cycles,pc=0,phase=0;cm_single_return_input(&m,&irq,n);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=irq;for(cycles=0;cycles<300u;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay){if(!phase&&(pc==05130u||pc==05133u)){row(n,0);phase=1;}else if(phase&&pc==m.words[2]&&get(2)==3){row(n,1);break;}}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==300u){fprintf(stderr,"case%u timeoutPC%o Q%o phase%u\n",n,pc,get(2),phase);return 2;}}return 0;}
