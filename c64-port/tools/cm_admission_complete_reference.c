/* SPDX-License-Identifier: GPL-2.0-or-later; original source admission only. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_admission_complete_cases.h"
#include "cm_fixed_compat_dump.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t n;unsigned i,cycles;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_ADMISSION_COMPLETE_CASES;++n){apollo_overlay_state s;cm_native_io io;unsigned status=EXEC_OK,returned;machine=baseline;(void)cm_admission_complete_input(&s,&io,n);returned=n%4u==CM_ADMIT_SPVAC?s.words[2]:s.words[2]+2u;
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  for(cycles=0;cycles<1000u;++cycles){unsigned z=get(RegZ);if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&(z==07755u||(z==returned&&get(RegBB)==s.words[6])))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==1000u){fprintf(stderr,"CM complete admission timeout %lu PC%05o\n",(unsigned long)n,get(RegZ));return 2;}
  if(get(RegZ)==07755u)status=get(RegQ)==02642u?EXEC_NO_VAC:EXEC_NO_CORE;
  for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;cm_fixed_dump(n,(uint16_t)(n%4u),status,&s,&io);
 }return 0;
}
