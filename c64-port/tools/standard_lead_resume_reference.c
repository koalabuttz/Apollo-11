/* SPDX-License-Identifier: GPL-2.0-or-later; independent original epilogue. */
#define main earlier_main
#include "reference_runner.c"
#undef main
#include "standard_lead_resume_cases.h"
#include "standard_lead_dump.h"
static agc_t pristine;
int main(int argc,char **argv){uint16_t n;unsigned i,cycles,target;apollo_overlay_state s;
 if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;
 for(n=0;n<STANDARD_LEAD_RESUME_CASES;++n){standard_lead_resume_input(&s,STANDARD_LEAD_MISSION,n);machine=pristine;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.AllowInterrupt=n/512u;machine.OutputChannel7=machine.InputChannel[7]=0160;target=STANDARD_LEAD_MISSION==STANDARD_LEAD_LM?06070:06037;
  for(cycles=0;cycles<100u;++cycles){if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&get(RegZ)==target)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==100u||machine.AllowInterrupt!=n/512u||machine.OutputChannel7!=0160)return 2;
  for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));standard_lead_dump(n,0,&s);
 }return 0;}
