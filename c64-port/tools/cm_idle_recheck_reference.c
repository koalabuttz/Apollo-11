/* SPDX-License-Identifier: GPL-2.0-or-later; original CM PRIOCHNG host oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_idle_recheck_cases.h"
#include "cm_priority_full_dump.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t n;unsigned i,cycles;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_IDLE_RECHECK_CASES;++n){apollo_overlay_state s;cm_native_io io;exec_transfer next;unsigned saw_transfer=0,saw_interp=0,saw_direct=0;machine=baseline;cm_idle_recheck_input(&s,&io,n);
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.InputChannel[011]=io.channel11;machine.AllowInterrupt=io.interrupt_enabled;
  for(cycles=0;cycles<2000u;++cycles){unsigned z=get(RegZ),fb=get(RegFB);if(!machine.PendFlag&&!machine.ExtraDelay){if(saw_transfer&&!saw_interp)break;if(saw_interp&&z==06037u)break;if(saw_direct)break;if(fb==02000u&&z==03012u)saw_transfer=1;if(fb==02000u&&z==03016u)saw_interp=1;if(z==1u)saw_direct=1;}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==2000u){fprintf(stderr,"CM priority full timeout %lu PC%05o FB%05o\n",(unsigned long)n,get(RegZ),get(RegFB));return 2;}
  for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.channel11=(agc_word)machine.InputChannel[011];io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;
  next.interpretive=(uint8_t)saw_interp;next.address.location=saw_interp?s.words[0164]:s.words[5];next.address.bank=(agc_word)(s.words[6]|io.superbank);cm_priority_full_dump(n,&s,&io,&next);
 }return 0;
}
