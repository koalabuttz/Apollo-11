/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original source oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_enema_image_cases.h"
#include "cm_enema_image_dump.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t n;unsigned i,cycles;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_ENEMA_IMAGE_CASES;++n){apollo_overlay_state s;cm_native_io io;unsigned z;uint16_t status;machine=baseline;cm_enema_image_input(&s,&io,n);
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;machine.InputChannel[011]=io.channel11;machine.InputChannel[012]=io.channel12;machine.InputChannel[013]=io.channel13;machine.InputChannel[014]=io.channel14;machine.InputChannel[015]=io.main_keys;machine.InputChannel[016]=io.navigation_keys;machine.Trap31A=io.trap31a;machine.Trap31B=io.trap31b;machine.Trap32=io.trap32;
  for(cycles=0;cycles<10000u;++cycles){z=get(RegZ);if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&(z==05360u||z==02746u||z==02365u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==10000u){fprintf(stderr,"CM canonical ENEMA timeout %lu PC%05o\n",(unsigned long)n,get(RegZ));return 2;}
  status=z==05360u?CM_ENEMA_DISPLAY:z==02746u?CM_ENEMA_BAD_PHASE:CM_ENEMA_FRESH_START;
  for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;io.channel11=(agc_word)machine.InputChannel[011];io.channel12=(agc_word)machine.InputChannel[012];io.channel13=(agc_word)machine.InputChannel[013];io.channel14=(agc_word)machine.InputChannel[014];io.trap31a=(agc_word)machine.Trap31A;io.trap31b=(agc_word)machine.Trap31B;io.trap32=(agc_word)machine.Trap32;cm_enema_image_dump(n,status,&s,&io);
 }return 0;
}
