/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original Comanche055 DOWN-TELEMETRY_PROGRAM oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_downtelemetry_cases.h"
static agc_t baseline;
static int quiet(void){return !machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode;}
static const unsigned in_channels[8]={011,012,013,014,030,031,032,033};
/* Each case: the same initial image and, before each downrupt, the same
 * lead-in registers and input channels as tests/cm_downtelemetry_cases.c;
 * runs DODOWNTM until Z reaches RESUME. Prints channels 13, 34 and 35 and Q after
 * each downrupt, then the channel-13 traps and all 2048 erasable words. */
int main(int argc,char **argv){unsigned n,i,k,cycles;
 if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_DNTM_CASES;++n){apollo_overlay_state s;cm_dntm_io io;unsigned steps=cm_dntm_steps((uint16_t)n);
  cm_dntm_begin(&s,&io,(uint16_t)n);
  machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.AllowInterrupt=0;
  printf("%u",n);
  for(k=0;k<steps;++k){agc_word in[8];
   for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<2u?0177777u:077777u));
   cm_dntm_entry(&s,&io,(uint16_t)n,(uint16_t)k);
   for(i=0;i<7u;++i)*memory(i)=(int16_t)s.words[i];
   in[0]=io.io.channel11;in[1]=io.io.channel12;in[2]=io.io.channel13;in[3]=io.io.channel14;
   in[4]=io.channel30;in[5]=io.channel31;in[6]=io.channel32;in[7]=io.channel33;
   for(i=0;i<8u;++i)machine.InputChannel[in_channels[i]]=(int16_t)in[i];
   for(cycles=0;cycles<4000u;++cycles){
    if(cycles&&quiet()&&get(RegZ)==CM_DNTM_RESUME)break;
    machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
   if(cycles==4000u){fprintf(stderr,"CM downtelemetry %u step %u timeout %05o\n",n,k,get(RegZ));return 2;}
   printf(" %u %u %u %u",(unsigned)machine.InputChannel[013]&077777u,(unsigned)machine.InputChannel[034]&077777u,(unsigned)machine.InputChannel[035]&077777u,(unsigned)*memory(2)&077777u);
  }
  printf(" %u %u %u",(unsigned)machine.Trap31A,(unsigned)machine.Trap31B,(unsigned)machine.Trap32);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));putchar('\n');
 }return 0;
}
