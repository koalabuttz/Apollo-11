/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only oracle runner: unchanged Comanche055 CM_ENTRY_DIGITAL_AUTOPILOT
 * in the pinned yaAGC, from each entry to the first transfer out of the
 * file (TASKOVER, WAITLIST, PHASCHNG, 2PHSCHNG, FLUSHJET, SPSIN, SPCOS,
 * IBNKCALL, RESUME, DOWNFLAG, BANKCALL, POSTJUMP, or bank 20's 2434 when an
 * overflowed TS CM/GYMDT skips TC WAITLIST onto its 2CADR). NEVER part of
 * the native port. CM/DAPIC's CDU-time loop runs up to about 1600 passes, so
 * the step limit is larger than the other Lane G runners'. */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_entry_dap_cases.h"
#define LIMIT 40000u
static agc_t baseline;
static int frontier(unsigned z){static const unsigned f[]={05324u,05245u,05412u,05372u,06000u,05075u,05074u,04740u,05333u,05560u,04662u,04701u};unsigned i;for(i=0;i<sizeof f/sizeof f[0];++i)if(z==f[i])return 1;return z==02434u&&(get(4)&076000)==040000;}
int main(int argc,char **argv){uint32_t n,limit=CM_ENTRY_DAP_CASES;if(argc<2)return 2;if(argc>2)limit=(uint32_t)strtoul(argv[2],NULL,0);memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
for(n=0;n<limit;++n){agc_image m;cm_entry_dap_io io;unsigned i,cycles;uint16_t row[CM_ENTRY_DAP_ROW_WORDS];cm_entry_dap_input(&m,&io,n);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=0;
 machine.InputChannel[05]=io.channel5;machine.InputChannel[06]=io.channel6;
 for(cycles=0;cycles<LIMIT;++cycles){if(!machine.PendFlag&&!machine.ExtraDelay&&frontier(get(5)))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 if(cycles==LIMIT){fprintf(stderr,"case %u: no exit\n",(unsigned)n);return 3;}
 memset(row,0,sizeof row);row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[3]=(uint16_t)(machine.InputChannel[05]&077777);row[4]=(uint16_t)(machine.InputChannel[06]&077777);
 for(i=0;i<2048u;++i)row[i+5u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));
 if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}
return 0;}
