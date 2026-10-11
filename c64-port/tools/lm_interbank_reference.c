/* SPDX-License-Identifier: GPL-2.0-or-later; original source oracle only. */
#define main prior_reference_main
#include "reference_runner.c"
#undef main
#include "lm_interbank_cases.h"
#include "lm_interbank_dump.h"
static agc_t pristine;
int main(int argc,char **argv){uint32_t n,start=argc>2?(uint32_t)strtoul(argv[2],0,0):0,end=argc>3?(uint32_t)strtoul(argv[3],0,0):LM_INTERBANK_CASES;unsigned i,cycles;apollo_overlay_state s;p66_bias_io io;
 if(argc<2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;
 for(n=start;n<end;++n){unsigned kind=lm_interbank_kind(n),caller,saved,base;lm_interbank_input(&s,&io,n);caller=s.words[2];base=kind==LM_IB_ISWRETRN?072:0133;saved=s.words[base];machine=pristine;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  for(cycles=0;cycles<200u;++cycles){unsigned z=get(RegZ);if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&((kind==LM_IB_BANKJUMP&&(s.words[0]&0140000u)==0100000u)?z==2u:(kind==LM_IB_SWRETURN||kind==LM_IB_ISWRETRN)?z==saved&&get(RegQ)==base+1u:(kind==LM_IB_MAKECADR||kind==LM_IB_SUPDACAL||kind==LM_IB_SUPERSW)?z==caller:z<04616u||z>04731u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==200u){fprintf(stderr,"LM interbank timeout%lu kind%u PC%05o BB%05o\n",(unsigned long)n,kind,get(RegZ),get(RegBB));return 2;}
  for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;lm_interbank_dump(n,(uint16_t)kind,0,&s,&io);
 }return ferror(stdout)?2:0;}
