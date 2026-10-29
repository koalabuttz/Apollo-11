/* SPDX-License-Identifier: GPL-2.0-or-later; host-only source oracle. */
#define main prior_reference_main
#include "reference_runner.c"
#undef main
#include "standard_lead_interp_cases.h"
#include "standard_lead_dump.h"
static agc_t pristine;
int main(int argc,char **argv){uint16_t n;unsigned i,phase,cycles,target,loc,fb;apollo_overlay_state s;
 if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;
 for(n=0;n<STANDARD_LEAD_INTERP_CASES;++n){standard_lead_interp_input(&s,STANDARD_LEAD_MISSION,n);machine=pristine;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.AllowInterrupt=0;machine.InputChannel[7]=machine.OutputChannel7=0160;
  for(phase=0;phase<4u;++phase){
   target=STANDARD_LEAD_MISSION==STANDARD_LEAD_LM?06070:06037;
   loc=phase==0u?(STANDARD_LEAD_MISSION==STANDARD_LEAD_LM?03542u:03502u):phase<3u?(STANDARD_LEAD_MISSION==STANDARD_LEAD_LM?02626u:02066u):0301u+n%48u;
   fb=phase==0u?046000u:STANDARD_LEAD_MISSION==STANDARD_LEAD_LM?010000u:026000u;
   if(phase==2u)target=STANDARD_LEAD_MISSION==STANDARD_LEAD_LM?02627:02067;
   for(cycles=0;cycles<1000u;++cycles){if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&get(RegZ)==target&&get(RegFB)==fb&&get(0164)==loc)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
   if(cycles==1000u){fprintf(stderr,"interp lead timeout case%u phase%u PC%05o FB%06o LOC%05o\n",n,phase,get(RegZ),get(RegFB),get(0164));return 2;}
   for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));standard_lead_dump((unsigned)n*4u+phase,0,&s);
   if(machine.AllowInterrupt!=1||machine.OutputChannel7!=0160)return 2;
  }
 }return 0;}
