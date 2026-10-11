/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_trig_cases.h"
static agc_t baseline;
static void row(uint16_t n,uint8_t selector){uint16_t w[2051];unsigned i;w[0]=n;w[1]=selector;w[2]=(uint16_t)machine.AllowInterrupt;for(i=0;i<2048u;++i)w[i+3u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));if(fwrite(w,sizeof w,1,stdout)!=1)exit(2);}
int main(int argc,char **argv){uint8_t selector;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;for(selector=0;selector<3u;++selector){uint32_t n;for(n=0;n<cm_trig_count(selector);++n){apollo_overlay_state m;agc_word irq;unsigned i,cycles,pc=0;cm_trig_input(&m,&irq,(uint16_t)n,selector);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=irq;for(cycles=0;cycles<2000u;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&((selector<2u&&(pc==05130u||pc==05133u))||(selector==2u&&pc==03605u&&get(RegFB)==046000u))){row((uint16_t)n,selector);break;}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==2000u){fprintf(stderr,"selector%u case%u timeoutPC%o\n",selector,(unsigned)n,pc);return 2;}if(n<8u)fprintf(stderr,"selector%u case%u PC%o Q%o A%o L%o TEMK%o SQ%o ITEMP1%o ITEMP2%o IRQ%u\n",selector,(unsigned)n,get(5),get(2),(unsigned)*memory(0)&0177777u,(unsigned)*memory(1)&0177777u,get(01075),get(01076),get(061),get(062),machine.AllowInterrupt);}}return 0;}
