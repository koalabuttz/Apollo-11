/* SPDX-License-Identifier: GPL-2.0-or-later
 * Historical assembly performs the pending-job check and exact CHANG2. */
#define main inherited_reference_main
#include "reference_runner.c"
#undef main
#include "p66_descent_live_cases.h"
#include "p66_descent_pending_counts.h"
static agc_t baseline;
static void emit(uint16_t n,unsigned phase,unsigned higher,unsigned stage){apollo_overlay_state image;p66_bias_io io;uint16_t words[PDL_WORDS];unsigned i;for(i=0;i<2048u;++i)image.words[i]=(uint16_t)((unsigned)*memory(i)&(i==RegA||i==RegL?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;pdl_words(&image,io,(agc_word)machine.InputChannel[014],words,stage);printf("%u %u %u",n,phase,higher);for(i=0;i<PDL_WORDS;++i)printf(" %u",words[i]);puts("");}
int main(int argc,char **argv){uint16_t n;unsigned phase;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<16u;++n)for(phase=23u;phase<=pending_counts[n]-3u;++phase){apollo_overlay_state initial;executive state;p66_bias_io io;agc_word channel;unsigned i,stage=0,cycles,changed=0,injected=0,lastloc=0;
  pdl_prepare(&initial,&state,&io,&channel,n);pgl_prepare_high(&initial,n);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)initial.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;machine.InputChannel[014]=channel;
  for(cycles=0;cycles<100000u;++cycles){unsigned pc=get(RegZ),loc=get(0164);if(!machine.PendFlag&&!machine.ExtraDelay){
   if(stage==phase&&!injected&&pc==06070){set(0067,12);injected=1;}
   if(pc==05126){if(changed||stage!=phase+1u){fprintf(stderr,"early/duplicate CHANG2 n%u phase%u stage%u PC%o LOC%o\n",n,phase,stage,pc,loc);return 6;}changed=1;}
   if(changed&&pc==03353){emit(n,phase,1,0);break;}
   if((stage==0&&pc==03353)||(stage>0&&loc!=lastloc&&(pc==06061||pc==06063)&&get(023)==0)){
    if(stage==phase){if(!injected){fprintf(stderr,"missed injection n%u phase%u LOC%o\n",n,phase,loc);return 7;}emit(n,phase,0,stage);}lastloc=loc;++stage;
   }
  }machine.ScalerCounter=0;if(agc_engine(&machine))return 4;}
  if(cycles==100000u){fprintf(stderr,"pending timeout n%u phase%u stage%u PC%o LOC%o\n",n,phase,stage,get(RegZ),get(0164));return 5;}
 }return 0;
}
