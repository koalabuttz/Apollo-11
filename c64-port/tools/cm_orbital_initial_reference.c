/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_orbital_initial_cases.h"
static agc_t baseline;
static void row(uint16_t n,uint16_t phase){uint16_t w[2051];unsigned i;w[0]=n;w[1]=phase;for(i=0;i<2048u;++i)w[i+2u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));w[2050]=(uint16_t)machine.AllowInterrupt;if(fwrite(w,sizeof w,1,stdout)!=1)exit(2);}
int main(int argc,char **argv){static const unsigned locations[3]={03310,03312,03313};uint16_t n;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_ORBITAL_INITIAL_CASES;++n){apollo_overlay_state image;agc_word irq;unsigned i,cycles,phase=0;cm_orbital_initial_input(&image,&irq,n);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)image.words[i];machine.AllowInterrupt=irq;row(n,255);
 for(cycles=0;cycles<20000u;++cycles){unsigned pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay){if((phase<3u&&pc==06030u&&get(0164)==locations[phase]&&get(023)==0)||(phase==3u&&pc==06037u&&(get(0164)==03665u||get(0164)==02512u))){row(n,(uint16_t)phase);if(n<4u)fprintf(stderr,"%u %u FB%o EB%o BB%o PC%o Q%o LOC%o CYR%o EDOP%o ADDRWD%o INDEXLOC%o POLISH%o IRQ%u\n",n,phase,get(4),get(3),get(6),pc,get(2),get(0164),get(020),get(023),get(0116),get(0130),get(0117),machine.AllowInterrupt);++phase;if(phase==4u)break;}}
 machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 if(cycles==20000u){fprintf(stderr,"case%u phase%u timeoutPC%o LOC%o EDOP%o\n",n,phase,get(RegZ),get(0164),get(023));return 2;}}
 return 0;}
