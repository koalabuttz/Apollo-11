/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only oracle runner: unchanged Comanche055 MASSPROP in the pinned yaAGC.
 * NEVER part of the native port. */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_tvc_massprop_cases.h"
#define LIMIT 4000u
static agc_t baseline;
int main(int argc,char **argv){uint32_t n,limit=CM_TVC_MASSPROP_CASES;if(argc<2)return 2;if(argc>2)limit=(uint32_t)strtoul(argv[2],NULL,0);memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
for(n=0;n<limit;++n){agc_image m;agc_word irq;unsigned i,cycles,q;uint16_t row[CM_TVC_MASSPROP_ROW_WORDS];cm_tvc_massprop_input(&m,&irq,n);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=irq;q=m.words[2]&077777u;
 for(cycles=0;cycles<LIMIT;++cycles){if(!machine.PendFlag&&!machine.ExtraDelay&&get(5)==q&&cycles)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 memset(row,0,sizeof row);row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[3]=(uint16_t)machine.AllowInterrupt;
 if(cycles==LIMIT)row[2]=CM_TVC_MASSPROP_NO_EXIT;else for(i=0;i<2048u;++i)row[i+4u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));
 if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}
return 0;}
