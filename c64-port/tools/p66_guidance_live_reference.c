/* SPDX-License-Identifier: GPL-2.0-or-later
 * Executes unchanged historical assembly; C operations only initialize inputs
 * and capture checkpoints. No translated guidance callback enters this oracle. */
#define main inherited_reference_main
#include "reference_runner.c"
#undef main
#include "p66_guidance_live_cases.h"
static agc_t baseline;
int main(int argc,char **argv){uint16_t n;unsigned mixed=(unsigned)(argc==3),steps=mixed?PGL_MIXED_STAGES:PGL_STAGES;if(argc!=2&&argc!=3)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<(mixed?16u:PGL_CASES);++n){apollo_overlay_state initial,image;executive state;unsigned i,cycles,stage=0;pgl_prepare(&initial,&state,n);if(mixed)pgl_prepare_high(&initial,n);machine=baseline;
  for(i=0;i<APOLLO_SHARED_WORDS;++i)*memory(i)=(int16_t)initial.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=n&1u?060:0;machine.AllowInterrupt=1;
  for(cycles=0;cycles<50000;++cycles){
   if(!machine.PendFlag&&!machine.ExtraDelay&&(((stage==0||(mixed&&stage==2))&&get(RegZ)==03353)||((stage>0&&!(mixed&&stage==2))&&get(RegZ)==06061&&get(023)==0&&get(0164)==(mixed?pgl_mixed_boundaries[stage]:pgl_boundaries[stage])))){uint16_t words[PGL_FULL_WORDS];unsigned phase=mixed&&stage>=2?stage-2:stage;
    for(i=0;i<APOLLO_SHARED_WORDS;++i)image.words[i]=(uint16_t)((unsigned)*memory(i)&(i==RegA||i==RegL?0177777u:077777u));
    pgl_words(&image,words,phase);printf("%u %u",n,stage);for(i=0;i<PGL_FULL_WORDS;++i)printf(" %u",words[i]);puts("");if(++stage==steps)break;if(mixed&&stage==1)set(0067,12);
   }
   if(!machine.PendFlag&&!machine.ExtraDelay&&get(RegZ)==03402&&get(RegFB)==0){fprintf(stderr,"unexpected source SQRT abort case%u stage%u\n",n,stage);return 3;}
   machine.ScalerCounter=0;if(agc_engine(&machine))return 4;
  }
  if(stage!=steps){fprintf(stderr,"source timeout case%u stage%u PC%o LOC%o\n",n,stage,get(RegZ),get(0164));return 5;}
 }
 return 0;
}
