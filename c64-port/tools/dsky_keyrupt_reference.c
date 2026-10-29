/* SPDX-License-Identifier: GPL-2.0-or-later
 * Independent unchanged original KEYRUPT1; never native code linked. */
#define main original_reference_main
#include "reference_runner.c"
#undef main
#include "dsky_keyrupt_cases.h"
int main(int argc,char **argv){unsigned n,i,cycles,pc,result;static agc_t pristine;static apollo_overlay_state state;if(argc!=2 || agc_load_binfile(&pristine,argv[1]))return 2;
 for(n=0;n<DSKY_KEYRUPT_CASE_COUNT;++n){memcpy(&machine,&pristine,sizeof machine);dsky_keyrupt_prepare(&state,(uint16_t)n);InhibitAlarms=1;for(i=0;i<APOLLO_SHARED_WORDS;++i)*memory(i)=(int16_t)state.words[i];machine.OutputChannel7=machine.InputChannel[7]=(int)((n/512u)%8u*020u);machine.AllowInterrupt=n%2u;machine.InputChannel[015]=dsky_keyrupt_channel15((uint16_t)n);
 result=0;for(cycles=0;cycles<3000;++cycles){pc=get(RegZ);if(!machine.PendFlag && !machine.ExtraDelay && pc==05270)break;if(!machine.PendFlag && !machine.ExtraDelay && pc==05716){result=(unsigned)machine.Fixed[1][get(RegQ)&01777]&077777u;break;}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==3000)return 2;
 printf("%u %u",n,result);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",(unsigned)*memory(i)&(i<=2 ? 0177777u : 077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt);
 }return 0;}
