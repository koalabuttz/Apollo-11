/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Independent host-only original engine; never part of native C64 port.
 * Runs unchanged Luminary TIME_OF_FREE_FALL for each sampled call chain:
 * each call from its entry NEWOPS until the actual caller NEWOPS frontier
 * (FBANK no longer 27) or the SQRT abort frontier (POODOO 05726 from
 * SQRTSUB, Q 03404). With a fourth argument it emits, for case argv[3],
 * every NEWOPS image (each call's entry, every intermediate frontier and the
 * caller's) and the abort frontier (development trace). */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_time_free_fall_cases.h"
static agc_t pristine;
static int emit(uint32_t n,unsigned tag){uint16_t row[2053];unsigned i;row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=(uint16_t)machine.AllowInterrupt;row[3]=(uint16_t)machine.OutputChannel7;row[4]=(uint16_t)tag;for(i=0;i<2048u;++i)row[i+5u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));return fwrite(row,sizeof row,1,stdout)==1;}
int main(int argc,char **argv){uint32_t n,first,limit;int trace=argc>4;if(argc<3||agc_load_binfile(&pristine,argv[1]))return 2;first=argc>3?(uint32_t)strtoul(argv[3],0,0):0u;limit=trace?first+1u:(uint32_t)strtoul(argv[2],0,0);InhibitAlarms=1;
 for(n=first;n<limit;++n){apollo_overlay_state m;p66_bias_io io;unsigned i,status=1,steps=0;uint8_t k,calls=lm_time_free_fall_calls(n);lm_time_free_fall_input(&m,&io,n);machine=pristine;machine.AllowInterrupt=io.interrupt_enabled;machine.InputChannel[7]=machine.OutputChannel7=io.superbank;
  for(k=0;k<calls&&status==1u;++k){unsigned cycles;
   if(k){for(i=0;i<2048u;++i)m.words[i]=(agc_word)((unsigned)*memory(i)&(i<2u?0177777u:077777u));lm_time_free_fall_call(&m,n,k);}
   for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];
   for(cycles=0;cycles<200000u;++cycles){if(!machine.PendFlag&&!machine.ExtraDelay){unsigned pc=get(5);
     if(cycles&&pc==06070u&&get(4)!=056000u){if(trace&&!emit(n,steps++))return 2;break;}
     if(cycles&&pc==05726u&&get(2)==03404u){if(trace&&!emit(n,steps++))return 2;status=2;break;}
     if(trace&&pc==06070u&&!emit(n,steps++))return 2;}
    machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
   if(cycles==200000u){fprintf(stderr,"TFF %u call %u timeout %o\n",n,k,get(5));return 2;}}
  if(!trace&&!emit(n,status))return 2;}
 return 0;}
