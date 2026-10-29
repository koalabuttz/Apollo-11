/* SPDX-License-Identifier: GPL-2.0-or-later; independent source oracle only. */
#define main existing_reference_main
#include "reference_runner.c"
#undef main
#include "standard_lead_cases.h"
#include "standard_lead_dump.h"
static agc_t pristine;
int main(int argc,char **argv){uint16_t n;unsigned i,cycles,target;apollo_overlay_state s;enum standard_lead_phase phase;agc_word fixed;
 if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;
 for(n=0;n<STANDARD_LEAD_STAGE_CASES;++n){standard_lead_input(&s,STANDARD_LEAD_MISSION,n,&phase,&fixed);machine=pristine;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.AllowInterrupt=0;machine.OutputChannel7=machine.InputChannel[7]=0160;
  if(phase==STANDARD_LEAD_BANK_PREPARE)target=STANDARD_LEAD_MISSION==STANDARD_LEAD_LM?04622:04666;
  else if(phase>=STANDARD_LEAD_BANK_RETURN0&&phase<=STANDARD_LEAD_BANK_RETURN2)target=s.words[STANDARD_LEAD_MISSION==STANDARD_LEAD_LM?01167:0364]+(unsigned)phase-STANDARD_LEAD_BANK_RETURN0;
  else if(phase==STANDARD_LEAD_INTERP_PREPARE)target=STANDARD_LEAD_MISSION==STANDARD_LEAD_LM?06037:06006;
  else if(phase==STANDARD_LEAD_INTERP_RESUME)target=STANDARD_LEAD_MISSION==STANDARD_LEAD_LM?06041:06010;
  else target=072;
  for(cycles=0;cycles<100u;++cycles){if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&get(RegZ)==target)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==100u){fprintf(stderr,"standard lead timeout case%u phase%u PC%05o expected%05o\n",n,(unsigned)phase,get(RegZ),target);return 2;}
  for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));standard_lead_dump(n,0,&s);
 }return 0;}
