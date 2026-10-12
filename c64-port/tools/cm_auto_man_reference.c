/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only oracle runner: unchanged Comanche055 AUTOMATIC_MANEUVERS in the
 * pinned yaAGC, from each entry to the first transfer out of the file
 * (RESUME, REINIT, T6PROGM, STICKCHK, SMALLMP). NEVER part of the native
 * port. */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_auto_man_cases.h"
#define LIMIT 4000u
static agc_t baseline;
static int frontier(unsigned z){static const unsigned f[]={05333u,02534u,03067u,03130u,02042u};unsigned i;for(i=0;i<sizeof f/sizeof f[0];++i)if(z==f[i])return 1;return 0;}
int main(int argc,char **argv){uint32_t n,limit=CM_AUTO_MAN_CASES;if(argc<2)return 2;if(argc>2)limit=(uint32_t)strtoul(argv[2],NULL,0);memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
for(n=0;n<limit;++n){agc_image m;cm_auto_man_io io;unsigned i,cycles;uint16_t row[CM_AUTO_MAN_ROW_WORDS];cm_auto_man_input(&m,&io,n);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=0;machine.InputChannel[031]=io.channel31;machine.InputChannel[032]=io.channel32;
 for(cycles=0;cycles<LIMIT;++cycles){if(!machine.PendFlag&&!machine.ExtraDelay&&frontier(get(5)))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 if(cycles==LIMIT)return 3;
 memset(row,0,sizeof row);row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);
 for(i=0;i<2048u;++i)row[i+5u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));
 if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}
return 0;}
