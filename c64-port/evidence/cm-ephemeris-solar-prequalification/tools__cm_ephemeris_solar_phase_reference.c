/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Original host engine checkpoints; no native execution or state replacement. */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_ephemeris_time_cases.h"
static agc_t pristine;
static agc_word before[2048];
static void save(agc_word *w){unsigned i;for(i=0;i<2048u;++i)w[i]=(agc_word)((unsigned)*memory(i)&(i<2u?0177777u:077777u));}
int main(int argc,char **argv){unsigned arg;if(argc<3||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;for(arg=2;arg<(unsigned)argc;++arg){uint32_t n=(uint32_t)strtoul(argv[arg],0,0);apollo_overlay_state m;p66_bias_io io;unsigned phase,cycles,i;cm_ephemeris_time_input(&m,&io,n);m.words[0164]=n%2u?02123:02110;m.words[m.words[0120]+052u]=023377;machine=pristine;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=io.interrupt_enabled;machine.InputChannel[7]=machine.OutputChannel7=io.superbank;
 for(phase=0;phase<5u;++phase){static const agc_word pc[4]={07350,03023,07427,07427},loc[4]={02147,02152,02154,02155};uint16_t row[4103];unsigned abort=0;save(before);
  for(cycles=0;cycles<20000u;++cycles){if(cycles&&!machine.PendFlag&&!machine.ExtraDelay){if(get(5)==03402u&&get(2)==03043u){abort=1;break;}if(phase<4u?(get(5)==pc[phase]&&get(0164)==loc[phase]):(get(5)==06037u&&get(0164)==(n%2u?03377u:02161u)))break;}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==20000u)return 2;row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=(uint16_t)phase;row[3]=(uint16_t)machine.AllowInterrupt;row[4]=(uint16_t)machine.OutputChannel7;row[5]=(uint16_t)(abort?2u:1u);row[6]=0;for(i=0;i<2048u;++i){row[7u+i]=before[i];row[2055u+i]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));}if(fwrite(row,sizeof row,1,stdout)!=1)return 2;if(abort)break;
 }
 }return 0;}
