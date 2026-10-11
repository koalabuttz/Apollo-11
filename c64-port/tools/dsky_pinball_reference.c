/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only original DSPIN/11DSPIN/2BLANK oracle, no native initialization linked. */
#define main original_reference_main
#include "reference_runner.c"
#undef main
#include "dsky_pinball_cases.h"
int main(int argc,char **argv){
 unsigned n,i,cycles,pc,result;static agc_t pristine;static apollo_overlay_state state;
 if(argc!=2 || agc_load_binfile(&pristine,argv[1]))return 2;
 for(n=0;n<DSKY_PINBALL_CASE_COUNT;++n){
  memcpy(&machine,&pristine,sizeof machine);dsky_pinball_prepare(&state,(uint16_t)n);InhibitAlarms=1;
  for(i=0;i<APOLLO_SHARED_WORDS;++i)*memory(i)=(int16_t)state.words[i];
  machine.OutputChannel7=machine.InputChannel[7]=(int)(0100);machine.AllowInterrupt=n%2u;
  result=0;for(cycles=0;cycles<15000;++cycles){pc=get(RegZ);if(!machine.PendFlag && !machine.ExtraDelay && pc==state.words[2] && get(RegQ)==(dsky_pinball_entry_for((uint16_t)n)==DSKY_2BLANK ? 3u : 0115u))break;if(!machine.PendFlag && !machine.ExtraDelay && pc==05716){result=(unsigned)machine.Fixed[1][get(RegQ)&01777]&077777u;break;}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==15000){fprintf(stderr,"pinball%u timeout PC%05o LOC%05o BB%05o\n",n,pc,get(0164),get(RegBB));return 2;}
  printf("%u %u",n,result);
  for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",(unsigned)*memory(i)&(i<=2 ? 0177777u : 077777u));
  printf(" %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt);
  fprintf(stderr,"pinball%u %ucycles PC%05o BB%05o\n",n,cycles,pc,get(RegBB));
 }
 return 0;
}
