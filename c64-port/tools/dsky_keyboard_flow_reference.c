/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only unchanged original KEYCOM→NUDIRECT→CHARIN, no native linked. */
#define main original_reference_main
#define ChannelOutput original_channel_output
#include "reference_runner.c"
#undef main
#undef ChannelOutput
#include "dsky_keyboard_flow_cases.h"
static unsigned channel11;
void ChannelOutput(agc_t *s,int c,int v){original_channel_output(s,c,v);if(c==011)channel11=(unsigned)v&077777u;}
int main(int argc,char **argv){unsigned n,i,phase,cycles,pc;static agc_t pristine;static apollo_overlay_state state;
 if(argc!=2 || agc_load_binfile(&pristine,argv[1]))return 2;
 for(n=0;n<DSKY_KEYBOARD_FLOW_CASE_COUNT;++n){memcpy(&machine,&pristine,sizeof machine);dsky_keyboard_flow_prepare(&state,(uint16_t)n);InhibitAlarms=1;channel11=dsky_keyboard_flow_channel11((uint16_t)n);machine.InputChannel[011]=(int)channel11;for(i=0;i<APOLLO_SHARED_WORDS;++i)*memory(i)=(int16_t)state.words[i];machine.OutputChannel7=machine.InputChannel[7]=(int)((n/128u)%8u*020u);machine.AllowInterrupt=0;
 for(phase=0;phase<3;++phase){if(phase==1){*memory(RegA)=2;*memory(RegL)=0;*memory(RegZ)=03225;*memory(RegFB)=02000;machine.AllowInterrupt=1;}
 for(cycles=0;cycles<15000;++cycles){pc=get(RegZ);if(!machine.PendFlag && !machine.ExtraDelay && pc==(phase==0 ? 05270u : phase==1 ? 02077u : 05155u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 if(cycles==15000)return 2;printf("%u %u 0",n,phase);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",(unsigned)*memory(i)&(i<=2 ? 0177777u : 077777u));printf(" %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,channel11);
 }}return 0;}
