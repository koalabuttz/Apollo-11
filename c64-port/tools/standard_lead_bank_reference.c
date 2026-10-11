/* SPDX-License-Identifier: GPL-2.0-or-later; host-only independent source. */
#define main old_reference_main
#include "reference_runner.c"
#undef main
#include "standard_lead_bank_cases.h"
#include "standard_lead_dump.h"
static agc_t pristine;
int main(int argc,char **argv){uint16_t n;unsigned i,phase,cycles,target,fb;apollo_overlay_state s;
 if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;
 for(n=0;n<STANDARD_LEAD_BANK_CASES;++n){standard_lead_bank_input(&s,STANDARD_LEAD_MISSION,n);machine=pristine;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.AllowInterrupt=0;machine.OutputChannel7=machine.InputChannel[7]=0160;
  for(phase=0;phase<3u;++phase){target=phase==0u?(STANDARD_LEAD_MISSION==STANDARD_LEAD_LM?02742u:02711u):phase==1u?(STANDARD_LEAD_MISSION==STANDARD_LEAD_LM?02607u:02047u):(unsigned)((s.words[STANDARD_LEAD_MISSION==STANDARD_LEAD_LM?01167:0364])&07777u);fb=phase==0u?06000u:phase==1u?(STANDARD_LEAD_MISSION==STANDARD_LEAD_LM?010000u:026000u):0;
   for(cycles=0;cycles<1000u;++cycles){if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&get(RegZ)==target&&(phase==2u||get(RegFB)==fb))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
   if(cycles==1000u){fprintf(stderr,"lead bank timeout%u/%u PC%05o FB%06o target%05o\n",n,phase,get(RegZ),get(RegFB),target);return 2;}
   for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));standard_lead_dump((unsigned)n*3u+phase,0,&s);if(machine.AllowInterrupt||machine.OutputChannel7!=0160)return 2;
  }
 }return 0;}
