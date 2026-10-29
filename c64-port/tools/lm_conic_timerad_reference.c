/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Independent host-only original engine; never part of native C64 port.
 * Runs unchanged Luminary TIMERAD or APSIDES for each sampled entry from its NEWOPS
 * until the caller's NEWOPS frontier (FBANK neither 12 nor 4 at NEWOPS), the
 * UNIT/SQRT abort frontier (POODOO1 05726 with Q 03404) or ABTCONIC's
 * POODOO frontier (05652 with Q 12,3744). With a fourth
 * argument it emits, for case argv[3], every NEWOPS image (the entry,
 * every intermediate frontier and the caller's) and the abort frontier
 * (development trace). */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_conic_timerad_cases.h"
static agc_t pristine;
static int emit(uint32_t n,unsigned tag){uint16_t row[2053];unsigned i;row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=(uint16_t)machine.AllowInterrupt;row[3]=(uint16_t)machine.OutputChannel7;row[4]=(uint16_t)tag;for(i=0;i<2048u;++i)row[i+5u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));return fwrite(row,sizeof row,1,stdout)==1;}
int main(int argc,char **argv){uint32_t n,first,limit;int trace=argc>4;if(argc<3||agc_load_binfile(&pristine,argv[1]))return 2;first=argc>3?(uint32_t)strtoul(argv[3],0,0):0u;limit=trace?first+1u:(uint32_t)strtoul(argv[2],0,0);InhibitAlarms=1;
 for(n=first;n<limit;++n){apollo_overlay_state m;p66_bias_io io;unsigned i,status=1,steps=0,cycles;lm_conic_timerad_input(&m,&io,n);machine=pristine;machine.AllowInterrupt=io.interrupt_enabled;machine.InputChannel[7]=machine.OutputChannel7=io.superbank;
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];
  for(cycles=0;cycles<20000000u;++cycles){if(!machine.PendFlag&&!machine.ExtraDelay){unsigned pc=get(5);
    if(cycles&&pc==06070u&&get(4)!=024000u&&get(4)!=010000u){if(trace&&!emit(n,steps++))return 2;break;}
    if(cycles&&pc==05726u&&get(2)==03404u){if(trace&&!emit(n,steps++))return 2;status=2;break;}
    if(cycles&&pc==05652u&&get(2)==03744u&&get(4)==024000u){if(trace&&!emit(n,steps++))return 2;status=3;break;}
    if(trace&&pc==06070u&&!emit(n,steps++))return 2;}
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==20000000u){fprintf(stderr,"TIMERAD %u timeout %o\n",n,get(5));return 2;}
  if(!trace&&!emit(n,status))return 2;}
 return 0;}
