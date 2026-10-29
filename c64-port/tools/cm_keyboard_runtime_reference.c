/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#define ChannelOutput original_channel_output
#include "cm_executive_reference.c"
#undef main
#undef ChannelOutput
#include "cm_keyboard_runtime_cases.h"
static agc_t pristine;static unsigned event;
void ChannelOutput(agc_t *s,int c,int v){original_channel_output(s,c,v);if(c==011)event=1;}
static void checkpoint(unsigned n,unsigned j,unsigned status,unsigned event,unsigned loc,unsigned bank){unsigned i;printf("%u %u %u",n,j,status);for(i=0;i<2048;++i)printf(" %u",(unsigned)*memory(i)&(i<=2?0177777u:077777u));printf(" %u %u %u %u %u %u %u 0\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,(unsigned)machine.InputChannel[011],(unsigned)machine.InIsr,event,loc,bank);}
int main(int argc,char **argv){unsigned n,i,j,cycles,pc;if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;
 for(n=0;n<CM_KEYBOARD_RUNTIME_CASES;++n){apollo_overlay_state s;cm_native_io io;uint16_t isr,key;cm_keyboard_runtime_input(&s,&io,&isr,&key,(uint16_t)n);machine=pristine;for(i=0;i<2048;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;machine.InIsr=isr;machine.InputChannel[015]=key;machine.InputChannel[011]=io.channel11;
 for(j=0;j<4;++j){event=0;for(cycles=0;cycles<10000;++cycles){pc=get(RegZ);if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&(j==0 ? !machine.InIsr : j==1 ? pc==02000&&get(RegFB)==060000 : j==2 ? pc==05217 : pc==03231))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 if(cycles==10000){fprintf(stderr,"CM keyboardruntime%u stage%u timeout%05o BB%05o\n",n,j,pc,get(RegBB));return 2;}
 checkpoint(n,j,j==3 ? EXEC_IDLE : EXEC_OK,event,j==3 ? 0 : pc,j==3 ? 0 : (unsigned)get(RegBB)|(unsigned)machine.OutputChannel7);
 }}return 0;}
