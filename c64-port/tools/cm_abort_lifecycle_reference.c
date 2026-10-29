/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original source oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_abort_lifecycle_cases.h"
#include "cm_abort_lifecycle_dump.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t n;unsigned i,cycles,phase;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_ABORT_LIFECYCLE_CASES;++n){apollo_overlay_state s;cm_native_io io;uint8_t in_isr;unsigned kind=n%3u;machine=baseline;(void)cm_abort_lifecycle_input(&s,&io,n,&in_isr);
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;machine.InIsr=in_isr;machine.InputChannel[011]=io.channel11;machine.InputChannel[012]=io.channel12;machine.InputChannel[013]=io.channel13;machine.InputChannel[014]=io.channel14;machine.InputChannel[015]=io.main_keys;machine.InputChannel[016]=io.navigation_keys;machine.Trap31A=io.trap31a;machine.Trap31B=io.trap31b;machine.Trap32=io.trap32;
  for(phase=0;phase<cm_abort_lifecycle_count(n);++phase){unsigned group=cm_abort_lifecycle_group(n,(uint8_t)phase),target,status=EXEC_OK;static const unsigned saved[3]={07762,05726,05754};static const unsigned recovery[7]={05560,05740,05742,05744,02501,04675,07767};
   if(group<5u)target=group==0u?02000u:group==1u?04675u:group==2u?saved[kind]:group==3u?05655u:kind==0u?07766u:05732u;
   else if(group<12u)target=(n/3u&1u)?07766u:recovery[group-5u];else target=group==12u?02651u:0;
   for(cycles=0;cycles<10000u;++cycles){unsigned z=get(RegZ);if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&(target?z==target:(z==05360u||z==02746u||z==02365u))&&(group!=0u||get(RegFB)==040000u)&&(group!=9u||get(RegFB)==012000u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
   if(cycles==10000u){fprintf(stderr,"CM abort lifecycle timeout %lu phase%u group%u PC%05o\n",(unsigned long)n,phase,group,get(RegZ));return 2;}
   if(group==13u)status=get(RegZ)==05360u?CM_ENEMA_DISPLAY:get(RegZ)==02746u?CM_ENEMA_BAD_PHASE:CM_ENEMA_FRESH_START;
   for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;io.channel11=(agc_word)machine.InputChannel[011];io.channel12=(agc_word)machine.InputChannel[012];io.channel13=(agc_word)machine.InputChannel[013];io.channel14=(agc_word)machine.InputChannel[014];io.trap31a=(agc_word)machine.Trap31A;io.trap31b=(agc_word)machine.Trap31B;io.trap32=(agc_word)machine.Trap32;in_isr=(uint8_t)machine.InIsr;cm_abort_lifecycle_dump(n,phase,status,&s,&io,in_isr);
  }
 }return 0;
}
