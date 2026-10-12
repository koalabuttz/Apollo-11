/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only oracle runner: unchanged Comanche055 TVCINITIALIZE in the pinned
 * yaAGC, from each entry to the first transfer out of the file. NEVER part
 * of the native port. S40.15 (entered by IBNKCALL) also stops at the
 * caller's return, where its TC Q goes. */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_tvc_init_cases.h"
#define LIMIT 4000u
static agc_t baseline;
static unsigned caller;
static int frontier(unsigned z){static const unsigned f[]={05333u,04740u,05245u,04701u,01613u,01703u};if(caller&&z==caller)return 1;unsigned i;for(i=0;i<sizeof f/sizeof f[0];++i)if(z==f[i])return 1;return 0;}
int main(int argc,char **argv){uint32_t n,limit=CM_TVC_INIT_CASES;if(argc<2)return 2;if(argc>2)limit=(uint32_t)strtoul(argv[2],NULL,0);memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
for(n=0;n<limit;++n){agc_image m;cm_tvc_init_io io;unsigned i,cycles;uint16_t row[CM_TVC_INIT_ROW_WORDS];cm_tvc_init_input(&m,&io,n);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=io.interrupt_enabled;machine.InputChannel[011]=io.channel11;caller=m.words[5]==02462u?m.words[2]:0u;
 for(cycles=0;cycles<LIMIT;++cycles){if(!machine.PendFlag&&!machine.ExtraDelay&&frontier(get(5)))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 if(cycles==LIMIT)return 3;
 memset(row,0,sizeof row);row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[3]=(uint16_t)(machine.InputChannel[011]&077777);row[4]=(uint16_t)machine.AllowInterrupt;
 for(i=0;i<2048u;++i)row[i+5u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));
 if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}
return 0;}
