/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original source oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_alarm_image_cases.h"
#include "cm_alarm_image_dump.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t n;unsigned i,cycles;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_ALARM_IMAGE_CASES;++n){apollo_overlay_state s;cm_native_io io;unsigned status,target,retired=0;machine=baseline;cm_alarm_image_input(&s,&io,n);target=n%4u==CM_RECORD_ALARM||n%4u==CM_RECORD_BORTENT?s.words[2]+1u:s.words[2];
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  for(cycles=0;cycles<1000u;++cycles){if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&((get(RegZ)==target&&get(RegQ)==(n%4u==CM_RECORD_VARALARM||n%4u==CM_RECORD_CURTAINS?01364u:05715u)&&(n%4u==CM_RECORD_VARALARM||n%4u==CM_RECORD_CURTAINS||retired))||get(RegZ)==05721u))break;machine.ScalerCounter=0;{unsigned returning=get(RegZ)==05714u&&machine.IndexValue!=0;if(agc_engine(&machine))return 2;if(returning&&machine.IndexValue==0)retired=1;}}
  if(cycles==1000u){fprintf(stderr,"CM PRIOLARM timeout %lu PC%05o\n",(unsigned long)n,get(RegZ));return 2;}
  status=EXEC_OK;
  for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;cm_alarm_image_dump(n,(uint16_t)status,&s,&io);
 }return 0;
}
