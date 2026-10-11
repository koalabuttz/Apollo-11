/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_fleshpot_cases.h"
static agc_t baseline;
static void row(uint16_t n,uint16_t phase){uint16_t w[2051];unsigned i;w[0]=n;w[1]=phase;w[2]=(uint16_t)machine.AllowInterrupt;for(i=0;i<2048u;++i)w[i+3u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));if(fwrite(w,sizeof w,1,stdout)!=1)exit(2);if(n==0u)fprintf(stderr,"phase%u PC%o Q%o A%o L%o EB%o FB%o BB%o LOC%o BANKSET%o EDOP%o CYR%o ADDRWD%o MODE%o\n",phase,w[8],w[5],w[3],w[4],w[6],w[7],w[9],w[3+0164],w[3+0165],w[3+023],w[3+020],w[3+0116],w[3+0163]);}
int main(int argc,char **argv){uint32_t n,limit=LM_FLESHPOT_CASES;if(argc<2)return 2;if(argc>2)limit=(uint32_t)strtoul(argv[2],NULL,0);memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;for(n=0;n<limit;++n){apollo_overlay_state m;agc_word irq;unsigned i,cycles,pc=0,phase=0;lm_fleshpot_input(&m,&irq,(uint16_t)n);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=irq;for(cycles=0;cycles<15000u;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay){if((phase==0u&&pc==02072u)||(phase==1u&&pc==07460u)||(phase==2u&&pc==06061u)){row((uint16_t)n,(uint16_t)phase++);}else if(phase==3u&&pc==04631u){row((uint16_t)n,3);break;}}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==15000u){fprintf(stderr,"case%u timeoutPC%o Q%o phase%u\n",(unsigned)n,pc,get(2),phase);return 2;}}return 0;}
