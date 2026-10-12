/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original source oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_alarm_sites_cases.h"
#include "cm_alarm_complete_dump.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t n,cases=cm_alarm_sites_cases();unsigned i,cycles;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<cases;++n){apollo_overlay_state s;cm_native_io io;unsigned target,retired=0,isr;enum cm_alarm_sites_kind kind;machine=baseline;kind=cm_alarm_sites_input(&s,&io,n);
  {uint16_t count;(void)cm_alarm_site_table(&count);isr=kind==CM_SITES_ABORT?0u:(n/(3u*count))&1u;}target=kind==CM_SITES_ABORT?05655u:s.words[2]+1u;
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;machine.InIsr=(int)isr;
  for(cycles=0;cycles<1000u;++cycles){if(cycles&&!machine.PendFlag&&!machine.ExtraDelay){unsigned z=get(RegZ);if(kind==CM_SITES_ABORT?z==05655u:((z==target&&get(RegQ)==05715u&&retired)||z==05721u))break;}machine.ScalerCounter=0;{unsigned returning=get(RegZ)==05714u&&machine.IndexValue!=0;if(agc_engine(&machine))return 2;if(returning&&machine.IndexValue==0)retired=1;}}
  if(cycles==1000u){fprintf(stderr,"cm-alarm-sites timeout %lu PC%05o\n",(unsigned long)n,get(RegZ));return 2;}
  if(machine.InIsr!=(int)isr)return 2;
  for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;cm_alarm_complete_dump(n,EXEC_OK,&s,&io);
 }return 0;
}
