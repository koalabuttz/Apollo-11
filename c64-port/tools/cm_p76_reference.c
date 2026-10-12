/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Independent host-only original engine; never part of native C64 port.
 * Runs the unchanged Comanche055 from each cm-p76 case's image
 * (tests/cm_p76_cases.c) to its first stop (cm_p76_stop: a NEWOPS
 * outside 13,2071-2230's interpretive code, UPFLAG, PHASCHNG, BANKCALL,
 * GOTOPOOH, INTPRET or POODOO).
 * Emits n lo, n hi, IRQ,
 * SUPER, tag 1 and the 2048 erasable words. */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_p76_cases.h"
static agc_t pristine;
static int emit(uint32_t n,unsigned tag){uint16_t row[2053];unsigned i;row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=(uint16_t)machine.AllowInterrupt;row[3]=(uint16_t)machine.OutputChannel7;row[4]=(uint16_t)tag;for(i=0;i<2048u;++i)row[i+5u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));return fwrite(row,sizeof row,1,stdout)==1;}
static int stopped(unsigned pc,unsigned fb,unsigned q,unsigned loc){return cm_p76_stop((agc_word)pc,(agc_word)fb,(agc_word)q,(agc_word)loc);}
int main(int argc,char **argv){uint32_t n,first,limit;if(argc<3||agc_load_binfile(&pristine,argv[1]))return 2;first=argc>3?(uint32_t)strtoul(argv[3],0,0):0u;limit=(uint32_t)strtoul(argv[2],0,0);InhibitAlarms=1;
 for(n=first;n<limit;++n){apollo_overlay_state m;p66_bias_io io;unsigned i,cycles;
  cm_p76_input(&m,&io,n);machine=pristine;machine.AllowInterrupt=io.interrupt_enabled;machine.InputChannel[7]=machine.OutputChannel7=io.superbank;
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];
  for(cycles=0;cycles<2000000u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode&&stopped((unsigned)get(5),(unsigned)get(4)&076000u,(unsigned)get(2),(unsigned)get(0164)))break;
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==2000000u){fprintf(stderr,"cm-p76 case %u timeout %o\n",n,get(5));return 2;}
  if(!emit(n,1))return 2;}
 return 0;}
