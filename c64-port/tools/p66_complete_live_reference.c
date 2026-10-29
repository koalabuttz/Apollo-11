/* SPDX-License-Identifier: GPL-2.0-or-later
 * Historical assembly alone executes all guidance and source services. */
#define main inherited_reference_main
#include "reference_runner.c"
#undef main
#include "p66_complete_live_cases.h"
static agc_t baseline;
static int pcl_display_stop(unsigned pc,unsigned fb){switch(pc){case 02546:case 02553:case 05146:case 05137:case 02614:case 03070:case 03122:case 03121:return fb==020000u;case 04155:return fb==020000u;case 02337:case 03602:case 03604:case 02537:case 02566:case 02570:return fb==062000u;case 02710:case 02717:return fb==060000u;default:return 0;}}
int main(int argc,char **argv){uint16_t n;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<16u;++n){apollo_overlay_state initial,image;executive state;p66_bias_io io;agc_word channel,channel11;unsigned i,cycles,stage=0,done=0,lastloc=0,lastpc=0,display=0;
  pcl_prepare(&initial,&state,&io,&channel,&channel11,n);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)initial.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;machine.InputChannel[014]=channel;machine.InputChannel[011]=channel11;
  for(cycles=0;cycles<100000u;++cycles){unsigned pc=get(RegZ),loc=get(0164);
   if(!machine.PendFlag&&!machine.ExtraDelay && ((stage==0&&pc==03353)||(stage>0&&loc!=lastloc&&(pc==06061||pc==06063)&&get(023)==0)||(pc==03531&&get(RegFB)==064000u)||(pc==05155&&(get(RegFB)==062000u||display))||(display&&pc!=lastpc&&pcl_display_stop(pc,get(RegFB)))||(pc==03214&&get(RegFB)==02000u&&display))){uint16_t words[PCL_WORDS];
    for(i=0;i<2048u;++i)image.words[i]=(uint16_t)((unsigned)*memory(i)&(i==RegA||i==RegL?0177777u:077777u));
    io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;channel=(agc_word)machine.InputChannel[014];
    pcl_words(&image,io,channel,(agc_word)machine.InputChannel[011],words,stage);printf("%u %u %u",n,stage++,pc==03353?03353:(pc==05155||display||pc==03214)?pc:loc);for(i=0;i<PCL_WORDS;++i)printf(" %u",words[i]);puts("");
    lastloc=loc;lastpc=pc;if(pc==05155)display=1;if(pc==03214&&get(RegFB)==02000u){done=1;break;}
   }
   if(!machine.PendFlag&&!machine.ExtraDelay&&pc==03402&&get(RegFB)==0){fprintf(stderr,"source SQRT abort case%u stage%u\n",n,stage);return 3;}
   machine.ScalerCounter=0;if(agc_engine(&machine))return 4;
  }
  if(!done){fprintf(stderr,"source timeout case%u stage%u PC%o LOC%o\n",n,stage,get(RegZ),get(0164));return 5;}
 }
 return 0;
}
