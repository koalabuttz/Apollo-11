/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Input-only fresh-original oracle; never linked into native code. */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "sband_length_cases.h"
static agc_t originals[2];
int main(int argc,char **argv){uint32_t n,start,end;unsigned i;
 if(argc<3||agc_load_binfile(&originals[0],argv[1])||agc_load_binfile(&originals[1],argv[2]))return 2;
 start=argc>3?(uint32_t)strtoul(argv[3],0,0):0;end=argc>4?(uint32_t)strtoul(argv[4],0,0):327680;InhibitAlarms=1;
 for(n=start;n<end;++n){apollo_overlay_state m;p66_bias_io io;enum apollo_fixed_mission mission;unsigned cycles;uint16_t row[2053];
  sband_length_input(&m,&io,n,&mission);machine=originals[mission];
  for(i=0;i<2048u;++i){*memory(i)=(int16_t)m.words[i];}
  machine.AllowInterrupt=io.interrupt_enabled;machine.OutputChannel7=machine.InputChannel[7]=io.superbank;
  for(cycles=0;cycles<20000u;++cycles){if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&(get(5)==(mission==APOLLO_FIXED_LM?06061u:06030u)||get(5)==03402u))break;
   if(getenv("SBAND_LENGTH_TRACE")&&!machine.PendFlag&&!machine.ExtraDelay){fprintf(stderr,"PC%o A%o L%o Q%o EB%o FB%o LOC%o CYR%o EDOP%o ADDR%o POL%o PUSH%o\n",get(5),get(0),get(1),get(2),get(3),get(4),get(0164),get(020),get(023),get(0116),get(0117),get(0166));}
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;
  }
  if(cycles==20000u){fprintf(stderr,"ABVAL %u timeout %o\n",n,get(5));return 2;}
  row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=(uint16_t)machine.AllowInterrupt;row[3]=(uint16_t)machine.OutputChannel7;row[4]=(uint16_t)(get(5)==03402u?SBAND_LENGTH_ABORT_1302:SBAND_LENGTH_OK);
  for(i=0;i<2048u;++i){row[i+5u]=(uint16_t)((unsigned)*memory(i)&(i<3u?0177777u:077777u));}
  if(fwrite(row,sizeof row,1,stdout)!=1)return 2;
 }return 0;
}
