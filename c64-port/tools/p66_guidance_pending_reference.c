/* SPDX-License-Identifier: GPL-2.0-or-later
 * Original assembly executes every arithmetic operation and CHANG2. */
#define main inherited_reference_main
#include "reference_runner.c"
#undef main
#include "p66_guidance_live_cases.h"
static agc_t baseline;
static void emit(uint16_t number,unsigned phase,unsigned higher){apollo_overlay_state image;uint16_t words[PGL_FULL_WORDS];unsigned i;for(i=0;i<APOLLO_SHARED_WORDS;++i)image.words[i]=(uint16_t)((unsigned)*memory(i)&(i==RegA||i==RegL?0177777u:077777u));pgl_words(&image,words,higher?0:phase);printf("%u %u %u",number,phase,higher);for(i=0;i<PGL_FULL_WORDS;++i)printf(" %u",words[i]);puts("");}
int main(int argc,char **argv){uint16_t n;unsigned phase;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<16;++n)for(phase=1;phase<PGL_STAGES;++phase){apollo_overlay_state initial;executive state;unsigned i,stage=0,cycles,changed=0,injected=0;pgl_prepare(&initial,&state,n);pgl_prepare_high(&initial,n);machine=baseline;for(i=0;i<APOLLO_SHARED_WORDS;++i)*memory(i)=(int16_t)initial.words[i];machine.InputChannel[7]=machine.OutputChannel7=n&1u?060:0;machine.AllowInterrupt=1;
  for(cycles=0;cycles<60000;++cycles){if(!machine.PendFlag&&!machine.ExtraDelay){
   if(stage==phase&&!injected&&get(RegZ)==06070){set(0067,12);injected=1;}
   if(get(RegZ)==05126){if(!changed){if(stage!=phase+1){fprintf(stderr,"early CHANG2 n%u phase%u stage%u LOC%o EDOP%o\n",n,phase,stage,get(0164),get(023));return 6;}changed=1;}else return 7;}
   if((stage==0&&get(RegZ)==03353)||(stage>0&&stage<=phase&&get(RegZ)==06061&&get(023)==0&&get(0164)==pgl_boundaries[stage])){if(stage==phase)emit(n,phase,0);++stage;}
   else if(stage==phase+1&&get(RegZ)==03353){if(!changed)return 8;emit(n,phase,1);break;}
  }machine.ScalerCounter=0;if(agc_engine(&machine))return 4;}
  if(cycles==60000){fprintf(stderr,"pending timeout n%u phase%u stage%u PC%o LOC%o\n",n,phase,stage,get(RegZ),get(0164));return 5;}
 }return 0;
}
