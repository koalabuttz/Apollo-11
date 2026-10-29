/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_rectify_cases.h"
static agc_t baseline;
static void row(uint16_t n,uint16_t phase){uint16_t w[2050];unsigned i;w[0]=n;w[1]=phase;for(i=0;i<2048u;++i)w[i+2u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));if(fwrite(w,sizeof w,1,stdout)!=1)exit(2);}
int main(int argc,char **argv){static const unsigned locations[11]={03423,03426,03427,03431,03434,03435,03437,03440,03442,03443,03444};uint16_t n;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_RECTIFY_CASES;++n){apollo_overlay_state image;unsigned i,cycles,phase=cm_rectify_first(n);cm_rectify_input(&image,n);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)image.words[i];machine.AllowInterrupt=0;row(n,12);
 for(cycles=0;cycles<20000u;++cycles){unsigned pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay){if(phase<11u&&pc==06030u&&get(0164)==locations[phase]&&get(023)==0){row(n,(uint16_t)phase);if(n<16u)fprintf(stderr,"%u %u FB%o EB%o BB%o PC%o Q%o LOC%o CYR%o EDOP%o ADDRWD%o INDEXLOC%o POLISH%o IRQ%u\n",n,phase,get(4),get(3),get(6),pc,get(2),get(0164),get(020),get(023),get(0116),get(0130),get(0117),machine.AllowInterrupt);++phase;}if(phase==11u&&pc==06037u&&get(0164)==03700u+n%32u){row(n,(uint16_t)phase);break;}}
 machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 if(cycles==20000u){fprintf(stderr,"case%u phase%u timeoutPC%o LOC%o EDOP%o\n",n,phase,get(RegZ),get(0164),get(023));return 2;}}
 return 0;}
