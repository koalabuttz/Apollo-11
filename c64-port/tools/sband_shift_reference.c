/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Input-only source oracle, never linked into native C64/host numerical code. */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "sband_shift_cases.h"
static agc_t originals[2];
int main(int argc,char **argv){uint32_t n,start,end;unsigned i;
 if(argc<3||agc_load_binfile(&originals[0],argv[1])||agc_load_binfile(&originals[1],argv[2]))return 2;
 start=argc>3?(uint32_t)strtoul(argv[3],0,0):0;end=argc>4?(uint32_t)strtoul(argv[4],0,0):524288;InhibitAlarms=1;
 for(n=start;n<end;++n){apollo_overlay_state m;agc_word irq,super;enum apollo_fixed_mission mission;uint8_t count;unsigned cycles;uint16_t row[2053];
  sband_shift_input(&m,&irq,&super,n,&mission,&count);machine=originals[mission];
  for(i=0;i<2048u;++i){*memory(i)=(int16_t)m.words[i];}
  machine.AllowInterrupt=irq;machine.OutputChannel7=machine.InputChannel[7]=super;
  for(cycles=0;cycles<4000u;++cycles){if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&get(5)==(mission==APOLLO_FIXED_LM?06061u:06030u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==4000u){fprintf(stderr,"shift %u timeout %o\n",n,get(5));return 2;}
  row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=(uint16_t)machine.AllowInterrupt;row[3]=(uint16_t)machine.OutputChannel7;row[4]=1;
  for(i=0;i<2048u;++i){row[i+5u]=(uint16_t)((unsigned)*memory(i)&(i<3u?0177777u:077777u));}
  if(fwrite(row,sizeof row,1,stdout)!=1)return 2;
 }return 0;
}
