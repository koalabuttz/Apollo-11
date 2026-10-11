/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main inherited_reference_main
#include "reference_runner.c"
#undef main
#include "p66_recurrence_live_cases.h"
extern int CounterPINC(int16_t *);
static agc_t baseline;
static int display_stop(unsigned pc,unsigned fb){switch(pc){case 02546:case 02553:case 05146:case 05137:case 02614:case 03070:case 03122:case 03121:case 04155:return fb==020000u;case 02337:case 03602:case 03604:case 02537:case 02566:case 02570:return fb==062000u;case 02710:case 02717:case 03534:return fb==060000u;default:return 0;}}
int main(int argc,char **argv){uint16_t n;if(argc!=2||agc_load_binfile(&baseline,argv[1]))return 2;InhibitAlarms=1;
 for(n=0;n<16u;++n){apollo_overlay_state image;executive state;p66_bias_io io;agc_word c14,c11;unsigned i,cycles,stage=0,lastloc=0,lastpc=0,descent=0,display=0,done=0,recur=0,injected=0,mode=0;
  prl_prepare(&image,&state,&io,&c14,&c11,n);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)image.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;machine.InputChannel[014]=c14;machine.InputChannel[011]=c11;
  for(cycles=0;cycles<120000u;++cycles){unsigned pc=get(RegZ),fb=get(RegFB),loc=get(0164);
   if(!machine.PendFlag&&!machine.ExtraDelay&&(((recur&&(pc==03263u||pc==05261u)&&pc!=lastpc))||((!descent||recur==2)&&pc==03275u&&fb==064000u)||(descent&&pc==03353u&&fb==064000u&&lastpc!=pc)||(descent&&loc!=lastloc&&(pc==06061u||pc==06063u)&&get(023)==0u)||(pc==03534u&&fb==060000u&&pc!=lastpc)||(pc==06070u&&loc==03357u&&mode&&pc!=lastpc)||(pc==03531u&&fb==064000u)||(pc==05155u&&(fb==060000u||fb==062000u||display))||(display&&pc!=lastpc&&display_stop(pc,fb))||(pc==03214u&&fb==02000u&&display&&pc!=lastpc))){
    if(pc==03353u&&!injected){if(get(0200)!=03534u||get(0201)!=060102u)return 8;set(0203,(agc_word)(037000u|(get(0203)&0777u)));set(067,014);injected=1;}
    printf("%u %u %u",n,stage++,pc==03275u||pc==03353u||pc==03534u||pc==06070u||pc==05155u||display||pc==03214u||pc==03263u||pc==05261u?pc:loc);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=1u?0177777u:077777u));printf(" %u %u %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,(unsigned)machine.InputChannel[014],(unsigned)machine.InputChannel[011],(unsigned)machine.InIsr);
    lastloc=loc;lastpc=pc;if(pc==03275u)descent=1;if(pc==03534u&&!display)mode=1;if(pc==05155u&&!mode)display=1;if(pc==06070u&&mode)mode=0;if(pc==03214u&&fb==02000u){if(recur==0){unsigned tick;for(tick=0;tick<100u;++tick){if(CounterPINC(memory(024)))CounterPINC(memory(025));if(CounterPINC(memory(026)))machine.InterruptRequests[3]=1;}if(machine.InterruptRequests[3]!=1)return 7;recur=1;}else if(recur==1){recur=2;display=0;lastloc=0;}else{done=1;break;}}
   }
   if(!machine.PendFlag&&!machine.ExtraDelay&&pc==03402u&&fb==0u){fprintf(stderr,"sourceSQRT case%u stage%u\n",n,stage);return 3;}
   machine.ScalerCounter=0;if(agc_engine(&machine))return 4;
  }if(!done){fprintf(stderr,"case%u source timeout PC%o LOC%o\n",n,get(RegZ),get(0164));return 5;}
 }return 0;
}
