/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_rcs_matrix_cases.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t n,limit=CM_RCS_MATRIX_CASES;if(argc<2)return 2;if(argc>2)limit=(uint32_t)strtoul(argv[2],NULL,0);memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;for(n=0;n<limit;++n){apollo_overlay_state m;agc_word irq;unsigned i,cycles;uint16_t row[2051];cm_rcs_matrix_input(&m,&irq,n);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=irq;for(cycles=0;cycles<6000u;++cycles){if(!machine.PendFlag&&!machine.ExtraDelay&&get(5)==05217u)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==6000u){fprintf(stderr,"case%u timeoutPC%o\n",n,get(5));return 2;}row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=(uint16_t)machine.AllowInterrupt;for(i=0;i<2048u;++i)row[i+3u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
