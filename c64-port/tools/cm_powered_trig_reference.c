/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_powered_trig_cases.h"
static agc_t baseline;
static void row(uint16_t n,uint8_t live,uint8_t phase){uint16_t w[2052];unsigned i;w[0]=n;w[1]=live;w[2]=phase;w[3]=(uint16_t)machine.AllowInterrupt;for(i=0;i<2048u;++i)w[i+4u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));if(fwrite(w,sizeof w,1,stdout)!=1)exit(2);}
int main(int argc,char **argv){uint8_t live;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;for(live=0;live<2u;++live){uint32_t n;for(n=0;n<CM_POWERED_TRIG_CASES;++n){apollo_overlay_state m;agc_word irq;unsigned i,cycles,pc=0,phase=0;cm_powered_trig_input(&m,&irq,(uint16_t)n,live);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=irq;for(cycles=0;cycles<10000u;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay){if(pc==m.words[2]&&get(2)==0143u){if(phase!=9u){fprintf(stderr,"wrongphase%u\n",phase);return 2;}row((uint16_t)n,live,9);break;}if(pc==06030u){if(phase>=9u){fprintf(stderr,"extraDANZIG%u\n",phase);return 2;}row((uint16_t)n,live,(uint8_t)phase++);}}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==10000u){fprintf(stderr,"live%u case%u timeoutPC%o Q%o phase%u\n",live,(unsigned)n,pc,get(2),phase);return 2;}}}return 0;}
