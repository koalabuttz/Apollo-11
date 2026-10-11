/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Original instructions only; independent host oracle, never native target. */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_quality_cases.h"
static agc_t pristine;
int main(int argc,char **argv)
{
 uint32_t n,start=0,limit=65536;unsigned i;
 if(argc<2||agc_load_binfile(&pristine,argv[1]))return 2;
 if(argc>2)start=(uint32_t)strtoul(argv[2],NULL,0);if(argc>3)limit=(uint32_t)strtoul(argv[3],NULL,0);InhibitAlarms=1;
 for(n=start;n<limit;++n){apollo_overlay_state m;p66_bias_io io;unsigned cycles,phase=0;uint16_t row[2054];
  cm_quality_input(&m,&io,n);machine=pristine;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=io.interrupt_enabled;machine.InputChannel[7]=machine.OutputChannel7=io.superbank;
  for(cycles=0;cycles<20000u;++cycles){unsigned stop=cycles&&!machine.PendFlag&&!machine.ExtraDelay&&(get(5)==06030u||(get(5)==06037u&&get(4)==022000u&&(get(0164)==03146u||get(0164)==03220u)));
   if(stop){row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=(uint16_t)machine.AllowInterrupt;row[3]=(uint16_t)machine.OutputChannel7;row[4]=(uint16_t)phase++;row[5]=(uint16_t)(get(5)==06037u);
    for(i=0;i<2048u;++i)row[i+6u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));
    if(getenv("CM_QUALITY_TRACE"))fprintf(stderr,"n%u phase%u PC%o A%o L%o Q%o EB%o FB%o LOC%o CYR%o EDOP%o ADDR%o POL%o PUSH%o BANK%o INT%o\n",n,phase-1u,get(5),get(0),get(1),get(2),get(3),get(4),get(0164),get(020),get(023),get(0116),get(0117),get(0166),get(0165),get(0115));
    if(fwrite(row,sizeof row,1,stdout)!=1)return 2;if(get(5)==06037u)break;
   }
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;
  }
  if(cycles==20000u){fprintf(stderr,"QUALITY %u timeout PC%o LOC%o\n",n,get(5),get(0164));return 2;}
 }
 return 0;
}
