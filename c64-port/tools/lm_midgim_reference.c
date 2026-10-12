/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Independent host-only original engine; never part of native C64 port.
 * Runs the unchanged Luminary midcourse display subroutine of each sampled
 * entry from its NEWOPS until the caller's NEWOPS frontier (FBANK and LOC
 * the return that the entry QPRET names; for SELECTMU, VN1645 at 35,3606),
 * the UNIT abort frontier (POODOO1 05726 with Q 03404) or ARCSIN's
 * argument-too-large frontier (03723 in bank 0, alarm 1301). With a fourth
 * argument it emits, for case argv[3], every NEWOPS image and the end
 * (development trace). */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_midgim_cases.h"
static agc_t pristine;
static int emit(uint32_t n,unsigned tag){uint16_t row[2053];unsigned i;row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=(uint16_t)machine.AllowInterrupt;row[3]=(uint16_t)machine.OutputChannel7;row[4]=(uint16_t)tag;for(i=0;i<2048u;++i)row[i+5u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));return fwrite(row,sizeof row,1,stdout)==1;}
int main(int argc,char **argv){uint32_t n,first,limit;int trace=argc>4;if(argc<3||agc_load_binfile(&pristine,argv[1]))return 2;first=argc>3?(uint32_t)strtoul(argv[3],0,0):0u;limit=trace?first+1u:(uint32_t)strtoul(argv[2],0,0);InhibitAlarms=1;
 for(n=first;n<limit;++n){apollo_overlay_state m;p66_bias_io io;unsigned i,status=1,steps=0,cycles,q,fb,loc;lm_midgim_input(&m,&io,n);machine=pristine;machine.AllowInterrupt=io.interrupt_enabled;machine.InputChannel[7]=machine.OutputChannel7=io.superbank;
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];
  q=m.words[4]==020000u?073606u:m.words[m.words[0120]+052u];fb=q&076000u;loc=02000u+(q&01777u);
  for(cycles=0;cycles<2000000u;++cycles){if(!machine.PendFlag&&!machine.ExtraDelay){unsigned pc=get(5);
    if(cycles&&pc==06070u&&get(4)==fb&&get(0164)==loc){if(trace&&!emit(n,steps++))return 2;break;}
    if(cycles&&pc==05726u&&get(2)==03404u){if(trace&&!emit(n,steps++))return 2;status=2;break;}
    if(cycles&&pc==03723u&&get(4)==0u){if(trace&&!emit(n,steps++))return 2;status=3;break;}
    if(trace&&pc==06070u&&!emit(n,steps++))return 2;}
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==2000000u){fprintf(stderr,"MIDGIM %u timeout %o\n",n,get(5));return 2;}
  if(!trace&&!emit(n,status))return 2;}
 return 0;}
