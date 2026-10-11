/* SPDX-License-Identifier: GPL-2.0-or-later; source oracle only. */
#define main earlier_main
#include "reference_runner.c"
#undef main
#include "standard_lead_cm_cases.h"
#include "standard_lead_dump.h"
static agc_t pristine;
int main(int argc,char **argv){uint16_t n;unsigned i,cycles,target,status;apollo_overlay_state s;cm_native_io io;
 if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;
 for(n=0;n<STANDARD_LEAD_CM_CASES;++n){standard_lead_cm_input(&s,&io,n);machine=pristine;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.AllowInterrupt=io.interrupt_enabled;machine.OutputChannel7=machine.InputChannel[7]=io.superbank;target=n<512u?03436:n<1024u?03131:02004;status=0;
  for(cycles=0;cycles<15000u;++cycles){if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&get(RegZ)==target&&get(RegFB)==066000)break;if(!machine.PendFlag&&!machine.ExtraDelay&&get(RegZ)==07755){status=EXEC_NO_CORE;break;}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==15000u){fprintf(stderr,"CM TESTLEAD timeout%u PC%05o\n",n,get(RegZ));return 2;}
  for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));standard_lead_dump(n,status,&s);fprintf(stderr,"%u %u %u\n",n,(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt);
 }return 0;}
