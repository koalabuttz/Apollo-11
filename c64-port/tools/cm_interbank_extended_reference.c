/* SPDX-License-Identifier: GPL-2.0-or-later; source-only full rawA interbank branches. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_interbank_extended_cases.h"
#include "cm_fixed_compat_dump.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t n;unsigned i,cycles;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_INTERBANK_EXTENDED_CASES;++n){apollo_overlay_state s;cm_native_io io;unsigned kind=(unsigned)(n/65536u);machine=baseline;(void)cm_interbank_extended_input(&s,&io,n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  if(kind==CM_IB_BANKJUMP&&(s.words[0]&0140000u)==0100000u){cm_fixed_dump(n,(uint16_t)kind,(unsigned)EXEC_OUT_OF_SCOPE,&s,&io);continue;}
  for(cycles=0;cycles<200u;++cycles){unsigned z=get(RegZ);if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&((kind==CM_IB_SWRETURN||kind==CM_IB_ISWRETURN)?z==0710u:(kind==CM_IB_MAKECADR||kind==CM_IB_SUPDACAL||kind==CM_IB_SUPERSW)?z==0705u:z<04662u||z>04775u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==200u){fprintf(stderr,"source extended interbank timeout %lu PC%05o BB%05o\n",(unsigned long)n,get(RegZ),get(RegBB));return 2;}
  for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;cm_fixed_dump(n,(uint16_t)kind,(unsigned)EXEC_OK,&s,&io);
 }return 0;}
