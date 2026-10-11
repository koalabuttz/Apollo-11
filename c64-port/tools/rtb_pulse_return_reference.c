/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "rtb_pulse_return_cases.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t n,limit=RTB_PULSE_RETURN_CASES;unsigned mission;if(argc<3)return 2;mission=(unsigned)strtoul(argv[2],0,0);if(argc>3)limit=(uint32_t)strtoul(argv[3],0,0);memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;for(n=0;n<limit;++n){apollo_overlay_state m;p66_bias_io io;unsigned i,cycles;uint16_t row[2053];rtb_pulse_return_input(&m,&io,(enum rtb_mission)mission,n);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=io.interrupt_enabled;machine.InputChannel[7]=machine.OutputChannel7=io.superbank;for(cycles=0;cycles<10u;++cycles){if(!machine.PendFlag&&!machine.ExtraDelay&&cycles&&get(5)==(mission?06030u:06061u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==10u)return 2;row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=(uint16_t)machine.AllowInterrupt;row[3]=(uint16_t)machine.InputChannel[7];row[4]=0;for(i=0;i<2048u;++i)row[i+5u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
