/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original source oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_abort_snapshot_cases.h"
#include "cm_fixed_compat_dump.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t n;unsigned i,cycles,phase;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_ABORT_SNAPSHOT_CASES;++n){apollo_overlay_state s;cm_native_io io;unsigned kind=n%3u;static const unsigned returns[3]={07762,05726,05754};machine=baseline;cm_abort_snapshot_input(&s,&io,n);
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  for(phase=0;phase<5u;++phase){unsigned target=phase==0u?02000u:phase==1u?04675u:phase==2u?returns[kind]:phase==3u?05655u:kind==0u?07766u:05732u;
   for(cycles=0;cycles<1000u;++cycles){if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&get(RegZ)==target&&(phase!=0u||get(RegFB)==040000u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
   if(cycles==1000u){fprintf(stderr,"CM abort snapshot timeout %lu phase%u PC%05o\n",(unsigned long)n,phase,get(RegZ));return 2;}
   for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;cm_fixed_dump(n,(uint16_t)phase,EXEC_OK,&s,&io);
  }
 }return 0;
}
