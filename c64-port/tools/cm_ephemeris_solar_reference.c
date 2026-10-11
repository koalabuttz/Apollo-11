/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Independent host-only original engine; never part of native C64 port. */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_ephemeris_solar.h"
#include "cm_ephemeris_time_cases.h"
static agc_t pristine;
int main(int argc,char **argv){uint32_t n,limit;if(argc<3||agc_load_binfile(&pristine,argv[1]))return 2;limit=argc>3?(uint32_t)strtoul(argv[3],0,0):65536u;InhibitAlarms=1;
 for(n=0;n<limit;++n){apollo_overlay_state m;p66_bias_io io;uint16_t row[2053];unsigned i,cycles,done;cm_ephemeris_time_input(&m,&io,n);m.words[0164]=n%2u?02123:02110;m.words[m.words[0120]+052u]=n%2u?023377u:((n/2u)%2u?030231u:022557u);done=n%2u?03377:02161;machine=pristine;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=io.interrupt_enabled;machine.InputChannel[7]=machine.OutputChannel7=io.superbank;
 for(cycles=0;cycles<20000u;++cycles){if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&get(5)==06037u&&get(0164)==done)break;if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&get(5)==03402u&&get(2)==03043u)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==20000u){fprintf(stderr,"ephemeris moon %u timeout %o\n",n,get(5));return 2;}
 row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=(uint16_t)machine.AllowInterrupt;row[3]=(uint16_t)machine.OutputChannel7;row[4]=get(5)==03402u?2u:1u;for(i=0;i<2048u;++i)row[i+5u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
