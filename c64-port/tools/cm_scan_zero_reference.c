/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original source oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_scan_zero_cases.h"
#include "cm_alarm2_image_dump.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t n;unsigned i,cycles;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_SCAN_ZERO_CASES;++n){apollo_overlay_state s;cm_native_io io;unsigned pc,status,target=0,bank=0,winner=0,highest,hole=0;machine=baseline;cm_scan_zero_input(&s,&io,n);
  highest=n/4096u==2u?s.words[0]+(s.words[0167]&0777u):0;
  for(i=1;i<7u;++i){unsigned priority=s.words[0167u+12u*i];if(i!=3u&&!priority){hole=i;break;}if(priority&&priority<040000u&&priority-1u>highest){winner=i;highest=priority-1u;}}
  if(hole)target=05747;
  else if(winner){target=s.words[0164u+12u*winner];bank=s.words[0165u+12u*winner];}
  else if(n/4096u==2u){target=s.words[2];bank=s.words[6]|io.superbank;}
  else target=03231;
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.InputChannel[011]=io.channel11;machine.AllowInterrupt=io.interrupt_enabled;
  for(cycles=0;cycles<2000u;++cycles){pc=get(RegZ);if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&pc==target&&(hole||target==03231u||(get(RegBB)==(bank&076007u)&&machine.OutputChannel7==(bank&0160u))))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==2000u){fprintf(stderr,"CM zero scan timeout %lu PC%05o FB%05o\n",(unsigned long)n,get(RegZ),get(RegFB));return 2;}status=!hole&&!winner&&n/4096u!=2u?EXEC_IDLE:EXEC_OK;
  for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;io.channel11=(agc_word)machine.InputChannel[011];cm_alarm2_image_dump(n,(uint16_t)status,&s,&io);
 }return 0;
}
