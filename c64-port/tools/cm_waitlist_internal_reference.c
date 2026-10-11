/* SPDX-License-Identifier: GPL-2.0-or-later; original CM internal WAITLIST entries. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_waitlist_internal_cases.h"
#include "cm_alarm2_image_dump.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t n;unsigned i,cycles;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_WAITLIST_INTERNAL_CASES;++n){apollo_overlay_state s;cm_native_io io;unsigned saw_dtcB=0,status=CM_WL_OK;machine=baseline;(void)cm_waitlist_internal_input(&s,&io,n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  for(cycles=0;cycles<2000u;++cycles){unsigned z=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay){if(z==01203u&&get(RegQ)==03377u){status=CM_WL_SOURCE_OVERFLOW;break;}if(z==07755u){status=CM_WL_FULL;break;}if(saw_dtcB)break;if(z==05265u)saw_dtcB=1;}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==2000u){fprintf(stderr,"CM internal wait timeout %lu PC%05o\n",(unsigned long)n,get(RegZ));return 2;}for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;cm_alarm2_image_dump(n,(uint16_t)status,&s,&io);
 }return 0;
}
