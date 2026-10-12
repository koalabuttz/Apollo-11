/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only oracle runner: unchanged Comanche055 JET_SELECTION_LOGIC in the
 * pinned yaAGC, from each entry to its only exit, TCF RESUME. NEVER part
 * of the native port. The REPLACER/REPLACEP/REPLACEY subroutines return
 * inside the file and are not frontiers. */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_jet_select_cases.h"
#define LIMIT 4000u
static agc_t baseline;
int main(int argc,char **argv){uint32_t n,limit=CM_JET_SELECT_CASES;if(argc<2)return 2;if(argc>2)limit=(uint32_t)strtoul(argv[2],NULL,0);memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
for(n=0;n<limit;++n){agc_image m;cm_jet_select_io io;unsigned i,cycles;uint16_t row[CM_JET_SELECT_ROW_WORDS];cm_jet_select_input(&m,&io,n);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=0;
 machine.InputChannel[05]=io.channel5;machine.InputChannel[06]=io.channel6;machine.InputChannel[013]=io.channel13;machine.InputChannel[031]=io.channel31;
 for(cycles=0;cycles<LIMIT;++cycles){if(!machine.PendFlag&&!machine.ExtraDelay&&get(5)==05333u)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 if(cycles==LIMIT)return 3;
 memset(row,0,sizeof row);row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[3]=(uint16_t)(machine.InputChannel[05]&077777);row[4]=(uint16_t)(machine.InputChannel[06]&077777);row[5]=(uint16_t)(machine.InputChannel[013]&077777);
 for(i=0;i<2048u;++i)row[i+6u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));
 if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}
return 0;}
